:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

!w type ipv4_addr;ok
!x type inet_proto;ok
!y type inet_service;ok
!z type time;ok

!set1 type ipv4_addr;ok
?set1 192.168.3.4;ok

?set1 192.168.3.4;ok
?set1 192.168.3.5, 192.168.3.6;ok
?set1 192.168.3.5, 192.168.3.6;ok
?set1 192.168.3.8, 192.168.3.9;ok
?set1 192.168.3.10, 192.168.3.11;ok
?set1 1234:1234:1234:1234:1234:1234:1234:1234;fail
?set2 192.168.3.4;fail

!set2 type ipv4_addr;ok
?set2 192.168.3.4;ok
?set2 192.168.3.5, 192.168.3.6;ok
?set2 192.168.3.5, 192.168.3.6;ok
?set2 192.168.3.8, 192.168.3.9;ok
?set2 192.168.3.10, 192.168.3.11;ok

ip saddr @set1 drop;ok
ip saddr != @set1 drop;ok
ip saddr @set2 drop;ok
ip saddr != @set2 drop;ok
ip saddr @set33 drop;fail
ip saddr != @set33 drop;fail

!set3 type ipv4_addr flags interval;ok
?set3 192.168.0.0/16;ok
?set3 172.16.0.0/12;ok
?set3 10.0.0.0/8;ok

!set4 type ipv4_addr flags interval;ok
?set4 192.168.1.0/24;ok
?set4 192.168.0.0/24;ok
?set4 192.168.2.0/24;ok
?set4 192.168.1.1;fail
?set4 192.168.3.0/24;ok

!set5 type ipv4_addr . ipv4_addr;ok
ip saddr . ip daddr @set5 drop;ok
add @set5 { ip saddr . ip daddr };ok

!map1 type ipv4_addr . ipv4_addr : mark;ok
add @map1 { ip saddr . ip daddr : meta mark };ok

# test nested anonymous sets
ip saddr { { 1.1.1.0, 3.3.3.0 }, 2.2.2.0 };ok;ip saddr { 1.1.1.0, 2.2.2.0, 3.3.3.0 }
ip saddr { { 1.1.1.0/24, 3.3.3.0/24 }, 2.2.2.0/24 };ok;ip saddr { 1.1.1.0/24, 2.2.2.0/24, 3.3.3.0/24 }

!set6 type ipv4_addr;ok
?set6 192.168.3.5, *;ok
ip saddr @set6 drop;ok

ip saddr vmap { 1.1.1.1 : drop, * : accept };ok
meta mark set ip saddr map { 1.1.1.1 : 0x00000001, * : 0x00000002 };ok

!map2 type ipv4_addr . ipv4_addr . inet_service : ipv4_addr . inet_service;ok
add @map2 { ip saddr . ip daddr . th dport : 10.0.0.1 . 80 };ok
