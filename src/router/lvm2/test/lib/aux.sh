#!/usr/bin/env bash
# Copyright (C) 2011-2017 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

. lib/utils

test -n "$BASH" && set -euE -o pipefail

run_valgrind() {
	# Execute script which may use $TESTNAME for creating individual
	# log files for each execute command
	exec "${VALGRIND:-valgrind}" "$@"
}

expect_failure() {
        echo "TEST EXPECT FAILURE"
}

check_daemon_in_builddir() {
	# skip if we don't have our own deamon...
	if test -z "${installed_testsuite+varset}"; then
		(which "$1" 2>/dev/null | grep -q "$abs_builddir") || skip "$1 is not in executed path."
	fi
	rm -f debug.log strace.log
}

create_corosync_conf() {
	COROSYNC_CONF="/etc/corosync/corosync.conf"
	COROSYNC_NODE=$(hostname)

	if test -a "$COROSYNC_CONF"; then
		if ! grep "created by lvm test suite" "$COROSYNC_CONF"; then
			rm "$COROSYNC_CONF"
		else
			mv "$COROSYNC_CONF" "$COROSYNC_CONF.prelvmtest"
		fi
	fi

	sed -e "s/@LOCAL_NODE@/$COROSYNC_NODE/" lib/test-corosync-conf > "$COROSYNC_CONF"
	echo "created new $COROSYNC_CONF"
}

DLM_CONF="/etc/dlm/dlm.conf"
create_dlm_conf() {
	if test -a "$DLM_CONF"; then
		if ! grep "created by lvm test suite" "$DLM_CONF"; then
			rm "$DLM_CONF"
		else
			mv "$DLM_CONF" "$DLM_CONF.prelvmtest"
		fi
	fi

	cp lib/test-dlm-conf "$DLM_CONF"
	echo "created new $DLM_CONF"
}

prepare_dlm() {
	if pgrep dlm_controld ; then
		echo "Cannot run while existing dlm_controld process exists."
		exit 1
	fi

	if pgrep corosync; then
		echo "Cannot run while existing corosync process exists."
		exit 1
	fi

	create_corosync_conf
	create_dlm_conf

	systemctl start corosync
	sleep 1
	if ! pgrep corosync; then
		echo "Failed to start corosync."
		exit 1
	fi

	systemctl start dlm
	sleep 1
	if ! pgrep dlm_controld; then
		echo "Failed to start dlm."
		exit 1
	fi
}

SANLOCK_CONF="/etc/sanlock/sanlock.conf"
create_sanlock_conf() {
	if test -a "$SANLOCK_CONF"; then
		if ! grep "created by lvm test suite" "$SANLOCK_CONF"; then
			rm "$SANLOCK_CONF"
		else
			mv "$SANLOCK_CONF" "$SANLOCK_CONF.prelvmtest"
		fi
	fi

	cp lib/test-sanlock-conf "$SANLOCK_CONF"
	echo "created new $SANLOCK_CONF"
}

prepare_sanlock() {
	if pgrep sanlock ; then
		echo "Cannot run while existing sanlock process exists"
		exit 1
	fi

	create_sanlock_conf

	systemctl start sanlock
	if ! pgrep sanlock; then
		echo "Failed to start sanlock"
		exit 1
	fi
}

prepare_lvmlockd() {
	if pgrep lvmlockd ; then
		echo "Cannot run while existing lvmlockd process exists"
		exit 1
	fi

	if test -n "$LVM_TEST_LOCK_TYPE_SANLOCK"; then
		# make check_lvmlockd_sanlock
		echo "starting lvmlockd for sanlock"
		lvmlockd -o 2

	elif test -n "$LVM_TEST_LOCK_TYPE_DLM"; then
		# make check_lvmlockd_dlm
		echo "starting lvmlockd for dlm"
		lvmlockd

	elif test -n "$LVM_TEST_LVMLOCKD_TEST_DLM"; then
		# make check_lvmlockd_test
		echo "starting lvmlockd --test (dlm)"
		lvmlockd --test -g dlm

	elif test -n "$LVM_TEST_LVMLOCKD_TEST_SANLOCK"; then
		# FIXME: add option for this combination of --test and sanlock
		echo "starting lvmlockd --test (sanlock)"
		lvmlockd --test -g sanlock -o 2
	else
		echo "not starting lvmlockd"
		exit 0
	fi

	sleep 1
	if ! pgrep lvmlockd; then
		echo "Failed to start lvmlockd"
		exit 1
	fi
}

prepare_clvmd() {
	test "${LVM_TEST_LOCKING:-0}" -ne 3 && return # not needed

	if pgrep clvmd ; then
		skip "Cannot use fake cluster locking with real clvmd ($(pgrep clvmd)) running."
	fi

	check_daemon_in_builddir clvmd

	test -e "$DM_DEV_DIR/control" || dmsetup table >/dev/null # create control node
	# skip if singlenode is not compiled in
	(clvmd --help 2>&1 | grep "Available cluster managers" | grep -q "singlenode") || \
		skip "Compiled clvmd does not support singlenode for testing."

#	lvmconf "activation/monitoring = 1"
	local run_valgrind=""
	test "${LVM_VALGRIND_CLVMD:-0}" -eq 0 || run_valgrind="run_valgrind"
	rm -f "$CLVMD_PIDFILE"
	echo "<======== Starting CLVMD ========>"
	echo -n "## preparing clvmd..."
	# lvs is executed from clvmd - use our version
	LVM_LOG_FILE_EPOCH=CLVMD LVM_LOG_FILE_MAX_LINES=1000000 LVM_BINARY=$(which lvm) $run_valgrind clvmd -Isinglenode -d 1 -f &
	echo $! > LOCAL_CLVMD

	for i in {200..0} ; do
		test "$i" -eq 0 && die "Startup of clvmd is too slow."
		test -e "$CLVMD_PIDFILE" && test -e "${CLVMD_PIDFILE%/*}/lvm/clvmd.sock" && break
		echo -n .
		sleep .1
	done
	echo ok
}

prepare_dmeventd() {
	if pgrep dmeventd ; then
		skip "Cannot test dmeventd with real dmeventd ($(pgrep dmeventd)) running."
	fi

	check_daemon_in_builddir dmeventd
	lvmconf "activation/monitoring = 1"

	local run_valgrind=""
	test "${LVM_VALGRIND_DMEVENTD:-0}" -eq 0 || run_valgrind="run_valgrind"
	echo -n "## preparing dmeventd..."
#	LVM_LOG_FILE_EPOCH=DMEVENTD $run_valgrind dmeventd -fddddl "$@" 2>&1 &
	LVM_LOG_FILE_EPOCH=DMEVENTD $run_valgrind dmeventd -fddddl "$@" >debug.log_DMEVENTD_out 2>&1 &
	echo $! > LOCAL_DMEVENTD

	# FIXME wait for pipe in /var/run instead
	for i in {200..0} ; do
		test "$i" -eq 0 && die "Startup of dmeventd is too slow."
		test -e "${DMEVENTD_PIDFILE}" && break
		echo -n .
		sleep .1
	done
	echo ok
}

