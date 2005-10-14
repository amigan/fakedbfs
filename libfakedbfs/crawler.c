/*
 * Copyright (c) 2005, Dan Ponte
 *
 * crawler.c - a directory crawler for platforms that don't have FTS
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
/* $Amigan: fakedbfs/libfakedbfs/crawler.c,v 1.3 2005/10/14 21:15:20 dcp1990 Exp $ */
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
#endif
/* other libraries */
#include <sqlite3.h>
/* us */
#include <fdbfsregex.h>
#include <fdbfsconfig.h>
#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/crawler.c,v 1.3 2005/10/14 21:15:20 dcp1990 Exp $")

crawl_t* new_crawler(f, mlevels, mlbd)
	fdbfs_t *f;
	int mlevels; /* max levels to traverse; spew errors after this */
	int mlbd; /* max levels before depth traversal....might not be used, oh well */
{
	crawl_t *new;

	new = allocz(sizeof(*new));
	new->maxlevels = mlevels;
	new->mlbefdep = mlbd;
	new->f = f;

	return new;
}

void destroy_crawler(cr)
	crawl_t *cr;
{
	destroy_frame(cr->topframe);
	free(cr);
}

int push_frame(dst, obj)
	crawlframe_t *dst;
	crawlframe_t *obj;
{
	if(dst->cindex >= dst->maxelements)
		return 0;

	dst->sp += dst->cindex == 0 ? 0 : sizeof(*dst->sp);
	if(dst->cindex != 0)
		dst->cindex++;

	memcpy(dst->sp, obj, sizeof(*dst->sp));

	return 1;
}

int pop_frame(src, dst)
	crawlframe_t *src;
	crawlframe_t *dst;
{
	if(src->cindex <= 0)
		return 0;

	src->sp -= sizeof(*src->sp);
	src->cindex--;

	memcpy(src->sp, dst, sizeof(*dst));

	return 1;
}

void traverse_and_free(cf)
	crawlframe_t *cf;
{
	crawlframe_t *tf = malloc(sizeof(crawlframe_t)); /* keep stack usage low; this gets recursive */
	if(cf->cindex == 0)
		return;

	while(pop_frame(cf, tf)) {
		destroy_frame(tf);
	}

	free(tf);
}

void destroy_frame(cf)
		crawlframe_t *cf;
{
	traverse_and_free(cf);
	free(cf->stack);
	free(cf);
}

crawlframe_t* create_frame(cr, size, parent, fid, level)
	crawl_t *cr;
	size_t size;
	crawlframe_t *parent;
	file_id_t *fid;
	int level;
{
	crawlframe_t *new;

	new = allocz(sizeof(*new));

	new->stack = new->sp = allocz(sizeof(crawlframe_t*) * size);
	new->stop = new->sp + (sizeof(crawlframe_t) * size);
	new->maxelements = size;
	new->cindex = 0;
	new->level = level;
	new->parent = parent;
	new->fajah = cr;

	if(fid != NULL)
		memcpy(&new->oid, fid, sizeof(file_id_t));

	return new;
}

int crawl_dir(cr, dir) /* simply adds dir to the base frame */
	crawl_t *cr;
	char *dir;
{
	file_id_t nfi;
	crawlframe_t *n;

	if(cr->topframe == NULL) { /* create the stack frame */
		cr->topframe = create_frame(cr, CRAWLFRAME_MAX, NULL, NULL, 0);
		if(cr->topframe == NULL) {
			cferr(cr->f, die, "Error creating toplevel stackframe. ");
			return 0;
		}
	}

	nfi.filename = strdup(dir);

	n = create_frame(cr, CRAWLFRAME_MAX, cr->topframe, &nfi, 1);

	if(n == NULL) {
		free(nfi.filename);
		cferr(cr->f, die, "Error creating stackframe. ");
		return 0;
	}

	if(!push_frame(cr->topframe, n)) {
		destroy_frame(n);
		free(nfi.filename);
		cferr(cr->f, die, "Error pushing stackframe. ");
		return 0;
	}


	return 1;
}
