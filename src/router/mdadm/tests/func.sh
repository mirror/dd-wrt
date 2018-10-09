#!/bin/bash

# We test mdadm on loop-back block devices.
# dir for storing files should be settable by command line maybe
size=20000
# super0, round down to multiple of 64 and substract 64
mdsize0=19904
# super00 is nested, subtract 128
mdsize00=19840
# super1.0 round down to multiple of 2, subtract 8
mdsize1=19992
mdsize1a=19988
mdsize12=19988
# super1.2 for linear: round to multiple of 2, subtract 4
mdsize1_l=19996
mdsize2_l=19996
# subtract another 4 for bitmaps
mdsize1b=19988
mdsize11=19992
mdsize11a=19456
mdsize12=19988

# ddf needs bigger devices as 32Meg is reserved!
ddfsize=65536

# $1 is optional parameter, it shows why to save log
save_log() {
	status=$1
	logfile="$status""$_basename".log

	cat $targetdir/stderr >> $targetdir/log
	cp $targetdir/log $logdir/$_basename.log
	echo "## $HOSTNAME: saving dmesg." >> $logdir/$logfile
	dmesg -c >> $logdir/$logfile
	echo "## $HOSTNAME: saving proc mdstat." >> $logdir/$logfile
	cat /proc/mdstat >> $logdir/$logfile
	array=($(mdadm -Ds | cut -d' ' -f2))
	[ "$1" == "fail" ] &&
		echo "FAILED - see $logdir/$_basename.log and $logdir/$logfile for details"
	if [ $DEVTYPE == 'lvm' ]
	then
		# not supported lvm type yet
		echo
	elif [ "$DEVTYPE" == 'loop' -o "$DEVTYPE" == 'disk' ]
	then
		if [ ! -z "$array" -a ${#array[@]} -ge 1 ]
		then
			echo "## $HOSTNAME: mdadm -D ${array[@]}" >> $logdir/$logfile
			$mdadm -D ${array[@]} >> $logdir/$logfile
			# ignore saving external(external file, imsm...) bitmap
			cat /proc/mdstat | grep -q "linear\|external" && return 0
			md_disks=($($mdadm -D -Y ${array[@]} | grep "/dev/" | cut -d'=' -f2))
			cat /proc/mdstat | grep -q "bitmap"
			if [ $? -eq 0 ]
			then
				echo "## $HOSTNAME: mdadm -X ${md_disks[@]}" >> $logdir/$logfile
				$mdadm -X ${md_disks[@]} >> $logdir/$logfile
				echo "## $HOSTNAME: mdadm -E ${md_disks[@]}" >> $logdir/$logfile
				$mdadm -E ${md_disks[@]} >> $logdir/$logfile
			fi
		else
			echo "## $HOSTNAME: no array assembled!" >> $logdir/$logfile
		fi
	fi
}

cleanup() {
	udevadm settle
	$mdadm -Ssq 2> /dev/null
	case $DEVTYPE in
	loop )
		for d in 0 1 2 3 4 5 6 7  8 9 10 11 12 13
		do
			losetup -d /dev/loop$d &> /dev/null
			rm -f /dev/disk/by-path/loop*
			rm -f /var/tmp/mdtest$d
		done
	;;
	lvm )
		for d in 0 1 2 3 4 5 6 7  8 9 10 11 12 13
		do
			eval "lvremove --quiet -f \$dev$d"
		done
	;;
	disk )
		$mdadm --zero ${disks[@]} &> /dev/null
	;;
	esac
}

do_clean()
{
	mdadm -Ss > /dev/null
	mdadm --zero $devlist 2> /dev/null
	dmesg -c > /dev/null
}

