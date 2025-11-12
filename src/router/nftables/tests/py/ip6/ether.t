:input;type filter hook input priority 0

*ip6;test-ip6;input

tcp dport 22 iiftype ether ip6 daddr 1::2 ether saddr 00:0f:54:0c:11:4 accept;ok;tcp dport 22 ip6 daddr 1::2 ether saddr 00:0f:54:0c:11:04 accept
tcp dport 22 ip6 daddr 1::2 ether saddr 00:0f:54:0c:11:04;ok
tcp dport 22 ether saddr 00:0f:54:0c:11:04 ip6 daddr 1::2;ok
ether saddr 00:0f:54:0c:11:04 ip6 daddr 1::2 accept;ok
