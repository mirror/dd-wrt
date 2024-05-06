#!/bin/bash

echo "Note: This will take up lots of CPU."
echo "If this freezes, please wait a few minutes; it should come back."

function test() {
	echo "Testing $1 addresses with $2 ports each."
	for i in {1..16}; do
		echo "Test $i"
		sudo insmod pool4-iterations.ko RANGE_COUNT=$1 TADDRS_PER_RANGE=$2 MAX_ITERATIONS=$3
		sudo rmmod pool4-iterations
		sudo dmesg -ct >> results-$1-$2-$3.txt
	done
}

rm -f results*
sudo dmesg -C

test 1 512 0
test 1 1024 0
test 1 2048 0
test 1 4096 0
test 1 8192 0
test 1 16384 0
test 1 32768 0
test 1 65536 0
test 2 65536 0
test 3 65536 0
test 4 65536 0
test 5 65536 0
test 6 65536 0
test 7 65536 0
test 8 65536 0
test 16 65536 0
test 32 65536 0

echo "Test results written to result*.txt files."
