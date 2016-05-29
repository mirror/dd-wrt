##
## Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2013
## Robert Lougher <rob@jamvm.org.uk>.
##
## This file is part of JamVM.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2,
## or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
##

noinst_LTLIBRARIES    = libengine.la
libengine_la_SOURCES  = interp.c interp2.c relocatability.c interp.h \
                        interp-threading.h interp-indirect.h \
                        interp-direct.h interp-inlining.h \
                        interp-direct-common.h

if COMPILE_TIME_RELOC_CHECKS
noinst_PROGRAMS = compute_relocatability
compute_relocatability_LDADD = libengine.la
endif

AM_CFLAGS   = @interp_cflags@
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/interp -I$(top_builddir)/src

