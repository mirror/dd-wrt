##
## @file support/junction/Makefile.am
## @brief Process this file with automake to produce src/libjunction/Makefile.in
##

##
## Copyright 2010, 2018 Oracle.  All rights reserved.
##
## This file is part of nfs-utils.
##
## nfs-utils is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License version 2.0 as
## published by the Free Software Foundation.
##
## nfs-utils is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License version 2.0 for more details.
##
## You should have received a copy of the GNU General Public License
## version 2.0 along with nfs-utils.  If not, see:
##
##      http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
##

noinst_HEADERS		= junction-internal.h

noinst_LTLIBRARIES	= libjunction.la
libjunction_la_SOURCES	= display.c export-cache.c junction.c \
			  locations.c nfs.c path.c xml.c

MAINTAINERCLEANFILES	= Makefile.in
