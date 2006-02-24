/*
 * Copyright (c) 2005-2006, Dan Ponte
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
/* $Amigan: fakedbfs/libfakedbfs/indexing.c,v 1.53 2006/02/24 17:56:19 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

#ifdef UNIX
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <dirent.h>
#	include <fts.h>
#	include <sys/param.h>
#endif
/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs/fdbfsregex.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/plugins.h>
#include <fakedbfs/debug.h>
#include <fakedbfs/db.h>
#include <fakedbfs/dbspecdata.h>
#include <fakedbfs/fields.h>
#include <fakedbfs/indexing.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/indexing.c,v 1.53 2006/02/24 17:56:19 dcp1990 Exp $")

static int add_file(f, file, catalogue, fields)
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
		if(c->fieldname == NULL) {
			free(tablename);
			free(fieldtable);
			return SERR(die, "fieldname was null! Perhaps a misbehaving plugin?");
		}
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
	
	sql = sqlite3_mprintf("INSERT OR REPLACE INTO %s (file, lastupdate, ctime %s) VALUES('%q', %d, %d %s);",
			tablename, sqlkeys, file, time(NULL), time(NULL), sqlvals);
#ifdef INDEX_SQL_DEBUG
	printf("SQL in indexer: %s\n", sql);
#endif
	rc = sqlite3_prepare(f->db, sql, strlen(sql), &stmt, &tail);

	sqlite3_free(sql);
	free(sqlkeys);
	free(sqlvals);
	free(tablename);
	free(fieldtable);

	if(rc != SQLITE_OK) {
		return ERR(die, "add_file(\"%s\"): SQLite error after prepare: %s", file, sqlite3_errmsg(f->db));
	}


	for(c = fields; c != NULL; c = c->next) {
#ifdef INDEX_SQL_DEBUG
		printf("binding field '%s' val ", c->fieldname);
#endif
		if(!fdbfs_db_bind_field(f, &fieldcount, c->type, c->val, c->len, stmt))
			return CERR(die, "add_file(\"%s\"): bind error. ", file);
	}

	rc = sqlite3_step(stmt);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		return ERR(die, "add_file(\"%s\"): SQLite error after step: %s", file, sqlite3_errmsg(f->db));
	}

	sqlite3_finalize(stmt);
	return 1;
}

static fields_t* fill_in_fields(f, filename)
	fdbfs_t *f;
	char *filename;
{
	struct Plugin *h = f->plugins, *c = NULL, *tpl = NULL;
	fields_t *fh = NULL;
	char *errmsg = NULL;
	char *extension;
	short int found = 0;
	int rc;

#if defined(UNIX) || defined(AMIGA) || defined(WIN32)
	extension = strrchr(filename, '/');
	extension = strrchr(extension == NULL ? filename : extension + 1, '.');
#endif

	if(extension != NULL) {
		++extension;
		if(strlen(extension) > 7 /* reasonable enough */ || strlen(extension) == 0) {
			extension = NULL;
		}
	}

	if(extension != NULL)
		for(c = h; c != NULL; c = c->next) {
			if(strstr(c->info->extensions, extension) != NULL) {
				rc = c->check_file(f, filename, &errmsg);
				if(rc == 1) {
					tpl = c;
					found = 1;
					break;
				} else if(rc == -1 || errmsg != NULL) {
					CERR(die, "check_file for plugin %s said: %s", c->info->pluginname, errmsg);
					free(errmsg);
					return NULL;
				}
			}
		}

	if(!found)
		for(c = h; c != NULL; c = c->next) {
			rc = c->check_file(f, filename, &errmsg);
			if(rc == 1) {
				tpl = c;
				found = 1;
				break;
			} else if(rc == -1 || errmsg != NULL) {
				CERR(die, "check_file for plugin %s said: %s", c->info->pluginname, errmsg);
				free(errmsg);
				return NULL;
			}
		}

	if(!found) {
#ifdef PLUGINS_VERBOSE
		fdbfs_debug_info(f, error, "cannot find plugin to extract metadata for %s!", filename);
#endif
		return NULL;
	}

	fh = tpl->extract_from_file(f, filename, &errmsg);
	if(errmsg != NULL) {
		CERR(die, "extract_from_file for plugin %s said: %s", c->info->pluginname, errmsg);
		free(errmsg);
		return NULL;
	}

	return fh;
}

