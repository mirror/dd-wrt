#!/bin/sh
#
# This script generates source codes from db.thrift.
#
# Usage: gen_code.sh <target_language>

THRIFT=${THRIFT:-thrift}

if [[ "$1" == "" ]] ; then
	echo "gen_code.sh <target_language>"
else
	$THRIFT -out generated/$1/ -r -gen $1 db.thrift
fi
