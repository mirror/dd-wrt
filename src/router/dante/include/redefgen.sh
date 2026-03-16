#!/bin/sh
# $Id: redefgen.sh,v 1.9 2013/10/25 17:43:38 karls Exp $
#
# generate redefac.h from autoheader.h.in
#
PATH=/bin:/usr/bin:/sbin:/usr/sbin

IN=$1/autoconf.h.in
OUT=$1/redefac.h

echo "/* Do not modify, generated from ${IN} */" > ${OUT}

for define in `egrep '^#undef' < $IN | egrep 'HAVE|NEED|FALLBACK|BAREFOOT' | awk '{ print $2 }'`; do
    echo "
#ifndef ${define}
#define ${define} 0
#endif" >> $OUT
done