static void list_subenum(h)
	struct EnumSubElem *h;
{
	struct EnumSubElem *c;

	printf("Enum sub of '%s'\n", h->father->fmtname);

	for(c = h; c != NULL; c = c->next) {
		printf("%d - '%s' %s%s", c->value, (c->flags & SUBE_IS_SELF ? h->father->fmtname : c->name), (c->value >= ALLSUBSTART ? "(allsub)" : ""),
				(c->flags & SUBE_IS_SELF ? " (self)" : ""));
		if((c = c->next) == NULL) { /* next for columns */
			printf("\n");
			break;
		}
		printf("\t\t%d - '%s' %s%s\n", c->value, (c->flags & SUBE_IS_SELF ? h->father->fmtname : c->name), (c->value >= ALLSUBSTART ? "(allsub)" : ""),
				(c->flags & SUBE_IS_SELF ? " (self)" : ""));
	}
}

static void list_enum(h)
	struct EnumHead *h;
{
	struct EnumElem *c;

	printf("Enum '%s'\n", h->name);

	for(c = h->headelem; c != NULL; c = c->next) {
		printf("%d - '%s' %s", c->value, c->fmtname, (c->other ? "(other)" : ""));
		if((c = c->next) == NULL) { /* next for columns */
			printf("\n");
			break;
		}
		printf("\t\t%d - '%s' %s\n", c->value, c->fmtname, (c->other ? "(other)" : ""));
	}
}

answer_t* fdbfs_askfunc_std(buf, def, fieldname, name, filen, dt, ehead, subhead)
	answer_t *buf;
	answer_t *def;
	char *fieldname;
	char *name;
	char *filen;
	enum DataType dt;
	struct EnumHead *ehead;
	struct EnumSubElem *subhead;
{
#define MAXLINELEN 512
	char bf[MAXLINELEN];
	char *nl;
	char *es;

	if(fieldname == NULL)
		return NULL;

checkagain:
	switch(dt) {
		case number:
		case character:
		case datime: /* fugly; oh well */
		case usnumber:
			printf("%s [%d]> ", fieldname, def->ad.integer);
			break;
		case boolean:
			printf("%s [%s]> ", fieldname, (def->ad.integer ? "true" : "false"));
			break;
		case oenum:
			printf("%s [%d - '%s'] (? lists)> ", fieldname, def->ad.integer, fdbfs_get_enum_string_by_value(ehead->headelem, def->ad.integer, 1));
			break;
		case oenumsub:
			es =  fdbfs_get_enum_sub_string_by_value(subhead, def->ad.integer);
			if(es == NULL)
				return (answer_t*)0x1;
			printf("%s sub [%d - '%s'] (? lists)> ", fieldname, def->ad.integer, es);
			break;
		case string:
			printf("%s ['%s']> ", fieldname, def->ad.string);
			break;
		case fp:
			printf("%s [%f]> ", fieldname, def->ad.fp);
			break;
		case image:
		case binary:
			return 0; /* we can't deal here */
			break;
	}

	fgets(bf, MAXLINELEN, stdin);

	if(*bf == '\n')
		return (answer_t*)0x1;

	if((nl = strrchr(bf, '\n')) != NULL)
		*nl = '\0';

	if(*bf == '?' && dt == oenum) {
		list_enum(ehead);
		goto checkagain;
	} else if(*bf == '?' && dt == oenumsub) {
		list_subenum(subhead);
		goto checkagain;
	}

	switch(dt) {
		case number:
		case datime:
		case usnumber: /* incorrect! oh well...shuts the compiler up. we don't use usnumber yet anyway */
		case character:
			buf->ad.integer = atoi(bf);
			break;
		case boolean:
			if(*bf == 't')
				buf->ad.integer = 1;
			else if(*bf == 'f')
				buf->ad.integer = 0;
			else
				goto checkagain; /* it isn't BASIC, folks */
			break;
		case oenum:
		case oenumsub:
			buf->ad.integer = atoi(bf);
			break;
		case string:
			buf->ad.string = strdup(bf);
			break;
		case fp:
			buf->ad.fp = strtod(bf, NULL);
			break;
		case image:
		case binary:
			return 0; /* I told you already */
	}