check_env() {
	user=$(id -un)
	[ "X$user" != "Xroot" ] && {
		echo "test: testing can only be done as 'root'."
		exit 1
	}
	[ \! -x $mdadm ] && {
		echo "test: please run make everything before perform testing."
		exit 1
	}
	cmds=(mdadm lsblk df udevadm losetup mkfs.ext3 fsck seq)
	for cmd in ${cmds[@]}
	do
		which $cmd > /dev/null || {
			echo "$cmd command not found!"
			exit 1
		}
	done
	if $(lsblk -a | grep -iq raid)
	then
		# donot run mdadm -Ss directly if there are RAIDs working.
		echo "test: please run test suite without running RAIDs environment."
		exit 1
	fi
	# Check whether to run multipath tests
	modprobe multipath 2> /dev/null
	grep -sq 'Personalities : .*multipath' /proc/mdstat &&
		MULTIPATH="yes"
}

do_setup() {
	trap cleanup 0 1 3 15
	trap ctrl_c 2

	check_env
	[ -d $logdir ] || mkdir -p $logdir

	devlist=
	if [ "$DEVTYPE" == "loop" ]
	then
		# make sure there are no loop devices remaining.
		# udev started things can sometimes prevent them being stopped
		# immediately
		while grep loop /proc/partitions > /dev/null 2>&1
		do
			$mdadm -Ssq
			losetup -d /dev/loop[0-9]* 2> /dev/null
			sleep 0.2
		done
	elif [ "$DEVTYPE" == "disk" ]
	then
		if [ ! -z "$disks" ]
		then
			for d in $(seq 0 ${#disks[@]})
			do
				eval "dev$d=${disks[$d]}"
				eval devlist=\"\$devlist \$dev$d\"
				eval devlist$d=\"\$devlist\"
			done
			$mdadm --zero ${disks[@]} &> /dev/null
		else
			echo "Forget to provide physical devices for disk mode."
			exit 1
		fi
	fi
	for d in 0 1 2 3 4 5 6 7 8 9 10 11 12 13
	do
		sz=$size
		[ $d -gt 7 ] && sz=$ddfsize
		case $DEVTYPE in
		loop)
			[ -f $targetdir/mdtest$d ] ||
				dd if=/dev/zero of=$targetdir/mdtest$d count=$sz bs=1K > /dev/null 2>&1
			# make sure udev doesn't touch
			mdadm --zero $targetdir/mdtest$d 2> /dev/null
			[ -b /dev/loop$d ] || mknod /dev/loop$d b 7 $d
			if [ $d -eq 7 ]
			then
				losetup /dev/loop$d $targetdir/mdtest6 # for multipath use
			else
				losetup /dev/loop$d $targetdir/mdtest$d
			fi
			eval dev$d=/dev/loop$d
			eval file$d=$targetdir/mdtest$d
		;;
		lvm)
			unset MULTIPATH
			eval dev$d=/dev/mapper/${LVM_VOLGROUP}-mdtest$d
			if ! lvcreate --quiet -L ${sz}K -n mdtest$d $LVM_VOLGROUP
			then
				trap '' 0 # make sure lvremove is not called
				eval echo error creating \$dev$d
				exit 129
			fi
		;;
		ram)
			unset MULTIPATH
			eval dev$d=/dev/ram$d
		;;
		esac
		eval devlist=\"\$devlist \$dev$d\"
		eval devlist$d=\"\$devlist\"
		#" <-- add this quote to un-confuse vim syntax highlighting
	done
	path0=$dev6
	path1=$dev7
	ulimit -c unlimited
	[ -f /proc/mdstat ] || modprobe md_mod
	echo 2000 > /proc/sys/dev/raid/speed_limit_max
	echo 0 > /sys/module/md_mod/parameters/start_ro
}

