:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*inet;test-inet;input
*netdev;test-netdev;ingress,egress

iifname . ip protocol . th dport vmap { "eth0" . tcp . 22 : accept, "eth1" . udp . 67 : drop };ok;iifname . ip protocol . th dport vmap { "eth0" . 6 . 22 : accept, "eth1" . 17 . 67 : drop }
ip saddr . @ih,32,32 { 1.1.1.1 . 0x14, 2.2.2.2 . 0x1e };ok
udp length . @th,160,128 vmap { 47-63 . 0xe373135363130333131303735353203 : accept };ok
