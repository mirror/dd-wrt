:output;type filter hook output priority 0

*inet;test-inet;output

# without nfproto specified, these should add an implicit dependency on
# the likely l3 proto (i.e., IPv6 for icmpv6 and IPv4 for icmp)

icmp type echo-request;ok
icmpv6 type echo-request;ok

# make sure only those nfproto matches are dropped if
# the next statement would add it as a dependency anyway

meta nfproto ipv4 icmp type echo-request;ok;icmp type echo-request
meta nfproto ipv4 icmpv6 type echo-request;ok

meta nfproto ipv6 icmp type echo-request;ok
meta nfproto ipv6 icmpv6 type echo-request;ok;icmpv6 type echo-request
