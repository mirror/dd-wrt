#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n mikrotik/src/router/httpd)
export PATH=/opt/4.1.1/bin:$OLDPATH
cd mikrotik/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
make -f Makefile.mik kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/rb532
cd ../../../
cp mikrotik/src/router/mipsel-uclibc/*.bin ~/GruppenLW/releases/$DATE/rb532
cp mikrotik/src/router/mipsel-uclibc/root.fs ~/GruppenLW/releases/$DATE/rb532
cp mikrotik/src/router/mipsel-uclibc/vmlinux ~/GruppenLW/releases/$DATE/rb532
cp mikrotik/src/router/mipsel-uclibc/image.gz ~/GruppenLW/releases/$DATE/rb532
