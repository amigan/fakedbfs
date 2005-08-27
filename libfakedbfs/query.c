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
/* $Amigan: fakedbfs/libfakedbfs/query.c,v 1.3 2005/08/27 02:39:31 dcp1990 Exp $ */
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

RCSID("$Amigan: fakedbfs/libfakedbfs/query.c,v 1.3 2005/08/27 02:39:31 dcp1990 Exp $")

int init_stack(f, size)
	query_t *f;
	size_t size;
{
	f->stacksize = size;
	f->items = 0;
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
	f->items = 0;
	return 1;
}

int spush(q, o1, o2, o3, used)
	query_t *q;
	int o1;
	unsigned int o2;
	void *o3;
	int used;
{
	if(q->items >= q->stacksize)
		return 0;
	q->csp += sizeof(*q->csp);
	q->items++;
	q->csp->o1 = o1;
	q->csp->o2 = o2;
	q->csp->o3 = o3;
	q->csp->used = used;
	return 1;
}

int spop(q, bf)
	query_t *q;
	operands_t *bf;
{
	if(q->items <= 0)
		return 0;
	bf->o1 = q->csp->o1;
	bf->o2 = q->csp->o2;
	bf->o3 = q->csp->o3;
	bf->used = q->csp->used;
	q->csp -= sizeof(*q->csp);
	q->items--;
	return 1;
}

int pop1(q, o1)
	query_t *q;
	int *o1;
{
	operands_t c;
	int rc;

	rc = spop(q, &c);
	if(!rc)
		return rc;

	*o1 = c.o1;
	return rc;
}

int pop2(q, o2)
	query_t *q;
	unsigned int *o2;
{
	operands_t c;
	int rc;

	rc = spop(q, &c);
	if(!rc)
		return rc;

	*o2 = c.o2;
	return rc;
}

int pop3(q, o3)
	query_t *q;
	void **o3;
{
	operands_t c;
	int rc;

	rc = spop(q, &c);
	if(!rc)
		return rc;

	*o3 = c.o3;
	return rc;
}

int push1(q, o1)
	query_t *q;
	int o1;
{
	return spush(q, o1, 0x0, NULL, USED_O1);
}

int push2(q, o2)
	query_t *q;
	unsigned int o2;
{
	return spush(q, 0x0, o2, NULL, USED_O2);
}

int push3(q, o3)
	query_t *q;
	void *o3;
{
	return spush(q, 0x0, 0x0, o3, USED_O3);
}

query_t* new_query(f, stacksize)
	fdbfs_t *f;
	size_t stacksize;
{
	query_t *n;
	n = allocz(sizeof(*n));
	n->f = f;
	init_stack(n, stacksize != 0 ? stacksize : DEFAULT_STACKSIZE);
	return n;
}

void destroy_query(q)
	query_t *q;
{
	inst_t *c, *next;

	destroy_stack(q);
	
	for(c = q->insthead; c != NULL; c = next) {
		next = c->next;
		free(c);
	}

	free(q);
}

int qi(q, opcode, op1, op2, op3, used)
	query_t *q;
	int opcode;
	int op1;
	unsigned int op2;
	void *op3;
	int used;
{
	inst_t *new = NULL;

	new = allocz(sizeof(*new));
	new->opcode = opcode;
	new->ops.o1 = op1;
	new->ops.o2 = op2;
	new->ops.o3 = op3;
	new->ops.used = used;
	
	if(q->insthead == NULL) {
		q->insthead = new;
		q->lastinst = new;
	} else {
		q->lastinst->next = new;
		q->lastinst = new;
	}

	return 1;
}

int qne(q, fld) /* Query Next/Execute */
	query_t *q;
	fields_t *fld;
{
	return 1;
}
