:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*inet;test-inet;input
*bridge;test-inet;input
*netdev;test-netdev;ingress,egress

!set1 type ipv4_addr timeout 60s;ok
?set1 192.168.3.4 timeout 30s, 10.2.1.1;ok

!set2 type ipv6_addr timeout 23d23h59m59s;ok
?set2 dead::beef timeout 5s;ok

ip saddr @set1 drop;ok
ip saddr != @set2 drop;fail

ip6 daddr != @set2 accept;ok
ip6 daddr @set1 drop;fail

!set3 type ipv4_addr . ipv4_addr . inet_service flags interval;ok
?set3 10.0.0.0/8 . 192.168.1.3-192.168.1.9 . 1024-65535;ok

ip saddr . ip daddr . tcp dport @set3 accept;ok
ip daddr . tcp dport { 10.0.0.0/8 . 10-23, 192.168.1.1-192.168.3.8 . 80-443 } accept;ok
