:input;type filter hook input priority 0

*ip;test-ip4;input

icmp type echo-request;ok
meta l4proto icmp icmp type echo-request;ok;icmp type echo-request
meta l4proto ipv6-icmp icmpv6 type nd-router-advert;ok;icmpv6 type nd-router-advert
meta l4proto 58 icmpv6 type nd-router-advert;ok;icmpv6 type nd-router-advert
icmpv6 type nd-router-advert;ok

meta protocol ip udp dport 67;ok;udp dport 67

meta ibrname "br0";fail
meta obrname "br0";fail

meta sdif "lo" accept;ok
meta sdifname != "vrf1" accept;ok

meta mark set ip dscp;ok

meta mark set ip dscp << 2 | 0x10;ok
meta mark set ip dscp << 26 | 0x10;ok