	return buf;
}

fields_t* fdbfs_find_field_by_name(h, name)
	fields_t *h;
	char *name;
{
	fields_t *c = NULL;
	
	for(c = h; c != NULL; c = c->next) {
		if(c->fieldname != NULL)
			if(strcmp(c->fieldname, name) == 0)
				return c;
	}
	
	return NULL;
}

/* Basically, what this [should] do:
 * Go through and get each field for the catalogue from the database and put
 * it into an appropriate tree (this should be done already; see the
 * cats_from_db() and enums_from_db() routines).
 * Step through these fields, searching the defs list each time for the current
 * element; use its value as the default.
 * Call the askfunc. If it's an enum, give it the pointer to the EnumHead, and
 * the subelem if it's a sub. For all other fields, just give a "none"-equiv.
 * default.
 * Have fun with this inefficient, New Age-esque code (though at least it's not
 * in programming language del giorno)
 *
 * Do an initial call of askfunc with fieldname
 * as NULL. If you get an 0x1 back (instead of NULL), this means not to expect anything but (answer_t*)0x1
 * back from them. At the end of the asking, do another call with fieldname as (char*)0x1.
 * Then loop through again, as usual. You should now get back peoperly-filled in answer_ts.
 * This is for apps that might want to fashion a dialogue during asking. Or we could just use a flag in the
 * fdbfs_t* that tells us whether to do this or not :).
 */
