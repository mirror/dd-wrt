:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*arp;test-arp;input
*bridge;test-bridge;input
*netdev;test-netdev;ingress,egress

meta length 1000;ok
meta length 22;ok
meta length != 233;ok
meta length 33-45;ok
meta length != 33-45;ok
meta length { 33, 55, 67, 88};ok
meta length { 33-55, 67-88};ok
meta length { 33-55, 56-88, 100-120};ok;meta length { 33-88, 100-120}
meta length != { 33, 55, 67, 88};ok
meta length { 33-55, 66-88};ok
meta length != { 33-55, 66-88};ok

meta protocol { ip, arp, ip6, vlan };ok;meta protocol { ip6, ip, 8021q, arp}
meta protocol != {ip, arp, ip6, 8021q};ok
meta protocol ip;ok
meta protocol != ip;ok

meta l4proto 22;ok
meta l4proto != 233;ok
meta l4proto 33-45;ok
meta l4proto != 33-45;ok
meta l4proto { 33, 55, 67, 88};ok
meta l4proto != { 33, 55, 67, 88};ok
meta l4proto { 33-55, 66-88};ok
meta l4proto != { 33-55, 66-88};ok

meta priority root;ok
meta priority none;ok
meta priority 0x87654321;ok;meta priority 8765:4321
meta priority 2271560481;ok;meta priority 8765:4321
meta priority 1:1234;ok
meta priority bcad:dadc;ok
meta priority aabb:0;ok
meta priority != bcad:dadc;ok
meta priority != aabb:0;ok
meta priority bcad:dada-bcad:dadc;ok
meta priority != bcad:dada-bcad:dadc;ok
meta priority {bcad:dada, bcad:dadc, aaaa:bbbb};ok
meta priority set cafe:beef;ok
meta priority != {bcad:dada, bcad:dadc, aaaa:bbbb};ok

meta mark 0x4;ok;meta mark 0x00000004
meta mark 0x32;ok;meta mark 0x00000032
meta mark and 0x03 == 0x01;ok;meta mark & 0x00000003 == 0x00000001
meta mark and 0x03 != 0x01;ok;meta mark & 0x00000003 != 0x00000001
meta mark 0x10;ok;meta mark 0x00000010
meta mark != 0x10;ok;meta mark != 0x00000010
meta mark 0xffffff00/24;ok;meta mark & 0xffffff00 == 0xffffff00

meta mark or 0x03 == 0x01;ok;meta mark | 0x00000003 == 0x00000001
meta mark or 0x03 != 0x01;ok;meta mark | 0x00000003 != 0x00000001
meta mark xor 0x03 == 0x01;ok;meta mark 0x00000002
meta mark xor 0x03 != 0x01;ok;meta mark != 0x00000002

meta iif "lo" accept;ok;iif "lo" accept
meta iif != "lo" accept;ok;iif != "lo" accept

meta iifname "dummy0";ok;iifname "dummy0"
meta iifname != "dummy0";ok;iifname != "dummy0"
meta iifname {"dummy0", "lo"};ok;iifname {"dummy0", "lo"}
meta iifname != {"dummy0", "lo"};ok;iifname != {"dummy0", "lo"}
meta iifname "dummy*";ok;iifname "dummy*"
meta iifname "dummy\*";ok;iifname "dummy\*"
meta iifname "";fail

meta iiftype {ether, ppp, ipip, ipip6, loopback, sit, ipgre};ok
meta iiftype != {ether, ppp, ipip, ipip6, loopback, sit, ipgre};ok
meta iiftype != ether;ok
meta iiftype ether;ok
meta iiftype != ppp;ok
meta iiftype ppp;ok

meta oif "lo" accept;ok;oif "lo" accept
meta oif != "lo" accept;ok;oif != "lo" accept

meta oifname "dummy0";ok;oifname "dummy0"
meta oifname != "dummy0";ok;oifname != "dummy0"
meta oifname { "dummy0", "lo"};ok;oifname { "dummy0", "lo"}
meta oifname "dummy*";ok;oifname "dummy*"
meta oifname "dummy\*";ok;oifname "dummy\*"
meta oifname "";fail

