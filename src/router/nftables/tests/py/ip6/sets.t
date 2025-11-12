:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

!w type ipv6_addr;ok
!x type inet_proto;ok
!y type inet_service;ok
!z type time;ok

?set2 192.168.3.4;fail
!set2 type ipv6_addr;ok
?set2 1234:1234::1234:1234:1234:1234:1234;ok
?set2 1234:1234::1234:1234:1234:1234:1234;ok
?set2 1234::1234:1234:1234;ok
?set2 1234:1234:1234:1234:1234::1234:1234, 1234:1234::123;ok
?set2 192.168.3.8, 192.168.3.9;fail
?set2 1234:1234::1234:1234:1234:1234;ok
?set2 1234:1234::1234:1234:1234:1234;ok
?set2 1234:1234:1234::1234;ok

ip6 saddr @set2 drop;ok
ip6 saddr != @set2 drop;ok
ip6 saddr @set33 drop;fail
ip6 saddr != @set33 drop;fail

!set3 type ipv6_addr flags interval;ok
?set3 1234:1234:1234:1234::/64;ok
?set3 1324:1234:1234:1235::/64;ok
?set3 1324:1234:1234:1233::/64;ok
?set3 1234:1234:1234:1234:1234:1234:/96;fail
?set3 1324:1234:1234:1236::/64;ok

!set4 type ipv6_addr flags interval;ok
?set4 1234:1234:1234:1234::/64,4321:1234:1234:1234::/64;ok
?set4 4321:1234:1234:1234:1234:1234::/96;fail

!set5 type ipv6_addr . ipv6_addr;ok
ip6 saddr . ip6 daddr @set5 drop;ok
add @set5 { ip6 saddr . ip6 daddr };ok

!map1 type ipv6_addr . ipv6_addr : mark;ok
add @map1 { ip6 saddr . ip6 daddr : meta mark };ok

delete @set5 { ip6 saddr . ip6 daddr };ok

!map2 type ipv6_addr . ipv6_addr . inet_service : ipv6_addr . inet_service;ok
add @map2 { ip6 saddr . ip6 daddr . th dport : 1234::1 . 80 };ok