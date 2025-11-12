:y;type filter hook prerouting priority -150

*ip;x;y

tproxy;fail
tproxy to 192.0.2.1;fail
tproxy to 192.0.2.1:50080;fail
tproxy to :50080;fail
meta l4proto 17 tproxy to 192.0.2.1;ok
meta l4proto 6 tproxy to 192.0.2.1:50080;ok
ip protocol 6 tproxy to :50080;ok
meta l4proto 17 tproxy ip to 192.0.2.1;ok;meta l4proto 17 tproxy to 192.0.2.1
meta l4proto 6 tproxy ip to 192.0.2.1:50080;ok;meta l4proto 6 tproxy to 192.0.2.1:50080
ip protocol 6 tproxy ip to :50080;ok;ip protocol 6 tproxy to :50080