meta oiftype {ether, ppp, ipip, ipip6, loopback, sit, ipgre};ok
meta oiftype != {ether, ppp, ipip, ipip6, loopback, sit, ipgre};ok
meta oiftype != ether;ok
meta oiftype ether;ok

meta skuid {"bin", "root", "daemon"} accept;ok;meta skuid { 0, 1, 2} accept
meta skuid != {"bin", "root", "daemon"} accept;ok;meta skuid != { 1, 0, 2} accept
meta skuid "root";ok;meta skuid 0
meta skuid != "root";ok;meta skuid != 0
meta skuid lt 3000 accept;ok;meta skuid < 3000 accept
meta skuid gt 3000 accept;ok;meta skuid > 3000 accept
meta skuid eq 3000 accept;ok;meta skuid 3000 accept
meta skuid 3001-3005 accept;ok
meta skuid != 2001-2005 accept;ok
meta skuid { 2001-2005, 3001-3005} accept;ok
meta skuid != { 2001-2005, 3001-3005} accept;ok

meta skgid {"bin", "root", "daemon"} accept;ok;meta skgid { 0, 1, 2} accept
meta skgid != {"bin", "root", "daemon"} accept;ok;meta skgid != { 1, 0, 2} accept
meta skgid "root";ok;meta skgid 0
meta skgid != "root";ok;meta skgid != 0
meta skgid lt 3000 accept;ok;meta skgid < 3000 accept
meta skgid gt 3000 accept;ok;meta skgid > 3000 accept
meta skgid eq 3000 accept;ok;meta skgid 3000 accept
meta skgid 2001-2005 accept;ok
meta skgid != 2001-2005 accept;ok

# BUG: meta nftrace 2 and meta nftrace 1
# $ sudo nft add rule ip test input meta nftrace 2
# <cmdline>:1:37-37: Error: Value 2 exceeds valid range 0-1
# add rule ip test input meta nftrace 2
#                                    ^
# $ sudo nft add rule ip test input meta nftrace 1
# <cmdline>:1:1-37: Error: Could not process rule: Operation not supported
# add rule ip test input meta nftrace 1
# -^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

meta mark set 0xffffffc8 xor 0x16;ok;meta mark set 0xffffffde
meta mark set 0x16 and 0x16;ok;meta mark set 0x00000016
meta mark set 0xffffffe9 or 0x16;ok;meta mark set 0xffffffff
meta mark set 0xffffffde and 0x16;ok;meta mark set 0x00000016
meta mark set 0xf045ffde or 0x10;ok;meta mark set 0xf045ffde
meta mark set 0xffffffde or 0x16;ok;meta mark set 0xffffffde
meta mark set 0x32 or 0xfffff;ok;meta mark set 0x000fffff
meta mark set 0xfffe xor 0x16;ok;meta mark set 0x0000ffe8

meta mark set {0xffff, 0xcc};fail
meta pkttype set {unicast, multicast, broadcast};fail

meta iif "lo";ok;iif "lo"
meta oif "lo";ok;oif "lo"
meta oifname "dummy2" accept;ok;oifname "dummy2" accept
meta skuid 3000;ok
meta skgid 3000;ok
# BUG:  meta nftrace 1;ok
# <cmdline>:1:1-37: Error: Could not process rule: Operation not supported
- meta nftrace 1;ok
meta rtclassid "cosmos";ok

meta pkttype broadcast;ok
meta pkttype host;ok
meta pkttype multicast;ok
meta pkttype != broadcast;ok
meta pkttype != host;ok
meta pkttype != multicast;ok
meta pkttype broadcastttt;fail
pkttype { broadcast, multicast} accept;ok;meta pkttype { broadcast, multicast} accept

meta cpu 1;ok
meta cpu != 1;ok
meta cpu 1-3;ok
meta cpu != 1-2;ok
meta cpu { 2,3};ok
meta cpu { 2-3, 5-7};ok
meta cpu != { 2,3};ok

