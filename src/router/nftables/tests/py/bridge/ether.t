:input;type filter hook input priority 0

*bridge;test-bridge;input

tcp dport 22 iiftype ether ip daddr 1.2.3.4 ether saddr 00:0f:54:0c:11:4 accept;ok;tcp dport 22 ip daddr 1.2.3.4 ether saddr 00:0f:54:0c:11:04 accept
tcp dport 22 ip daddr 1.2.3.4 ether saddr 00:0f:54:0c:11:04;ok
tcp dport 22 ether saddr 00:0f:54:0c:11:04 ip daddr 1.2.3.4;ok
ether saddr 00:0f:54:0c:11:04 ip daddr 1.2.3.4 accept;ok

ether daddr 00:01:02:03:04:05 ether saddr set ff:fe:dc:ba:98:76 drop;ok

ether daddr set {01:00:5e:00:01:01, 01:00:5e:00:02:02};fail
