#!/bin/bash

for module in ath{_{pci,rate_{amrr,onoe,sample},hal}} wlan{_{wep,tkip,ccmp,acl,xauth,scan_{sta,ap}},}
do
	 if grep -q ^$module /proc/modules; then modprobe -r $module; fi
done