static fields_t* ask_for_fields(f, filen, cat, defs) /* this routine is extremely inefficient
					  ....I think. But it sure as hell
					 isn't the worst. */
	fdbfs_t *f;
	char *filen;
	char *cat;
	fields_t *defs;
{
	answer_t *cans;
	answer_t def;
	answer_t cta;
	short int dialoguemode = 0;
	short int i, hasoth = 0; /* inaccurate var names */
	short int otherm = 0;
	int zero = 0;
	fields_t *c = NULL /*, *h = NULL, *n = NULL*/;

	cans = f->askfieldfunc(NULL, NULL, NULL, NULL, NULL, 0x0, NULL, NULL);
	if(cans == (answer_t*)0x1)
		dialoguemode = 1;
	else if(cans == (answer_t*)-1) {
		SERR(die, "askfieldfunc initial gave error.");
		return NULL;
	}

#define cval (hasoth ? c->otherval : (c->val != NULL ? c->val : &zero))
#define clen (hasoth ? c->othlen : c->len)
#define ctype (hasoth ? c->othtype : c->type)
	if(dialoguemode) {
		for(c = defs; c != NULL; c = c->next) {
			if(c->otherval != NULL)
				otherm = 1;
			else
				otherm = 0;

			for(i = 0; i < (otherm ? 2 : 1); i++) {
				if(i == 1)
					hasoth = i;
				else
					hasoth = 0;
				memset(&def, 0, sizeof(def));
				def.dt = ctype;
				switch(c->type) {
					case number:
					case datime:
					case usnumber:
					case boolean:
					case oenum:
					case oenumsub:
					case character:
						def.ad.integer = *(int*)cval;
						break;
					case string:
						def.ad.string = (char*)cval;
						break;
					case fp:
						def.ad.fp = *(FLOATTYPE *)cval;
						break;
					case image:
					case binary:
						def.ad.vd = cval;
						def.len = clen;
						break;
				}


				cans = f->askfieldfunc(&cta, &def, c->fmtname, c->fieldname, filen, def.dt, c->ehead, c->subhead /* XXX: this is useless; the app must
																    look up the value of the associated enum.
																    we might provide functions to do this in the
																    future */);
				if(cans == (answer_t*)-1) {
					SERR(die, "askfunc said error here (dialoguemode).");
					/* clean up */
					return NULL;
				}
			}
		}

		cans = f->askfieldfunc(NULL, NULL, NULL, (char*)0x1 /* before real */, NULL, 0x0, NULL, NULL);
		if(cans == (answer_t*)-1) {
				SERR(die, "askfunc said error here (dialoguemode - before real).");
				/* clean up */
				return NULL;
		}
	}

	for(c = defs; c != NULL; c = c->next) {
		if(c->otherval != NULL)
			otherm = 1;
		else
			otherm = 0;

		for(i = 0; i < (hasoth ? 2 : 1); i++) {
			if(i == 1)
				hasoth = i;
			else
				hasoth = 0;

			memset(&def, 0, sizeof(def));
			def.dt = ctype;
			switch((hasoth ? c->othtype : c->type)) {
				case number:
				case datime:
				case usnumber:
				case boolean:
				case oenum:
				case oenumsub:
					def.ad.integer = *(int*)cval;
					break;
				case string:
					def.ad.string = (char*)cval;
					break;
				case fp:
					def.ad.fp = *(FLOATTYPE *)cval;
					break;
				case image:
				case binary:
					def.ad.vd = cval;
					def.len = clen;
					break;
			}

			if(c->type == oenumsub) {
				c->subhead = fdbfs_get_subhead_by_enval(c->subparent->ehead->headelem, *(unsigned int*)c->subparent->val);
			}

			cans = f->askfieldfunc(&cta, &def, c->fmtname, c->fieldname, filen, def.dt, c->ehead, c->subhead);
			if(cans == (answer_t*)-1) {
				SERR(die, "askfunc said error here.");
				/* clean up */
				return NULL;
			} else if(cans == (answer_t*)0x1) {
				/* no change... */
			} else if(cans == &cta) {
				/* changed... */
				free(cval);
				switch(ctype) {
					case oenum:
					case oenumsub:
					case number:
					case datime:
					case usnumber:
					case boolean:
						if(hasoth) {
							if(c->otherval != NULL)
								free(c->otherval);
							c->otherval = malloc(sizeof(int));
							*(int*)c->otherval = cta.ad.integer;
						} else {
							hasoth = 0;
							if(c->val != NULL)
								free(c->val);
							c->val = malloc(sizeof(int));
							*(int*)c->val = cta.ad.integer;
						}
						break;
					case string:
						if(c->val != NULL)
							free(c->val);
						if(hasoth)
							c->otherval = strdup(cta.ad.string);
						else
							c->val = strdup(cta.ad.string);
						break;
					case fp:
						if(hasoth) {
							if(c->otherval != NULL)
								free(c->otherval);
							c->otherval = malloc(sizeof(FLOATTYPE));
							*(FLOATTYPE*)c->otherval = cta.ad.fp;
						} else {
							if(c->val != NULL)
								free(c->val);
							c->val = malloc(sizeof(FLOATTYPE));
							*(FLOATTYPE*)c->val = cta.ad.fp;
						}
						break;
					case image:
					case binary:
						if(!hasoth) {
							if(c->val != NULL)
								free(c->val);
							c->val = malloc(cta.len);
							memcpy(c->val, cta.ad.vd, cta.len);
						} else {
							if(c->otherval != NULL)
								free(c->otherval);
							c->otherval = malloc(cta.len);
							memcpy(c->otherval, cta.ad.vd, cta.len);
						}
						break;
				}
			}
		}
	}

	return defs;
}


fields_t* fdbfs_find_field_by_ehead(h, e)
	fields_t *h;
	struct EnumHead *e;
{
	fields_t *c;

	for(c = h; c != NULL; c = c->next) {
		if(c->ehead == e)
			return c;
	}

	return NULL;
}

fields_t* fdbfs_find_field_by_ename(h, e)
	fields_t *h;
	char *e;
{
	fields_t *c;

	for(c = h; c != NULL; c = c->next) {
		if(h->ehead != NULL)
			if(strcmp(e, h->ehead->name) == 0)
				return c;
	}

	return NULL;
}

