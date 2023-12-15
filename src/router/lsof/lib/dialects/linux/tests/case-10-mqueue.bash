#!/bin/bash
source tests/common.bash

MQUEUE_MNTPOINT=/tmp/$$

TARGET=$tcasedir/mq_open
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

if grep -q mqueue /proc/mounts; then
    :
elif ! [ $(id -u) = 0 ]; then
    echo "root privileged is needed to run $(basename $0 .sh), skipping" >> $report
    exit 77
else
    mkdir -p ${MQUEUE_MNTPOINT}
    if ! mount -t mqueue none ${MQUEUE_MNTPOINT}; then
	echo "failed to mount mqeueu file system, skipping"
	exit 77
    fi
fi

umount_mqueue()
{
    if [ -d ${MQUEUE_MNTPOINT} ]; then
	umount ${MQUEUE_MNTPOINT}
	rmdir ${MQUEUE_MNTPOINT}
    fi
}

cleanup()
{
    local status=$1
    local pid=$2

    umount_mqueue
    while kill -0 $pid 2> /dev/null; do
	kill -CONT $pid
	sleep 1
    done
    exit $status
}

$TARGET | {
    if read label0 pid sep label1 fd; then
	if line=`$lsof -p $pid -a -d $fd -Ft`; then
	    if echo "$line" | grep -q PSXMQ; then
		cleanup 0 $pid
	    else
		echo "unexpected output: $line" >> $report
		cleanup 1 $pid
	    fi
	else
	    echo "lsof rejects following command line: $lsof -p $pid -a -d $fd" >> $report
	    cleanup 1 $pid
	fi
    else
	echo "$TARGET prints an unexpected line: $label0 $pid $sep $label1 $fd" >> $report
	umount_mqueue
	case "$pid" in
	    [0-9]*)
		kill $pid
		;;
	esac
	exit 1
    fi
}
