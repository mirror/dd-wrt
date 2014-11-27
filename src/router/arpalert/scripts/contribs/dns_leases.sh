#!/bin/bash

LEASES=/usr/local/arpalert/var/lib/arpalert/arpalert.leases
for i in `cat $LEASES | awk '{ print $2 }'`
do
        echo -en "$i\t\t"
        if [ `nslookup -sil $i | grep name | awk '{ print $4 }'` ]
        then
                echo -en "`nslookup -sil $i | grep name | awk '{ print $4 }'`\t\t"
        else
                echo -en "DNS Unassigned\t\t\t"
        fi
        echo -en "`cat $LEASES | grep -w $i | awk '{ print $1 }'`\n"
done
