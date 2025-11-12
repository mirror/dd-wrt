:y;type filter hook prerouting priority -150

*ip6;x;y

tproxy;fail
tproxy to [2001:db8::1];fail
tproxy to [2001:db8::1]:50080;fail
tproxy to :50080;fail
meta l4proto 6 tproxy to [2001:db8::1];ok
meta l4proto 17 tproxy to [2001:db8::1]:50080;ok
meta l4proto 6 tproxy to :50080;ok
meta l4proto 6 tproxy ip6 to [2001:db8::1];ok;meta l4proto 6 tproxy to [2001:db8::1]
meta l4proto 17 tproxy ip6 to [2001:db8::1]:50080;ok;meta l4proto 17 tproxy to [2001:db8::1]:50080
meta l4proto 6 tproxy ip6 to :50080;ok;meta l4proto 6 tproxy to :50080
