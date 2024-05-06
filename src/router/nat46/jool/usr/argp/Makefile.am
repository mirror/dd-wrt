noinst_LTLIBRARIES = libjoolargp.la

libjoolargp_la_SOURCES = \
	command.c command.h \
	dns.c dns.h \
	log.c log.h \
	main.c main.h \
	requirements.c requirements.h \
	userspace-types.c userspace-types.h \
	wargp.c wargp.h \
	xlator_type.c xlator_type.h \
	\
	wargp/address.c wargp/address.h \
	wargp/bib.c wargp/bib.h \
	wargp/denylist4.c wargp/denylist4.h \
	wargp/eamt.c wargp/eamt.h \
	wargp/file.c wargp/file.h \
	wargp/global.c wargp/global.h \
	wargp/instance.c wargp/instance.h \
	wargp/joold.c wargp/joold.h \
	wargp/pool4.c wargp/pool4.h \
	wargp/session.c wargp/session.h \
	wargp/stats.c wargp/stats.h

libjoolargp_la_CFLAGS  = ${WARNINGCFLAGS}
libjoolargp_la_CFLAGS += -I${top_srcdir}/src
libjoolargp_la_CFLAGS += ${LIBNLGENL3_CFLAGS}
if !XTABLES_ENABLED
libjoolargp_la_CFLAGS += -DXTABLES_DISABLED
endif

libjoolargp_la_LIBADD  = ../util/libjoolutil.la
libjoolargp_la_LIBADD += ../nl/libjoolnl.la
