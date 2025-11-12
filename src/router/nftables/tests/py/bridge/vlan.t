:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*bridge;test-bridge;input
*netdev;test-netdev;ingress,egress

vlan id 4094;ok
vlan id 0;ok
# bad vlan id
vlan id 4096;fail
vlan id 4094 vlan dei 0;ok
vlan id 4094 vlan dei 1;ok
vlan id 4094 vlan dei != 1;ok
vlan id 4094 vlan cfi 1;ok;vlan id 4094 vlan dei 1
# bad dei
vlan id 4094 vlan dei 2;fail
vlan id 4094 vlan dei 1 vlan pcp 8;fail
vlan id 4094 vlan dei 1 vlan pcp 7;ok
vlan id 4094 vlan dei 1 vlan pcp 3;ok

ether type vlan vlan id 4094;ok;vlan id 4094
ether type vlan vlan id 0;ok;vlan id 0
ether type vlan vlan id 4094 vlan dei 0;ok;vlan id 4094 vlan dei 0
ether type vlan vlan id 4094 vlan dei 1;ok;vlan id 4094 vlan dei 1
ether type vlan vlan id 4094 vlan dei 2;fail

vlan id 4094 tcp dport 22;ok
vlan id 1 ip saddr 10.0.0.1;ok
vlan id 1 ip saddr 10.0.0.0/23;ok
vlan id 1 ip saddr 10.0.0.0/23 udp dport 53;ok
ether type vlan vlan id 1 ip saddr 10.0.0.0/23 udp dport 53;ok;vlan id 1 ip saddr 10.0.0.0/23 udp dport 53

vlan id { 1, 2, 4, 100, 4095 } vlan pcp 1-3;ok
vlan id { 1, 2, 4, 100, 4096 };fail

ether type vlan ip protocol 1 accept;ok;ether type 8021q ip protocol 1 accept

# IEEE 802.1AD
ether type 8021ad vlan id 1 ip protocol 6 accept;ok
ether type 8021ad vlan id 1 vlan type 8021q vlan id 2 vlan type ip counter;ok
ether type 8021ad vlan id 1 vlan type 8021q vlan id 2 vlan type ip ip protocol 6;ok;ether type 8021ad vlan id 1 vlan type 8021q vlan id 2 ip protocol 6

# illegal dependencies
ether type ip vlan id 1;fail
ether type ip vlan id 1 ip saddr 10.0.0.1;fail

# mangling
vlan id 1 vlan id set 2;ok

ether saddr 00:01:02:03:04:05 vlan id 1;ok
vlan id 2 ether saddr 0:1:2:3:4:6;ok;ether saddr 00:01:02:03:04:06 vlan id 2

ether saddr . vlan id { 0a:0b:0c:0d:0e:0f . 42, 0a:0b:0c:0d:0e:0f . 4095 };ok

ether saddr 00:11:22:33:44:55 counter ether type 8021q;ok
