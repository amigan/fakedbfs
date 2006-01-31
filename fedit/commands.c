/*
 * Copyright (c) 2005, Dan Ponte
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
/* $Amigan: fakedbfs/fedit/commands.c,v 1.7 2006/01/31 17:53:03 dcp1990 Exp $ */
/* system includes */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>

#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/fakedbfsapps.h>
RCSID("$Amigan: fakedbfs/fedit/commands.c,v 1.7 2006/01/31 17:53:03 dcp1990 Exp $")

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

