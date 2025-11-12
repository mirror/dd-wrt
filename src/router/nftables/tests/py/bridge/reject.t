:input;type filter hook input priority 0

*bridge;test-bridge;input

# The output is specific for bridge family
reject with icmp host-unreachable;ok
reject with icmp net-unreachable;ok
reject with icmp prot-unreachable;ok
reject with icmp port-unreachable;ok
reject with icmp net-prohibited;ok
reject with icmp host-prohibited;ok
reject with icmp admin-prohibited;ok

reject with icmpv6 no-route;ok
reject with icmpv6 admin-prohibited;ok
reject with icmpv6 addr-unreachable;ok
reject with icmpv6 port-unreachable;ok

mark 12345 ip protocol tcp reject with tcp reset;ok;meta mark 0x00003039 ip protocol 6 reject with tcp reset

reject;ok
ether type ip reject;ok;reject with icmp port-unreachable
ether type ip6 reject;ok;reject with icmpv6 port-unreachable

reject with icmpx host-unreachable;ok
reject with icmpx no-route;ok
reject with icmpx admin-prohibited;ok
reject with icmpx port-unreachable;ok;reject

ether type ipv6 reject with icmp host-unreachable;fail
ether type ip6 reject with icmp host-unreachable;fail
ether type ip reject with icmpv6 no-route;fail
ether type vlan reject;ok;ether type 8021q reject
ether type arp reject;fail
ether type vlan reject with tcp reset;ok;meta l4proto 6 ether type 8021q reject with tcp reset
ether type arp reject with tcp reset;fail
ip protocol udp reject with tcp reset;fail

ether type ip reject with icmpx admin-prohibited;ok
ether type ip6 reject with icmpx admin-prohibited;ok
ether type 8021q reject with icmpx admin-prohibited;ok
ether type arp reject with icmpx admin-prohibited;fail
