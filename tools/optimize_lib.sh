#!/bin/sh

#set -x

NM=${CROSS_COMPILE}nm
LD=${CROSS_COMPILE}ld
STRIP=${CROSS_COMPILE}strip

DIR=$1
LIB_SO=$2
LIB_A=$3
LIB_SO_M=$4
SEARCHDIR=$5
INSTALLLIB=$6

MAP=${DIR}/.map
SYM=${DIR}/.sybmols
UNR=${DIR}/.unresolved
find "$SEARCHDIR" -name .svn -exec rm -rf {} +
BINARIES=`find "$SEARCHDIR" -path "$SEARCHDIR/lib" -prune -o -type f -printf '%p\n' | file -f - | grep ELF | cut -d':' -f1|awk '{print $0; printf ":"}'`

if [ ! -f "${DIR}/${LIB_SO}" ] ; then
	echo "Cann't find ${DIR}/${LIB_SO}";
	exit 0;
fi

if [ ! -f "${DIR}/${LIB_A}" ] ; then
	echo "Cann't find ${DIR}/${LIB_A}";
	exit 0;
fi

rm -f $MAP
rm -f $SYM
rm -f $UNR
rm -f $UNR.tmp

$NM -o --defined-only --no-sort "${DIR}/${LIB_SO}" | cut -d' ' -f3 > $MAP
IFS=":"
for bin in $BINARIES ; do 
	bin=${bin%$'\n'}
	$NM --dynamic -u --no-sort "$bin" >> $UNR
done
cp $UNR $UNR.tmp
sort -u $UNR.tmp > $UNR

echo done

IFS=" "
if [ ! -z $7 ] ; then
	for symbol in `cat $UNR` ; do
		symbol=${symbol%$'\n'}
		if grep -q "^$symbol" $MAP ; then echo "-Wl,-u,$symbol" >> $SYM ;
	fi ; done 
else
	for symbol in `cat $UNR` ; do 
		symbol=${symbol%$'\n'}
		if grep -q "^$symbol" $MAP ; then echo "-u $symbol" >> $SYM ;
	fi ; done 
fi
if ls $SYM ; then
	if [ ! -z $7 ] ; then
		echo "link with arguments"
		xargs -t $CC -Wl,-z,max-page-size=4096 -shared -o "${DIR}/${LIB_SO_M}" "${DIR}/${LIB_A}" `cat $7`< $SYM ;
	else
		echo "link with no arguments"
		xargs -t $LD -z max-page-size=4096 -shared -o "${DIR}/${LIB_SO_M}" "${DIR}/${LIB_A}" < $SYM ;
	fi
fi

if [ "a$INSTALLLIB" != "a" -a -f "${DIR}/${LIB_SO_M}" ] ; then
	echo install "${DIR}/${LIB_SO_M}" $INSTALLLIB
	install "${DIR}/${LIB_SO_M}" $INSTALLLIB
	$STRIP $INSTALLLIB
fi
