/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * debug.h - debug and error stuff
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
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
/* $Amigan: fakedbfs/include/fakedbfs/debug.h,v 1.2 2006/02/25 07:12:31 dcp1990 Exp $ */
/**
 * @file debug.h
 * @brief Debug and error handling.
 */


int fdbfs_ferr(fdbfs_t *f, enum ErrorAction severity, char *fmt, ...);
int fdbfs_cferr(fdbfs_t *f, enum ErrorAction severity, char *fmt, ...);
void fdbfs_debug_dump_fields(fields_t *h);
struct EnumSubElem* fdbfs_debug_dump_enum_sub_elem(struct EnumSubElem *e, short int allsub); /* returns next */
void fdbfs_debug_dump_enum_sub_elem_list(struct EnumSubElem *head, short int allsub);
struct EnumElem* fdbfs_debug_dump_enum_elem(struct EnumElem *e);
void fdbfs_debug_dump_enum_elem_list(struct EnumElem *head);
struct EnumHead* fdbfs_debug_dump_enum_head(struct EnumHead *e);
void fdbfs_debug_dump_enum_head_list(struct EnumHead *head);
struct CatElem* fdbfs_debug_dump_cat_elem(struct CatElem *e);
void fdbfs_debug_dump_cat_elem_list(struct CatElem *head);
struct CatalogueHead* fdbfs_debug_dump_cat_head(struct CatalogueHead *e);
void fdbfs_debug_dump_cat_head_list(struct CatalogueHead *head);
void fdbfs_debug_dump_head_members(Heads *hd);
int fdbfs_debug_info(fdbfs_t *f, enum ErrorAction sev, char *fmt, ...);
void fdbfs_estr_free(error_t *e);

/**
 * @brief Dump confnode_t and children
 *
 * @param c confnode_t to dump.
 */
void fdbfs_debug_dump_confnode(confnode_t *c);
