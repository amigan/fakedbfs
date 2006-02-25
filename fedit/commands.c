/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * commands.c - fedit commands
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the author nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* $Amigan: fakedbfs/fedit/commands.c,v 1.9 2006/02/25 09:52:13 dcp1990 Exp $ */
/* system includes */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>

#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/types.h>
#include <fakedbfs/conf.h>
#include <fakedbfs/fakedbfsapps.h>
RCSID("$Amigan: fakedbfs/fedit/commands.c,v 1.9 2006/02/25 09:52:13 dcp1990 Exp $")

#define COMMFLAG_MIN	0x1 /* at least this many args */
#define COMMFLAG_MAX	0x2 /* at most this many args */
#define COMMFLAG_EQU	0x3 /* exactly this many args */

extern fdbfs_t *f;


enum command_n {
	REMOVE_CAT, /* remove entire catalogue */
	EDIT_REC, /* edit record in catalogue */
	IEDIT_REC, /* interactively edit catalogue */
	REMOVE_REC, /* remove record in catalogue */
	EXPORT_CAT, /* export contents of entire catalogue or single record */
	EDIT_CSPEC, /* edit catalogue spec */
	EXPORT_CSPEC, /* export single cspec and all enums it depends on */
	EXPORT_CSPECS, /* export all specs */
	REMOVE_ENUM, /* remove enum */
	EDIT_ENUM, /* edit enumeration */
	LIST_CONF, /* show conf */
	SET_CONF, /* set conf MIB */
	LIST_CATS /* list catalogues */
};
	
struct command {
	char *commandname;
	enum command_n comm;
	int nargs;
	int commflag;
};

struct command cmds[] = {
	{"rmcat", REMOVE_CAT, 1, COMMFLAG_MIN /* args are a list of catalogues to remove */},
	{"editrec", EDIT_REC, 3 /* three arguments: first is the catalogue, second is a query specifying which to edit, third is a defspec */, COMMFLAG_EQU},
	{"iedit", IEDIT_REC, 0, COMMFLAG_EQU},
	{"rmrec", REMOVE_REC, 1, COMMFLAG_MIN /* args are just a bunch of querys specifying what to delete */},
	{"dumpcat", EXPORT_CAT, 1, COMMFLAG_MAX /* if an argument is given, it is the name of a file to dump to, otherwise stdout */},
	{"editspec", EDIT_CSPEC, 0, COMMFLAG_EQU /* this won't be implemented for a while, methinks */},
	{"dumpcspec", EXPORT_CSPEC, 2, COMMFLAG_MIN /* args are a list of specs and file to dump to, file last, use - for stdout */},
	{"dumpspecs", EXPORT_CSPECS, 1, COMMFLAG_MAX /* if given, arg is file, stdout otherwise */},
	{"rmenum", REMOVE_ENUM, 1, COMMFLAG_MIN /* arg is list of enum names to remove; this will break other cats! */},
	{"editenum", EDIT_ENUM, 0, COMMFLAG_EQU /* implemented when EDIT_CSPEC is */},
	{"lscats", LIST_CATS, 0, COMMFLAG_EQU /* list names of cats */},
	{"lsconf", LIST_CONF, 0, COMMFLAG_MIN /* MIBs to show */},
	{"setconf", SET_CONF, 1, COMMFLAG_MIN /* form of mib=value */},
	{NULL, 0, 0, 0} /* terminator */
};

int find_cmd(argc, argv)
	int argc;
	char **argv;
{
	struct command *cp;

	argc--;

	for(cp = cmds; cp->commandname != NULL && cp != NULL; cp++) {
		if(strcmp(*argv, cp->commandname) == 0) {
			switch(cp->commflag) {
				case COMMFLAG_MIN:
					if(argc < cp->nargs) {
						fprintf(stderr, "%s: %d is wrong number of arguments!\n", *argv, argc);
						return -1;
					}
					break;
				case COMMFLAG_MAX:
					if(argc > cp->nargs) {
						fprintf(stderr, "%s: %d is wrong number of arguments!\n", *argv, argc);
						return -1;
					}
					break;
				case COMMFLAG_EQU:
					if(argc != cp->nargs) {
						fprintf(stderr, "%s: %d is wrong number of arguments!\n", *argv, argc);
						return -1;
					}
					break;
			}
			return cp->comm;
		}
	}

	fprintf(stderr, "no such command '%s'\n", *argv);

	return -1;
}

static void cn_print_leaf(c, mib)
	confnode_t *c;
	char *mib;
{
	union Data dta;
	enum DataType dt;

	printf("%s: ", mib);
	dt = c->type;
	dta = c->data;
	switch(dt) {
		case number:
			printf("%d", dta.integer);
			break;
		case usnumber:
			printf("%u", dta.usint);
			break;
		case boolean:
			printf("%s", dta.integer ? "true" : "false");
			break;
		case string:
			printf("'%s'", dta.string);
			break;
		case fp:
			printf("%g", dta.fp);
			break;
		case datime:
			printf("%s (%lld)", ctime((const time_t *)&dta.linteger), dta.linteger);
			break;
		case character:
			printf("%c (%d, hex %x, oct %o)", dta.character, dta.character, dta.character, dta.character);
			break;
		case bigint:
			printf("%lld", dta.linteger);
			break;
		case usbigint:
			printf("%llu", dta.uslinteger);
			break;
		case -1:
			printf("error!");
			break;
		default:
			printf("datatype not supported");
			break;
	}
	printf("\n");
}


