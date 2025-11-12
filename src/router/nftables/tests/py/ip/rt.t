:output;type filter hook input priority 0

*ip;test-ip4;output

rt nexthop 192.168.0.1;ok;rt ip nexthop 192.168.0.1
rt nexthop fd00::1;fail
rt ip6 nexthop fd00::1;fail
