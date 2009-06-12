#! /bin/bash
#==========================================================================
#
#      tests/test_server.sh
#
#      PPP test server script
#
#==========================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
# -------------------------------------------
# This file is part of eCos, the Embedded Configurable Operating System.
# Copyright (C) 2003, 2004 eCosCentric Ltd.
#
# eCos is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 or (at your option) any later version.
#
# eCos is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with eCos; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
#
# As a special exception, if other files instantiate templates or use macros
# or inline functions from this file, or you compile this file and link it
# with other works to produce a work based on this file, this file does not
# by itself cause the resulting work to be covered by the GNU General Public
# License. However the source code for this file must still be made available
# in accordance with section (3) of the GNU General Public License.
#
# This exception does not invalidate any other reasons why a work based on
# this file might be covered by the GNU General Public License.
#
# -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
#==========================================================================
######DESCRIPTIONBEGIN####
#
# Author(s):    nickg
# Contributors:
# Date:         2003-06-26
# Purpose:      
# Description:  
#              
#
#####DESCRIPTIONEND####
#
#==========================================================================

# --------------------------------------------------------------------
# Global variables

ppp_prev=
ppp_optarg=

ppp_dev=
ppp_myip=
ppp_hisip=
ppp_debug=
ppp_redboot=
ppp_redboot_baud=38400
ppp_baud=$ppp_redboot_baud
ppp_flow=crtscts
ppp_ping_interval=1

# --------------------------------------------------------------------
# Parse the options:

for ppp_option
do

  # If the previous option needs an argument, assign it.
  if test -n "$ppp_prev"; then
    eval "$ppp_prev=\$ppp_option"
    ppp_prev=
    continue
  fi

  # If this option is of the form --thing=value then store
  # the value into $ppp_optarg.
  case "$ppp_option" in
  -*=*) ppp_optarg=`echo "$ppp_option" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) ppp_optarg= ;;
  esac

  # Now parse the option

  case "$ppp_option" in
  
  --dev) ppp_prev=ppp_dev ;;
  --dev=*) ppp_dev=$ppp_optarg ;;

  --myip) ppp_prev=ppp_myip ;;
  --myip=*) ppp_myip=$ppp_optarg ;;

  --hisip) ppp_prev=ppp_hisip ;;
  --hisip=*) ppp_hisip=$ppp_optarg ;;

  --baud) ppp_prev=ppp_baud ;;
  --baud=*) ppp_baud=$ppp_optarg ;;

  --flow) ppp_prev=ppp_flow ;;
  --flow=*) ppp_flow=$ppp_optarg ;;

  --redboot) ppp_redboot=y ;;

  --redboot-baud) ppp_prev=ppp_redboot_baud ;;
  --redboot-baud=*) ppp_redboot_baud=$ppp_optarg ;;

  --debug) ppp_debug=y ;;

  *)
	  echo "test_server: Unrecognized option: \"$ppp_option\"." >&2
	  exit 1
	  ;;
  esac
done

# --------------------------------------------------------------------
# debug echo function. This only generates output if the --debug
# flag was given.

dbecho()
{
    if [ $ppp_debug ] ; then
        echo $*
    fi
}

# --------------------------------------------------------------------
# Usage message

usage()
{
    echo "test_server --dev=<devname>"
    echo "            --myip=<myip_addr>"
    echo "            --hisip=<hisip_addr>"
    echo "            [--baud=<baud_rate>]"
    echo "            [--flow=[crtscts|xonxoff|none]]"
    echo "            [--redboot [--redboot-baud=<baud_rate>]]"
    echo "            [--debug]"
    exit 1
}

# --------------------------------------------------------------------
# Check that all the required options are present, and report their
# values.

if [ -z "$ppp_dev" ] ; then usage ; fi
if [ -z "$ppp_myip" ] ; then usage ; fi
if [ -z "$ppp_hisip" ] ; then usage ; fi

dbecho "Device : " $ppp_dev
dbecho "My IP  : " $ppp_myip
dbecho "His IP : " $ppp_hisip
dbecho "Baud   : " $ppp_baud
dbecho "Flow   : " $ppp_flow

if [ "$ppp_flow" == "none" ] ; then ppp_flow="" ; fi

# --------------------------------------------------------------------
# Bring the PPP link up by calling pppd. The pid of the PPPD is
# stored in pppd_pid for later use.

pppup()
{
    dbecho pppd $ppp_dev $ppp_baud $ppp_flow local nodetach $ppp_myip:$ppp_hisip $* &
    pppd $ppp_dev $ppp_baud $ppp_flow local nodetach $ppp_myip:$ppp_hisip $* &
    pppd_pid=$!
#    dbecho "PPPD Pid: " $pppd_pid
}

# --------------------------------------------------------------------
# Simple test for bringing PPP up. Once the link is up the remote
# end is pinged for a while and then we bring the link down by
# signalling the PPPD.

ppp_up_test()
{
    dbecho ppp_up_test
    pppup
    sleep 6
    ping -i$ppp_ping_interval -w45 -s3000 -c20 $ppp_hisip
    kill -SIGINT $pppd_pid
    wait $pppd_pid
}

# --------------------------------------------------------------------
# Up/down test. In this case the link is brought down by the remote
# end.

ppp_updown_test()
{
    dbecho ppp_updown_test
    pppup
    wait $pppd_pid
}


# --------------------------------------------------------------------
# Chat tests. These use chat itself to test the chat scripts on the
# remote end. The tests are:
#
# chat_test_1 - run throught the entire script
# chat_test_2 - simulate a carrier drop
# chat_test_3 - simulate a timeout

chat_test_1()
{
    chat -V "Chat Test" "CONNECT\rlogin:\c"  ppp "Password:\c"  hithere ""  <$ppp_dev >$ppp_dev
}

chat_test_2()
{
    chat -V "Chat Test" "CONNECT\rlogin:\c"  ppp "NO CARRIER"  <$ppp_dev >$ppp_dev
}

chat_test_3()
{
    chat -V "Chat Test" "CONNECT\rlogin:\c"  ppp  <$ppp_dev >$ppp_dev
}

# --------------------------------------------------------------------
# Authentication tests. These bring up the PPPD with different
# authentication requirements against which the remote end tests
# itself. The tests are:
#
# auth_test_1 - authenticate by default method (usually CHAP)
# auth_test_2 - require PAP authentication
# auth_test_3 - require CHAP authentication

auth_test_1()
{
    dbecho ppp_up_test
    pppup auth
    sleep 6
    ps -p $pppd_pid >/dev/null
    if [ "$?" == "0" ] ; then
        ping -i$ppp_ping_interval -w45 -s3000 -c5 $ppp_hisip
        kill -SIGINT $pppd_pid
        wait $pppd_pid
    fi
}

auth_test_2()
{
    dbecho ppp_up_test
    pppup auth require-pap
    sleep 6
    ps -p $pppd_pid >/dev/null
    if [ "$?" == "0" ] ; then
        ping -i$ppp_ping_interval -w45 -s3000 -c5 $ppp_hisip
        kill -SIGINT $pppd_pid
        wait $pppd_pid
    fi
}

auth_test_3()
{
    dbecho ppp_up_test
    pppup auth require-chap
    sleep 6
    ps -p $pppd_pid >/dev/null
    if [ "$?" == "0" ] ; then
        ping -i$ppp_ping_interval -w45 -s3000 -c5 $ppp_hisip
        kill -SIGINT $pppd_pid
        wait $pppd_pid
    fi
}


# --------------------------------------------------------------------
# TCP echo test. After bringing up the link this test runs the
# tcp_source and tcp_sink test programs to exercise the link.  This
# can take a long time so it is not really suitable for automated
# testing.

tcp_echo_test()
{
    local sink_pid
    dbecho tcp_echo_test
    pppup
    sleep 10
    tcp_sink $ppp_hisip &
    sink_pid=$!
    sleep 5
    tcp_source $ppp_hisip 60
    wait $sink_pid
    sleep 5
    wait $pppd_pid
}

# --------------------------------------------------------------------
# Network characterisation test. After bringing up the link this test
# runs the nc_test_master test program to exercise the link.  This can
# take a long time so it is not really suitable for automated testing.

nc_test_slave_test()
{
    dbecho nc_test_slave_test
    pppup
    sleep 10
    nc_test_master $ppp_hisip
    sleep 5
    wait $pppd_pid
}

# --------------------------------------------------------------------
# Change the baud rate. Depending on the value sent as part of the
# BAUD message, change the link baudrate.

new_baud()
{
    ppp_new_baud=`echo $ppp_test | sed 's/.*BAUD:\([0-9]*\).*/\1/'`
    dbecho "New Baud " $ppp_new_baud
    case $ppp_new_baud in
        016) ppp_baud=14400; ppp_ping_interval=6 ;;
        017) ppp_baud=19200; ppp_ping_interval=4 ;;
        018) ppp_baud=38400; ppp_ping_interval=2 ;;
        019) ppp_baud=57600; ppp_ping_interval=2 ;;
        020) ppp_baud=115200; ppp_ping_interval=1 ;;
        021) ppp_baud=230400 ;;

        *) dbecho "Unknown baud rate: " $ppp_new_baud
    esac
    dbecho "New Baud Rate : " $ppp_baud
}        
        


# --------------------------------------------------------------------
# Look for a RedBoot> prompt.

ppp_redboot_prompt()
{
    local done=
    dbecho ppp_redboot_prompt
    
    stty -F $ppp_dev $ppp_redboot_baud

    while [ ! $done ] ; do
        chat -V "RedBoot>" "\c" <$ppp_dev >$ppp_dev
        if [ "$?" == "0" ] ; then done=1; fi
    done
}

# --------------------------------------------------------------------
# Main loop.

while true ; do

    if [ $ppp_redboot ] ; then ppp_redboot_prompt; fi

    ppp_running=y
    
    while [ $ppp_running ] ; do

        dbecho ""

        dbecho "Setting baud rate : " $ppp_baud
        stty -F $ppp_dev $ppp_baud

        dbecho "Waiting for test..."

        read ppp_test ppp_junk < $ppp_dev

        ppp_test=`echo $ppp_test | sed 's/\([a-zA-Z_:0-9]*\).*/\1/'`
        
        dbecho "PPP test: >" $ppp_test "<"
    
        case $ppp_test in
        
            PPP_UPDOWN) ppp_updown_test ;;
        
            PPP_UP) ppp_up_test ;;

            CHAT_TEST_1) chat_test_1 ;;
            CHAT_TEST_2) chat_test_2 ;;
            CHAT_TEST_3) chat_test_3 ;;

            PPP_AUTH_1) auth_test_1 ;;
            PPP_AUTH_2) auth_test_2 ;;
            PPP_AUTH_3) auth_test_3 ;;

            TCP_ECHO) tcp_echo_test ;;

            NC_TEST_SLAVE) nc_test_slave_test ;;

            BAUD:*) new_baud ;;

            FINISH) unset ppp_running ;;
            
            *) echo "Unknown test: " $ppp_test ;;
        
        esac

    done

done

# --------------------------------------------------------------------
# end of test_server.sh