meta iifgroup 0;ok;iifgroup "default"
meta iifgroup != 0;ok;iifgroup != "default"
meta iifgroup "default";ok;iifgroup "default"
meta iifgroup != "default";ok;iifgroup != "default"
meta iifgroup {"default", 11};ok;iifgroup {"default", 11}
meta iifgroup != {"default", 11};ok;iifgroup != {"default", 11}
meta iifgroup { 11,33};ok;iifgroup { 11,33}
meta iifgroup {11-33, 44-55};ok;iifgroup {11-33, 44-55}
meta iifgroup != { 11,33};ok;iifgroup != { 11,33}
meta iifgroup != {11-33, 44-55};ok;iifgroup != {11-33, 44-55}
meta oifgroup 0;ok;oifgroup "default"
meta oifgroup != 0;ok;oifgroup != "default"
meta oifgroup "default";ok;oifgroup "default"
meta oifgroup != "default";ok;oifgroup != "default"
meta oifgroup {"default", 11};ok;oifgroup {"default", 11}
meta oifgroup != {"default", 11};ok;oifgroup != {"default", 11}
meta oifgroup { 11,33};ok;oifgroup { 11,33}
meta oifgroup {11-33, 44-55};ok;oifgroup {11-33, 44-55}
meta oifgroup != { 11,33};ok;oifgroup != { 11,33}
meta oifgroup != {11-33, 44-55};ok;oifgroup != {11-33, 44-55}

meta cgroup 1048577;ok
meta cgroup != 1048577;ok
meta cgroup { 1048577, 1048578 };ok
meta cgroup != { 1048577, 1048578};ok
meta cgroup 1048577-1048578;ok
meta cgroup != 1048577-1048578;ok

meta iif . meta oif { "lo" . "lo" };ok;iif . oif { "lo" . "lo" }
meta iif . meta oif . meta mark { "lo" . "lo" . 0x0000000a };ok;iif . oif . meta mark { "lo" . "lo" . 0x0000000a }
meta iif . meta oif vmap { "lo" . "lo" : drop };ok;iif . oif vmap { "lo" . "lo" : drop }

meta random eq 1;ok;meta random 1
meta random gt 1000000;ok;meta random > 1000000

meta time "1970-05-23 21:07:14" drop;ok
meta time 12341234 drop;ok;meta time "1970-05-23 22:07:14" drop
meta time "2019-06-21 17:00:00" drop;ok
meta time "2019-07-01 00:00:00" drop;ok
meta time "2019-07-01 00:01:00" drop;ok
meta time "2019-07-01 00:00:01" drop;ok
meta time < "2022-07-01 11:00:00" accept;ok
meta time > "2022-07-01 11:00:00" accept;ok
meta day "Saturday" drop;ok
meta day 6 drop;ok;meta day "Saturday" drop
meta day "Satturday" drop;fail
meta hour "17:00" drop;ok
meta hour "17:00:00" drop;ok;meta hour "17:00" drop
meta hour "17:00:01" drop;ok
meta hour "00:00" drop;ok
meta hour "00:01" drop;ok
meta hour "01:01" drop;ok
meta hour "02:02" drop;ok
meta hour "03:03" drop;ok
meta hour "23:59:60" drop;ok;meta hour "00:00" drop
meta hour "00:00"-"02:02" drop;ok
meta hour "01:01"-"03:03" drop;ok
meta hour "02:02"-"04:04" drop;ok
meta hour "21:00"-"02:00" drop;ok
time < "2022-07-01 11:00:00" accept;ok;meta time < "2022-07-01 11:00:00" accept
time > "2022-07-01 11:00:00" accept;ok;meta time > "2022-07-01 11:00:00" accept

meta time "meh";fail
meta hour "24:00" drop;fail
meta day 7 drop;fail

meta mark set vlan id map { 1 : 0x00000001, 4095 : 0x00004095 };ok
!map1 typeof vlan id : meta mark;ok
meta mark set vlan id map @map1;ok

meta mark set meta mark | iif | meta cpu;ok
