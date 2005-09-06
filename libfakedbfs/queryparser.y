/*
 * Copyright (c) 2005, Dan Ponte
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
/* $Amigan: fakedbfs/libfakedbfs/queryparser.y,v 1.1 2005/09/06 07:44:30 dcp1990 Exp $ */
%name {QParse}
%include {
#include <sqlite3.h>
#include <stdlib.h>
#include <fakedbfs.h>
#include <string.h>
#include <unistd.h>
RCSID("$Amigan: fakedbfs/libfakedbfs/queryparser.y,v 1.1 2005/09/06 07:44:30 dcp1990 Exp $")
extern char *yytext;
}
%token_type {Toke}
%extra_argument {query_t *q}
%syntax_error {
	fdbfs_t *in = (fdbfs_t *)q->f;
	ferr(in, die, "Query Syntax error near %s", yytext);
	return;
}

query ::= command.
command ::= . {
		q->f = NULL;
	}