static void lsc_precurs(pre, c)
	char *pre; /* prefix */
	confnode_t *c; /* where to start */
{
	for(; c != NULL; c = c->next) {
		if(c->tag == NULL)
			continue;
		if(c->flags & CN_FLAG_LEAF) {
			size_t sz = strlen(pre) + strlen(c->tag) + 2;
			char *tbf;
			tbf = malloc(sz * sizeof(char));
			if(*pre != '\0') {
				strlcpy(tbf, pre, sz);
				strlcat(tbf, ".", sz);
			}
			strlcat(tbf, c->tag, sz);
			cn_print_leaf(c, tbf);
			free(tbf);
		} else {
			size_t sz = strlen(pre) + strlen(c->tag) + 2;
			char *tbf;
			tbf = malloc(sz * sizeof(char));
			if(strlen(pre) > 0) {
				strlcpy(tbf, pre, sz);
				strlcat(tbf, ".", sz);
				strlcat(tbf, c->tag, sz);
			} else {
				strlcpy(tbf, c->tag, sz);
			}
			lsc_precurs(tbf, c->child);
			free(tbf);
		}
	}
}

static void do_lsconf(argc, argv)
	int argc;
	char **argv;
{
	int i;
	confnode_t *c;

	if(argc == 1) {
		/* XXX: list all MIBs */
		lsc_precurs("", f->rconf);
	} else for(i = 1; i < argc; i++) {
		c = fdbfs_conf_search_mib(f->rconf, argv[i]);
		if(c == NULL) {
			fprintf(stderr, "%s: not found\n", argv[i]);
			continue;
		}
		if(c->flags & CN_FLAG_LEAF) {
			cn_print_leaf(c, argv[i]);
		} else {
			lsc_precurs(argv[i], c->child);
		}
	}
}

static enum DataType parse_datatype(val, dta, mib)
	char *val; /* format of typemod:value */
	data_t *dta;
	char *mib; /* only for error messages */
{
	char *c;
	enum DataType od;

	if(strlen(val) < 3) {
		fprintf(stderr, "error setting %s: assignment not long enough to be valid\n", mib);
		return -1;
	}

	if(val[1] != ':') {
		fprintf(stderr, "error setting %s: syntax error; need colon as part of assignment\n", mib);
		return -1;
	}
	c = val + 2;

	switch(*val) {
		case 'N':
		case 'n':
			od = number;
			dta->integer = atoi(c);
			break;
		case 'B':
		case 'b':
			od = boolean;
			dta->integer = (*c == 'f' || *c == 'F' ? 0 : 1);
			break;
		case 'F':
		case 'f':
			od = fp;
			dta->fp = atof(c);
			break;
		case 'L':
		case 'l':
			od = bigint;
			dta->linteger = atoll(c);
			break;
		case 'U':
		case 'u':
			od = usnumber;
			dta->usint = (unsigned int)strtoul(c, NULL, 0);
			break;
		case 'M':
		case 'm':
			od = usbigint;
			dta->uslinteger = strtoull(c, NULL, 0);
			break;
		case 'S':
		case 's':
			od = string;
			dta->string = strdup(c);
			break;
		case 'C':
		case 'c':
			od = character;
			dta->character = *c;
			break;
		case 'D':
		case 'd':
			od = datime; /* only accept epoch format */
			dta->linteger = atoll(c);
			break;
		default:
			fprintf(stderr, "error setting %s: type modifier not supported\n", mib);
			return -1;
	}

	return od;
}

static void do_setconf(argc, argv)
	int argc;
	char **argv;
{
	int i;
	char *eq, *cur, *rval;
	enum DataType dt;
	data_t dta;

	for(i = 1; i < argc; i++) {
		if((eq = strchr(argv[i], '=')) == NULL) {
			fprintf(stderr, "error setting %s: no assignment operator\n", argv[i]);
			continue;
		}
		cur = strdup(argv[i]);
		eq = cur + (eq - argv[i]); /* clever! now we don't need to enter strchr() again */
		rval = eq + 1;
		*eq = '\0';
		dt = parse_datatype(rval, &dta, cur);
		if(dt == -1) {
			free(cur);
			continue;
		}
		if(!fdbfs_conf_set(f, cur, dt, dta)) {
			fprintf(stderr, "error setting %s: %s\n", cur, (f->error.emsg != NULL ? f->error.emsg : "Unknown error"));
			free(cur);
			continue;
		}
		free(cur);
	}
}
		
int exec_command(cm, argc, argv)
	int cm;
	int argc;
	char **argv;
{
	int i;

	switch(cm) {
		case REMOVE_CAT:
			for(i = 1; i < argc; i++) {
				if(!fdbfs_db_rm_catalogue(f, argv[i])) {
					fprintf(stderr, "error removing catalogue '%s': %s\n", argv[i], f->error.emsg);
					return 0;
				}
			}
			break;
		case LIST_CATS:
			{
				struct CatalogueHead *c;
				for(c = f->heads.db_cath; c != NULL; c = c->next) {
					printf("%s (%s)\n", c->fmtname, c->name);
				}
			}
			break;
		case LIST_CONF:
			do_lsconf(argc, argv);
			break;
		case SET_CONF:
			do_setconf(argc, argv);
			break;
		case EDIT_REC: /* <catalogue> <query> <defspec> */
		default:
			fprintf(stderr, "command not implemented!\n");
			return 0;
	}

	return 1;
}


int do_commands(argc, argv)
	int argc;
	char **argv;
{
	int cm;
	int rc;

	cm = find_cmd(argc, argv);

	if(cm == -1) {
		return 0;
	}

	rc = exec_command(cm, argc, argv);

	return rc;
}

