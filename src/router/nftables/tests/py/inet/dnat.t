:prerouting;type nat hook prerouting priority 0

*inet;test-inet;prerouting

iifname "foo" tcp dport 80 redirect to :8080;ok

iifname "eth0" tcp dport 443 dnat ip to 192.168.3.2;ok
iifname "eth0" tcp dport 443 dnat ip6 to [dead::beef]:4443;ok
meta l4proto tcp dnat to :80;ok;meta l4proto 6 dnat to :80

dnat ip to ct mark map { 0x00000014 : 1.2.3.4};ok
dnat ip to ct mark . ip daddr map { 0x00000014 . 1.1.1.1 : 1.2.3.4};ok

dnat ip6 to 1.2.3.4;fail
dnat to 1.2.3.4;fail
dnat ip6 to ct mark . ip daddr map { 0x00000014 . 1.1.1.1 : 1.2.3.4};fail
ip6 daddr dead::beef dnat to 10.1.2.3;fail

meta l4proto { tcp, udp } dnat ip to 1.1.1.1:80;ok;meta l4proto { 6, 17} dnat ip to 1.1.1.1:80
ip protocol { tcp, udp } dnat ip to 1.1.1.1:80;ok;ip protocol { 6, 17} dnat ip to 1.1.1.1:80
meta l4proto { tcp, udp } tcp dport 20 dnat to 1.1.1.1:80;fail
ip protocol { tcp, udp } tcp dport 20 dnat to 1.1.1.1:80;fail
