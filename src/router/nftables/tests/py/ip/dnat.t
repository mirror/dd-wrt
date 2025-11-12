:prerouting;type nat hook prerouting priority 0

*ip;test-ip4;prerouting

iifname "eth0" tcp dport 80-90 dnat to 192.168.3.2;ok
iifname "eth0" tcp dport != 80-90 dnat to 192.168.3.2;ok
iifname "eth0" tcp dport {80, 90, 23} dnat to 192.168.3.2;ok
iifname "eth0" tcp dport != {80, 90, 23} dnat to 192.168.3.2;ok
iifname "eth0" tcp dport != 23-34 dnat to 192.168.3.2;ok
iifname "eth0" tcp dport 81 dnat to 192.168.3.2:8080;ok
iifname "eth0" tcp dport 81 dnat to 192.168.3.2:8080-8999;ok
iifname "eth0" tcp dport 81 dnat to 192.168.3.2-192.168.3.4:8080;ok
iifname "eth0" tcp dport 81 dnat to 192.168.3.2-192.168.3.4:8080-8999;ok

dnat to ct mark map { 0x00000014 : 1.2.3.4};ok
dnat to ct mark . ip daddr map { 0x00000014 . 1.1.1.1 : 1.2.3.4};ok

dnat ip to ip saddr . tcp dport map { 192.168.1.2 . 80 : 10.141.10.0/24  . 8888 - 8999 };ok
dnat ip to ip saddr . tcp dport map { 192.168.1.2 . 80 : 10.141.10.0/24  . 80 };ok
dnat ip to ip saddr . tcp dport map { 192.168.1.2 . 80 : 10.141.10.2 . 8888 - 8999 };ok
ip daddr 192.168.0.1 dnat ip to tcp dport map { 443 : 10.141.10.4 . 8443, 80 : 10.141.10.4 . 8080 };ok
meta l4proto 6 dnat ip to iifname . ip saddr map { "enp2s0" . 10.1.1.136 : 1.1.2.69 . 22, "enp2s0" . 10.1.1.1-10.1.1.135 : 1.1.2.66-1.84.236.78 . 22 };ok
dnat ip to iifname . ip saddr map { "enp2s0" . 10.1.1.136 : 1.1.2.69/32, "enp2s0" . 10.1.1.1-10.1.1.135 : 1.1.2.66-1.84.236.78 };ok
