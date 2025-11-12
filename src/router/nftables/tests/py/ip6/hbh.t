:filter-input;type filter hook input priority 0

*ip6;test-ip6;filter-input
*inet;test-inet;filter-input

hbh hdrlength 22;ok
hbh hdrlength != 233;ok
hbh hdrlength 33-45;ok
hbh hdrlength != 33-45;ok
hbh hdrlength {33, 55, 67, 88};ok
hbh hdrlength != {33, 55, 67, 88};ok

hbh nexthdr {esp, ah, comp, udp, udplite, tcp, dccp, sctp, icmpv6};ok;hbh nexthdr { 58, 136, 51, 50, 6, 17, 132, 33, 108}
hbh nexthdr != {esp, ah, comp, udp, udplite, tcp, dccp, sctp, icmpv6};ok;hbh nexthdr != { 58, 136, 51, 50, 6, 17, 132, 33, 108}
hbh nexthdr 22;ok
hbh nexthdr != 233;ok
hbh nexthdr 33-45;ok
hbh nexthdr != 33-45;ok
hbh nexthdr {33, 55, 67, 88};ok
hbh nexthdr != {33, 55, 67, 88};ok
hbh nexthdr ip;ok;hbh nexthdr 0
hbh nexthdr != ip;ok;hbh nexthdr != 0
