:y;type filter hook prerouting priority -150

*inet;x;y

tproxy;fail
meta l4proto 17 tproxy to 192.0.2.1;fail
meta l4proto 6 tproxy to 192.0.2.1:50080;fail
meta l4proto 17 tproxy ip to 192.0.2.1;ok
meta l4proto 6 tproxy ip to 192.0.2.1:50080;ok
ip protocol 6 tproxy ip6 to [2001:db8::1];fail

meta l4proto 6 tproxy to [2001:db8::1];fail
meta l4proto 17 tproxy to [2001:db8::1]:50080;fail
meta l4proto 6 tproxy ip6 to [2001:db8::1];ok
meta l4proto 17 tproxy ip6 to [2001:db8::1]:50080;ok
ip6 nexthdr 6 tproxy ip to 192.0.2.1;fail

meta l4proto 17 tproxy ip to :50080;ok
meta l4proto 17 tproxy ip6 to :50080;ok
meta l4proto 17 tproxy to :50080;ok
ip daddr 0.0.0.0/0 meta l4proto 6 tproxy ip to :2000;ok

meta l4proto 6 tproxy ip to 127.0.0.1:symhash mod 2 map { 0 : 23, 1 : 42 };ok