prepare_lvmpolld() {
	lvmconf "global/use_lvmpolld = 1"

	local run_valgrind=""
	test "${LVM_VALGRIND_LVMPOLLD:-0}" -eq 0 || run_valgrind="run_valgrind"

	kill_sleep_kill_ LOCAL_LVMPOLLD "${LVM_VALGRIND_LVMPOLLD:-0}"

	echo -n "## preparing lvmpolld..."
	$run_valgrind lvmpolld -f "$@" -s "$TESTDIR/lvmpolld.socket" -B "$TESTDIR/lib/lvm" -l all &
	echo $! > LOCAL_LVMPOLLD
	for i in {200..0} ; do
		test "$i" -eq 0 && die "Startup of lvmpolld is too slow."
		test -e "$TESTDIR/lvmpolld.socket" && break
		echo -n .;
		sleep .1;
	done # wait for the socket
	echo ok
}

lvmpolld_talk() {
	local use=nc
	if type -p socat >& /dev/null; then
		use=socat
	elif echo | not nc -U "$TESTDIR/lvmpolld.socket" ; then
		echo "WARNING: Neither socat nor nc -U seems to be available." 1>&2
		echo "## failed to contact lvmpolld."
		return 1
	fi

	if test "$use" = nc ; then
		nc -U "$TESTDIR/lvmpolld.socket"
	else
		socat "unix-connect:$TESTDIR/lvmpolld.socket" -
	fi | tee -a lvmpolld-talk.txt
}

lvmpolld_dump() {
	(echo 'request="dump"'; echo '##') | lvmpolld_talk "$@"
}

prepare_lvmdbusd() {
	local daemon
	rm -f debug.log_LVMDBUSD_out

	kill_sleep_kill_ LOCAL_LVMDBUSD 0

        # FIXME: This is not correct! Daemon is auto started.
	echo -n "## checking lvmdbusd is NOT running..."
	if pgrep -f -l lvmdbusd | grep python3 || pgrep -x -l lvmdbusd ; then
		skip "Cannot run lvmdbusd while existing lvmdbusd process exists"
	fi
	echo ok

	# skip if we don't have our own lvmdbusd...
	echo -n "## find lvmdbusd to use..."
	if test -z "${installed_testsuite+varset}"; then
		# NOTE: this is always present - additional checks are needed:
		daemon="$abs_top_builddir/daemons/lvmdbusd/lvmdbusd"
		if test -x "$daemon" || chmod ugo+x "$daemon"; then
			echo "$daemon"
		else
			echo "Failed to make '$daemon' executable">&2
			return 1
		fi
		# Setup the python path so we can run
		export PYTHONPATH="$abs_top_builddir/daemons"
	else
		daemon=$(which lvmdbusd || :)
		echo "$daemon"
	fi
	test -x "$daemon" || skip "The lvmdbusd daemon is missing"
	which python3 >/dev/null || skip "Missing python3"

	python3 -c "import pyudev, dbus, gi.repository" || skip "Missing python modules"

	# Copy the needed file to run on the system bus if it doesn't
	# already exist
	if [ ! -f /etc/dbus-1/system.d/com.redhat.lvmdbus1.conf ]; then
		install -m 644 "$abs_top_builddir/scripts/com.redhat.lvmdbus1.conf" /etc/dbus-1/system.d/
	fi

	echo "## preparing lvmdbusd..."
	lvmconf "global/notify_dbus = 1"

	"$daemon" --debug  > debug.log_LVMDBUSD_out 2>&1 &
	local pid=$!

	sleep 1
	echo -n "## checking lvmdbusd IS running..."
	comm=
	# TODO: Is there a better check than wait 1 second and check pid?
	if ! comm=$(ps -p $pid -o comm=) >/dev/null || [[ $comm != lvmdbusd ]]; then
		echo "Failed to start lvmdbusd daemon"
		return 1
	fi
	echo "$pid" > LOCAL_LVMDBUSD
	echo ok
}

#
# Temporary solution to create some occupied thin metadata
# This heavily depends on thin metadata output format to stay as is.
# Currently it expects 2MB thin metadata and 200MB data volume size
# Argument specifies how many devices should be created.
#
prepare_thin_metadata() {
	local devices=$1
	local transaction_id=${2:-0}
	local data_block_size=${3:-128}
	local nr_data_blocks=${4:-3200}
	local i

	echo '<superblock uuid="" time="1" transaction="'"$transaction_id"'" data_block_size="'"$data_block_size"'" nr_data_blocks="'"$nr_data_blocks"'">'
	for i in $(seq 1 "$devices")
	do
		echo ' <device dev_id="'"$i"'" mapped_blocks="37" transaction="'"$i"'" creation_time="0" snap_time="1">'
		echo '  <range_mapping origin_begin="0" data_begin="0" length="37" time="0"/>'
		echo ' </device>'
	done
	echo "</superblock>"
}

