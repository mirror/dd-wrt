:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

gre version 0;ok
gre ip saddr 10.141.11.2;ok
gre ip saddr 10.141.11.0/24;ok
gre ip protocol 1;ok
gre udp sport 8888;ok
gre icmp type echo-reply;ok
gre ether saddr 62:87:4d:d6:19:05;fail
gre vlan id 10;fail
gre ip dscp 0x02;ok
gre ip saddr . gre ip daddr { 1.2.3.4 . 4.3.2.1 };ok

gre ip saddr set 1.2.3.4;fail
