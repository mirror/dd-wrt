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
if [ x$4 = x1 ]
then
	echo "relocate nvram"
	writetool ${FIFO} ${MTDPART} 
	echo "write first time"
	dd if=${FIFO} of=${MTDPART} bs=65536 conv=fsync
	echo "sync"
	busybox sync
	echo "write second time"
	dd if=${FIFO} of=${MTDPART} bs=65536 conv=fsync
	echo "sync"
	busybox sync
	echo "write third time"
	dd if=${FIFO} of=${MTDPART} bs=65536 conv=fsync
	echo "sync"
	busybox sync
else
	write ${FIFO} ${MTDPART}
fi
# flush buffer cache
hdparm -f ${MTDPART}
busybox hdparm -f ${MTDPART}
# flush drive cache
hdparm -F ${MTDPART}
busybox hdparm -F ${MTDPART}
busybox sync
busybox sync
busybox sync
# flush buffer cache
hdparm -f ${MTDPART}
busybox hdparm -f ${MTDPART}
# flush drive cache
hdparm -F ${MTDPART}
busybox hdparm -F ${MTDPART}
if [ x$3 = x1 ]
then
	busybox reboot
	sleep 20
	echo b > /proc/sysrq-trigger
fi
