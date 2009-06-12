#!/bin/bash
# restart using a Tcl shell \
    exec sh -c 'for tclshell in tclsh tclsh83 cygtclsh80 ; do \
            ( echo | $tclshell ) 2> /dev/null && exec $tclshell "`( cygpath -w \"$0\" ) 2> /dev/null || echo $0`" "$@" ; \
        done ; \
        echo "swap4.tcl: cannot find Tcl shell" ; exit 1' "$0" "$@"

proc filter { input_file output_file } {
    set input_fd [open $input_file  "r"]
    set output_fd  [open $output_file "w"]

    fconfigure $input_fd  -translation binary
    fconfigure $output_fd -translation binary
    while { 1 } {
	set data [read $input_fd 4]
	if { [eof $input_fd] } {
	    break
	}
	binary scan $data "i" var
	puts -nonewline $output_fd [binary format "I" $var]
    }
    close $input_fd
    close $output_fd
}

filter [lindex $argv 0] [lindex $argv 1]
