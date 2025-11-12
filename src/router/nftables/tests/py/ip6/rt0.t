:output;type filter hook input priority 0

*ip6;test-ip6;output

rt nexthop 192.168.0.1;fail
rt nexthop fd00::1;ok;rt ip6 nexthop fd00::1
