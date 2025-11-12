:input;type filter hook input priority 0

*bridge;test-bridge;input

ip protocol icmp icmp type echo-request;ok;ip protocol 1 icmp type echo-request
icmp type echo-request;ok
ip6 nexthdr icmpv6 icmpv6 type echo-request;ok;ip6 nexthdr 58 icmpv6 type echo-request
icmpv6 type echo-request;ok
