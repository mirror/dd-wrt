#!/bin/bash

# Compiles the minimum Linux kernel necessary for module cross-compilation.
#
# Arguments:
# - $1: Path to the Linux repository you cloned.
#       The script needs write permissions on this repo, as it be cleant and
#       HEAD will be bounced around.
# - $2: Space-separated list of commits you want to generate precompiled Linuxes
#       out of.
#       These can be any git-compatible commit labels such as commit hashes
#       ("afd2ff9b7e1b367"), tag names ("v3.13") and branches ("main").
# - $3: Directory where the Linux instances will be stored. Must be absolute.
#       Eg. kernel v3.13 will be placed in $3/v3.13.
#       Optional. Will default to `pwd`. (ie. caller's directory.)
#
# Example:
# ./prepare-kernels.sh ~/Downloads/linux-git/linux "v4.7 v4.8 v4.9" $(pwd)


function die() {
	echo $1
	exit 1
}


if [ -z "${1+set}" ]; then
	die 'I need arguments. Please run "head -n18 prepare-kernels.sh".'
fi
if [ -z "${2+set}" ]; then
	die 'I need a 2nd argument. Please run "head -n18 prepare-kernels.sh".'
fi

LINUX_DIR=$1
VERSIONS=$2
OUT_DIR=$(pwd)
if [ -n "${3+set}" ]; then
	OUT_DIR=$3
fi

echo "Preparing '$LINUX_DIR'."
cd $LINUX_DIR || die "Cannot cd to '$LINUX_DIR'. Aborting."
rm -f .config || die "Cannot rm on '$LINUX_DIR'. Aborting."
git clean -xdf > /dev/null || die "Cannot clean '$LINUX_DIR'. Aborting."
git fetch || die "Cannot update the Linux repository. Aborting."

echo "Preparing '$OUT_DIR'."
mkdir -p $OUT_DIR || die "Could not create directory '$OUT_DIR'. Aborting."


for i in $VERSIONS; do
	echo "------------------------"

	mkdir -p $OUT_DIR/$i
	if [[ $? -ne 0 ]]; then
		echo "Could not create directory '$OUT_DIR/$i'. Skipping."
		continue
	fi

	echo "Checking out $i."
	git checkout $i
	RESULT=$?
	if [[ $RESULT -ne 0 ]]; then
		echo "git checkout failed. Skipping kernel."
		continue
	fi

	echo "Configuring $i."
	yes "" | make oldconfig > /dev/null 2>&1
	RESULT=$?
	if [[ $RESULT -ne 0 ]]; then
		echo "Kernel $i config spew error code $RESULT. Skipping."
		continue
	fi

	echo "Preparing $i."
	make modules_prepare > /dev/null 2>&1
	RESULT=$?
	if [[ $RESULT -ne 0 ]]; then
		echo "Kernel $i prepare spew error code $RESULT. Skipping."
		continue
	fi

	echo "Copying kernel to '$OUT_DIR/$i'."
	# TODO Wait, what the hell? do we really need all of it?
	cp -r * $OUT_DIR/$i

	echo "Cleaning $i."
	make clean > /dev/null 2>&1
	rm .config
	git clean -xdf > /dev/null
done


echo "Done."

