#!/bin/sh

# Assumes the network namespaces have already been created.

ip netns exec joolns `dirname $0`/setup-jool.sh
ip netns exec client6ns `dirname $0`/setup-n6.sh
ip netns exec client4ns `dirname $0`/setup-n4.sh

insmod `dirname $0`/../../mod/graybox.ko
