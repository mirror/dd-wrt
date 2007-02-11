#!/bin/sh
#
# test_kul.sh
#
# Number of SUCCESSes:	2
#
# Run key localization tests and compare with data given in the USM
# documentation.
#
# FIX	CHECKEXACT() will produce 2 lines when SCAPI is built with
#	SNMP_TESTING_CODE, but only 1 line without the #define.  The script
#	assumes SNMP_TESTING_CODE is defined.
#
#


. eval_tools.sh

VERIFY ktest

STARTTEST


#------------------------------------ -o- 
# Declarations.
#
DATAFILE_PREFIX=data.kul-
DATAFILE_SUFFIXES="md5 sha1"

P=
Ku=
engineID=
kul=



#------------------------------------ -o- 
# Test.
#
for dfs in $DATAFILE_SUFFIXES; do
	OUTPUT "== Test of key localization correctness with transform \"$dfs\"."

	set x `awk '{ print $1 }' ${DATAFILE_PREFIX}$dfs`
	shift

	[ $# -lt 4 ] && FAILED 1 \
	    "Wrong number of lines ($#) in datafile \"$DATAFILE_PREFIX}$dfs\"."

	P=$1
	Ku=$2
	engineID=$3
	kul=$4

	CAPTURE "ktest -l -P $P -E $engineID "
	FAILED $? "ktest"

	CHECKEXACT $Ku
	[ $? -eq 2 ]
	FAILED $? "Master key was not generated."

	CHECKEXACT $kul
	[ $? -eq 2 ]
	FAILED $? "Localized key was not generated."


	SUCCESS "Key localization correctness test with transform \"$dfs\"."
done




#------------------------------------ -o- 
# Cleanup, exit.
#
STOPTEST

exit $failcount

