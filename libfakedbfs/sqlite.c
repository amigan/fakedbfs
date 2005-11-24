/*
 * Copyright (c) 2005, Dan Ponte
 *
 * sqlite.c - SQLite bindings
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
/* $Amigan: fakedbfs/libfakedbfs/sqlite.c,v 1.19 2005/11/24 02:41:56 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/sqlite.c,v 1.19 2005/11/24 02:41:56 dcp1990 Exp $")


int open_db(f)
	fdbfs_t *f;
{
	int rc;
	rc = sqlite3_open(f->dbname, &f->db);
	if(rc) {
		return ERR(die, "open_db: error opening database: %s", sqlite3_errmsg(f->db));
	}

	rc = sqlite3_create_function(f->db, "regexp", 2, SQLITE_ANY /* not exactly true... */, (void*)f, regex_func, NULL, NULL);
	if(rc != SQLITE_OK) {
		return ERR(die, "open_db: error creating regexp function: %s", sqlite3_errmsg(f->db));
	}
	return 1;
}

int close_db(f)
	fdbfs_t *f;
{
	int rc;
	if((rc = sqlite3_close(f->db)) != SQLITE_OK) {
		return ERR(die, "close_db: error closing database: %s", sqlite3_errmsg(f->db));
	}
	return 1;
}

int table_exists(f, tname)
	fdbfs_t *f;
	char *tname;
{
	const char *tail;
	const unsigned char *result;
	sqlite3_stmt *cst;
	const char* const sql = "SELECT name FROM sqlite_master WHERE type='table'"; /* check for the table in the master table */
	int rc;
	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, &tail)) != SQLITE_OK) {
		ERR(die, "table_exists(f, \"%s\"): SQLite error after prepare %s", tname, sqlite3_errmsg(f->db));
		return 0;
	}
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		result = sqlite3_column_text(cst, 0);
		if(strcmp((const char *)result, tname) == 0) {
			sqlite3_finalize(cst);
			return 1;
		}
	}
	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		ERR(die, "table_exists(f, \"%s\"): SQLite error after step: %s", tname, sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}
	sqlite3_finalize(cst);
	return 0;
}

int cat_exists(f, cat)
	fdbfs_t *f;
	char *cat;
{
	const char *tail;
	sqlite3_stmt *cst;
	char* sql;
	int num = 0;
	int rc;

	sql = sqlite3_mprintf("SELECT name FROM cat_list WHERE name='%q'", cat);
	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, &tail)) != SQLITE_OK) {
		ERR(die, "cat_exists(f, \"%s\"): SQLite error after prepare %s", cat, sqlite3_errmsg(f->db));
		sqlite3_free(sql);
		return -1;
	}
	sqlite3_free(sql);
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		num = 1;
		break;
	}

	if(rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) {
		ERR(die, "cat_exists(f, \"%s\"): SQLite error after step: %s", cat, sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return -1;
	}
	sqlite3_finalize(cst);
	return num;
}


int create_table(f, tname, tspec)
	fdbfs_t *f;
	char *tname;
	char *tspec;
{
	char *sql;
	char *emsg;
	int rc;
	sql = sqlite3_mprintf("CREATE TABLE %s (%s)", tname, tspec);
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "create_table(f, \"%s\", \"%s\"): SQLite error after exec: %s", tname, tspec, emsg);
		return 0;
	}
	return 1;
}


const char* gettype(t)
	enum DataType t;
{
	/* make sure you know what you're doing; we have defaults here */
	switch(t) {
		case number:
		case usnumber:
		case boolean:
		case oenum:
		case oenumsub:
			return " INTEGER DEFAULT 0";
			break;
		case string:
			return " TEXT DEFAULT NULL";
			break;
		case fp:
			return " REAL DEFAULT 0";
			break;
		case image:
		case binary:
			return " BLOB DEFAULT NULL";
			break;
	}
	return " BLOB";
}

