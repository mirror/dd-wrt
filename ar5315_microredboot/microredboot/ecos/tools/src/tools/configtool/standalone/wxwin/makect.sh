#!/bin/sh
# A shell script for helping to compile the Configuration Tool

LEVEL=$1
TARGET="$2 $3 $4 $5 $6 $7 $8 $9"

if [ "$1" = "" ] || [ "$2" = "" ]; then
  echo Usage: makect debug/release full/wx/ct/ecc/cleanall
  exit
fi

if [ "$LEVEL" != "debug" ] && [ "$LEVEL" != "release" ]; then
  echo First argument should be debug or release.
  exit
fi

ECOSDIR=/home/julians/cvs/eCos

# Necessary if you are using a local version of the compiler
# and these haven't been installed
export BISON_SIMPLE=/usr/lib/bison.simple
export BISON_HAIRY=/usr/lib/bison.hairy
export ECOSDIR
export CONFIGTOOLSRCDIR=$ECOSDIR/host/tools/configtool/standalone/wxwin
export WXDIR=/home/julians/local/wx2dev/wxWindows
make -f ${CONFIGTOOLSRCDIR}/Makefile WXDIR=$WXDIR ECOSDIR=$ECOSDIR LEVEL=$LEVEL $TARGET