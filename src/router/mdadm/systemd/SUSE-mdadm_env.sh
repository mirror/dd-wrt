#!/bin/sh

# extract configuration from /etc/sysconfig/mdadm and write
# environment to /run/sysconfig/mdadm to be used by
# systemd unit files.

MDADM_SCAN="yes"

# Following adapted from /etc/init.d/mdadmd on openSUSE

mdadmd_CONFIG=/etc/sysconfig/mdadm
if test -r $mdadmd_CONFIG; then
   . $mdadmd_CONFIG
fi

if [ x$MDADM_DELAY != x"" ]; then
  MDADM_DELAY="-d "$MDADM_DELAY;
fi

if [ x$MDADM_MAIL != x"" ]; then
  MDADM_MAIL="-m \"$MDADM_MAIL\""
fi

if [ x$MDADM_PROGRAM != x"" ]; then
  MDADM_PROGRAM="-p \"$MDADM_PROGRAM\""
fi

if [ x$MDADM_SCAN = x"yes" ]; then
  MDADM_SCAN="--scan"
else
  MDADM_SCAN=""
fi

if [ x$MDADM_SEND_MAIL_ON_START = x"yes" ]; then
  MDADM_SEND_MAIL="-t"
else
  MDADM_SEND_MAIL=""
fi

if [ x$MDADM_CONFIG != x"" ]; then
  MDADM_CONFIG="-c \"$MDADM_CONFIG\""
fi

mkdir -p /run/sysconfig
echo "MDADM_MONITOR_ARGS=$MDADM_RAIDDEVICES $MDADM_DELAY $MDADM_MAIL $MDADM_PROGRAM $MDADM_SCAN $MDADM_SEND_MAIL $MDADM_CONFIG" > /run/sysconfig/mdadm
