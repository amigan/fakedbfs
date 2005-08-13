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
/* $Amigan: fakedbfs/libfakedbfs/dbinit.c,v 1.6 2005/08/13 01:05:49 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include "dbspec.h"
/* us */
#include <dbspecdata.h>
#include <fakedbfs.h>
#include <lexdefines.h>
/* dbspec stuff */
#define ParseTOKENTYPE Toke
#define ParseARG_PDECL ,Heads *heads

RCSID("$Amigan: fakedbfs/libfakedbfs/dbinit.c,v 1.6 2005/08/13 01:05:49 dcp1990 Exp $")

void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseFree(void *p, void (*freeProc)(void*));
extern FILE *yyin; /* XXX: do not use; not thread safe */
typedef struct yy_buffer_state *YY_BUFFER_STATE;
int yylex(void);
YY_BUFFER_STATE yy_scan_string(const char *);
void yy_delete_buffer(YY_BUFFER_STATE);
void Parse(void *yyp, int yymajor, ParseTOKENTYPE yyminor ParseARG_PDECL);
void ParseTrace(FILE *TraceFILE, char *zTracePrompt);

char* normalise(s)
	char *s;
{
	char *n = strdup(s);
	size_t siz = strlen(n);
	int i;
	for(i = 0; i < siz; i++) {
		n[i] = tolower(n[i]);
		switch(n[i]) {
			case ' ':
			case '/':
				n[i] = '_';
		}
	}
	return n;
}
int parse_definition(f, filename)
	fdbfs_t *f;
	char *filename;
{
	char buffer[MAXLINE];
	Toke tz;
	int rc;
	FILE *tf;
	Heads h;
	void *parser;
	
	memset(&tz, 0, sizeof(tz));
	memset(&h, 0, sizeof(h));
	h.instance = f;
	h.db_enumh = enums_from_db(f);
	if(h.db_enumh == NULL && f->error.emsg != NULL)
		return CERR(die, "parse_definition: enum importation failed. ", NULL);
	h.db_cath = cats_from_db(f, h.db_enumh);
	if(h.db_cath == NULL && f->error.emsg != NULL)
		return CERR(die, "parse_definition: catalogue importation failed. ", NULL);
	tf = fopen(filename, "r");
	if(!tf)
		return ERR(die, "parse_definition: opening %s: %s", filename, strerror(errno));
	parser = ParseAlloc(malloc);
#ifdef PARSERDEBUG
	ParseTrace(stderr, "PARSER: ");
#endif
	while(!feof(tf)) {
		fgets(buffer, MAXLINE - 1, tf);
		yy_scan_string(buffer);
		while((rc = yylex()) != 0) {
			tz.num = yylval.number;
			tz.str = yylval.string;
#ifdef PARSERDEBUG
			fprintf(stderr, "rc == %d\n", rc);
#endif
			Parse(parser, rc, tz, &h);
			if(h.err || f->error.emsg != NULL) {
				CERR(die, "parse_definition(f, \"%s\"): error after Parse(). ", filename);
				fclose(tf);
				return 0;
			}
		}
	}
	Parse(parser, 0, tz, &h);
	ParseFree(parser, free);
	free_enum_head_list(h.db_enumh);
	free_cat_head_list(h.db_cath);
	dump_head_members(&h);
	fclose(tf);
	return make_tables_from_spec(f, filename, &h);
}

struct EnumElem* find_elem_by_name(h, name)
	struct EnumElem *h;
	char *name;
{
	struct EnumElem *c;
	if(h == NULL) return NULL;
	if(name == NULL)
		return NULL;
	for(c = h; c != NULL; c = c->next) {
		if(c->name != NULL)
			if(strcasecmp(name, c->name) == 0)
				return c;
	}
	return NULL;
}

struct CatalogueHead* find_cathead_by_name(h, name)
	struct CatalogueHead *h;
	char *name;
{
	struct CatalogueHead *c;
	if(h == NULL) return NULL;
	if(name == NULL)
		return NULL;
	for(c = h; c != NULL; c = c->next) {
		if(c->name != NULL)
			if(strcasecmp(name, c->name) == 0)
				return c;
	}
	return NULL;
}

