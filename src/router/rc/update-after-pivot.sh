#!/bin/sh
if [ x$1 = x ]
then
	echo "No file/fifo given, exit"
	exit
fi
if [ x$2 = x ]
then
	echo "No mtd partition given, exit"
	exit
fi

FIFO=$1
MTDPART=$2

/bin/busybox mount -o remount,ro /oldroot/
umount /oldroot/sys/kernel/debug/
umount /oldroot/sys
umount /oldroot/dev/pts
umount /oldroot/proc
umount -l /oldroot
cd /tmp
write ${FIFO} ${MTDPART}
busybox sync
busybox sync
busybox sync
if [ x$3 = x1 ]
then
	busybox reboot
fi
