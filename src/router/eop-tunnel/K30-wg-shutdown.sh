#!/bin/sh
# shutdown script for saving ipset used in WireGuard
# all scripts in /jffs/etc/config, /tmp/etc/config and ... see sysinit.c
# which start with Kxx (where xx is a number) are run at shutdown
# egc 2025

nv=/usr/sbin/nvram
tunnels=$($nv get oet_tunnels)
for i in $(seq 1 $tunnels); do
	if [[ $($nv get oet${i}_en) -eq 1 ]]; then
		if [[ $($nv get oet${i}_proto) -eq 2 ]] && [[ $($nv get oet${i}_failgrp) -ne 1 || $($nv get oet${i}_failstate) -eq 2 ]]; then
			if [[ $($nv get oet${i}_dpbr) -ne 0 ]]; then
				if [[ ! -z "$($nv get oet${i}_ipsetfile | sed '/^[[:blank:]]*#/d')" ]]; then
					IPSET_F=$($nv get oet${i}_ipsetfile)
					IPSET=${IPSET_F##*/}
					if [[ $($nv get oet${i}_ipsetsave) -eq 1 ]]; then
						ipset save -! > "${IPSET_F}"
						logger -p user.info "WireGuard IPSET: $IPSET saved to $IPSET_F"
					fi
				fi
			fi
		fi
	fi
done
