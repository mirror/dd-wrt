:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0

*inet;test-inet;input

meta nfproto ipv4;ok
meta nfproto ipv6;ok
meta nfproto {ipv4, ipv6};ok
meta nfproto != {ipv4, ipv6};ok
meta nfproto ipv6 tcp dport 22;ok
meta nfproto ipv4 tcp dport 22;ok
meta nfproto ipv4 ip saddr 1.2.3.4;ok;ip saddr 1.2.3.4
meta nfproto ipv6 meta l4proto tcp;ok;meta nfproto ipv6 meta l4proto 6
meta nfproto ipv4 counter ip saddr 1.2.3.4;ok

meta protocol ip udp dport 67;ok
meta protocol ip6 udp dport 67;ok

meta ipsec exists;ok
meta secpath missing;ok;meta ipsec missing
meta ibrname "br0";fail
meta obrname "br0";fail
meta mark set ct mark >> 8;ok

meta mark . tcp dport { 0x0000000a-0x00000014 . 80-90, 0x00100000-0x00100123 . 100-120 };ok
ip saddr . meta mark { 1.2.3.4 . 0x00000100 , 1.2.3.6-1.2.3.8 . 0x00000200-0x00000300 };ok
ip saddr . meta mark { 1.2.3.4 . 0x00000100 , 5.6.7.8 . 0x00000200 };ok
ip saddr . ether saddr . meta l4proto { 1.2.3.4 . aa:bb:cc:dd:ee:ff . 6 };ok

meta mark set ip dscp;ok
meta mark set ip dscp | 0x40;ok
meta mark set ip6 dscp;ok
meta mark set ip6 dscp | 0x40;ok

meta mark set ct mark and 0xffff0000 or meta mark and 0xffff;ok;meta mark set ct mark & 0xffff0000 | meta mark & 0x0000ffff
