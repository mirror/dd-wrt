:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

vxlan vni 10;fail
udp dport 4789 vxlan vni 10;ok
udp dport 4789 vxlan ip saddr 10.141.11.2;ok
udp dport 4789 vxlan ip saddr 10.141.11.0/24;ok
udp dport 4789 vxlan ip protocol 1;ok
udp dport 4789 vxlan udp sport 8888;ok
udp dport 4789 vxlan icmp type echo-reply;ok
udp dport 4789 vxlan ether saddr 62:87:4d:d6:19:05;ok
udp dport 4789 vxlan vlan id 10;ok
udp dport 4789 vxlan ip dscp 0x02;ok
udp dport 4789 vxlan ip saddr . vxlan ip daddr { 1.2.3.4 . 4.3.2.1 };ok

udp dport 4789 vxlan ip saddr set 1.2.3.4;fail
