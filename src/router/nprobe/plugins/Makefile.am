#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# auto makefile for the nprobe plugins.
# (this file is processed with 'automake' to produce Makefile.in)
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#
# Fusco Francesco     <fusco@ntop.org>
# Luca Deri           <deri@ntop.org>
#
# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

SUBDIRS = . 

PLUGIN_DIST_COMMON = Makefile.am Makefile.in
DISTCLEANFILES =  \#* *~ *.log *.o
CLEANFILES     = $(DISTCLEANFILES)
EXTRA_DIST  =

SUFFIXES    = .so

# 
# Where to install the plugin 
# 
plugindir = $(libdir)/nprobe/plugins

INCLUDES = -I.. @INCS@ 


#
# The meat for nprobe
#

#                  l7Plugin.so
#                  classIdPlugin.so
noinst_PROGRAMS = \
                  dbPlugin.so \
                  httpPlugin.so \
                  smtpPlugin.so \
                  sipPlugin.so \
                  rtpPlugin.so \
                  dumpPlugin.so

#                  libl7Plugin.la
#                  libclassIdPlugin.la
lib_LTLIBRARIES = \
                  libdbPlugin.la \
                  libhttpPlugin.la \
                  libsmtpPlugin.la \
                  libsipPlugin.la \
                  librtpPlugin.la \
                  libdumpPlugin.la

######################################################

#libl7Plugin_la_SOURCES = l7Plugin.c
#libl7Plugin_la_LDFLAGS = -shared -release @PACKAGE_VERSION@ @DYN_FLAGS@
#libl7Plugin_la_CFLAGS = $(AM_CFLAGS)

#.libs/libl7Plugin.so@SO_VERSION_PATCH@:
#	@if test -f libl7Plugin_la-l7Plugin.o; then \
#    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libl7Plugin.so@SO_VERSION_PATCH@ libl7Plugin_la-l7Plugin.o; \
#    else \
#    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libl7Plugin.so@SO_VERSION_PATCH@ l7Plugin.o; \
#    fi

#l7Plugin.so$(EXEEXT): .libs/libl7Plugin.so@SO_VERSION_PATCH@
#	@$(LN_S) .libs/libl7Plugin.so l7Plugin.so

######################################################

#libclassIdPlugin_la_SOURCES = classIdPlugin.c
#libclassIdPlugin_la_LDFLAGS = -shared -release @PACKAGE_VERSION@ @DYN_FLAGS@
#libclassIdPlugin_la_CFLAGS = $(AM_CFLAGS)

#.libs/libclassIdPlugin.so@SO_VERSION_PATCH@:
#	@if test -f libclassIdPlugin_la-classIdPlugin.o; then \
#    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libclassIdPlugin.so@SO_VERSION_PATCH@ libclassIdPlugin_la-classIdPlugin.o; \
#    else \
#    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libclassIdPlugin.so@SO_VERSION_PATCH@ classIdPlugin.o; \
#    fi

#classIdPlugin.so$(EXEEXT): .libs/libclassIdPlugin.so@SO_VERSION_PATCH@
#	@$(LN_S) .libs/libclassIdPlugin.so classIdPlugin.so$(EXEEXT)

######################################################

libdbPlugin_la_SOURCES = dbPlugin.c
libdbPlugin_la_LDFLAGS = -shared -release @PACKAGE_VERSION@ @DYN_FLAGS@
libdbPlugin_la_CFLAGS = $(AM_CFLAGS)

.libs/libdbPlugin.so@SO_VERSION_PATCH@:
	@if test -f libdbPlugin_la-dbPlugin.o; then \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libdbPlugin.so@SO_VERSION_PATCH@ libdbPlugin_la-dbPlugin.o; \
    else \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libdbPlugin.so@SO_VERSION_PATCH@ dbPlugin.o; \
    fi

dbPlugin.so$(EXEEXT): .libs/libdbPlugin.so@SO_VERSION_PATCH@
	@$(LN_S) .libs/libdbPlugin.so dbPlugin.so$(EXEEXT)

######################################################

libhttpPlugin_la_SOURCES = httpPlugin.c
libhttpPlugin_la_LDFLAGS = -shared -release @PACKAGE_VERSION@ @DYN_FLAGS@
libhttpPlugin_la_CFLAGS = $(AM_CFLAGS)

