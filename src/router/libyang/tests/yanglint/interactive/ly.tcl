# @brief The main source of functions and variables for testing yanglint in the interactive mode.

# For testing yanglint.
source [expr {[info exists ::env(TESTS_DIR)] ? "$env(TESTS_DIR)/common.tcl" : "../common.tcl"}]
# For testing any interactive tool.
source "$::env(TESTS_DIR)/../tool_i.tcl"

# The script continues by defining variables and functions specific to the interactive yanglint tool.

# set the timeout to 5 seconds
set timeout 5
# prompt of yanglint
set prompt "> "
# turn off dialog between expect and yanglint
log_user 0
# setting some large terminal width
stty columns 720

# default setup for every unit test
variable ly_setup {
    spawn $TUT
    ly_skip_warnings
    # Searchpath is set, so modules can be loaded via the 'load' command.
    ly_cmd "searchpath $::env(YANG_MODULES_DIR)"
}

# default cleanup for every unit test
variable ly_cleanup {
    ly_exit
}

# Skip no dir and/or no history warnings and prompt.
proc ly_skip_warnings {} {
    global prompt
    expect -re "(YANGLINT.*)*$prompt" {}
}

# Send command 'cmd' to the process, expect error header and then check output string by 'pattern'.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex. It must not contain a prompt.
proc ly_cmd_err {cmd pattern} {
    global prompt

    send -- "${cmd}\r"
    expect -- "${cmd}\r\n"

    expect {
        -re "YANGLINT\\\[E\\\]: .*${pattern}.*\r\n${prompt}$" {}
        -re "libyang\\\[\[0-9]+\\\]: .*${pattern}.*\r\n${prompt}$" {}
        -re "\r\n${prompt}$" {
            error "unexpected output:\n$expect_out(buffer)"
        }
    }
}

# Send command 'cmd' to the process, expect warning header and then check output string by 'pattern'.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex. It must not contain a prompt.
proc ly_cmd_wrn {cmd pattern} {
    ly_cmd_header $cmd "YANGLINT\\\[W\\\]:" $pattern
}

# Send 'exit' and wait for eof.
proc ly_exit {} {
    send "exit\r"
    expect eof
}

# Check if yanglint is configured as DEBUG.
# Return 1 on success.
proc yanglint_debug {} {
    global TUT
    # Call non-interactive yanglint with --help.
    set output [exec -- $TUT "-h"]
    # Find option --debug.
    if { [regexp -- "--debug=GROUPS" $output] } {
        return 1
    } else {
        return 0
    }
}
