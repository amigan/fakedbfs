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
/* $Amigan: fakedbfs/plugins/music/music.c,v 1.11 2006/02/23 21:26:01 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <id3.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>

#ifdef UNIX
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <dirent.h>
#endif

#include <fakedbfs/fdbfsregex.h>
#include <fakedbfs/fakedbfs.h>
#include <fakedbfs/plugins.h>
#include <fakedbfs/fields.h>
#include <fakedbfs/debug.h>

RCSID("$Amigan: fakedbfs/plugins/music/music.c,v 1.11 2006/02/23 21:26:01 dcp1990 Exp $")
#define MUSICPLUGINVER "0.1"

#include "constdefs.h"

const struct PluginInfo plugin_inf = {
	"mp3/ogg/wav/flac", /* extensions supported */
	"Music", /* name */
	MUSICPLUGINVER, /* version */
	"Dan Ponte <dcp1990@neptune.atopia.net>", /* author */
	"http://www.theamigan.net/fakedbfs/", /* www */
	MAJOR_API_VERSION, /* major api version */
	MINOR_API_VERSION /* minor api version */
};


int plugin_init(f, errmsg)
	fdbfs_t *f;
	char **errmsg;
{
	void *asd;
	asd = (void*)ID3_v1_genre_description; /* stupid unused warning */
	/* do nothing */
	return 1;
}

int plugin_shutdown(f, errmsg)
	fdbfs_t *f;
	char **errmsg;
{
	return 1;
}

int check_file(f, filename, errmsg)
	fdbfs_t *f;
	char *filename;
	char **errmsg;
{
	char *ext;

	/* useless, I know */

	ext = (filename + strlen(filename)) - strlen(MP3EXT);

	if(strcasecmp(ext, MP3EXT) == 0)
		return 1;
	
	ext = (filename + strlen(filename)) - strlen(OGGEXT);

	if(strcasecmp(ext, OGGEXT) == 0)
		return 1;

	ext = (filename + strlen(filename)) - strlen(WAVEXT);

	if(strcasecmp(ext, WAVEXT) == 0)
		return 1;

	ext = (filename + strlen(filename)) - strlen(FLACEXT);

	if(strcasecmp(ext, FLACEXT) == 0)
		return 1;

	return 0;
}

