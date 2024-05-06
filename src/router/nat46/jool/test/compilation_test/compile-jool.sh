#!/bin/bash

# Compiles a kernel module on every linux instance found in the $2 directory.
#
# Arguments:
# - $1: Directory where the kernel module's makefile is.
#       Optional. Defaults to <here>/../../mod.
# - $2: Directory where you precompiled a bunch of kernels. Must be absolute.
#       Same argument as prepare-kernel.sh's $3.
#       Optional. Defaults to `pwd`. (ie. caller's directory.)
#       The script will assume that every subdirectory here is a kernel.
# - $3: Space-separated list of kernels (in $2) you want to compile on.
#       Optional; defaults to all the kernels in $2.
#
# Example: ./compile-jool.sh ~/Jool/mod $(pwd) "v4.7 v4.8 v4.9"
#
# If you get errors, see $2/results.log.


function die() {
	echo $1
	exit 1
}


RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

ORIGINAL_DIR=$(pwd)

# Parse $1
MODULE_DIR=`dirname $0`/../../mod
if [ -n "${1+set}" ]; then
	MODULE_DIR=$1
fi
cd $MODULE_DIR || die "Cannot cd to '$MODULE_DIR'. Aborting."

# Parse $2
KERNELS_DIR=$ORIGINAL_DIR
if [ -n "${2+set}" ]; then
	KERNELS_DIR=$2
fi
[[ ! -d $KERNELS_DIR ]] && die "'$KERNELS_DIR' is not a directory. Aborting."

# Parse $3
KERNELS=$(ls $KERNELS_DIR)
if [ -n "${3+set}" ]; then
	KERNELS=$3
fi


# Initialize the log
LOGFILE=$KERNELS_DIR/results.log
rm -f $LOGFILE
echo "(See $LOGFILE for verbose output.)"


# Ok, compile
for i in $KERNELS; do
	if [[ ! -d $KERNELS_DIR/$i ]]; then
		echo "Skipping '$i'."
		continue
	fi

	echo "Compiling on kernel '$i'."
	echo "-----------------------" >> $LOGFILE
	echo "------ Kernel $i ------" >> $LOGFILE
	echo "-----------------------" >> $LOGFILE
	make KERNEL_DIR=$KERNELS_DIR/$i >> $LOGFILE 2>&1
	if [[ $? -eq 0 ]]; then
		echo -e "${GREEN}Success.${NC}"
	else
		echo -e "${RED}Failure.${NC}"
	fi
done

