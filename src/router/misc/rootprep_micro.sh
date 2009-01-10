#!/bin/bash
#
# Miscellaneous steps to prepare the root filesystem
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: rootprep.sh,v 1.1.1.5 2003/10/29 03:06:28 honor Exp $
#

ROOTDIR=$PWD

#jffs2
mkdir -p jffs
mkdir -p mmc
mkdir -p usr
mkdir -p usr/local
mkdir -p opt
mkdir -p sys
cd usr/local
ln -sf /tmp/share share
cd ../../
# tmp
mkdir -p tmp
ln -sf tmp/var var

(cd $ROOTDIR/usr && ln -sf ../tmp)

# dev
mkdir -p dev
cd dev

mknod nvram c 253 0
mknod ppp c 108 0
mknod console c 5 1
mknod tty c 5 0
mkdir tts
mknod tts/0 c 4 64
mknod tts/1 c 4 65
mkdir mtd
mknod mtd/0 c 90 0
mknod mtd/0ro c 90 1
mknod mtd/1 c 90 2
mknod mtd/1ro c 90 3
mknod mtd/2 c 90 4
mknod mtd/2ro c 90 5
mknod mtd/3 c 90 6
mknod mtd/3ro c 90 7
mknod mtd/4 c 90 8
mknod mtd/4ro c 90 9
mkdir mtdblock
mknod mtdblock/0 b 31 0
mknod mtdblock/1 b 31 1
mknod mtdblock/2 b 31 2
mknod mtdblock/3 b 31 3
mknod mtdblock/4 b 31 4
mknod urandom c 1 9
mknod zero c 1 5
mknod random c 1 8
mknod null c 1 3
mkdir misc
mknod misc/watchdog c 10 130
mknod kmem c 1 2
mknod mem c 1 1
mknod ptmx c 5 2
mknod port c 1 4
mkdir gpio 
mknod gpio/control c 254 3
mknod gpio/in c 254 0
mknod gpio/out c 254 1
mknod gpio/outen c 254 2
ln -s mtdblock/2 root
mkdir pty
mknod pty/m0 c 2 0
mknod pty/m1 c 2 1
mknod pty/m2 c 2 2
mknod pty/m3 c 2 3
mknod pty/m4 c 2 4
mknod pty/m5 c 2 5
mknod pty/m6 c 2 6
mknod pty/m7 c 2 7
mknod pty/m8 c 2 8
mknod pty/m9 c 2 9
mknod pty/m10 c 2 10
mknod pty/m11 c 2 11
mknod pty/m12 c 2 12
mknod pty/m13 c 2 13
mknod pty/m14 c 2 14
mknod pty/m15 c 2 15
mknod pty/m16 c 2 16
mknod pty/m17 c 2 17
mknod pty/m18 c 2 18
mknod pty/m19 c 2 19
mkdir pts
mknod pts/0 c 136 0

cd ..
# etc
mkdir -p etc
echo "/jffs/lib" > etc/ld.so.conf
echo "/jffs/usr/lib" >> etc/ld.so.conf
echo "/jffs/usr/local/lib" >> etc/ld.so.conf
echo "/mmc/lib" >> etc/ld.so.conf
echo "/mmc/usr/lib" >> etc/ld.so.conf
echo "/mmc/usr/local/lib" >> etc/ld.so.conf
echo "/lib" >> etc/ld.so.conf
echo "/usr/lib" >> etc/ld.so.conf
/sbin/ldconfig -r $ROOTDIR

# miscellaneous
mkdir -p mnt
mkdir -p proc
