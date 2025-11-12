:ipsec-forw;type filter hook forward priority 0

*ip;ipsec-ip4;ipsec-forw
*ip6;ipsec-ip6;ipsec-forw
*inet;ipsec-inet;ipsec-forw

ipsec in reqid 1;ok
ipsec in spnum 0 reqid 1;ok;ipsec in reqid 1

ipsec out reqid 0xffffffff;ok;ipsec out reqid 4294967295
ipsec out spnum 0x100000000;fail

ipsec i reqid 1;fail

ipsec out spi 1-561;ok

ipsec in spnum 2 ip saddr { 1.2.3.4, 10.6.0.0/16 };ok
ipsec in ip6 daddr dead::beef;ok
ipsec out ip6 saddr dead::feed;ok

ipsec in spnum 256 reqid 1;fail

counter ipsec out ip daddr 192.168.1.2;ok
