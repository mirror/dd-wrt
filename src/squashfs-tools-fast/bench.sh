#!/bin/sh
IN=`date +%s`
rm -f target.test
./mksquashfs-lzma target target.test  -noappend -root-owned -le
#./mksquashfs-lzma target target.test  -noappend -root-owned -le -b 1048576
OUT=`date +%s`
echo $(($OUT-$IN))
