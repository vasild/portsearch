#
# Portsearch
#
# $Id: Makefile,v 1.5 2006/01/10 09:23:47 dd Exp $
#

MKDEP?=	/usr/bin/mkdep
RM?=	/bin/rm

CFLAGS+=	-Wall -Werror

CFLAGS:=	${CFLAGS:C/-O[023s]//g} -g

PROGS=\
	get_port_mtime_main \
	portsearch \
	vector_main

# get_port_mtime_main
get_port_mtime_main_objs=\
	get_port_mtime.o \
	get_port_mtime_main.o

# portsearch
portsearch_objs=\
	display.o \
	execcmd.o \
	exhaust_fp.o \
	get_port_mtime.o \
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

test: test_get_port_mtime

clean:
	${RM} -f *.o *.core ${PROGS} .depend

#

test_get_port_mtime: get_port_mtime_main
	@./get_port_mtime_main.sh
	@echo "${@} passed"

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
