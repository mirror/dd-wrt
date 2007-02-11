#!/bin/sh
#
# eval_onescript.sh SCRIPT  [TESTNUMBER [HEADERONLY] ]
#
# Evaluates one test program, and helps it out by doing a bit of setup
# for it.  It does this by sourcing some configuration files for it
# first, and if it exited without calling FINISHED, call it.
#
# Not intended to be a tool for the common user!  Called by RUNTESTS
# directly instead.
#

testnum=1
if [ "x$2" != "x" ]; then
	testnum=$2
fi
export testnum

unset SNMP_HEADERONLY
if [ "x$3" = "xyes" ]; then
    SNMP_HEADERONLY=yes
fi
export SNMP_HEADERONLY

. TESTCONF.sh

. eval_tools.sh

ECHO "$testnum:  "

. ./$1

# We shouldn't get here...
# If we do, it means they didn't exit properly.
# So we will.
FINISHED
