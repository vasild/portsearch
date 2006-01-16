# stripped version of generate-plist from bsd.port.mk 1.518
# to send plist to stdout
# the idea is to speed up the things, otherwise we could use
# make generate-plist && cat `make -V TMPPLIST` && make clean
#
# $Id: Makefile,v 1.1 2006/01/16 08:51:32 dd Exp $
#

show-plist:
	@for file in ${PLIST_FILES}; do \
		${ECHO_CMD} $${file} | ${SED} ${PLIST_SUB:S/$/!g/:S/^/ -e s!%%/:S/=/%%!/} ; \
	done
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
	@if [ -f ${PLIST} ]; then \
		${SED} ${PLIST_SUB:S/$/!g/:S/^/ -e s!%%/:S/=/%%!/} ${PLIST} ; \
	fi

# EOF
