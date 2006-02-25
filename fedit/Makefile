# $Amigan: fakedbfs/fedit/Makefile,v 1.3 2006/02/25 09:52:13 dcp1990 Exp $
include ../config.mk
include ../globals.mk
MAINBIN=fedit
LDFLAGS+=-L../libfakedbfs
LIBS+=-lfakedbfs $(LIBEXT)
SRCS=fedit.c commands.c
OBJS=fedit.o commands.o
# we need -std=c99 on FreeBSD so we can get the strtoll stuff
CFLAGS+=-std=c99 $(CPPFLAGS) # why?
all: .depend ${MAINBIN}
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
${MAINBIN}: $(OBJS)
	$(CC) $(LDFLAGS) -o ${MAINBIN} ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.core *.o *~ .depend ${MAINBIN}