int match_filename(filename, errmsg, tc, th)
	char *filename;
	char **errmsg;
	fields_t **tc;
	fields_t **th;
{
	fields_t *h = NULL, *c = NULL, *n;
	char *ours, *cur, oc;
	freg_t *tre;
	char emsg[256];
	fregmatch_t matches[NSUBS + 1];
	int rc;

	tre = new_freg(emsg, sizeof(emsg));

	if(tre == NULL) {
		*errmsg = strdup(emsg);
		return 0;
	}

	rc = fregcomp(tre, FILENAME_REGEX, (CASE_INSENS ? FREG_NOCASE : 0));
	if(rc != 0) {
		*errmsg = strdup(tre->errmsg);
		destroy_freg(tre);
		return 0;
	}

	rc = fregexec(tre, filename, matches, NSUBS + 1);
	if(rc != FREG_NOMATCH && rc != 0) {
		*errmsg = strdup(tre->errmsg);
		destroy_freg(tre);
		return 0;
	} else if(rc != FREG_NOMATCH) {
		h = malloc(sizeof(*h));
		memset(h, 0, sizeof(*h));
		h->fieldname = strdup(ARTISTNAME);
		h->fmtname = strdup(ARTISTFMT);
		ours = strdup(filename);
		cur = ours + matches[1].s;
		oc = *(ours + matches[1].e);
		*(ours + matches[1].e) = '\0';
		h->val = strdup(cur);
		h->type = string;
		*(ours + matches[1].e) = oc;
		
		n = malloc(sizeof(*n));
		memset(n, 0, sizeof(*n));
		h->next = n;
		c = n;
		c->fieldname = strdup(ALBUMNAME);
		c->fmtname = strdup(ALBUMFMT);
		c->type = string;
		cur = ours + matches[2].s;
		oc = *(ours + matches[2].e);
		*(ours + matches[2].e) = '\0';
		c->val = strdup(cur);
		*(ours + matches[2].e) = oc;

		n = malloc(sizeof(*n));
		memset(n, 0, sizeof(*n));
		c->next = n;
		c = n;
		c->fieldname = strdup(DISCNAME);
		c->fmtname = strdup(DISCFMT);
		c->type = number;
		if(matches[3].s == matches[3].e - 1) {
			c->val = malloc(sizeof(int));
			*(int*)c->val = 1;
		} else {
			cur = ours + matches[3].s;
			oc = *(ours + matches[3].e);
			*(ours + matches[3].e) = '\0';
			c->val = malloc(sizeof(int));
			*(int*)c->val = atoi(cur);
			*(ours + matches[3].e) = oc;
		}

		n = malloc(sizeof(*n));
		memset(n, 0, sizeof(*n));
		c->next = n;
		c = n;
		c->fieldname = strdup(TRACKNAME);
		c->fmtname = strdup(TRACKFMT);
		c->type = number;
		cur = ours + matches[4].s;
		oc = *(ours + matches[4].e);
		*(ours + matches[4].e) = '\0';
		c->val = malloc(sizeof(int));
		*(int*)c->val = atoi(cur);
		*(ours + matches[4].e) = oc;

		n = malloc(sizeof(*n));
		memset(n, 0, sizeof(*n));
		c->next = n;
		c = n;
		c->fieldname = strdup(TITLENAME);
		c->fmtname = strdup(TITLEFMT);
		c->type = string;
		cur = ours + matches[5].s;
		oc = *(ours + matches[5].e);
		*(ours + matches[5].e) = '\0';
		c->val = malloc(sizeof(int));
		c->val = strdup(cur);
		*(ours + matches[5].e) = oc;

		free(ours);
	}
	destroy_freg(tre);

	*tc = c;
	*th = h;

	return 1;
}

int add_int_field(name, fmtname, th, tc, value)
	char *name;
	char *fmtname;
	fields_t **th;
	fields_t **tc;
	int value;
{
	fields_t *n;

	n = malloc(sizeof(*n));
	memset(n, 0, sizeof(*n));
	n->type = number;

	if(*th == NULL) {
		*th = n;
		*tc = n;
	} else if(*tc != NULL) {
		(*tc)->next = n;
		*tc = n;
	} else {
		free(n);
		return 0;
	}

	n->fieldname = strdup(name);
	n->fmtname = strdup(fmtname);
	n->val = malloc(sizeof(value));
	*(int*)n->val = value;

	return 1;
}

static int add_str_field(name, fmtname, th, tc, value)
	char *name;
	char *fmtname;
	fields_t **th;
	fields_t **tc;
	char *value;
{
	fields_t *n;

	n = malloc(sizeof(*n));
	memset(n, 0, sizeof(*n));
	n->type = string;

	if(*th == NULL) {
		*th = n;
		*tc = n;
	} else if(*tc != NULL) {
		(*tc)->next = n;
		*tc = n;
	} else {
		free(n);
		return 0;
	}

	n->fieldname = strdup(name);
	n->fmtname = strdup(fmtname);
	n->val = value;

	return 1;
}

int add_image_field(name, fmtname, th, tc, value, sz)
	char *name;
	char *fmtname;
	fields_t **th;
	fields_t **tc;
	void *value;
	size_t sz;
{
	fields_t *n;

	n = malloc(sizeof(*n));
	memset(n, 0, sizeof(*n));
	n->type = image;

	if(*th == NULL) {
		*th = n;
		*tc = n;
	} else if(*tc != NULL) {
		(*tc)->next = n;
		*tc = n;
	} else {
		free(n);
		return 0;
	}

	n->fieldname = strdup(name);
	n->fmtname = strdup(fmtname);
	n->val = value;
	n->len = sz;

	return 1;
}


