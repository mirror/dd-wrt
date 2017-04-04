#!/bin/sh
found=0
start_gps=0
for port in /dev/ttyUSB*
do
        RET=`comgt -s -d ${port} /etc/comgt/sierradetect.comgt 2>/dev/null`
        if [ x${RET} = xcontrol ]
        then
                found=1
                nvram set 3gss=1
                nvram set 3ginfoiface=${port}
		if [ ${port} == "/dev/ttyUSB2" ]
		then
			nvram set gps_tty="/dev/ttyUSB1"
			start_gps=1
		fi
		if [ ${port} == "/dev/ttyUSB3" ]
		then
			nvram set gps_tty="/dev/ttyUSB2"
			start_gps=1
		fi
		if [ x${start_gps} = x1 ]
		then
			#export COMGTATC='AT!GPSFIX=1,30,10' ; \
			#	comgt -s -d ${port} /etc/comgt/atcommand.comgt
			export COMGTATC='AT!GPSTRACK=1,255,50,1000,1' ; \
				comgt -s -d ${port} /etc/comgt/atcommand.comgt
		fi
        fi
done
if [ ${found} = 0 ]
then
        nvram unset 3gss
        nvram unset 3ginfoiface
fi
