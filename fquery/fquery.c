/*
 * Copyright (c) 2005, Dan Ponte
 *
 * fquery.c - queryer for fakedbfs
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
/* $Amigan: fakedbfs/fquery/fquery.c,v 1.14 2006/03/06 05:10:18 dcp1990 Exp $ */
/* system includes */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>


#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define ARGSPEC "vhfnd:"
#define FQUERYVER "0.1"
#define MAXPLEN 1023

#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/fakedbfsapps.h>

RCSID("$Amigan: fakedbfs/fquery/fquery.c,v 1.14 2006/03/06 05:10:18 dcp1990 Exp $")

static int dbfu = 0;
static char *dbf = NULL;
static short int full = 0;
static short int nflag = 0;
static fdbfs_t *f;
static query_t *q;

void version(void)
{
	printf("fquery v%s. (C)2005, Dan Ponte.\n"
		       "Under the BSD license; see the source for more details.\n"
		       "fakedbfs library v%s ('%s'). Originally built for v%s.\n%s\n"
		       "Visit http://www.theamigan.net/fakedbfs/ for more info.\n",
		       FQUERYVER, fakedbfsver, fakedbfsvname, FAKEDBFSVER, fakedbfscopyright);
}

void usage(pn)
	char *pn;
{
	fprintf(stderr, "%s: usage: %s [-d dbfile] [-h] [-v] [-f] [-n] query\n",
			pn, pn);
}

int print_name(h)
	fields_t *h;
{
	fields_t *c;

	for(c = h; c != NULL; c = c->next) {
		if(strcmp(c->fieldname, "file") == 0) {
			printf("%s\n", (char*)c->val);
			return 1;
		}
	}

	return 0;
}

int print_name_null(h)
	fields_t *h;
{
	fields_t *c;

	for(c = h; c != NULL; c = c->next) {
		if(strcmp(c->fieldname, "file") == 0) {
			fputs((char*)c->val, stdout);
			fputc('\0', stdout);
			return 1;
		}
	}
	
	return 0;
}

int print_fields(h)
	fields_t *h;
{
	fields_t *c;
	char *tbf, *nl;

	for(c = h; c != NULL; c = c->next) {
		printf("%s (%s) = ", c->fmtname, c->fieldname);
		switch(c->type) {
			case string:
				printf("\"%s\"", (char*)c->val);
				break;
			case number:
				printf("%d (%x)", *(int*)c->val, *(int*)c->val);
				break;
			case bigint:
				printf("%lld (%llx)", *(long long int*)c->val, *(long long int*)c->val);
				break;
			case boolean:
				printf("%s", *(int*)c->val ? "true" : "false");
				break;
			case fp:
				printf("%g", *(FLOATTYPE*)c->val);
				break;
			case datime:
				tbf = ctime((const time_t*)c->val);
				if((nl = strrchr(tbf, '\n')) != NULL) {
					*nl = '\0'; /* shave off the newline */
				}
				printf("%s (%lld)", tbf, *(long long int*)c->val);
				break;
			case oenum:
				printf("%s (%d)", fdbfs_get_enum_string_by_value(c->ehead->headelem, *(int*)c->val, 1), *(int*)c->val);
				break;
			/* TODO: handle oenumsub */
			default:
				break;
		}
		printf("\n");
	}

	printf("\n");

	return 1;
}

int main(argc, argv)
	int argc;
	char *argv[];
{
	int c;
	int rc, trc = 0;
	fields_t *ch;
	char *estr, *pname, *cf;

	pname = strdup(argv[0]);

	while((c = getopt(argc, argv, ARGSPEC)) != -1)
		switch(c) {
			case 'v':
				version();
				free(pname);
				if(dbf != NULL) free(dbf);
				return 0;
			case 'h':
				usage(pname);
				free(pname);
				if(dbf != NULL) free(dbf);
				return 0;
			case 'f':
				full = 1;
				break;
			case 'd':
				dbf = strdup(optarg);
				dbfu = 1;
				break;
			case 'n':
				nflag = 1;
				break;
			case '?':
			default:
				usage(pname);
				free(pname);
				if(dbf != NULL) free(dbf);
				return 1;
		}
	argc -= optind;
	argv += optind;

	if(argc != 1) {
		usage(pname);
		free(pname);
		return -1;
	}

	free(pname);

	if(!dbfu) {
		if(getenv(FDBFSDBENV) != NULL)
			dbf = strdup(getenv(FDBFSDBENV));
		else
			asprintf(&dbf, "%s/.fakedbfsdb", getenv("HOME"));
	}

	f = fdbfs_new(dbf, &estr, DEBUGFUNC_STDERR, 1);
	if(f == NULL) {
		fprintf(stderr, "error creating fdbfs instance: %s\n", estr);
		free(dbf);
		free(estr);
		return -1;
	}

	if(!fdbfs_read_specs_from_db(f)) {
		fprintf(stderr, "error reading specs from db: %s\n", f->error.emsg);
		fdbfs_estr_free(&f->error);
		free(dbf);
		fdbfs_destroy(f);
		return -1;
	}

	
	cf = strdup(*argv);

	q = fdbfs_query_new(f, 0x0 /* default */);
	if(q == NULL) {
		fprintf(stderr, "error creating query: %s\n", f->error.emsg);
		fdbfs_estr_free(&f->error);
		free(dbf);
		free(cf);
		fdbfs_destroy(f);
		return 1;
	}

	if(!fdbfs_query_parse(q, cf)) {
		fprintf(stderr, "error compiling query: %s\n", f->error.emsg);
		fdbfs_estr_free(&f->error);
		free(dbf);
		free(cf);
		fdbfs_query_destroy(q);
		fdbfs_destroy(f);
		return 1;
	}

	while((rc = fdbfs_query_qne(q)) == Q_NEXT) {
		fdbfs_query_pop3(q, (void*)&ch); /* we don't care if US_DYNA is returned; we know it will be */
		if(full)
			print_fields(ch);
		else if(nflag)
			print_name_null(ch);
		else
			print_name(ch);
		fdbfs_free_field_list(ch);
	}

	if(rc == Q_FDBFS_ERROR) {
		fprintf(stderr, "error stepping: %s\n", f->error.emsg);
		fdbfs_estr_free(&f->error);
		trc = -2;
	} else if(rc != Q_FINISHED) {
		fprintf(stderr, "query execution error: %s\n", fdbfs_query_error(rc));
		trc = -3;
	} else {
		if(full)
			printf("--END--\n");
	}

	fdbfs_query_destroy(q);
	fdbfs_destroy(f);
	free(dbf);
	free(cf);

	return trc;
}
