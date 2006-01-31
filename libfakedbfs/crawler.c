/*
 * Copyright (c) 2005-2006, Dan Ponte
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
/* $Amigan: fakedbfs/libfakedbfs/crawler.c,v 1.8 2006/01/31 17:26:26 dcp1990 Exp $ */
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
#	define HAVE_DIR_H 1
#endif
/* other libraries */
#include <sqlite3.h>
/* us */
#include <fakedbfs/fdbfsregex.h>
#include <fakedbfs/fdbfsconfig.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/debug.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/crawler.c,v 1.8 2006/01/31 17:26:26 dcp1990 Exp $")

static void destroy_frame(crawlframe_t *cf);

static int push_frame(dst, obj)
	crawlframe_t *dst;
	crawlframe_t *obj;
{
	if(dst->cindex >= dst->maxelements)
		return 0;

	dst->sp += dst->cindex == 0 ? 0 : 1; /* hope this works */
	if(dst->cindex != 0)
		dst->cindex++;

	*dst->sp = obj;

	return 1;
}

static crawlframe_t *pop_frame(src)
	crawlframe_t *src;
{
	if(src->cindex <= 0)
		return NULL;

	src->cindex--;

	return *(src->sp--);
}



static crawlframe_t* create_frame(cr, size, parent, fid, level)
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


static void traverse_and_free(cf)
	crawlframe_t *cf;
{
	crawlframe_t *tf;
	if(cf->cindex == 0)
		return;

	while((tf = pop_frame(cf)) != NULL) {
		destroy_frame(tf);
	}
}

static void destroy_frame(cf)
		crawlframe_t *cf;
{
	traverse_and_free(cf);
	free(cf->stack);
	/*
	 * CAVEAT EMPTOR:
	 * cf->oid.filename is *NOT* freed here; we assume that the application doing crawl_go() will
	 * use it and free when done. This may not be the case.
	 */
	free(cf);
}

crawl_t* fdbfs_crawler_new(f, mlevels, mlbd)
	fdbfs_t *f;
	int mlevels; /* max levels to traverse; spew errors after this */
	int mlbd; /* max levels before depth traversal....might not be used, oh well
		   * set to 1 for depth-only traversal, 0 for no depth traversal at all */
{
	crawl_t *new;

	new = allocz(sizeof(*new));
	new->maxlevels = mlevels;
	new->mlbefdep = mlbd;
	new->f = f;

	return new;
}

void fdbfs_crawler_destroy(cr)
	crawl_t *cr;
{
	destroy_frame(cr->topframe);
	free(cr);
}

int fdbfs_crawl_dir(cr, dir) /* simply adds dir to the base frame */
	crawl_t *cr;
	char *dir;
{
	file_id_t nfi;
	crawlframe_t *n;

	if(cr->topframe == NULL) { /* create the stack frame */
		cr->topframe = create_frame(cr, CRAWLFRAME_MAX, NULL, NULL, 0);
		if(cr->topframe == NULL) {
			fdbfs_cferr(cr->f, die, "Error creating toplevel stackframe. ");
			return 0;
		}
		cr->curframe = cr->topframe;
	}

	nfi.filename = strdup(dir);

	n = create_frame(cr, CRAWLFRAME_MAX, cr->topframe, &nfi, 1);

	if(n == NULL) {
		free(nfi.filename);
		fdbfs_cferr(cr->f, die, "Error creating stackframe. ");
		return 0;
	}

	if(!push_frame(cr->topframe, n)) {
		destroy_frame(n);
		free(nfi.filename);
		fdbfs_cferr(cr->f, die, "Error pushing stackframe. ");
		return 0;
	}


	return 1;
}

static int setup_dir(cr, ds, oid)
	crawl_t *cr;
	struct DirState *ds;
	file_id_t *oid;
{
	/* TODO: implement this for each platform */
	return 1;
}

int fdbfs_crawl_go(cr, flags, fid)
	crawl_t *cr;
	int flags;
	file_id_t *fid; /* copy to */
{
	crawlframe_t *c;
	
	if(cr->curframe == NULL)
		return CRAWL_ERROR;


	if((c = pop_frame(cr->curframe)) == NULL) {
		crawlframe_t *t;

		t = cr->curframe->parent;

		destroy_frame(t);

		if((cr->curframe = t) == NULL) {
			return CRAWL_FINISHED; /* we've hit the bottom; all done */
		}

		return fdbfs_crawl_go(cr, flags, fid); /* cascade down until we don't see anything */
	}

	if(!setup_dir(cr, &c->ds, &c->oid)) {
		if(flags & CRAWL_DIE_ON_ERROR)
			return CRAWL_ERROR;
	}

	memcpy(fid, &c->oid, sizeof(file_id_t));

	/* be sure to open the DIR handle in this function when you see one, and close the most recent one when
	 * dropping down on the stack frame list!
	 */
	return CRAWL_DIR;
}
