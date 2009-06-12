#! /bin/sh

#ORIG="/work4/OpenBSD.ORIG/sys"
ORIG="/work4/IPv6/kame/freebsd4/sys"
FILES=`cd ${ORIG};find sys kern net* -type f -or -type l | grep -v CVS`
#ECOS="/work2/ecc/ecc/net/tcpip/current"
ECOS="/work2/ecc/ecc/net/bsd_tcpip/current"

(cd ${ECOS};find src/sys include -type f | grep -v CVS >/tmp/ECOS_FILES)

for i in $FILES; do
  (echo -n "/";basename $i) >/tmp/FILE
  if fgrep -q -f /tmp/FILE /tmp/ECOS_FILES >/tmp/FOUND; then
    echo -n "$i "
    fgrep -f /tmp/FILE /tmp/ECOS_FILES
  fi
done
