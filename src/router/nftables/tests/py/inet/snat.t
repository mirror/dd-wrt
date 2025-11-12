:postrouting;type nat hook postrouting priority 0

*inet;test-inet;postrouting

# explicit family: 'snat to ip':
iifname "eth0" tcp dport 81 snat ip to 192.168.3.2;ok

# infer snat target family from network header base:
iifname "eth0" tcp dport 81 ip saddr 10.1.1.1 snat to 192.168.3.2;ok;iifname "eth0" tcp dport 81 ip saddr 10.1.1.1 snat ip to 192.168.3.2
iifname "eth0" tcp dport 81 snat ip6 to dead::beef;ok

iifname "foo" masquerade random;ok


snat to 192.168.3.2;fail
snat ip6 to 192.168.3.2;fail
snat to dead::beef;fail
snat ip to dead::beef;fail
snat ip daddr 1.2.3.4 to dead::beef;fail
snat ip daddr 1.2.3.4 ip6 to dead::beef;fail
snat ip6 saddr dead::beef to 1.2.3.4;fail