struct EnumHead* find_enumhead_by_name(h, name)
	struct EnumHead *h;
	char *name;
{
	struct EnumHead *c;
	if(h == NULL) return NULL;
	if(name == NULL)
		return NULL;
	for(c = h; c != NULL; c = c->next) {
		if(c->name != NULL)
			if(strcasecmp(name, c->name) == 0)
				return c;
	}
	return NULL;
}

struct EnumSubElem* copy_sub_list(from, to, fajah, lastval)
	struct EnumSubElem *from;
	struct EnumSubElem *to; /* must be allocated; subsequent items will be allocated */
	struct EnumElem *fajah;
	int *lastval;
{
	struct EnumSubElem *c;
	struct EnumSubElem *tc, *tlast;
	if(from == NULL) return NULL;
	c = from;
	tc = to;
	memcpy(tc, c, sizeof(*c));
	tc->value = *lastval++;
	tc->father = fajah;
	tc->flags |= SUBE_IS_SAMEAS;
	tc->next = NULL;
	tlast = tc;
	for(c = c->next; c != NULL; c = c->next) {
		tc = allocz(sizeof(*c));
		tlast->next = tc;
		memcpy(tc, c, sizeof(*c));
		tc->value = *lastval++;
		tc->father = fajah;
		tc->flags |= SUBE_IS_SAMEAS;
		tc->next = NULL;
		tlast = tc;
	}
	return tlast;
}



#define BUFFERSIZE 4096

int init_db_tables(f)
	fdbfs_t *f;
{
	int rc;
	rc = create_table(f, "enum_list", "id INTEGER PRIMARY KEY, name TEXT, defined_in_table TEXT, defined_in_specfile TEXT, lastupdated INTEGER");
	if(!rc) return rc;
	rc = create_table(f, "cat_list", "id INTEGER PRIMARY KEY, name TEXT, alias TEXT, defined_in_table TEXT, field_desc_table TEXT, defined_in_specfile TEXT, lastupdated INTEGER");
	if(!rc) return rc;
	return 1;
}

size_t number_size(n)
	unsigned int n;
{
	/* we go up to 999,999 */
	if(n > 999999) return 7; /* insecure. so what? hopefully the snprintf() will truncate for us */
	if(n >= 100000) return 6;
	if(n >= 10000) return 5;
	if(n >= 1000) return 4;
	if(n >= 100) return 3;
	if(n >= 10) return 2;
	if(n >= 1) return 1;
	if(n == 0) return 1;
	fprintf(stderr, "WARNING in number_size(%d): had more than 7 digits! Expect the unexpected. Returning 8.\n", n);
	return 8; /* random number time */
}

char* construct_subelem_field(h)
	struct EnumSubElem *h;
{
	struct EnumSubElem *c;
	char *buf;
	char ourbuffer[512];
	size_t size = 1; /* null */
	if(h == NULL) return NULL;

	/* calculate the size */
	for(c = h; c != NULL; c = c->next) {
		if(c->name != NULL)
			size += strlen(c->name) + 1 /* record separator */ + 1 /* value/key separator */;
		else
			size += 1 + 1 + 1;

		size += number_size(c->value);
	}
	buf = malloc(size);
	*buf = '\0'; /* so we can use strlcat() for everything */

	/* actual traversing */
	for(c = h; c != NULL; c = c->next) {
		snprintf(ourbuffer, sizeof(ourbuffer), "%s\037%d\036", (c->flags & SUBE_IS_SELF ? "\021" : /* printf says "(null)" if it's null. this might truncate the other values */
													      (c->name == NULL ? "\021" : c->name))/* default to self if null */,
				c->value);
		if(ourbuffer[strlen(ourbuffer) - 1] != '\036')
			ourbuffer[strlen(ourbuffer) - 1] = '\036'; /* in the unlikely event that snprintf truncated */
		strlcat(buf, ourbuffer, size);
	}
	return buf;
}
		
int new_enum(f, specfile, h)
	fdbfs_t *f;
	char *specfile;
	struct EnumHead *h;
{
	struct EnumElem *ce;
	char *selm;
	char *tablename;
	const char *tabprefix = ENUM_TABLE_PREFIX;
	const size_t ln = sizeof(char) * (strlen(h->name) + strlen(tabprefix) + 3);
	tablename = malloc(ln);
	strlcpy(tablename, tabprefix, ln);
	strlcat(tablename, h->name, ln);
	if(!create_table(f, tablename, "id INTEGER PRIMARY KEY, name TEXT, fmtname TEXT, value INTEGER, other INTEGER, subelements TEXT")) {
		CERR(die, "new_enum(f, \"%s\", h): error making table. ", specfile);
		free(tablename);
		return 0;
	}
	if(!add_to_enum_list_table(f, h->name, tablename, specfile)) {
		CERR(die, "new_enum(f, \"%s\", h): error adding to enum list table. ", specfile);
		free(tablename);
		return 0;
	}

