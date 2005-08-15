/*
 * Copyright (c) 2005, Dan Ponte
 *
 * indexing.c - indexing code
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
/* $Amigan: fakedbfs/libfakedbfs/indexing.c,v 1.3 2005/08/15 20:28:03 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/indexing.c,v 1.3 2005/08/15 20:28:03 dcp1990 Exp $")

int index_file(f, file, catalogue, fields)
	fdbfs_t *f;
	char *file;
	char *catalogue;
	fields_t *fields;
{
	fields_t *c;
	int fieldcount = 1;
	char *sqlkeys, *sqlvals;
	char *tablename, *fieldtable;
	size_t sqlkeyslen = 1, tablen = 1, fieldlen = 1, sqlvalslen = 1;
	/* sqlite stuff */
	int rc;
	char *sql;
	const char *tail;
	sqlite3_stmt *stmt;
	
	tablen += strlen(CAT_TABLE_PREFIX);
	tablen += strlen(catalogue);
	tablename = malloc(tablen);
	strlcpy(tablename, CAT_TABLE_PREFIX, tablen);
	strlcat(tablename, catalogue, tablen);

	fieldlen += strlen(CAT_FIELD_TABLE_PREFIX);
	fieldlen += strlen(catalogue);
	fieldtable = malloc(fieldlen);
	strlcpy(fieldtable, CAT_FIELD_TABLE_PREFIX, fieldlen);
	strlcat(fieldtable, catalogue, fieldlen);


	for(c = fields; c != NULL; c = c->next) {
		sqlkeyslen += strlen(c->fieldname) + 1 /* comma */;
		sqlvalslen += 2; /* ",?" */
	}
	
	sqlkeys = malloc(sqlkeyslen);
	sqlvals = malloc(sqlvalslen);

	*sqlkeys = '\0';
	*sqlvals = '\0';

	for(c = fields; c != NULL; c = c->next) {
		strlcat(sqlkeys, ",", sqlkeyslen);
		strlcat(sqlkeys, c->fieldname, sqlkeyslen);

		strlcat(sqlvals, ",?", sqlvalslen);
	}

	/* begin SQLite specific stuff */
	
	sql = sqlite3_mprintf("INSERT OR REPLACE INTO %s (file, lastupdate %s) VALUES('%q', %d %s);",
			tablename, sqlkeys, file, time(NULL), sqlvals);
	rc = sqlite3_prepare(f->db, sql, strlen(sql), &stmt, &tail);

	sqlite3_free(sql);
	free(sqlkeys);
	free(sqlvals);
	free(tablename);
	free(fieldtable);

	if(rc != SQLITE_OK) {
		return ERR(die, "index_file(\"%s\"): SQLite error after prepare: %s", file, sqlite3_errmsg(f->db));
	}

	for(c = fields; c != NULL; c = c->next) {
		if(!bind_field(f, &fieldcount, c->type, c->val, c->len, stmt))
			return CERR(die, "index_file(\"%s\"): bind error. ", file);
	}

	rc = sqlite3_step(stmt);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		return ERR(die, "index_file(\"%s\"): SQLite error after step: %s", file, sqlite3_errmsg(f->db));
	}

	sqlite3_finalize(stmt);
	return 1;
}

fields_t* fill_in_fields(f, filename)
	fdbfs_t *f;
	char *filename;
{
	struct Plugin *h = f->plugins, *c = NULL;
	struct fields_t *fh = NULL, *fc = NULL, *fn = NULL;
	char *errmsg;

	/* do stuff */
	return fh;
}
