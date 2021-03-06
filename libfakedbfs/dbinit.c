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
 * @file dbinit.c
 * @brief Database initialisation stuff.
 */
/* $Amigan: fakedbfs/libfakedbfs/dbinit.c,v 1.54 2009/02/11 15:06:00 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include "dbspec.h"
/* us */
#include <fakedbfs/dbspecdata.h>
#include <fakedbfs/lexdefines.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/db.h>
#include <fakedbfs/debug.h>
/* dbspec stuff */
#define DSpecParseTOKENTYPE Toke
#define DSpecParseARG_PDECL ,Heads *heads

RCSID("$Amigan: fakedbfs/libfakedbfs/dbinit.c,v 1.54 2009/02/11 15:06:00 dcp1990 Exp $")

void *DSpecParseAlloc(void *(*mallocProc)(size_t));
void DSpecParseFree(void *p, void (*freeProc)(void*));
extern FILE *fdbfs_dsin;
typedef struct fdbfs_ds_buffer_state *YY_BUFFER_STATE;
int fdbfs_dslex(void);
YY_BUFFER_STATE fdbfs_ds_scan_string(const char *);
void fdbfs_ds_delete_buffer(YY_BUFFER_STATE);
void DSpecParse(void *yyp, int yymajor, DSpecParseTOKENTYPE yyminor DSpecParseARG_PDECL);
void DSpecParseTrace(FILE *TraceFILE, char *zTracePrompt);
void yywrap(void);

char* fdbfs_normalise(s)
	const char *s;
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

struct EnumElem* fdbfs_find_elem_by_name(h, name)
	struct EnumElem *h;
	const char *name;
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

struct CatElem* fdbfs_find_catelem_enum(h, en)
	struct CatElem *h;
	struct EnumHead *en;
{
	struct CatElem *c;

	for(c = h; c != NULL; c = c->next) {
		if(c->enumptr == en && c->type == oenum)
			return c;
	}

	return NULL;
}

char* fdbfs_get_enum_string_by_value(h, val, fmted)
	struct EnumElem *h;
	unsigned int val;
	short int fmted;
{
	struct EnumElem *c;

	for(c = h; c != NULL; c = c->next) {
		if(c->value == val)
			return (fmted ? c->fmtname : c->name);
	}

	return NULL;
}

struct EnumSubElem* fdbfs_get_subhead_by_enval(h, val)
	struct EnumElem *h;
	unsigned int val;
{
	struct EnumElem *c;

	for(c = h; c != NULL; c = c->next) {
		if(c->value == val)
			return c->subhead;
	}
	
	return NULL;
}

char* fdbfs_get_enum_sub_string_by_value(h, val)
	struct EnumSubElem *h;
	unsigned int val;
{
	struct EnumSubElem *c;

	for(c = h; c != NULL; c = c->next) {
		if(c->value == val)
			return c->name;
	}

	return NULL;
}

struct CatalogueHead* fdbfs_find_cathead_by_name(h, name)
	struct CatalogueHead *h;
	const char *name;
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


