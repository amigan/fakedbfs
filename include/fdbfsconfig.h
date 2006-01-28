/* configuration header */
/* $Amigan: fakedbfs/include/fdbfsconfig.h,v 1.4 2006/01/28 22:35:33 dcp1990 Exp $ */

#ifndef HAVE_FDBFSCONFIG_H

#ifdef HAVE_CONFIG
#include "../config.h"
#else

#	if defined(UNIX)
#		define PREFIX "/usr/local"
#		define LIBPATH PREFIX "/lib/fakedbfs"
#	endif

#	if defined(AMIGA)
#		define PREFIX "FakeDBFS:"
#		define LIBPATH "FakeDBFS:lib/plugins"
#	endif

#	if defined(WIN32)
#		define PREFIX "C:/FAKEDBFS"
#		define LIBPATH PREFIX "/PLUGINS"
#	endif

#endif

#define DEFAULT_STACKSIZE	30 /* don't make this less than the max. number of fields you have, plus others,
				      plus at least 1 for the fields_t* after a step (maybe more if you're lazy and
				      don't pop it off right after! */

#define CRAWLFRAME_MAX		20 /* this takes a LOT of memory...this many crawlframe_t's per directory frame...it's just a tradeoff between being able to
				      crawl more directories or using less memory....once the hybrid system is finished (depth and breadth), this will be
				      less of an issue */
#define BUSY_RETRIES		3	/* number of times to retry upon a trappable SQLITE_BUSY exception...don't make this
					   too big! */
#endif
