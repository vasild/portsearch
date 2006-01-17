#
# Portsearch
#
# $Id: Makefile,v 1.2 2006/01/17 07:21:03 dd Exp $
#

SUBDIRS=	src
SUBTARGETS=	build depend clean

.for target in ${SUBTARGETS}
${target}:
.for sub in ${SUBDIRS}
	${MAKE} -C ${sub} ${target}
.endfor
.endfor

install:
	${INSTALL_PROGRAM} src/portsearch ${PREFIX}/bin/
	${MKDIR} ${DATADIR}
	${INSTALL_DATA} Mk/Makefile ${DATADIR}/

.include "Makefile.var"

# EOF
