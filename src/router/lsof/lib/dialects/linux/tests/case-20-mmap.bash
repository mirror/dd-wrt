#!/bin/bash
source tests/common.bash

cleanup()
{
    if [ -n "$(mount | grep "tmp\/ext4.img" | grep -v grep)" ]
    then
        umount /tmp/ext4.img
    fi
    
    if [ -f /tmp/ext4.img ]
    then
        rm -f /tmp/ext4.img
    fi
    
    if [ -d /tmp/TEST ]
    then
        rm -rf /tmp/TEST
    fi
    
    pidlist=`ps aux | grep "TEST\/MMAP" | grep -v grep | awk '{print $2}'`
    for pid in $pidlist
    do
        kill -9 $pid
    done
}

# cleanup the environment
cleanup

# create a new namespace and mmap
output=$(unshare --mount --propagation private $3/mount-and-mmap.bash $@ 2>&1)
echo "$output" >> $report
if echo "$output" | grep -q 'Operation not permitted'; then
    echo "unshare is not supported on this platform, skipping" >> $report
    exit 77
fi

# get pid of the mmap process
pid=`ps aux | grep "TEST\/MMAP" | grep -v grep | awk '{print $2}'`
if [ -z "$pid" ]
then
    echo "mmap process does not exist" >> $report
    cleanup
    exit 1
fi

# lsof -p pid and obtain the output
output=`$lsof -p $pid | grep "TEST\/MMAP" | grep "stat: No such file or directory"`

if [ -z "$output" ]
then
    cleanup
    exit 0
else
    echo "unexpected output: $output" >> $report
    cleanup
    exit 1
fi
