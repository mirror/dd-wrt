#!/bin/sh
#
# test_keychange.sh
#
# Number of SUCCESSes:  3
#
#
# Run test of KeyChange TC against data given by Bert Wijnen in email.
#
# NOTE	The snmp_debug flag bit DEBUG_RANDOMZEROS in file snmplib/debug.h
#	MUST be set before compiling and linking libsnmp.a to testing/ktest
#	for this to succeed.
#


. eval_tools.sh

VERIFY ktest

STARTTEST


#------------------------------------ -o- 
# Declarations.
#
DATAFILE_PREFIX=data.keychange-
DATAFILE_SUFFIXES="md5 sha1 sha1-des"

oldkul=
newkul=
keychangestring=



#------------------------------------ -o- 
# Test.
#
for dfs in $DATAFILE_SUFFIXES; do
	OUTPUT "== Test of KeyChange TC correctness for \"$dfs\"."

	set x `awk '{ print $1 }' ${DATAFILE_PREFIX}$dfs`
	shift

	[ $# -lt 3 ] && FAILED 1 \
	    "Wrong number of lines ($#) in datafile \"$DATAFILE_PREFIX}$dfs\"."

	oldkey=$1
	newkey=$2
	keychangestring=$3

	CAPTURE "ktest -k -O $oldkey -N $newkey"
	FAILED $? "ktest"

	CHECKEXACT $keychangestring
	[ $? -eq 1 ]
	FAILED $? "Proper KeyChange string was not generated."


	SUCCESS "KeyChange TC correctness test for \"$dfs\"."
done




#------------------------------------ -o- 
# Cleanup, exit.
#
STOPTEST

exit $failcount

