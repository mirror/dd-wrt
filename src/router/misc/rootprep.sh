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
mkdir -p usr/bin
cd usr/bin
ln -sf /opt/bin/perl perl
cd ../../
mkdir -p bin
cd bin
ln -sf /opt/bin/bash bash
cd ../
# tmp
mkdir -p tmp
ln -sf tmp/var var

(cd $ROOTDIR/usr && ln -sf ../tmp)

# dev
mkdir -p dev

# etc
mkdir -p etc
echo "/jffs/lib" > etc/ld.so.conf
echo "/jffs/usr/lib" >> etc/ld.so.conf
echo "/jffs/usr/local/lib" >> etc/ld.so.conf
echo "/mmc/lib" >> etc/ld.so.conf
echo "/mmc/usr/lib" >> etc/ld.so.conf
echo "/mmc/usr/local/lib" >> etc/ld.so.conf
echo "/opt/lib" >> etc/ld.so.conf
echo "/opt/usr/lib" >> etc/ld.so.conf
echo "/opt/usr/local/lib" >> etc/ld.so.conf
echo "/lib" >> etc/ld.so.conf
echo "/usr/lib" >> etc/ld.so.conf
/sbin/ldconfig -r $ROOTDIR

# miscellaneous
ln -sf tmp/mnt mnt
mkdir -p proc
mkdir -p opt/lib/iptables
ln -sf /opt/usr/lib/iptables usr/lib/iptables
