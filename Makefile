#
# Portsearch
#
# $Id: Makefile,v 1.4 2006/01/31 06:30:40 dd Exp $
#

SUBDIRS=	src
SUBTARGETS=	build depend clean

.for target in ${SUBTARGETS}
${target}:
.for sub in ${SUBDIRS}
	${MAKE} -C ${sub} ${target}
.endfor
.endfor

all: build

install: build
	${INSTALL_PROGRAM} src/portsearch ${PREFIX}/bin/
	${MKDIR} ${DATADIR}
	${INSTALL_DATA} Mk/Makefile ${DATADIR}/

.include "Makefile.var"

# EOF
