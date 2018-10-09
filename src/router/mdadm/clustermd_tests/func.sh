#!/bin/bash

check_ssh()
{
	NODE1="$(grep '^NODE1' $CLUSTER_CONF | cut -d'=' -f2)"
	NODE2="$(grep '^NODE2' $CLUSTER_CONF | cut -d'=' -f2)"
	[ -z "$NODE1" -o -z "$NODE2" ] && {
		echo "Please provide node-ip in $CLUSTER_CONF."
		exit 1
	}
	for ip in $NODE1 $NODE2
	do
		ssh -o NumberOfPasswordPrompts=0 $ip -l root "pwd" > /dev/null
		[ $? -ne 0 ] && {
			echo "Please setup ssh-access with no-authorized mode."
			exit 1
		}
	done
}

fetch_devlist()
{
	ISCSI_ID="$(grep '^ISCSI_TARGET_ID' $CLUSTER_CONF | cut -d'=' -f2)"
	devlist="$(grep '^devlist' $CLUSTER_CONF | cut -d'=' -f2)"
	if [ ! -z "$ISCSI_ID" -a ! -z "$devlist" ]
	then
		echo "Config ISCSI_TARGET_ID or devlist in $CLUSTER_CONF."
		exit 1
	elif [ ! -z "$ISCSI_ID" -a -z "$devlist" ]
	then
		for ip in $NODE1 $NODE2
		do
			ssh $ip "ls /dev/disk/by-path/*$ISCSI_ID*" > /dev/null
			[ $? -ne 0 ] && {
				echo "$ip: No disks found in '$ISCSI_ID' connection."
				exit 1
			}
		done
		devlist=($(ls /dev/disk/by-path/*$ISCSI_ID*))
	fi
	# sbd disk cannot use in testing
	for i in ${devlist[@]}
	do
		sbd -d $i dump &> /dev/null
		[ $? -eq '0' ] && devlist=(${devlist[@]#$i})
	done
	for i in $(seq 0 ${#devlist[@]})
	do
		eval "dev$i=${devlist[$i]}"
	done
	[ "${#devlist[@]}" -lt 6 ] && {
		echo "Cluster-md testing requires 6 disks at least."
		exit 1
	}
}

check_dlm()
{
	if ! crm configure show | grep -q dlm
	then
		crm configure primitive dlm ocf:pacemaker:controld \
			op monitor interval=60 timeout=60 \
			meta target-role=Started &> /dev/null
		crm configure group base-group dlm
		crm configure clone base-clone base-group \
			meta interleave=true
	fi
	sleep 1
	for ip in $NODE1 $NODE2
	do
		ssh $ip "pgrep dlm_controld > /dev/null" || {
			echo "$ip: dlm_controld daemon doesn't exist."
			exit 1
		}
	done
	crm_mon -r -n1 | grep -iq "fail\|not" && {
		echo "Please clear cluster-resource errors."
		exit 1
	}
}

check_env()
{
	user=$(id -un)
	[ "X$user" = "Xroot" ] || {
		echo "testing can only be done as 'root'."
		exit 1
	}
	[ \! -x $mdadm ] && {
		echo "test: please run make everything before perform testing."
		exit 1
	}
	check_ssh
	commands=(mdadm iscsiadm bc modinfo dlm_controld
		  udevadm crm crm_mon lsblk pgrep sbd)
	for ip in $NODE1 $NODE2
	do
		for cmd in ${commands[@]}
		do
			ssh $ip "which $cmd &> /dev/null" || {
				echo "$ip: $cmd, command not found!"
				exit 1
			}
		done
		mods=(raid1 raid10 md_mod dlm md-cluster)
		for mod in ${mods[@]}
		do
			ssh $ip "modinfo $mod > /dev/null" || {
				echo "$ip: $mod, module doesn't exist."
				exit 1
			}
		done
		ssh $ip "lsblk -a | grep -iq raid"
		[ $? -eq 0 ] && {
			echo "$ip: Please run testing without running RAIDs environment."
			exit 1
		}
		ssh $ip "modprobe md_mod"
	done
	fetch_devlist
	check_dlm
	[ -d $logdir ] || mkdir -p $logdir
}

# $1/node, $2/optional
stop_md()
{
	if [ "$1" == "all" ]
	then
		NODES=($NODE1 $NODE2)
	elif [ "$1" == "$NODE1" -o "$1" == "$NODE2" ]
	then
		NODES=$1
	else
		die "$1: unknown parameter."
	fi
	if [ -z "$2" ]
	then
		for ip in ${NODES[@]}
		do
			ssh $ip mdadm -Ssq
		done
	else
		for ip in ${NODES[@]}
		do
			ssh $ip mdadm -S $2
		done
	fi
}

# $1/optional, it shows why to save log
save_log()
{
	status=$1
	logfile="$status""$_basename".log

	cat $targetdir/stderr >> $targetdir/log
	cp $targetdir/log $logdir/$_basename.log

	for ip in $NODE1 $NODE2
	do
		echo "##$ip: saving dmesg." >> $logdir/$logfile
		ssh $ip "dmesg -c" >> $logdir/$logfile
		echo "##$ip: saving proc mdstat." >> $logdir/$logfile
		ssh $ip "cat /proc/mdstat" >> $logdir/$logfile
		array=($(ssh $ip "mdadm -Ds | cut -d' ' -f2"))

		if [ ! -z "$array" -a ${#array[@]} -ge 1 ]
		then
			echo "##$ip: mdadm -D ${array[@]}" >> $logdir/$logfile
			ssh $ip "mdadm -D ${array[@]}" >> $logdir/$logfile
			md_disks=($(ssh $ip "mdadm -DY ${array[@]} | grep "/dev/" | cut -d'=' -f2"))
			cat /proc/mdstat | grep -q "bitmap"
			if [ $? -eq 0 ]
			then
				echo "##$ip: mdadm -X ${md_disks[@]}" >> $logdir/$logfile
				ssh $ip "mdadm -X ${md_disks[@]}" >> $logdir/$logfile
				echo "##$ip: mdadm -E ${md_disks[@]}" >> $logdir/$logfile
				ssh $ip "mdadm -E ${md_disks[@]}" >> $logdir/$logfile
			fi
		else
			echo "##$ip: no array assembled!" >> $logdir/$logfile
		fi
	done
	[ "$1" == "fail" ] &&
		echo "See $logdir/$_basename.log and $logdir/$logfile for details"
	stop_md all
}

do_setup()
{
	check_env
	ulimit -c unlimited
}

do_clean()
{
	for ip in $NODE1 $NODE2
	do
		ssh $ip "mdadm -Ssq; dmesg -c > /dev/null"
	done
	mdadm --zero ${devlist[@]} &> /dev/null
}

cleanup()
{
	check_ssh
	do_clean
}

# check: $1/cluster_node $2/feature $3/optional
check()
{
	NODES=()
	if [ "$1" == "all" ]
	then
		NODES=($NODE1 $NODE2)
	elif [ "$1" == "$NODE1" -o "$1" == "$NODE2" ]
	then
		NODES=$1
	else
		die "$1: unknown parameter."
	fi
	case $2 in
		spares )
			for ip in ${NODES[@]}
			do
				spares=$(ssh $ip "tr '] ' '\012\012' < /proc/mdstat | grep -c '(S)'")
				[ "$spares" -ne "$3" ] &&
					die "$ip: expected $3 spares, but found $spares"
			done
		;;
		raid* )
			for ip in ${NODES[@]}
			do
				ssh $ip "grep -sq "$2" /proc/mdstat" ||
					die "$ip: check '$2' failed."
			done
		;;
		PENDING | recovery | resync | reshape )
			cnt=5
			for ip in ${NODES[@]}
			do
				while ! ssh $ip "grep -sq '$2' /proc/mdstat"
				do
					if [ "$cnt" -gt '0' ]
					then
						sleep 0.2
						cnt=$[cnt-1]
					else
						die "$ip: no '$2' happening!"
					fi
				done
			done
		;;
		wait )
			local cnt=60
			for ip in ${NODES[@]}
			do
				p=$(ssh $ip "cat /proc/sys/dev/raid/speed_limit_max")
				ssh $ip "echo 200000 > /proc/sys/dev/raid/speed_limit_max"
				while ssh $ip "grep -Esq '(resync|recovery|reshape|check|repair)' /proc/mdstat"
				do
					if [ "$cnt" -gt '0' ]
					then
						sleep 5
						cnt=$[cnt-1]
					else
						die "$ip: Check '$2' timeout over 300 seconds."
					fi
				done
				ssh $ip "echo $p > /proc/sys/dev/raid/speed_limit_max"
			done
		;;
		bitmap )
			for ip in ${NODES[@]}
			do
				ssh $ip "grep -sq '$2' /proc/mdstat" ||
					die "$ip: no '$2' found in /proc/mdstat."
			done
		;;
		nobitmap )
			for ip in ${NODES[@]}
			do
				ssh $ip "grep -sq 'bitmap' /proc/mdstat" &&
					die "$ip: 'bitmap' found in /proc/mdstat."
			done
		;;
		chunk )
			for ip in ${NODES[@]}
			do
				chunk_size=`awk -F',' '/chunk/{print $2}' /proc/mdstat | awk -F'[a-z]' '{print $1}'`
				[ "$chunk_size" -ne "$3" ] &&
					die "$ip: chunksize should be $3, but it's $chunk_size"
			done
		;;
		state )
			for ip in ${NODES[@]}
			do
				ssh $ip "grep -Esq 'blocks.*\[$3\]\$' /proc/mdstat" ||
					die "$ip: no '$3' found in /proc/mdstat."
			done
		;;
		nosync )
			for ip in ${NODES[@]}
			do
				ssh $ip "grep -Eq '(resync|recovery)' /proc/mdstat" &&
					die "$ip: resync or recovery is happening!"
			done
		;;
		readonly )
			for ip in ${NODES[@]}
			do
				ssh $ip "grep -sq "read-only" /proc/mdstat" ||
					die "$ip: check '$2' failed!"
			done
		;;
		dmesg )
			for ip in ${NODES[@]}
			do
				ssh $ip "dmesg | grep -iq 'error\|call trace\|segfault'" &&
					die "$ip: check '$2' prints errors!"
			done
		;;
		* )
			die "unknown parameter $2"
		;;
	esac
}
