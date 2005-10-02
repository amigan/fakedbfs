# $Amigan: fakedbfs/fcreatedb/Makefile,v 1.7 2005/10/02 16:43:09 dcp1990 Exp $
include ../config.mk
include ../globals.mk
LDFLAGS+=-L../libfakedbfs
LIBS=-lfakedbfs $(LIBEXT)
SRCS=fcreatedb.c
OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=-ansi $(CPPFLAGS) # why?
#OBJS=sqlite.o dbspec.o dbinit.o lex.yy.o memory.o error.o indexing.o
all: .depend fcreatedb
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
fcreatedb: $(OBJS)
	$(CC) $(LDFLAGS) -o fcreatedb ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c ${.SOURCE}
clean:
	rm -f *.core *.o *~ .depend fcreatedb