const char* getcoltype(t)
	coltype_t t;
{
	switch(t) {
		case integer:
			return "INTEGER";
		case real:
			return "REAL";
		case text:
			return "TEXT";
		case blob:
		default:
			return "BLOB";
	}
	return "BLOB";
}

int add_column(f, tname, cname, datatype)
	fdbfs_t *f;
	char *tname;
	char *cname;
	coltype_t datatype;
{
	char *sql;
	char *emsg;
	int rc;
	sql = sqlite3_mprintf("ALTER TABLE %s ADD COLUMN %s %s", tname, cname, getcoltype(datatype));
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "add_column(f, \"%s\", \"%s\", %d): SQLite error after exec: %s", tname, cname, (int)datatype, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

int del_column(f, tname, cname)
	fdbfs_t *f;
	char *tname;
	char *cname;
{
	/*
	 * Do nothing; for other database drivers, this function might actually accomplish something
	 */
	return 1;
}

int add_to_enum_list_table(f, name, tname, specf)
	fdbfs_t *f;
	char *name;
	char *tname;
	char *specf;
{
	char *sql;
	char *emsg;
	int rc;
	sql = sqlite3_mprintf("INSERT OR REPLACE INTO enum_list (name, defined_in_table, "
			"defined_in_specfile, lastupdated) VALUES('%q', '%q', '%q', %d);",
			name, tname, specf, time(NULL));
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "add_to_enum_list_table(f, \"%s\", \"%s\", \"%s\"): SQLite error after exec: %s", name, tname, specf, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

int add_to_cat_list_table(f, name, alias, tablename, fieldtable, specf)
	fdbfs_t *f;
	char *name;
	char *alias;
	char *tablename;
	char *fieldtable;
	char *specf;
{
	char *sql, *emsg;
	int rc;
	sql = sqlite3_mprintf("INSERT OR REPLACE INTO cat_list (name, alias, defined_in"
			"_table, field_desc_table, defined_in_specfile, lastupdated) "
			"VALUES('%q', '%q', '%q', '%q', '%q', %d);",
			name, alias, tablename, fieldtable, specf, time(NULL));
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "add_to_cat_list_table(f, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"): SQLite error after exec: %s", name, alias, tablename, fieldtable, specf, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

int add_to_field_desc(f, tablename, name, alias, type, typen)
	fdbfs_t *f;
	char *tablename;
	char *name;
	char *alias;
	enum DataType type;
	char *typen;
{
	char *sql, *emsg, *othna;
	int rc;
	if(type == oenum) {
		size_t len = strlen(name) + strlen(OTHER_ELEM_PREFIX) + 1;
		othna = malloc(sizeof(char) * len);
		strlcpy(othna, OTHER_ELEM_PREFIX, len);
		strlcat(othna, name, len);
	} else
		othna = strdup("NULL");
	sql = sqlite3_mprintf("INSERT OR REPLACE INTO %s (fieldname, alias, datatype,"
			" enumname, otherfield) VALUES('%q', '%q', %d, %s%q%s, %s%q%s);",
			tablename, name, alias, type, typen != NULL ? "'" : "",
			typen != NULL ? typen : "NULL", typen != NULL ? "'" : "", type == oenum ? "'" : "", othna, type == oenum ? "'" : "");
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "add_to_field_desc(f, \"%s\", \"%s\", \"%s\", %d, \"%s\"): SQLite error after exec: %s", tablename, name, alias, (int)type, typen, emsg);
		if(type == oenum)
			free(othna);
		sqlite3_free(emsg);
		return 0;
	}
	if(type == oenum)
		free(othna);
	return 1;
}

