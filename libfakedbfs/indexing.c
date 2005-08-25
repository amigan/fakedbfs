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
/* $Amigan: fakedbfs/libfakedbfs/indexing.c,v 1.18 2005/08/25 16:55:37 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>

#ifdef UNIX
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <dirent.h>
#endif
/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/indexing.c,v 1.18 2005/08/25 16:55:37 dcp1990 Exp $")

int add_file(f, file, catalogue, fields)
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
			return ERR(die, "fieldname was null! Perhaps a misbehaving plugin?", NULL);
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
	
	sql = sqlite3_mprintf("INSERT OR REPLACE INTO %s (file, lastupdate %s) VALUES('%q', %d %s);",
			tablename, sqlkeys, file, time(NULL), sqlvals);
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
		if(!bind_field(f, &fieldcount, c->type, c->val, c->len, stmt))
			return CERR(die, "add_file(\"%s\"): bind error. ", file);
	}

	rc = sqlite3_step(stmt);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		return ERR(die, "add_file(\"%s\"): SQLite error after step: %s", file, sqlite3_errmsg(f->db));
	}

	sqlite3_finalize(stmt);
	return 1;
}

fields_t* fill_in_fields(f, filename)
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
				rc = c->check_file(filename, &errmsg);
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
			rc = c->check_file(filename, &errmsg);
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
		debug_info(f, error, "cannot find plugin to extract metadata for %s!", filename);
#endif
		return NULL;
	}

	fh = tpl->extract_from_file(filename, &errmsg);
	if(errmsg != NULL) {
		CERR(die, "extract_from_file for plugin %s said: %s", c->info->pluginname, errmsg);
		free(errmsg);
		return NULL;
	}

	return fh;
}

answer_t* askfunc_std(buf, def, fieldname, name, filen, dt, ehead, subhead)
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
			printf("%s [%d]> ", fieldname, def->integer);
			break;
		case boolean:
			printf("%s [%s]> ", fieldname, (def->integer ? "true" : "false"));
			break;
		case oenum:
			printf("%s [%d - '%s']> ", fieldname, def->integer, get_enum_string_by_value(ehead->headelem, def->integer, 1));
			break;
		case oenumsub:
			es =  get_enum_sub_string_by_value(subhead, def->integer);
			printf("%s sub [%d - '%s']> ", fieldname, def->integer, (es == NULL ? "(none)" : es));
			break;
		case string:
			printf("%s ['%s']> ", fieldname, def->string);
			break;
		case fp:
			printf("%s [%f]> ", fieldname, def->fp);
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

	switch(dt) {
		case number:
			buf->integer = atoi(bf);
			break;
		case boolean:
			if(*bf == 't')
				buf->integer = 1;
			else if(*bf == 'f')
				buf->integer = 0;
			else
				goto checkagain; /* it isn't BASIC, folks */
			break;
		case oenum:
		case oenumsub:
			/* TODO: make this, well, better... */
			buf->integer = atoi(bf);
			break;
		case string:
			buf->string = strdup(bf);
			break;
		case fp:
			sscanf(bf, "%f", &buf->fp);
			break;
		case image:
		case binary:
			return 0; /* I told you already */
	}

	return buf;
}

fields_t* find_field_by_name(h, name)
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
fields_t* ask_for_fields(f, filen, cat, defs) /* this routine is extremely inefficient
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
	short int otherm = 0;
	short int i, hasoth = 0;
	fields_t *c = NULL /*, *h = NULL, *n = NULL*/;

	cans = f->askfieldfunc(NULL, NULL, NULL, NULL, NULL, 0x0, NULL, NULL);
	if(cans == (answer_t*)0x1)
		dialoguemode = 1;
	else if(cans == (answer_t*)-1) {
		ERR(die, "askfieldfunc initial gave error.", NULL);
		return NULL;
	}

