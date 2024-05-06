#!/bin/sh

echo "Preparing the Graybox network namespaces..."

if ip netns list | grep 'joolns' > /dev/null; then
	echo "The namespaces seem to already exist. Skipping step."
	exit 0
fi

ip netns add joolns
ip netns add client6ns
ip netns add client4ns

ip link add name to_client_v6 type veth peer name to_jool_v6
ip link set dev to_client_v6 netns joolns
ip link set dev to_jool_v6 netns client6ns

ip link add name to_client_v4 type veth peer name to_jool_v4
ip link set dev to_client_v4 netns joolns
ip link set dev to_jool_v4 netns client4ns

ip netns exec joolns ip link set up dev to_client_v6
ip netns exec joolns ip link set up dev to_client_v4
ip netns exec client6ns ip link set up dev to_jool_v6
ip netns exec client4ns ip link set up dev to_jool_v4
