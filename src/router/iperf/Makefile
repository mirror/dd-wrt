##################################################
# Makefile
# iperf top level
#
# by Mark Gates <mgates@nlanr.net>
# October 1999
# $Id: Makefile,v 1.6 2003/03/12 20:57:31 kgibbs Exp $

SHELL = /bin/sh

TOP = .

##################################################
# files

LIBRARY        = lib/libdast.a

PROG           = iperf

JAR_FILE       = jperf.jar

CONFIG_FILES   = Makefile.rules cfg/config.h cfg/configure
PLATFORM = `./cfg/config.guess`

##################################################
# targets
#
# all       -- compile library and iperf program
# config    -- run autoconf script
# clean     -- remove all compiled object files
# depend    -- create dependency files
# distclean -- remove most files that can be generated
# install   -- compile and install library and headers
# tar       -- tar up the directory, skipping machine specific files
#

all: $(LIBRARY) $(PROG)

foo: 
	@echo $(PLATFORM)
	UNAME = `uname -a`
	@echo $(UNAME)

clean:
	@echo ; (cd cfg  ; make clean)
	@echo ; (cd lib  ; make clean)
	@echo ; (cd src  ; make clean)

config:
	@echo ; (cd cfg  ; make all)

depend:
	@echo ; (cd lib  ; make depend)
	@echo ; (cd src  ; make depend)

distclean:
	@echo ; (cd cfg  ; make distclean)
	@echo ; (cd lib  ; make distclean)
	@echo ; (cd src  ; make distclean)

install: $(PROG)
	install -D $(PROG) $(INSTALLDIR)/usr/sbin/$(PROG)

bindist: all
	@PLATFORM=`./cfg/config.guess`;         \
	FILE=iperf-$(VERSION)-$$PLATFORM.tar;   \
	( tar -cvf $$FILE iperf doc README KNOWN_PROBLEMS;  \
	  gzip $$FILE;                          \
	  echo Saved at $$FILE.gz;              \
	);
tar:
	@echo
	@DIR=`pwd | sed 's|^.*/||'`;                                      \
	DIRV=$$DIR-$(VERSION);                                            \
	( cd ..;                                                          \
	  mv $$DIR $$DIRV;                                                \
	  tar --exclude-from $$DIRV/.tar.excludes -cvf $$DIRV.tar $$DIRV; \
	  gzip $$DIRV.tar;                                                \
	  mv $$DIRV $$DIR;                                                \
	);                                                                \
	if test -w RELEASES/source/ ; then                                \
		mv -i ../$$DIRV.tar.gz RELEASES/source/;                        \
		echo Saved in RELEASES/source/$$DIRV.tar.gz;                    \
	else                                                              \
		mv -i ../$$DIRV.tar.gz .;                                       \
		echo Saved in $$DIRV.tar.gz;                                    \
	fi

.PHONY: all config clean depend distclean install tar

##################################################
# build specific files

$(LIBRARY):
	-rm -f $(LIBRARY)
	@echo ; (cd lib  ; make all)

$(PROG):
	-rm -f $(PROG)
	@echo ; (cd src  ; make all)

Makefile.rules: Makefile.rules.in
	make config

cfg/configure: cfg/configure.in cfg/dast.m4
	make config

# The configure script won't update config.h if it doesn't
# changed, so remove config.h first to force rebuilding it.
cfg/config.h: cfg/config.h.in
	-rm -f cfg/config.h
	make config

# Version info maintained in VERSION, but we want to
# compile it into iperf, so create a header file too.
include VERSION

src/version.h: VERSION
	@echo Update version information to $(VERSION), $(VERSION_DATE)
	@echo \#define IPERF_VERSION \"$(VERSION)\" > .ver
	@echo \#define IPERF_VERSION_DATE \"$(VERSION_DATE)\" >> .ver
	@mv -f .ver $@

# some versions of make apparently can't handle expanding
# wildcards like lib/*.[ch] lib/*.[ch]pp

$(LIBRARY): $(CONFIG_FILES) \
	lib/Condition.hpp         \
	lib/Mutex.hpp             \
	lib/Socket.cpp            \
	lib/Socket.hpp            \
	lib/SocketAddr.cpp        \
	lib/SocketAddr.hpp        \
	lib/Thread.cpp            \
	lib/Thread.hpp            \
	lib/Timestamp.hpp         \
	lib/delay.cpp             \
	lib/delay.hpp             \
	lib/error.c               \
	lib/gettimeofday.c        \
	lib/gettimeofday.h        \
	lib/gnu_getopt.c          \
	lib/gnu_getopt.h          \
	lib/gnu_getopt_long.c     \
	lib/headers.h             \
	lib/inet_pton.c           \
	lib/inet_ntop.c           \
	lib/inet_aton.h           \
	lib/signal.c              \
	lib/snprintf.c            \
	lib/snprintf.h            \
	lib/sockets.c             \
	lib/stdio.c               \
	lib/string.c              \
	lib/tcp_window_size.c     \
	lib/util.h

$(PROG): $(LIBRARY)               \
	$(CONFIG_FILES)           \
	src/Client.cpp            \
	src/Client.hpp            \
	src/Extractor.cpp         \
	src/Extractor.hpp         \
	src/Listener.cpp          \
	src/Listener.hpp          \
	src/Locale.hpp            \
	src/PerfSocket.cpp        \
	src/PerfSocket.hpp        \
	src/PerfSocket_TCP.cpp    \
	src/PerfSocket_UDP.cpp    \
	src/Server.cpp            \
	src/Server.hpp            \
	src/Settings.cpp          \
	src/Settings.hpp          \
	src/Speaker.cpp           \
	src/Speaker.hpp           \
	src/Notify.cpp            \
	src/Notify.hpp            \
	src/Audience.cpp          \
	src/Audience.hpp          \
	src/List.cpp              \
	src/List.h                \
	src/main.cpp              \
	src/version.h
