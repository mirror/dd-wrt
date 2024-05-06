AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = graybox
graybox_SOURCES = \
	graybox.c \
	genetlink.c genetlink.h \
	log.c log.h \
	command/expect.c command/expect.h \
	command/send.c command/send.h \
	command/stats.c command/stats.h \
	command/common.c command/common.h

graybox_CFLAGS = -Wall -pedantic -I${srcdir}/.. ${LIBNLGENL3_CFLAGS}
graybox_LDADD  = ${LIBNLGENL3_LIBS}

man_MANS = graybox.7
