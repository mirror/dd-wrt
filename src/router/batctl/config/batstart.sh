#!/bin/sh
batifaces=`nvram show 2>/dev/null | grep "^bat_" | grep "enable=1" | awk -F "_" '{printf $2" "}'`

if [ -n "${batifaces}" ]
then
insmod /lib/batman-adv/batman-adv.ko
        for ifname in ${batifaces}
        do      
                batctl if add ${ifname}
                batbridge=`nvram get bat_${ifname}_bridge`
                ifconfig bat0 up
                brctl addif ${batbridge} bat0
        done
fi
