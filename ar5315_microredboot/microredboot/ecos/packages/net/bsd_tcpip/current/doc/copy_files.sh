#! /bin/sh

#ORIG="/work4/OpenBSD.ORIG/sys"
ORIG="/work4/IPv6/kame/freebsd4/sys"
FILES=`cd ${ORIG};find sys kern net* -type f | grep -v CVS`
#ECOS="/work2/ecc/ecc/net/tcpip/current"
#ECOS="/work2/ecc/ecc/net/bsd_tcpip/current"
ECOS=/tmp/bsd_tcpip/current

# $1 - source file from BSD
# $2 - dest file in eCos tree
# Run via:
#   find_matching_files.sh >/tmp/FILES
#   xargs -t -n2 </tmp/FILES

mkdir -p `dirname ${ECOS}/$2`
cp ${ORIG}/$1 ${ECOS}/$2
