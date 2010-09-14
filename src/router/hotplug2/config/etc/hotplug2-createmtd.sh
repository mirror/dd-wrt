#!/bin/sh
MYDEV=`echo $DEVICENAME | cut -c1-8`
if [ x$MYDEV = xmtdblock ]
then
        MINDEV=`echo $DEVICENAME | cut -c9-`
        mkdir -p /dev/mtdblock >/dev/null 2>&1
        ln -s /dev/$DEVICENAME /dev/mtdblock/$MINDEV
#boeshack
#	if [ ! -e /dev/$DEVICENAME ]
#	then
#		echo mknod /dev/$DEVICENAME b $MAJOR $MINOR >>/tmp/hotplug-missing-stuff.txt
#		mknod /dev/$DEVICENAME b $MAJOR $MINOR
#	fi
else
        MINDEV=`echo $DEVICENAME | cut -c4-`
        mkdir -p /dev/mtd >/dev/null 2>&1
        ln -s /dev/$DEVICENAME /dev/mtd/$MINDEV
fi
