# $Amigan: fakedbfs/libfakedbfs/Makefile,v 1.31 2008/08/04 04:38:24 dcp1990 Exp $
include ../config.mk
include ../globals.mk
CC=cc
CPPFLAGS+=-DYY_NO_UNPUT
LIBS=-lsqlite3 -lfl -lm ../ficl/libficl.a $(LIBEXT)
CFLAGS+=-ansi
SRCS=libfakedbfs.c sqlite.c dbinit.c lex.fdbfs_ds.c memory.c error.c indexing.c debug.c plugins.c query.c queryparser.c qtokenise.c fregex.c crawler.c \
	defspecs.c dspparser.c ficl.c conf.c fields.c
OBJS=libfakedbfs.o sqlite.o dbspec.o dbinit.o lex.fdbfs_ds.o memory.o error.o indexing.o debug.o plugins.o query.o queryparser.o qtokenise.o fregex.o crawler.o \
	defspecs.o dspparser.o ficl.o conf.o fields.o
#OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=$(CPPFLAGS) # why?
all: ../ficl/libficl.a .depend libfakedbfs.so libfakedbfs.a
.depend: $(SRCS) dbspec.h
	mkdep $(CPPFLAGS) $(SRCS)
lex.fdbfs_ds.c: dbspec.l
	flex -Pfdbfs_ds dbspec.l
queryparser.c: queryparser.y
	../buildtools/lemon -s queryparser.y
dbspec.h: dbspec.y
	../buildtools/lemon -s dbspec.y
dbspec.c: dbspec.h
dspparser.c: dspparser.y
	../buildtools/lemon -s dspparser.y
libfakedbfs.so: .depend $(OBJS)
	if [ ${PLATFORM} = macosx ] ; then libtool -dynamic -o libfakedbfs.so -arch_only i386 ${OBJS} $(LIBS) ; else  $(CC) $(LDFLAGS) -shared -o libfakedbfs.so ${OBJS} $(LIBS); fi
# below is for osx
#libfakedbfs.so: .depend $(OBJS)
#	libtool -dynamic -o libfakedbfs.so -arch_only i386 ${OBJS} $(LIBS)
libfakedbfs.a: $(OBJS)
	ar rc libfakedbfs.a $(OBJS)
	ranlib libfakedbfs.a
%.o: %.c
	$(CC) $(CFLAGS) -c $<
lint:
	lint -s $(CPPFLAGS) $(SRCS)
install: all
	install libfakedbfs.so ${PREFIX}/lib
clean:
	rm -f *.core *.o *~ .depend dbspec.c dbspec.h dbspec.out libfakedbfs.so lex.fdbfs_ds.c libfakedbfs.a queryparser.c queryparser.h queryparser.out \
		dspparser.c dspparser.h dspparser.out
