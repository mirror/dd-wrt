SOURCES = \
	base64.c \
	dom.c \
	filter.c \
	iks.c \
	ikstack.c \
	io-posix.c \
	jabber.c \
	md5.c \
	sax.c \
	sha.c \
	stream.c \
	utility.c

EXTRA_CFLAGS = -fPIC
EXTRA_INCLUDES  = $(INCLIBC) -I.

LIBNAME = iksemel

include $(ROCKBASE)/make_include/lib.mk

.PHONY : etc
etc ::
	/bin/true
