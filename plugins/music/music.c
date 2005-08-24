/*
 * Copyright (c) 2005, Dan Ponte
 *
 * music.c - source for plugin that extracts music metadata
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
/* $Amigan: fakedbfs/plugins/music/music.c,v 1.1 2005/08/24 06:23:34 dcp1990 Exp $ */
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

#include <fakedbfs.h>

RCSID("$Amigan: fakedbfs/plugins/music/music.c,v 1.1 2005/08/24 06:23:34 dcp1990 Exp $")
#define MUSICPLUGINVER "0.1"

struct PluginInfo plugin_inf = {
	"mp3/ogg/wav", /* extensions supported */
	"Music", /* name */
	MUSICPLUGINVER, /* version */
	"Dan Ponte <dcp1990@neptune.atopia.net", /* author */
	"http://www.theamigan.net/fakedbfs/", /* www */
	MAJOR_API_VERSION, /* major api version */
	MINOR_API_VERSION /* minor api version */
};


int plugin_init(errmsg)
	char **errmsg;
{
	/* do nothing */
	return 1;
}

int plugin_shutdown(errmsg)
	char **errmsg;
{
	return 1;
}
