# @brief Common functions and variables for Tool Under Test (TUT).
#
# The script requires variables:
# TUT_PATH - Assumed absolute path to the directory in which the TUT is located.
# TUT_NAME - TUT name (without path).
#
# The script sets the variables:
# TUT - The path (including the name) of the executable TUT.
# error_prompt - Delimiter on error.
# error_head - Header on error.

package require Expect

# Complete the path for Tool Under Test (TUT). For example, on Windows, TUT can be located in the Debug or Release
# subdirectory. Note that Release build takes precedence over Debug.
set conftypes {{} Release Debug}
foreach i $conftypes {
    if { [file executable "$TUT_PATH/$i/$TUT_NAME"] || [file executable "$TUT_PATH/$i/$TUT_NAME.exe"] } {
        set TUT "$TUT_PATH/$i/$TUT_NAME"
        break
    }
}
if {![info exists TUT]} {
    error "$TUT_NAME executable not found"
}

# prompt of error message
set error_prompt ">>>"
# the beginning of error message
set error_head "$error_prompt Check-failed"

# detection on eof and timeout will be on every expect command
expect_after {
    eof {
        global error_head
        error "$error_head unexpected termination"
    } timeout {
        global error_head
        error "$error_head timeout"
    }
}

# Run commands from command line
tcltest::loadTestedCommands

# namespace of internal functions
namespace eval ly::private {}

# Send command 'cmd' to the process, then check output string by 'pattern'.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex or an exact string to match. If is not specified, only prompt assumed afterwards.
# It must not contain a prompt. There can be an '$' character at the end of the pattern, in which case the regex
# matches the characters before the prompt.
# Parameter 'opt' can contain:
#   -ex     has a similar meaning to the expect command. The 'pattern' parameter is used as a simple string
#           for exact matching of the output. So 'pattern' is not a regular expression but some characters
#           must still be escaped, eg ][.
proc ly_cmd {cmd {pattern ""} {opt ""}} {
    global prompt

    send -- "${cmd}\r"
    expect -- "${cmd}\r\n"

    if { $pattern eq "" } {
        # command without output
        expect ^$prompt
        return
    }

    # definition of an expression that matches failure
    set failure_pattern "\r\n${prompt}$"

    if { $opt eq "" && [string index $pattern end] eq "$"} {
        # check output by regular expression
        # It was explicitly specified how the expression should end.
        set pattern [string replace $pattern end end]
        expect {
            -re "${pattern}\r\n${prompt}$" {}
            -re $failure_pattern {
                error "unexpected output:\n$expect_out(buffer)"
            }
        }
    } elseif { $opt eq "" } {
        # check output by regular expression
        expect {
            -re "${pattern}.*\r\n${prompt}$" {}
            -re $failure_pattern {
                error "unexpected output:\n$expect_out(buffer)"
            }
        }
    } elseif { $opt eq "-ex" } {
        # check output by exact matching
        expect {
            -ex "${pattern}\r\n${prompt}" {}
            -re $failure_pattern {
                error "unexpected output:\n$expect_out(buffer)"
            }
        }
    } else {
        global error_head
        error "$error_head unrecognized value of parameter 'opt'"
    }
}

# Send command 'cmd' to the process, expect some header and then check output string by 'pattern'.
# This function is useful for checking an error that appears in the form of a header.
# Parameter header is the expected header on the output.
# Parameter cmd is a string of arguments.
# Parameter pattern is a regex. It must not contain a prompt.
proc ly_cmd_header {cmd header pattern} {
    global prompt

    send -- "${cmd}\r"
    expect -- "${cmd}\r\n"

    expect {
        -re "$header .*${pattern}.*\r\n${prompt}$" {}
        -re "\r\n${prompt}$" {
            error "unexpected output:\n$expect_out(buffer)"
        }
    }
}

# Whatever is written is sent, output is ignored and then another prompt is expected.
# Parameter cmd is optional and any output is ignored.
proc ly_ignore {{cmd ""}} {
    global prompt

    send "${cmd}\r"
    expect -re "$prompt$"
}

# Send a completion request and check if the anchored regex output matches.
proc ly_completion {input output} {
    global prompt

    send -- "${input}\t"
    # expecting echoing input, output and 10 terminal control characters
    expect -re "^${input}\r${prompt}${output}.*\r.*$"
}

# Send a completion request and check if the anchored regex hint options match.
proc ly_hint {input prev_input hints} {
    global prompt

    set output {}
    foreach i $hints {
        # each element might have some number of spaces and CRLF around it
        append output "${i} *(?:\\r\\n)?"
    }

    send -- "${input}\t"
    # expecting the hints, previous input from which the hints were generated
    # and some number of terminal control characters
    expect -re "${output}\r${prompt}${prev_input}.*\r.*$"
}
