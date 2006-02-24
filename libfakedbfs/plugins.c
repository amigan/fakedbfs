/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * plugins.c - file plugin stuff
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
/* $Amigan: fakedbfs/libfakedbfs/plugins.c,v 1.12 2006/02/24 08:01:02 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifdef UNIX
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#endif

/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs/fdbfsconfig.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/plugins.h>
#include <fakedbfs/debug.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/plugins.c,v 1.12 2006/02/24 08:01:02 dcp1990 Exp $")

#define LIBERR(sym) { \
		fdbfs_debug_info(f, error, "probe_plugin: symbol referece %s failed in %s: %s", sym, fpth, dlerror()); \
		free(fpth); \
		free(n); \
		dlclose(libhandle); \
		return last; \
	}

static struct Plugin* probe_plugin(f, dirpath, filename, last)
	fdbfs_t *f;
	char *dirpath;
	char *filename;
	struct Plugin *last;
{
#if defined(UNIX)
	void *libhandle;
	char *fpth;
	char *emsg;
	size_t len;
	struct Plugin *n;
	void *cptr;

	len = strlen(dirpath) + 1 /* slash */ + strlen(filename) + 1 /* null */;
	fpth = malloc(len * sizeof(char));
	snprintf(fpth, len, "%s/%s", dirpath, filename);
	
	if((libhandle = dlopen(fpth, RTLD_NOW /* for undef'd stuff */)) == NULL) {
		fdbfs_debug_info(f, warning, "probe_plugin: trouble loading %s: %s", fpth, dlerror());
		free(fpth);
		return last;
	}

	n = allocz(sizeof(*n));

	n->pl.libhandle = libhandle;
	n->info = (struct PluginInfo *)dlsym(libhandle, "plugin_inf");
	if(n->info == NULL) 
		LIBERR("plugin_inf");
	
	if(n->info->majapi != MAJOR_API_VERSION) {
		fdbfs_debug_info(f, error, "error! %s (%s v%s)'s majapi (%d) != MAJOR_API_VERSION (%d)! These are incompatible.",
				fpth, n->info->pluginname, n->info->version, n->info->majapi, MAJOR_API_VERSION);
		free(fpth);
		free(n);
		dlclose(libhandle);
		return last;
	}

	if(n->info->minapi > MINOR_API_VERSION) {
		fdbfs_debug_info(f, error, "error! %s (%s v%s)'s minapi (%d) > MINOR_API_VERSION (%d)! These are incompatible.",
				fpth, n->info->pluginname, n->info->version, n->info->majapi, MAJOR_API_VERSION);
		free(fpth);
		free(n);
		dlclose(libhandle);
		return last;
	}

	n->init = (int(*)(fdbfs_t*, char**, void**))dlfunc(libhandle, "plugin_init");
	if(n->init == NULL)
		LIBERR("plugin_init()");
	n->shutdown = (int(*)(fdbfs_t*, char **))dlfunc(libhandle, "plugin_shutdown");
	if(n->init == NULL)
		LIBERR("plugin_shutdown()");
	n->check_file = (int(*)(fdbfs_t*, char*, char**))dlfunc(libhandle, "check_file");
	if(n->check_file == NULL)
		LIBERR("check_file()");
	n->extract_from_file = (fields_t*(*)(fdbfs_t*, char*, char **))dlfunc(libhandle, "extract_from_file");
	if(n->extract_from_file == NULL)
		LIBERR("extract_from_file()");

	if(!n->init(f, &emsg, &cptr)) {
		fdbfs_debug_info(f, warning, "error: init for %s said: %s", fpth, emsg);
		free(n);
		free(fpth);
		dlclose(libhandle);
		return last;
	}

	n->cptr = cptr;

	if(last != NULL)
		last->next = n;

	free(fpth);
	
	return n;
#else
#	warning "No plugin code on this platform"
	return NULL;
#endif
}

static struct Plugin* search_plugs(f, plugins, path)
	fdbfs_t *f;
	struct Plugin *plugins;
	char *path;
{
#if defined(UNIX)
#define LIBENDING ".so"
	/* warning; Unix-specific shiite here */
	DIR *d;
	char *ending;
	char *ce;
	struct dirent *dp;
	struct Plugin *lp = plugins, *lpo = plugins;

	d = opendir(path);

	if(d == NULL) {
		ERR(die, "cannot open directory %s", path);
		return NULL;
	}

	while((dp = readdir(d)) != NULL) {
		if(dp->d_namlen <= strlen(LIBENDING))
			continue;
		ending = dp->d_name + (dp->d_namlen - strlen(LIBENDING));
		if(strcmp(ending, LIBENDING) == 0) {
			lpo = lp;
			ce = strdup(dp->d_name);
			if((lp = probe_plugin(f, path, ce, lp)) == lpo && f->error.emsg != NULL) {
				free(ce);
				closedir(d);
				SCERR(die, "Error scanning directory. ");
				return lpo;
			}
			free(ce);
		}
	}

	closedir(d);

	return lp;
#else
#	warning "No plugin code on this platform"
	return NULL;
#endif
}
	
int fdbfs_plugins_init(f)
	fdbfs_t *f;
{
	char *path;
	char *p, *op;
	struct Plugin *lastplug = NULL, *head = NULL;
	size_t len;

	if(f->conf.pluginpath != NULL)
		path = strdup(f->conf.pluginpath);
	else {
		len = strlen(LIBPATH) + 1 /* DELIMCHAR */ + strlen(getenv("HOME")) + 1 /* slash */ + strlen(FDBFSDIR) + 1 /* null */;
		path = malloc(sizeof(char) * len);
		snprintf(path, len, "%s" DELIMCHAR "%s/%s", LIBPATH, getenv("HOME"), FDBFSDIR);
	}

	op = path;

	while((p = strsep(&op, DELIMCHAR)) != NULL) {
		lastplug = search_plugs(f, lastplug, p);
		if(head == NULL)
			head = lastplug;
		if(f->error.emsg != NULL) {
			f->plugins = head;
			free(path);
			SCERR(die, "Error searching. ");
			return 0;
		}
	}

	f->plugins = head;

	free(path);
	return 1;
}

int fdbfs_plugin_set_cptr(f, pinf, dat)
	fdbfs_t *f;
	const struct PluginInfo *pinf;
	void *dat;
{
	struct Plugin *c;

	for(c = f->plugins; c != NULL; c = c->next) {
		if(c->info == pinf) { /* this is the same plugin */ 
			c->cptr = dat;
			return 1;
		}
	}

	return 0;
}

int fdbfs_plugin_get_cptr(f, pinf, dat)
	fdbfs_t *f;
	const struct PluginInfo *pinf;
	void **dat;
{
	struct Plugin *c;

	for(c = f->plugins; c != NULL; c = c->next) {
		if(c->info == pinf) { /* this is the same plugin */ 
			*dat = c->cptr;
			return 1;
		}
	}

	return 0;
}

void fdbfs_plugins_set_path(f, path)
	fdbfs_t *f;
	char *path;
{
	f->conf.pluginpath = strdup(path);
}
