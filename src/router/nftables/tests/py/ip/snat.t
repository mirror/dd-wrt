:postrouting;type nat hook postrouting priority 0

*ip;test-ip4;postrouting

iifname "eth0" tcp dport 80-90 snat to 192.168.3.2;ok
iifname "eth0" tcp dport != 80-90 snat to 192.168.3.2;ok
iifname "eth0" tcp dport {80, 90, 23} snat to 192.168.3.2;ok
iifname "eth0" tcp dport != {80, 90, 23} snat to 192.168.3.2;ok
iifname "eth0" tcp dport 80-90 snat to 192.168.3.0-192.168.3.255;ok;iifname "eth0" tcp dport 80-90 snat to 192.168.3.0/24
iifname "eth0" tcp dport 80-90 snat to 192.168.3.15-192.168.3.240;ok

iifname "eth0" tcp dport != 23-34 snat to 192.168.3.2;ok

meta l4proto 17 snat ip to ip saddr map { 10.141.11.4 : 192.168.2.3 . 80 };ok
snat ip to ip saddr map { 10.141.11.4 : 192.168.2.2-192.168.2.4 };ok
snat ip to ip saddr map { 10.141.12.14 : 192.168.2.0/24 };ok
snat ip prefix to ip saddr map { 10.141.11.0/24 : 192.168.2.0/24 };ok

meta l4proto { 6, 17} snat ip to ip saddr . th dport map { 10.141.11.4 . 20 : 192.168.2.3 . 80};ok
snat ip to ip saddr map { 10.141.11.4 : 192.168.2.3 . 80 };fail
snat ip to ip saddr . th dport map { 10.141.11.4 . 20 : 192.168.2.3 . 80 };fail