static int complete_fields_from_db(f, cat, h)
	fdbfs_t *f;
	char *cat;
	fields_t **h;
{
	fields_t *c = NULL, *last = NULL, *new = NULL;
	struct CatElem *cc = NULL, *ch = NULL;
	struct CatalogueHead *hch = NULL, *hhh = NULL;

	if(*h == NULL)
		last = NULL;
	else {
		for(c = *h; c->next != NULL; c = c->next) ;
		last = c;
	}

	hhh = f->heads.db_cath;

	if(hhh == NULL) {
		SERR(die, "list not filled in from DB! call read_specs_from_db() first.");
		return 0;
	}
	
	for(hch = hhh; hch != NULL; hch = hch->next) {
		if(strcmp(hch->name, cat) == 0) {
			ch = hch->headelem;
		}
	}
	
	if(ch == NULL) {
		ERR(die, "couldn't find catalogue %s in list!", cat);
		return 0;
	}

	for(cc = ch; cc != NULL; cc = cc->next) {
		c = fdbfs_find_field_by_name(*h, cc->name);
		if(c == NULL) {
			new = allocz(sizeof(*new));
			new->fieldname = strdup(cc->name);
			new->fmtname = strdup(cc->alias);
			new->type = cc->type;
			/* this seems like a memory leak waiting to happen...make it null and add some logic
			 * to the field filling in code:
				new->val = allocz(sizeof(int));
				*(int*)new->val = 0;
			*/
			new->val = allocz(sizeof(int)); /* wasteful, but prevents BS from happening during indexing in interactive mode */
			new->len = 1;
			if(new->type == oenum) {
				new->ehead = cc->enumptr;
			}
			if(new->type == oenum && cc->enumptr->otherelem != NULL) {
				new->othtype = cc->enumptr->otherelem->othertype;
				new->otherval = allocz(sizeof(int));
				new->val = allocz(sizeof(int));
				*(int*)new->val = 0;
				new->othlen = 1;
			}
			if(new->type == oenumsub) {
				new->ehead = cc->subcatel->enumptr;
				new->subparent = fdbfs_find_field_by_ehead(*h, new->ehead);
				/* new->subhead = cc->subcatel->enumptr->headelem->subhead; */
			}
			if(last != NULL) {
				last->next = new;
				last = new;
			} else {
				last = new;
				*h = last;
			}
		}
	}

	return 1;
}

static void erase_fields(h, fields, oh)
	fields_t **h;
	fields_t *fields;
	fields_t *oh;
{
	fields_t *c;

	if(fields != NULL) /* this is to remove fields from the list, since it's the caller's responsibility to free their own defaults */
		for(c = *h; c != NULL; c = c->next) {
			if(c->flags & FIELDS_FLAG_LASTDEF) {
				c->next = NULL;
				c->flags &= ~FIELDS_FLAG_LASTDEF;
				*h = oh;
				break;
			}
		}
}

static int file_has_changed(f, cat, filename, statstruct)
	fdbfs_t *f;
	char *cat;
	char *filename;
	void *statstruct; /* we cast later; this is for portability */
{
#if defined(UNIX)
	struct stat s;
#endif
	int rc;
	time_t upd; /* time_t must be signed!!!!! */
	
	upd = (time_t)fdbfs_db_get_lastupdate(f, cat, filename);

	if(upd == -1) { /*error*/
		SCERR(die, "get_lastupdate errored. ");
		return -1;
	} else if(upd == -2) {
		return 1;
	}
#if defined(UNIX)
	if(statstruct == NULL) {
		rc = stat(filename, &s);
		if(rc == -1) {
			ERR(die, "stat failed: %s", strerror(errno));
			return -1;
		}
	} else {
		memcpy(&s, statstruct, sizeof(s));
	}

	if(S_ISDIR(s.st_mode)) {
		return -2; /* a directory */
	}

	if(s.st_mtime > upd)
		return 1;
	else
		return 0;
#else
#	error "No code to stat for mtime under this OS"
#endif

	return 0;
}

