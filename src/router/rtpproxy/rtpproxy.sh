#!/bin/sh
#
# $Id$
#
# PROVIDE: rtpproxy

. /etc/rc.subr

name="rtpproxy"
rcvar="${name}_enable"

command="/usr/local/bin/rtpproxy"
pidfile="/var/run/rtpproxy.pid"

load_rc_config ${name}

rtpproxy_enable=${rtpproxy_enable:-"YES"}
rtpproxy_laddr=${rtpproxy_laddr:-"*"}

command_args="-l ${rtpproxy_laddr} -p /var/run/rtpproxy.pid"

start_precmd="touch /var/run/rtpproxy.runs"
stop_postcmd="rm -f /var/run/rtpproxy.runs"

export SIPLOG_BEND=logfile
export SIPLOG_LOGFILE_FILE=/var/log/sip.log

run_rc_command "${1}"
