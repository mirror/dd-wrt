#!/bin/sh

./configure --disable-ipv6 --disable-ripngd --disable-ospfd \
            --disable-ospf6d --disable-bgpd --disable-bgpd-announce \
	    --prefix=/usr/local/zebra

    
