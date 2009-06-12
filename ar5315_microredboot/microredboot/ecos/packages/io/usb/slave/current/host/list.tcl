# {{{  Banner                           

#===============================================================================
#
#    list.tcl
#
#    Support for USB testing
#
#===============================================================================
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
#===============================================================================
######DESCRIPTIONBEGIN####
#
# Author(s):	bartv
# Date:		2001-08-21
# Purpose:      Provide details of the available endpoints
#
#####DESCRIPTIONEND####
#===============================================================================
#

# }}}

puts "Control endpoint: valid packet sizes are $usbtest::control(min_size) to $usbtest::control(max_size)"

if { 0 != [llength $usbtest::bulk_in_endpoints] } {
    puts "Bulk IN endpoints: $usbtest::bulk_in_endpoints"
    foreach ep $usbtest::bulk_in_endpoints {
	puts [format "  %2d: packet sizes %d to %d, padding %d" $ep \
		$usbtest::bulk_in($ep,min_size) $usbtest::bulk_in($ep,max_size) $usbtest::bulk_in($ep,max_in_padding)]
	if { "" == $usbtest::bulk_in($ep,devtab) } {
	    puts "      no devtab entry"
	} else {
	    puts "      devtab entry $usbtest::bulk_in($ep,devtab)"
	}
    }
}
if { 0 != [llength $usbtest::bulk_out_endpoints] } {
    puts "Bulk OUT endpoints: $usbtest::bulk_out_endpoints"
    foreach ep $usbtest::bulk_out_endpoints {
	puts [format "  %2d: packet sizes %d to %d" $ep \
		$usbtest::bulk_out($ep,min_size) $usbtest::bulk_out($ep,max_size)]
	if { "" == $usbtest::bulk_out($ep,devtab) } {
	    puts "      no devtab entry"
	} else {
	    puts "      devtab entry $usbtest::bulk_out($ep,devtab)"
	}
    }
}

if { 0 != [llength $usbtest::isochronous_in_endpoints] } {
    puts "Isochronous IN endpoints: $usbtest::isochronous_in_endpoints"
    foreach ep $usbtest::isochronous_in_endpoints {
	puts [format "  %2d: packet sizes %d to %d" $ep \
		$usbtest::isochronous_in($ep,min_size) $usbtest::isochronous_in($ep,max_size)]
	if { "" == $usbtest::isochronous_in($ep,devtab) } {
	    puts "      no devtab entry"
	} else {
	    puts "      devtab entry $usbtest::isochronous_in($ep,devtab)"
	}
    }
}
if { 0 != [llength $usbtest::isochronous_out_endpoints] } {
    puts "Isochronous OUT endpoints: $usbtest::isochronous_out_endpoints"
    foreach ep $usbtest::isochronous_out_endpoints {
	puts [format "  %2d: packet sizes %d to %d" $ep \
		$usbtest::isochronous_out($ep,min_size) $usbtest::isochronous_out($ep,max_size)]
	if { "" == $usbtest::isochronous_out($ep,devtab) } {
	    puts "      no devtab entry"
	} else {
	    puts "      devtab entry $usbtest::isochronous_out($ep,devtab)"
	}
    }
}

if { 0 != [llength $usbtest::interrupt_in_endpoints] } {
    puts "Interrupt IN endpoints: $usbtest::interrupt_in_endpoints"
    foreach ep $usbtest::interrupt_in_endpoints {
	puts [format "  %2d: packet sizes %d to %d" $ep \
		$usbtest::interrupt_in($ep,min_size) $usbtest::interrupt_in($ep,max_size)]
	if { "" == $usbtest::interrupt_in($ep,devtab) } {
	    puts "      no devtab entry"
	} else {
	    puts "      devtab entry $usbtest::interrupt_in($ep,devtab)"
	}
    }
}
if { 0 != [llength $usbtest::interrupt_out_endpoints] } {
    puts "Interrupt OUT endpoints: $usbtest::interrupt_out_endpoints"
    foreach ep $usbtest::interrupt_out_endpoints {
	puts [format "  %2d: packet sizes %d to %d" $ep \
		$usbtest::interrupt_out($ep,min_size) $usbtest::interrupt_out($ep,max_size)]
	if { "" == $usbtest::interrupt_out($ep,devtab) } {
	    puts "      no devtab entry"
	} else {
	    puts "      devtab entry $usbtest::interrupt_out($ep,devtab)"
	}
    }
}
