:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

gretap ip saddr 10.141.11.2;ok
gretap ip saddr 10.141.11.0/24;ok
gretap ip protocol 1;ok
gretap udp sport 8888;ok
gretap icmp type echo-reply;ok
gretap ether saddr 62:87:4d:d6:19:05;ok
gretap vlan id 10;ok
gretap ip dscp 0x02;ok
gretap ip saddr . gretap ip daddr { 1.2.3.4 . 4.3.2.1 };ok

gretap ip saddr set 1.2.3.4;fail
