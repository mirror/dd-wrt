:input;type filter hook input priority 0

*ip;test-ip4;input

dup to 192.168.2.1;ok
dup to 192.168.2.1 device "lo";ok
dup to ip saddr map { 192.168.2.120 : 192.168.2.1 } device "lo";ok
