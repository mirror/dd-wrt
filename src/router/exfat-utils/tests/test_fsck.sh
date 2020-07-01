#!/bin/bash

TESTCASE_DIR=$1
IMAGE_FILE=exfat.img
FSCK_PROG=../build/sbin/fsck.exfat
FSCK_OPTS=-y

echo "Running $TESTCASE_DIR"
echo "-----------------------------------"

if [ "$EUID" -ne "0" ]; then
	echo "This script should be ran as root"
	exit
fi

# Set up image file as loop device
unxz -cfk $TESTCASE_DIR/$IMAGE_FILE.xz > $IMAGE_FILE
sudo losetup -f $IMAGE_FILE
DEV_FILE=`losetup -j $IMAGE_FILE | awk '{print $1}' | sed 's/://g'`

# Run fsck for repair
$FSCK_PROG $FSCK_OPTS $DEV_FILE
if [ "$?" -ne "1" ]; then
	echo ""
	echo "Failed to repair $TESTCASE_DIR"
	losetup -d $DEV_FILE
	exit
fi

# Run fsck again
$FSCK_PROG -n $DEV_FILE
if [ "$?" -ne "0" ]; then
	echo ""
	echo "Failed, corrupted $TESTCASE_DIR"
	losetup -d $DEV_FILE
	exit
fi

if [  -e "$TESTCASE_DIR/exfat.img.expected.xz" ]; then
	EXPECTED_FILE=$IMAGE_FILE.expected
	unxz -cfk "$TESTCASE_DIR/$EXPECTED_FILE.xz" > $EXPECTED_FILE
	diff <(xxd $IMAGE_FILE) <(xxd $EXPECTED_FILE)
	if [ "$?" -ne "0" ]; then
		echo ""
		echo "Failed $TESTCASE_DIR"
		losetup -d $DEV_FILE
		exit
	fi
fi

echo ""
echo "Passed $TESTCASE_DIR"

losetup -d $DEV_FILE
