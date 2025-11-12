:input;type filter hook input priority 0

*inet;test-inet;input

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

mark 12345 reject with tcp reset;ok;meta l4proto 6 meta mark 0x00003039 reject with tcp reset

reject;ok
meta nfproto ipv4 reject;ok;reject with icmp port-unreachable
meta nfproto ipv6 reject;ok;reject with icmpv6 port-unreachable

reject with icmpx host-unreachable;ok
reject with icmpx no-route;ok
reject with icmpx admin-prohibited;ok
reject with icmpx port-unreachable;ok;reject
reject with icmpx 3;ok;reject with icmpx admin-prohibited

meta nfproto ipv4 reject with icmp host-unreachable;ok;reject with icmp host-unreachable
meta nfproto ipv6 reject with icmpv6 no-route;ok;reject with icmpv6 no-route

meta nfproto ipv6 reject with icmp host-unreachable;fail
meta nfproto ipv4 ip protocol icmp reject with icmpv6 no-route;fail
meta nfproto ipv6 ip protocol icmp reject with icmp host-unreachable;fail
meta l4proto udp reject with tcp reset;fail

meta nfproto ipv4 reject with icmpx admin-prohibited;ok
meta nfproto ipv6 reject with icmpx admin-prohibited;ok

ether saddr aa:bb:cc:dd:ee:ff ip daddr 192.168.0.1 reject;ok;ether saddr aa:bb:cc:dd:ee:ff ip daddr 192.168.0.1 reject with icmp port-unreachable
