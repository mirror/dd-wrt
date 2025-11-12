:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*inet;test-inet;input
*bridge;test-bridge;input
*netdev;test-netdev;ingress,egress

- ip version 2;ok

# bug ip hdrlength
- ip hdrlength 10;ok
- ip hdrlength != 5;ok
- ip hdrlength 5-8;ok
- ip hdrlength != 3-13;ok
- ip hdrlength {3, 5, 6, 8};ok
- ip hdrlength != {3, 5, 7, 8};ok
- ip hdrlength { 3-5};ok
- ip hdrlength != { 3-59};ok
# ip hdrlength 12
# <cmdline>:1:1-38: Error: Could not process rule: Invalid argument
# add rule ip test input ip hdrlength 12
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# <cmdline>:1:37-38: Error: Value 22 exceeds valid range 0-15
# add rule ip test input ip hdrlength 22

ip dscp cs1;ok
ip dscp != cs1;ok
ip dscp 0x38;ok;ip dscp cs7
ip dscp != 0x20;ok;ip dscp != cs4
ip dscp {cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7, af11, af12, af13, af21, af22, af23, af31, af32, af33, af41, af42, af43, ef};ok
- ip dscp {0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x00, 0x0a, 0x0c, 0x0e, 0x12, 0x14, 0x16, 0x1a, 0x1c, 0x1e, 0x22, 0x24, 0x26, 0x2e};ok
ip dscp != {cs0, cs3};ok
ip dscp vmap { cs1 : continue , cs4 : accept } counter;ok

ip length 232;ok
ip length != 233;ok
ip length 333-435;ok
ip length != 333-453;ok
ip length { 333, 553, 673, 838};ok
ip length != { 333, 553, 673, 838};ok

ip id 22;ok
ip id != 233;ok
ip id 33-45;ok
ip id != 33-45;ok
ip id { 33, 55, 67, 88};ok
ip id != { 33, 55, 67, 88};ok

ip frag-off 0xde accept;ok
ip frag-off != 0xe9;ok
ip frag-off 0x21-0x2d;ok
ip frag-off != 0x21-0x2d;ok
ip frag-off { 0x21, 0x37, 0x43, 0x58};ok
ip frag-off != { 0x21, 0x37, 0x43, 0x58};ok
ip frag-off & 0x1fff != 0x0;ok
ip frag-off & 0x2000 != 0x0;ok
ip frag-off & 0x4000 != 0x0;ok

ip ttl 0 drop;ok
ip ttl 233;ok
ip ttl 33-55;ok
ip ttl != 45-50;ok
ip ttl {43, 53, 45 };ok
ip ttl != {43, 53, 45 };ok

ip protocol tcp;ok;ip protocol 6
ip protocol != tcp;ok;ip protocol != 6
ip protocol { icmp, esp, ah, comp, udp, udplite, tcp, dccp, sctp} accept;ok;ip protocol { 33, 136, 17, 51, 50, 6, 132, 1, 108} accept
ip protocol != { icmp, esp, ah, comp, udp, udplite, tcp, dccp, sctp} accept;ok;ip protocol != { 33, 136, 17, 51, 50, 6, 132, 1, 108} accept

ip protocol 255;ok
ip protocol 256;fail

ip checksum 13172 drop;ok
ip checksum 22;ok
ip checksum != 233;ok
ip checksum 33-45;ok
ip checksum != 33-45;ok
ip checksum { 33, 55, 67, 88};ok
ip checksum != { 33, 55, 67, 88};ok

ip saddr set {192.19.1.2, 191.1.22.1};fail

ip saddr 192.168.2.0/24;ok
ip saddr != 192.168.2.0/24;ok
ip saddr 192.168.3.1 ip daddr 192.168.3.100;ok
ip saddr != 1.1.1.1;ok
ip saddr 1.1.1.1;ok
ip daddr 192.168.0.1-192.168.0.250;ok
ip daddr 10.0.0.0-10.255.255.255;ok
ip daddr 172.16.0.0-172.31.255.255;ok
ip daddr 192.168.3.1-192.168.4.250;ok
ip daddr != 192.168.0.1-192.168.0.250;ok
ip daddr { 192.168.5.1, 192.168.5.2, 192.168.5.3 } accept;ok
ip daddr != { 192.168.5.1, 192.168.5.2, 192.168.5.3 } accept;ok

ip daddr 192.168.1.2-192.168.1.55;ok
ip daddr != 192.168.1.2-192.168.1.55;ok
ip saddr 192.168.1.3-192.168.33.55;ok
ip saddr != 192.168.1.3-192.168.33.55;ok

ip daddr 192.168.0.1;ok
ip daddr 192.168.0.1 drop;ok
ip daddr 192.168.0.2;ok

ip saddr & 0xff == 1;ok;ip saddr & 0.0.0.255 == 0.0.0.1
ip saddr & 0.0.0.255 < 0.0.0.127;ok

ip saddr & 0xffff0000 == 0xffff0000;ok;ip saddr 255.255.0.0/16

ip version 4 ip hdrlength 5;ok
ip hdrlength 0;ok
ip hdrlength 15;ok
ip hdrlength vmap { 0-4 : drop, 5 : accept, 6 : continue } counter;ok
ip hdrlength 16;fail

# limit impact to lo
iif "lo" ip daddr set 127.0.0.1;ok
iif "lo" ip checksum set 0;ok
iif "lo" ip id set 0;ok
iif "lo" ip ecn set 1;ok;iif "lo" ip ecn set ect1
iif "lo" ip ecn set ce;ok
iif "lo" ip ttl set 23;ok
iif "lo" ip protocol set 1;ok

iif "lo" ip dscp set af23;ok
iif "lo" ip dscp set cs0;ok

ip saddr . ip daddr { 192.0.2.1 . 10.0.0.1-10.0.0.2 };ok
ip saddr . ip daddr vmap { 192.168.5.1-192.168.5.128 . 192.168.6.1-192.168.6.128 : accept };ok

ip saddr 1.2.3.4 ip daddr 3.4.5.6;ok
ip saddr 1.2.3.4 counter ip daddr 3.4.5.6;ok

ip dscp 1/6;ok;ip dscp & 0x3f == lephb

ip ecn set ip ecn | ect0;ok
ip ecn set ip ecn | ect1;ok
ip ecn set ip ecn & ect0;ok
ip ecn set ip ecn & ect1;ok
tcp flags set tcp flags & (fin | syn | rst | psh | ack | urg);ok
tcp flags set tcp flags | ecn | cwr;ok
ip dscp set ip dscp | lephb;ok
ip dscp set ip dscp & lephb;ok
ip dscp set ip dscp & 0x1f;ok
ip dscp set ip dscp & 0x4f;fail
ip version set ip version | 1;ok
ip version set ip version & 1;ok
ip version set ip version | 0x1f;fail
ip hdrlength set ip hdrlength | 1;ok
ip hdrlength set ip hdrlength & 1;ok
ip hdrlength set ip hdrlength | 0x1f;fail