int add_enum_elem(f, tname, name, fmtname, value, dtype, subelements)
	fdbfs_t *f;
	char *tname;
	char *name;
	char *fmtname;
	unsigned int value;
	enum DataType dtype; /* XXX: check if enum values (the C type) are constant */
	char *subelements;
{
	char *sql;
	int rc;
	const char *tail;
	sqlite3_stmt *cst;

	sql = sqlite3_mprintf("INSERT OR REPLACE INTO %s "
		 "(name, fmtname, value, other, subelements) VALUES('%q', '%q', %d, %d, ?)",
		 tname, name, fmtname, value, (int)dtype);
	rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, &tail);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "add_enum_elem(f, \"%s\", \"%s\", \"%s\", %u, %d, \"%s\"): SQLite error after prepare: %s", tname, name, fmtname, value, (int)dtype, subelements,
				sqlite3_errmsg(f->db));
		return 0;
	}
	
	if(subelements == NULL)
		sqlite3_bind_null(cst, 1);
	else
		sqlite3_bind_text(cst, 1, subelements, strlen(subelements), SQLITE_STATIC);
	
	rc = sqlite3_step(cst);
	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		ERR(die, "add_enum_elem(f, \"%s\", \"%s\", \"%s\", %u, %d, \"%s\"): SQLite error after step: %s", tname, name, fmtname, value, (int)dtype, subelements,
				sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	sqlite3_finalize(cst);

	return 1;
}

int bind_field(f, count, type, value, len, stmt)
	fdbfs_t *f;
	int *count;
	enum DataType type;
	void *value; /* value will ALWAYS be a _pointer_ to something; there will be no value-casting. If it's null, it means (int)0x0 */
	size_t len; /* only for stuff that uses BLOB; for text, we will use strlen() */
	sqlite3_stmt *stmt;
{
	switch(type) {
		case number:
		case usnumber:
		case boolean:
		case oenum:
		case oenumsub:
			if(sqlite3_bind_int(stmt, (*count)++, value != NULL ? *(int*)value : 0x0) != SQLITE_OK) {
				return ERR(die, "bind_int: %s", sqlite3_errmsg(f->db));
			}
			break;
		case string:
			if(sqlite3_bind_text(stmt, (*count)++, (const char*)value, strlen((char*)value), SQLITE_STATIC /* not really, but as far as sqlite is concerned... */) !=
					SQLITE_OK)
				return ERR(die, "bind_text: %s", sqlite3_errmsg(f->db));
			break;
		case fp:
			if(sqlite3_bind_double(stmt, (*count)++, *(double*)value) != SQLITE_OK)
				return ERR(die, "bind_double(c = %x): %s", *count, sqlite3_errmsg(f->db));
			break;
		case image:
		case binary:
			if(sqlite3_bind_blob(stmt, (*count)++, (const char*)value, len, SQLITE_STATIC /* not really, but as far as sqlite is concerned... */) != SQLITE_OK)
				return ERR(die, "bind_blob(%d): %s", *count, sqlite3_errmsg(f->db));
			break;
	}
	return 1;
}

int get_lastupdate(f, cat, filename)
	fdbfs_t *f;
	char *cat;
	char *filename;
{
	sqlite3_stmt *cst;
	int rc;
	char *sql;
	const char *tail;
	int lu;
	/* CAT_TABLE_PREFIX */

	sql = sqlite3_mprintf("SELECT lastupdate FROM " CAT_TABLE_PREFIX "%s WHERE file == '%q'", cat, filename);
	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, &tail)) != SQLITE_OK) {
		ERR(die, "get_lastupdate: SQLite error after prepare %s", sqlite3_errmsg(f->db));
		sqlite3_free(sql);
		return -1;
	}
	sqlite3_free(sql);
	rc = sqlite3_step(cst);
	switch(rc) {
		case SQLITE_ROW:
		case SQLITE_OK:
			break;
		case SQLITE_DONE:
			sqlite3_finalize(cst);
			return -2;
			break;
		default:
			ERR(die, "get_lastupdate: SQLite error after prepare %s", sqlite3_errmsg(f->db));
			sqlite3_finalize(cst);
			return -1;
	}

	lu = sqlite3_column_int(cst, 0);

	sqlite3_finalize(cst);

	return lu;
}

