:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*inet;test-inet;input
*bridge;test-bridge;input
*netdev;test-netdev;ingress,egress

ip saddr . ip daddr . ether saddr { 1.1.1.1 . 2.2.2.2 . ca:fe:ca:fe:ca:fe };ok
ip saddr vmap { 10.0.1.0-10.0.1.255 : accept, 10.0.1.1-10.0.2.255 : drop };fail
ip saddr vmap { 3.3.3.3-3.3.3.4 : accept, 1.1.1.1-1.1.1.255 : accept, 1.1.1.0-1.1.2.1 : drop};fail
