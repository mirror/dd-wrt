#!/bin/sh

# What is this for?
# The test script is normally run as part of another script, which
# likely just configured the interfaces, and so the neighbors might
# still be discovering each other.
# If we run the tests while the neighbors are stil exchanging
# handshakes or whatever, the test packets can disappear.
# These pings should sleep us until we know a packet can do a full
# round-trip.

echo "Waiting for the network to be ready..."

# It seems there are some handshakes that get confused if we ping too early.
# According to my tests, they take about 2 seconds.
# If we ping during these handshakes, sometimes it seems to run into some sort
# of temporary deadlock, which can last up to like 10 seconds.
# We avoid this deadlock by wasting 2 seconds and a bit of extra.
sleep 3

for i in $(seq 10); do
	ip netns exec client6ns ping $1 -c 1 >/dev/null 2>&1
	if [ $? -eq 0 ]; then
		echo "Ready."
		exit 0
	fi
	sleep 1
done

echo "It appears the network hasn't been configured."
echo "Quitting."
exit 1

