:input;type filter hook input priority 0

*inet;test-inet;input

ip protocol icmp icmp type echo-request;ok;ip protocol 1 icmp type echo-request
icmp type echo-request;ok
ip6 nexthdr icmpv6 icmpv6 type echo-request;ok;ip6 nexthdr 58 icmpv6 type echo-request
icmpv6 type echo-request;ok
# must not remove 'ip protocol' dependency, this explicitly matches icmpv6-in-ipv4.
ip protocol ipv6-icmp meta l4proto ipv6-icmp icmpv6 type 1;ok;ip protocol 58 icmpv6 type destination-unreachable
