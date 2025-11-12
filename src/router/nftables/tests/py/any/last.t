:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*arp;test-arp;input
*bridge;test-bridge;input
*netdev;test-netdev;ingress

last;ok
last used 300s;ok;last
last used foo;fail
