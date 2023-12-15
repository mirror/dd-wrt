# liblsof
lib_LTLIBRARIES = liblsof.la

liblsof_la_SOURCES = lib/ckkv.c lib/cvfs.c lib/dvch.c lib/fino.c lib/isfn.c lib/lkud.c lib/lsof.c lib/misc.c lib/node.c lib/pdvn.c lib/prfp.c lib/print.c lib/proc.c lib/ptti.c lib/rdev.c lib/rnmt.c lib/rmnt.c lib/rnam.c lib/rnch.c lib/rnmh.c
liblsof_la_SOURCES += lib/common.h lib/proto.h lib/hash.h
include_HEADERS = include/lsof.h include/lsof_fields.h

# Hide internal functions
AM_CFLAGS = -fvisibility=hidden

DIALECT_ROOT = $(top_srcdir)/lib/dialects
DIALECT_PATH = $(DIALECT_ROOT)/$(LSOF_DIALECT_DIR)

# Dialect specific sources
if LINUX
liblsof_la_SOURCES += lib/dialects/linux/dfile.c \
		lib/dialects/linux/dmnt.c \
		lib/dialects/linux/dnode.c \
		lib/dialects/linux/dproc.c \
		lib/dialects/linux/dsock.c \
		lib/dialects/linux/dstore.c \
		lib/dialects/linux/dlsof.h \
		lib/dialects/linux/dproto.h \
		lib/dialects/linux/machine.h
endif

if DARWIN
liblsof_la_SOURCES += lib/dialects/darwin/ddev.c \
		lib/dialects/darwin/dfile.c \
		lib/dialects/darwin/dmnt.c \
		lib/dialects/darwin/dproc.c \
		lib/dialects/darwin/dsock.c \
		lib/dialects/darwin/dstore.c \
		lib/dialects/darwin/dlsof.h \
		lib/dialects/darwin/dproto.h \
		lib/dialects/darwin/machine.h
endif

if FREEBSD
liblsof_la_SOURCES += lib/dialects/freebsd/dmnt.c \
		lib/dialects/freebsd/dnode.c \
		lib/dialects/freebsd/dproc.c \
		lib/dialects/freebsd/dsock.c \
		lib/dialects/freebsd/dstore.c \
		lib/dialects/freebsd/dlsof.h \
		lib/dialects/freebsd/dproto.h \
		lib/dialects/freebsd/machine.h
endif

if NETBSD
liblsof_la_SOURCES += lib/dialects/netbsd/dmnt.c \
		lib/dialects/netbsd/dnode.c \
		lib/dialects/netbsd/dproc.c \
		lib/dialects/netbsd/dsock.c \
		lib/dialects/netbsd/dstore.c \
		lib/dialects/netbsd/dlsof.h \
		lib/dialects/netbsd/dproto.h \
		lib/dialects/netbsd/machine.h
endif

if OPENBSD
liblsof_la_SOURCES += lib/dialects/openbsd/dfile.c \
		lib/dialects/openbsd/dmnt.c \
		lib/dialects/openbsd/dnode.c \
		lib/dialects/openbsd/dproc.c \
		lib/dialects/openbsd/dsock.c \
		lib/dialects/openbsd/dstore.c \
		lib/dialects/openbsd/dlsof.h \
		lib/dialects/openbsd/dproto.h \
		lib/dialects/openbsd/machine.h
endif

if SOLARIS
liblsof_la_SOURCES += lib/dialects/sun/ddev.c \
		lib/dialects/sun/dfile.c \
		lib/dialects/sun/dmnt.c \
		lib/dialects/sun/dnode.c \
		lib/dialects/sun/dproc.c \
		lib/dialects/sun/dsock.c \
		lib/dialects/sun/dstore.c \
		lib/dialects/sun/dlsof.h \
		lib/dialects/sun/dproto.h \
		lib/dialects/sun/machine.h
endif

if AIX
liblsof_la_SOURCES += lib/dialects/aix/ddev.c \
		lib/dialects/aix/dfile.c \
		lib/dialects/aix/dmnt.c \
		lib/dialects/aix/dnode.c \
		lib/dialects/aix/dnode2.c \
		lib/dialects/aix/dproc.c \
		lib/dialects/aix/dsock.c \
		lib/dialects/aix/dstore.c \
		lib/dialects/aix/dlsof.h \
		lib/dialects/aix/dproto.h \
		lib/dialects/aix/machine.h
endif

# Binary
bin_PROGRAMS = lsof

lsof_SOURCES = src/arg.c src/main.c src/print.c src/ptti.c src/store.c src/usage.c src/util.c
lsof_SOURCES += src/cli.h

if LINUX
lsof_SOURCES += src/dialects/linux/dprint.c
endif
if DARWIN
lsof_SOURCES += src/dialects/darwin/dprint.c
endif
# TODO: link to dynamic library instead after internal functions are no longer used
lsof_SOURCES += $(liblsof_la_SOURCES)
#lsof_LDADD = liblsof.la

liblsof_la_CPPFLAGS = -I$(DIALECT_PATH) -Iautotools -DAUTOTOOLS -I$(top_srcdir)/lib -I$(top_srcdir)/include
lsof_CPPFLAGS = -I$(DIALECT_PATH) -Iautotools -DAUTOTOOLS -I$(top_srcdir)/lib -I$(top_srcdir)/include -I$(top_srcdir)/src

