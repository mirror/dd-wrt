#!/bin/sh

# Reverts ./setup.sh.
# Does not destroy the namespaces.

ip netns exec joolns `dirname $0`/end-jool.sh
ip netns exec client6ns `dirname $0`/end-n6.sh
ip netns exec client4ns `dirname $0`/end-n4.sh

rmmod graybox
