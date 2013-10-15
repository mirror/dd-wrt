#! /bin/sh
#
# In daemon mode htpdate writes the systematic drift of the clock to syslog.
# With this script you can convert the PPM drift values into adjtimex
# parameters. Use it when you know what you are doing...
#
# The Linux adjtimex manpage gives you more information.
#
# Feel free to contribute for other OS's.

if [ ! $1 ]; then
	echo "Usage: $0 <PPM> [current TICK] [current FREQ]"
	echo "By default TICK=10000 and FREQ=0"
	echo
	exit 1
fi

PPM=$1
TICK=10000
FREQ=0

if [ $2 ]; then
	TICK=$2
fi
	
if [ $3 ]; then
	FREQ=$3
fi

FREQTOT=`echo "$PPM * 65536 + $TICK * 6553600 + $FREQ" | bc`

TICK=`echo "$FREQTOT / 6553600" | bc`
FREQ=`echo "$FREQTOT % 6553600" | bc | awk -F. '{print $1}'`

echo "TICK=$TICK"
echo "FREQ=$FREQ"
echo "Suggested command: adjtimex -tick $TICK -frequency $FREQ"
