:output;type filter hook output priority 0

*ip6;test-ip6;output

ct mark set ip6 dscp << 2 | 0x10;ok
ct mark set ip6 dscp << 26 | 0x10;ok
ct mark set ip6 dscp | 0x04;ok
ct mark set ip6 dscp | 0xff000000;ok
ct mark set ip6 dscp & 0x0f << 2;ok;ct mark set ip6 dscp & 0x3c
ct mark set ct mark | ip6 dscp | 0x200 counter;ok;ct mark set ct mark | ip6 dscp | 0x00000200 counter
