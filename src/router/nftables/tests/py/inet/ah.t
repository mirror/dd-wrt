:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

- ah nexthdr esp;ok
- ah nexthdr ah;ok
- ah nexthdr comp;ok
- ah nexthdr udp;ok
- ah nexthdr udplite;ok
- ah nexthdr tcp;ok
- ah nexthdr dccp;ok
- ah nexthdr sctp;ok

- ah nexthdr { esp, ah, comp, udp, udplite, tcp, dccp, sctp};ok;ah nexthdr { 6, 132, 50, 17, 136, 33, 51, 108}
- ah nexthdr != { esp, ah, comp, udp, udplite, tcp, dccp, sctp};ok

ah hdrlength 11-23;ok
ah hdrlength != 11-23;ok
ah hdrlength {11, 23, 44 };ok
ah hdrlength != {11, 23, 44 };ok

ah reserved 22;ok
ah reserved != 233;ok
ah reserved 33-45;ok
ah reserved != 33-45;ok
ah reserved {23, 100};ok
ah reserved != {23, 100};ok

ah spi 111;ok
ah spi != 111;ok
ah spi 111-222;ok
ah spi != 111-222;ok
ah spi {111, 122};ok
ah spi != {111, 122};ok

# sequence
ah sequence 123;ok
ah sequence != 123;ok
ah sequence {23, 25, 33};ok
ah sequence != {23, 25, 33};ok
ah sequence 23-33;ok
ah sequence != 23-33;ok