#define cval (otherm ? c->otherval : c->val)
#define clen (otherm ? c->othlen : c->len)
#define ctype (otherm ? c->othtype : c->type)
	if(dialoguemode) {
		for(c = defs; c != NULL; c = c->next) {
			if(c->otherval != NULL)
				hasoth = 1;
			else
				hasoth = 0;

			for(i = 0; i < (hasoth ? 2 : 1); i++) {
				memset(&def, 0, sizeof(def));
				def.dt = ctype;
				switch(c->type) {
					case number:
					case boolean:
					case oenum:
					case oenumsub:
						def.integer = *(int*)cval;
						break;
					case string:
						def.string = (char*)cval;
						break;
					case fp:
						def.fp = *(FLOATTYPE *)cval;
						break;
					case image:
					case binary:
						def.vd = cval;
						def.len = clen;
						break;
				}

				cans = f->askfieldfunc(&cta, &def, c->fmtname, c->fieldname, filen, def.dt, c->ehead, c->subhead);
				if(cans == (answer_t*)-1) {
					ERR(die, "askfunc said error here (dialoguemode).", NULL);
					/* clean up */
					return NULL;
				}
			}
		}
		cans = f->askfieldfunc(NULL, NULL, NULL, (char*)0x1 /* before real */, NULL, 0x0, NULL, NULL);
		if(cans == (answer_t*)-1) {
				ERR(die, "askfunc said error here (dialoguemode - before real).", NULL);
				/* clean up */
				return NULL;
		}
	}

	for(c = defs; c != NULL; c = c->next) {
		if(c->otherval != NULL)
			hasoth = 1;
		else
			hasoth = 0;

		for(i = 0; i < (hasoth ? 2 : 1); i++) {
			memset(&def, 0, sizeof(def));
			def.dt = ctype;
			switch(c->type) {
				case number:
				case boolean:
				case oenum:
				case oenumsub:
					def.integer = *(int*)cval;
					break;
				case string:
					def.string = (char*)cval;
					break;
				case fp:
					def.fp = *(FLOATTYPE *)cval;
					break;
				case image:
				case binary:
					def.vd = cval;
					def.len = clen;
					break;
			}

			cans = f->askfieldfunc(&cta, &def, c->fmtname, c->fieldname, filen, def.dt, c->ehead, c->subhead);
			if(cans == (answer_t*)-1) {
				ERR(die, "askfunc said error here.", NULL);
				/* clean up */
				return NULL;
			} else if(cans == (answer_t*)0x1) {
				/* no change... */
			} else if(cans == &cta) {
				/* changed... */
				free(cval);
				switch(ctype) {
					case number:
					case boolean:
					case oenum:
					case oenumsub:
						if(otherm) {
							c->otherval = malloc(sizeof(int));
							*(int*)c->otherval = cta.integer;
						} else {
							hasoth = 0;
							c->val = malloc(sizeof(int));
							*(int*)c->val = cta.integer;
						}
						break;
					case string:
						if(otherm)
							c->otherval = strdup(cta.string);
						else
							c->val = strdup(cta.string);
						break;
					case fp:
						if(otherm) {
							c->otherval = malloc(sizeof(FLOATTYPE));
							*(FLOATTYPE*)c->otherval = cta.fp;
						} else {
							c->val = malloc(sizeof(FLOATTYPE));
							*(FLOATTYPE*)c->val = cta.fp;
						}
						break;
					case image:
					case binary:
						if(!otherm) {
							c->val = malloc(cta.len);
							memcpy(c->val, cta.vd, cta.len);
						} else {
							c->otherval = malloc(cta.len);
							memcpy(c->otherval, cta.vd, cta.len);
						}
						break;
				}
			}
		}
	}

	return defs;
}

