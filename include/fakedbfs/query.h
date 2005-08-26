/*
 * Copyright (c) 2005, Dan Ponte
 *
 * query.h - query opcodes and such
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
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
/* $Amigan: fakedbfs/include/fakedbfs/query.h,v 1.1 2005/08/26 21:36:14 dcp1990 Exp $ */
#ifndef HAVE_QUERY_H
#define HAVE_QUERY_H 1

/* See doc/QUERY_OPCODES for how these are used */
#define OP_BEGINQ	0x1
#define OP_SETCAT	0x2
#define OP_BEGIN_GRP	0x3
#define OP_CLOSE_GRP	0x4
#define OPL_AND		0x5
#define OPL_OR		0x6
#define OPL_NOT		0x7
#define OPL_EQUAL	0x8
#define OP_COLNAME	0x9
#define OP_STRING	0xA
#define OP_INT		0xB
#define OP_UINT		0xC
#define OP_FLOAT	0xE
#define OP_PUSH		0xF
#define OP_POP		0x10
#define OP_ENDQ		0x11

#define USED_O1		0x1
#define USED_O2		0x2
#define USED_O3		0x3

#define DEFAULT_STACKSIZE 20

typedef struct {
	int o1;
	unsigned int o2;
	void *o3;
	int used;
} operands_t;

typedef struct Inst {
	int opcode;
	operands_t ops;
	struct Inst *next;
} inst_t;

typedef struct {
	operands_t *stackbase, *top, *csp;
	size_t stacksize; /* int number of elements, not bytes */
	inst_t *insthead;
} query_t;
#endif
