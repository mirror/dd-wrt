:input;type filter hook input priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input

tcp sport 1 tcp dport 2;ok
tcp sport != 1 tcp dport != 2;ok
tcp sport 1 tcp dport != 2;ok
tcp sport != 1 tcp dport 2;ok
meta l4proto != 6 th dport 2;ok
meta l4proto 6 tcp dport 22;ok;tcp dport 22
tcp sport > 1 tcp dport > 2;ok
tcp sport 1 tcp dport > 2;ok
