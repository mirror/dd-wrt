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
mtd erase ${MTDPART}
mtd -f write ${FIFO} ${MTDPART}
busybox sync
busybox sync
busybox sync
busybox reboot
