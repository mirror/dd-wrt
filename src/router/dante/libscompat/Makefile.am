#NOTE: This makefile is now only used for unit tests, the other
#      source dirs include the source files directly when building.
noinst_LTLIBRARIES = libscompat.la
libscompat_la_SOURCES = daemon.c difftime.c getifaddrs.c        \
   hstrerror.c inet_ntoa.c inet_pton.c issetugid.c              \
   memmove.c pselect.c seteuid.c setproctitle.c sockatmark.c    \
   strlcpy.c strvis.c vsyslog.c

noinst_HEADERS = ifaddrs.h timers.h vis_compat.h

#in_cksum; not used in libscompat, only placed here for license reasons
EXTRA_DIST = in_cksum.h in_cksum.c

INCLUDES = -I$(top_srcdir)/include -I.

AM_CFLAGS = -DSOCKS_CLIENT=0 -DSOCKS_SERVER=0 -DSOCKSLIBRARY_DYNAMIC=0

#lint files
LINT = @LINT@
LINTFLAGS = @LINTFLAGS@

if RUNLINT
SUFFIXES = .ln
#XXX add in_cksum directly, not built in this dir, but linting simplest here
LINTFILES = $(LIBSCSRC:.lo=.ln) in_cksum.ln
LIBDEPS = $(LINTFILES) llib-lscompat.ln

CLEANFILES = $(LINTFILES)

.c.ln:
	$(LINT) $(LINTPASS1) $(LINTFLAGS) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(AM_CFLAGS) $< > $@.lint 2>&1

llib-lscompat.ln: $(LINTFILES)
	$(LINT) $(LINTPASS2) $(LINTFLAGS) $(LINTLIBOPT)scompat $(LINTFILES) > $@.lint 2>&1
endif #RUNLINT

libscompat_la_DEPENDENCIES = $(LIBDEPS)