int complete_fields_from_db(f, cat, h)
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
		for(c = *h; c->next != NULL; c = c->next);
		last = c;
	}

	hhh = f->heads.db_cath;

	if(hhh == NULL) {
		ERR(die, "list not filled in from DB! call read_specs_from_db() first.", NULL);
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
		c = find_field_by_name(*h, cc->name);
		if(c == NULL) {
			new = allocz(sizeof(*new));
			new->fieldname = strdup(cc->name);
			new->fmtname = strdup(cc->alias);
			new->type = cc->type;
			new->val = allocz(1);
			*(char*)new->val = '\0';
			new->len = 1;
			if(new->type == oenum) {
				new->ehead = cc->enumptr;
			}
			if(new->type == oenum && cc->enumptr->otherelem != NULL) {
				new->othtype = cc->enumptr->otherelem->othertype;
				new->otherval = allocz(1);
				new->othlen = 1;
			}
			if(cc->type == oenumsub) {
				new->ehead = cc->subcatel->enumptr;
				new->subhead = cc->subcatel->enumptr->headelem->subhead;
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

int index_file(f, filename, cat, batch, useplugs, forceupdate, fields)
	fdbfs_t *f;
	char *filename;
	char *cat;
	int batch;
	int useplugs;
	int forceupdate;
	fields_t *fields;
{
	int rc = 0;
	fields_t *c = NULL, *h = NULL;

	if(rc != 2) {
		rc = file_has_changed(f, cat, filename);
		if(rc == 0) {
			if(!forceupdate)
				return 1;
		} else if(rc == -2) { /* is a directory */
			debug_info(f, warning, "in index_file: %s is a directory", filename);
		} else if(rc == -1) { /* hack alert */
			debug_info(f, warning, "error checking if %s changed: %s", filename, f->error.emsg);
			estr_free(&f->error);
		}
	}

	if(useplugs) {
		h = fill_in_fields(f, filename);
		if(h == NULL && f->error.emsg != NULL) {
			return 0;
		}
	}

	if(fields != NULL) {
		if(h != NULL) {
			for(c = fields; c->next /* we want to use it */ != NULL;
					c = c->next);
			c->next = h;
			h = fields;
		}
	}

	rc = complete_fields_from_db(f, cat, &h); /* fill out any others so the askfunc will ask the user */
	if(!rc) {
		CERR(die, "complete fields failed. ", NULL);
		free_field_list(h);
		return 0;
	}

	if(!batch) {
		ask_for_fields(f, filename, cat, h); /* we can only change existing stuff now anyway...this appends to h for us */
	}	

	rc = add_file(f, filename, cat, h);
	if(!rc) {
		CERR(die, "error indexing file %s. ", filename);
		free_field_list(h);
		return 0;
	}
	
	free_field_list(h);

	return 1;
}

int file_has_changed(f, cat, filename)
	fdbfs_t *f;
	char *cat;
	char *filename;
{
#if defined(UNIX)
	struct stat s;
#endif
	int rc;
	time_t upd; /* time_t must be signed!!!!! */
	
	upd = (time_t)get_lastupdate(f, cat, filename);

	if(upd == -1) { /*error*/
		CERR(die, "get_lsatupdate errored. ", NULL);
		return -1;
	} else if(upd == -2) {
		return 1;
	}
#if defined(UNIX)
	rc = stat(filename, &s);
	if(rc == -1) {
		ERR(die, "stat failed: %s", strerror(errno));
		return -1;
	}

	if(S_ISDIR(s.st_mode)) {
		return -2; /* a directory */
	}

	if(s.st_mtime > upd)
		return 1;
	else
		return 0;
#else
#	error "No code to stat for mtime"
#endif

	return 0;
}

	
int index_dir(f, dir, cat, useplugs, batch, nocase, re, recurselevel)
	fdbfs_t *f;
	char *dir;
	char *cat;
	int useplugs;
	int batch;
	int nocase;
	char *re;
	int recurselevel;
{
#if defined(UNIX)
	DIR *d;
	struct dirent *cd;
	int rc;
	char *emsg;
	char *fpth;
	size_t fplen;
	regex_t tre;

	if(re != NULL) {
		rc = regcomp(&tre, re, REG_EXTENDED | REG_NOSUB | (nocase ? REG_ICASE : 0));
		if(rc != 0) {
			emsg = malloc(128);
			regerror(rc, &tre, emsg, 127);
			ERR(die, "index_dir: error compiling regex (rc %d) '%s': %s", rc, re, emsg);
			free(emsg);
			return 0;
		}
	}

	d = opendir(dir);

	if(d == NULL) {
		ERR(die, "index_dir: cannot open directory '%s'", dir);
		regfree(&tre);
		return 0;
	}

	while((cd = readdir(d)) != NULL) {
		if(re != NULL) {
			rc = regexec(&tre, cd->d_name, 0, NULL, 0);
			if(rc == REG_NOMATCH)
				continue;
			else if(rc != 0) {
				emsg = malloc(128);
				regerror(rc, &tre, emsg, 127);
				ERR(die, "index_dir: error executing regex: %s", emsg);
				free(emsg);
				regfree(&tre);
				return 0;
			}

			fplen = cd->d_namlen + 1 /* slash */ + strlen(dir) + 1 /* NULL */;
			fpth = malloc(sizeof(char) * fplen);
			snprintf(fpth, fplen, "%s/%s", dir, cd->d_name);

			rc = file_has_changed(f, cat, fpth);
			if(rc == 0) {
				free(fpth);
				continue;
			} else if(rc == 1) {
				index_file(f, fpth, cat, batch, useplugs, 2, NULL);
				free(fpth);
			} else if(rc == -2) { /* is a directory */
				/* TODO: recurse*/
				free(fpth);
			} else if(rc == -1) { /* hack alert */
				debug_info(f, warning, "error checking if %s changed: %s", fpth, f->error.emsg);
				estr_free(&f->error);
				free(fpth);
			} else
				free(fpth);
		} else {
			fplen = cd->d_namlen + 1 /* slash */ + strlen(dir) + 1 /* NULL */;
			fpth = malloc(sizeof(char) * fplen);
			snprintf(fpth, fplen, "%s/%s", dir, cd->d_name);

			rc = file_has_changed(f, cat, fpth);
			if(rc == 0) {
				free(fpth);
				continue;
			} else if(rc == 1) {
				index_file(f, fpth, cat, batch, useplugs, 2, NULL);
				free(fpth);
			} else if(rc == -2) { /* is a directory */
				/* recurse*/
				free(fpth);
			} else if(rc == -1) { /* hack alert */
				debug_info(f, warning, "error checking if %s changed: %s", fpth, f->error.emsg);
				estr_free(&f->error);
				free(fpth);
			}
		}
	}

	closedir(d);
	if(re != NULL)
		regfree(&tre);
#else
#	error "Code not written for index_dir under this OS"
#endif
	return 1;
}

