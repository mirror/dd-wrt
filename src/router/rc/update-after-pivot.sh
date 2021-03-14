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
	/usr/sbin/writetool ${FIFO} ${MTDPART} 
	echo "write first time"
	dd if=${FIFO} of=${MTDPART} bs=65536 conv=fsync seek=1 skip=1
	echo "sync"
	sync
	echo "write second time"
	dd if=${FIFO} of=${MTDPART} bs=65536 conv=fsync seek=1 skip=1
	echo "sync"
	sync
	echo "write third time"
	dd if=${FIFO} of=${MTDPART} bs=65536 conv=fsync seek=1 skip=1
	echo "sync"
	sync
else
	write ${FIFO} ${MTDPART}
fi
# flush buffer cache
hdparm -f ${MTDPART}
busybox hdparm -f ${MTDPART}
# flush drive cache
hdparm -F ${MTDPART}
busybox hdparm -F ${MTDPART}
sync
sync
sync
# flush buffer cache
hdparm -f ${MTDPART}
busybox hdparm -f ${MTDPART}
# flush drive cache
hdparm -F ${MTDPART}
busybox hdparm -F ${MTDPART}
echo 1 > /proc/sys/vm/drop_caches
echo 2 > /proc/sys/vm/drop_caches
echo 3 > /proc/sys/vm/drop_caches
if [ x$3 = x1 ]
then
	sleep 10
	reboot
	sleep 20
	echo b > /proc/sysrq-trigger
fi
