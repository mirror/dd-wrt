dnl Process this file with aclocal to get an aclocal.m4 file. Then
dnl process that with autoconf.
dnl ====================================================================
dnl
dnl     acinclude.m4
dnl
dnl ====================================================================
dnl ####ECOSGPLCOPYRIGHTBEGIN####
dnl -------------------------------------------
dnl This file is part of eCos, the Embedded Configurable Operating System.
dnl Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
dnl
dnl eCos is free software; you can redistribute it and/or modify it under
dnl the terms of the GNU General Public License as published by the Free
dnl Software Foundation; either version 2 or (at your option) any later version.
dnl
dnl eCos is distributed in the hope that it will be useful, but WITHOUT ANY
dnl WARRANTY; without even the implied warranty of MERCHANTABILITY or
dnl FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
dnl for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with eCos; if not, write to the Free Software Foundation, Inc.,
dnl 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
dnl
dnl As a special exception, if other files instantiate templates or use macros
dnl or inline functions from this file, or you compile this file and link it
dnl with other works to produce a work based on this file, this file does not
dnl by itself cause the resulting work to be covered by the GNU General Public
dnl License. However the source code for this file must still be made available
dnl in accordance with section (3) of the GNU General Public License.
dnl
dnl This exception does not invalidate any other reasons why a work based on
dnl this file might be covered by the GNU General Public License.
dnl
dnl Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
dnl at http://sources.redhat.com/ecos/ecos-license/
dnl -------------------------------------------
dnl ####ECOSGPLCOPYRIGHTEND####
dnl ====================================================================
dnl#####DESCRIPTIONBEGIN####
dnl
dnl Author(s):	bartv
dnl Contact(s):	bartv
dnl Date:	2002/01/10
dnl Version:	0.01
dnl
dnl####DESCRIPTIONEND####
dnl ====================================================================

dnl Access shared macros.
dnl AM_CONDITIONAL needs to be mentioned here or else aclocal does not
dnl incorporate the macro into aclocal.m4
sinclude(../../../../../../acsupport/acinclude.m4)
