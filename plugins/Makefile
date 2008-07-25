# BSD $(MAKE) $(MAKE)file for the fakedbfs distribution's plugins
# (C)2005, Dan Ponte
# $Amigan: fakedbfs/plugins/Makefile,v 1.4 2008/07/25 17:40:31 dcp1990 Exp $
include ../config.mk
include ../globals.mk
COMPONENTS=music
all: all-rec
clean: clean-rec
all-rec:
	@for i in $(COMPONENTS) ; do $(MAKE) -C $$i -f ${PLUGMAKEF} ; done
clean-rec:
	@for i in $(COMPONENTS) ; do $(MAKE) -C $$i -f ${PLUGMAKEF} clean ; done
