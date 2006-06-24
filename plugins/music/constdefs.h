/*
 * Copyright (c) 2005, Dan Ponte
 *
 * constdefs.h - constant definitions for music plugin. Keep in sync with specfile!
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
/* $Amigan: fakedbfs/plugins/music/constdefs.h,v 1.6 2006/06/24 17:30:40 dcp1990 Exp $ */

#define MP3EXT ".mp3"
#define OGGEXT ".ogg"
#define WAVEXT ".wav"
#define WMAEXT ".wma"
#define FLACEXT ".flac"

#define FILENAME_REGEX "([^/]*) - ([^/]*) -( [0-9][0-9]-| )([0-9][0-9]) ([^/]*)\\.([a-zA-Z0-9]{2,4})$"
#define CASE_INSENS 1
#define NSUBS 5

#define ARTISTNAME "artist"
#define ARTISTFMT "Artist"

#define ALBUMNAME "album"
#define ALBUMFMT "Album"

#define TRACKNAME "track"
#define TRACKFMT "Track"

#define TITLENAME "title"
#define TITLEFMT "Title"

#define YEARNAME "year"
#define YEARFMT "Year"

#define ACOVERNAME "albumcover"
#define ACOVERFMT "Album Cover"

#define DISCNAME "disc"
#define DISCFMT "Disc"

#define OGG_PREFER_MIB	"fdbfs.plugins.music.prefer_ogg_fn"
#define MP3_PREFER_MIB	"fdbfs.plugins.music.prefer_mp3_fn"
#define FLAC_PREFER_MIB	"fdbfs.plugins.music.prefer_flac_fn"

#define	PREFER_OGG_FN	0x1
#define	PREFER_MP3_FN	0x2
#define	PREFER_FLAC_FN	0x4