teardown_devs_prefixed() {
	local prefix=$1
	local stray=${2:-0}
	local IFS=$IFS_NL
	local once=1
	local dm

	rm -rf "${TESTDIR:?}/dev/$prefix*"

	# Send idle message to frozen raids (with hope to unfreeze them)
	for dm in $(dm_status | grep -E "$prefix.*raid.*frozen"); do
		echo "## unfreezing: dmsetup message \"${dm%:*}\""
		dmsetup message "${dm%:*}" 0 "idle" &
	done

	# Resume suspended devices first
	for dm in $(dm_info name -S "name=~$PREFIX&&suspended=Suspended"); do
		test "$dm" != "No devices found" || break
		echo "## resuming: dmsetup resume \"$dm\""
		dmsetup clear "$dm"
		dmsetup resume "$dm" &
	done

	wait

	local mounts=( $(grep "$prefix" /proc/mounts | cut -d' ' -f1) )
	if test ${#mounts[@]} -gt 0; then
		test "$stray" -eq 0 || echo "## removing stray mounted devices containing $prefix:" "${mounts[@]}"
		if umount -fl "${mounts[@]}"; then
			udev_wait
		fi
	fi

	# Remove devices, start with closed (sorted by open count)
	# Run 'dmsetup remove' in parallel
	rm -f REMOVE_FAILED
	#local listdevs=( $(dm_info name,open --sort open,name | grep "$prefix.*:0") )
	#dmsetup remove --deferred ${listdevs[@]%%:0} || touch REMOVE_FAILED

	# 2nd. loop is trying --force removal which can possibly 'unstuck' some bloked operations
	for i in 0 1; do
		test "$i" = 1 && test "$stray" = 0 && break  # no stray device removal

		while :; do
			local sortby="name"
			local num_devs=0

			# HACK: sort also by minors - so we try to close 'possibly later' created device first
			test "$i" = 0 || sortby="-minor"

			for dm in $(dm_info name,open --separator ';'  --nameprefixes --unquoted --sort open,"$sortby" -S "name=~$prefix" --mangle none || true) ; do
				test "$dm" != "No devices found" || break 2
				DM_NAME=${dm##DM_NAME=}
				DM_NAME=${DM_NAME%%;DM_OPEN*}
				DM_OPEN=${dm##*;DM_OPEN=}
				if test "$i" = 0; then
					if test "$once" = 1 ; then
						once=0
						echo "## removing stray mapped devices with names beginning with $prefix: "
					fi
					test "$DM_OPEN" = 0 || break  # stop loop with 1st. opened device
					dmsetup remove "$DM_NAME" --mangle none || true # &>/dev/null || touch REMOVE_FAILED &
				else
					dmsetup remove -f "$DM_NAME" --mangle none || true
				fi

				num_devs=$(( num_devs + 1 ))
			done

			test "$i" = 0 || break

			test "$num_devs" -gt 0 || break

			udev_wait
			wait
		done # looping till there are some removed devices
	done
}

teardown_devs() {
	# Delete any remaining dm/udev semaphores
	teardown_udev_cookies

	test ! -f MD_DEV || cleanup_md_dev
	test ! -f DEVICES || teardown_devs_prefixed "$PREFIX"
	test ! -f RAMDISK || { modprobe -r brd || true ; }

	# NOTE: SCSI_DEBUG_DEV test must come before the LOOP test because
	# prepare_scsi_debug_dev() also sets LOOP to short-circuit prepare_loop()
	if test -f SCSI_DEBUG_DEV; then
		udev_wait
		test "${LVM_TEST_PARALLEL:-0}" -eq 1 || modprobe -r scsi_debug
	else
		test ! -f LOOP || losetup -d "$(< LOOP)" || true
		test ! -f LOOPFILE || rm -f "$(< LOOPFILE)"
	fi

	not diff LOOP BACKING_DEV >/dev/null 2>&1 || rm -f BACKING_DEV
	rm -f DEVICES LOOP RAMDISK

	# Attempt to remove any loop devices that failed to get torn down if earlier tests aborted
	test "${LVM_TEST_PARALLEL:-0}" -eq 1 || test -z "$COMMON_PREFIX" || {
		local stray_loops=( $(losetup -a | grep "$COMMON_PREFIX" | cut -d: -f1) )
		test ${#stray_loops[@]} -eq 0 || {
			teardown_devs_prefixed "$COMMON_PREFIX" 1
			echo "## removing stray loop devices containing $COMMON_PREFIX:" "${stray_loops[@]}"
			for i in "${stray_loops[@]}" ; do test ! -b "$i" || losetup -d "$i" || true ; done
			# Leave test when udev processed all removed devices
			udev_wait
		}
	}
	restore_dm_mirror
}

kill_sleep_kill_() {
	local pidfile=$1
	local slow=$2

	if test -s "$pidfile" ; then
		pid=$(< "$pidfile")
		rm -f "$pidfile"
		kill -TERM "$pid" 2>/dev/null || return 0
		if test "$slow" -eq 0 ; then sleep .1 ; else sleep 1 ; fi
		kill -KILL "$pid" 2>/dev/null || true
		local wait=0
		while ps "$pid" > /dev/null && test "$wait" -le 10; do
			sleep .5
			wait=$(( wait + 1 ))
		done
	fi
}

print_procs_by_tag_() {
	(ps -o pid,args ehax | grep -we"LVM_TEST_TAG=${1:-kill_me_$PREFIX}") || true
}

count_processes_with_tag() {
	print_procs_by_tag_ | wc -l
}

kill_tagged_processes() {
	local pid
	local wait

	# read uses all vars within pipe subshell
	local pids=()
	while read -r pid wait; do
		if test -n "$pid" ; then
			echo "## killing tagged process: $pid ${wait:0:120}..."
			kill -TERM "$pid" 2>/dev/null || true
		fi
		pids+=( "$pid" )
	done < <(print_procs_by_tag_ "$@")

	test ${#pids[@]} -eq 0 && return

	# wait if process exited and eventually -KILL
	wait=0
	for pid in "${pids[@]}" ; do
		while ps "$pid" > /dev/null && test "$wait" -le 10; do
			sleep .2
			wait=$(( wait + 1 ))
		done
		test "$wait" -le 10 || kill -KILL "$pid" 2>/dev/null || true
	done
}

teardown() {
	local TEST_LEAKED_DEVICES=""
	echo -n "## teardown..."
	unset LVM_LOG_FILE_EPOCH

	if test -f TESTNAME ; then

	if test ! -f SKIP_THIS_TEST ; then
		# Evaluate left devices only for non-skipped tests
		TEST_LEAKED_DEVICES=$(dmsetup table | grep "$PREFIX" | grep -v "${PREFIX}pv") || true
	fi

	kill_tagged_processes

	if test -n "$LVM_TEST_LVMLOCKD_TEST" ; then
		echo ""
		echo "## stopping lvmlockd in teardown"
		killall lvmlockd
		sleep 1
		killall lvmlockd || true
		sleep 1
		killall -9 lvmlockd || true
	fi

	dm_table | not grep -E -q "$vg|$vg1|$vg2|$vg3|$vg4" || {
		# Avoid activation of dmeventd if there is no pid
		cfg=$(test -s LOCAL_DMEVENTD || echo "--config activation{monitoring=0}")
		if dm_info suspended,name | grep -q "^Suspended:.*$PREFIX" ; then
			echo "## skipping vgremove, suspended devices detected."
		else
			vgremove -ff "$cfg"  \
			"$vg" "$vg1" "$vg2" "$vg3" "$vg4" &>/dev/null || rm -f debug.log strace.log
		fi
	}

	kill_sleep_kill_ LOCAL_LVMDBUSD 0

	echo -n .

	kill_sleep_kill_ LOCAL_LVMPOLLD "${LVM_VALGRIND_LVMPOLLD:-0}"

	echo -n .

	kill_sleep_kill_ LOCAL_CLVMD "${LVM_VALGRIND_CLVMD:-0}"

	echo -n .

	kill_sleep_kill_ LOCAL_DMEVENTD "${LVM_VALGRIND_DMEVENTD:-0}"

	echo -n .

	test -d "$DM_DEV_DIR/mapper" && teardown_devs

	echo -n .

	fi

	test -z "$TEST_LEAKED_DEVICES" || {
		echo "## unexpected devices left dm table:"
		echo "$TEST_LEAKED_DEVICES"
		return 1
	}

	if test "${LVM_TEST_PARALLEL:-0}" = 0 && test -z "$RUNNING_DMEVENTD"; then
		not pgrep dmeventd &>/dev/null # printed in STACKTRACE
	fi

	echo -n .

	test -n "$TESTDIR" && {
		cd "$TESTOLDPWD" || die "Failed to enter $TESTOLDPWD"
		# after this delete no further write is possible
		rm -rf "${TESTDIR:?}" || echo BLA
	}

	echo "ok"
}

prepare_loop() {
	local size=$1
	shift # all other params are directly passed to all 'losetup' calls
	local i
	local slash

	test -f LOOP && LOOP=$(< LOOP)
	echo -n "## preparing loop device..."

	# skip if prepare_scsi_debug_dev() was used
	if test -f SCSI_DEBUG_DEV -a -f LOOP ; then
		echo "(skipped)"
		return 0
	fi

	test ! -e LOOP
	test -n "$DM_DEV_DIR"

	for i in 0 1 2 3 4 5 6 7; do
		test -e "$DM_DEV_DIR/loop$i" || mknod "$DM_DEV_DIR/loop$i" b 7 $i
	done

	echo -n .

	local LOOPFILE="$PWD/test.img"
	rm -f "$LOOPFILE"
	dd if=/dev/zero of="$LOOPFILE" bs=$((1024*1024)) count=0 seek=$(( size + 1 )) 2> /dev/null
	if LOOP=$(losetup "$@" -s -f "$LOOPFILE" 2>/dev/null); then
		:
	elif LOOP=$(losetup -f) && losetup "$@" "$LOOP" "$LOOPFILE"; then
		# no -s support
		:
	else
		# no -f support
		# Iterate through $DM_DEV_DIR/loop{,/}{0,1,2,3,4,5,6,7}
		for slash in '' /; do
			for i in 0 1 2 3 4 5 6 7; do
				local dev="$DM_DEV_DIR/loop$slash$i"
				! losetup "$dev" >/dev/null 2>&1 || continue
				# got a free
				losetup "$@" "$dev" "$LOOPFILE"
				LOOP=$dev
				break
			done
			test -z "$LOOP" || break
		done
	fi
	test -n "$LOOP" # confirm or fail
	BACKING_DEV=$LOOP
	echo "$LOOP" > LOOP
	echo "$LOOP" > BACKING_DEV
	echo "ok ($LOOP)"
}

prepare_ramdisk() {
	local size=$1

	echo -n "## preparing ramdisk device..."
	modprobe brd rd_size=$((size * 1024)) || return

	BACKING_DEV=/dev/ram0
	echo "ok ($BACKING_DEV)"
	touch RAMDISK
}

# A drop-in replacement for prepare_loop() that uses scsi_debug to create
# a ramdisk-based SCSI device upon which all LVM devices will be created
# - scripts must take care not to use a DEV_SIZE that will enduce OOM-killer
prepare_scsi_debug_dev() {
	local DEV_SIZE=$1
	shift # rest of params directly passed to modprobe
	local DEBUG_DEV

	rm -f debug.log strace.log
	test ! -f "SCSI_DEBUG_DEV" || return 0
	test ! -f LOOP
	test -n "$DM_DEV_DIR"

	# Skip test if scsi_debug module is unavailable or is already in use
	modprobe --dry-run scsi_debug || skip
	lsmod | not grep -q scsi_debug || skip

	# Create the scsi_debug device and determine the new scsi device's name
	# NOTE: it will _never_ make sense to pass num_tgts param;
	# last param wins.. so num_tgts=1 is imposed
	touch SCSI_DEBUG_DEV
	modprobe scsi_debug dev_size_mb="$DEV_SIZE" "$@" num_tgts=1 || skip

	for i in {1..20} ; do
		sleep .1 # allow for async Linux SCSI device registration
		DEBUG_DEV="/dev/$(grep -H scsi_debug /sys/block/sd*/device/model | cut -f4 -d /)"
		test -b "$DEBUG_DEV" && break
	done
	test -b "$DEBUG_DEV" || return 1 # should not happen

	# Create symlink to scsi_debug device in $DM_DEV_DIR
	SCSI_DEBUG_DEV="$DM_DEV_DIR/$(basename "$DEBUG_DEV")"
	echo "$SCSI_DEBUG_DEV" > SCSI_DEBUG_DEV
	echo "$SCSI_DEBUG_DEV" > BACKING_DEV
	# Setting $LOOP provides means for prepare_devs() override
	test "$DEBUG_DEV" = "$SCSI_DEBUG_DEV" || ln -snf "$DEBUG_DEV" "$SCSI_DEBUG_DEV"
}

cleanup_scsi_debug_dev() {
	teardown_devs
	rm -f SCSI_DEBUG_DEV LOOP
}

prepare_md_dev() {
	local level=$1
	local rchunk=$2
	local rdevs=$3
	local with_bitmap="--bitmap=internal"
	local coption="--chunk"
	local maj
	local mddev
	local mddir="md/"
	local mdname
	local mddevdir
	maj=$(mdadm --version 2>&1) || skip "mdadm tool is missing!"

	cleanup_md_dev

	rm -f debug.log strace.log

	case "$level" in
	"1")  coption="--bitmap-chunk" ;;
	"0")  with_bitmap="" ;;
	esac
	# Have MD use a non-standard name to avoid colliding with an existing MD device
	# - mdadm >= 3.0 requires that non-standard device names be in /dev/md/
	# - newer mdadm _completely_ defers to udev to create the associated device node
	maj=${maj##*- v}
	maj=${maj%%.*}
	[ "$maj" -ge 3 ] || mddir=""

	mdname="md_lvm_test0"
	mddev="/dev/${mddir}$mdname"
	mddevdir="$DM_DEV_DIR/$mddir"

	mdadm --create --metadata=1.0 "$mddev" --auto=md --level "$level" $with_bitmap "$coption"="$rchunk" --raid-devices="$rdevs" "${@:4}" || {
		# Some older 'mdadm' version managed to open and close devices internaly
		# and reporting non-exclusive access on such device
		# let's just skip the test if this happens.
		# Note: It's pretty complex to get rid of consequences
		#       the following sequence avoid leaks on f19
		# TODO: maybe try here to recreate few times....
		mdadm --stop "$mddev" || true
		udev_wait
		mdadm --zero-superblock "${@:4}" || true
		udev_wait
		skip "Test skipped, unreliable mdadm detected!"
	}
	test -b "$mddev" || skip "mdadm has not created device!"

	# LVM/DM will see this device
	case "$DM_DEV_DIR" in
	"/dev") readlink -f "$mddev" > MD_DEV_PV ;;
	*)	mkdir -p "$mddevdir"
		cp -LR "$mddev" "$mddevdir"
		echo "${mddevdir}${mdname}" > MD_DEV_PV ;;
	esac
	echo "$mddev" > MD_DEV
	printf "%s\n" "${@:4}" > MD_DEVICES
}

cleanup_md_dev() {
	test -f MD_DEV || return 0

	local IFS=$IFS_NL
	local dev
	local mddev
	local mddev_pv
	mddev=$(< MD_DEV)
	mddev_pv=$(< MD_DEV_PV)
	udev_wait
	mdadm --stop "$mddev" || true
	udev_wait  # wait till events are process, not zeroing to early
	test "$DM_DEV_DIR" != "/dev" && rm -rf "${mddev_pv%/*}"
	for dev in $(< MD_DEVICES); do
		mdadm --zero-superblock "$dev" || true
	done
	udev_wait
	if [ -b "$mddev" ]; then
		# mdadm doesn't always cleanup the device node
		# sleeps offer hack to defeat: 'md: md127 still in use'
		# see: https://bugzilla.redhat.com/show_bug.cgi?id=509908#c25
		sleep 2
		rm -f "$mddev"
	fi
	rm -f MD_DEV MD_DEVICES MD_DEV_PV
}

prepare_backing_dev() {
	local size=${1=32}
	shift

	if test -f BACKING_DEV; then
		BACKING_DEV=$(< BACKING_DEV)
		return 0
	elif test -b "$LVM_TEST_BACKING_DEVICE"; then
		BACKING_DEV=$LVM_TEST_BACKING_DEVICE
		echo "$BACKING_DEV" > BACKING_DEV
		return 0
	elif test "${LVM_TEST_PREFER_BRD-1}" = "1" && \
	     test ! -d /sys/block/ram0 && \
	     kernel_at_least 4 16 0 && \
	     test "$size" -lt 16384; then
		# try to use ramdisk if possible, but for
		# big allocs (>16G) do not try to use ramdisk
		# Also we can't use BRD device prior kernel 4.16
		# since they were DAX based and lvm2 often relies
		# in save table loading between exiting backend device
		# and  bio-based 'error' device.
		# However with request based DAX brd device we get this:
		# device-mapper: ioctl: can't change device type after initial table load.
		prepare_ramdisk "$size" "$@" && return
		echo "(failed)"
	fi

	prepare_loop "$size" "$@"
}

prepare_devs() {
	local n=${1:-3}
	local devsize=${2:-34}
	local pvname=${3:-pv}
	local shift=0

	# sanlock requires more space for the internal sanlock lv
	# This could probably be lower, but what are the units?
	if test -n "$LVM_TEST_LOCK_TYPE_SANLOCK" ; then
		devsize=1024
	fi

	touch DEVICES
	prepare_backing_dev $(( n * devsize ))
	# shift start of PV devices on /dev/loopXX by 1M
	not diff LOOP BACKING_DEV >/dev/null 2>&1 || shift=2048
	blkdiscard "$BACKING_DEV" 2>/dev/null || true
	echo -n "## preparing $n devices..."

	local size=$(( devsize * 2048 )) # sectors
	local count=0
	rm -f CREATE_FAILED
	init_udev_transaction
	for i in $(seq 1 "$n"); do
		local name="${PREFIX}$pvname$i"
		local dev="$DM_DEV_DIR/mapper/$name"
		DEVICES[$count]=$dev
		count=$((  count + 1 ))
		echo 0 $size linear "$BACKING_DEV" $(( ( i - 1 ) * size + shift )) > "$name.table"
		dmsetup create -u "TEST-$name" "$name" "$name.table" || touch CREATE_FAILED &
		test -f CREATE_FAILED && break;
	done
	wait
	finish_udev_transaction

	if test -f CREATE_FAILED -a -n "$LVM_TEST_BACKING_DEVICE"; then
		LVM_TEST_BACKING_DEVICE=
		rm -f BACKING_DEV CREATE_FAILED
		prepare_devs "$@"
		return $?
	fi

	# non-ephemeral devices need to be cleared between tests
	test -f LOOP -o -f RAMDISK || for d in "${DEVICES[@]}"; do
		# ensure disk header is always zeroed
		dd if=/dev/zero of="$d" bs=32k count=1
		wipefs -a "$d" 2>/dev/null || true
	done

	#for i in `seq 1 $n`; do
	#	local name="${PREFIX}$pvname$i"
	#	dmsetup info -c $name
	#done
	#for i in `seq 1 $n`; do
	#	local name="${PREFIX}$pvname$i"
	#	dmsetup table $name
	#done

	printf "%s\\n" "${DEVICES[@]}" > DEVICES
#	( IFS=$'\n'; echo "${DEVICES[*]}" ) >DEVICES
	echo "ok"
}


common_dev_() {
	local tgtype=$1
	local dev=$2
	local name=${dev##*/}
	shift 2
	local read_ms=${1-0}
	local write_ms=${2-0}

	case "$tgtype" in
	delay)
		test "$read_ms" -eq 0 && test "$write_ms" -eq 0 && {
			# zero delay is just equivalent to 'enable_dev'
			enable_dev "$dev"
			return
		}
		shift 2
		;;
	# error|zero target does not take read_ms & write_ms only offset list
	esac

	local pos
	local size
	local type
	local pvdev
	local offset

	read -r pos size type pvdev offset < "$name.table"

	for fromlen in "${@-0:}"; do
		from=${fromlen%%:*}
		len=${fromlen##*:}
		if test "$len" = "$fromlen"; then
			# Missing the colon at the end: empty len
			len=
		fi
		test -n "$len" || len=$(( size - from ))
		diff=$(( from - pos ))
		if test $diff -gt 0 ; then
			echo "$pos $diff $type $pvdev $(( pos + offset ))"
			pos=$(( pos + diff ))
		elif test $diff -lt 0 ; then
			die "Position error"
		fi

		case "$tgtype" in
		delay)
			echo "$from $len delay $pvdev $(( pos + offset )) $read_ms $pvdev $(( pos + offset )) $write_ms" ;;
		error|zero)
			echo "$from $len $tgtype" ;;
		esac
		pos=$(( pos + len ))
	done > "$name.devtable"
	diff=$(( size - pos ))
	test "$diff" -gt 0 && echo "$pos $diff $type $pvdev $(( pos + offset ))" >>"$name.devtable"

	restore_from_devtable "$dev"
}

