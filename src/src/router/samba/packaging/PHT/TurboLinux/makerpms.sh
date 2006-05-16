#!/bin/sh
# Copyright (C) 1998 John H Terpstra, 1999 K Spoon
#
SPECDIR=/usr/src/turbo/SPECS
SRCDIR=/usr/src/turbo/SOURCES
USERID=`id -u`
GRPID=`id -g`

( cd ../../../.. ; chown -R ${USERID}.${GRPID} ${SRCDIR}/samba-2.0.7 )
( cd ../../../.. ; tar czvf ${SRCDIR}/samba-2.0.7.tar.gz samba-2.0.7 )
cp -a *.spec $SPECDIR
cp -a *.patch smb.* samba.log $SRCDIR
cd $SPECDIR
rpm -ba -v samba2.spec
