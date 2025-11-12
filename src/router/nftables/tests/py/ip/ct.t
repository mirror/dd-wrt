:output;type filter hook output priority 0

*ip;test-ip4;output

ct original ip saddr 192.168.0.1;ok
ct reply ip saddr 192.168.0.1;ok
ct original ip daddr 192.168.0.1;ok
ct reply ip daddr 192.168.0.1;ok

# same, but with a netmask
ct original ip saddr 192.168.1.0/24;ok
ct reply ip saddr 192.168.1.0/24;ok
ct original ip daddr 192.168.1.0/24;ok
ct reply ip daddr 192.168.1.0/24;ok

ct l3proto ipv4;ok
ct l3proto foobar;fail

ct protocol 6 ct original proto-dst 22;ok
ct original protocol 17 ct reply proto-src 53;ok;ct protocol 17 ct reply proto-src 53

# wrong address family
ct reply ip daddr dead::beef;fail

meta mark set ct original daddr map { 1.1.1.1 : 0x00000011 };fail
meta mark set ct original ip daddr map { 1.1.1.1 : 0x00000011 };ok
meta mark set ct original saddr . meta mark map { 1.1.1.1 . 0x00000014 : 0x0000001e };fail
meta mark set ct original ip saddr . meta mark map { 1.1.1.1 . 0x00000014 : 0x0000001e };ok
ct original saddr . meta mark { 1.1.1.1 . 0x00000014 };fail
ct original ip saddr . meta mark { 1.1.1.1 . 0x00000014 };ok

ct mark set ip dscp << 2 | 0x10;ok
ct mark set ip dscp << 26 | 0x10;ok
ct mark set ip dscp & 0x0f << 1;ok;ct mark set ip dscp & af33
ct mark set ip dscp & 0x0f << 2;ok;ct mark set ip dscp & 0x3c
ct mark set ip dscp | 0x04;ok
ct mark set ip dscp | 1 << 20;ok;ct mark set ip dscp | 0x100000
ct mark set ct mark | ip dscp | 0x200 counter;ok;ct mark set ct mark | ip dscp | 0x00000200 counter
