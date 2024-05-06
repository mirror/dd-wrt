#!/bin/sh

# Reverts the stuff ./namespace-create.sh did.

echo "Destroying the Graybox network namespaces..."

ip netns exec joolns ip link del to_client_v6
ip netns exec joolns ip link del to_client_v4

ip netns del joolns
ip netns del client6ns
ip netns del client4ns