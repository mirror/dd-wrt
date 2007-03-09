## Process this file with automake to produce Makefile.in
## =====================================================================
##
##     Makefile.am
##
##     libcdl.a
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
## Date:	1998/12/16
## Version:	0.01
##
######DESCRIPTIONEND####
## =====================================================================

AUTOMAKE_OPTIONS 	= 1.3 foreign
SUBDIRS          	= testsuite

## Pick up compiler flags etc. from configury
INCLUDES        	= @ecos_INCLUDES@
LIBS			= @ecos_LIBS@ @ecos_LDADD@
AM_CFLAGS		= @ecos_CFLAGS@
AM_CXXFLAGS		= @ecos_CXXFLAGS@
ecos_infra_incdir	= @ecos_infra_incdir@

## This package exports its interface via cdl.hxx
include_HEADERS = cdlcore.hxx cdl.hxx

## The main target is a static library. The problem is naming it
## correctly. automake seems to insist on calling it libcdl.a,
## even when compiling it with VC++. Various attempts to use
## cdl.lib instead with VC++ have failed. Instead the library
## will always be built as libcdl.a, but gets copied to cdl.lib
## when necessary.

sources := interp.cxx	    \
           expr.cxx         \
           func.cxx	    \
           refer.cxx        \
           cdlmisc.cxx      \
           conflict.cxx     \
           property.cxx     \
           parse.cxx        \
           base.cxx         \
           value.cxx        \
           build.cxx	    \
           dialog.cxx       \
           wizard.cxx       \
           interface.cxx    \
           transact.cxx     \
           infer.cxx        \
           database.cxx     \
           config.cxx       \
           package.cxx      \
           component.cxx    \
           option.cxx

noinst_LIBRARIES = libcdl.a
libcdl_a_SOURCES = $(sources)

if MSVC
all-local: cdl.lib

cdl.lib: libcdl.a
	$(INSTALL) $< $@

clean-local:
	$(RM) cdl.lib

install-exec-local: all-local
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	$(INSTALL) cdl.lib $(DESTDIR)$(libdir)/cdl.lib

else

all-local:
clean-local:
install-exec-local: libcdl.a
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	$(INSTALL) libcdl.a $(DESTDIR)$(libdir)/libcdl.a
endif


## Add specific dependencies for all source files.
## Every module in libcdl uses assertions and tracing.
## These pull in the others.
infra_headers := $(ecos_infra_incdir)/cyg/infra/cyg_ass.h  \
                 $(ecos_infra_incdir)/cyg/infra/cyg_trac.h \
                 $(ecos_infra_incdir)/pkgconf/infra.h      \
                 $(ecos_infra_incdir)/pkgconf/hostinfra.h  \
                 $(ecos_infra_incdir)/cyg/infra/cyg_type.h

## Depending on the flavor of CDL being implemented, different
## headers will get included.
cdlcore_headers := cdlconfig.h cdlcore.hxx
cdl_headers     := $(cdlcore_headers) cdl.hxx

interp.$(OBJEXT)    : Makefile $(cdlcore_headers) $(infra_headers)

expr.$(OBJEXT)      : Makefile $(cdlcore_headers) $(infra_headers)

infer.$(OBJEXT)     : Makefile $(cdlcore_headers) $(infra_headers)

func.$(OBJEXT)      : Makefile $(cdlcore_headers) $(infra_headers)

refer.$(OBJEXT)     : Makefile $(cdlcore_headers) $(infra_headers)

cdlmisc.$(OBJEXT)   : Makefile $(cdlcore_headers) $(infra_headers)

conflict.$(OBJEXT)  : Makefile $(cdlcore_headers) $(infra_headers)

property.$(OBJEXT)  : Makefile $(cdlcore_headers) $(infra_headers)

parse.$(OBJEXT)     : Makefile $(cdlcore_headers) $(infra_headers)

base.$(OBJEXT)      : Makefile $(cdlcore_headers) $(infra_headers)

value.$(OBJEXT)     : Makefile $(cdlcore_headers) $(infra_headers)

build.$(OBJEXT)     : Makefile $(cdlcore_headers) $(infra_headers)

dialog.$(OBJEXT)    : Makefile $(cdlcore_headers) $(infra_headers)

wizard.$(OBJEXT)    : Makefile $(cdlcore_headers) $(infra_headers)

interface.$(OBJEXT) : Makefile $(cdlcore_headers) $(infra_headers)

transact.$(OBJEXT)  : Makefile $(cdlcore_headers) $(infra_headers)

database.$(OBJEXT)  : Makefile $(cdl_headers) $(infra_headers)

config.$(OBJEXT)    : Makefile $(cdl_headers) $(infra_headers)

package.$(OBJEXT)   : Makefile $(cdl_headers) $(infra_headers)

component.$(OBJEXT) : Makefile $(cdl_headers) $(infra_headers)

option.$(OBJEXT)    : Makefile $(cdl_headers) $(infra_headers)


