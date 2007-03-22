#!/bin/sh
#
# src/install.sh
# This script is part of the IPTraf installation system.  Do not attempt
# to run this directly from the command prompt.
#
# Version 3.0.0 Copyright (c) Gerard Paul Java 2002
#

if [ "$1" = "" ]; then
    echo "This script is part of the IPTraf installation system, and"
    echo "should not be run by itself."
    exit 1
fi

INSTALL=/usr/bin/install
TARGET=$1
WORKDIR=$2
LOGDIR=$3
LOCKDIR=$4

mkdir -p $TARGET
echo
echo "*** Installing executable programs and preparing work directories"
echo
echo ">>> Installing iptraf in $TARGET"
$INSTALL -m 0700 -o root -g root iptraf $TARGET
echo ">>> Installing rvnamed in $TARGET"
$INSTALL -m 0700 -o root -g root rvnamed $TARGET

if [ ! -d $WORKDIR ]; then
    echo ">>> Creating IPTraf work directory $WORKDIR"
else
    echo ">>> IPTraf work directory $WORKDIR already exists"
    rm -f $WORKDIR/othfilter.dat
fi

$INSTALL -m 0700 -o root -g root -d $WORKDIR

if [ ! -d $LOGDIR ]; then
    echo ">>> Creating IPTraf log directory $LOGDIR"
else
    echo ">>> IPTraf log directory $LOGDIR already exists"
fi
$INSTALL -m 0700 -o root -g root -d $LOGDIR

if [ ! -d $LOCKDIR ]; then
    echo ">>> Creating IPTraf lockfile directory $LOCKDIR"
else
    echo ">>> IPTraf lockfile directory $LOCKDIR already exists"
fi
$INSTALL -m 0700 -o root -g root -d $LOCKDIR
echo
echo
echo "*** iptraf, and rvnamed executables are in $TARGET"
echo "*** Log files are placed in $LOGDIR"

################# Filter clearing for 3.0 ##########################

if [ ! -f $WORKDIR/version ]; then
    echo ">>> Clearing old filter list"
    if [ -f $WORKDIR/tcpfilters.dat ]; then
        mv -f $WORKDIR/tcpfilters.dat $WORKDIR/tcpfilters.dat~
    fi
    
    if [ -f $WORKDIR/udpfilters.dat ]; then
        mv -f $WORKDIR/udpfilters.dat $WORKDIR/udpfilters.dat~
    fi

    if [ -f $WORKDIR/othipfilters.dat ]; then
        mv -f $WORKDIR/othipfilters.dat $WORKDIR/othipfilters.dat~
    fi

    rm -f $WORKDIR/savedfilters.dat
fi
####################################################################

cat version > $WORKDIR/version


exit 0

