/*
 * Copyright (c) 2005, Dan Ponte
 *
 * fdbfsregex.h - regex stuff (POSIX or pcre)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
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
/* $Amigan: fakedbfs/include/fakedbfs/fdbfsregex.h,v 1.3 2005/12/24 22:11:37 dcp1990 Exp $ */
#ifndef HAVE_FDBRE_H
#define HAVE_FDBRE_H 1
#include <fdbfsconfig.h>

#ifdef USE_PCRE
#include <pcre.h>
#else
#ifndef _REGEX_H_
#include <regex.h>
#endif
#endif

#ifdef USE_PCRE
#define REGEX_T	pcre*
#define EXTRA_T pcre_extra*
#else
#define REGEX_T	regex_t
#define EXTRA_T void*
#endif

#define FREG_NOCASE	0x1
#define FREG_NOSUB	0x2

#define FREG_OK		0x0
#define FREG_NOMATCH	0x1

typedef struct {
	REGEX_T re;
	EXTRA_T extra;
	int offset; /* only for pcre; don't rely on it */
	char *errmsg;
	short int dynamic; /* errmsg malloc()d? */
} freg_t;

typedef struct {
	int s;
	int e;
} fregmatch_t;


/* functions */

int frinitialise(freg_t *fr);
freg_t* new_freg(char *emsg, size_t emsgsize);
void frerrfree(freg_t *fr);
void destroy_freg(freg_t *fr);
int fregcomp(freg_t *fr, char *regpat, int flags); /* posix regcomp semantics: rc == 0 means "ok" */
int fregexec(freg_t *fr, char *str, fregmatch_t *matches, size_t matchsize);
#endif