	for(ce = h->headelem; ce != NULL; ce = ce->next) {
		selm = construct_subelem_field(ce->subhead);
		if(!add_enum_elem(f, tablename, ce->name, ce->fmtname, ce->value, (ce->other ? ce->othertype : oenum), selm)) {
			free(tablename);
			if(selm != NULL) free(selm);
			CERR(die, "new_enum(f, \"%s\", h): error adding enum elem. ", specfile);
			return 0;
		}
		if(selm != NULL)
			free(selm);
	}
	free(tablename);
	return 1;
}


int new_catalog(f, specfile, h)
	fdbfs_t *f;
	char *specfile;
	struct CatalogueHead *h;
{
	struct CatElem *c;
	char *tdesc;
	char *tablename, *fieldtable;
	char ilbuffer[512];
	const char *tdescpref = "id INTEGER PRIMARY KEY, file TEXT, lastupdate INTEGER, ";
	const char *tnamepre = CAT_TABLE_PREFIX;
	const char *fieldpre = CAT_FIELD_TABLE_PREFIX;
	size_t tds = 1;
	const size_t tln = sizeof(char) * (strlen(h->name) + strlen(tnamepre) + 3);
	const size_t fln = sizeof(char) * (strlen(h->name) + strlen(fieldpre) + 3);
	
	tablename = malloc(tln);
	fieldtable = malloc(fln);
	
	strlcpy(tablename, tnamepre, tln);
	strlcat(tablename, h->name, tln);
	strlcpy(fieldtable, fieldpre, fln);
	strlcat(fieldtable, h->name, fln);

	if(!create_table(f, fieldtable, "id INTEGER PRIMARY KEY, fieldname TEXT, alias TEXT" "datatype INTEGER, enumname")) {
		CERR(die, "new_catalog(f, \"%s\", h): error creating field table %s. ", specfile, fieldtable);
		free(tablename);
		free(fieldtable);
		return 0;
	}

	if(!add_to_cat_list_table(f, h->name, h->fmtname, tablename, fieldtable, specfile)) {
		CERR(die, "new_catalog(f, \"%s\", h): error adding to cat list table. ", specfile);
		free(tablename);
		free(fieldtable);
		return 0;
	}

	/* calculate sizes */
	for(c = h->headelem; c != NULL; c = c->next) {
		tds += strlen(c->name) + 1 /* comma */
		       	+ strlen(gettype(c->type) /* gettype includes the space */);
	}
	tds += strlen(tdescpref);
	tdesc = malloc(tds);
	strlcpy(tdesc, tdescpref, tds);

	/* actual stuff */
	for(c = h->headelem; c != NULL; c = c->next) {
		snprintf(ilbuffer, sizeof(ilbuffer), ",%s%s", c->name, gettype(c->type));
		strlcat(tdesc, ilbuffer, tds);
		if(c->flags & CATE_USES_FC)
			*c->name = toupper(*c->name); /* XXX: is this safe? */
		if(!add_to_field_desc(f, fieldtable, c->name, c->alias, c->type, ((c->type == oenum || c->type == oenumsub) && c->enumptr != NULL ? c->enumptr->name : NULL))) {
			CERR(die, "new_catalog(f, \"%s\", h): error adding to cat field desc. ", specfile);
			free(tablename);
			free(fieldtable);
			free(tdesc);
			return 0;
		}
	}
	create_table(f, tablename, tdesc);
	free(tdesc);
	free(tablename);
	free(fieldtable);
	return 1;
}

