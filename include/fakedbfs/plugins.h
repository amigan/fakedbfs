/*
 * Copyright (c) 2005-2006, Dan Ponte
 *
 * plugins.h - file plugin stuff
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
/* $Amigan: fakedbfs/include/fakedbfs/plugins.h,v 1.6 2006/06/24 17:19:55 dcp1990 Exp $ */
/**
 * @file plugins.h
 * @brief Plugin stuff.
 */
#ifndef HAVE_FDBFS_PLUGINS_H
#define HAVE_FDBFS_PLUGINS_H
#include <fakedbfs/types.h>
#include <fakedbfs/fficl.h>


struct Plugin {
	const struct PluginInfo *info;
	
	union {
		void *libhandle; /* if shared */
		ficlplug_t fplug;
	} pl;

	void *cptr; /* client data */
			
	int flags;
	struct Plugin *next;
};

#define PLUGIN_IS_FICL	0x1 /* flag */
#define PLUGIN_IS_PRIMITIVE	0x2 /* not shared */

/**
 * @brief Initialise plugin subsystem.
 * 
 * @param f Instance of fakedbfs.
 * @return 0 on error.
 */
int fdbfs_plugins_init(fdbfs_t *f);

/**
 * @brief Set plugin path.
 *
 * fdbfs_plugins_set_path() sets the path that will be searched for plugins.
 * @param f The instance of fakedbfs on which to operate.
 * @param path The new path.
 */
void fdbfs_plugins_set_path(fdbfs_t *f, const char *path);

/**
 * @brief Destroy plugin list
 *
 * @param h Head of list.
 */
void fdbfs_free_plugin_list(struct Plugin *h);

/**
 * @brief Set client data.
 *
 * Sets an internal pointer to an arbitrary value for later retrieval.
 *
 * @param f fakedbfs instance to operate on.
 * @param pinf PluginInfo structure identifying this plugin;; must be the same pointer that was registered with.
 * @param dat Where to store the data.
 */
int fdbfs_plugin_set_cptr(fdbfs_t *f, const struct PluginInfo *pinf, void *dat);

/**
 * @brief Get client data.
 *
 * Retrieves value set with fdbfs_plugin_set_cptr().
 *
 * @param f fakedbfs instance to operate on.
 * @param pinf PluginInfo structure identifying this plugin;; must be the same pointer that was registered with.
 * @param[out] dat The data to set to.
 */
int fdbfs_plugin_get_cptr(fdbfs_t *f, const struct PluginInfo *pinf, void **dat);
#endif
