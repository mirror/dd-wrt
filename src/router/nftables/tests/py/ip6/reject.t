:output;type filter hook output priority 0

*ip6;test-ip6;output

reject;ok
reject with icmpv6 no-route;ok
reject with icmpv6 admin-prohibited;ok
reject with icmpv6 addr-unreachable;ok
reject with icmpv6 port-unreachable;ok;reject
reject with icmpv6 policy-fail;ok
reject with icmpv6 reject-route;ok
reject with icmpv6 3;ok;reject with icmpv6 addr-unreachable
mark 0x80000000 reject with tcp reset;ok;meta mark 0x80000000 reject with tcp reset

reject with icmpv6 host-unreachable;fail
reject with icmp host-unreachable;fail