int fdbfs_index_file(f, filename, cat, batch, useplugs, forceupdate, fields)
	fdbfs_t *f;
	char *filename;
	char *cat;
	int batch;
	int useplugs;
	int forceupdate;
	fields_t *fields;
{
	int rc = 0;
	fields_t *c = NULL, *h = NULL, *oh = NULL;

	if(rc != 2) {
		rc = file_has_changed(f, cat, filename, NULL);
		if(rc == 0) {
			if(!forceupdate)
				return 1;
		} else if(rc == -2) { /* is a directory */
			fdbfs_debug_info(f, warning, "in index_file: %s is a directory", filename);
		} else if(rc == -1) { /* hack alert */
			fdbfs_debug_info(f, warning, "error checking if %s changed: %s", filename, f->error.emsg);
			fdbfs_estr_free(&f->error);
		}
	}

	if(useplugs) {
		h = fill_in_fields(f, filename);
		if(h == NULL && f->error.emsg != NULL) {
			return 0;
		}
	}

	if(fields != NULL) {
		/* when we free our field list after this, we MUSTN'T free fields! */
		if(h != NULL) {
			for(c = fields; c->next /* we want to use it */ != NULL;
					c = c->next);
			c->next = h;
			c->flags |= FIELDS_FLAG_LASTDEF;
			oh = h;
			h = fields;
		} else {
			for(c = fields; c->next /* we want to use it */ != NULL;
					c = c->next);
			c->flags |= FIELDS_FLAG_LASTDEF;
			h = fields;
		}
	}

	if(!batch) {
		rc = complete_fields_from_db(f, cat, &h); /* fill out any others so the askfunc will ask the user */

		for(c = h; c != NULL; c = c->next) {
			if(c->flags & FIELDS_FLAG_LASTDEF)
				oh = c->next;
		}
		
		if(!rc) {
			SCERR(die, "complete fields failed. ");
			erase_fields(&h, fields, oh);
			fdbfs_free_field_list(h);
			return 0;
		}

		ask_for_fields(f, filename, cat, h); /* we can only change existing stuff now anyway...this appends to h for us */
	}	

	rc = add_file(f, filename, cat, h);
	if(!rc) {
		CERR(die, "error indexing file %s. ", filename);
		erase_fields(&h, fields, oh);
		fdbfs_free_field_list(h);
		return 0;
	}
	
	erase_fields(&h, fields, oh);
	fdbfs_free_field_list(h);

	return 1;
}

