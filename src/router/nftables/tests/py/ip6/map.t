:input;type filter hook input priority 0
*ip6;test-ip6;input

mark set ip6 saddr and ::ffff map { ::2 : 0x0000002a, ::ffff : 0x00000017};ok;meta mark set ip6 saddr & ::ffff map { ::2 : 0x0000002a, ::ffff : 0x00000017}

