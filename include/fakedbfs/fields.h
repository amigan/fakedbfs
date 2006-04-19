/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * fields.h - fields stuff
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
 * @file fields.h
 * @brief fields and answer stuff.
 */
/* $Amigan: fakedbfs/include/fakedbfs/fields.h,v 1.7 2006/04/19 19:58:22 dcp1990 Exp $ */
#ifndef HAVE_FDBFS_FIELDS_H
#define HAVE_FDBFS_FIELDS_H

/**
 * @brief Free fields_t object.
 *
 * If e->flags has the FIELDS_FLAG_MMAPED bit set, it will be munmap()ed, but only if HAVE_MMAP was defined at compile time.
 * @param e fields_t object to free.
 * @return Next element, or NULL on error.
 */
fields_t* fdbfs_free_field(fields_t *e);

/**
 * @brief Free list of fields_t's.
 *
 * @param h Head of list to traverse and free.
 */
void fdbfs_free_field_list(fields_t *h);

/**
 * @brief Search for fields_t in a list by name.
 *
 * @param h List to traverse.
 * @param name Name to search for.
 * @return fields_t that matches, or NULL if not found.
 */
fields_t* fdbfs_find_field_by_name(fields_t *h, const char *name);

/**
 * @brief Search for fields_t in a list by enumhead.
 *
 * @param h List to traverse.
 * @param e EnumHead to search for.
 * @return fields_t that matches, or NULL if not found.
 */
fields_t* fdbfs_find_field_by_ehead(fields_t *h, struct EnumHead *e);

/**
 * @brief Search for fields_t in a list by enumhead name.
 *
 * @param h List to traverse.
 * @param e EnumHead name to search for.
 * @return fields_t that matches, or NULL if not found.
 */
fields_t* fdbfs_find_field_by_ename(fields_t *h, const char *e);

/**
 * @brief Parse defspec and create fields_t list from it.
 *
 * @param f Fakedbfs instance to operate on.
 * @param tsp Defspec to parse.
 * @return fields_t list from defspec, or NULL on error.
 */
fields_t* fdbfs_fields_from_dsp(fdbfs_t *f, const char *tsp);

/**
 * @brief Free answer_t object.
 *
 * @param e Object to free.
 */
void fdbfs_free_answer_t(answer_t *e);

/**
 * @brief Add integer element to fields_t list.
 * 
 * @param name Name of element.
 * @param fmtname Friendly name of element.
 * @param[in,out] th Pointer to pointer containing address of head of list (will initialise if NULL)
 * @param[in,out] tc Pointer to pointer containing address of last element on list (will set to last element).
 * @param value The value to set the element to.
 * @return 0 on error (*th and *tc will remain unchanged)
 */
int fdbfs_field_add_int(const char *name, const char *fmtname, fields_t **th, fields_t **tc, int value);

/**
 * @brief Add string element to fields_t list.
 * 
 * @param name Name of element.
 * @param fmtname Friendly name of element.
 * @param[in,out] th Pointer to pointer containing address of head of list (will initialise if NULL)
 * @param[in,out] tc Pointer to pointer containing address of last element on list (will set to last element).
 * @param value The value to set the element to. (not copied)
 * @return 0 on error (*th and *tc will remain unchanged)
 */
int fdbfs_field_add_string(const char *name, const char *fmtname, fields_t **th, fields_t **tc, char *value);

/**
 * @brief Add image element to fields_t list.
 * 
 * @param name Name of element.
 * @param fmtname Friendly name of element.
 * @param[in,out] th Pointer to pointer containing address of head of list (will initialise if NULL)
 * @param[in,out] tc Pointer to pointer containing address of last element on list (will set to last element).
 * @param value The value to set the element to. (not copied)
 * @param sz The length of the data.
 * @return 0 on error (*th and *tc will remain unchanged)
 */
int fdbfs_field_add_image(const char *name, const char *fmtname, fields_t **th, fields_t **tc, void *value, size_t sz);

/**
 * @brief Searches for a field by its next member.
 *
 * @param h Head of list to search.
 * @param next Value to search for.
 * @return Element found or NULL on not found.
 */
fields_t* fdbfs_find_field_by_next(fields_t *h, fields_t *next);

/**
 * @brief Sets the MIME type of a fields list.
 *
 * Sets the MIME type. Will remove any previously-existing mime fields.
 * @param mimetype The mimetype to set to.
 * @param[in,out] Pointer to pointer to head of list.
 * @param[in,out] Pointer to pointer to last element of list.
 * @return 0 on error.
 */
int fdbfs_fields_set_mime(const char *mimetype, fields_t **th, fields_t **tc);

/**
 * @brief Appends field to a field list
 *
 * It is recommended to use the fdbfs_field_int()/string()/whatever functions.
 * This is mostly for internal use.
 *
 * @param name Name of field.
 * @param fmtname Alias of field. 
 * @param[in,out] th Pointer to pointer to head of field list. Will be initialised if (*th == NULL).
 * @param[in,out] tc Pointer to pointer to last element in list. Will be reset.
 * @param type Datatype of field.
 * @param value Pointer to data to set to.
 * @param len Size of data.
 */
int fdbfs_field_append(const char *name, const char *fmtname, fields_t **th, fields_t **tc, enum DataType type, void *value, size_t len);


#endif
