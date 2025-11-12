:input;type filter hook input priority 0

*ip6;test-ip6;input

srh last-entry 0;ok
srh last-entry 127;ok
srh last-entry { 0, 4-127, 255 };ok

srh flags 0;ok
srh flags 127;ok
srh flags { 0, 4-127, 255 };ok

srh tag 0;ok
srh tag 127;ok
srh tag { 0, 4-127, 0xffff };ok;srh tag { 0, 4-127, 65535 }

srh sid[1] dead::beef;ok
srh sid[2] dead::beef;ok

srh last-entry { 0, 4-127, 256 };fail
srh flags { 0, 4-127, 256 };fail
srh tag { 0, 4-127, 0x10000 };fail
