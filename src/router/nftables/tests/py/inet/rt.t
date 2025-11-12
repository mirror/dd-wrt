:output;type filter hook output priority 0

*inet;test-inet;output

meta nfproto ipv4 rt nexthop 192.168.0.1;ok;meta nfproto ipv4 rt ip nexthop 192.168.0.1
rt ip6 nexthop fd00::1;ok

# missing context
rt nexthop 192.168.0.1;fail
rt nexthop fd00::1;fail

# wrong context
rt ip nexthop fd00::1;fail

tcp option maxseg size set rt mtu;ok