# Testing scripts
AM_TESTS_ENVIRONMENT = export LSOF_DIALECT_DIR=$(LSOF_DIALECT_DIR); export LSOF_DIALECT=$(LSOF_DIALECT);
EXTRA_DIST =
# Dialect neutral
DIALECT_NEUTRAL_TESTS = tests/case-00-hello.bash \
	tests/case-01-version.bash \
	tests/case-20-exit-status.bash \
	tests/case-20-fd-only-inclusion.bash \
	tests/case-20-handle-missing-files.bash \
	tests/case-20-offset-field.bash \
	tests/case-20-repeat-count.bash \
	tests/case-21-exit-Q-status.bash \
	tests/case-22-empty-process-name.bash
TESTS = $(DIALECT_NEUTRAL_TESTS)
EXTRA_DIST += $(DIALECT_NEUTRAL_TESTS) \
	tests/case-13-classic.bash \
	tests/case-14-classic-opt.bash \
	tests/common.bash

# Dialect specific
check_PROGRAMS =

# Linux
LINUX_TESTS = lib/dialects/linux/tests/case-10-mqueue.bash \
	lib/dialects/linux/tests/case-10-ux-socket-state.bash \
	lib/dialects/linux/tests/case-20-epoll.bash \
	lib/dialects/linux/tests/case-20-eventfd-endpoint.bash \
	lib/dialects/linux/tests/case-20-inet6-ffffffff-handling.bash \
	lib/dialects/linux/tests/case-20-inet6-socket-endpoint.bash \
	lib/dialects/linux/tests/case-20-inet-socket-endpoint.bash \
	lib/dialects/linux/tests/case-20-mmap.bash \
	lib/dialects/linux/tests/case-20-mqueue-endpoint.bash \
	lib/dialects/linux/tests/case-20-open-flags-cx.bash \
	lib/dialects/linux/tests/case-20-open-flags-path.bash \
	lib/dialects/linux/tests/case-20-open-flags-tmpf.bash \
	lib/dialects/linux/tests/case-20-pidfd-pid.bash \
	lib/dialects/linux/tests/case-20-pipe-endpoint.bash \
	lib/dialects/linux/tests/case-20-pipe-no-close-endpoint.bash \
	lib/dialects/linux/tests/case-20-pty-endpoint.bash \
	lib/dialects/linux/tests/case-20-ux-socket-endpoint.bash \
	lib/dialects/linux/tests/case-20-ux-socket-endpoint-unaccepted.bash
EXTRA_DIST += $(LINUX_TESTS) lib/dialects/linux/tests/Makefile lib/dialects/linux/tests/case-00-linux-hello.bash
if LINUX
check_PROGRAMS += lib/dialects/linux/tests/epoll \
	lib/dialects/linux/tests/eventfd \
	lib/dialects/linux/tests/mmap \
	lib/dialects/linux/tests/mq_fork \
	lib/dialects/linux/tests/mq_open \
	lib/dialects/linux/tests/open_with_flags \
	lib/dialects/linux/tests/pidfd \
	lib/dialects/linux/tests/pipe \
	lib/dialects/linux/tests/pty \
	lib/dialects/linux/tests/ux
lib_dialects_linux_tests_mq_fork_LDADD = -lrt
lib_dialects_linux_tests_mq_open_LDADD = -lrt

TESTS += $(LINUX_TESTS)
endif

# Testing programs
TESTS += tests/LTbasic tests/LTbigf tests/LTdnlc tests/LTlock tests/LTnfs tests/LTnlink tests/LTsock tests/LTszoff tests/LTunix

check_PROGRAMS += tests/LTbasic tests/LTbigf tests/LTdnlc tests/LTlock tests/LTnfs tests/LTnlink tests/LTsock tests/LTszoff tests/LTunix
tests_LTbasic_SOURCES = tests/LTbasic.c tests/LTlib.c
tests_LTbasic_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include
tests_LTbigf_SOURCES = tests/LTbigf.c tests/LTlib.c
tests_LTbigf_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include
tests_LTdnlc_SOURCES = tests/LTdnlc.c tests/LTlib.c
tests_LTdnlc_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include
tests_LTlock_SOURCES = tests/LTlock.c tests/LTlib.c
tests_LTlock_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include
tests_LTnfs_SOURCES = tests/LTnfs.c tests/LTlib.c
tests_LTnfs_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include
tests_LTnlink_SOURCES = tests/LTnlink.c tests/LTlib.c
tests_LTnlink_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include
tests_LTsock_SOURCES = tests/LTsock.c tests/LTlib.c
tests_LTsock_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include
tests_LTszoff_SOURCES = tests/LTszoff.c tests/LTlib.c
tests_LTszoff_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include
tests_LTunix_SOURCES = tests/LTunix.c tests/LTlib.c
tests_LTunix_CFLAGS = @LSOF_TEST_CFLAGS@ -I$(top_srcdir)/include

TESTS += tests/LTbasic2
check_PROGRAMS += tests/LTbasic2
tests_LTbasic2_CFLAGS = -I$(top_srcdir)/include
tests_LTbasic2_LDADD = liblsof.la

# Documentation
EXTRA_DIST += 00.README.FIRST 00CREDITS 00DCACHE 00DIALECTS 00DIST 00FAQ 00LSOF-L 00MANIFEST 00PORTING 00QUICKSTART 00README 00TEST 00XCONFIG
# Testing
EXTRA_DIST += tests/00README tests/TestDB tests/CkTestDB tests/Makefile tests/LsofTest.h check.bash

# Fix distcheck error
clean-local:
	rm -rf lsof.man
distclean-local:
	rm -rf lockf_owner.h lockf.h
