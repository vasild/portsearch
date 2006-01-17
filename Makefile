#
# Portsearch
#
# $Id: Makefile,v 1.3 2006/01/17 07:49:39 dd Exp $
#

SUBDIRS=	src
SUBTARGETS=	build depend clean

.for target in ${SUBTARGETS}
${target}:
.for sub in ${SUBDIRS}
	${MAKE} -C ${sub} ${target}
.endfor
.endfor

install: build
	${INSTALL_PROGRAM} src/portsearch ${PREFIX}/bin/
	${MKDIR} ${DATADIR}
	${INSTALL_DATA} Mk/Makefile ${DATADIR}/

.include "Makefile.var"

# EOF
