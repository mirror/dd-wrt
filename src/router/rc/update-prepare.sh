#!/bin/sh -x
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

if [ x$5 = xusedd ]
then
	USEDD=1
else
	USEDD=0
fi

if [ x$2 = x ]
then
	echo "No mtd partition given, exit"
	exit
fi
MTDPART=$2


copylibs(){
P=$2
/usr/sbin/ldd.sh $1 | awk '{if (NF == 4) print $1" "$3}' | while read link file
do
		DIR=`dirname $file`
		if [ ! -d ${DIR} ]
		then
			mkdir -p ${P}/${DIR}
		fi

		if [ -L $file ] ;then 
			ODIR="$PWD"
			LFILEDIR="$(dirname $file)"
			cd "$LFILEDIR"
			lfile="$(ls -l $file | awk '{print $NF}')"
			cp -a $lfile ${P}/${LFILEDIR}/ 2>&1 | grep -v "File exists"
			cd "$ODIR"
		fi

		cp -a $file ${P}/${DIR}/ 2>&1 | grep -v "File exists"
		cp -a ${DIR}/${link} ${P}/${DIR}/ 2>&1 | grep -v "File exists"
done
}
R=/tmp/new_root/
# for pivot we need a new root, folder is not enough
if [ x$3 != xnomount ]
then
	mkdir -p ${R}
	mount -n -t tmpfs none $R
	mkdir -p ${R}/tmp
fi

for i in etc bin lib usr/lib sbin proc dev sys usr/sbin oldroot
do
	mkdir -p ${R}/$i
done
for i in /bin/busybox /bin/sh /bin/mount /bin/umount /bin/sync /bin/ls /bin/cat /bin/ps /bin/cp /bin/login /bin/mv /sbin/reboot \
		/sbin/pivot_root /usr/sbin/chroot /bin/dd \
	/sbin/mtd \
	/sbin/rc /sbin/event /sbin/startservice /sbin/stopservice /sbin/write /sbin/ledtool \
	/usr/sbin/httpd /etc/www /etc/passwd /lib/services.so /usr/lib/validate.so /usr/lib/visuals.so 
	
do
	cp -a $i $R/$i
	copylibs $i $R
done

#for httpd:
# for big mem routers ok, this is mostly 2.xMB
cp /etc/www $R/etc
cp /usr/sbin/update-after-pivot.sh $R/bin
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
exec chroot . /bin/sh /bin/update-after-pivot.sh ${FIFO} ${MTDPART} ${REBOOT} ${USEDD} <dev/console >dev/console 2>&1
