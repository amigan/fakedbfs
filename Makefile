# BSD make makefile for the fakedbfs distribution
# (C)2005, Dan Ponte
# $Amigan: fakedbfs/Makefile,v 1.3 2005/08/14 08:07:49 dcp1990 Exp $
include globals.mk
COMPONENTS=buildtools libfakedbfs fcreatedb
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
