#  Makefile for storage modules in the Squid Object Cache server
#
#  $Id: Makefile.am,v 1.9 2006/09/09 16:04:38 serassio Exp $
#

AUTOMAKE_OPTIONS = subdir-objects

if USE_AIOPS_WIN32
AIOPS_SOURCE = aufs/aiops_win32.c
else
AIOPS_SOURCE = aufs/aiops.c
endif

EXTRA_LIBRARIES = libaufs.a libcoss.a libdiskd.a libnull.a libufs.a
noinst_LIBRARIES = @STORE_LIBS@

EXTRA_libaufs_a_SOURCES = aufs/aiops.c aufs/aiops_win32.c

libaufs_a_SOURCES = $(AIOPS_SOURCE) aufs/async_io.c aufs/store_asyncufs.h \
	aufs/store_dir_aufs.c aufs/store_io_aufs.c aufs/async_io.h
libcoss_a_SOURCES = coss/store_coss.h   coss/store_io_coss.c coss/store_dir_coss.c \
	coss/async_io.c coss/async_io.h
libdiskd_a_SOURCES = diskd/diskd.c diskd/store_dir_diskd.c diskd/store_diskd.h \
	diskd/store_io_diskd.c
libnull_a_SOURCES = null/store_null.c
libufs_a_SOURCES = ufs/store_dir_ufs.c ufs/store_io_ufs.c ufs/store_ufs.h

if NEED_DISKD
DISKD = diskd-daemon
else
DISKD = 
endif

libexec_PROGRAMS = $(DISKD)

diskd_daemon_SOURCES = diskd/diskd.c

LDADD = $(top_builddir)/lib/libmiscutil.a @XTRA_LIBS@

EXTRA_DIST = \
	coss/coss-notes.txt

INCLUDES      = -I. -I$(top_builddir)/include -I$(top_srcdir)/include \
	-I$(top_srcdir)/src

## targets below to emulate distributed makefiles
aufs/all: libaufs.a
aufs/clean: clean
coss/all: libcoss.a
coss/clean: clean
null/all: libnull.a
null/clean: clean
ufs/all: libufs.a
ufs/clean: clean