int make_tables_from_spec(f, sfile, h)
	fdbfs_t *f;
	char *sfile;
	Heads *h;
{
	struct CatalogueHead *cch;
	struct EnumHead *ceh;

	/* enums */
	for(ceh = h->enumhead; ceh != NULL; ceh = ceh->next) {
		if(!new_enum(f, sfile, ceh)) {
			CERR(die, "make_tables_from_spec(f, \"%s\", h): error adding enum. ", sfile);
			return 0;
		}
	}
	
	/* catalogues */
	for(cch = h->cathead; cch != NULL; cch = cch->next) {
		if(!new_catalog(f, sfile, cch)) {
			CERR(die, "make_tables_from_spec(f, \"%s\", h): error adding catalogue. ", sfile);
			return 0;
		}
	}
	return 1;
}

int start_db(f)
	fdbfs_t *f;
{
	if(!table_exists(f, "cat_list")) {
		if(f->error.emsg == NULL) {
			return init_db_tables(f);
		} else {
			return 0;
		}
	}
	return 1;
}

struct EnumSubElem* subelements_from_field(f, fajah, subs)
	fdbfs_t *f;
	struct EnumElem *fajah;
	char *subs;
{
	char *cs = subs;
	char *p;
	char *dsp, *op;
	struct EnumSubElem *c = NULL, *n, *h = NULL;

	while((p = strsep(&cs, "\x1E" /* record sep */)) != NULL) {
		dsp = p;
		op = strsep(&dsp, "\x1F");
		/* op is the name, dsp is the value */
		n = allocz(sizeof(*n));
		if(*op == '\x11' /* DC1 */) {
			n->name = NULL;
			n->flags |= SUBE_IS_SELF;
		} else {
			n->name = strdup(op);
		}
		n->value = atoi(dsp);
		n->father = fajah;
		if(h == NULL) {
			h = c = n;
		} else {
			c->next = n;
			c = n;
		}
	}

	return h;
}


struct EnumElem* enumelems_from_dbtab(f, table)
	fdbfs_t *f;
	char *table;
{
	char *sql;
	int rc;
	char *cename, *cefmt;
	unsigned int cval;
	short int oth;
	struct EnumElem *c = NULL, *n, *h = NULL;
	char *subs;

	sqlite3_stmt *cst;

	sql = sqlite3_mprintf("SELECT * FROM %s;", table);
	
	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, NULL)) !=
			SQLITE_OK) {
		ERR(die, "enums_from_dbtab(): error in prepare: %s", sqlite3_errmsg(f->db));
		return NULL;
	}

	sqlite3_free(sql);
	
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		cename = strdup(sqlite3_column_text(cst, 1));
		cefmt = strdup(sqlite3_column_text(cst, 2));
		cval = sqlite3_column_int(cst, 3);
		oth = sqlite3_column_int(cst, 4);
		subs = strdup(sqlite3_column_text(cst, 5));
		n = allocz(sizeof(*n));
		n->name = cename;
		n->fmtname = cefmt;
		n->value = cval;
		n->other = (oth == oenum ? 0 : 1);
		n->othertype = oth;
		n->subhead = subelements_from_field(f, n, subs);
		free(subs);
		
		if(h == NULL) {
			h = c = n;
		} else {
			c->next = n;
			c = c->next;
		}
		
		if(n->subhead == NULL) {
			free_enum_elem_list(h);
			sqlite3_finalize(cst);
			CERR(die, "enumelems_from_dbtab: subelements. ", NULL);
			return NULL;
		}
	}

	sqlite3_finalize(cst);

	return h;
}
	
struct EnumHead* enums_from_db(f)
	fdbfs_t *f;
{
	const char *enumlistsql = "SELECT * FROM enum_list;";
	char *enumname;
	char *tabledef;
	struct EnumHead *c = NULL;
	struct EnumHead *n;
	struct EnumHead *h = NULL;
	int rc;

	sqlite3_stmt *cst;

	if((rc = sqlite3_prepare(f->db, enumlistsql, strlen(enumlistsql), &cst, NULL)) !=
			SQLITE_OK) {
		ERR(die, "enums_from_db(): error in prepare: %s", sqlite3_errmsg(f->db));
		return NULL;
	}
	
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		enumname = strdup(sqlite3_column_text(cst, 1));
		tabledef = strdup(sqlite3_column_text(cst, 2));
		n = allocz(sizeof(*n));
		n->name = enumname;
		n->headelem = enumelems_from_dbtab(f, tabledef);
		n->flags |= ENUMH_FROM_DB;
		free(tabledef);

		if(h == NULL) {
			h = c = n;
		} else {
			c->next = n;
			c = c->next;
		}

		if(n->headelem == NULL) {
			free_enum_head_list(h);
			sqlite3_finalize(cst);
			CERR(die, "enums_from_db: from dbtab error. ", NULL);
			return NULL;
		}
		
	}

	if(rc != SQLITE_DONE && rc != SQLITE_OK) {
		CERR(die, "enums_from_db: error pulling existing enums: %s", sqlite3_errmsg(f->db));
		return NULL;
	}

	sqlite3_finalize(cst);

	return h;
}

