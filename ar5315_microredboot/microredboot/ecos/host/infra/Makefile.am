## Process this file with automake to produce Makefile.in
## =====================================================================
##
##     Makefile.am
##
##     Host side implementation of the eCos infrastructure
##
## =====================================================================
#######ECOSHOSTGPLCOPYRIGHTBEGIN####
## ----------------------------------------------------------------------------
# Copyright (C) 2002 Bart Veer
# Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.
#
# This file is part of the eCos host tools.
#
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation; either version 2 of the License, or (at your option) 
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 
# 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
## ----------------------------------------------------------------------------
#######ECOSHOSTGPLCOPYRIGHTEND####
## =====================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):	bartv
## Contact(s):	bartv
## Date:	1998/07/13
## Version:	0.01
##
######DESCRIPTIONEND####
## =====================================================================

AUTOMAKE_OPTIONS = 1.3 foreign
SUBDIRS          = testsuite

## The generated makefile is responsible for the following:
##
##   1) copy across the shared header files cyg_type.h, cyg_ass.h and
##      cyg_trac.h from the include directory into the appropriate part
##      of the build tree.
##
##   2) on the target side the infrastructure headers depend on
##      the configuration system via <pkgconf/infra.h>, and on the
##      HAL package via <cyg/hal/basetype.h>. These two files need
##      to be implemented somewhat differently on the host side.
##      There are usable versions in the source tree.
##
##   3) build libcyginfra.a
##
##   4) allow the whole system to be installed, and all the usual
##      makefile targets.

## The main target is a static library. The problem is naming it
## correctly. automake seems to insist on calling it libcyginfra.a,
## even when compiling it with VC++. Various attempts to use
## cyginfra.lib instead with VC++ have failed. Instead the library
## will always be built as libcyginfra.a, but it may get installed
## as cyginfra.lib.

noinst_LIBRARIES      = libcyginfra.a
libcyginfra_a_SOURCES = assert.cxx    \
                        trace.cxx     \
                        checkdata.cxx \
                        testcase.cxx

## Pick up compiler flags etc. from configury
INCLUDES	= @ecos_INCLUDES@
LIBS		= @ecos_LIBS@ @ecos_LDADD@
AM_CFLAGS	= @ecos_CFLAGS@
AM_CXXFLAGS	= @ecos_CXXFLAGS@

if MSVC
all-local: cyginfra.lib

cyginfra.lib: libcyginfra.a
	$(INSTALL) $< $@

clean-local:
	$(RM) cyginfra.lib

install-exec-local: all-local
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	$(INSTALL) cyginfra.lib $(DESTDIR)$(libdir)/cyginfra.lib

else

all-local:
clean-local:

install-exec-local: libcyginfra.a
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	$(INSTALL) libcyginfra.a $(DESTDIR)$(libdir)/libcyginfra.a

endif

## ----------------------------------------------------------------------------
## The header files do not all belong in the same directory. This
## causes problems with automake. First list the header files.

copies = cyg/infra/cyg_type.h \
         cyg/infra/cyg_ass.h  \
         cyg/infra/cyg_trac.h \
         cyg/infra/testcase.h \
         cyg/hal/basetype.h   \
         pkgconf/infra.h      \
         pkgconf/hostinfra.h

## The headers are not installed as part of the default install
## target, instead a data hook is used. This avoids problems when the
## appropriate directories do not yet exist in the install tree.
noinst_HEADERS = $(copies)

