#!/bin/bash
# used for tests
# juste send one arpa request

for i in $(arp -na | sed -e "s/.*(//; s/).*//")
do
	arp -d $i
done
ping -c 1 $1
