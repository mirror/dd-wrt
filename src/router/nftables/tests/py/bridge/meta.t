:input;type filter hook input priority 0

*bridge;test-bridge;input

meta obrname "br0";ok
meta ibrname "br0";ok
meta ibrvproto vlan;ok;meta ibrvproto 8021q
meta ibrpvid 100;ok

meta protocol ip udp dport 67;ok
meta protocol ip6 udp dport 67;ok

meta broute set 1;fail
