/*
 * Copyright (c) 2005, Dan Ponte
 *
 * error.c - library error handling stuff
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
/* $Amigan: fakedbfs/libfakedbfs/error.c,v 1.6 2005/08/17 15:38:25 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h> /* this may not be portable...just rewrite these routines that use it if needed. */
/* us */
#include <fakedbfs.h>
#define BUFSIZE 512

RCSID("$Amigan: fakedbfs/libfakedbfs/error.c,v 1.6 2005/08/17 15:38:25 dcp1990 Exp $")

int ferr(fdbfs_t *f, enum ErrorAction severity, char *fmt, ...)
{
	va_list ap;
	char *p;
	if(fmt == NULL) return 0;
	p = malloc(sizeof(char) * BUFSIZE);
	f->error.freeit = 1;
	va_start(ap, fmt);
	vsnprintf(p, BUFSIZE, fmt, ap);
	f->error.emsg = p;
	f->error.action = severity;
	va_end(ap);
	return 0; /* it's an error condition; this way we can be lazy and just say return(ferr(blah)); */
}

int cferr(fdbfs_t *f, enum ErrorAction severity, char *fmt, ...)  /* cascade errors */
{
	va_list ap;
	char *p;
	size_t bs;

	if(fmt == NULL) return 0;
	bs = BUFSIZE + (f->error.emsg != NULL ? strlen(f->error.emsg) : 0) + 3;
	p = malloc(bs * sizeof(char));
	va_start(ap, fmt);
	vsnprintf(p, BUFSIZE, fmt, ap);
	va_end(ap);
	if(f->error.emsg != NULL) {
		strlcat(p, "{", bs);
		strlcat(p, f->error.emsg, bs);
		strlcat(p, "}", bs);
	}
	estr_free(&f->error);
	f->error.freeit = 1;
	f->error.emsg = p;
	f->error.action = severity;
	return 0;
}

int debug_info(fdbfs_t *f, enum ErrorAction sev, char *fmt, ...)
{
	va_list ap;
	char *p;

	va_start(ap, fmt);
	if(f->debugfunc == DEBUGFUNC_STDERR) {
		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);
	}
	else {
		if(vasprintf(&p, fmt, ap) == -1) {
			vfprintf(stderr, fmt, ap);
			fputc('\n', stderr);
		} else {
			f->debugfunc(p, sev);
			free(p);
		}
	}
	return 1;
}
