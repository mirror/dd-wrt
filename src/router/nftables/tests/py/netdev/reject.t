:ingress;type filter hook ingress device lo priority 0

*netdev;test-netdev;ingress

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
reject with icmpv6 policy-fail;ok
reject with icmpv6 reject-route;ok

mark 12345 reject with tcp reset;ok;meta l4proto 6 meta mark 0x00003039 reject with tcp reset

reject;ok
meta protocol ip reject;ok;reject with icmp port-unreachable
meta protocol ip6 reject;ok;reject with icmpv6 port-unreachable

reject with icmpx host-unreachable;ok
reject with icmpx no-route;ok
reject with icmpx admin-prohibited;ok
reject with icmpx port-unreachable;ok;reject

meta protocol ip reject with icmp host-unreachable;ok;reject with icmp host-unreachable
meta protocol ip6 reject with icmpv6 no-route;ok;reject with icmpv6 no-route

meta protocol ip6 reject with icmp host-unreachable;fail
meta protocol ip ip protocol icmp reject with icmpv6 no-route;fail
meta protocol ip6 ip protocol icmp reject with icmp host-unreachable;fail
meta l4proto udp reject with tcp reset;fail

meta protocol ip reject with icmpx admin-prohibited;ok
meta protocol ip6 reject with icmpx admin-prohibited;ok
