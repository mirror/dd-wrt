#!/bin/sh
IN=`date +%s`
rm -f target.test
./mksquashfs-lzma target target.test
OUT=`date +%s`
echo $(($OUT-$IN))
