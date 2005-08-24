/*
 * Copyright (c) 2005, Dan Ponte
 *
 * dbinit.c - DB initialisation
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
/* $Amigan: fakedbfs/libfakedbfs/libfakedbfs.c,v 1.12 2005/08/24 04:59:42 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
/* us */
#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/libfakedbfs.c,v 1.12 2005/08/24 04:59:42 dcp1990 Exp $")

#ifndef lint
const char const *fakedbfsver _unused = FAKEDBFSVER;
const char const *fakedbfscopyright _unused = "libfakedbfs (C)2005, Dan Ponte. Under the BSD license.";
#endif

fdbfs_t *new_fdbfs(dbfile, error, debugf, useplugins)
	char *dbfile;
	char **error; /* if we return NULL, this must be freed */
	void (*debugf)(char*, enum ErrorAction);
	int useplugins;
{
	fdbfs_t *f;
	int rc;
	f = malloc(sizeof(*f));
	memset(f, 0, sizeof(*f));
	f->dbname = dbfile;
	f->debugfunc = debugf;
	rc = open_db(f);
	if(!rc) {
		*error = strdup(f->error.emsg);
		estr_free(&f->error);
		free(f);
		return NULL;
	}
	if(useplugins) {
		if(getenv(FDBFSPLUGENV) != NULL)
			set_plug_path(f, getenv(FDBFSPLUGENV));
		init_plugins(f);
	}

	set_aff(f, askfunc_std);

	return f;
}

int destroy_fdbfs(f)
	fdbfs_t *f;
{
	if(!close_db(f)) {
		estr_free(&f->error);
		free(f);
		return 0;
	}
	destroy_plugin_list(f->plugins);
	if(f->heads.db_enumh != NULL)
		free_enum_head_list(f->heads.db_enumh);
	if(f->heads.db_cath != NULL)
		free_cat_head_list(f->heads.db_cath);
	free(f);
	return 1;
}

void set_aff(f, aff)
	fdbfs_t *f;
	answer_t *(*aff)AFFPROTO;
{
	f->askfieldfunc = aff;
}

void set_plug_path(f, path)
	fdbfs_t *f;
	char *path;
{
	f->conf.pluginpath = strdup(path);
}

int read_specs_from_db(f)
	fdbfs_t *f;
{
	f->heads.db_enumh = enums_from_db(f);
	if(f->heads.db_enumh == NULL && f->error.emsg != NULL)
		return 0;
	f->heads.db_cath = cats_from_db(f, f->heads.db_enumh);
	if(f->heads.db_cath == NULL && f->error.emsg != NULL)
		return 0;
	return 1;
}
