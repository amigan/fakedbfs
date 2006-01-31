/*
 * Copyright (c) 2005-2006, Dan Ponte
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
/* $Amigan: fakedbfs/libfakedbfs/ficl.c,v 1.6 2006/01/31 17:43:09 dcp1990 Exp $ */
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

#include <fakedbfs/fdbfsconfig.h>
#include <fakedbfs/fficl.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/debug.h>
#include <fakedbfs/plugins.h>
#include <ficl.h>
#define FICLBINDINGSVER "0.1"

#define FICLWORD(word)		cword = ficlDictionarySetPrimitive(dict, #word, fdbfs_ficl_word_ ## word, 0x0)
#define WORDDEF(word)		void fdbfs_ficl_word_ ## word(ficlVm *vm)

RCSID("$Amigan: fakedbfs/libfakedbfs/ficl.c,v 1.6 2006/01/31 17:43:09 dcp1990 Exp $")



const struct PluginInfo fdbfs_ficl_inf = {
	"",
	"FORTH plugin",
	FICLBINDINGSVER,
	"Dan Ponte <dcp1990@neptune.atopia.net>",
	"http://www.theamigan.net/fakedbfs/",
	MAJOR_API_VERSION,
	MINOR_API_VERSION
};

void fdbfs_ficl_output(cb, text) /* HACK! */
	ficlCallback *cb;
	char *text;
{
	ficlstate_t *fs = cb->context;

	if(fs->do_outp) {
		if(text != NULL)
			fputs(text, stdout);
		else
			fflush(stdout);
	}
}


void fdbfs_ficl_error(cb, text) /* HACK! */
	ficlCallback *cb;
	char *text;
{
	if(text != NULL)
		fputs(text, stderr);
	else
		fflush(stderr);
}

int fdbfs_ficl_init(f)
	fdbfs_t *f;
{
	ficlSystem *fsys;
	ficlDictionary *sysdict;
	ficlSystemInformation si;

	f->fst.f = f;
	f->fst.do_outp = 0; /* supress start messages */

	ficlSystemInformationInitialize(&si);
	si.textOut = fdbfs_ficl_output;
	si.errorOut = fdbfs_ficl_error;
	si.context = &f->fst;

	fsys = ficlSystemCreate(&si);

	f->fst.do_outp = 1;
	
	if(fsys == NULL) {
		return SERR(die, "Error creating ficl system!");
	}

	f->fsys = fsys;
	
	sysdict = ficlSystemGetDictionary(fsys);

	return fdbfs_ficl_addwords(f, sysdict);
}

void fdbfs_ficl_destroy(f)
	fdbfs_t *f;
{
	if(f->fsys != NULL)
		ficlSystemDestroy(f->fsys);
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

int fdbfs_ficl_addwords(f, dict)
	fdbfs_t *f;
	ficlDictionary *dict;
{
	ficlWord *cword;

	FICLWORD(newfield);
	FICLWORD(linkfield);
	FICLWORD(fmtname);

	return 1;
}

static char* word_prefix(fn)
	char *fn;
{
	char *pdelim, *ewd, *ns;
	size_t ln;

	pdelim = strrchr(fn, '/');
	if(!pdelim)
		return NULL;

	++pdelim;
	
	ewd = strrchr(fn, '.');
	if(!ewd)
		return NULL;

	*ewd = '\0';
	ln = strlen(pdelim) + 1;
	ns = malloc(ln + 1);
	strlcpy(ns, pdelim, ln);
	strlcat(ns, "_", ln);
	*ewd = '.';

	return ns;
}

static int parse_ficl_file(f, fp)
	fdbfs_t *f;
	ficlplug_t *fp;
{
#define BUFSIZE	1024 /* 1k */
	FILE *fh;
	char *buffer; /* use the heap, Luke */
	int rc;

	if(!(fh = fopen(fp->filename, "r"))) {
		return ERR(die, "Couldn't open %s: %s", fp->filename, strerror(errno));
	}

	buffer = malloc(BUFSIZE * sizeof(char));

	while(!feof(fh)) {
		fgets(buffer, BUFSIZE, fh);
		rc = ficlVmEvaluate(fp->vm, buffer);
		if(rc != FICL_VM_STATUS_OUT_OF_TEXT) {
			ERR(die, "ficlVmEvaulate returned %d", rc);
			free(buffer);
			fclose(fh);
			return 0;
		}
	}
	
	free(buffer);
	fclose(fh);
#undef BUFSIZE

	return 1;
}


struct Plugin* fdbfs_ficl_load_ficlplugin(f, plugfile)
	fdbfs_t *f;
	char *plugfile;
{
	int rc;
	char *plending;
	struct Plugin *n;

	plending = plugfile + (strlen(plugfile) - 3);
	if(strcasecmp(plending, ".4th") != 0) {
		fdbfs_debug_info(f, error, "Plugin is not a forth plugin.\n");
		return NULL;
	}

	n = allocz(sizeof(*n));

	n->info = &fdbfs_ficl_inf;
	n->flags |= PLUGIN_IS_FICL;
	n->pl.fplug.vm = ficlSystemCreateVm(f->fsys);
	if(n->pl.fplug.vm == NULL) {
		free(n);
		return NULL;
	}
	n->pl.fplug.filename = strdup(plugfile);
	n->pl.fplug.wordprefix = word_prefix(n->pl.fplug.filename);
	if(n->pl.fplug.wordprefix == NULL) {
		free(n->pl.fplug.filename);
		ficlVmDestroy(n->pl.fplug.vm);
		free(n);
		ERR(die, "Couldn't derive wordprefix from file %s", plugfile);
		return NULL;
	}
	rc = parse_ficl_file(f, &n->pl.fplug);
	if(!rc) {
		free(n->pl.fplug.filename);
		ficlVmDestroy(n->pl.fplug.vm);
		free(n);
		SCERR(die, "Error reading plugin ");
		return NULL;
	}

	return n;
}
