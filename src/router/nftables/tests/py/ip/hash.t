:pre;type nat hook prerouting priority 0
*ip;test-ip4;pre

ct mark set jhash ip saddr . ip daddr mod 2 seed 0xdeadbeef;ok
ct mark set jhash ip saddr . ip daddr mod 2;ok
ct mark set jhash ip saddr . ip daddr mod 2 seed 0x0;ok
ct mark set jhash ip saddr . ip daddr mod 2 seed 0xdeadbeef offset 100;ok
ct mark set jhash ip saddr . ip daddr mod 2 offset 100;ok
dnat to jhash ip saddr mod 2 seed 0xdeadbeef map { 0 : 192.168.20.100, 1 : 192.168.30.100 };ok
ct mark set symhash mod 2 offset 100;ok
