# BSD make makefile for the fakedbfs distribution
# (C)2005, Dan Ponte
# $Amigan: fakedbfs/Makefile,v 1.4 2005/08/17 15:30:00 dcp1990 Exp $
include globals.mk
COMPONENTS=buildtools libfakedbfs fcreatedb findex
COMPSUF=${COMPONENTS:S/$/_cmp/}
CMPSCLEAN=${COMPONENTS:S/$/_cl/}
CLEANFILES=.config
all: ${COMPSUF}
${COMPSUF}: .config
	cd ${@:S/_cmp$//} && $(MAKE)
clean: ${CMPSCLEAN} ourclean
${CMPSCLEAN}:
	cd ${@:S/_cl$//} && $(MAKE) clean
#nothing yet...
.config:
	touch .config
ourclean:
	rm -f ${CLEANFILES}
