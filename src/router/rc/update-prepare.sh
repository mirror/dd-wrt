#!/bin/sh
#
# Working update script vor Ventana for use with web and file
# for now only for MUSL
#
#
if [ x$3 = xusefile ]
then
	UPDATEFILE=`basename $1`
	FIFO=${UPDATEFILE}
else
	FIFO=`basename $1`
fi

if [ x$4 = xnoreboot ]
then
	REBOOT=0
else
	REBOOT=1
fi

if [ x$2 = x ]
then
	echo "No mtd partition given, exit"
	exit
fi
MTDPART=$2


copylibs(){
P=$2
# for musl, needs to be changed if not musl!!
# uclibc we should use ldd() { LD_TRACE_LOADED_OBJECTS=1 $*; }
#
/tmp/ldd $1 | awk '{if (NF == 4) print $1" "$3}' | while read link file
do
		DIR=`dirname $file`
		if [ ! -d ${DIR} ]
		then
			mkdir -p ${P}/${DIR}
		fi
		cp -a $file ${P}/${DIR}/
		cp -a ${DIR}/${link} ${P}/${DIR}/
done
}
R=/tmp/new_root/
# for musl... fix me for other c-library
ln -s /lib/libc.so /tmp/ldd
# for pivot we need a new root, folder is not enough
if [ x$3 != xnomount ]
then
	mkdir -p ${R}
	mount -n -t tmpfs none $R
	mkdir -p ${R}/tmp
fi

B=${R}/bin
for i in etc bin lib usr/lib sbin proc dev sys usr/sbin oldroot
do
	mkdir -p ${R}/$i
done
#cp -a /lib/ld-musl-armhf.so.1 ${R}/lib
for i in /bin/busybox /sbin/mtd /sbin/ledtool /usr/sbin/httpd
do
	cp $i $B
	copylibs $i $R
done
#for httpd:
# for big mem routers ok, this is mostly 2.xMB
cp /etc/www $R/etc
for i in validate.so visuals.so
do
		cp /usr/lib/$i $R/usr/lib/
done
for i in sh mount umount sync ls cat reboot ps cp mv
do
	(cd $B ; ln -s busybox $i )
done
(cd $R/usr/sbin/ ; ln -s ../../bin/busybox chroot )
(cd $R/sbin/ ; ln -s ../bin/busybox pivot_root )

cp /usr/sbin/update-after-pivot.sh $R/bin
cp /tmp/update-after-pivot.sh $R/bin
cat /etc/passwd >$R/etc/passwd
mount -t proc proc ${R}proc 
mount -t sysfs sys ${R}sys 
cp -av /dev ${R}/
#not nice, but works.
for i in /tmp/services/* ; do echo `basename $i .start` ; done | grep -v "httpd" | while read service ; do stopservice $service ; done
killall process_monitor
killall mstpd
killall resetbutton
if [ -f /tmp/${UPDATEFILE} ]
then
	mv /tmp/${UPDATEFILE} ${R}/tmp/
fi
cd ${R}
pivot_root . oldroot
exec chroot . /bin/sh /bin/update-after-pivot.sh ${FIFO} ${MTDPART} ${REBOOT} <dev/console >dev/console 2>&1
