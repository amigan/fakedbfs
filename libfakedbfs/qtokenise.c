/*
 * Copyright (c) 2005, Dan Ponte
 *
 * qtokenise.c - tokeniser for query parser
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
/* $Amigan: fakedbfs/libfakedbfs/qtokenise.c,v 1.10 2005/12/20 00:33:20 dcp1990 Exp $ */
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

#include "queryparser.h"
#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/qtokenise.c,v 1.10 2005/12/20 00:33:20 dcp1990 Exp $")


/* from SQLite... */
/*
** If X is a character that can be used in an identifier and
** X&0x80==0 then isIdChar[X] will be 1.  If X&0x80==0x80 then
** X is always an identifier character.  (Hence all UTF-8
** characters can be part of an identifier).  isIdChar[X] will
** be 0 for every character in the lower 128 ASCII characters
** that cannot be used as part of an identifier.
**
** In this implementation, an identifier can be a string of
** alphabetic characters, digits, and "_" plus any character
** with the high-order bit set.  The latter rule means that
** any sequence of UTF-8 characters or characters taken from
** an extended ISO8859 character set can form an identifier.
**
** Ticket #1066.  the SQL standard does not allow '$' in the
** middle of identfiers.  But many SQL implementations do. 
** SQLite will allow '$' in identifiers for compatibility.
** But the feature is undocumented.
*/
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


static int uqstrtok(cp, len)
	char *cp;
	size_t len;
{
	if(strncasecmp(cp, "NULL", len) == 0) {
		return NIL;
	} else if(strncasecmp(cp, "QUERY", len) == 0) {
		return QUERY;
	} else if(strncasecmp(cp, "CAT", len) == 0) {
		return CATALOGUE;
	} else if(strncasecmp(cp, "WHERE", len) == 0) {
		return COND;
	} else if(strncasecmp(cp, "COLS", len) == 0) {
		return COLS;
	} else if(strncmp(cp, "vfile", len) == 0) {
		return VFILE;
	} else {
		return UQSTRING;
	}
}

size_t toktl(cp, tval)
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
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			for(i = 1; isspace(*(cp + i)); i++) ;
			*tval = SPACE;
			l += i - 1;
			break;
		case '"':
		case '\'':
			tdel = *cp;
			for(i = 1; (c = *(cp + i)) != '\0'; i++) {
				if(c == tdel) {
					if(cp[i + 1] == tdel) {
						i++;
					} else
						break;
				}
			}

			if(c)
				i++;

			*tval = STRING;

			l += i - 1;
			break;
		case '(':
			*tval = OPAR;
			break;
		case ')':
			*tval = CPAR;
			break;
		case '=':
			if(nc == '=') {
				*tval = EQUALS;
				l++;
				break;
			} else if(nc == '~') {
				*tval = REGEQU;
				l++;
				break;
			} else {
				*tval = ASSIGN;
			}
		case '!':
			*tval = B_NOT;
			break;
		case '|':
			if(nc == '|') {
				*tval = B_OR;
				l++;
			} else {
				*tval = BW_OR;
			}
			break;
		case '&':
			if(nc == '&') {
				*tval = B_AND;
				l++;
				break;
			} else {
				*tval = BW_AND;
			}
			break;
		case '/':
			tdel = *cp;
			for(i = 1; (c = *(cp + i)) != '\0'; i++) {
				if(c == tdel) {
					if(cp[i + 1] == tdel) {
						i++;
					} else
						break;
				}
			}

			*tval = REGEXP;

			if(c)
				i++;
			l += i - 1;

			break;
		case ',':
			*tval = COMMA;
			break;
		case 'i':
			if(!isalnum(nc) && !IdChar(nc)) {
				*tval = CINSENS;
				break;
			}
			break;
		case '\0':
			*tval = -1;
			l = 0;
			break;
	}

	if(*tval != 0)
		return l;
	else if(isdigit(*cp) || (*cp == '-' && isdigit(nc))) {
		if(isdigit(nc)) {
			*tval = SIGNEDINT;
			i = 2;
		} else {
			i = 1;
			*tval = USINT;
		}
		for(; isdigit(*(cp + i)); i++) ;
		if(*(cp + i) == '.' && isdigit(*(cp + i + 1))) {
			i += 2;
			while(isdigit(*(cp + i)))
				i++;
			*tval = FLOAT;
		}

		l += i - 1;
	} else {
		if(IdChar(*cp)) {
			for(i = 1; IdChar(*(cp + i)); i++) ;
			*tval = uqstrtok(cp, i);
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

int extract_token_data(cp, t, len, toke)
	char *cp;
	int t;
	size_t len;
	Toke *toke;
{
	char ocp;

	switch(t) {
		case STRING:
		case REGEXP:
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
		case SIGNEDINT:
			ocp = *(cp + len);
			*(cp + len) = '\0';
			toke->num = (int)strtol(cp, NULL, 10);
			*(cp + len) = ocp;
			break;
		case USINT:
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
			



int qtok(cp, tval, toke, ctok)
	char **cp;
	int *tval;
	Toke *toke;
	char *ctok; /* at least 512 bytes */
{
	int len, rc;
	char *last = *cp;

	len = toktl(*cp, tval);

	strncpy(ctok, last, (len >= 511 ? 511 : len));

	if(*tval == -1 || *tval == ILLEGAL || *tval == 0)
		return 0;

	rc = extract_token_data(*cp, *tval, len, toke);

	*cp += len;


	return rc;
}
