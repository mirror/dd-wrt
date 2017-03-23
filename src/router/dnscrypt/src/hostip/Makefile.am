
bin_PROGRAMS = \
	hostip

hostip_SOURCES = \
	app.c \
	app.h \
	options.c \
	options.h

AM_CFLAGS = @CWFLAGS@

AM_CPPFLAGS = \
	-I../libevent-modified/include

hostip_LDADD = \
	../libevent-modified/libevent_extra.la \
	../libevent-modified/libevent_core.la
