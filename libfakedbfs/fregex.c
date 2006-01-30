/*
 * Copyright (c) 2005, Dan Ponte
 *
 * fregex.c - FDBFS regexp library abstraction layer
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
/* $Amigan: fakedbfs/libfakedbfs/fregex.c,v 1.8 2006/01/30 21:28:42 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <float.h>

/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs/query.h>

#include <fakedbfs/fdbfsconfig.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/fdbfsregex.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/fregex.c,v 1.8 2006/01/30 21:28:42 dcp1990 Exp $")

int frinitialise(fr)
	freg_t *fr;
{
	/* any initialisation code for specific reg librarys goes here */
	return 1; /* ok */
}

freg_t* new_freg(emsg, emsgsize)
	char *emsg;
	size_t emsgsize;
{
	freg_t *new;

	new = allocz(sizeof(*new));
	if(new == NULL) {
#ifdef __LP64__
#	define SIZEOF_FMT "%ld"
#else
#	define SIZEOF_FMT "%d"
#endif
		snprintf(emsg, emsgsize, "Error allocating " SIZEOF_FMT " bytes.", sizeof(*new));
		return NULL;
	}

	if(!frinitialise(new)) {
		strlcpy(emsg, new->errmsg, emsgsize);
		destroy_freg(new);
		return NULL;
	}

	return new;
}

void frerrfree(fr)
	freg_t *fr;
{
	if(fr->dynamic && fr->errmsg != NULL)
		free(fr->errmsg);
	fr->errmsg = NULL;
}

void destroy_freg(fr)
	freg_t *fr;
{
	frerrfree(fr);
#ifdef USE_PCRE
	if(fr->re != NULL)
		free(fr->re);
	if(fr->extra != NULL)
		free(fr->extra);
#else
	regfree(&fr->re);
#endif
	free(fr);
}

int fregcomp(fr, regpat, flags) /* posix regcomp semantics: rc == 0 means "ok" */
	freg_t *fr;
	char *regpat;
	int flags;
{
#ifdef USE_PCRE
	int pcflags = 0;
	if(flags & FREG_NOCASE)
		pcflags |= PCRE_CASELESS;
	
	fr->re = pcre_compile(regpat, pcflags, (const char**)&fr->errmsg, &fr->offset, NULL);
	if(fr->re == NULL) {
		fr->dynamic = 0;
		return 1;
	}

	fr->extra = pcre_study(fr->re, 0, (const char**)&fr->errmsg);
	if(fr->extra == NULL && fr->errmsg != NULL) {
		fr->dynamic = 0;
		return 2;
	}

	return 0;
#else
	int posixflags = REG_EXTENDED;
	int rc;

	if(flags & FREG_NOCASE)
		posixflags |= REG_ICASE;
	if(flags & FREG_NOSUB)
		posixflags |= REG_NOSUB;

	rc = regcomp(&fr->re, regpat, posixflags);
	if(rc) {
		fr->errmsg = malloc(128 * sizeof(char));
		fr->dynamic = 1;
		regerror(rc, &fr->re, fr->errmsg, 127);
		regfree(&fr->re);
		return rc;
	}

	return 0;
#endif
}

int fregexec(fr, str, matches, matchsize)
	freg_t *fr;
	char *str;
	fregmatch_t *matches;
	size_t matchsize; /* number of elements */
{
#ifdef USE_PCRE
	int rc, i;
	int *mats = NULL;
	size_t nmats = 0;

	if(matches != NULL) {
		nmats = matchsize * 3;
		mats = allocz(sizeof(*mats) * nmats);
	}

	rc = pcre_exec(fr->re, fr->extra, str, strlen(str), 0, 0, mats, mats != NULL ? nmats : 0);

	if(rc == PCRE_ERROR_NOMATCH) {
		if(mats != NULL)
			free(mats);
		return 1;
	} else if(rc < 0) {
		fr->errmsg = malloc(128 * sizeof(char));
		snprintf(fr->errmsg, 128, "Error with pcre_exec. (rc = %d)", rc);
		fr->dynamic = 1;
		if(mats != NULL)
			free(mats);
		return rc;
	} else {
		if(mats != NULL) {
			for(i = 0; i < nmats; i++) {
				matches[i].s = mats[(i * 2)];
				matches[i].e = mats[(i * 2) + 1];
			}
			free(mats);
		}
		return 0;
	}
	/* NOTREACHED */

	return rc;
#else
	regmatch_t *mats = NULL;
	int rc;
	int i;
	
	if(matches != NULL) {
		mats = allocz(sizeof(*mats) * matchsize);
	}

	rc = regexec(&fr->re, str, mats != NULL ? matchsize : 0, mats, 0);

	switch(rc) {
		case REG_NOMATCH:
			if(mats != NULL)
				free(mats);
			return 1;
		case 0:
			if(mats != NULL) {
				for(i = 0; i < matchsize; i++) {
					matches[i].s = mats[i].rm_so;
					matches[i].e = mats[i].rm_eo;
				}
				free(mats);
			}
			return 0;
		default:
			fr->errmsg = malloc(128 * sizeof(char));
			regerror(rc, &fr->re, fr->errmsg, 127);
			fr->dynamic = 1;
			if(mats != NULL)
				free(mats);
			return rc;
	}

	/* NOTREACHED */
	return rc;
#endif
}
