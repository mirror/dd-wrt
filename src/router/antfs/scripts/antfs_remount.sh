#!/bin/sh

if [ $# -eq 0 ] ; then
	echo "<INFO>Usage:  $0  device"
	exit 1
fi

ntfsdir=`mount | grep $1 | cut -d' ' -f3`
ntfsdev=`mount | grep $1 | cut -d' ' -f1`

if [ "$ntfsdev" == "" ] ; then
    echo "<ERROR> No matching device found"
    exit 1
fi

rm -f $ntfsdir/testfile
dd if=/dev/urandom of=/tmp/testfile bs=1M count=10

echo "<INFO> cp to $ntfsdir"
cp /tmp/testfile $ntfsdir/

echo "<INFO> remount"
umount $ntfsdev
mount -t antfs $ntfsdev $ntfsdir -o utf8

echo "<INFO> Checksum test"
md5sum1=`md5sum /tmp/testfile | cut -d' ' -f1`
md5sum2=`md5sum $ntfsdir/testfile | cut -d' ' -f1`

rm /tmp/testfile

echo "<INFO>$md5sum1 $md5sum2"
if [ "$md5sum1" == "$md5sum2" ] ; then
    echo "<INFO> Finished successfully"
    exit 0
else
    echo "<ERROR> Checksum failed"
    exit 1
fi

