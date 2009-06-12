#!/bin/bash
# restart using a Tcl shell \
    exec sh -c 'for tclshell in tclsh tclsh83 cygtclsh80 ; do \
            ( echo | $tclshell ) 2> /dev/null && exec $tclshell "`( cygpath -w \"$0\" ) 2> /dev/null || echo $0`" "$@" ; \
        done ; \
        echo "slow_cat.tcl: cannot find Tcl shell" ; exit 1' "$0" "$@"

# Can be used like this:
#  Get flash ready for programming using Minicom or similar
#   [o (Option menu), a (flAsh menu), b (Boot write)]
# Then execute the following
#  slow_cat.tcl < install/bin/gdb_module.srec  >/dev/ttyS0

# Delay lines by 1/10 of a second to allow the flash to recover The
# gets also strips off the broken (DOS) new-lines that objcopy is
# generating. The puts replace them with 0x0a which the firmware
# requires.
while { 0 <= [gets stdin line] } {
    puts $line
    after 100
}
