# BSD make makefile for the fakedbfs distribution's plugins
# (C)2005, Dan Ponte
# $Amigan: fakedbfs/plugins/Makefile,v 1.1 2005/08/24 06:23:34 dcp1990 Exp $
include ../globals.mk
COMPONENTS=music
COMPSUF=${COMPONENTS:S/$/_cmp/}
CMPSCLEAN=${COMPONENTS:S/$/_cl/}
all: ${COMPSUF}
${COMPSUF}:
	cd ${@:S/_cmp$//} && $(MAKE)
clean: ${CMPSCLEAN}
${CMPSCLEAN}:
	cd ${@:S/_cl$//} && $(MAKE) clean
