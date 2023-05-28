#!/bin/sh
#Copyright (C) BlueWave Projects and Services 2015-2023

do_ndsctl () {

	for tic in $(seq $timeout); do
		ndsstatus="ready"
		ndsctlout=$(ndsctl $ndsctlcmd)
		ret=$?

		if [ $ret = "0" ]; then
			ndsstatus=$(echo "$ndsctlout" | grep "Version")
			break
		elif [ $ret = "4" ]; then
			ndsstatus="busy"
			sleep 1
			continue
		else
			ndsstatus="failed"
			break
		fi
	done

	return $ret
}
iteration=$1
timeout=2
ndsctlcmd="status"
do_ndsctl
echo "Iteration: $iteration, Return Code: $ret, Return Value: $ndsstatus"

exit 0
