:pre;type nat hook prerouting priority 0
*ip;test-ip4;pre

ct mark set numgen inc mod 2;ok
ct mark set numgen inc mod 2 offset 100;ok
dnat to numgen inc mod 2 map { 0 : 192.168.10.100, 1 : 192.168.20.200 };ok
dnat to numgen inc mod 10 map { 0-5 : 192.168.10.100, 6-9 : 192.168.20.200};ok
dnat to numgen inc mod 7 offset 167772161;ok
dnat to numgen inc mod 255 offset 167772161;ok
