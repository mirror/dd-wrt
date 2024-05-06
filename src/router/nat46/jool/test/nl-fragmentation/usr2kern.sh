#!/bin/bash

# Quick 'n dirty userspace-to-kernel Netlink packet fragmentation test.
# First, go to /src/usr/nl/core.c. In the joolnl_alloc_msg() function,
# change
#
#	msg = nlmsg_alloc();
#
# into
#
#	msg = nlmsg_alloc_size(256);
#
# Then recompile/reinstall jool using `make CFLAGS_MODULE="-DDEBUG -DJKMEMLEAK"`
# and run this script.
# All validations are visual for now; sorry.

# Arguments:
#
# $1: The names of the tests you want to run. (If you want to run all the tests,
#     omit this argument.)
#     Examples:
#     - ./usr2kern.sh eamt # Run only the "eamt" test.
#     - ./usr2kern.sh eamt,bib # Run the "eamt" and the "bib" tests.
#     - ./usr2kern.sh # Run everything


TESTS="$1"
CYAN="\\x1b[36m"
NC="\\x1b[0m" # No Color

sudo modprobe -r jool_siit
sudo modprobe -r jool


function pause() {
	read -p "Press Enter to continue"
}

function print_table_check() {
	echo -e "${CYAN}Please check $1 entries 1-16:${NC}"
}

function dmesg_check() {
	sudo modprobe -r $1
	echo -e "${CYAN}Please check the kernel receives entries in separate requests:${NC}"
	dmesg
}


# EAMT
if [[ -z "$TESTS" || "$TESTS" = *eamt* ]]; then
	clear
	sudo modprobe jool_siit
	sudo dmesg -C

	echo -e "\t{
		\"framework\": \"iptables\",
		\"instance\": \"default\",
		\"eamt\": [" > tmp.txt
	for i in {1..16}; do
		echo -e "\t\t{
			\"ipv6 prefix\": \"2001:db8:1::$i\",
			\"ipv4 prefix\": \"192.0.2.$i\"
		}" >> tmp.txt
		if [ $i -ne 16 ]; then
			echo "," >> tmp.txt
		fi
	done
	echo -e "\t]\n}" >> tmp.txt

	sudo jool_siit file handle tmp.txt
	print_table_check
	sudo jool_siit eamt display
	dmesg_check jool_siit

	rm tmp.txt
	pause
fi

# Denylist4
if [[ -z "$TESTS" || "$TESTS" = *denylist4* ]]; then
	clear
	sudo modprobe jool_siit
	sudo dmesg -C

	echo -e "\t{
		\"framework\": \"iptables\",
		\"instance\": \"default\",
		\"denylist4\": [" > tmp.txt
	for i in {1..16}; do
		echo "\"198.51.100.$i\"" >> tmp.txt
		if [ $i -ne 16 ]; then
			echo "," >> tmp.txt
		fi
	done
	echo -e "\t]\n}" >> tmp.txt

	sudo jool_siit file handle tmp.txt
	print_table_check
	sudo jool_siit denylist4 display
	dmesg_check jool_siit

	rm tmp.txt
	pause
fi

# pool4
if [[ -z "$TESTS" || "$TESTS" = *pool4* ]]; then
	clear
	sudo modprobe jool
	sudo dmesg -C

	echo -e "\t{
		\"framework\": \"iptables\",
		\"instance\": \"default\",
		\"global\": { \"pool6\": \"64::/96\" },
		\"pool4\": [" > tmp.txt
	for i in {1..16}; do
		echo "{
			\"prefix\": \"192.0.2.$i/32\",
			\"protocol\": \"TCP\"
		}" >> tmp.txt
		if [ $i -ne 16 ]; then
			echo "," >> tmp.txt
		fi
	done
	echo -e "\t]\n}" >> tmp.txt

	sudo jool file handle tmp.txt
	print_table_check
	sudo jool pool4 display
	dmesg_check jool

	rm tmp.txt
	pause
fi

# BIB
if [[ -z "$TESTS" || "$TESTS" = *bib* ]]; then
	clear
	sudo modprobe jool
	sudo dmesg -C

	echo -e "\t{
		\"framework\": \"iptables\",
		\"instance\": \"default\",
		\"global\": { \"pool6\": \"64::/96\" },
		\"pool4\": [{
			\"prefix\": \"192.0.2.1/32\",
			\"port range\": \"1-16\",
			\"protocol\": \"TCP\"
		}],
		\"bib\": [" > tmp.txt
	for i in {1..16}; do
		echo "{
			\"protocol\": \"TCP\",
			\"ipv6 address\": \"2001:db8::1#$i\",
			\"ipv4 address\": \"192.0.2.1#$i\"
		}" >> tmp.txt
		if [ $i -ne 16 ]; then
			echo "," >> tmp.txt
		fi
	done
	echo -e "\t]\n}" >> tmp.txt

	sudo jool file handle tmp.txt
	print_table_check
	sudo jool bib display --tcp --numeric
	dmesg_check jool

	rm tmp.txt
	pause
fi

echo "Done."
