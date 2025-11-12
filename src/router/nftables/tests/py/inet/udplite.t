:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

udplite sport 80 accept;ok
udplite sport != 60 accept;ok
udplite sport 50-70 accept;ok
udplite sport != 50-60 accept;ok
udplite sport { 49, 50} drop;ok
udplite sport != { 49, 50} accept;ok

udplite dport 80 accept;ok
udplite dport != 60 accept;ok
udplite dport 70-75 accept;ok
udplite dport != 50-60 accept;ok
udplite dport { 49, 50} drop;ok
udplite dport != { 49, 50} accept;ok

- udplite csumcov 6666;ok
- udplite csumcov != 6666;ok
- udplite csumcov 50-65 accept;ok
- udplite csumcov != 50-65 accept;ok
- udplite csumcov { 50, 65} accept;ok
- udplite csumcov != { 50, 65} accept;ok

udplite checksum 6666 drop;ok
udplite checksum != { 444, 555} accept;ok
udplite checksum 22;ok
udplite checksum != 233;ok
udplite checksum 33-45;ok
udplite checksum != 33-45;ok
udplite checksum { 33, 55, 67, 88};ok
udplite checksum != { 33, 55, 67, 88};ok
