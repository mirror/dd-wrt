:postrouting;type nat hook postrouting priority 0

*ip6;test-ip6;postrouting

tcp dport 80-90 snat to [2001:838:35f:1::]-[2001:838:35f:2::]:80-100;ok
tcp dport 80-90 snat to [2001:838:35f:1::]-[2001:838:35f:2::]:100;ok
