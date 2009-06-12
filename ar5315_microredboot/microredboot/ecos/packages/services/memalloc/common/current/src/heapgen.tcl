#!/bin/bash
# restart using a Tcl shell \
    exec sh -c 'for tclshell in tclsh tclsh83 cygtclsh80 ; do \
            ( echo | $tclshell ) 2> /dev/null && exec $tclshell "`( cygpath -w \"$0\" ) 2> /dev/null || echo $0`" "$@" ; \
        done ; \
        echo "heapgen.tcl: cannot find Tcl shell" ; exit 1' "$0" "$@"

#===============================================================================
#
#    heapgen.tcl
#
#    Script to generate memory pool instantiations based on the memory map
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
# Author(s):	jlarmour
# Contributors:	
# Date:		2000-06-13
# Purpose:      Generate memory pool instantiations based on the memory map
#               along with information in a header file to allow access from
#               C source
# Description:
# Usage:
#
#####DESCRIPTIONEND####
#===============================================================================

set debug 0

proc dputs { args } {
    global debug
    if { $debug > 0 } {
        puts -nonewline "DEBUG: "
        foreach i $args {
          puts -nonewline $i
        }
        puts ""
    }
}

proc tcl_path { posix_path } {
    global tcl_platform
    if { $tcl_platform(platform) == "windows" } {
        return [ exec cygpath -w $posix_path ]
    } else {
        return $posix_path
    }
}

dputs "argc=" $argc
dputs "argv=" $argv

if { $argc != 2 } {
    error "Usage: heapgen.tcl installdir builddir"
}

set installdir [ tcl_path [ lindex $argv 0 ] ]
set builddir   [ tcl_path [ lindex $argv 1 ] ]

dputs "builddir=" $builddir
dputs "installdir=" $installdir
dputs "pwd=" [pwd]

# Fetch relevant config data placed in the generated file heapgeninc.tcl
source [ file join $builddir heapgeninc.tcl ]

dputs "memlayout_h=" $memlayout_h

# ----------------------------------------------------------------------------
# Get heap information

# trim brackets
set ldi_name [ string trim $memlayout_ldi "<>" ]
dputs $ldi_name 
# prefix full leading path including installdir
set ldifile [open [ file join $installdir include $ldi_name ] r]

# now read the .ldi file and find the user-defined sections with the
# prefix "heap"
set heaps ""
while { [gets $ldifile line] >= 0} {
    # Search for user-defined name beginning heap (possibly with leading
    # underscores
    if [ regexp {^[ \t]+(CYG_LABEL_DEFN\(|)[ \t]*_*heap} $line ] {
        set heapnamestart [ string first heap $line ]
        set heapnameend1 [ string first ")" $line ]
        incr heapnameend1 -1
        set heapnameend2 [ string wordend $line $heapnamestart ]
        if { $heapnameend1 < 0 } {
            set $heapnameend1 $heapnameend2
        }
        set heapnameend [ expr $heapnameend1 < $heapnameend2 ? $heapnameend1 : $heapnameend2 ]
        set heapname [ string range $line $heapnamestart $heapnameend ]
        set heaps [ concat $heaps $heapname ]
        dputs [ format "Found heap \"%s\"" $heapname ]
    }
}
close $ldifile

set heapcount [ llength $heaps ]
set heapcount1 [ expr 1 + $heapcount ]

# ----------------------------------------------------------------------------
# Generate header file

# Could have made it generate the header file straight into include/pkgconf,
# but that knowledge of the build system is best left in the make rules in CDL

set hfile [ open [ file join $builddir heaps.hxx ] w]
puts $hfile "#ifndef CYGONCE_PKGCONF_HEAPS_HXX"
puts $hfile "#define CYGONCE_PKGCONF_HEAPS_HXX"
puts $hfile "/* <pkgconf/heaps.hxx> */\n"
puts $hfile "/* This is a generated file - do not edit! */\n"
# Allow CYGMEM_HEAP_COUNT to be available to the implementation header file
puts $hfile [ format "#define CYGMEM_HEAP_COUNT %d" $heapcount ]
puts $hfile [ concat "#include " $malloc_impl_h ]
puts $hfile ""
puts $hfile [ format "extern %s *cygmem_memalloc_heaps\[ %d \];" \
        $malloc_impl_class $heapcount1 ]
puts $hfile "\n#endif"
puts $hfile "/* EOF <pkgconf/heaps.hxx> */"
close $hfile

# ----------------------------------------------------------------------------
# Generate C file in the current directory (ie. the build directory)
# that instantiates the pools

set cfile [ open [ file join $builddir heaps.cxx ] w ]
puts $cfile "/* heaps.cxx */\n"
puts $cfile "/* This is a generated file - do not edit! */\n"
puts $cfile "#include <pkgconf/heaps.hxx>"
puts $cfile [ concat "#include " $memlayout_h ]
puts $cfile "#include <cyg/infra/cyg_type.h>"
puts $cfile "#include <cyg/hal/hal_intr.h>"
puts $cfile [ concat "#include " $malloc_impl_h ]
puts $cfile ""

foreach heap $heaps {
    puts $cfile "#ifdef HAL_MEM_REAL_REGION_TOP\n"

    puts $cfile [ format "%s CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_MEMALLOC) cygmem_pool_%s ( (cyg_uint8 *)CYGMEM_SECTION_%s ," \
            $malloc_impl_class $heap $heap ]
    puts $cfile [ format "    HAL_MEM_REAL_REGION_TOP( (cyg_uint8 *)CYGMEM_SECTION_%s + CYGMEM_SECTION_%s_SIZE ) - (cyg_uint8 *)CYGMEM_SECTION_%s ) " \
            $heap $heap $heap ]
    puts $cfile "        ;\n"

    puts $cfile "#else\n"

    puts $cfile [ format "%s CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_MEMALLOC) cygmem_pool_%s ( (cyg_uint8 *)CYGMEM_SECTION_%s , CYGMEM_SECTION_%s_SIZE ) ;\n" \
            $malloc_impl_class $heap $heap $heap ]

    puts $cfile "#endif"
}

puts $cfile ""
puts $cfile [ format "%s *cygmem_memalloc_heaps\[ %d \] = { " \
        $malloc_impl_class $heapcount1 ]
foreach heap $heaps {
    puts $cfile [ format "    &cygmem_pool_%s," $heap ]
}
puts $cfile "    NULL\n};"

puts $cfile "\n/* EOF heaps.cxx */"
close $cfile

# ----------------------------------------------------------------------------
# EOF heapgen.tcl
