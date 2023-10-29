#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2021-2023 OpenVPN, Inc.
#
#  Author:	Antonio Quartulli <antonio@openvpn.net>

#set -x
set -e

UDP_PEERS_FILE=${UDP_PEERS_FILE:-udp_peers.txt}
TCP_PEERS_FILE=${TCP_PEERS_FILE:-tcp_peers.txt}
OVPN_CLI=${OVPN_CLI:-./ovpn-cli}
ALG=${ALG:-aes}

function create_ns() {
	ip netns add peer$1
}

function setup_ns() {
	if [ $1 -eq 0 ]; then
		ip -n peer0 link add br0 type bridge
		ip -n peer0 link set br0 up

		for p in $(seq 1 $NUM_PEERS); do
			ip link add veth${p} netns peer0 type veth peer name veth${p} netns peer${p}
			ip -n peer0 addr add 10.10.0.${p}/16 dev br0
			ip -n peer0 link set veth${p} master br0			
			ip -n peer0 link set veth${p} up

			ip -n peer${p} addr add 10.10.${p}.2/16 dev veth${p}
			ip -n peer${p} link set veth${p} up
		done
	fi

	ip -n peer$1 link add tun0 type ovpn-dco
	ip -n peer$1 addr add $2 dev tun0
	ip -n peer$1 link set tun0 up
}

function add_peer() {
	if [ $tcp -eq 0 ]; then
		if [ $1 -eq 0 ]; then
			ip netns exec peer0 $OVPN_CLI tun0 new_multi_peer 1 $UDP_PEERS_FILE

			for p in $(seq 1 $NUM_PEERS); do
			#	ip netns exec peer0 $OVPN_CLI tun0 new_peer ${p} ${p} 10.10.${p}.2 1 5.5.5.$((${p} + 1))
				ip netns exec peer0 $OVPN_CLI tun0 new_key ${p} $ALG 0 data64.key
			done
		else
			ip netns exec peer${1} $OVPN_CLI tun0 new_peer 1 ${1} 10.10.0.${1} 1 5.5.5.1
			ip netns exec peer${1} $OVPN_CLI tun0 new_key ${1} $ALG 1 data64.key
		fi
	else
		if [ $1 -eq 0 ]; then
			(ip netns exec peer$1 $OVPN_CLI tun0 listen 1 $TCP_PEERS_FILE && {
				for p in $(seq 1 $NUM_PEERS); do
					ip netns exec peer0 $OVPN_CLI tun0 new_key ${p} $ALG 0 data64.key
				done
			}) &
			sleep 5
		else
			ip netns exec peer${1} $OVPN_CLI tun0 connect ${1} 10.10.0.${1} 1 5.5.5.1
			ip netns exec peer${1} $OVPN_CLI tun0 new_key ${1} $ALG 1 data64.key
		fi
	fi
}


# clean up
for p in $(seq 1 10); do
	ip -n peer0 link del veth${p} 2>/dev/null || true
done
for p in $(seq 0 10); do
	ip -n peer${p} link del tun0 2>/dev/null || true
	ip netns del peer${p} 2>/dev/null || true
done

tcp=0
if [ "$1" == "-t" ]; then
	shift
	tcp=1
	NUM_PEERS=${NUM_PEERS:-$(wc -l $TCP_PEERS_FILE | awk '{print $1}')}
else
	NUM_PEERS=${NUM_PEERS:-$(wc -l $UDP_PEERS_FILE | awk '{print $1}')}
fi

for p in $(seq 0 $NUM_PEERS); do
	create_ns ${p}
done

for p in $(seq 0 $NUM_PEERS); do
	setup_ns ${p} 5.5.5.$((${p} + 1))/24
done

for p in $(seq 0 $NUM_PEERS); do
	add_peer ${p}
done

for p in $(seq 1 $NUM_PEERS); do
	ip netns exec peer${p} ping -qfc 2000 -w 5 5.5.5.1
done
