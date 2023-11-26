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

for i in etc bin lib usr/lib sbin proc dev sys usr/sbin usr/bin oldroot
do
	mkdir -p ${R}/$i
done
for i in /bin/busybox /bin/sh /bin/mount /bin/umount /bin/sync /bin/ls /bin/cat /bin/ps /bin/cp /bin/login /bin/mv /sbin/reboot \
		/sbin/pivot_root /usr/sbin/chroot /bin/dd /bin/sleep /bin/echo /sbin/mtd /sbin/hotplug2 \
	/sbin/rc /sbin/hdparm /sbin/event /sbin/startservice /sbin/stopservice /sbin/service /sbin/write /sbin/ledtool \
	/usr/sbin/httpd /sbin/service /usr/sbin/writetool /sbin/watchdog \
	/usr/sbin/sdparm /usr/sbin/wpa_supplicant  /usr/sbin/hostapd  /usr/sbin/wpad /usr/bin/killall
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
mount -o remount,ro /usr/local
umount /usr/local
cp -av /dev ${R}/
service samba3 stop
service nfs stop
service snmp stop
service transmission stop
service plex stop
service ftpsrv stop
service freeradius stop
service pppoeserver stop
service upnp stop
service olsrd stop
service rsync stop
service nfs stop
service dnsmasq stop
service syslog stop
service smartd stop
service sshd stop
service telnetd stop
#not nice, but works.
rm -f /tmp/services/watchdog.0
for i in /tmp/services/* ; do echo `basename $i .0` ; done | grep -v "httpd" | while read service ; do service $service stop ; done
killall process_monitor
killall mstpd
killall resetbutton
killall cron
killall -9 rpc.mountd
killall -9 rpcbind
killall -9 rpc.nfsd
killall -9 rpc.statd
killall -9 irqbalance
killall -9 mstpd
killall -9 wland
killall wdswatchdog.sh
killall schedulerb.sh
killall proxywatchdog.sh
nvram set flash_active 1
service syslog stop
mount -f -o remount,ro /jffs
umount -r -f /jffs
service syslog start

nvram set shutdown=fast
# disable write cache
hdparm -W 0 ${MTDPART}
# flush buffer cache
hdparm -f ${MTDPART}
# flush drive cache
hdparm -F ${MTDPART}
sdparm -s WCE -S ${MTDPART}
sdparm -c WCE -S ${MTDPART}

if [ -f /tmp/${UPDATEFILE} ]
then
	mv /tmp/${UPDATEFILE} ${R}/tmp/
fi
cd ${R}
pivot_root . oldroot
exec chroot . /bin/sh /bin/update-after-pivot.sh ${FIFO} ${MTDPART} ${REBOOT} ${USEDD} <dev/console >dev/console 2>&1
