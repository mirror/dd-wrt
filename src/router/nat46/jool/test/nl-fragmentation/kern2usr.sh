#!/bin/bash

# Quick 'n dirty kernel-to-userspace Netlink packet fragmentation test.
# First, go to /src/mod/common/nl/nl_core.c. In the jresponse_init() function,
# change
#
#	response->skb = genlmsg_new(GENLMSG_DEFAULT_SIZE, GFP_KERNEL);
#
# into
#
#	response->skb = genlmsg_new(32, GFP_KERNEL);
#
# Then recompile/reinstall jool using `make CFLAGS_MODULE="-DDEBUG -DJKMEMLEAK"`
# and run this script.
# All validations are visual for now; sorry.

# Arguments:
#
# $1: The names of the tests you want to run. (If you want to run all the tests,
#     omit this argument.)
#     Examples:
#     - ./kern2usr.sh siit-instance # Run only the "siit-instance" test.
#     - ./kern2usr.sh siit-instance,nat64-pool4 # Run the "siit-instance" and
#       the "nat64-pool4" tests.
#     - ./kern2usr.sh # Run everything


TESTS="$1"
CYAN="\\x1b[36m"
NC="\\x1b[0m" # No Color


function pause() {
	read -p "Press Enter to continue"
}

function print_table_check() {
	echo -e "${CYAN}Please check $1 entries 1-16:${NC}"
}

function dmesg_check() {
	sudo modprobe -r $1
	echo -e "${CYAN}Please check the kernel logs include at least one offset:${NC}"
	dmesg
}


sudo modprobe -r jool_siit
sudo modprobe -r jool

# ---------------------------------------------------------------

# Instance
if [[ -z "$TESTS" || "$TESTS" = *siit-instance* ]]; then
	clear
	sudo modprobe jool_siit
	for i in {1..16}; do
		sudo jool_siit instance add $i --iptables --pool6 64:ff9b::/96
	done
	sudo dmesg -C
	
	print_table_check "instance"
	sudo jool_siit instance display
	dmesg_check jool_siit

	pause
fi


# Globals
if [[ -z "$TESTS" || "$TESTS" = *siit-global* ]]; then
	clear
	sudo modprobe jool_siit
	sudo jool_siit instance add --iptables --pool6 64:ff9b::/96
	sudo dmesg -C
	
	echo -e "${CYAN}Please check this output makes reasonable sense:${NC}"
	sudo jool_siit global display
	dmesg_check jool_siit

	pause
fi


# EAMT
if [[ -z "$TESTS" || "$TESTS" = *siit-eamt* ]]; then
	clear
	sudo modprobe jool_siit
	sudo jool_siit instance add --iptables --pool6 64:ff9b::/96
	for i in {1..16}; do
		sudo jool_siit eamt add 192.0.2.$i 2001:db8::$i
	done
	sudo dmesg -C
	
	print_table_check "eamt"
	sudo jool_siit eamt display
	dmesg_check jool_siit

	pause
fi


# denylist4
if [[ -z "$TESTS" || "$TESTS" = *siit-denylist4* ]]; then
	clear
	sudo modprobe jool_siit
	sudo jool_siit instance add --iptables --pool6 64:ff9b::/96
	for i in {1..16}; do
		sudo jool_siit denylist4 add 203.0.113.$i
	done
	sudo dmesg -C
	
	print_table_check "denylist4"
	sudo jool_siit denylist4 display
	dmesg_check jool_siit

	pause
fi


# Instance
if [[ -z "$TESTS" || "$TESTS" = *nat64-instance* ]]; then
	clear
	sudo modprobe jool
	for i in {1..16}; do
		sudo jool instance add $i --iptables --pool6 64:ff9b::/96
	done
	sudo dmesg -C
	
	print_table_check "instance"
	sudo jool instance display
	dmesg_check jool

	pause
fi


# Globals
if [[ -z "$TESTS" || "$TESTS" = *nat64-global* ]]; then
	clear
	sudo modprobe jool
	sudo jool instance add --iptables --pool6 64:ff9b::/96
	sudo dmesg -C
	
	echo -e "${CYAN}Please check this output makes reasonable sense:${NC}"
	sudo jool global display
	dmesg_check jool

	pause
fi


# pool4
if [[ -z "$TESTS" || "$TESTS" = *nat64-pool4* ]]; then
	clear
	sudo modprobe jool
	sudo jool instance add --iptables --pool6 64:ff9b::/96
	for i in {1..16}; do
		sudo jool pool4 add --tcp 192.0.2.$i 10-20
	done
	sudo dmesg -C
	
	print_table_check "pool4"
	sudo jool pool4 display --tcp
	sudo jool pool4 display --udp
	sudo jool pool4 display --icmp
	dmesg_check jool

	pause
fi


# BIB
if [[ -z "$TESTS" || "$TESTS" = *nat64-bib* ]]; then
	clear
	sudo modprobe jool
	sudo jool instance add --iptables --pool6 64:ff9b::/96
	sudo jool pool4 add --tcp 192.0.2.1 1-16
	for i in {1..16}; do
		sudo jool bib add --tcp 192.0.2.1#$i 2001:db8::1#$i
	done
	sudo dmesg -C
	
	print_table_check "bib"
	sudo jool bib display --tcp --numeric
	sudo jool bib display --udp --numeric
	sudo jool bib display --icmp --numeric
	dmesg_check jool

	pause
fi


# Session
if [[ -z "$TESTS" || "$TESTS" = *nat64-session* ]]; then
	clear
	echo -e "${CYAN}Preparing test namespace. Please wait...${NC}"
	
	sudo ip netns add joolns
	sudo ip link add name to_jool type veth peer name to_world
	sudo ip link set up dev to_jool
	sudo ip link set dev to_world netns joolns
	sudo ip netns exec joolns ip link set up dev to_world
	
	for i in {1..16}; do
		sudo ip addr add 2001:db8::1:$i/96 dev to_jool
	done
	sudo ip addr add 192.0.2.8/24 dev to_jool
	sudo ip netns exec joolns ip addr add 2001:db8::1/96 dev to_world
	sudo ip netns exec joolns ip addr add 192.0.2.1/24 dev to_world
	sudo ip route add 64:ff9b::/96 via 2001:db8::1
	sleep 3
	
	sudo modprobe jool
	sudo ip netns exec joolns jool instance add --netfilter --pool6 64:ff9b::/96
	
	# Wait until the namespace is traversable.
	ping6 2001:db8::1 -c1 -W60
	ping 192.0.2.1 -c1 -W60
	
	# Create the sessions.
	for i in {1..16}; do
		ping6 64:ff9b::192.0.2.8 -I 2001:db8::1:$i -c1 > /dev/null
	done
	
	sudo dmesg -C
	
	print_table_check "session"
	sudo ip netns exec joolns jool session display --tcp --numeric --csv
	sudo ip netns exec joolns jool session display --udp --numeric --csv
	sudo ip netns exec joolns jool session display --icmp --numeric --csv
	dmesg_check jool
	
	sudo ip netns del joolns
	echo -e "${CYAN}Test namespace deleted.${NC}"
	
	pause
fi

echo "Done."