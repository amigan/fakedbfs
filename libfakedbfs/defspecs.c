/*
 * Copyright (c) 2005, Dan Ponte
 *
 * defspecs.c - indexing default specs
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
/* $Amigan: fakedbfs/libfakedbfs/defspecs.c,v 1.2 2005/11/24 02:41:56 dcp1990 Exp $ */
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
#include <query.h>

#include <fdbfsconfig.h>

#include <fakedbfs.h>
#include "dspparser.h"

RCSID("$Amigan: fakedbfs/libfakedbfs/defspecs.c,v 1.2 2005/11/24 02:41:56 dcp1990 Exp $")

static const char isIdChar[] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 2x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 3x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 4x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,  /* 5x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 6x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  /* 7x */
};

#define IdChar(C)  (((c=C)&0x80)!=0 || (c>0x1f && isIdChar[c-0x20]))

#define ParseTOKENTYPE Toke
#define ParseARG_PDECL , dspdata_t *d

void DSPParse(void *yyp, int yymajor, ParseTOKENTYPE yyminor ParseARG_PDECL);
void DSPParseTrace(FILE *TraceFILE, char *zTracePrompt);
void *DSPParseAlloc(void *(*mallocProc)(size_t));
void DSPParseFree(void *p, void (*freeProc)(void*));


static int dspstrtok(cp, len)
	char *cp;
	size_t len;
{
	if(strncasecmp(cp, "image", len) == 0) {
		return IMAGE;
	} else if(strncasecmp(cp, "binary", len) == 0) {
		return BINFILE;
	} else if(strncasecmp(cp, "true", len) == 0) {
		return B_TRUE;
	} else if(strncasecmp(cp, "false", len) == 0) {
		return B_FALSE;
	} else if(strncasecmp(cp, "enum", len) == 0) {
		return ENUM;
	} else if(strncasecmp(cp, "ensub", len) == 0) {
		return ENSUB;
	} else {
		return UQSTRING;
	}
}

size_t dsp_token(cp, tval)
	char *cp;
	int *tval;
{
	size_t l = 1;
	int i;
	int c, nc = 0;
	char tdel;

	*tval = 0;

	switch(*cp) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			for(i = 1; isspace(*(cp + i)); i++)
				;
			*tval = SPACE;
			l += i - 1;
			break;
		case '"':
		case '\'':
			tdel = *cp;
			for(i = 1; (c = *(cp + i)) != '\0'; i++) {
				if(c == tdel) {
					if(cp[i + 1] == tdel)
						i++;
					else
						break;
				}
			}

			if(c)
				i++;

			*tval = STRING;

			l += i - 1;
			break;
		case '=':
			*tval = ASSIGN;
			break;
		case '(':
			*tval = OPAREN;
			break;
		case ')':
			*tval = CPAREN;
			break;
		case ';':
			*tval = SEMICOLON;
			break;
		case ',':
			*tval = COMMA;
		case '\0':
			*tval = -1;
			l = 0;
			break;
	}

	if(*tval != 0)
		return l;
	else
		if(isdigit(*cp) || (*cp == '-' && isdigit(nc))) {
			if(isdigit(nc)) {
				*tval = SINT;
				i = 2;
			} else {
				i = 1;
				*tval = SINT; /* all signed */
			}

			for(; isdigit(*(cp + i)); i++)
				;

			if(*(cp + i) == '.' && isdigit(*(cp + i + 1))) {
				i += 2;
				while(isdigit(*(cp + i)))
					i++;
				*tval = FLOAT;
			}

			l += i - 1;
		} else {
			if(IdChar(*cp)) {
				for(i = 1; IdChar(*(cp + i)); i++)
					;
				*tval = dspstrtok(cp, i);
				l += i - 1;
			}
		}

	if(*tval != 0)
		return l;
	else {
		*tval = ILLEGAL;
		return 1;
	}

	/* NOTREACHED */
}

int ext_tdata(cp, t, len, toke)
	char *cp;
	int t;
	size_t len;
	Toke *toke;
{
	char ocp;

	switch(t) {
		case STRING:
			ocp = *(cp + (len - 1));
			*(cp + (len - 1)) = '\0';
			if(strlen(cp + 1) < 1) {
				*(cp + (len - 1)) = ocp;
				return -2;
			} else {
				toke->str = strdup(cp + 1);
			}
			*(cp + (len - 1)) = ocp;
			break;
		case UQSTRING:
			ocp = *(cp + len);
			*(cp + len) = '\0';
			toke->str = strdup(cp);
			*(cp + len) = ocp;
			break;
		case SINT:
			ocp = *(cp + len);
			*(cp + len) = '\0';
			toke->num = (int)strtol(cp, NULL, 10);
			*(cp + len) = ocp;
			break;
		case UINT:
			ocp = *(cp + len);
			*(cp + len) = '\0';
			toke->unum = (unsigned int)strtoul(cp, NULL, 10);
			*(cp + len) = ocp;
			break;
		case FLOAT:
			ocp = *(cp + len);
			*(cp + len) = '\0';
			toke->flt = malloc(sizeof(FLOATTYPE));
			*toke->flt = (FLOATTYPE)strtof(cp, NULL);
			*(cp + len) = ocp;
			break;
	}

	return 1;
}

int dsptok(cp, tval, toke, ctok)
	char **cp;
	int *tval;
	Toke *toke;
	char *ctok; /* 512 byte buffer */
{
	int len, rc;
	char *last = *cp;

	len = dsp_token(*cp, tval);

	strncpy(ctok, last, (len >= 511 ? 511 : len));

	if(*tval == -1 || *tval == ILLEGAL || *tval == 0)
		return 0;

	rc = ext_tdata(*cp, *tval, len, toke);

	*cp += len;

	return rc;
}


fields_t* fields_from_dsp(f, tsp)
	fdbfs_t *f;
	char *tsp;
{
	Toke to;
	dspdata_t d;
	char *cp = tsp;
	char ctb[513];
	void *pa;
	int trc;
	int token;

	memset(&d, 0, sizeof(d));

	d.f = f;

	pa = DSPParseAlloc(malloc);

	memset(ctb, 0, sizeof(ctb));

	while((trc = dsptok(&cp, &token, &to, ctb)) != 0) {
		if(token == SPACE)
			continue;
		if(trc == -1) {
			ERR(die, "Error tokenising %s!", tsp);
			DSPParseFree(pa, free);
			return 0;
		}
		d.yytext = ctb;
		DSPParse(pa, token, to, &d);
		if(d.f->error.emsg != NULL || d.error) {
			CERR(die, "Query parse of '%s'. ", tsp);
			DSPParseFree(pa, free);
			return 0;
		}
		memset(ctb, 0, sizeof(ctb));
	}
	DSPParse(pa, 0, to, &d);
	DSPParseFree(pa, free);
	return d.fhead;
}

