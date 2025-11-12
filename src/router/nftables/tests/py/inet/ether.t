:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*bridge;test-bridge;input
*netdev;test-netdev;ingress,egress

tcp dport 22 iiftype ether ether saddr 00:0f:54:0c:11:4 accept;ok;tcp dport 22 ether saddr 00:0f:54:0c:11:04 accept
tcp dport 22 ether saddr 00:0f:54:0c:11:04 accept;ok

ether saddr 00:0f:54:0c:11:04 accept;ok

vlan id 1;ok
ether type vlan vlan id 2;ok;vlan id 2

# invalid dependency
ether type ip vlan id 1;fail
