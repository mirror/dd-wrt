#!/bin/sh
FIFO=$1
MTDPART=$2

/bin/busybox mount -o remount,ro /oldroot/
umount /oldroot/sys/kernel/debug/
umount /oldroot/sys
umount /oldroot/dev/pts
umount /oldroot/proc
umount -l /oldroot
cd /tmp
if [ x$3 = x1 ]
then
	mtd erase ${MTDPART}
	mtd -f write ${FIFO} ${MTDPART}
else
# if i use the fifo directly, i get many "attempt to write non page aligned data" erros
# and the update fails
# one "attempt to write non page aligned data" comes at the end, that seams to be ok (padding?)
# 
	cat ${FIFO} >updatefile
	mtd erase ${MTDPART}
	mtd -f write updatefile ${MTDPART}
fi
busybox sync
busybox sync
busybox sync
busybox reboot
