/* configuration header */
/* $Amigan: fakedbfs/include/fakedbfs/fdbfsconfig.h,v 1.2 2005/10/02 16:43:14 dcp1990 Exp $ */

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
#endif
