/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * conf.h - configuration subsystem
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
 * @file conf.h
 * @brief Configuration subsystem header.
 */
/* $Amigan: fakedbfs/include/fakedbfs/conf.h,v 1.1 2006/01/29 21:04:36 dcp1990 Exp $ */
#ifndef HAVE_FDBFS_CONF_H
#define HAVE_FDBFS_CONF_H
#include <fakedbfs/types.h>

struct _config {
	char *pluginpath; /* search path, delimited by pipes (``|'') */
};

struct _confnode {
	char *tag;
	unsigned int flags;
	datatype_t type;
	data_t data;
	struct _confnode *child;
	struct _confnode *next;
};

confnode_t* fdbfs_conf_node_create(char *tag, confnode_t *parent, int leaf);
int fdbfs_conf_init(fdbfs_t *f);
int fdbfs_conf_init_db(fdbfs_t *f);
int fdbfs_conf_add_to_tree(fdbfs_t *f, char *mib, enum DataType type, union Data *data, short dynamic);
int fdbfs_conf_read_from_db(fdbfs_t *f);
enum DataType fdbfs_conf_get(fdbfs_t *f, char *mib, union Data *data);
int fdbfs_conf_set(fdbfs_t *f, char *mib, enum DataType type, union Data data);
void fdbfs_conf_destroy_tree(confnode_t *t);
confnode_t* fdbfs_conf_node_create(char *tag, confnode_t *parent, int leaf);
#endif
