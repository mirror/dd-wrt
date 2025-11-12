:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*netdev;test-netdev;ingress,egress

dup to "lo";ok
dup to meta mark map { 0x00000001 : "lo", 0x00000002 : "lo"};ok

