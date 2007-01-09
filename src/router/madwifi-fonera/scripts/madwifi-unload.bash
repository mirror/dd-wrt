#!/bin/bash

for module in ath{_{pci,rate_{amrr,onoe,sample},hal}} wlan{_{wep,tkip,ccmp,acl,xauth,scan_{sta,ap}},}
do
	 grep -q ^$module /proc/modules && modprobe -r $module
done
