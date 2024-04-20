package require tcltest

# Hook to determine if any of the tests failed.
# Sets a global variable exitCode to 1 if any test fails otherwise it is set to 0.
proc tcltest::cleanupTestsHook {} {
    variable numTests
    set ::exitCode [expr {$numTests(Failed) > 0}]
}

if {[info exists ::env(TESTS_DIR)]} {
    tcltest::configure -testdir "$env(TESTS_DIR)"
}

tcltest::runAllTests
exit $exitCode
