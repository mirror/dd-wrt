:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*arp;test-arp;input
*bridge;test-bridge;input
*netdev;test-netdev;ingress

counter;ok
counter packets 0 bytes 0;ok;counter
counter packets 2 bytes 1;ok;counter
counter bytes 1024 packets 1;ok;counter