fields_t* extract_from_mp3(filename, errmsg)
	char *filename;
	char **errmsg;
{
	ID3Tag *t;
	ID3Frame *cfr;
	char *cv;
	fields_t *h = NULL, *c = NULL, *mime;

	if(!match_filename(filename, errmsg, &c, &h))
		return NULL;

	t = ID3Tag_New();

	ID3Tag_Link(t, filename);

	cfr = ID3Tag_FindFrameWithID(t, ID3FID_YEAR);
	if(cfr != NULL) {
		int tval;
		size_t tsz;
		ID3Field *tfield;
		tfield = ID3Frame_GetField(cfr, ID3FN_TEXT);
		tsz = ID3Field_Size(tfield);
		if(tsz > 0) {
			cv = malloc(sizeof(char) * 6); /* y10k compliant!!! */
			ID3Field_GetASCII(tfield, cv, 5);
			tval = atoi(cv);
			add_int_field(YEARNAME, YEARFMT, &h, &c, tval);
			free(cv);
		}
	}

	cfr = ID3Tag_FindFrameWithID(t, ID3FID_PICTURE); /* album cover */
	if(cfr != NULL) {
		void *tdta;
		size_t tsz;
		ID3Field *tfield;
		tfield = ID3Frame_GetField(cfr, ID3FN_PICTURETYPE);

		tsz = ID3Field_Size(tfield);

		if(tsz != 0) {
			tdta = malloc(tsz);
			ID3Field_GetBINARY(tfield, tdta, tsz);
			add_image_field(ACOVERNAME, ACOVERFMT, &h, &c, tdta, tsz);
		}
	}

	ID3Tag_Delete(t);

	mime = allocz(sizeof(*mime));
	mime->fieldname = strdup("mime");
	mime->fmtname = strdup("MIME type");
	mime->type = string;
	mime->val = strdup("audio/mpeg");
	mime->next = h;
	h = mime;

	return h;
}

#define CMTDELIM '='
#define CMT_ARTIST	0x1
#define CMT_ALBUM	0x2
#define CMT_TITLE	0x3
#define CMT_TRACK	0x4
#define CMT_YEAR	0x5
#define CMT_DISC	0x6
#define CMT_UNKNOWN	0xFF
static int comment_tagname(cstr, clen, rdat)
	char *cstr;
	size_t clen;
	char **rdat;
{
	char obf[clen + 1];
	char *equ, *val;
	int rv = CMT_UNKNOWN;
	
	strlcpy(obf, cstr, sizeof(obf));

	if((equ = strchr(obf, CMTDELIM)) == NULL)
		return CMT_UNKNOWN;

	*equ = '\0';
	val = equ + 1;

	if(strcasecmp(obf, "artist") == 0) {
		rv = CMT_ARTIST;
	} else if(strcasecmp(obf, "album") == 0) {
		rv = CMT_ALBUM;
	} else if(strcasecmp(obf, "title") == 0) {
		rv = CMT_TITLE;
	} else if(strcasecmp(obf, "tracknumber") == 0 || strcasecmp(obf, "track") == 0) {
		rv = CMT_TRACK;
	} else if(strcasecmp(obf, "date") == 0 || strcasecmp(obf, "year") == 0) {
		rv = CMT_YEAR;
	} else if(strcasecmp(obf, "disc") == 0) {
		rv = CMT_DISC;
	}
	
	if(rv != CMT_UNKNOWN)
		*rdat = strdup(val);

	return rv;
}

static int fields_from_vcomments(vf, errmsg, c, h)
	OggVorbis_File *vf;
	char **errmsg;
	fields_t **c, **h;
{
	char *cs;
	int i;
	vorbis_comment *vc;

	if((vc = ov_comment(vf, -1)) == NULL) {
		*errmsg = "ov_comment error";
		return 0;
	}

	for(i = 0; i < vc->comments; i++) {
		switch(comment_tagname(vc->user_comments[i], vc->comment_lengths[i], &cs)) {
			case CMT_ARTIST:
				add_str_field(ARTISTNAME, ARTISTFMT, h, c, cs);
				break;
			case CMT_ALBUM:
				add_str_field(ALBUMNAME, ALBUMFMT, h, c, cs);
				break;
			case CMT_TITLE:
				add_str_field(TITLENAME, TITLEFMT, h, c, cs);
				break;
			case CMT_TRACK:
				add_int_field(TRACKNAME, TRACKFMT, h, c, atoi(cs));
				free(cs);
				break;
			case CMT_DISC:
				add_int_field(DISCNAME, DISCFMT, h, c, atoi(cs));
				free(cs);
				break;
			case CMT_YEAR:
				add_int_field(YEARNAME, YEARFMT, h, c, atoi(cs));
				free(cs);
				break;
		}
	}
	
	return 1;
}

