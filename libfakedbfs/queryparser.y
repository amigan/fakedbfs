/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * queryparser.y - Parser for the query language. Turns it into a compiled query_t
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
/* $Amigan: fakedbfs/libfakedbfs/queryparser.y,v 1.9 2006/03/26 01:22:24 dcp1990 Exp $ */
%name {QParse}
%include {
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/query.h>
#include <fakedbfs/debug.h>
RCSID("$Amigan: fakedbfs/libfakedbfs/queryparser.y,v 1.9 2006/03/26 01:22:24 dcp1990 Exp $")
}
%token_type {Toke}
%nonassoc ASSIGN BW_OR BW_AND ILLEGAL SPACE.
%left B_OR.
%left B_AND.
%extra_argument {query_t *q}
%syntax_error {
	fdbfs_t *in = (fdbfs_t *)q->f;
	fdbfs_ferr(in, die, "Query Syntax error near '%s'", q->yytext);
	q->error++;
	return;
}

statem ::= command.
command ::= query.

query ::= qry CATALOGUE catname COND cexp EOQUERY. {
		fdbfs_query_qi(q, OP_ENDQ, 0x0, 0x0, NULL, 0);
	}

qry ::= bqry.
qry ::= sqry COLS OPAR coldefs CPAR. 

coldefs ::= coldefs COMMA coldef.
coldefs ::= coldef.

coldef ::= UQSTRING(A). {
		fdbfs_query_qi(q, OP_SELCN, 0x0, 0x0, A.str, USED_O3 | US_DYNA);
	}
sqry ::= QUERY. {
		fdbfs_query_qi(q, OP_BEGINQ, 0x1, 0x0, NULL, USED_O1);
	}
bqry ::= QUERY. {
		fdbfs_query_qi(q, OP_BEGINQ, 0x0, 0x0, NULL, USED_O1);
	}

catname ::= UQSTRING(A). {
		fdbfs_query_qi(q, OP_SETCAT, 0x0, 0x0, A.str, USED_O3 | US_DYNA);
	}

cexp ::= en.

en ::= en op val.
en ::= regex.
en ::= val.
en ::= oparen en cparen.
en ::= .


op ::= andop.
op ::= orop.
op ::= notop.
op ::= equop.
op ::= nequop.
op ::= gtop.
op ::= ltop.
op ::= gteop.
op ::= lteop.

val ::= colname.
val ::= null.
val ::= integer.
val ::= eqstring.
val ::= voidfile.
val ::= floatp.

gteop ::= GTEQU. {
		fdbfs_query_qi(q, OPL_GTHEQU, 0x0, 0x0, NULL, 0x0);
	}
lteop ::= LTEQU. {
		fdbfs_query_qi(q, OPL_LTHEQU, 0x0, 0x0, NULL, 0x0);
	}
ltop ::= LT. {
		fdbfs_query_qi(q, OPL_LTHAN, 0x0, 0x0, NULL, 0x0);
	}
gtop ::= GT. {
		fdbfs_query_qi(q, OPL_GTHAN, 0x0, 0x0, NULL, 0x0);
	}
nequop ::= NEQU. {
		fdbfs_query_qi(q, OPL_NEQU, 0x0, 0x0, NULL, 0x0);
	}
oparen ::= OPAR. {
		fdbfs_query_qi(q, OP_BEGIN_GRP, 0x0, 0x0, NULL, 0x0);
	}
cparen ::= CPAR. {
		fdbfs_query_qi(q, OP_CLOSE_GRP, 0x0, 0x0, NULL, 0x0);
	}

colname ::= UQSTRING(A). {
		fdbfs_query_qi(q, OP_COLNAME, 0x0, 0x0, A.str, USED_O3 | US_DYNA);
	}

andop ::= B_AND. {
		fdbfs_query_qi(q, OPL_AND, 0x0, 0x0, NULL, 0x0);
	}
orop ::= B_OR. {
		fdbfs_query_qi(q, OPL_OR, 0x0, 0x0, NULL, 0x0);
	}
notop ::= B_NOT. {
		fdbfs_query_qi(q, OPL_NOT, 0x0, 0x0, NULL, 0x0);
	}
equop ::= EQUALS. {
		fdbfs_query_qi(q, OPL_EQUAL, 0x0, 0x0, NULL, 0x0);
	}
null ::= NIL. {
		fdbfs_query_qi(q, OP_NULL, 0x0, 0x0, NULL, 0x0);
	}
integer ::= SIGNEDINT(A). {
		fdbfs_query_qi(q, OP_INT, A.num, 0x0, NULL, USED_O1);
	}
integer ::= USINT(A). {
		fdbfs_query_qi(q, OP_UINT, 0x0, A.unum, NULL, USED_O2);
	}
eqstring ::= STRING(A). {
		fdbfs_query_qi(q, OP_STRING, 0x0, 0x0, A.str, USED_O3 | US_DYNA);
	}
voidfile ::= VFILE OPAR STRING(A) CPAR. {
		void *tfp;
		tfp = fdbfs_query_read_file(q->f, A.str);
		free(A.str);
		if(tfp == NULL) {
			fdbfs_cferr(q->f, die, "Error reading file. ");
			return;
		}
		fdbfs_query_qi(q, OP_VOID, 0x0, 0x0, tfp, USED_O3 | US_FILE);
	}
floatp ::= FLOAT(A). {
		fdbfs_query_qi(q, OP_FLOAT, 0x0, 0x0, A.flt, USED_O3 | US_DYNA);
	}

regex ::= colname REGEQU REGEXP(A) flags(B). {
		qreg_t *qr;
		char *ems;

		if((qr = fdbfs_qreg_compile(A.str, B.num, &ems)) == NULL) {
			fdbfs_ferr(q->f, die, "Error compiling regex %s: %s", A.str, ems);
			free(ems);
			free(A.str);
			return;
		}

		fdbfs_query_qi(q, OP_REGEXP, 0x0, 0x0, qr, USED_O3);
	}
regex ::= colname REGNEQU REGEXP(A) flags(B). {
		qreg_t *qr;
		char *ems;

		if((qr = fdbfs_qreg_compile(A.str, B.num, &ems)) == NULL) {
			fdbfs_ferr(q->f, die, "Error compiling regex %s: %s", A.str, ems);
			free(ems);
			free(A.str);
			return;
		}

		fdbfs_query_qi(q, OP_NOTREGEXP, 0x0, 0x0, qr, USED_O3);
	}

flags(A) ::= CINSENS. {
		A.num = 1;
	}
flags(A) ::= . {
		A.num = 0;
	}
