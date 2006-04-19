/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * dbspectok.c - DB spec tokeniser
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
/**
 * @file dbspectok.c
 * @brief DB spec tokeniser.
 *
 * This file is currently defunct.
 */
/* $Amigan: fakedbfs/libfakedbfs/dbspectok.c,v 1.1 2006/04/19 18:55:05 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <float.h>
#include <ctype.h>

/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs/query.h>

#include <fakedbfs/fdbfsconfig.h>

#include "queryparser.h"
#include <fakedbfs/fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/dbspectok.c,v 1.1 2006/04/19 18:55:05 dcp1990 Exp $")

static size_t dbtoktl(cp, tval)
	char *cp;
	int *tval;
{
	size_t l = 1;
	char tdel;
	int i;
	char c;
	char nc = *(*cp == '\0' ? cp : cp + 1);

	*tval = 0;

	switch(*cp) {
		case '\n':
			/* if we want, we can increase some line counter of sorts or something */
		case '\t':
		case '\r':
		case ' ':
			for(i = 1; isspace(*(cp + i)); i++);
			*tval = SPACE;
			l += i - 1;
			break;
		case '"':
			/* XXX: finish this */
			break;
	}

	return 1;
}

int fdbfs_dbstok(cp, tval, toke, ctok)
	char **cp;
	int *tval;
	Toke *toke;
	char *ctok;
{
	int len, rc;
	char *last = *cp;

	len = dbtoktl(*cp, tval);

	strncpy(ctok, last, (len >= 511 ? 511 : len));

	if(*tval == -1 | *tval == ILLEGAL || *tval == 0)
		return 0;

	rc = extract_dstok_data(*cp, *tval, len, toke);

	*cp += len;

	return rc;
}