install-data-local: $(copies)
	$(mkinstalldirs)  $(DESTDIR)$(includedir) \
                $(DESTDIR)$(includedir)/cyg       \
                $(DESTDIR)$(includedir)/cyg/infra \
                $(DESTDIR)$(includedir)/cyg/hal   \
                $(DESTDIR)$(includedir)/pkgconf
	$(INSTALL_DATA) cyg/infra/cyg_type.h    $(DESTDIR)$(includedir)/cyg/infra
	$(INSTALL_DATA) cyg/infra/cyg_ass.h     $(DESTDIR)$(includedir)/cyg/infra
	$(INSTALL_DATA) cyg/infra/cyg_trac.h    $(DESTDIR)$(includedir)/cyg/infra
	$(INSTALL_DATA) cyg/infra/testcase.h    $(DESTDIR)$(includedir)/cyg/infra
	$(INSTALL_DATA) cyg/hal/basetype.h      $(DESTDIR)$(includedir)/cyg/hal
	$(INSTALL_DATA) pkgconf/infra.h         $(DESTDIR)$(includedir)/pkgconf
	$(INSTALL_DATA) pkgconf/hostinfra.h     $(DESTDIR)$(includedir)/pkgconf

## It is necessary to have an extra set of rules to copy the header files
## into the build tree, so that they are in a sensible location when
## building the sources in this package. There are explicit dependencies
## for all the object files on these headers.
##
## The header file copies should be part of the clean.
CLEANFILES = $(copies)

cyg/infra/cyg_type.h: cyg_type.h
	@if [ -d cyg ] ; then true ; else mkdir cyg ; fi
	@if [ -d cyg/infra ] ; then true ; else mkdir cyg/infra ; fi
	$(INSTALL_DATA) $(<) $(@)

cyg/infra/cyg_ass.h: cyg_ass.h
	@if [ -d cyg ] ; then true ; else mkdir cyg ; fi
	@if [ -d cyg/infra ] ; then true ; else mkdir cyg/infra ; fi
	$(INSTALL_DATA) $(<) $(@)

cyg/infra/cyg_trac.h: cyg_trac.h
	@if [ -d cyg ] ; then true ; else mkdir cyg ; fi
	@if [ -d cyg/infra ] ; then true ; else mkdir cyg/infra ; fi
	$(INSTALL_DATA) $(<) $(@)

cyg/infra/testcase.h: testcase.h
	@if [ -d cyg ] ; then true ; else mkdir cyg ; fi
	@if [ -d cyg/infra ] ; then true ; else mkdir cyg/infra ; fi
	$(INSTALL_DATA) $(<) $(@)

pkgconf/infra.h : infra.h
	@if [ -d pkgconf ] ; then true ; else mkdir pkgconf ; fi
	$(INSTALL_DATA) $(<) $(@)

pkgconf/hostinfra.h : hostinfra.h
	@if [ -d pkgconf ] ; then true ; else mkdir pkgconf ; fi
	$(INSTALL_DATA) $(<) $(@)

cyg/hal/basetype.h : basetype.h
	@if [ -d cyg ] ; then true ; else mkdir cyg ; fi
	@if [ -d cyg/hal ] ; then true ; else mkdir cyg/hal ; fi
	$(INSTALL_DATA) $(<) $(@)

## ----------------------------------------------------------------------------
## Add specific dependencies for all source files

assert.$(OBJEXT) :  Makefile \
                    pkgconf/infra.h      \
                    pkgconf/hostinfra.h  \
                    cyg/infra/cyg_type.h \
                    cyg/hal/basetype.h   \
                    cyg/infra/cyg_ass.h

trace.$(OBJEXT) :   Makefile \
                    pkgconf/infra.h      \
                    pkgconf/hostinfra.h  \
                    cyg/infra/cyg_type.h \
                    cyg/hal/basetype.h   \
                    cyg/infra/cyg_ass.h  \
                    cyg/infra/cyg_trac.h

checkdata.$(OBJEXT) : Makefile \
                      cyg/infra/cyg_ass.h       \
                      pkgconf/infra.h           \
                      pkgconf/hostinfra.h       \
                      cyg/infra/cyg_type.h      \
                      cyg/hal/basetype.h

testcase.$(OBJEXT): Makefile \
                    cyg/infra/testcase.h \
                    cyg/infra/cyg_type.h \
                    cyg/hal/basetype.h   \
                    pkgconf/infra.h      \
                    pkgconf/hostinfra.h
