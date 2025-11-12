:input;type filter hook input priority 0

*ip6;test-ip6;input
*inet;test-inet;input

rt nexthdr 1;ok
rt nexthdr != 1;ok
rt nexthdr {udplite, ipcomp, udp, ah, sctp, esp, dccp, tcp, ipv6-icmp};ok;rt nexthdr { 33, 136, 50, 132, 51, 17, 108, 6, 58}
rt nexthdr != {udplite, ipcomp, udp, ah, sctp, esp, dccp, tcp, ipv6-icmp};ok;rt nexthdr != { 33, 136, 50, 132, 51, 17, 108, 6, 58}
rt nexthdr icmp;ok;rt nexthdr 1
rt nexthdr != icmp;ok;rt nexthdr != 1
rt nexthdr 22;ok
rt nexthdr != 233;ok
rt nexthdr 33-45;ok
rt nexthdr != 33-45;ok
rt nexthdr { 33, 55, 67, 88};ok
rt nexthdr != { 33, 55, 67, 88};ok

rt hdrlength 22;ok
rt hdrlength != 233;ok
rt hdrlength 33-45;ok
rt hdrlength != 33-45;ok
rt hdrlength { 33, 55, 67, 88};ok
rt hdrlength != { 33, 55, 67, 88};ok

rt type 22;ok
rt type != 233;ok
rt type 33-45;ok
rt type != 33-45;ok
rt type { 33, 55, 67, 88};ok
rt type != { 33, 55, 67, 88};ok

rt seg-left 22;ok
rt seg-left != 233;ok
rt seg-left 33-45;ok
rt seg-left != 33-45;ok
rt seg-left { 33, 55, 67, 88};ok
rt seg-left != { 33, 55, 67, 88};ok
