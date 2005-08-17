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
/* $Amigan: fakedbfs/libfakedbfs/indexing.c,v 1.7 2005/08/17 01:14:32 dcp1990 Exp $ */
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

RCSID("$Amigan: fakedbfs/libfakedbfs/indexing.c,v 1.7 2005/08/17 01:14:32 dcp1990 Exp $")

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
		debug_info(f, error, "cannot find plugin to extract metadata for %s!", filename);
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

int index_file(f, filename, cat, batch, useplugs, fields)
	fdbfs_t *f;
	char *filename;
	char *cat;
	int batch;
	int useplugs;
	fields_t *fields;
{
	int rc;
	fields_t *c = NULL, *h = NULL;

	if(useplugs) {
		h = fill_in_fields(f, filename);
		if(h == NULL && f->error.emsg != NULL) {
			return 0;
		}
	}

	if(fields != NULL) {
		if(h != NULL) {
			for(c = fields; c != NULL; c = c->next);
			c->next = h;
			h = fields;
		}
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
#endif

	return 0;
}

	
int index_dir(f, dir, cat, useplugs, batch, nocase, re)
	fdbfs_t *f;
	char *dir;
	char *cat;
	int useplugs;
	int batch;
	int nocase;
	char *re;
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
		if(!rc) {
			emsg = malloc(128);
			regerror(rc, &tre, emsg, 127);
			ERR(die, "index_dir: error compiling regex '%s': %s", re, emsg);
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
				index_file(f, fpth, cat, batch, useplugs, NULL);
				free(fpth);
			} else if(rc == -2) { /* is a directory */
				/* recurse*/
				free(fpth);
			} else if(rc == -1) { /* hack alert */
				debug_info(f, warning, "error checking if %s changed: %s", fpth, f->error.emsg);
				estr_free(&f->error);
				free(fpth);
			}

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
				index_file(f, fpth, cat, batch, useplugs, NULL);
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
#endif
	return 1;
}

/* int index_file(f, filename, cat, batch, useplugs, fields) */
