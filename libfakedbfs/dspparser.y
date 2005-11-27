/*
 * Copyright (c) 2005, Dan Ponte
 *
 * dspparser.y - parser for default specs
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
/* $Amigan: fakedbfs/libfakedbfs/dspparser.y,v 1.4 2005/11/27 03:58:43 dcp1990 Exp $ */
%name {DSPParse}
%include {
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif
#include <fakedbfs.h>
RCSID("$Amigan: fakedbfs/libfakedbfs/dspparser.y,v 1.4 2005/11/27 03:58:43 dcp1990 Exp $")
}
%token_type {Toke}
%nonassoc ILLEGAL SPACE.
%nonassoc ENUM ENSUB COMMA UINT.
%extra_argument {dspdata_t *d}
%syntax_error {
	ferr(d->f, die, "Defspec Syntax error near '%s'", d->yytext);
	d->error++;
	return;
}

defspec ::= assignments.
assignments ::= assignments assignment SEMICOLON.
assignments ::= .
assignment ::= lvalue ASSIGN rvalue.

lvalue ::= UQSTRING(B). {
	struct CatElem *ce;
	if((ce = find_catelem_by_name(d->cat->headelem, B.str)) == NULL) {
		ferr(d->f, die, "No such element %s in catalogue", B.str);
		d->error = 1;
		free(B.str); /* this causes problems. */
		break;
	}
	d->cf = allocz(sizeof(fields_t));
	d->cf->fieldname = B.str;
	d->cf->fmtname = strdup(ce->alias);
	d->cf->type = ce->type;
	if(d->cf->type == oenum) {
		d->cf->ehead = ce->enumptr;
	}
	if(d->cf->type == oenum && ce->enumptr->otherelem != NULL) {
		d->cf->othtype = ce->enumptr->otherelem->othertype;
	} else if(d->cf->type == oenumsub) {
		d->cf->ehead = ce->subcatel->enumptr;
		d->cf->subparent = find_field_by_ehead(d->fhead, d->cf->ehead);
		/* handle subelements */
	}
	if(d->lastf != NULL) {
		d->lastf->next = d->cf;
		d->lastf = d->cf;
	} else {
		d->lastf = d->cf;
		d->fhead = d->lastf;
	}
}

rvalue ::= SINT(A). {
	if(d->cf->type != number && d->cf->type != datime /* for now */) {
		ferr(d->f, die, "Type of field '%s' is not number!", d->cf->fieldname);
		break;
	}
	d->cf->val = malloc(sizeof(int));
	*(int *)d->cf->val = A.num;
}

rvalue ::= STRING(A). {
	if(d->cf->type != string) {
		ferr(d->f, die, "Type of field '%s' is not string!", d->cf->fieldname);
		free(A.str);
		break;
	}
	d->cf->val = A.str;
}

rvalue ::= FLOAT(A). {
	if(d->cf->type != real) {
		ferr(d->f, die, "Type of field '%s' is not float!", d->cf->fieldname);
		free(A.flt);
		break;
	}
	d->cf->val = A.flt;
}

rvalue ::= boolean(A). {
	if(d->cf->type != boolean && d->cf->type != number) {
		ferr(d->f, die, "Type of field '%s' is not boolean or number!", d->cf->fieldname);
		break;
	}
	d->cf->val = malloc(sizeof(int));
	*(int *)d->cf->val = A.num;
}

rvalue ::= bintype OPAREN STRING(A) CPAREN. {
	char *fn = A.str;
#ifdef HAVE_MMAP
	int fd, rc;
	struct stat sst;
#endif
	if(d->cf->type != binary && d->cf->type != image) {
		ferr(d->f, die, "Type of field '%s' is not binary or image!", d->cf->fieldname);
		free(A.str);
		break;
	}
#ifdef HAVE_MMAP
	fd = open(fn, O_RDONLY);
	if(fd < 0) {
		ferr(d->f, die, "Error opening %s: %s", fn, strerror(errno));
		free(fn);
		break;
	}

	if((rc = fstat(fd, &sst)) == -1) {
		ferr(d->f, die, "Couldn't stat fd: %s", strerror(errno));
		free(fn);
		close(fd);
		break;
	}

	if((d->cf->val = mmap(NULL, sst.st_size, PROT_READ, 0, fd, 0)) == MAP_FAILED) {
		ferr(d->f, die, "Couldn't mmap: %s", strerror(errno));
		d->cf->val = NULL;
		free(fn);
		close(fd);
		break;
	}
	d->cf->len = sst.st_size;
	d->cf->flags |= FIELDS_FLAG_MMAPED;

	close(fd);
	free(fn);
#else
	ferr(d->f, die, "Error opening %s: feature not implemented on this OS", fn);
	free(A.str);
	break;
#endif
}


bintype ::= BINFILE.
bintype ::= IMAGE.

boolean(A) ::= B_TRUE. {
	A.num = 1;
}

boolean(A) ::= B_FALSE. {
	A.num = 0;
}
