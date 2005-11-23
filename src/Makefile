#
# Portsearch
#
# $Id: Makefile,v 1.1 2005/11/23 06:41:47 dd Exp $
#

CFLAGS+=	-Wall -Werror

PROGS=	portsearch

build: ${PROGS}

clean:
	rm -f *.o portsearch *.core

portsearch:
	${CC} ${CFLAGS} ${@}.c -o ${@}

# EOF
