:input;type filter hook input priority 0

*ip6;test-ip6;input

icmpv6 type nd-router-advert;ok
meta l4proto ipv6-icmp icmpv6 type nd-router-advert;ok;icmpv6 type nd-router-advert

meta l4proto icmp icmp type echo-request;ok;icmp type echo-request
meta l4proto 1 icmp type echo-request;ok;icmp type echo-request
icmp type echo-request;ok

meta protocol ip udp dport 67;ok
meta protocol ip6 udp dport 67;ok;udp dport 67

meta sdif "lo" accept;ok
meta sdifname != "vrf1" accept;ok

meta mark set ip6 dscp << 2 | 0x10;ok
meta mark set ip6 dscp << 26 | 0x10;ok
