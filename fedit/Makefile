# $Amigan: fakedbfs/fedit/Makefile,v 1.2 2005/11/19 17:09:36 dcp1990 Exp $
include ../config.mk
include ../globals.mk
MAINBIN=fedit
LDFLAGS+=-L../libfakedbfs
LIBS+=-lfakedbfs $(LIBEXT)
SRCS=fedit.c commands.c
OBJS=fedit.o commands.o
CFLAGS+=-ansi $(CPPFLAGS) # why?
all: .depend ${MAINBIN}
.depend: $(SRCS)
	mkdep $(CPPFLAGS) $(SRCS)
${MAINBIN}: $(OBJS)
	$(CC) $(LDFLAGS) -o ${MAINBIN} ${OBJS} $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f *.core *.o *~ .depend ${MAINBIN}
