:input;type filter hook input priority 0

*ip;test-ip;input

ip protocol tcp tcp dport ssh accept;ok;tcp dport 22 accept
ip protocol ne tcp udp dport ssh accept;ok;ip protocol != 6 udp dport 22 accept
