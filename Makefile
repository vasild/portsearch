#
# Portsearch
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
	${BSD_INSTALL_PROGRAM} src/portsearch $(DESTDIR)${PREFIX}/bin/
	${MKDIR} $(DESTDIR)${DATADIR}
	${BSD_INSTALL_DATA} Mk/Makefile $(DESTDIR)${DATADIR}/

.include "Makefile.var"

# EOF
