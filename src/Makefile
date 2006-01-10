#
# Portsearch
#
# $Id: Makefile,v 1.6 2006/01/10 10:48:46 dd Exp $
#

MKDEP?=	/usr/bin/mkdep
RM?=	/bin/rm

CFLAGS+=	-Wall -Werror

CFLAGS:=	${CFLAGS:C/-O[023s]//g} -g

PROGS=\
	portsearch \
	vector_main

# portsearch
portsearch_objs=\
	display.o \
	execcmd.o \
	exhaust_fp.o \
	logmsg.o \
	mkdb.o \
	portsearch.o \
	store_txt.o \
	vector.o \
	xmalloc.o

# vector
vector_main_objs=\
	vector.o \
	vector_main.o

#
# define targets
#

build: ${PROGS}

depend:
	${MKDEP} ${CFLAGS} *.c

clean:
	${RM} -f *.o *.core ${PROGS} .depend

#

.for prog in ${PROGS}

.for obj in ${${prog}_objs}
.if !target(${obj})
${obj}:
	${CC} ${CFLAGS} -c ${obj:.o=.c} -o ${obj}
.endif
.endfor

${prog}: ${${prog}_objs}
	${CC} ${LDFLAGS} ${${prog}_objs} -o ${prog}

.endfor

# EOF
