/*
 * Copyright (c) 2005, Dan Ponte
 *
 * ficl.c - ficl plugin bindings
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
/* $Amigan: fakedbfs/libfakedbfs/ficl.c,v 1.1 2005/12/23 19:57:03 dcp1990 Exp $ */
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

#include <fdbfsconfig.h>

#include <fakedbfs.h>
#define FICLBINDINGSVER "0.1"

#define FICLWORD(word)		cword = ficlDictionarySetPrimitive(dict, #word, ficl_word_ ## word, 0x0)
#define WORDDEF(word)		void ficl_word_ ## word(ficlVm *vm)

RCSID("$Amigan: fakedbfs/libfakedbfs/ficl.c,v 1.1 2005/12/23 19:57:03 dcp1990 Exp $")

static const struct PluginInfo ficl_inf = {
	"",
	"FORTH plugin",
	FICLBINDINGSVER,
	"Dan Ponte <dcp1990@neptune.atopia.net>",
	"http://www.theamigan.net/fakedbfs/",
	MAJOR_API_VERSION,
	MINOR_API_VERSION
};

int ficl_init(f)
	fdbfs_t *f;
{
	ficlSystem *fsys;
	ficlDictionary *sysdict;

	fsys = ficlSystemCreate(NULL);
	
	if(fsys == NULL) {
		return SERR(die, "Error creating ficl system!");
	}

	f->fsys = fsys;
	
	sysdict = ficlSystemGetDictionary(fsys);

	return ficl_addwords(f, sysdict);
}



/* newfield (s"fieldname" ... fields_t *new)
 * Takes a string for fieldname, pushes a pointer to a new fields_t object.
 */
WORDDEF(newfield)
{
	char *fname;
	unsigned int fnlen;
	fields_t *new;
	ficlStack *dst = vm->dataStack;

	fnlen = ficlStackPopUnsigned(dst);
	fname = ficlStackPopPointer(dst);

	new = allocz(sizeof(*new));
	new->fieldname = strdup(fname);

	ficlStackPushPointer(dst, new);
}

/* linkfield (fields_t* prev, fields_t *new ... fields_t *cur)
 * pops two pointers to fields_t, sets the next field of the first one to the second one, and returns the second one on the stack
 */
WORDDEF(linkfield)
{
	fields_t *prev, *next;
	ficlStack *dst = vm->dataStack;

	next = ficlStackPopPointer(dst);
	prev = ficlStackPopPointer(dst);

	prev->next = next;

	ficlStackPushPointer(dst, next);
}

/* fmtname ( fields_t *, s" fmtname" -- fields_t *)
 * sets the fmtname field to a string
 */
WORDDEF(fmtname)
{
	fields_t *tf;
	char *str;
	unsigned int len;
	ficlStack *dst = vm->dataStack;

	len = ficlStackPopUnsigned(dst);
	str = ficlStackPopPointer(dst);
	tf = ficlStackPopPointer(dst);

	tf->fmtname = strdup(str);

	ficlStackPushPointer(dst, tf);
}

int ficl_addwords(f, dict)
	fdbfs_t *f;
	ficlDictionary *dict;
{
	ficlWord *cword;

	FICLWORD(newfield);
	FICLWORD(linkfield);
	FICLWORD(fmtname);

	return 1;
}
