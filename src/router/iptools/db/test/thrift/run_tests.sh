#!/bin/sh -
#
# $Id$
#
# Run all unit tests for BDB Server. This includes
#    - Server unit tests (com.sleepycat.server)
#    - Client driver unit tests (com.sleepycat.client)
#    - DPL unit tests (com.sleepycat.client.persist)
#    - Collections unit tests (com.sleepycat.client.collections)
#    - Other unit tests for DPL and Collections (com.sleepycat.client.bind/util)

# Pre-requisite:
#    - DB Java API must be built.
#    - dist/s_thrift must run cleanly to build BDB Server jars
#      under lang/thrift/jars

# 3rd party libraries/tools requirement:
#    - Apache Ant. The ant shell script must be accessible from system path
#      or specified with the "ANT" environment variable.
#    - Junit. Require 4.10+ with hamcrest-core.jar.
#      The path to "junit.jar" must be specified with the "JUNIT_JAR"
#      environment variable.

[ "x$JUNIT_JAR" = "x" ] && {
	echo 'FAIL: unset environment variable JUNIT_JAR for junit.jar.'
	exit 1
}

[ -f $JUNIT_JAR ] || {
	echo 'FAIL: JUNIT_JAR not a valid path to the junit.jar.'
	exit 1
}

case `uname` in
	*CYGWIN*WOW64*)
	d=../../build_windows/x64/Debug
	DB_LIB_DIR="$d"
	JUNIT_JAR="`cygpath -m $JUNIT_JAR`"
	CP_SEP=";"
	PATH="`pwd`/$DB_LIB_DIR:$PATH"
	export PATH
	;;
	*CYGWIN*)
	d=../../build_windows/Win32/Debug
	DB_LIB_DIR="$d"
	JUNIT_JAR="`cygpath -m $JUNIT_JAR`"
	CP_SEP=";"
	PATH="`pwd`/$DB_LIB_DIR:$PATH"
	export PATH
	;;
	*)
	d=../../build_unix
	DB_LIB_DIR="$d/.libs"
	CP_SEP=":"
	LD_LIBRARY_PATH="`pwd`/$DB_LIB_DIR:$LD_LIBRARY_PATH"
	export LD_LIBRARY_PATH
	ANT_OPTS="-Djava.library.path=$LD_LIBRARY_PATH"
	;;
esac
JUNIT_DIR="`dirname $JUNIT_JAR`"
REQUIRED_JARS=$JUNIT_JAR$CP_SEP$JUNIT_DIR/hamcrest-core.jar
DB_JAR="$d/db.jar"
CLASSPATH="${REQUIRED_JARS}${CP_SEP}${CLASSPATH}"
export DB_JAR
export REQUIRED_JARS
export CP_SEP
export CLASSPATH
export ANT_OPTS

ANT=${ANT:-ant}

$ANT -Ddb.jar=$DB_JAR clean
$ANT -Ddb.jar=$DB_JAR test
$ANT -Ddb.jar=$DB_JAR clean

exit 0