struct CatElem* fdbfs_find_catelem_by_name(h, name)
	struct CatElem *h;
	const char *name;
{
	struct CatElem *c;
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

struct EnumHead* fdbfs_find_enumhead_by_name(h, name)
	struct EnumHead *h;
	const char *name;
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

struct EnumSubElem* fdbfs_copy_sub_list(from, to, fajah, lastval)
	struct EnumSubElem *from;
	struct EnumSubElem *to; /* must be allocated; subsequent items will be allocated */
	struct EnumElem *fajah;
	unsigned int *lastval;
{
	struct EnumSubElem *c;
	struct EnumSubElem *tc, *tlast;
	if(from == NULL) return NULL;
	c = from;
	tc = to;
#ifdef FREEDEBUG
	printf("to %p\n", tc);
#endif
	memcpy(tc, c, sizeof(*c));
	tc->value = *(lastval)++;
	tc->father = fajah;
	tc->flags |= SUBE_IS_SAMEAS;
	tc->next = NULL;
	tlast = tc;
	for(c = c->next; c != NULL; c = c->next) {
		if(c->flags & SUBE_IS_ALLSUB)
			continue;
		tc = allocz(sizeof(*c));
		tlast->next = tc;
		memcpy(tc, c, sizeof(*c));
		tc->value = (*lastval)++;
		tc->father = fajah;
		tc->flags |= SUBE_IS_SAMEAS;
		tc->next = NULL;
#ifdef FREEDEBUG
		printf("nam %s %p\n", tc->name, tc);
#endif
		tlast = tc;
	}
	return tlast;
}



#define BUFFERSIZE 4096

/**
 * @brief Initialise database tables.
 *
 * @param f Instance of fakedbfs to work on.
 * @return 0 on error.
 */
static int init_db_tables(f)
	fdbfs_t *f;
{
	int rc;
	rc = fdbfs_db_create_table(f, "enum_list", "id INTEGER PRIMARY KEY, name TEXT, defined_in_table TEXT, defined_in_specfile TEXT, lastupdated INTEGER");
	if(!rc) return rc;
	rc = fdbfs_db_create_table(f, "cat_list", "id INTEGER PRIMARY KEY, name TEXT UNIQUE, alias TEXT, defined_in_table TEXT, field_desc_table TEXT, lastupdated INTEGER");
	if(!rc) return rc;
	rc = fdbfs_db_create_table(f, "cfd_list", "id INTEGER PRIMARY KEY, name TEXT UNIQUE, alias TEXT, defined_in_table TEXT, defined_in_specfile TEXT, refcount INTEGER, lastupdated INTEGER");
	if(!rc) return rc;
	return 1;
}

size_t fdbfs_number_size(n)
	unsigned int n;
{
	return (n == 0) ? 1 : floor(log10(n)) + 1;
}

size_t fdbfs_signed_size(n)
	int n;
{
	return (n == 0) ? 1 : floor(log10(n)) + (n < 0 ? 2 : 1); /* sign */
}

/**
 * @brief Construct subelement field from EnumSubElem list.
 *
 * Constructs a subelement field suitable for insertion into the database per the TABLE_FORMAT.
 * @param h Subelement head to examine.
 * @return malloc()'d string containing the subelements, NULL on error.
 */
static char* construct_subelem_field(h)
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

		size += fdbfs_number_size(c->value);
	}
	buf = malloc(size);
	if(buf == NULL) {
		printf("malloc error: %s\n", strerror(errno));
		return NULL;
	}
	*buf = '\0'; /* so we can use strlcat() for everything */

	/* actual traversing */
	for(c = h; c != NULL; c = c->next) {
		/* XXX: this needs to be more fail-safe. We can't afford having the damned thing just bomb
		 * without telling anyone if the buffer turns out to be too small.
		 */
		snprintf(ourbuffer, sizeof(ourbuffer), "%s\037%d\036", (c->flags & SUBE_IS_SELF ? "\021" : /* printf says "(null)" if it's null. this might truncate the other values */
													      (c->name == NULL ? "\021" : c->name))/* default to self if null */,
				c->value);
		if(ourbuffer[strlen(ourbuffer) - 1] != '\036')
			ourbuffer[strlen(ourbuffer) - 1] = '\036'; /* in the unlikely event that snprintf truncated */
		strlcat(buf, ourbuffer, size);
	}
	return buf;
}
		
