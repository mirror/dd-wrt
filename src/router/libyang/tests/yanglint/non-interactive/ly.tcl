# @brief The main source of functions and variables for testing yanglint in the non-interactive mode.

# For testing yanglint.
source [expr {[info exists ::env(TESTS_DIR)] ? "$env(TESTS_DIR)/common.tcl" : "../common.tcl"}]
# For testing any non-interactive tool.
source "$::env(TESTS_DIR)/../tool_ni.tcl"

# The script continues by defining variables and functions specific to the non-interactive yanglint tool.
