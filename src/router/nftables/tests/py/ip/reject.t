:output;type filter hook output priority 0

*ip;test-ip4;output

reject;ok
reject with icmp host-unreachable;ok
reject with icmp net-unreachable;ok
reject with icmp prot-unreachable;ok
reject with icmp port-unreachable;ok;reject
reject with icmp net-prohibited;ok
reject with icmp host-prohibited;ok
reject with icmp admin-prohibited;ok
reject with icmp 3;ok;reject
mark 0x80000000 reject with tcp reset;ok;meta mark 0x80000000 reject with tcp reset

reject with icmp no-route;fail
reject with icmpv6 no-route;fail
