/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * fedit.c - database editing utility
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
/* $Amigan: fakedbfs/fedit/fedit.c,v 1.8 2006/02/27 19:45:31 dcp1990 Exp $ */
/* system includes */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define ARGSPEC "vhd:"
#define FEDITVER "0.1"
#define MAXPLEN 1023

#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/fakedbfsapps.h>

RCSID("$Amigan: fakedbfs/fedit/fedit.c,v 1.8 2006/02/27 19:45:31 dcp1990 Exp $")

static int dbfu = 0;
static char *dbf = NULL;
fdbfs_t *f;

int do_commands(int, char**);

void version(void)
{
	printf("fedit v%s. (C)2005-2006, Dan Ponte.\n"
		       "Under the BSD license; see the source for more details.\n"
		       "fakedbfs library v%s ('%s'). Originally built for v%s.\n%s\n"
		       "Visit http://www.theamigan.net/fakedbfs/ for more info.\n",
		       FEDITVER, fakedbfsver, fakedbfsvname, FAKEDBFSVER, fakedbfscopyright);
}
#define BUFSIZE	2048
#define MELEMS	20
#ifdef WANT_SHELL
int do_shell(void)
{
	char *inbuf = malloc(BUFSIZE); /* use the heap, luke */
	char *rstr;
	char *agv[MELEMS];
	int argc;
	int rc;

	while(1) {
#ifdef HAVE_READLINE
		rstr = readline("fakedbfs> ");
		strlcpy(inbuf, rstr, BUFSIZE);
#else
		printf("fakedbfs> ");
		fgets(inbuf, BUFSIZE, stdin);
#endif
		if(cmd_split(inbuf, agv, MELEMS, &argc)) {
			rc = check_builtins(argc, agv);
			if(rc == 0)
				break;
			switch(rc) {
				case -1:
					continue;
					break;
				default:
					do_commands(argc, agv);
					break;
			}
		}
	}

	return 1;
}
#endif

void usage(pn)
	char *pn;
{
	fprintf(stderr, "%s: usage: %s [-d dbfile] [-h] [-v] command [arguments ...]\n",
			pn, pn);
}

int main(argc, argv)
	int argc;
	char *argv[];
{
	int c;
	int trc = 0;
	char *estr, *pname;

	pname = argv[0];

	while((c = getopt(argc, argv, ARGSPEC)) != -1)
		switch(c) {
			case 'v':
				version();
				if(dbf != NULL) free(dbf);
				return 0;
			case 'h':
				usage(pname);
				if(dbf != NULL) free(dbf);
				return 0;
			case 'd':
				dbf = strdup(optarg);
				dbfu = 1;
				break;
			case '?':
			default:
				usage(pname);
				if(dbf != NULL) free(dbf);
				return 1;
		}
	argc -= optind;
	argv += optind;

	if(argc < 1) {
		usage(pname);
		return -1;
	}

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

	trc = do_commands(argc, argv);

	fdbfs_destroy(f);
	free(dbf);

	return trc;
}
