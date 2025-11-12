:output;type filter hook output priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip6;test-ip6;output
*inet;test-inet;output
*netdev;test-netdev;ingress,egress

frag nexthdr tcp;ok;frag nexthdr 6
frag nexthdr != icmp;ok;frag nexthdr != 1
frag nexthdr {esp, ah, comp, udp, udplite, tcp, dccp, sctp};ok;frag nexthdr { 51, 136, 132, 6, 108, 50, 17, 33}
frag nexthdr != {esp, ah, comp, udp, udplite, tcp, dccp, sctp};ok;frag nexthdr != { 51, 136, 132, 6, 108, 50, 17, 33}
frag nexthdr esp;ok;frag nexthdr 50
frag nexthdr ah;ok;frag nexthdr 51

frag reserved 22;ok
frag reserved != 233;ok
frag reserved 33-45;ok
frag reserved != 33-45;ok
frag reserved { 33, 55, 67, 88};ok
frag reserved != { 33, 55, 67, 88};ok

frag frag-off 22;ok
frag frag-off != 233;ok
frag frag-off 33-45;ok
frag frag-off != 33-45;ok
frag frag-off { 33, 55, 67, 88};ok
frag frag-off != { 33, 55, 67, 88};ok

frag reserved2 1;ok
frag more-fragments 0;ok
frag more-fragments 1;ok

frag id 1;ok
frag id 22;ok
frag id != 33;ok
frag id 33-45;ok
frag id != 33-45;ok
frag id { 33, 55, 67, 88};ok
frag id != { 33, 55, 67, 88};ok
