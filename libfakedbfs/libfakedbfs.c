/*
 * Copyright (c) 2005-2006, Dan Ponte
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
/**
 * @file libfakedbfs.c
 * @brief Main libfakedbfs functions.
 */
/* $Amigan: fakedbfs/libfakedbfs/libfakedbfs.c,v 1.22 2006/02/24 08:01:02 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
/* us */
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/db.h>
#include <fakedbfs/debug.h>
#include <fakedbfs/plugins.h>
#include <fakedbfs/conf.h>
#include <fakedbfs/fficl.h>


#ifndef lint
RCSID("$Amigan: fakedbfs/libfakedbfs/libfakedbfs.c,v 1.22 2006/02/24 08:01:02 dcp1990 Exp $")
/** @brief fakedbfs version. */
const char *fakedbfsver _unused = FAKEDBFSVER;
/** @brief fakedbfs version name */
const char *fakedbfsvname _unused = VERNAME;
/** @brief fakedbfs copyright notice. */
const char *fakedbfscopyright _unused = "libfakedbfs (C)2005, Dan Ponte. Under the BSD license.";
/** @brief Major version */
const int  fakedbfsmaj _unused = FAKEDBFSMAJOR;
/** @brief Minor version */
const int  fakedbfsmin _unused = FAKEDBFSMINOR;
/** @brief Micro version */
const int  fakedbfsmic _unused = FAKEDBFSMICRO;
#endif


fdbfs_t *fdbfs_new(dbfile, error, debugf, useplugins)
	char *dbfile;
	char **error; /* if we return NULL, this must be freed */
	void (*debugf)(char*, enum ErrorAction);
	int useplugins; /* this also controls the configuration subsystem; if false, then we don't read config stuff (useful for servers and stuff where config stuff isn't
			   completely necessary) */
{
	fdbfs_t *f;
	int rc;
	f = malloc(sizeof(*f));
	memset(f, 0, sizeof(*f));
	f->dbname = strdup(dbfile);
	f->debugfunc = debugf;
	rc = fdbfs_db_open(f);
	if(!rc) {
		*error = strdup(f->error.emsg);
		fdbfs_estr_free(&f->error);
		free(f);
		return NULL;
	}
	if(useplugins) {
		if(getenv(FDBFSPLUGENV) != NULL)
			fdbfs_plugins_set_path(f, getenv(FDBFSPLUGENV));
		if(!fdbfs_conf_init(f)) {
			*error = strdup(f->error.emsg);
			fdbfs_estr_free(&f->error);
			free(f);
			return NULL;
		}
		fdbfs_plugins_init(f);
		if(!fdbfs_ficl_init(f)) {
			*error = strdup("Couldn't initialise ficl system");
			fdbfs_estr_free(&f->error);
			free(f);
			return NULL;
		}
	}

	fdbfs_set_aff(f, fdbfs_askfunc_std);

	return f;
}

int fdbfs_destroy(f)
	fdbfs_t *f;
{
	if(!fdbfs_db_close(f)) {
		fdbfs_estr_free(&f->error);
		free(f);
		return 0;
	}
	free(f->dbname);
	fdbfs_free_plugin_list(f->plugins);
	if(f->heads.db_enumh != NULL)
		fdbfs_free_enum_head_list(f->heads.db_enumh);
	if(f->heads.db_cath != NULL)
		fdbfs_free_cat_head_list(f->heads.db_cath);

	fdbfs_conf_destroy_tree(f->rconf);
	fdbfs_ficl_destroy(f);

	free(f);
	return 1;
}

void fdbfs_set_aff(f, aff)
	fdbfs_t *f;
	answer_t *(*aff)AFFPROTO;
{
	f->askfieldfunc = aff;
}

int fdbfs_read_specs_from_db(f)
	fdbfs_t *f;
{
	f->heads.db_enumh = fdbfs_enums_from_db(f);
	if(f->heads.db_enumh == NULL && f->error.emsg != NULL)
		return 0;
	f->heads.db_cath = fdbfs_cats_from_db(f, f->heads.db_enumh);
	if(f->heads.db_cath == NULL && f->error.emsg != NULL)
		return 0;
	return 1;
}
