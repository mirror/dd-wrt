#!/bin/bash
i="0";
while [ $i -lt $2 ]
do
	i=`expr $i + 1`
	arp -d $1
	ping -c 1 $1
done
