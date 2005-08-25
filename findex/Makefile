# $Amigan: fakedbfs/findex/Makefile,v 1.2 2005/08/25 07:20:35 dcp1990 Exp $
include ../globals.mk
LDFLAGS+=-L../libfakedbfs
LIBS=-lfakedbfs
SRCS=findex.c
OBJS!=echo ${SRCS} | sed -e 's/\.c/.o/g'
CFLAGS+=-ansi $(CPPFLAGS) # why?
#OBJS=sqlite.o dbspec.o dbinit.o lex.yy.o memory.o error.o indexing.o
all: .depend findex
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
findex: $(OBJS)
	$(CC) $(LDFLAGS) -o findex ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c ${.SOURCE}
clean:
	rm -f *.core *.o *~ .depend findex
