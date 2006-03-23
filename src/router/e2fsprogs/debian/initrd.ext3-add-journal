#!/bin/sh
#
# /usr/share/e2fsprogs/initrd.ext3-add-journal
#
cd /
mount -nt proc proc proc
rootdev=$(cat proc/sys/kernel/real-root-dev)
cmdline=$(cat /proc/cmdline)
umount -n proc
if [ $rootdev != 256 ]; then
    mount -nt tmpfs tmpfs /dev2
    get_device
    roottype=`/bin/e2initrd_helper -r /dev2/root2`
    if test -n "$roottype" ; then
	mount -nt tmpfs tmpfs /etc
	echo >> /etc/fstab
	echo >> /etc/mtab
	if test "$roottype" = "ext3" ; then
	    /sbin/tune2fs -O has_journal /dev2/root2 > /dev/null 2>&1
	else
	    /sbin/tune2fs -O ^has_journal /dev2/root2 > /dev/null 2>&1
	fi
	umount -n /etc
    fi
    umount -n /dev2
    umount -n /proc > /dev/null 2>&1
fi