static int new_enum(f, specfile, h)
	fdbfs_t *f;
	const char *specfile;
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
	if(!fdbfs_db_create_table(f, tablename, "id INTEGER PRIMARY KEY, name TEXT, fmtname TEXT, value INTEGER, other INTEGER, subelements TEXT")) {
		CERR(die, "new_enum(f, \"%s\", h): error making table. ", specfile);
		free(tablename);
		return 0;
	}
	if(!fdbfs_db_add_to_enum_list_table(f, h->name, tablename, specfile)) {
		CERR(die, "new_enum(f, \"%s\", h): error adding to enum list table. ", specfile);
		free(tablename);
		return 0;
	}

	for(ce = h->headelem; ce != NULL; ce = ce->next) {
		selm = construct_subelem_field(ce->subhead);
		if(!fdbfs_db_add_enum_elem(f, tablename, ce->name, ce->fmtname, ce->value, (ce->other ? ce->othertype : oenum), selm)) {
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

static int new_cft(f, specfile, h)
	fdbfs_t *f;
	const char *specfile;
	struct CatalogueHead *h;
{
	struct CatElem *c;
	char *fieldtable;
	const char *fieldpre = CAT_FIELD_TABLE_PREFIX;
	const size_t fln = sizeof(char) * (strlen(h->name) + strlen(fieldpre) + 3);
	char *ptname;
	char *tali;
	
	fieldtable = malloc(fln);
	
	strlcpy(fieldtable, fieldpre, fln);
	strlcat(fieldtable, h->name, fln);

	if(!fdbfs_db_create_table(f, fieldtable, "id INTEGER PRIMARY KEY, fieldname TEXT, alias TEXT, datatype INTEGER, enumname TEXT, otherfield TEXT")) {
		CERR(die, "new_cft(f, h): error creating field table %s. ", fieldtable);
		free(fieldtable);
		return 0;
	}

	if(!fdbfs_db_add_to_cfd_list_table(f, h->name, h->fmtname, fieldtable, specfile)) {
		CERR(die, "new_cft('%s'): error adding to CFD list table. ", h->name);
		fdbfs_db_drop_table(f, fieldtable); /* probably fruitless */
		free(fieldtable);
		return 0;
	}

	/* actual stuff */
	for(c = h->headelem; c != NULL; c = c->next) {
		tali = NULL;
		if(c->flags & CATE_USES_FC && c->alias == NULL) {
			c->alias = strdup(c->name);
			*c->alias = toupper(*c->alias);
		}

		if(c->flags & CATE_USES_FC && c->alias == c->name) {
			tali = strdup(c->name);
			*tali = toupper(*tali);
		}

		if(c->type == oenum)
			ptname = c->enumptr->name;
		else if(c->type == oenumsub)
			ptname = c->subcatel->name;
		else
			ptname = NULL;

		if(!fdbfs_db_add_to_field_desc(f, fieldtable, c->name, tali ? tali : c->alias, c->type, ptname)) {
			SCERR(die, "new_cft(f, h): error adding to cat field desc. ");
			if(tali)
				free(tali);
			free(fieldtable);
			return 0;
		}

		if(tali)
			free(tali);
	}
	free(fieldtable);
	return 1;
}

static int new_catalog(f, name, alias, h)
	fdbfs_t *f;
	const char *name;
	const char *alias;
	struct CatalogueHead *h;
{
	struct CatElem *c;
	char *tdesc;
	char *tablename, *fieldtable;
	char ilbuffer[512];
	const char *tdescpref = "id INTEGER PRIMARY KEY, file TEXT UNIQUE, lastupdate INTEGER, ctime INTEGER, mime TEXT";
	const char *tnamepre = CAT_TABLE_PREFIX;
	const char *fieldpre = CAT_FIELD_TABLE_PREFIX;
	size_t tds = 1;
	const size_t tln = sizeof(char) * (strlen(name) + strlen(tnamepre) + 3);
	const size_t fln = sizeof(char) * (strlen(h->name) + strlen(fieldpre) + 3);
	
	tablename = malloc(tln);
	fieldtable = malloc(fln);
	
	strlcpy(tablename, tnamepre, tln);
	strlcat(tablename, name, tln);
	strlcpy(fieldtable, fieldpre, fln);
	strlcat(fieldtable, h->name, fln);

	if(!fdbfs_db_add_to_cat_list_table(f, name, alias, tablename, fieldtable)) {
		CERR(die, "new_catalog(f, \"%s\", \"%s\", h): error adding to cat list table. ", name, alias);
		free(tablename);
		free(fieldtable);
		return 0;
	}

	if(!fdbfs_db_cfd_update_refcount(f, h->name, 1, 1)) {
		CERR(die, "new_catalog(f, \"%s\", \"%s\", h): error updating CFD refcount. ", name, alias);
		free(tablename);
		free(fieldtable);
		return 0;
	}

	/* calculate sizes */
	for(c = h->headelem; c != NULL; c = c->next) {
		tds += strlen(c->name) + 1 /* comma */
		       	+ strlen(fdbfs_db_gettype(c->type) /* gettype includes the space */);
		if(c->type == oenum && c->enumptr->otherelem != NULL)
			tds += strlen(c->name) + strlen(OTHER_ELEM_PREFIX) + 1 + strlen(c->enumptr->otherelem != NULL ? /* why is this ternary here? */
					fdbfs_db_gettype(c->enumptr->otherelem->othertype) : " BLOB" /*universal */);
	}
	tds += strlen(tdescpref);
	tdesc = malloc(tds);
	strlcpy(tdesc, tdescpref, tds);

	/* actual stuff */
	for(c = h->headelem; c != NULL; c = c->next) {
		snprintf(ilbuffer, sizeof(ilbuffer), ",%s%s", c->name, fdbfs_db_gettype(c->type));
		strlcat(tdesc, ilbuffer, tds);
		if(c->type == oenum && c->enumptr->otherelem != NULL) {
			snprintf(ilbuffer, sizeof(ilbuffer), ",%s%s%s", OTHER_ELEM_PREFIX, c->name, c->enumptr->otherelem != NULL ?
				fdbfs_db_gettype(c->enumptr->otherelem->othertype) : " BLOB");
			strlcat(tdesc, ilbuffer, tds);
		}
		if(c->flags & CATE_USES_FC && c->alias == NULL) {
			c->alias = strdup(c->name);
			*c->alias = toupper(*c->alias);
		}
	}
	if(!fdbfs_db_create_table(f, tablename, tdesc)) {
		CERR(die, "new_catalog(f, \"%s\", \"%s\", h): error creating field table %s. ", name, alias, fieldtable);
		free(tablename);
		free(tdesc);
		free(fieldtable);
		return 0;
	}
	free(tdesc);
	free(tablename);
	free(fieldtable);
	return 1;
}

/**
 * @brief Make tables from specfile.
 *
 * Stores what it finds in the heads in the database.
 * @param f Instance of fakedbfs to operate on.
 * @param sfile Filename of specfile.
 * @param h Heads object to read from.
 * @return 0 on error.
 */
static int make_tables_from_spec(f, sfile, h)
	fdbfs_t *f;
	const char *sfile;
	Heads *h;
{
	struct CatalogueHead *cch;
	struct EnumHead *ceh;

	/* enums */
	for(ceh = h->enumhead; ceh != NULL; ceh = ceh->next) {
		if(!new_enum(f, sfile, ceh)) {
			CERR(die, "make_tables_from_spec(f, \"%s\", h): error adding enum. ", sfile);
			fdbfs_free_cat_head_list(h->cathead);
			fdbfs_free_enum_head_list(h->enumhead);
			return 0;
		}
	}
	
	/* catalogues */
	for(cch = h->cathead; cch != NULL; cch = cch->next) {
		if(!new_cft(f, sfile, cch)) {
			fdbfs_free_cat_head_list(h->cathead);
			fdbfs_free_enum_head_list(h->enumhead);
			CERR(die, "make_tables_from_spec(f, \"%s\", h): error adding catalogue. ", sfile);
			return 0;
		}
#if 0
		if(!new_catalog(f, cch->name, sfile, cch)) {
			fdbfs_free_cat_head_list(h->cathead);
			fdbfs_free_enum_head_list(h->enumhead);
			CERR(die, "make_tables_from_spec(f, \"%s\", h): error adding catalogue. ", sfile);
			return 0;
		}
#endif
	}

	fdbfs_free_cat_head_list(h->cathead);
	fdbfs_free_enum_head_list(h->enumhead);

	
	return 1;
}

int fdbfs_create_catalogue(f, name, alias, cname)
	fdbfs_t *f;
	const char *name;
	const char *alias;
	const char *cname;
{
	struct CatalogueHead *cat;

	cat = fdbfs_find_cathead_by_name(f->heads.db_cath, cname);
	if(cat == NULL) {
		ERR(die, "create_catalogue: no such catalogue named '%s'", cname);
		return 0;
	}
	if(!new_catalog(f, name, alias, cat)) {
		return 0;
	}

	return 1;
}



int fdbfs_db_start(f)
	fdbfs_t *f;
{
	if(!fdbfs_db_table_exists(f, "cat_list")) {
		if(f->error.emsg == NULL) {
			return init_db_tables(f);
		} else {
			return 0;
		}
	}
	return 1;
}

/**
 * @brief Construct EnumSubElem tree from subelements field.
 *
 * Basically the reverse of construct_subelem_field().
 * @param f Instance of fakedbfs to work on.
 * @param fajah Father of subelements read.
 * @param subs Field to parse.
 * @return EnumSubElem list, or NULL on error.
 */
static struct EnumSubElem* subelements_from_field(f, fajah, subs)
	fdbfs_t *f;
	struct EnumElem *fajah;
	char *subs;
{
	char *cs = subs;
	char *p;
	char *dsp, *op;
	struct EnumSubElem *c = NULL, *n, *h = NULL;

	if(subs == NULL)
		return NULL;

	while((p = strsep(&cs, "\x1E" /* record sep */)) != NULL) {
		dsp = p;
		op = strsep(&dsp, "\x1F");
		/* op is the name, dsp is the value */
		if(dsp == NULL)
			continue;
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

struct EnumElem* fdbfs_enumelems_from_dbtab(f, table, e)
	fdbfs_t *f;
	const char *table;
	struct EnumHead *e;
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
		sqlite3_free(sql);
		ERR(die, "enums_from_dbtab(): error in prepare: %s", sqlite3_errmsg(f->db));
		return NULL;
	}

	sqlite3_free(sql);
	
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		cename = strdup((const char*)sqlite3_column_text(cst, 1));
		cefmt = strdup((const char*)sqlite3_column_text(cst, 2));
		cval = sqlite3_column_int(cst, 3);
		oth = sqlite3_column_int(cst, 4);
		subs = (char*)sqlite3_column_text(cst, 5);
		subs = (subs == NULL ? NULL : strdup(subs));
		n = allocz(sizeof(*n));
		n->name = cename;
		n->fmtname = cefmt;
		n->value = cval;
		n->other = (oth == oenum ? 0 : 1);
		n->othertype = oth;
		n->subhead = subelements_from_field(f, n, subs);
#ifdef QUERY_DEBUG
		printf("oth of %s is %d (%s)\n", n->name, oth, oth == oenum ? "oenum" : "not oenum");
#endif
		if(oth != oenum) {
#ifdef QUERY_DEBUG
			printf("oth of eh %p is not oenum\n", e);
#endif
			e->otherelem = n;
		}
		if(subs != NULL)
			free(subs);
		
		if(h == NULL) {
			h = c = n;
		} else {
			c->next = n;
			c = c->next;
		}
		
		if(n->subhead == NULL && f->error.emsg != NULL) {
			fdbfs_free_enum_elem_list(h);
			sqlite3_finalize(cst);
			SCERR(die, "enumelems_from_dbtab: subelements. ");
			return NULL;
		}
	}

	sqlite3_finalize(cst);


	return h;
}
	
struct EnumHead* fdbfs_enums_from_db(f)
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
		enumname = strdup((const char*)sqlite3_column_text(cst, 1));
		tabledef = strdup((const char*)sqlite3_column_text(cst, 2));
		n = allocz(sizeof(*n));
		n->name = enumname;
		n->headelem = fdbfs_enumelems_from_dbtab(f, tabledef, n);
		n->flags |= ENUMH_FROM_DB;
		free(tabledef);

		if(h == NULL) {
			h = c = n;
		} else {
			c->next = n;
			c = c->next;
		}

		if(n->headelem == NULL && f->error.emsg != NULL) {
			fdbfs_free_enum_head_list(h);
			sqlite3_finalize(cst);
			SCERR(die, "enums_from_db: from dbtab error. ");
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

struct CatElem* fdbfs_catelems_from_dbtab(f, table, enumhead)
	fdbfs_t *f;
	const char *table;
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
		sqlite3_free(sql);
		ERR(die, "catelems_from_dbtab(): error in prepare: %s", sqlite3_errmsg(f->db));
		return NULL;
	}

	sqlite3_free(sql);
	
	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		ccname = strdup((const char*)sqlite3_column_text(cst, 1));
		ccfmt = strdup((const char*)sqlite3_column_text(cst, 2));
		cctype = sqlite3_column_int(cst, 3);
		cenumname = (char*)sqlite3_column_text(cst, 4);
		if(cenumname != NULL)
			cenumname = strdup(cenumname);
		else
			cenumname = strdup("");
		n = allocz(sizeof(*n));
		n->name = ccname;
		n->alias = ccfmt;
		n->type = cctype;

		if(cctype == oenum) {
			n->enumptr = fdbfs_find_enumhead_by_name(enumhead, cenumname);
			if(n->enumptr == NULL) {
				CERR(die, "Cannot find enum named %s! ", cenumname);
				free(cenumname);
				fdbfs_free_cat_elem_list(h != NULL ? h : n);
				sqlite3_finalize(cst);
				return NULL;
			}
		} else if(cctype == oenumsub) {
			n->subcatel = fdbfs_find_catelem_by_name(h, cenumname);
			if(n->subcatel == NULL) {
				CERR(die, "Cannot find cat elem for sub named %s! ", cenumname);
				free(cenumname);
				fdbfs_free_cat_elem_list(h != NULL ? h : n);
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


actcat_t* fdbfs_catalogues_from_db(f, cathead)
	fdbfs_t *f;
	struct CatalogueHead *cathead;
{
	const char *csql = "SELECT name,alias,field_desc_table FROM cat_list;";
	int rc;
	sqlite3_stmt *cst;
	const char *cn;
	actcat_t *h = NULL, *n = NULL;

	if((rc = sqlite3_prepare(f->db, csql, strlen(csql), &cst, NULL))  != SQLITE_OK) {
		ERR(die, "error in cataloguess_from_db: SQLite said %s", sqlite3_errmsg(f->db));
		return NULL;
	}

	while((rc = sqlite3_step(cst)) == SQLITE_ROW) {
		n = allocz(sizeof(*n));
		n->name = strdup((const char*)sqlite3_column_text(cst, 0));
		n->alias = strdup((const char*)sqlite3_column_text(cst, 1));
		cn = (const char*)sqlite3_column_text(cst, 2);
		if(strlen(cn) < 5 /* cft_ */) {
			ERR(die, "invalid field_desc_table '%s'", cn);
cherr:
			n->next = h;
			fdbfs_actcats_free(n);
			sqlite3_finalize(cst);
			return NULL;
		}
		n->def = fdbfs_find_cathead_by_name(cathead, (char*)(cn + 4));
		if(n->def == NULL) {
			ERR(die, "no such CFD '%s'", cn + 4);
			goto cherr; /* BASIC!!! */
		}
		n->next = h;
		h = n;
	}

	sqlite3_finalize(cst);

	return h;
}
			
struct CatalogueHead* fdbfs_cats_from_db(f, enumhead)
	fdbfs_t *f;
	struct EnumHead* enumhead;
{
	const char *catlistsql = "SELECT * FROM cfd_list;";
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
		catname = strdup((const char*)sqlite3_column_text(cst, 1));
		catalias = strdup((const char*)sqlite3_column_text(cst, 2));
		tabledef = strdup((const char*)sqlite3_column_text(cst, 3));
		n = allocz(sizeof(*n));
		n->name = catname;
		n->fmtname = catalias; /* god only knows why I use such liberal naming of
					  alias and fmtname fields. Maybe I was drunk. */
		n->headelem = fdbfs_catelems_from_dbtab(f, tabledef, enumhead);
		n->flags |= CATH_FROM_DB;

		free(tabledef);

		if(h == NULL) {
			h = c = n;
		} else {
			c->next = n;
			c = c->next;
		}

		if(n->headelem == NULL) {
			fdbfs_free_cat_head_list(h);
			sqlite3_finalize(cst);
			SCERR(die, "cats_from_db: from dbtab error. ");
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

int fdbfs_db_rm_catalogue(f, catname)
	fdbfs_t *f;
	const char *catname;
{
	char *ttbl;
	size_t ttl;
	char *tcfd;

	if(!fdbfs_db_cat_getcfdname(f, catname, &tcfd)) {
		SCERR(die, "rm_catalogue: SQLite error getting cfdname. ");
		return 0;
	}
	if(!fdbfs_db_delete(f, "cat_list", "name", "==", catname)) {
		SCERR(die, "rm_catalogue: delete error");
		free(tcfd);
		return 0;
	}
	ttl = strlen(catname) + sizeof("c_") + 1;
	ttbl = malloc(ttl);
	strlcpy(ttbl, "c_", ttl);
	strlcat(ttbl, catname, ttl);
	if(!fdbfs_db_drop_table(f, ttbl)) {
		SCERR(die, "rm_catalogue: table drop error");
		free(ttbl);
		free(tcfd);
		return 0;
	}
	if(!fdbfs_db_cfd_update_refcount(f, tcfd, 0, 1)) {
		SCERR(die, "rm_catalogue: error updating refcount. ");
		free(ttbl);
		free(tcfd);
	}
#if 0
	strlcpy(ttbl, "cft_", ttl);
	strlcat(ttbl, catname, ttl);
	if(!fdbfs_db_drop_table(f, ttbl)) {
		SCERR(die, "rm_catalogue: cft table drop error");
		free(ttbl);
		return 0;
	}
#endif
	free(tcfd);
	free(ttbl);

	return 1;
}

int fdbfs_dbspec_parse(f, filename)
	fdbfs_t *f;
	const char *filename;
{
	Toke tz;
	int rc;
	FILE *tf;
	Heads h;
	void *parser;
	
	memset(&tz, 0, sizeof(tz));
	memset(&h, 0, sizeof(h));
	h.instance = f;
	h.db_enumh = fdbfs_enums_from_db(f);
	if(h.db_enumh == NULL && f->error.emsg != NULL)
		return SCERR(die, "parse_definition: enum importation failed. ");
	h.db_cath = fdbfs_cats_from_db(f, h.db_enumh); /* XXX: this is inefficient; just have read_specs_from_db be called once and use that */
	if(h.db_cath == NULL && f->error.emsg != NULL)
		return SCERR(die, "parse_definition: catalogue importation failed. ");
	if(strcmp(filename, "-") == 0)
		tf = stdin;
	else
		tf = fopen(filename, "r");
	if(!tf)
		return ERR(die, "parse_definition: opening %s: %s", filename, strerror(errno));
	parser = DSpecParseAlloc(malloc);
	fdbfs_dsin = tf;
#ifdef PARSERDEBUG
	DSpecParseTrace(stderr, "PARSER: ");
#endif
	while((rc = fdbfs_dslex()) != 0) {
		tz.num = yylval.number;
		tz.str = yylval.string;
#ifdef PARSERDEBUG
		fprintf(stderr, "rc == %d\n", rc);
#endif
		DSpecParse(parser, rc, tz, &h);
		if(h.err || f->error.emsg != NULL) {
			DSpecParseFree(parser, free);
			fdbfs_free_head_members(&h);
			CERR(die, "parse_definition(f, \"%s\"): error after DSpecParse(). ", filename);
			fclose(tf);
			return 0;
		}
	}
	DSpecParse(parser, 0, tz, &h);
	DSpecParseFree(parser, free);
	fdbfs_free_enum_head_list(h.db_enumh);
	fdbfs_free_cat_head_list(h.db_cath);
	fclose(tf);
	return make_tables_from_spec(f, filename, &h);
}

void fdbfs_dswrap(void)
{
	yywrap();
}

int fdbfs_cat_type_exists(f, ct)
	fdbfs_t *f;
	const char *ct;
{
	if(fdbfs_find_cathead_by_name(f->heads.db_cath, ct))
		return 1;
	else
		return 0;
	/* NOTREACHED */
}

actcat_t* fdbfs_find_catalogue(f, name)
	fdbfs_t *f;
	const char *name;
{
	actcat_t *c;
	if(f->catsh == NULL) return NULL;
	if(name == NULL)
		return NULL;
	for(c = f->catsh; c != NULL; c = c->next) {
		if(c->name != NULL)
			if(strcasecmp(name, c->name) == 0)
				return c;
	}
	return NULL;
}

#if defined(MACOSX)
int yylex(void)
{
	return fdbfs_dslex();
}
#endif