struct CatElem* catelems_from_dbtab(f, table, enumhead)
	fdbfs_t *f;
	char *table;
	struct EnumHead *enumhead;
{
	char *sql;
	int rc;
	char *ccname, *ccfmt, *cenumname;
	enum DataType cctype;
	struct CatElem *c = NULL, *n, *h = NULL;

	sqlite3_stmt *cst;

	sql = sqlite3_mprintf("SELECT * FROM %s;", table);
	
	if((rc = sqlite3_prepare(f->db, sql, strlen(sql), &cst, NULL)) !=
			SQLITE_OK) {
		ERR(die, "catelems_from_dbtab(): error in prepare: %s", sqlite3_errmsg(f->db));
		return NULL;
	}

	sqlite3_free(sql);
	
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		ccname = strdup(sqlite3_column_text(cst, 1));
		ccfmt = strdup(sqlite3_column_text(cst, 2));
		cctype = sqlite3_column_int(cst, 3);
		cenumname = strdup(sqlite3_column_text(cst, 4));
		n = allocz(sizeof(*n));
		n->name = ccname;
		n->alias = ccfmt;
		n->type = cctype;
		/* XXX: we don't check that an oenum element exists before allowing an 
		 * oenumsub. This may not be a big deal, but if anyone is bored and
		 * wants to fix it, feel free :-P
		 */
		if(cctype == oenum || cctype == oenumsub) {
			n->enumptr = find_enumhead_by_name(enumhead, cenumname);
			if(n->enumptr == NULL) {
				CERR(die, "Cannot find enum named %s! ", cenumname);
				free(cenumname);
				free_cat_elem_list(h != NULL ? h : n);
				sqlite3_finalize(cst);
				return NULL;
			}
		}

		free(cenumname);
		
		if(h == NULL) {
			h = c = n;
		} else {
			c->next = n;
			c = c->next;
		}
	}

	sqlite3_finalize(cst);

	return h;
}

struct CatalogueHead* cats_from_db(f, enumhead)
	fdbfs_t *f;
	struct EnumHead* enumhead;
{
	const char *catlistsql = "SELECT * FROM cat_list;";
	char *catname, *catalias;
	char *tabledef;
	struct CatalogueHead *c = NULL;
	struct CatalogueHead *n;
	struct CatalogueHead *h = NULL;
	int rc;

	sqlite3_stmt *cst;

	if((rc = sqlite3_prepare(f->db, catlistsql, strlen(catlistsql), &cst, NULL)) !=
			SQLITE_OK) {
		ERR(die, "cats_from_db(): error in prepare: %s", sqlite3_errmsg(f->db));
		return NULL;
	}
	
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		catname = strdup(sqlite3_column_text(cst, 1));
		catalias = strdup(sqlite3_column_text(cst, 2));
		tabledef = strdup(sqlite3_column_text(cst, 4));
		n = allocz(sizeof(*n));
		n->name = catname;
		n->fmtname = catalias; /* god only knows why I use such liberal naming of
					  alias and fmtname fields. Maybe I was drunk. */
		n->headelem = catelems_from_dbtab(f, tabledef, enumhead);
		n->flags |= CATH_FROM_DB;

		free(tabledef);

		if(h == NULL) {
			h = c = n;
		} else {
			c->next = n;
			c = c->next;
		}

		if(n->headelem == NULL) {
			free_cat_head_list(h);
			sqlite3_finalize(cst);
			CERR(die, "cats_from_db: from dbtab error. ", NULL);
			return NULL;
		}
		
	}

	if(rc != SQLITE_DONE && rc != SQLITE_OK) {
		CERR(die, "cats_from_db: error pulling existing enums: %s", sqlite3_errmsg(f->db));
		return NULL;
	}

	sqlite3_finalize(cst);

	return h;
}
