#
# Portsearch
#
# $Id: Makefile,v 1.8 2006/01/16 08:40:09 dd Exp $
#

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
	xlibc.o

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

.include "../Makefile.var"

# EOF
