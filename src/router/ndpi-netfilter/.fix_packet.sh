#!/bin/bash

cd src/lib/protocols
find -type f -name \*.c | while read A; do
	grep -q 'packet = &ndpi_struct->packet' $A || continue
	echo $A
	sed -i -e 's/packet = &ndpi_struct->packet/packet = ndpi_get_packet_struct(ndpi_struct)/' $A
done
