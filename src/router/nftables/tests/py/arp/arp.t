# filter chains available are: input, output, forward
:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*arp;test-arp;input
*netdev;test-netdev;ingress,egress

arp htype 1;ok
arp htype != 1;ok
arp htype 22;ok
arp htype != 233;ok
arp htype 33-45;ok
arp htype != 33-45;ok
arp htype { 33, 55, 67, 88};ok
arp htype != { 33, 55, 67, 88};ok

arp ptype 0x0800;ok;arp ptype ip

arp hlen 22;ok
arp hlen != 233;ok
arp hlen 33-45;ok
arp hlen != 33-45;ok
arp hlen { 33, 55, 67, 88};ok
arp hlen != { 33, 55, 67, 88};ok

arp plen 22;ok
arp plen != 233;ok
arp plen 33-45;ok
arp plen != 33-45;ok
arp plen { 33, 55, 67, 88};ok
arp plen != { 33, 55, 67, 88};ok

arp operation {nak, inreply, inrequest, rreply, rrequest, reply, request};ok
arp operation != {nak, inreply, inrequest, rreply, rrequest, reply, request};ok
arp operation 1-2;ok
arp operation request;ok
arp operation reply;ok
arp operation rrequest;ok
arp operation rreply;ok
arp operation inrequest;ok
arp operation inreply;ok
arp operation nak;ok
arp operation != request;ok
arp operation != reply;ok
arp operation != rrequest;ok
arp operation != rreply;ok
arp operation != inrequest;ok
arp operation != inreply;ok
arp operation != nak;ok

arp saddr ip 1.2.3.4;ok
arp daddr ip 4.3.2.1;ok
arp saddr ether aa:bb:cc:aa:bb:cc;ok
arp daddr ether aa:bb:cc:aa:bb:cc;ok

arp saddr ip 192.168.1.1 arp daddr ether fe:ed:00:c0:ff:ee;ok
arp daddr ether fe:ed:00:c0:ff:ee arp saddr ip 192.168.1.1;ok;arp saddr ip 192.168.1.1 arp daddr ether fe:ed:00:c0:ff:ee

meta iifname "invalid" arp ptype 0x0800 arp htype 1 arp hlen 6 arp plen 4 @nh,192,32 0xc0a88f10 @nh,144,48 set 0x112233445566;ok;iifname "invalid" arp htype 1 arp ptype ip arp hlen 6 arp plen 4 arp daddr ip 192.168.143.16 arp daddr ether set 11:22:33:44:55:66
