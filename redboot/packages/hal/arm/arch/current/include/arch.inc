##=============================================================================
##
##	arch.inc
##
##	ARM architecture assembler header file
##
##=============================================================================
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
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s): 	jskov
## Contributors:jskov
## Date:	2000-11-15
## Purpose:	ARM definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the ARM
##              It also includes the variant/platform assembly header file.
## Usage:
##		#include <cyg/hal/arch.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/basetype.h>

##-----------------------------------------------------------------------------
## ARM entry definitions. This allows _ prefixing to change by modifying
## the CYG_LABEL_DEFN macro.

#define FUNC_START(name)        \
        .type CYG_LABEL_DEFN(name),@function;   \
        .globl CYG_LABEL_DEFN(name);            \
CYG_LABEL_DEFN(name):

#define FUNC_END(name)  \
        .globl CYG_LABEL_DEFN(name);            \
CYG_LABEL_DEFN(name):
        
#define SYM_DEF(name)   \
        .globl CYG_LABEL_DEFN(name);            \
CYG_LABEL_DEFN(name):
	
#------------------------------------------------------------------------------
# end of arch.inc
