# $Amigan: fakedbfs/fquery/Makefile,v 1.6 2005/10/07 20:42:18 dcp1990 Exp $
include ../config.mk
include ../globals.mk
LDFLAGS+=-L../libfakedbfs
LIBS+=-lfakedbfs $(LIBEXT)
SRCS=fquery.c
OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=-ansi $(CPPFLAGS) # why?
#OBJS=sqlite.o dbspec.o dbinit.o lex.yy.o memory.o error.o indexing.o
all: .depend fquery
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
fquery: $(OBJS)
	$(CC) $(LDFLAGS) -o fquery ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.core *.o *~ .depend fquery