#if defined(UNIX)
/* all FTS stuff was heavily inspired by FreeBSD's /usr/src/bin/ls/ls.c */
int fdbfs_cindexer_dir(f, cat, batch, useplugs, list, options, re, defs) /* this has no prototype in fakedbfs.h; it's less ugly this way. */
		fdbfs_t *f;
		char *cat;
		int batch;
		int useplugs;
		FTSENT *list;
		int options;
		freg_t *re;
		fields_t *defs;
{
	int rc;
	FTSENT *c;
	char *fpth;
	char respath[PATH_MAX];
	size_t fplen;

	for(c = list; c; c = c->fts_link) {
		if(c->fts_info == FTS_ERR || c->fts_info == FTS_NS) {
			fdbfs_debug_info(f, error, "error %s: %s", c->fts_name, strerror(c->fts_errno));
			c->fts_number = 1;
			continue;
		}
		if(c->fts_info == FTS_D) /* no directories */
			continue;
		if(re != NULL) {
			rc = fregexec(re, c->fts_name, NULL, 0);
			if(rc == FREG_NOMATCH) {
				continue;
			} else if(rc != 0) {
				ERR(die, "index_dir: error executing regex: %d - %s", rc, re->errmsg);
				frerrfree(re);
				return 0;
			}


			
#if 0
			fplen = strlen(c->fts_path) + 1 /* null */ + strlen(c->fts_name);
#endif
			fplen = c->fts_pathlen + 1 + c->fts_namelen;
			fpth = malloc(fplen * sizeof(char));
			strlcpy(fpth, c->fts_path, fplen);
			strlcat(fpth, c->fts_name, fplen);
			if(realpath(fpth, respath) != NULL) {
				free(fpth);
				fpth = strdup(respath);
			}
#if 0
			fpth = strdup(c->fts_name);
#endif

			rc = file_has_changed(f, cat, fpth, c->fts_statp);
			if(rc == 0) {
				free(fpth);
				continue;
			} else if(rc == 1) {
				fdbfs_index_file(f, fpth, cat, batch, useplugs, 2, defs);
				free(fpth);
			} else if(rc == -2) { /* is a directory */
				/* this does nothing; all directories are handled by index_dir() */
				free(fpth);
			} else if(rc == -1) { /* hack alert */
				fdbfs_debug_info(f, warning, "error checking if %s changed: %s", fpth, f->error.emsg);
				fdbfs_estr_free(&f->error);
				free(fpth);
			} else
				free(fpth);
		} else {
#if 0
			printf("path == '%s', name == '%s', accpath == '%s'\n", c->fts_path, c->fts_name, c->fts_accpath);
#endif
			fplen = c->fts_pathlen + 1 + c->fts_namelen;
			fpth = malloc(fplen * sizeof(char));
			strlcpy(fpth, c->fts_path, fplen);
			strlcat(fpth, c->fts_name, fplen);
			if(realpath(fpth, respath) != NULL) {
				free(fpth);
				fpth = strdup(respath);
			}

			rc = file_has_changed(f, cat, fpth, c->fts_statp);
			if(rc == 0) {
				free(fpth);
				continue;
			} else if(rc == 1) {
				fdbfs_index_file(f, fpth, cat, batch, useplugs, 2, defs);
				free(fpth);
			} else if(rc == -2) { /* is a directory */
				free(fpth);
			} else if(rc == -1) { /* hack alert */
				fdbfs_debug_info(f, warning, "error checking if %s changed: %s", fpth, f->error.emsg);
				fdbfs_estr_free(&f->error);
				free(fpth);
			}
		}
	}

	return 1;
}
#endif

	
int fdbfs_index_dir(f, dirs, cat, useplugs, batch, nocase, re, recurse, defs)
	fdbfs_t *f;
	char **dirs;
	char *cat;
	int useplugs;
	int batch;
	int nocase;
	char *re;
	int recurse;
	fields_t *defs;
{
#if defined(UNIX)
	FTS *fpt;
	FTSENT *p, *chp;
	char emsg[128];
	int rc = 1;
	freg_t *tre = NULL;

	if(re != NULL) {
		tre = new_freg(emsg, sizeof(emsg));
		if(tre == NULL) {
			ERR(die, "index_dir: error allocating regex: %s", emsg);
			return 0;
		}
		if((rc = fregcomp(tre, re, (nocase ? FREG_NOCASE : 0) | FREG_NOSUB)) != 0) {
			ERR(die, "index_dir: error compiling regex (rc %d) '%s': %s", rc, re, tre->errmsg);
			destroy_freg(tre);
			return 0;
		}
	}

	rc = 1;

	fpt = fts_open(dirs, FTS_NOCHDIR /* required to be thread safe...if we chdir(2) and another thread needs to do something, we're fucked */, NULL);

	chp = fts_children(fpt, 0);
	if (chp != NULL)
		rc = fdbfs_cindexer_dir(f, cat, batch, useplugs, chp, 0, re != NULL ? tre : NULL, defs);

	if(f == NULL) {
		ERR(die, "index_dir: cannot open director{y,ies} because of %s", strerror(errno));
		destroy_freg(tre);
		return 0;
	}

	while((p = fts_read(fpt)) != NULL && rc) {
		switch(p->fts_info) {
			case FTS_DC:
				/* cycle... */
				break;
			case FTS_DNR:
			case FTS_ERR:
				fdbfs_debug_info(f, error, "%s: %s", p->fts_name, strerror(p->fts_errno));
				break;
			case FTS_D:
				/* directory */
				chp = fts_children(fpt, 0);
				rc = fdbfs_cindexer_dir(f, cat, batch, useplugs, chp, 0, re != NULL ? tre : NULL, defs);

				if(!rc)
					break;

				if(!recurse && chp != NULL)
					fts_set(fpt, p, FTS_SKIP);
				break;
			default:
				break;
		}
	}

	fts_close(fpt);
	if(re != NULL)
		destroy_freg(tre);
#else
#	error "Code not written for index_dir under this OS"
#endif
	return rc;
}

