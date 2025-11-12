:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

geneve vni 10;fail
udp dport 6081 geneve vni 10;ok
udp dport 6081 geneve ip saddr 10.141.11.2;ok
udp dport 6081 geneve ip saddr 10.141.11.0/24;ok
udp dport 6081 geneve ip protocol 1;ok
udp dport 6081 geneve udp sport 8888;ok
udp dport 6081 geneve icmp type echo-reply;ok
udp dport 6081 geneve ether saddr 62:87:4d:d6:19:05;ok
udp dport 6081 geneve vlan id 10;ok
udp dport 6081 geneve ip dscp 0x02;ok
udp dport 6081 geneve ip saddr . geneve ip daddr { 1.2.3.4 . 4.3.2.1 };ok

udp dport 6081 geneve ip saddr set 1.2.3.4;fail
