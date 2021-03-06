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
/* $Amigan: fakedbfs/findex/findex.c,v 1.22 2006/01/29 21:03:55 dcp1990 Exp $ */
/* system includes */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/fakedbfsapps.h>

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define ARGSPEC "vhrfsd:ice:u:"
#define FINDEXVER "0.1"
#define MAXPLEN 1023

RCSID("$Amigan: fakedbfs/findex/findex.c,v 1.22 2006/01/29 21:03:55 dcp1990 Exp $")

static int dbfu = 0;
static int recurse = 0;
static int interactive = 0;
static int nocase = 0;
static int readstd = 0;
static int forceit = 0;
static char *dbf = NULL;
static char *regex = NULL;
static char *tsp = NULL;
char tbfr[MAXPLEN];
static fdbfs_t *f;
static fields_t *defhead = NULL;

void version(void)
{
	printf("findex v%s. (C)2005, Dan Ponte.\n"
		       "Under the BSD license; see the source for more details.\n"
		       "fakedbfs library v%s ('%s'). Originally built for v%s.\n%s\n"
		       "Visit http://www.theamigan.net/fakedbfs/ for more info.\n",
		       FINDEXVER, fakedbfsver, fakedbfsvname, FAKEDBFSVER, fakedbfscopyright);
}

void usage(pn)
	char *pn;
{
	fprintf(stderr, "%s: usage: %s [-d dbfile] [-h] [-v] [-i|-s] [-r] [-c] [-f] "
			"[-e regex] [-u values] <catalogue name> [file/dir ... (if not -s, required)]\n",
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
	char *cps[] = {cf, NULL}; /* hack, oh well */
	if(!fdbfs_index_dir(f, cps, cat, 1, (interactive ? 0 : 1), nocase, regex, recurse, defhead)) {
		fprintf(stderr, "Error in index_dir: %s\n", f->error.emsg);
		fdbfs_estr_free(&f->error);
		return 0;
	}

	return 1;
}



int main(argc, argv)
	int argc;
	char *argv[];
{
	int c, i, rc;
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
				if(tsp != NULL) free(tsp);
				if(regex != NULL) free(regex);
				return 0;
			case 'h':
				usage(pname);
				free(pname);
				if(dbf != NULL) free(dbf);
				if(tsp != NULL) free(tsp);
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
					if(tsp != NULL) free(tsp);
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
					if(tsp != NULL) free(tsp);
					if(regex != NULL) free(regex);
					return 1;
				} else {
					interactive = 1;
				}
				break;
			case 'u':
				tsp = strdup(optarg);
				break;
			case '?':
			default:
				usage(pname);
				free(pname);
				if(dbf != NULL) free(dbf);
				if(regex != NULL) free(regex);
				if(tsp != NULL) free(tsp);
				return 1;
		}
	argc -= optind;
	argv += optind;

	if(argc < (!readstd ? 2 : 1)) {
		usage(pname);
		free(pname);
		if(regex != NULL) free(regex);
		if(tsp != NULL) free(tsp);
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

	f = fdbfs_new(dbf, &estr, DEBUGFUNC_STDERR, 1);
	if(f == NULL) {
		fprintf(stderr, "error creating fdbfs instance: %s\n", estr);
		free(dbf);
		free(estr);
		free(cat);
		if(tsp != NULL) free(tsp);
		if(regex != NULL) free(regex);
		return -1;
	}

	if(!fdbfs_read_specs_from_db(f)) {
		fprintf(stderr, "error reading specs from db: %s\n", f->error.emsg);
		fdbfs_estr_free(&f->error);
		free(dbf);
		free(cat);
		if(tsp != NULL) free(tsp);
		if(regex != NULL) free(regex);
		fdbfs_destroy(f);
		return -1;
	}

	if((rc = fdbfs_db_cat_exists(f, cat)) != 1) {
		switch(rc) {
			case 0:
				fprintf(stderr, "Catalogue %s doesn't exist!\n", cat);
				free(dbf);
				free(cat);
				if(regex != NULL) free(regex);
				if(tsp != NULL) free(tsp);
				fdbfs_destroy(f);
				return -2;
			case -1:
			default:
				fprintf(stderr, "error checking if catalogue exists: %s\n", f->error.emsg);
				fdbfs_estr_free(&f->error);
				free(dbf);
				free(cat);
				if(tsp != NULL) free(tsp);
				if(regex != NULL) free(regex);
				fdbfs_destroy(f);
				return -1;
		}
	}

	if(tsp != NULL) {
		if((defhead = fdbfs_fields_from_dsp(f, tsp)) == NULL) {
			fprintf(stderr, "error creating default field list from '%s': %s\n", tsp, f->error.emsg);
			fdbfs_estr_free(&f->error);
			free(dbf);
			free(cat);
			free(tsp);
			if(regex != NULL) free(regex);
			fdbfs_destroy(f);
			return -1;
		}
	}

	
	for(i = 0; i < argc; i++) {
		cf = strdup(argv[i]);

		if(interactive)
			printf("File: %s\n", cf);

		if(!idxus(cf, cat)) {
			free(cf);
			break;
		} else
			free(cf);
	}

	if(readstd)
		while(!feof(stdin)) {
			if(fgets(tbfr, MAXPLEN, stdin) == NULL)
				break;
			if((tnl = strrchr(tbfr, '\n')) != NULL)
				*tnl = '\0';
			if(!idxus(tbfr, cat))
				break;
		}


	if(defhead != NULL)
		fdbfs_free_field_list(defhead);

	fdbfs_destroy(f);
	free(dbf);
	free(cat);
	if(tsp != NULL) free(tsp);
	if(regex != NULL) free(regex);

	return 0;
}