# Replace linear PV device with its 'delayed' version
# Could be used to more deterministicaly hit some problems.
# Parameters: {device path} [read delay ms] [write delay ms] [offset[:[size]]]...
# Original device is restored when both delay params are 0 (or missing).
# If the size is missing, the remaing portion of device is taken
# i.e.  delay_dev "$dev1" 0 200 256:
delay_dev() {
	if test ! -f HAVE_DM_DELAY ; then
		target_at_least dm-delay 1 1 0 || return 0
		touch HAVE_DM_DELAY
	fi
	common_dev_ delay "$@"
}

disable_dev() {
	local dev
	local silent=""
	local error=""
	local notify=""

	while test -n "$1"; do
	    if test "$1" = "--silent"; then
		silent=1
		shift
	    elif test "$1" = "--error"; then
		error=1
		shift
	    else
		break
	    fi
	done

	udev_wait
	for dev in "$@"; do
		maj=$(($(stat -L --printf=0x%t "$dev")))
		min=$(($(stat -L --printf=0x%T "$dev")))
		echo "Disabling device $dev ($maj:$min)"
		notify="$notify $maj:$min"
		if test -n "$error"; then
		    echo 0 10000000 error | dmsetup load "$dev"
		    dmsetup resume "$dev"
		else
		    dmsetup remove -f "$dev" 2>/dev/null || true
		fi
	done
}

