/* configuration header */
/* $Amigan: fakedbfs/fdbfsconfig.h,v 1.1 2005/08/15 20:28:03 dcp1990 Exp $ */

#if defined(UNIX)
#define PREFIX "/usr/local"
#define LIBPATH PREFIX "/lib/fakedbfs"
#endif

#if defined(AMIGA)
#define PREFIX "FakeDBFS:"
#define LIBPATH "FakeDBFS:lib/plugins"
#endif

#if defined(WIN32)
#define PREFIX "C:/FAKEDBFS"
#define LIBPATH PREFIX "/PLUGINS"
#endif
