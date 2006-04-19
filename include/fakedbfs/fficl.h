/*
 * Copyright (c) 2005, Dan Ponte
 *
 * fficl.h - ficl stuff
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
/* $Amigan: fakedbfs/include/fakedbfs/fficl.h,v 1.4 2006/04/19 19:58:22 dcp1990 Exp $ */
/**
 * @file fficl.h
 * @brief fakedbfs ficl bindings.
 */
#ifndef HAVE_FDBFS_FICL_H
#define HAVE_FDBFS_FICL_H
#include <fakedbfs/types.h>
#include <ficl.h>

struct _ficlplug {
	char *wordprefix; /** < word prefix, must be unique */
	char *filename; /** < filename of the source */
	ficlVm *vm; /** < the VM */
};


/* ficl stuff */

/**
 * @brief Initialise ficl subsystem.
 *
 * Creates a new ficlSystem.
 * @param f Instance of fakedbfs.
 * @return 0 on error.
 */
int fdbfs_ficl_init(fdbfs_t *f);

/**
 * @brief Destroy ficl subsystem.
 *
 * @param f Instance of fakedbfs.
 */
void fdbfs_ficl_destroy(fdbfs_t *f);

/**
 * @brief Add words to ficl system dictionary.
 *
 * @param f Instance of fakedbfs.
 * @param dict Dictionary to add to.
 * @return 0 on error.
 */
int fdbfs_ficl_addwords(fdbfs_t *f, ficlDictionary *dict);


/**
 * @brief Check plugin for file compatibility using ficl subsystem.
 *
 * Ficl analogue to (struct Plugin).check_file().
 * @param fpl ficlplug_t object to operate on.
 * @param filename Filename to check.
 * @param errmsg Pointer to char* that will be set to error message on error.
 * @retval -1 Error occured. Examine and free *errmsg.
 * @retval 0 Doesn't match.
 * @retval 1 Matches...this plugin is compatibile with this file.
 */
int fdbfs_ficl_p_check_file(ficlplug_t *fpl, const char *filename, char **errmsg);

/**
 * @brief Initialise ficl plugin.
 *
 * The usefulness of this is questionable, but...
 * @param fpl ficlplug_t object to operate on.
 * @param errmsg Pointer to char* that will be set to error message on error.
 * @return 0 on error (examine and free *errmsg).
 */
int fdbfs_ficl_p_init(ficlplug_t *fpl, char **errmsg);

/**
 * @brief Shutdown ficl plugin.
 *
 * Analogous to (struct Plugin).shutdown().
 * @param fpl ficlplug_t to operate on.
 * @param errmsg Pointer to char* that will be set to error message on error.
 * @return 0 on error (examine and free *errmsg).
 */
int fdbfs_ficl_p_shutdown(ficlplug_t *fpl, char **errmsg);

/**
 * @brief Extract metedata using ficl plugin.
 *
 * Analogous to (struct Plugin).extract_from_file().
 * @param fpl ficlplug_t object to operate on.
 * @param filename Filename to extract from.
 * @param errmsg Pointer to char* that will be set to error message on error.
 * @return List of fields_t for the specified file, or NULL on error (examine
 * and free *errmsg).
 */
fields_t* fdbfs_ficl_p_extract_from_file(ficlplug_t *fpl, const char *filename,
		char **errmsg);


/**
 * @brief Load ficlplugin and return a Plugin object.
 *
 * Parses the ficl plugin in the specified filename.
 * @param f Fakedbfs instance.
 * @param plugfile Filename to parse.
 * @return Plugin object describing the plugin, or NULL on error.
 */
struct Plugin* fdbfs_ficl_load_ficlplugin(fdbfs_t *f, const char *plugfile);

#endif
