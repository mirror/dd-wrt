## Process this file with automake to produce Makefile.in
## =====================================================================
##
##     Makefile.am
##
##     ecostest
##
## =====================================================================
#####ECOSHOSTGPLCOPYRIGHTBEGIN####
## -------------------------------------------                              
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
## -------------------------------------------                              
#####ECOSHOSTGPLCOPYRIGHTEND####
## =====================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):   bartv
## Contact(s):  bartv
## Date:       1999/11/09
## Version:     0.01
##
######DESCRIPTIONEND####
## =====================================================================

AUTOMAKE_OPTIONS = 1.3 cygnus
SUBDIRS          =

## Pick up compiler flags etc. from configury
AM_CFLAGS       = @ecos_CFLAGS@
AM_CXXFLAGS     = @ecos_CXXFLAGS@
if MSVC
  INCLUDES      = @ecos_INCLUDES@ -I@MSVC_SRCDIR@/../../common/common
else
  INCLUDES      = @ecos_INCLUDES@ -I$(srcdir)/../../common/common
endif
LIBS            = @ecos_LIBS@ @ecos_LDADD@

BUILT_SOURCES   = build.cxx flags.cxx
bin_PROGRAMS    = ecosconfig
ecosconfig_SOURCES = ecosconfig.cxx cdl_exec.cxx build.cxx flags.cxx
if MSVC
ecosconfig_DEPENDENCIES = @ecos_libcdl_libdir@/cdl.lib @ecos_infra_libdir@/cyginfra.lib
else
ecosconfig_DEPENDENCIES = @ecos_libcdl_libdir@/libcdl.a @ecos_infra_libdir@/libcyginfra.a
endif


noinst_HEADERS = cdl_exec.hxx ecosconfig.hxx

## Unfortunately some of the required sources live elsewhere, yet do not get built
## into a library. Copy them into the build tree.
build.cxx: ../../common/common/build.cxx
	$(INSTALL) $< $@

flags.cxx: ../../common/common/flags.cxx
	$(INSTALL) $< $@