# check various things
check() {
	case $1 in
	opposite_result )
		if [ $? -eq 0 ]; then
			die "This command shouldn't run successfully"
		fi
	;;
	spares )
		spares=$(tr '] ' '\012\012' < /proc/mdstat | grep -c '(S)' || exit 0)
		[ $spares -ne $2 ] &&
			die "expected $2 spares, found $spares"
	;;
	raid* | linear )
		grep -sq "active $1 " /proc/mdstat ||
			die "active $1 not found"
	;;
	algorithm )
		grep -sq " algorithm $2 " /proc/mdstat ||
			die "algorithm $2 not found"
	;;
	resync | recovery | reshape )
		cnt=5
		while ! grep -sq $1 /proc/mdstat
		do
			if [ $cnt -gt 0 ] && grep -v idle /sys/block/md*/md/sync_action > /dev/null
			then # Something isn't idle - wait a bit
				sleep 0.5
				cnt=$[cnt-1]
			else
				die "no $1 happening"
			fi
		done
	;;
	nosync )
		sleep 0.5
		# Since 4.2 we delay the close of recovery until there has been a chance for
		# spares to be activated.  That means that a recovery that finds nothing
		# to do can still take a little longer than expected.
		# add an extra check: is sync_completed shows the end is reached, assume
		# there is no recovery.
		if grep -sq -E '(resync|recovery|reshape) *=' /proc/mdstat
		then
			incomplete=`grep / /sys/block/md*/md/sync_completed 2> /dev/null | sed '/^ *\([0-9]*\) \/ \1/d'`
			[ -n "$incomplete" ] &&
				die "resync or recovery is happening!"
		fi
	;;
	wait )
		p=`cat /proc/sys/dev/raid/speed_limit_max`
		echo 2000000 > /proc/sys/dev/raid/speed_limit_max
		sleep 0.1
		while grep -Eq '(resync|recovery|reshape|check|repair) *=' /proc/mdstat ||
			grep -v idle > /dev/null /sys/block/md*/md/sync_action
		do
			sleep 0.5
		done
		echo $p > /proc/sys/dev/raid/speed_limit_max
	;;
	state )
		grep -sq "blocks.*\[$2\]\$" /proc/mdstat ||
			die "state $2 not found!"
		sleep 0.5
	;;
	bitmap )
		grep -sq bitmap /proc/mdstat ||
			die "no bitmap"
	;;
	nobitmap )
		grep -sq "bitmap" /proc/mdstat &&
			die "bitmap present"
	;;
	readonly )
		grep -sq "read-only" /proc/mdstat ||
			die "array is not read-only!"
	;;
	inactive )
		grep -sq "inactive" /proc/mdstat ||
			die "array is not inactive!"
	;;
	# It only can be used when there is only one raid
	chunk )
		chunk_size=`awk -F',' '/chunk/{print $2}' /proc/mdstat | awk -F'[a-z]' '{print $1}'`
		if [ "$chunk_size" -ne "$2" ] ; then
			die "chunksize should be $2, but it's $chunk_size"
		fi
	;;
	* )
		die "unknown check $1"
	;;
	esac
}

no_errors() {
	if [ -s $targetdir/stderr ]
	then
		echo Bad errors from mdadm:
		cat $targetdir/stderr
		exit 2
	fi
}

# basic device test
testdev() {
	[ -b $1 ] || die "$1 isn't a block device."
	[ "$DEVTYPE" == "disk" ] && return 0
	udevadm settle
	dev=$1
	cnt=$2
	dvsize=$3
	chunk=$4
	if [ -z "$5" ]
	then
		mkfs.ext3 -F -j $dev > /dev/null 2>&1 && fsck -fn $dev >&2
	fi
	dsize=$[dvsize/chunk]
	dsize=$[dsize*chunk]
	rasize=$[dsize*2*cnt]
	# rasize is in sectors
	if [ -n "$DEV_ROUND_K" ]
	then
		rasize=$[rasize/DEV_ROUND_K/2]
		rasize=$[rasize*DEV_ROUND_K*2]
	fi
	[ `/sbin/blockdev --getsize $dev` -eq 0 ] && sleep 2
	_sz=`/sbin/blockdev --getsize $dev`
	[ $rasize -lt $_sz -o $[rasize*4/5] -gt $_sz ] &&
		die "size is wrong for $dev: $cnt * $dvsize (chunk=$chunk) = $rasize, not $_sz"
	return 0
}

rotest() {
	dev=$1
	fsck -fn $dev >&2
}
