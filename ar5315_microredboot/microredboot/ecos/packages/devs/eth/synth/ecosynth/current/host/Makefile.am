## Process this file with automake to produce Makefile.in
## =====================================================================
##
##     Makefile.am
##
##     Build support for the host-side synthetic target support,
##     the ethernetpackage.
##
## =====================================================================
######COPYRIGHTBEGIN####
#                                                                          
# ----------------------------------------------------------------------------
# Copyright (C) 2002, 2003 Bart Veer
#
# This file is part of the eCos synthetic target ethernet support.
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
# ----------------------------------------------------------------------------
#                                                                          
######COPYRIGHTEND####
## =====================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):	bartv
## Contact(s):	bartv
## Date:	2002/08/07
## Version:	0.01
##
######DESCRIPTIONEND####
## =====================================================================

AUTOMAKE_OPTIONS = 1.3 foreign

## Only some platforms are supported. Having the configure script throw
## an error when attempting to configure on an unsupported platform
## would be a mistake, since that would prevent any configury from
## the toplevel on unsupported platforms. Instead an automake conditional
## is used, leading to null makefiles on unsupported platforms.

if SUPPORTED

AM_CFLAGS       = @ecos_CFLAGS@ -DECOSYNTH_VERSION=\"@VERSION@\" \
 -DECOS_REPOSITORY=\"@ECOS_REPOSITORY@\" \
 -DLIBEXECDIR=\"$(libexecdir)\" \
 -DPACKAGE_DIR=\"@PACKAGE_DIR@\" \
 -DPACKAGE_VERSION=\"@PACKAGE_VERSION@\" \
 -DPACKAGE_INSTALL=\"@PACKAGE_INSTALL@\"
AM_CXXFLAGS     = @ecos_CXXFLAGS@
INCLUDES        = @ecos_INCLUDES@
LIBS            = @ecos_LIBS@ @ecos_LDADD@

## The synthetic target support consists of a single program rawether,
## a number of Tcl scripts, and some additional data files. These are
## all installed in a single directory $(libexec)/ecos/<package>_<version>/
## Neither the rawether executable nor any of the scripts are directly
## executable, instead rawether gets fork()'d/execve()'d by the Tcl
## script so $(libexec) is appropriate. Strictly speaking the
## Tcl scripts and data files are architecture-independent so should
## probably be installed in an analogous directory below $(datadir),
## but that would add more directories for little real gain. The scripts
## are treated as data files since they should not be executed directly,
## i.e. they should not be installed with the execute bit set.

etherdir 		= $(libexecdir)/ecos/@PACKAGE_INSTALL@
ether_PROGRAMS		= rawether
ether_DATA		= ethernet.tcl ethernet.tdf netrecord.xbm

rawether_SOURCES	= rawether.c

## Manual dependencies
rawether.$(OBJEXT)	: Makefile ../src/protocol.h

## The rawether program needs to run with root privileges, or it will
## be unable to access the ethernet hardware.
install-data-hook:
	chown root $(etherdir)/rawether
	chmod u+s $(etherdir)/rawether

else
## When automake scans for hooks it does not take conditionals fully
## into account. If the conditional is not satisfied the generated
## makefile will still try to invoke the hook, so dummy hooks are needed.
install-data-hook:
	echo Nothing to be done for this platform
endif