enable_dev() {
	local dev
	local silent=""

	if test "$1" = "--silent"; then
	    silent=1
	    shift
	fi

	rm -f debug.log strace.log
	init_udev_transaction
	for dev in "$@"; do
		local name=${dev##*/}
		dmsetup create -u "TEST-$name" "$name" "$name.table" 2>/dev/null || \
			dmsetup load "$name" "$name.table"
		# using device name (since device path does not exists yes with udev)
		dmsetup resume "$name"
	done
	finish_udev_transaction
}

# Throttle down performance of kcopyd when mirroring i.e. disk image
throttle_sys="/sys/module/dm_mirror/parameters/raid1_resync_throttle"
throttle_dm_mirror() {
	test -e "$throttle_sys" || return
	test -f THROTTLE || cat "$throttle_sys" > THROTTLE
	echo ${1-1} > "$throttle_sys"
}

# Restore original kcopyd throttle value and have mirroring fast again
restore_dm_mirror() {
	test ! -f THROTTLE || {
		cat THROTTLE > "$throttle_sys"
		rm -f THROTTLE
	}
}


# Once there is $name.devtable
# this is a quick way to restore to this table entry
restore_from_devtable() {
	local dev
	local silent=""

	if test "$1" = "--silent"; then
	    silent=1
	    shift
	fi

	rm -f debug.log strace.log
	init_udev_transaction
	for dev in "$@"; do
		local name=${dev##*/}
		dmsetup load "$name" "$name.devtable"
		dmsetup resume "$name"
	done
	finish_udev_transaction
}

#
# Convert device to device with errors
# Takes the list of pairs of error segment from:len
# Combination with zero or delay is unsupported
# Original device table is replaced with multiple lines
# i.e.  error_dev "$dev1" 8:32 96:8
error_dev() {
	common_dev_ error "$@"
}

#
# Convert existing device to a device with zero segments
# Takes the list of pairs of zero segment from:len
# Combination with error or delay is unsupported
# Original device table is replaced with multiple lines
# i.e.  zero_dev "$dev1" 8:32 96:8
zero_dev() {
	common_dev_ zero "$@"
}

backup_dev() {
	local dev

	for dev in "$@"; do
		dd if="$dev" of="${dev}.backup" bs=1024
	done
}

restore_dev() {
	local dev

	for dev in "$@"; do
		test -e "${dev}.backup" || \
			die "Internal error: $dev not backed up, can't restore!"
		dd of="$dev" if="${dev}.backup" bs=1024
	done
}

prepare_pvs() {
	prepare_devs "$@"
	pvcreate -ff "${DEVICES[@]}"
}

prepare_vg() {
	teardown_devs

	prepare_devs "$@"
	vgcreate $SHARED -s 512K "$vg" "${DEVICES[@]}"
}

extend_filter() {
	local filter

	filter=$(grep ^devices/global_filter CONFIG_VALUES | tail -n 1)
	for rx in "$@"; do
		filter=$(echo "$filter" | sed -e "s:\\[:[ \"$rx\", :")
	done
	lvmconf "$filter"
}

extend_filter_LVMTEST() {
	extend_filter "a|$DM_DEV_DIR/$PREFIX|" "$@"
}

hide_dev() {
	local filter

	filter=$(grep ^devices/global_filter CONFIG_VALUES | tail -n 1)
	for dev in "$@"; do
		filter=$(echo "$filter" | sed -e "s:\\[:[ \"r|$dev|\", :")
	done
	lvmconf "$filter"
}

unhide_dev() {
	local filter

	filter=$(grep ^devices/global_filter CONFIG_VALUES | tail -n 1)
	for dev in "$@"; do
		filter=$(echo "$filter" | sed -e "s:\"r|$dev|\", ::")
	done
	lvmconf "$filter"
}

mkdev_md5sum() {
	rm -f debug.log strace.log
	mkfs.ext2 "$DM_DEV_DIR/$1/$2" || return 1
	md5sum "$DM_DEV_DIR/$1/$2" > "md5.$1-$2"
}

generate_config() {
	if test -n "$profile_name"; then
		config_values="PROFILE_VALUES_$profile_name"
		config="PROFILE_$profile_name"
		touch "$config_values"
	else
		config_values=CONFIG_VALUES
		config=CONFIG
	fi

	LVM_TEST_LOCKING=${LVM_TEST_LOCKING:-1}
	LVM_TEST_LVMPOLLD=${LVM_TEST_LVMPOLLD:-0}
	LVM_TEST_LVMLOCKD=${LVM_TEST_LVMLOCKD:-0}
        # FIXME:dct: This is harmful! Variables are unused here and are tested not being empty elsewhere:
	#LVM_TEST_LOCK_TYPE_SANLOCK=${LVM_TEST_LOCK_TYPE_SANLOCK:-0}
	#LVM_TEST_LOCK_TYPE_DLM=${LVM_TEST_LOCK_TYPE_DLM:-0}
	if test "$DM_DEV_DIR" = "/dev"; then
	    LVM_VERIFY_UDEV=${LVM_VERIFY_UDEV:-0}
	else
	    LVM_VERIFY_UDEV=${LVM_VERIFY_UDEV:-1}
	fi
	test -f "$config_values" || {
            cat > "$config_values" <<-EOF
activation/checks = 1
activation/monitoring = 0
activation/polling_interval = 1
activation/retry_deactivation = 1
activation/snapshot_autoextend_percent = 50
activation/snapshot_autoextend_threshold = 50
activation/udev_rules = 1
activation/udev_sync = 1
activation/verify_udev_operations = $LVM_VERIFY_UDEV
activation/raid_region_size = 512
allocation/wipe_signatures_when_zeroing_new_lvs = 0
backup/archive = 0
backup/backup = 0
devices/cache_dir = "$TESTDIR/etc"
devices/default_data_alignment = 1
devices/dir = "$DM_DEV_DIR"
devices/filter = "a|.*|"
devices/global_filter = [ "a|$DM_DEV_DIR/mapper/${PREFIX}.*pv[0-9_]*$|", "r|.*|" ]
devices/md_component_detection  = 0
devices/scan = "$DM_DEV_DIR"
devices/sysfs_scan = 1
devices/write_cache_state = 0
global/abort_on_internal_errors = 1
global/cache_check_executable = "$LVM_TEST_CACHE_CHECK_CMD"
global/cache_dump_executable = "$LVM_TEST_CACHE_DUMP_CMD"
global/cache_repair_executable = "$LVM_TEST_CACHE_REPAIR_CMD"
global/detect_internal_vg_cache_corruption = 1
global/fallback_to_local_locking = 0
global/library_dir = "$TESTDIR/lib"
global/locking_dir = "$TESTDIR/var/lock/lvm"
global/locking_type=$LVM_TEST_LOCKING
global/notify_dbus = 0
global/si_unit_consistency = 1
global/thin_check_executable = "$LVM_TEST_THIN_CHECK_CMD"
global/thin_dump_executable = "$LVM_TEST_THIN_DUMP_CMD"
global/thin_repair_executable = "$LVM_TEST_THIN_REPAIR_CMD"
global/use_lvmpolld = $LVM_TEST_LVMPOLLD
global/use_lvmlockd = $LVM_TEST_LVMLOCKD
log/activation = 1
log/file = "$TESTDIR/debug.log"
log/indent = 1
log/level = 9
log/overwrite = 1
log/syslog = 0
log/verbose = 0
EOF
		# For 'rpm' builds use system installed binaries.
		# For test suite run use binaries from builddir.
		test -z "${abs_top_builddir+varset}" || {
			cat >> "$config_values" <<-EOF
dmeventd/executable = "$abs_top_builddir/test/lib/dmeventd"
global/fsadm_executable = "$abs_top_builddir/test/lib/fsadm"
EOF
		}
	}

	# append all parameters  (avoid adding empty \n)
	local v
	test $# -gt 0 && printf "%s\\n" "$@" >> "$config_values"

	declare -A CONF 2>/dev/null || {
		# Associative arrays is not available
		local s
		for s in $(cut -f1 -d/ "$config_values" | sort | uniq); do
			echo "$s {"
			local k
			for k in $(grep ^"$s"/ "$config_values" | cut -f1 -d= | sed -e 's, *$,,' | sort | uniq); do
				grep "^$k[ \t=]" "$config_values" | tail -n 1 | sed -e "s,^$s/,	 ," || true
			done
			echo "}"
			echo
		done | tee "$config" | sed -e "s,^,## LVMCONF: ,"
		return 0
	}

	local sec
	local last_sec=""

	# read sequential list and put into associative array
	while IFS= read -r v; do
		CONF["${v%%[={ ]*}"]=${v#*/}
	done < "$config_values"

	# sort by section and iterate through them
	printf "%s\\n" "${!CONF[@]}" | sort | while read -r v ; do
		sec=${v%%/*} # split on section'/'param_name
		test "$sec" = "$last_sec" || {
			test -z "$last_sec" || echo "}"
			echo "$sec {"
			last_sec=$sec
		}
		echo "    ${CONF[$v]}"
	done > "$config"
	echo "}" >> "$config"

	sed -e "s,^,## LVMCONF: ," "$config"
}

lvmconf() {
	local profile_name=""
	test $# -eq 0 || {
		# Compare if passed args aren't already all in generated lvm.conf
		local needed=0
		for i in "$@"; do
			val=$(grep "${i%%[={ ]*}" CONFIG_VALUES 2>/dev/null | tail -1) || { needed=1; break; }
			test "$val" = "$i" || { needed=1; break; }
		done
		test "$needed" -eq 0 && {
			echo "## Skipping reconfiguring for: (" "$@" ")"
			return 0 # not needed
		}
	}
	generate_config "$@"
	mv -f CONFIG "$LVM_SYSTEM_DIR/lvm.conf"
}

profileconf() {
	local pdir="$LVM_SYSTEM_DIR/profile"
	local profile_name=$1
	shift
	generate_config "$@"
	mkdir -p "$pdir"
	mv -f "PROFILE_$profile_name" "$pdir/$profile_name.profile"
}

prepare_profiles() {
	local pdir="$LVM_SYSTEM_DIR/profile"
	local profile_name
	mkdir -p "$pdir"
	for profile_name in "$@"; do
		test -L "lib/$profile_name.profile" || skip
		cp "lib/$profile_name.profile" "$pdir/$profile_name.profile"
	done
}

unittest() {
	test -x "$TESTOLDPWD/unit/unit-test" || skip
	"$TESTOLDPWD/unit/unit-test" "${@}"
}

mirror_recovery_works() {
	case "$(uname -r)" in
	  3.3.4-5.fc17.i686|3.3.4-5.fc17.x86_64) return 1 ;;
	esac
}

raid456_replace_works() {
# The way kmem_cache aliasing is done in the kernel is broken.
# It causes RAID 4/5/6 tests to fail.
#
# The problem with kmem_cache* is this:
# *) Assume CONFIG_SLUB is set
# 1) kmem_cache_create(name="foo-a")
# - creates new kmem_cache structure
# 2) kmem_cache_create(name="foo-b")
# - If identical cache characteristics, it will be merged with the previously
#   created cache associated with "foo-a".  The cache's refcount will be
#   incremented and an alias will be created via sysfs_slab_alias().
# 3) kmem_cache_destroy(<ptr>)
# - Attempting to destroy cache associated with "foo-a", but instead the
#   refcount is simply decremented.  I don't even think the sysfs aliases are
#   ever removed...
# 4) kmem_cache_create(name="foo-a")
# - This FAILS because kmem_cache_sanity_check colides with the existing
#   name ("foo-a") associated with the non-removed cache.
#
# This is a problem for RAID (specifically dm-raid) because the name used
# for the kmem_cache_create is ("raid%d-%p", level, mddev).  If the cache
# persists for long enough, the memory address of an old mddev will be
# reused for a new mddev - causing an identical formulation of the cache
# name.  Even though kmem_cache_destory had long ago been used to delete
# the old cache, the merging of caches has cause the name and cache of that
# old instance to be preserved and causes a colision (and thus failure) in
# kmem_cache_create().  I see this regularly in testing the following
# kernels:
#
# This seems to be finaly resolved with this patch:
# http://www.redhat.com/archives/dm-devel/2014-March/msg00008.html
# so we need to put here exlusion for kernes which do trace SLUB
#
	case "$(uname -r)" in
	  3.6.*.fc18.i686*|3.6.*.fc18.x86_64) return 1 ;;
	  3.9.*.fc19.i686*|3.9.*.fc19.x86_64) return 1 ;;
	  3.1[0123].*.fc18.i686*|3.1[0123].*.fc18.x86_64) return 1 ;;
	  3.1[01234].*.fc19.i686*|3.1[01234].*.fc19.x86_64) return 1 ;;
	  3.1[123].*.fc20.i686*|3.1[123].*.fc20.x86_64) return 1 ;;
	  3.14.*.fc21.i686*|3.14.*.fc21.x86_64) return 1 ;;
	  3.15.*rc6*.fc21.i686*|3.15.*rc6*.fc21.x86_64) return 1 ;;
	  3.16.*rc4*.fc21.i686*|3.16.*rc4*.fc21.x86_64) return 1 ;;
	esac
}

#
# Some 32bit kernel cannot pass some erroring magic which forces
# thin-pool to be falling into Error state.
#
# Skip test on such kernels (see: https://bugzilla.redhat.com/1310661)
#
thin_pool_error_works_32() {
	case "$(uname -r)" in
	  2.6.32-618.*.i686) return 1 ;;
	  2.6.32-623.*.i686) return 1 ;;
	  2.6.32-573.1[28].1.el6.i686) return 1 ;;
	esac
}

udev_wait() {
	pgrep udev >/dev/null || return 0
	which udevadm &>/dev/null || return 0
	if test -n "${1-}" ; then
		udevadm settle --exit-if-exists="$1" || true
	else
		udevadm settle --timeout=15 || true
	fi
}

# wait_for_sync <VG/LV>
wait_for_sync() {
	local i
	for i in {1..100} ; do
		check in_sync "$@" && return
		sleep .2
	done

	echo "Sync is taking too long - assume stuck"
	return 1
}

# Check if tests are running on 64bit architecture
can_use_16T() {
	test "$(getconf LONG_BIT)" -eq 64
}

# Check if major.minor.revision' string is 'at_least'
version_at_least() {
	local major
	local minor
	local revision
	IFS=".-" read -r major minor revision <<< "$1"
	shift

	test -z "$1" && return 0
	test -n "$major" || return 1
	test "$major" -gt "$1" && return 0
	test "$major" -eq "$1" || return 1

	test -z "$2" && return 0
	test -n "$minor" || return 1
	test "$minor" -gt "$2" && return 0
	test "$minor" -eq "$2" || return 1

	test -z "$3" && return 0
	test "$revision" -ge "$3" 2>/dev/null || return 1
}
#
# Check wheter kernel [dm module] target exist
# at least in expected version
#
# [dm-]target-name major minor revision
#
# i.e.   dm_target_at_least  dm-thin-pool  1 0
target_at_least() {
	rm -f debug.log strace.log
	case "$1" in
	  dm-*) modprobe "$1" || true ;;
	esac

	if test "$1" = dm-raid; then
		case "$(uname -r)" in
		  3.12.0*) return 1 ;;
		esac
	fi

	local version
	version=$(dmsetup targets 2>/dev/null | grep "${1##dm-} " 2>/dev/null)
	version=${version##* v}

	version_at_least "$version" "${@:2}" || {
		echo "Found $1 version $version, but requested ${*:2}." >&2
		return 1
	}
}

# Check whether the kernel driver version is greater or equal
# to the specified version. This can be used to skip tests on
# kernels where they are known to not be supported.
#
# e.g. driver_at_least 4 33
#
driver_at_least() {
	local version
	version=$(dmsetup version | tail -1 2>/dev/null)
	version=${version##*:}
	version_at_least "$version" "$@" || {
		echo "Found driver version $version, but requested" "$@" "." >&2
		return 1
	}
}

have_thin() {
	lvm segtypes 2>/dev/null | grep -q thin$ || {
		echo "Thin is not built-in." >&2
		return 1
	}
	target_at_least dm-thin-pool "$@"

	declare -a CONF=()
	# disable thin_check if not present in system
	if test -n "$LVM_TEST_THIN_CHECK_CMD" && test ! -x "$LVM_TEST_THIN_CHECK_CMD"; then
		CONF[0]="global/thin_check_executable = \"\""
	fi
	if test -n "$LVM_TEST_THIN_DUMP_CMD" && test ! -x "$LVM_TEST_THIN_DUMP_CMD"; then
		CONF[1]="global/thin_dump_executable = \"\""
	fi
	if test -n "$LVM_TEST_THIN_REPAIR_CMD" && test ! -x "$LVM_TEST_THIN_REPAIR_CMD"; then
		CONF[2]="global/thin_repair_executable = \"\""
	fi
	if test ${#CONF[@]} -ne 0 ; then
		echo "TEST WARNING: Reconfiguring" "${CONF[@]}"
		lvmconf "${CONF[@]}"
	fi
}

have_vdo() {
	lvm segtypes 2>/dev/null | grep -q vdo$ || {
		echo "VDO is not built-in." >&2
		return 1
	}
	target_at_least dm-vdo "$@"
}

have_raid() {
	target_at_least dm-raid "$@"

	# some kernels have broken mdraid bitmaps, don't use them!
	# may oops kernel, we know for sure all FC24 are currently broken
	# in general any 4.1, 4.2 is likely useless unless patched
	case "$(uname -r)" in
	  4.[12].*fc24*) return 1 ;;
	esac
}

have_raid4 () {
	local r=0

	have_raid 1 8 0 && r=1
	have_raid 1 9 1 && r=0

	return $r
}

have_cache() {
	lvm segtypes 2>/dev/null | grep -q cache$ || {
		echo "Cache is not built-in." >&2
		return 1
	}
	target_at_least dm-cache "$@"

	declare -a CONF=()
	# disable cache_check if not present in system
	if test -n "$LVM_TEST_CACHE_CHECK_CMD" -a ! -x "$LVM_TEST_CACHE_CHECK_CMD" ; then
		CONF[0]="global/cache_check_executable = \"\""
	fi
	if test -n "$LVM_TEST_CACHE_DUMP_CMD" -a ! -x "$LVM_TEST_CACHE_DUMP_CMD" ; then
		CONF[1]="global/cache_dump_executable = \"\""
	fi
	if test -n "$LVM_TEST_CACHE_REPAIR_CMD" -a ! -x "$LVM_TEST_CACHE_REPAIR_CMD" ; then
		CONF[2]="global/cache_repair_executable = \"\""
	fi
	if test ${#CONF[@]} -ne 0 ; then
		echo "TEST WARNING: Reconfiguring" "${CONF[@]}"
		lvmconf "${CONF[@]}"
	fi
}

have_tool_at_least() {
	local version
	version=$("$1" -V 2>/dev/null)
	version=${version%%-*}
	shift

	version_at_least "$version" "$@"
}

# check if lvm shell is build-in  (needs readline)
have_readline() {
	echo version | lvm &>/dev/null
}

have_multi_core() {
	which nproc &>/dev/null || return 0
	[ "$(nproc)" -ne 1 ]
}

dmsetup_wrapped() {
	udev_wait
	dmsetup "$@"
}

awk_parse_init_count_in_lvmpolld_dump() {
	printf '%s' \
	\
	$'BEGINFILE { x=0; answ=0; FS="="; key="[[:space:]]*"vkey }' \
	$'{' \
		$'if (/.*{$/) { x++ }' \
		$'else if (/.*}$/) { x-- }' \
		$'else if ( x == 2 && $1 ~ key) { value=substr($2, 2); value=substr(value, 1, length(value) - 1); }' \
		$'if ( x == 2 && value == vvalue && $1 ~ /[[:space:]]*init_requests_count/) { answ=$2 }' \
		$'if (answ > 0) { exit 0 }' \
	$'}' \
	$'END { printf "%d", answ }'
}

check_lvmpolld_init_rq_count() {
	local ret
	ret=$(awk -v vvalue="$2" -v vkey="${3:-lvname}" "$(awk_parse_init_count_in_lvmpolld_dump)" lvmpolld_dump.txt)
	test "$ret" -eq "$1" || {
		die "check_lvmpolld_init_rq_count failed. Expected $1, got $ret"
	}
}

wait_pvmove_lv_ready() {
	# given sleep .1 this is about 60 secs of waiting
	local retries=${2-300}

	if [ -e LOCAL_LVMPOLLD ]; then
		local lvid=""
		while : ; do
			test "$retries" -le 0 && die "Waiting for lvmpolld timed out"
			test -n "$lvid" || {
				# wait till wanted LV really appears
				lvid=$(get lv_field "${1//-//}" vg_uuid,lv_uuid -a 2>/dev/null) && {
					lvid=${lvid//\ /}
					lvid=${lvid//-/}
				}
			}
			test -z "$lvid" || {
				lvmpolld_dump > lvmpolld_dump.txt
				! check_lvmpolld_init_rq_count 1 "$lvid" lvid || break;
			}
			sleep .1
			retries=$((retries-1))
		done
	else
		while : ; do
			test "$retries" -le 0 && die "Waiting for pvmove LV to get activated has timed out"
			dmsetup info -c -o tables_loaded "$1" >out 2>/dev/null|| true;
			not grep Live out >/dev/null || break
			sleep .1
			retries=$((retries-1))
		done
	fi
}

# Holds device open with sleep which automatically expires after given timeout
# Prints  PID of running holding sleep process in background
hold_device_open() {
	local vgname=$1
	local lvname=$2
	local sec=${3-20} # default 20sec

	sleep "$sec" < "$DM_DEV_DIR/$vgname/$lvname" >/dev/null 2>&1 &
	SLEEP_PID=$!
	# wait till device is openned
	for i in $(seq 1 50) ; do
		if test "$(dmsetup info --noheadings -c -o open "$vgname"-"$lvname")" -ne 0 ; then
			echo "$SLEEP_PID"
			return
		fi
		sleep .1
	done

	die "$vgname-$lvname expected to be openned, but it's not!"
}

# return total memory size in kB units
total_mem() {
	local a
	local b

	while IFS=":" read -r a b ; do
		case "$a" in MemTotal*) echo "${b%% kB}" ; break ;; esac
	done < /proc/meminfo
}

kernel_at_least() {
	version_at_least "$(uname -r)" "$@"
}

test "${LVM_TEST_AUX_TRACE-0}" = "0" || set -x

test -f DEVICES && devs=$(< DEVICES)

if test "$1" = "dmsetup" ; then
    shift
    dmsetup_wrapped "$@"
else
    "$@"
fi
