:input;type filter hook input priority 0

*ip;test-ip;input

tcp dport 22 iiftype ether ip daddr 1.2.3.4 ether saddr 00:0f:54:0c:11:4 accept;ok;tcp dport 22 ip daddr 1.2.3.4 ether saddr 00:0f:54:0c:11:04 accept
tcp dport 22 ip daddr 1.2.3.4 ether saddr 00:0f:54:0c:11:04;ok
tcp dport 22 ether saddr 00:0f:54:0c:11:04 ip daddr 1.2.3.4;ok
ether saddr 00:0f:54:0c:11:04 ip daddr 1.2.3.4 accept;ok
