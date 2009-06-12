## Process this file with automake to produce Makefile.in
## =====================================================================
##
##     Makefile.am
##
##     Build support for the eCos USB host-side
##     
##
## =====================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
## =====================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):	bartv
## Contact(s):	bartv
## Date:	2002/01/10
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

AM_CFLAGS       = @ecos_CFLAGS@ -DUSBHOST_VERSION=\"@VERSION@\" -DPKGVERSION=\"@PACKAGE_VERSION@\" -DUSBAUXDIR=\"$(libexecdir)/ecos/@PACKAGE_INSTALL@\"
AM_CXXFLAGS     = @ecos_CXXFLAGS@
INCLUDES        = @ecos_INCLUDES@
LIBS            = @ecos_LIBS@ @ecos_LDADD@

## The USB testing support consists of two programs, usbhost and
## usbchmod, and a number of Tcl scripts. usbhost is executable by the
## user so it should get installed in bindir, appropriately prefixed
## and with a symbolic link installed as per e.g. tclsh (it is known
## that this code is Linux-specific so the use of symbolic links is
## not a problem). usbchmod is not directly executable so it should
## get installed in or below libexec. The Tcl scripts are architecture
## independent so should probably be installed below $(datadir), but
## for now putting them in the same directory as usbchmod will do
## fine.

noinst_PROGRAMS		= usbhost
usbhost_SOURCES		= usbhost.c
usbhost_DEPENDENCIES	= ../tests/protocol.h ../tests/common.c
usbhost_LDADD           = -lpthread

usbauxdir		= $(libexecdir)/ecos/@PACKAGE_INSTALL@
usbaux_PROGRAMS		= usbchmod
usbaux_DATA		= usbhost.tcl list.tcl verbose.tcl
usbchmod_SOURCES	= usbchmod.c

## Two special operations are required during installation.
## 1) usbhost needs to be installed suitably versioned, e.g. as
##    usbhost_current, and with a symbolic link from usbhost to
##    the new executable.
## 2) usbchmod needs to be suid root
install-exec-hook:
	$(mkinstalldirs) $(bindir)
	$(INSTALL_PROGRAM) usbhost $(bindir)/usbhost_@PACKAGE_VERSION@
	rm -f $(bindir)/usbhost
	$(LN_S) $(bindir)/usbhost_@PACKAGE_VERSION@ $(bindir)/usbhost

install-data-hook:
	chown root $(usbauxdir)/usbchmod
	chmod u+s $(usbauxdir)/usbchmod

else

## When automake scans for hooks it does not take conditionals fully
## into account. If the conditional is not satisfied the generated
## makefile will still try to invoke the hook, so dummy hooks are needed.
install-exec-hook:
	echo Nothing to be done for this platform

install-data-hook:
	echo Nothing to be done for this platform

endif
