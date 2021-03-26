# This is a stripped version of generate-plist target from bsd.port.mk as of
# $FreeBSD: head/Mk/bsd.port.mk 568966 2021-03-22 15:13:55Z mat $
# It is used to send plist to stdout.
# The idea is to speed up things, otherwise we could use
# make generate-plist && cat `make -V TMPPLIST` && make clean
# in each port's directory.
#

show-plist:
	@for file in ${PLIST_FILES}; do \
		${ECHO_CMD} $${file} | ${SED} ${PLIST_SUB_SANITIZED:S/$/!g/:S/^/ -e s!%%/:S/=/%%!/} ; \
	done
.if !empty(PLIST)
.for f in ${PLIST}
	@if [ -f "${f}" ]; then \
		${SED} ${PLIST_SUB_SANITIZED:S/$/!g/:S/^/ -e s!%%/:S/=/%%!/} ${f} ; \
	fi
.endfor
.endif
.for dir in ${PLIST_DIRS}
	@${ECHO_CMD} ${dir} | ${SED} ${PLIST_SUB_SANITIZED:S/$/!g/:S/^/ -e s!%%/:S/=/%%!/} -e 's,^,@dir ,'
.endfor

# EOF
