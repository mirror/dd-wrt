:prerouting;type filter hook prerouting priority 0

*bridge;test-bridge;prerouting

ether daddr set meta ibrhwaddr;ok
meta ibrhwaddr set 00:1a:2b:3c:4d:5e;fail
