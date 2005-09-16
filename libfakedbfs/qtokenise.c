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
/* $Amigan: fakedbfs/libfakedbfs/qtokenise.c,v 1.1 2005/09/16 21:06:26 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <float.h>

/* other libraries */
#include <sqlite3.h>
/* us */
#include <query.h>
#include <fakedbfs.h>

#include "../fdbfsconfig.h"

#include "queryparser.h"


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


short int isvalchar(c)
	char c;
{
	switch(c)a{
		case '-':
		case '_':
			return 1;
		default:
			return 0;
	}

	return 0;
}


size_t qtok(cp, tok, tval)
	char *cp;
	Toke *tok;
	int *tval;
{
	char *last = *cp;
	char *find;
	int kg = 1;
	
	*tval = 0;

	switch(*cp) {
		case '"':
			while(1) {
				if((find = strchr(*:scp, '"')) != NULL) {
					if(*(find - 1) != '\\')
						break;
				} else {
					*tval = -1;
					return NULL;
				}
			}
			*find = '\0';
			*tval = STRING;
			tok->str = strdup(*cp);
			*find = '"';
			*cp = find;
			break;
		case '(':
			(*cp)++;
			*tval = OPAR;
			break;
		case ')':
			(*cp)++;
			*tval = CPAR;
			break;
		case '=':
			if(*(*cp + 1) == '=') {
				*cp += 2;
				*tval = EQUALS;
				break;
			} else if(*(*cp + 1) == '~') {
				*cp += 2;
				*tval = REGEQU;
				break;
			} else {
				*tval = -1;
				return NULL;
			}
		case '!':
			(*cp)++;
			*tval = B_NOT;
			break;
		case '|':
			if(*(*cp + 1) == '|') {
				*cp += 2;
				*tval = B_OR;
				break;
			} else {
				*tval = -1;
				return NULL;
			}
		case '&':
			if(*(*cp + 1) == '&') {
				*cp += 2;
				*tval = B_AND;
				break;
			} else {
				*tval = -1;
				return NULL;
			}
		case '/':
			(*cp)++;
			*tval = REGSTART;
			break;
		case '\\':
			(*cp)++;
			*tval = REGEND;
			break;
		case ',':
			(*cp)++;
			*tval = COMMA;
			break;
		case 'i':
			if(!isalnum(*(*cp + 1)) && !isvalchar(*(*cp + 1))) {
				*tval = CINSENS;
				(*cp)++;
				break;
			}
			break;
	}

	if(*tval != 0)
		return last;

	if(strncasecmp(cp, "NULL", 4) == 0) {
		*cp += 4;
		*tval = NIL;
	} else if(strncmp(cp, "vfile", 5) == 0) {
		*cp += 5;
		*tval = VFILE;
	} else if(isdigit(**cp)) {
		tok->unum = (unsigned int)strtoul(nptr, (char **)NULL, 10);
		*tval = UINT;
	} else if(**cp == '-' && isdigit(*(*(cp + 1)))) {
		tok->num = (int)strtol(nptr, (char **)NULL, 10);
		*tval = INT;
	}

	/* make recursive call */
