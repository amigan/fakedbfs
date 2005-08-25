/*
 * Copyright (c) 2005, Dan Ponte
 *
 * findex.c - indexer for fakedbfs
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
/* $Amigan: fakedbfs/findex/findex.c,v 1.10 2005/08/25 18:52:08 dcp1990 Exp $ */
/* system includes */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include <fakedbfs.h>
#include <fakedbfsapps.h>

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define ARGSPEC "vhrfsd:ice:"
#define FINDEXVER "0.1"
#define RECURSELVL 10
#define MAXPLEN 1023

RCSID("$Amigan: fakedbfs/findex/findex.c,v 1.10 2005/08/25 18:52:08 dcp1990 Exp $")

static int dbfu = 0;
static int recurse = 0;
static int interactive = 0;
static int nocase = 0;
static int readstd = 0;
static int forceit = 0;
static char *dbf = NULL;
static char *regex = NULL;
char tbfr[MAXPLEN];
static fdbfs_t *f;

void version(void)
{
	printf("findex v%s. (C)2005, Dan Ponte.\n"
		       "Under the BSD license; see the source for more details.\n"
		       "fakedbfs library v%s. Originally built for v%s.\n%s\n"
		       "Visit http://www.theamigan.net/fakedbfs/ for more info.\n",
		       FINDEXVER, fakedbfsver, FAKEDBFSVER, fakedbfscopyright);
}

void usage(pn)
	char *pn;
{
	fprintf(stderr, "%s: usage: %s [-d dbfile] [-h] [-v] [-i|-s] [-r] [-c] [-f] "
			"[-e regex] <catalogue name> [file/dir ... (if not -s, required)]\n",
			pn, pn);
}

int is_dir(file)
	char *file;
{
#if defined(UNIX)
	struct stat s;

	if(stat(file, &s) == -1)
		return -1;

	return S_ISDIR(s.st_mode);
#else
	return 0;
#endif
}

int idxus(cf, cat)
	char *cf;
	char *cat;
{
	int c;
	c = is_dir(cf);
	if(c == -1) {
		fprintf(stderr, "warning: couldn't stat %s: %s\n", cf, strerror(errno));
	} else if(!c) {
		/* index the file */
		if(!index_file(f, cf, cat, (interactive ? 0 : 1), 1, forceit, NULL /* XXX: for now; implement specification on command line */)) {
			fprintf(stderr, "Error in index_file: %s\n", f->error.emsg);
			return 0;
		}
	} else if(c) {
		/* index the directory */
		if(!index_dir(f, cf, cat, 1, (interactive ? 0 : 1), nocase, regex, RECURSELVL)) {
			fprintf(stderr, "Error in index_dir: %s\n", f->error.emsg);
			return 0;
		}
	}

	return 1;
}



int main(argc, argv)
	int argc;
	char *argv[];
{
	int c, i;
	char *estr, *pname, *cat, *cf;
	char *tnl;

	pname = strdup(argv[0]);

	while((c = getopt(argc, argv, ARGSPEC)) != -1)
		switch(c) {
			case 'c':
				nocase = 1;
				break;
			case 'v':
				version();
				free(pname);
				if(dbf != NULL) free(dbf);
				if(regex != NULL) free(regex);
				return 0;
			case 'h':
				usage(pname);
				free(pname);
				if(dbf != NULL) free(dbf);
				if(regex != NULL) free(regex);
				return 0;
			case 'f':
				forceit = 1;
				break;
			case 'd':
				dbf = strdup(optarg);
				dbfu = 1;
				break;
			case 's':
				if(interactive) {
					fprintf(stderr, "error: -s and -i mutually exclusive\n");
					usage(pname);
					free(pname);
					if(dbf != NULL) free(dbf);
					if(regex != NULL) free(regex);
					return 1;
				} else {
					readstd = 1;
				}
				break;
			case 'r':
				recurse = 1;
				break;
			case 'e':
				regex = strdup(optarg);
				break;
			case 'i':
				if(readstd) {
					fprintf(stderr, "error: -s and -i mutually exclusive\n");
					usage(pname);
					free(pname);
					if(dbf != NULL) free(dbf);
					if(regex != NULL) free(regex);
					return 1;
				} else {
					interactive = 1;
				}
				break;
			case '?':
			default:
				usage(pname);
				free(pname);
				if(dbf != NULL) free(dbf);
				if(regex != NULL) free(regex);
				return 1;
		}
	argc -= optind;
	argv += optind;

	if(argc < (!readstd ? 2 : 1)) {
		usage(pname);
		free(pname);
		if(regex != NULL) free(regex);
		return -1;
	}

	cat = strdup(argv[0]);

	argc--;
	argv++;

	free(pname);

	if(!dbfu) {
		if(getenv(FDBFSDBENV) != NULL)
			dbf = strdup(getenv(FDBFSDBENV));
		else
			asprintf(&dbf, "%s/.fakedbfsdb", getenv("HOME"));
	}

	f = new_fdbfs(dbf, &estr, DEBUGFUNC_STDERR, 1);
	if(f == NULL) {
		fprintf(stderr, "error creating fdbfs instance: %s\n", estr);
		free(dbf);
		free(estr);
		free(cat);
		if(regex != NULL) free(regex);
		return -1;
	}

	if(!read_specs_from_db(f)) {
		fprintf(stderr, "error reading specs from db: %s\n", f->error.emsg);
		free(dbf);
		free(estr);
		free(cat);
		if(regex != NULL) free(regex);
		destroy_fdbfs(f);
		return -1;
	}

	
	for(i = 0; i < argc; i++) {
		cf = strdup(argv[i]);
		if(!idxus(cf, cat)) {
			free(cf);
			break;
		} else
			free(cf);
	}

	if(readstd)
		while(!feof(stdin)) {
			fgets(tbfr, MAXPLEN, stdin);
			if((tnl = strrchr(tbfr, '\n')) != NULL)
				*tnl = '\0';
			if(!idxus(tbfr, cat))
				break;
		}


	destroy_fdbfs(f);
	free(dbf);
	free(cat);
	if(regex != NULL) free(regex);

	return 0;
}
