# $Amigan: fakedbfs/libfakedbfs/Makefile,v 1.18 2005/10/02 16:43:15 dcp1990 Exp $
include ../config.mk
include ../globals.mk
CC=cc
CPPFLAGS+=-DYY_NO_UNPUT
LIBS=-lsqlite3 -lfl -lm $(LIBEXT)
CFLAGS+=-ansi
SRCS=libfakedbfs.c sqlite.c dbspec.c dbinit.c lex.yy.c memory.c error.c indexing.c debug.c plugins.c query.c queryparser.c qtokenise.c
OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=$(CPPFLAGS) # why?
#OBJS=sqlite.o dbspec.o dbinit.o lex.yy.o memory.o error.o indexing.o
all: .depend libfakedbfs.so libfakedbfs.a
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
lex.yy.c: dbspec.l
	flex dbspec.l
queryparser.c: queryparser.y
	../buildtools/lemon -s queryparser.y
dbspec.c: dbspec.y
	../buildtools/lemon -s dbspec.y
libfakedbfs.so: $(OBJS)
	$(CC) $(LDFLAGS) -shared -o libfakedbfs.so ${OBJS} $(LIBS)
libfakedbfs.a: $(OBJS)
	ar rc libfakedbfs.a $(OBJS)
	ranlib libfakedbfs.a
%.o: %.c
	$(CC) $(CFLAGS) -c ${.SOURCE}
lint:
	lint -s $(CPPFLAGS) $(SRCS)
install: all
	install libfakedbfs.so /usr/local/lib
clean:
	rm -f *.core *.o *~ .depend dbspec.c dbspec.h dbspec.out libfakedbfs.so lex.yy.c libfakedbfs.a queryparser.c queryparser.h queryparser.out
