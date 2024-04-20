# @brief The main source of functions and variables for testing yangre.

package require tcltest
namespace import ::tcltest::test ::tcltest::cleanupTests

if { ![info exists ::env(TESTS_DIR)] } {
    # the script is not run via 'ctest' so paths must be set
    set ::env(TESTS_DIR) "./"
    set TUT_PATH "../../build"
} else {
    # cmake (ctest) already sets ::env variables
    set TUT_PATH $::env(YANGRE)
}
set TUT_NAME "yangre"
source "$::env(TESTS_DIR)/../tool_ni.tcl"

# The script continues by defining variables and functions specific to the yangre tool.
