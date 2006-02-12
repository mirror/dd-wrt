#!/bin/sh
#
#   $Id: test-step.sh,v 1.1 2004/04/27 01:33:22 dyang Exp $
#
#   libnet
#   Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
#                             route|daemon9 <route@infonexus.com>
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
#   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
#   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
#   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#   SUCH DAMAGE.

#
#   Hard coded defaults (ie: user hits enter at prompts)
#

DST_IP=10.0.0.1
SRC_IP=10.0.0.2
SRC_PRT=100
DST_PRT=200
DEV=de0

echo "Libnet test code step through dealy"
echo ""
echo "This script will run you through all the testcode modules and verify
echo "functionality \(hopefully\).  You may want to use tcpdump in conjuction
echo "with this script to make sure things are working as they should be."
echo ""
echo "Although this script is here to quickly take you through the testcode"
echo "modules and confirm correct operation, the user is encouraged to descend"
echo "into the test directories and peruse the code for examples on how to use"
echo "the library effectively."
echo ""
echo "I will need some information from you for some of these tests."
echo "We don't check for sane input here, so don't be an idiot."
echo ""

#
#   Intial setup.
#

echo -n "Most of the test modules require a source IP address `echo \($SRC_IP\)`: "
read tmp
if test "$__tmp" != ""; then
    SRC_IP=__tmp
fi

echo -n "And most of the test modules also require a destinaion IP address `echo \($DST_IP\)`: "
read __tmp
if test "$__tmp" != ""; then
    DST_IP=__tmp
fi

echo -n "The TCP and UDP tests need a source port `echo \($SRC_PRT\)`: "
read __tmp
if test "$__tmp" != ""; then
    SRC_PRT=__tmp
fi

echo -n "And a destination port `echo \($DST_PRT\)`: "
read __tmp
if test "$__tmp" != ""; then
    DST_PRT=__tmp
fi

echo -n "Some need a link-layer device `echo \($DEV\)`: "
read __tml
if test "$__tmp" != ""; then
    DEV=__tmp
fi

echo -n "The ethernet tests need a source ethernet address (x:x:x:x:x:x):"
read __tmp
if test "$__tmp" != ""; then
    SRC_ETH=__tmp
fi

echo -n "And a destination ethernet address (x:x:x:x:x:x):"
read __tmp
if test "$__tmp" != ""; then
    DST_ETH=__tmp
fi

echo "Gotit!  The game can start now."

#
#   Internal win/loss variable setup.
#

TOTAL_WON=$((0))
TOTAL_LOST=$((0))

# The TCP tests
WIN=$((0))
LOSE=$((0))
cd TCP
echo ""
echo "TCP test suite"

if test -x tcp; then
    ./tcp -s$SRC_IP.$SRC_PRT -d$DST_IP.$DST_PRT
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x tcp+data; then
    ./tcp+data -s$SRC_IP.$SRC_PRT -d$DST_IP.$DST_PRT
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x tcp+data+ipopt; then
    ./tcp+data+ipopt -s$SRC_IP.$SRC_PRT -d$DST_IP.$DST_PRT
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi

echo "completed TCP test suite $WIN test(s) succeeded, $LOSE test(s) failed"
TOTAL_WON=`expr $TOTAL_WON + $WIN`
TOTAL_LOST=`expr $TOTAL_LOST + $LOSE`
cd ..

# The UDP tests
WIN=$((0))
LOSE=$((0))
cd UDP
echo ""
echo "UDP test suite"

if test -x udp; then
    ./udp -s$SRC_IP.$SRC_PRT -d$DST_IP.$DST_PRT
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x udp+data; then
    ./udp+data -s$SRC_IP.$SRC_PRT -d$DST_IP.$DST_PRT
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi

echo "completed UDP test suite $WIN test(s) succeeded, $LOSE test(s) failed"
TOTAL_WON=`expr $TOTAL_WON + $WIN`
TOTAL_LOST=`expr $TOTAL_LOST + $LOSE`
cd ..

# The ICMP tests
WIN=$((0))
LOSE=$((0))
cd ICMP
echo ""
echo "ICMP test suite"

if test -x icmp_echo; then
    ./icmp_echo -s$SRC_IP -d$DST_IP
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x icmp_timestamp; then
    ./icmp_timestamp -s$SRC_IP -d$DST_IP
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x icmp_timexceed; then
    ./icmp_timexceed -s$SRC_IP -d$DST_IP
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x icmp_unreach; then
    ./icmp_unreach -s$SRC_IP -d$DST_IP
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x silvertongue; then
    ./silvertongue $SRC_IP 200.200.200.200 $DST_IP
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi

echo "completed ICMP test suite $WIN test(s) succeeded, $LOSE test(s) failed"
TOTAL_WON=`expr $TOTAL_WON + $WIN`
TOTAL_LOST=`expr $TOTAL_LOST + $LOSE`
cd ..

# The Ethernet tests
WIN=$((0))
LOSE=$((0))
cd Ethernet
echo ""
echo "Ethernet test suite"

if test -x icmp_mask; then
    ./icmp_mask -i$DEV -s$SRC_IP -d$DST_IP
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x arp; then
    ./arp -i$DEV
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi
if test -x tcp; then
    ./tcp -i$DEV -s$SRC_IP.$SRC_PRT -d$DST_IP.$DST_PRT
    if [ $? -eq -0 ]; then
        WIN=`expr $WIN + 1`
    else
        LOSE=`expr $LOSE + 1`
    fi
    echo "-return-"
    read
fi

echo "completed Ethernet test suite $WIN test(s) succeeded, $LOSE test(s) failed"
TOTAL_WON=`expr $TOTAL_WON + $WIN`
TOTAL_LOST=`expr $TOTAL_LOST + $LOSE`
cd ..

# Random tests


# IP tests

echo "completed entire test suite $TOTAL_WON test(s) succeeded, $TOTAL_LOST test(s) failed"

# EOF