.libs/libhttpPlugin.so@SO_VERSION_PATCH@:
	@if test -f libhttpPlugin_la-httpPlugin.o; then \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libhttpPlugin.so@SO_VERSION_PATCH@ libhttpPlugin_la-httpPlugin.o; \
    else \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libhttpPlugin.so@SO_VERSION_PATCH@ httpPlugin.o; \
    fi

httpPlugin.so$(EXEEXT): .libs/libhttpPlugin.so@SO_VERSION_PATCH@
	@$(LN_S) .libs/libhttpPlugin.so httpPlugin.so$(EXEEXT)

######################################################

libsmtpPlugin_la_SOURCES = smtpPlugin.c
libsmtpPlugin_la_LDFLAGS = -shared -release @PACKAGE_VERSION@ @DYN_FLAGS@
libsmtpPlugin_la_CFLAGS = $(AM_CFLAGS)

.libs/libsmtpPlugin.so@SO_VERSION_PATCH@:
	@if test -f libsmtpPlugin_la-smtpPlugin.o; then \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libsmtpPlugin.so@SO_VERSION_PATCH@ libsmtpPlugin_la-smtpPlugin.o; \
    else \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libsmtpPlugin.so@SO_VERSION_PATCH@ smtpPlugin.o; \
    fi

smtpPlugin.so$(EXEEXT): .libs/libsmtpPlugin.so@SO_VERSION_PATCH@
	@$(LN_S) .libs/libsmtpPlugin.so smtpPlugin.so$(EXEEXT)

######################################################

libsipPlugin_la_SOURCES = sipPlugin.c
libsipPlugin_la_LDFLAGS = -shared -release @PACKAGE_VERSION@ @DYN_FLAGS@
libsipPlugin_la_CFLAGS = $(AM_CFLAGS)

.libs/libsipPlugin.so@SO_VERSION_PATCH@:
	@if test -f libsipPlugin_la-sipPlugin.o; then \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libsipPlugin.so@SO_VERSION_PATCH@ libsipPlugin_la-sipPlugin.o; \
    else \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libsipPlugin.so@SO_VERSION_PATCH@ sipPlugin.o; \
    fi

sipPlugin.so$(EXEEXT): .libs/libsipPlugin.so@SO_VERSION_PATCH@
	@$(LN_S) .libs/libsipPlugin.so sipPlugin.so$(EXEEXT)

######################################################

librtpPlugin_la_SOURCES = rtpPlugin.c
librtpPlugin_la_LDFLAGS = -shared -release @PACKAGE_VERSION@ @DYN_FLAGS@
librtpPlugin_la_CFLAGS = $(AM_CFLAGS)

.libs/librtpPlugin.so@SO_VERSION_PATCH@:
	@if test -f librtpPlugin_la-rtpPlugin.o; then \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/librtpPlugin.so@SO_VERSION_PATCH@ librtpPlugin_la-rtpPlugin.o; \
    else \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/librtpPlugin.so@SO_VERSION_PATCH@ rtpPlugin.o; \
    fi

rtpPlugin.so$(EXEEXT): .libs/librtpPlugin.so@SO_VERSION_PATCH@
	@$(LN_S) .libs/librtpPlugin.so rtpPlugin.so$(EXEEXT)

######################################################

libdumpPlugin_la_SOURCES = dumpPlugin.c
libdumpPlugin_la_LDFLAGS = -shared -release @PACKAGE_VERSION@ @DYN_FLAGS@
libdumpPlugin_la_CFLAGS = $(AM_CFLAGS)

.libs/libdumpPlugin.so@SO_VERSION_PATCH@:
	@if test -f libdumpPlugin_la-dumpPlugin.o; then \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libdumpPlugin.so@SO_VERSION_PATCH@ libdumpPlugin_la-dumpPlugin.o; \
    else \
    $(CC) @MAKE_SHARED_LIBRARY_PARM@ -o .libs/libdumpPlugin.so@SO_VERSION_PATCH@ dumpPlugin.o; \
    fi

dumpPlugin.so$(EXEEXT): .libs/libdumpPlugin.so@SO_VERSION_PATCH@
	@$(LN_S) .libs/libdumpPlugin.so dumpPlugin.so$(EXEEXT)

.NOTPARALLEL:
