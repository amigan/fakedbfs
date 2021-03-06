/*
 * Copyright (c) 2005-2006, Dan Ponte
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
/* $Amigan: fakedbfs/libfakedbfs/sqlite.c,v 1.39 2009/02/11 15:06:00 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs/fdbfsconfig.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/db.h>
#include <fakedbfs/debug.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/sqlite.c,v 1.39 2009/02/11 15:06:00 dcp1990 Exp $")


/**
 * @brief Handle busy conditions in the sqlite database. (internal)
 */
int fdbfs_db_busy_handler(data, prior)
	void *data;
	int prior;
{
	fdbfs_t *f = (fdbfs_t *)data;

	fdbfs_debug_info(f, error, "SQLite error: BUSY (%d prior calls)\n", prior);
	if(prior < BUSY_RETRIES) {
		fdbfs_debug_info(f, warning, "Sleeping for 1 second...\n");
		sleep(1);
		return 0;
	} else {
		fdbfs_debug_info(f, error, "Aborting query!\n");
		return 1;
	}

	return 2;
}

int fdbfs_db_open(f)
	fdbfs_t *f;
{
	int rc;
	rc = sqlite3_open(f->dbname, &f->db);
	if(rc) {
		return ERR(die, "open_db: error opening database: %s", sqlite3_errmsg(f->db));
	}

	f->reg_norm.f = f;
	f->reg_negated.f = f;
	f->reg_norm.negated = 0;
	f->reg_negated.negated = 1;

	sqlite3_busy_handler(f->db, fdbfs_db_busy_handler, f);

	rc = sqlite3_create_function(f->db, "regexp", 2, SQLITE_ANY /* not exactly true... */, (void*)&f->reg_norm, fdbfs_db_regex_func, NULL, NULL);
	if(rc != SQLITE_OK) {
		return ERR(die, "open_db: error creating regexp function: %s", sqlite3_errmsg(f->db));
	}
	rc = sqlite3_create_function(f->db, "notregexp", 2, SQLITE_ANY /* not exactly true... */, (void*)&f->reg_negated, fdbfs_db_regex_func, NULL, NULL);
	if(rc != SQLITE_OK) {
		return ERR(die, "open_db: error creating regexp function: %s", sqlite3_errmsg(f->db));
	}
	return 1;
}

int fdbfs_db_close(f)
	fdbfs_t *f;
{
	int rc;
	if((rc = sqlite3_close(f->db)) != SQLITE_OK) {
		return ERR(die, "close_db: error closing database: %s", sqlite3_errmsg(f->db));
	}
	return 1;
}

int fdbfs_db_table_exists(f, tname)
	fdbfs_t *f;
	const char *tname;
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

int fdbfs_db_cat_exists(f, cat)
	fdbfs_t *f;
	const char *cat;
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

int fdbfs_db_create_table(f, tname, tspec)
	fdbfs_t *f;
	const char *tname;
	const char *tspec;
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

const char* fdbfs_db_gettype(t)
	enum DataType t;
{
	/* make sure you know what you're doing; we have defaults here */
	switch(t) {
		case number:
		case usnumber:
		case boolean:
		case oenum:
		case oenumsub:
		case datime:
		case bigint:
		case usbigint:
		case character:
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

/**
 * @brief Get the column type.
 *
 * getcoltype() returns a database-specific type string only for the four basic SQLite types.
 */
static const char* getcoltype(t)
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

int fdbfs_db_add_column(f, tname, cname, datatype)
	fdbfs_t *f;
	const char *tname;
	const char *cname;
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

int fdbfs_db_del_column(f, tname, cname)
	fdbfs_t *f;
	const char *tname;
	const char *cname;
{
	/*
	 * Do nothing; for other database drivers, this function might actually accomplish something
	 */
	return 1;
}

int fdbfs_db_add_to_enum_list_table(f, name, tname, specf)
	fdbfs_t *f;
	const char *name;
	const char *tname;
	const char *specf;
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

int fdbfs_db_cat_getcfdname(f, catname, tcfd)
	fdbfs_t *f;
	const char *catname;
	char **tcfd;
{
	char *sql;
	int rc;
	sqlite3_stmt *stmt;
	const char *tail;

	sql = sqlite3_mprintf("SELECT field_desc_table FROM cat_list WHERE name == '%q'", catname);
	rc = sqlite3_prepare(f->db, sql, strlen(sql), &stmt, &tail);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "cat_getcfdname: %s", sqlite3_errmsg(f->db));
		return 0;
	}
	rc = sqlite3_step(stmt);
	if(rc != SQLITE_DONE && rc != SQLITE_OK && rc != SQLITE_ROW) {
		ERR(die, "cat_getcfdname: step returned !DONE: %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(stmt);
		return 0;
	}
	if(rc == SQLITE_ROW) {
		const char *tcf;
		tcf = (const char*)sqlite3_column_text(stmt, 0);
		if(strlen(tcf) < 5) {
			goto badres;
		}
		*tcfd = strdup(tcf + 4);
	} else {
badres:
		*tcfd = NULL;
	}
	sqlite3_finalize(stmt);

	return 1;
}

int fdbfs_db_cfd_update_refcount(f, name, add, val)
	fdbfs_t *f;
	const char *name;
	int add; /* 1 to add, 0 to sub */
	unsigned int val;
{
	char *sql;
	char *emsg;
	int rc;
	
	sql = sqlite3_mprintf("UPDATE cfd_list SET refcount = (SELECT refcount FROM cfd_list WHERE name == '%q') %c (%d) WHERE name == '%q'",
			name, add ? '+' : '-', val, name);
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "db_cfd_update_refcount(f, %s, %d): SQLite error: %s", name, add, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

int fdbfs_db_add_to_cfd_list_table(f, name, alias, tablename, specfile)
	fdbfs_t *f;
	const char *name;
	const char *alias;
	const char *tablename;
	const char *specfile;
{
	char *sql, *emsg;
	int rc;
	sql = sqlite3_mprintf("INSERT OR REPLACE INTO cfd_list (name, alias, defined_in"
			"_table, defined_in_specfile, refcount, lastupdated) "
			"VALUES('%q', '%q', '%q', '%q', %d, %d);",
			name, alias, tablename, specfile, 0x0, time(NULL));
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "add_to_cfd_list_table(f, \"%s\", \"%s\", \"%s\", \"%s\"): SQLite error after exec: %s", name, alias, tablename, specfile, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}


int fdbfs_db_add_to_cat_list_table(f, name, alias, tablename, fieldtable)
	fdbfs_t *f;
	const char *name;
	const char *alias;
	const char *tablename;
	const char *fieldtable;
{
	char *sql, *emsg;
	int rc;
	sql = sqlite3_mprintf("INSERT OR REPLACE INTO cat_list (name, alias, defined_in"
			"_table, field_desc_table, lastupdated) "
			"VALUES('%q', '%q', '%q', '%q', %d);",
			name, alias, tablename, fieldtable, time(NULL));
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "add_to_cat_list_table(f, \"%s\", \"%s\", \"%s\", \"%s\"): SQLite error after exec: %s", name, alias, tablename, fieldtable, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

int fdbfs_db_add_to_field_desc(f, tablename, name, alias, type, typen)
	fdbfs_t *f;
	const char *tablename;
	const char *name;
	const char *alias;
	enum DataType type;
	const char *typen;
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

int fdbfs_db_delete(f, from, wherecol, wherecmp, whereval)
	fdbfs_t *f;
	const char *from;
	const char *wherecol;
	const char *wherecmp;
	const char *whereval;
{
	char *sql, *emsg;
	int rc;
	sql = sqlite3_mprintf("DELETE FROM %s WHERE %s %s '%q'", from, wherecol, wherecmp, whereval);
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "db_delete(f, '%s', '%s', '%s', '%s'): SQLite error after exec: %s", from, wherecol,
				wherecmp, whereval, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

int fdbfs_db_drop_table(f, tablename)
	fdbfs_t *f;
	const char *tablename;
{
	char *sql, *emsg;
	int rc;
	sql = sqlite3_mprintf("DROP TABLE %s", tablename);
	rc = sqlite3_exec(f->db, sql, NULL, NULL, &emsg);
	sqlite3_free(sql);
	if(rc != SQLITE_OK) {
		ERR(die, "drop_table(f, '%s'): SQLite error after exec: %s", tablename, emsg);
		sqlite3_free(emsg);
		return 0;
	}
	return 1;
}

int fdbfs_db_add_enum_elem(f, tname, name, fmtname, value, dtype, subelements)
	fdbfs_t *f;
	const char *tname;
	const char *name;
	const char *fmtname;
	unsigned int value;
	enum DataType dtype; /* XXX: check if enum values (the C type) are constant */
	const char *subelements;
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

int fdbfs_db_bind_field(f, count, type, value, len, stmt)
	fdbfs_t *f;
	int *count;
	enum DataType type;
	void *value; /* value will ALWAYS be a _pointer_ to something; there will be no value-casting. If it's null, it means (int)0x0 */
	size_t len; /* only for stuff that uses BLOB; for text, we will use strlen() */
	sqlite3_stmt *stmt;
{
	double tval = 0.0;
	int istrans = 0;
	switch(type) {
		case number:
		case usnumber:
		case boolean:
		case oenum:
		case oenumsub:
		case character:
#ifdef INDEX_SQL_DEBUG
			printf("int %d\n", value != NULL ? *(int*)value : 0x0);
#endif
			if(sqlite3_bind_int(stmt, (*count)++, value != NULL ? *(int*)value : 0x0) != SQLITE_OK) {
				return ERR(die, "bind_int: %s", sqlite3_errmsg(f->db));
			}
			break;
		case datime:
		case bigint:
		case usbigint:
			if(sqlite3_bind_int64(stmt, (*count)++, value != NULL ? *(int64_t*)value : 0x0) != SQLITE_OK) {
				return ERR(die, "bind_wideint: %s", sqlite3_errmsg(f->db));
			}
			break;
		case string:
			if(value == NULL) {
				value = "";
				istrans = 1;
			}
#ifdef INDEX_SQL_DEBUG
			printf("string '%s'\n", (char*)value);
#endif
			if(sqlite3_bind_text(stmt, (*count)++, (const char*)value, strlen((char*)value),
						istrans ? SQLITE_TRANSIENT : SQLITE_STATIC
						/* not really, but as far as sqlite is concerned... */) !=
					SQLITE_OK)
				return ERR(die, "bind_text: %s", sqlite3_errmsg(f->db));
			break;
		case fp:
			if(value == NULL) {
				value = &tval;
			}
#ifdef INDEX_SQL_DEBUG
			printf("float %f\n", *(double*)value);
#endif
			if(sqlite3_bind_double(stmt, (*count)++, *(double*)value) != SQLITE_OK)
				return ERR(die, "bind_double(c = %x): %s", *count, sqlite3_errmsg(f->db));
			break;
		case image:
		case binary:
			if(value == NULL) {
				value = "";
				len = 1;
				istrans = 1;
			}
#ifdef INDEX_SQL_DEBUG
			printf("bin %p\n", value);
#endif
			if(sqlite3_bind_blob(stmt, (*count)++, (const char*)value, len,
						istrans ? SQLITE_TRANSIENT : SQLITE_STATIC
						/* not really, but as far as sqlite is concerned... */) != SQLITE_OK)
				return ERR(die, "bind_blob(%d): %s", *count, sqlite3_errmsg(f->db));
			break;
	}
	return 1;
}

int fdbfs_db_get_lastupdate(f, cat, filename)
	fdbfs_t *f;
	const char *cat;
	const char *filename;
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

int fdbfs_db_mib_add(f, mib, type, data)
	fdbfs_t *f;
	const char *mib;
	enum DataType type;
	union Data data;
{
	sqlite3_stmt *cst;
	const char *tail;
	const char *sql = "INSERT OR REPLACE INTO " CONFTABLE " (mib, type, value) VALUES(?, ?, ?)";
	int rc;

	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, &tail)) != SQLITE_OK) {
		ERR(die, "db_mib_add: SQLite error after prepare %s", sqlite3_errmsg(f->db));
		return 0;
	}

	if((rc = sqlite3_bind_text(cst, 1, mib, strlen(mib), SQLITE_TRANSIENT /* necessary? */)) != SQLITE_OK) {
		ERR(die, "db_mib_add: SQLite error after bind_text (mib): %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	if((rc = sqlite3_bind_int(cst, 2, type)) != SQLITE_OK) {
		ERR(die, "db_mib_add: SQLite error after bind_int (type): %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	switch(type) {
		case oenum:
		case oenumsub:
		case image:
			sqlite3_finalize(cst);
			SERR(die, "db_mib_add: error: oenum/oenumsub not supported by conf system");
			return 0;
		case number:
		case usnumber:
		case boolean:
		case character:
			rc = sqlite3_bind_int(cst, 3, data.integer);
			break;
		case datime:
		case usbigint:
		case bigint:
			rc = sqlite3_bind_int64(cst, 3, (int64_t)data.linteger);
			break;
		case string:
			rc = sqlite3_bind_text(cst, 3, data.string, strlen(data.string), SQLITE_STATIC);
			break;
		case fp:
			rc = sqlite3_bind_double(cst, 3, data.fp);
			break;
		case binary:
			rc = sqlite3_bind_blob(cst, 3, data.pointer.ptr, data.pointer.len, SQLITE_STATIC);
			break;
		default:
			sqlite3_finalize(cst);
			return SERR(die, "db_mib_add: unsupported datatype");
	}

	if(rc != SQLITE_OK) {
		ERR(die, "db_mib_add: SQLite error after bind (data): %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	if((rc = sqlite3_step(cst)) != SQLITE_DONE) {
		ERR(die, "db_mib_add: error executing query: %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	return 1;
}

int fdbfs_db_mib_update(f, mib, type, data)
	fdbfs_t *f;
	const char *mib;
	enum DataType type;
	union Data data;
{
	sqlite3_stmt *cst;
	const char *tail;
	const char *sql = "UPDATE " CONFTABLE " SET type = ?, value = ? WHERE mib == ?";
	int rc;

	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, &tail)) != SQLITE_OK) {
		ERR(die, "db_mib_update: SQLite error after prepare %s", sqlite3_errmsg(f->db));
		return 0;
	}

	if((rc = sqlite3_bind_text(cst, 3, mib, strlen(mib), SQLITE_TRANSIENT /* necessary? */)) != SQLITE_OK) {
		ERR(die, "db_mib_update: SQLite error after bind_text (mib): %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	if((rc = sqlite3_bind_int(cst, 1, type)) != SQLITE_OK) {
		ERR(die, "db_mib_update: SQLite error after bind_int (type): %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	switch(type) {
		case oenum:
		case oenumsub:
		case image:
			sqlite3_finalize(cst);
			SERR(die, "db_mib_update: error: oenum/oenumsub not supported by conf system");
			return 0;
		case number:
		case usnumber:
		case boolean:
		case character:
			rc = sqlite3_bind_int(cst, 2, data.integer);
			break;
		case datime:
		case usbigint:
		case bigint:
			rc = sqlite3_bind_int64(cst, 2, (int64_t)data.linteger);
			break;
		case string:
			rc = sqlite3_bind_text(cst, 2, data.string, strlen(data.string), SQLITE_STATIC);
			break;
		case fp:
			rc = sqlite3_bind_double(cst, 2, data.fp);
			break;
		case binary:
			rc = sqlite3_bind_blob(cst, 2, data.pointer.ptr, data.pointer.len, SQLITE_STATIC);
			break;
		default:
			sqlite3_finalize(cst);
			return SERR(die, "db_mib_update: unsupported datatype");
	}

	if(rc != SQLITE_OK) {
		ERR(die, "db_mib_update: SQLite error after bind (data): %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	if((rc = sqlite3_step(cst)) != SQLITE_DONE) {
		ERR(die, "db_mib_update: error executing query: %s", sqlite3_errmsg(f->db));
		sqlite3_finalize(cst);
		return 0;
	}

	return 1;
}
