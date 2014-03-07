# This is a stripped version of generate-plist target from bsd.port.mk as of
# $FreeBSD: head/Mk/bsd.port.mk 346930 2014-03-03 16:50:32Z makc $
# It is used to send plist to stdout.
# The idea is to speed up things, otherwise we could use
# make generate-plist && cat `make -V TMPPLIST` && make clean
# in each port's directory.
#

show-plist:
	@for file in ${PLIST_FILES}; do \
		${ECHO_CMD} $${file} | ${SED} ${PLIST_SUB:S/$/!g/:S/^/ -e s!%%/:S/=/%%!/} ; \
	done
.if defined(NO_STAGE)
	@for man in ${__MANPAGES}; do \
		${ECHO_CMD} $${man} ; \
	done
.for _PREFIX in ${PREFIX}
.if ${_TMLINKS:M${_PREFIX}*}x != x
	@for i in ${_TMLINKS:M${_PREFIX}*:S|^${_PREFIX}/||}; do \
		${ECHO_CMD} "$$i" ; \
	done
.endif
.if ${_TMLINKS:N${_PREFIX}*}x != x
	@for i in ${_TMLINKS:N${_PREFIX}*:S|^/||}; do \
		${ECHO_CMD} "$$i" ; \
	done
.endif
.endfor
.endif
	@if [ -f "${PLIST}" ]; then \
		${SED} ${PLIST_SUB:S/$/!g/:S/^/ -e s!%%/:S/=/%%!/} ${PLIST} ; \
	fi

# EOF
