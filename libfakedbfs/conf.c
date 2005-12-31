/*
 * Copyright (c) 2005, Dan Ponte
 *
 * conf.c - fakedbfs configuration subsystem
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
/* $Amigan: fakedbfs/libfakedbfs/conf.c,v 1.2 2005/12/31 04:59:24 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <float.h>
#include <ctype.h>

/* other libraries */
#include <sqlite3.h>

#include <fdbfsconfig.h>

#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/libfakedbfs/conf.c,v 1.2 2005/12/31 04:59:24 dcp1990 Exp $")

static void conf_node_link_parent(parent, n)
	confnode_t *parent, *n;
{
	confnode_t *c;

	if(parent->child == NULL)
		parent->child = n;
	else {
		for(c = parent->child; c->next != NULL; c = c->next) ;
		c->next = NULL;
	}
}

confnode_t* conf_node_create(tag, parent, leaf)
	char *tag;
	confnode_t *parent;
	int leaf;
{
	confnode_t *n;

	n = allocz(sizeof(*n));
	n->tag = strdup(tag);

	if(parent != NULL) {
		conf_node_link_parent(parent, n);
	}

	if(leaf)
		n->flags |= CN_FLAG_LEAF;

	return n;
}
int conf_init(f)
	fdbfs_t *f;
{
	f->rconf = conf_node_create(ROOT_NODE_TAG, NULL, 0);

	return 1;
}
