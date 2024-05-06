# Confession time:
#
# I haven't the slightliest clue how to convert this Makefile into a proper
# Makefile.am. By default, libtool (from the autotools) creates a lot of
# auxiliary files, and they don't seem to play well with iptables. I tried
# finding example iptables projects that use autotools, and I couldn't find
# anything. (Part of the problem is that I don't know how to look for them, or
# whether they exist.)
#
# I don't even know if it can be done to begin with, because autotools is
# supposed to be a Unix standard thing, while iptables is a Linux-specific
# thing. If the autotools make too many assumptions regarding the use of these
# shared objects, I suppose it might render them uniptablable.
#
# So I don't know if the problem can be fixed, but if it does, I'll probably
# need to read the entire libtool manual to figure it out. But it's like a
# million pages long. I don't even know what I'm supposed to be looking for:
# https://www.gnu.org/software/libtool/manual/libtool.html
# TODO But I mean, I should probably read it anyway.

DEFAULT_CFLAGS  = -Wall -pedantic -std=gnu11 -O2
DEFAULT_CFLAGS += -I../..
DEFAULT_CFLAGS += $(shell pkg-config xtables --cflags)

XTABLES_SO_DIR = $(shell pkg-config xtables --variable xtlibdir)

all:
	make libxt_JOOL_SIIT.so;
	make libxt_JOOL.so;
install:
	mkdir -p ${DESTDIR}${XTABLES_SO_DIR}
	cp *.so ${DESTDIR}${XTABLES_SO_DIR}
uninstall:
	rm -f ${DESTDIR}${XTABLES_SO_DIR}/libxt_JOOL_SIIT.so
	rm -f ${DESTDIR}${XTABLES_SO_DIR}/libxt_JOOL.so
lib%.so: lib%.o
	${CC} -shared -fPIC ${LDFLAGS} -o $@ $^;
lib%.o: lib%.c
	${CC} ${DEFAULT_CFLAGS} ${CPPFLAGS} ${CFLAGS} -D_INIT=lib$*_init -fPIC -c -o $@ $<;
clean distclean maintainer-clean:
	rm -f *.so
distdir:
	mkdir -p ${distdir}
	cp *.c *.man Makefile ${distdir}
