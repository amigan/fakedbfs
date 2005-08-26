/*
 * Copyright (c) 2005, Dan Ponte
 *
 * query.c - query code
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
/* $Amigan: fakedbfs/libfakedbfs/query.c,v 1.2 2005/08/26 21:36:14 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>

/* other libraries */
#include <sqlite3.h>
/* us */
#include <query.h>
#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/query.c,v 1.2 2005/08/26 21:36:14 dcp1990 Exp $")

int init_stack(f, size)
	query_t *f;
	size_t size;
{
	f->stacksize = size;
	f->stackbase = malloc(sizeof(operands_t) * size);
	f->csp = f->stackbase;
	f->top = f->stackbase + ((sizeof(operands_t) * size) - sizeof(operands_t));
	return 1;
}

int destroy_stack(f)
	query_t *f;
{
	free(f->stackbase);
	f->csp = f->stackbase = f->top = NULL;
	f->stacksize = 0;
	return 1;
}

query_t* new_query(stacksize)
	size_t stacksize;
{
	query_t *n;
	n = allocz(sizeof(*n));
	init_stack(n, stacksize != 0 ? stacksize : DEFAULT_STACKSIZE);
	return n;
}

void destroy_query(q)
	query_t *q;
{
	destroy_stack(q);
	free(q);
}

int qi(q, opcode, op1, op2, op3, used)
	query_t *q;
