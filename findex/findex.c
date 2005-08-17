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
/* $Amigan: fakedbfs/findex/findex.c,v 1.2 2005/08/17 15:30:12 dcp1990 Exp $ */
/* system includes */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <fakedbfs.h>
#include <fakedbfsapps.h>

#define ARGSPEC "vhrd:e:"
#define FINDEXVER "0.1"

RCSID("$Amigan: fakedbfs/findex/findex.c,v 1.2 2005/08/17 15:30:12 dcp1990 Exp $")

static int dbfu = 0;
static int recurse = 0;
char *dbf;
char *regex = NULL;
fdbfs_t *f;

void version(void)
{
	printf("findex v%s. (C)2005, Dan Ponte.\n"
		       "Under the BSD license; see the source for more details.\n"
		       "fakedbfs library v%s. Originally built for v%s.\n%s\n"
		       "Visit http://www.theamigan.net/fakedbfs/ for more info.\n", FINDEXVER, fakedbfsver, FAKEDBFSVER, fakedbfscopyright);
}
void usage(pn)
	char *pn;
{
	fprintf(stderr, "%s: usage: %s [-d dbfile] [-h] [-v] [-r] "
			"[-e regex] file/dir ...\n",
			pn, pn);
}

int main(argc, argv)
	int argc;
	char *argv[];
{
	int c;
	char *estr, *pname;

	pname = strdup(argv[0]);

	while((c = getopt(argc, argv, ARGSPEC)) != -1)
		switch(c) {
			case 'v':
				version();
				return 0;
			case 'h':
				usage(pname);
				return 0;
			case 'd':
				dbf = strdup(optarg);
				dbfu = 1;
				break;
			case 'r':
				recurse = 1;
				break;
			case 'e':
				regex = strdup(optarg);
				break;
			case '?':
			default:
				usage(pname);
				free(pname);
				return 1;
		}
	argc -= optind;
	argv += optind;

	if(argc < 1) {
		usage(pname);
		free(pname);
		if(regex != NULL) free(regex);
		return -1;
	}

	free(pname);

	if(!dbfu) {
		if(getenv(FDBFSDBENV) != NULL)
			dbf = strdup(getenv(FDBFSDBENV));
		else
			asprintf(&dbf, "%s/.fakedbfsdb", getenv("HOME"));
	}

	f = new_fdbfs(dbf, &estr, DEBUGFUNC_STDERR);
	if(f == NULL) {
		fprintf(stderr, "error creating fdbfs instance: %s\n", estr);
		free(dbf);
		free(estr);
		if(regex != NULL) free(regex);
		return -1;
	}

	destroy_fdbfs(f);
	free(dbf);
	if(regex != NULL) free(regex);

	return 0;
}
