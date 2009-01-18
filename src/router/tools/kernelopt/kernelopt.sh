#!/bin/sh
LINUXDIR=$1
TARGETDIR=$2
CROSS_COMPILE=$3

NM=${CROSS_COMPILE}nm
LD=${CROSS_COMPILE}ld
STRIP=${CROSS_COMPILE}strip

BINARIES=`find $TARGETDIR/lib/modules -type f -print | file -f - | grep ELF | cut -d':' -f1`

$NM -o --defined-only --no-sort ${LINUXDIR}/vmlinux | cut -d' ' -f3 > .kernsymbols
$NM --undefined-only --no-sort $BINARIES | cut -d'U' -f2 > .modsymbols

grep -v "^__kstrtab" .kernsymbols > .sympre1
grep -v "^__ksymtab" .sympre1 > .sympre2
rm -f .sympre3
for symbol in `cat .sympre2` ; do 
    echo "$symbol" >> .sympre3
done

rm -f .link

for symbol in `cat .modsymbols` ; do 
	if grep -q "^__kstrtab_$symbol" .kernsymbols ; then echo "__kstrtab_$symbol" >> .link ;
fi ; done 

for symbol in `cat .modsymbols` ; do 
	if grep -q "^__ksymtab_$symbol" .kernsymbols ; then echo "__ksymtab_$symbol" >> .link ;
fi ; done 
cat .sympre3 >> .link


if ls .link ; then 
#	objcopy  ${LINUXDIR}/vmlinux.o ${LINUXDIR}/vmlinux_opt.o -S --keep-symbols .link
	objcopy  ${LINUXDIR}/vmlinux ${LINUXDIR}/vmlinux_opt -S --keep-symbols .link
#	xargs -s 512000 -t $LD -r -o ${LINUXDIR}/vmlinux_opt.o ${LINUXDIR}/vmlinux.o < .link ;
fi
