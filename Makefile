#
# Portsearch
#
# $Id: Makefile,v 1.1 2006/01/16 08:40:09 dd Exp $
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
	${INSTALL_PROGRAM} src/portsearch ${PREFIX}/bin
	${INSTALL_DATA} Mk/Makefile ${DATADIR}

.include "Makefile.var"

# EOF
