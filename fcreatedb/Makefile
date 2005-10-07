# $Amigan: fakedbfs/fcreatedb/Makefile,v 1.9 2005/10/07 20:51:18 dcp1990 Exp $
include ../config.mk
include ../globals.mk
LDFLAGS+=-L../libfakedbfs
LIBS=-lfakedbfs $(LIBEXT)
SRCS=fcreatedb.c
OBJS=fcreatedb.o
#OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=-ansi $(CPPFLAGS) # why?
#OBJS=sqlite.o dbspec.o dbinit.o lex.yy.o memory.o error.o indexing.o
all: .depend fcreatedb
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
fcreatedb: $(OBJS)
	$(CC) $(LDFLAGS) -o fcreatedb ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.core *.o *~ .depend fcreatedb