fields_t* extract_from_ogg(filename, errmsg)
	char *filename;
	char **errmsg;
{
	fields_t *h = NULL, *c = NULL, *mime;
	FILE *oggf;
	OggVorbis_File *vf;
	int rc;

	if(!(oggf = fopen(filename, "r"))) {
		char emsg[128];
		snprintf(emsg, 128, "Can't open %s for reading: %s", filename, strerror(errno));
		*errmsg = strdup(emsg);
		return NULL;
	}

	vf = malloc(sizeof(*vf));

	if((rc = ov_open(oggf, vf, NULL, 0)) < 0) {
		char *em;
		char emsg[128];
		switch(rc) {
			case OV_EREAD:
				em = "Could not read";
				break;
			case OV_ENOTVORBIS:
				em = "Not vorbis";
				break;
			case OV_EVERSION:
				em = "Vorbis version mismatch";
				break;
			case OV_EBADHEADER:
				em = "Invalid bitstream header";
				break;
			case OV_EFAULT:
				em = "Internal libvorbis fault";
				break;
			default:
				em = "Unknown error";
		}
		snprintf(emsg, 128, "Can't open %s with libvorbis: %s", filename, em);
		free(vf);
		*errmsg = strdup(emsg);
		return NULL;
	}

	rc = fields_from_vcomments(vf, errmsg, &c, &h);
	if(!rc) {
		char *oem;
		char emsg[128];
		oem = *errmsg;
		snprintf(emsg, 128, "Can't grab vorbis comments from file %s: %s", filename, oem);
		*errmsg = strdup(emsg);
		ov_clear(vf);
		free(vf);
		fclose(oggf);
		return h;
	}

	ov_clear(vf);
	free(vf);
	fclose(oggf);

	/* XXX: make this beautiful-er; if we can't get certain vorbis comment data, fall
	 * back on the filename
	 */
#if 0
	match_filename(filename, errmsg, &c, &h);
#endif

	mime = allocz(sizeof(*mime));
	mime->fieldname = strdup("mime");
	mime->fmtname = strdup("MIME type");
	mime->type = string;
	mime->val = strdup("application/ogg");
	mime->next = h;
	h = mime;

	return h;
}

static fields_t* extract_from_flac(filename, errmsg)
	char *filename;
	char **errmsg;
{
	fields_t *h = NULL, *c = NULL;

	/* TODO: implement FLAC vorbis comment stuff */
	
	add_str_field("mime", "MIME type", &h, &c, strdup("audio/x-flac"));
	match_filename(filename, errmsg, &c, &h);

	return h;
}
	

fields_t* extract_from_file(f, filename, errmsg)
	fdbfs_t *f;
	char *filename;
	char **errmsg;
{
	char *ext;
	fields_t *h = NULL, *c = NULL;

	ext = (filename + strlen(filename)) - strlen(MP3EXT);

	if(strcasecmp(ext, MP3EXT) == 0)
		return extract_from_mp3(filename, errmsg);
	
	ext = (filename + strlen(filename)) - strlen(OGGEXT);

	if(strcasecmp(ext, OGGEXT) == 0)
		return extract_from_ogg(filename, errmsg);

	ext = (filename + strlen(filename)) - strlen(WAVEXT);

	if(strcasecmp(ext, WAVEXT) == 0) {
		match_filename(filename, errmsg, &c, &h);
		add_str_field("mime", "MIME type", &h, &c, strdup("audio/x-wav"));
		return h;
	}

	ext = (filename + strlen(filename)) - strlen(FLACEXT);

	if(strcasecmp(ext, FLACEXT) == 0) {
		return extract_from_flac(filename, errmsg);
	}

	return NULL;
}
