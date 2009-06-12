## Process this file with automake to produce Makefile.in
## =====================================================================
##
##     Makefile.am
##
##     Build support for the host-side synthetic target support,
##     the watchdog package
##
## =====================================================================
######COPYRIGHTBEGIN####
#                                                                          
# ----------------------------------------------------------------------------
# Copyright (C) 2002 Bart Veer
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
## Date:	2002/09/04
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

## The only thing that needs to be installed is a Tcl script and some
## data files.
watchdogdir 		= $(libexecdir)/ecos/@PACKAGE_INSTALL@
watchdog_DATA		= watchdog.tcl \
                          watchdog.tdf \
                          alarm.gif doghouse.gif eye.gif asleep.gif \
                          sound1.au sound2.au
endif
