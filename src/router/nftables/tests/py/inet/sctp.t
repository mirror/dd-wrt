:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

sctp sport 23;ok
sctp sport != 23;ok
sctp sport 23-44;ok
sctp sport != 23-44;ok
sctp sport { 23, 24, 25};ok
sctp sport != { 23, 24, 25};ok

sctp dport 23;ok
sctp dport != 23;ok
sctp dport 23-44;ok
sctp dport != 23-44;ok
sctp dport { 23, 24, 25};ok
sctp dport != { 23, 24, 25};ok

sctp checksum 1111;ok
sctp checksum != 11;ok
sctp checksum 21-333;ok
sctp checksum != 32-111;ok
sctp checksum { 22, 33, 44};ok
sctp checksum != { 22, 33, 44};ok

sctp vtag 22;ok
sctp vtag != 233;ok
sctp vtag 33-45;ok
sctp vtag != 33-45;ok
sctp vtag {33, 55, 67, 88};ok
sctp vtag != {33, 55, 67, 88};ok

# assert all chunk types are recognized
sctp chunk data exists;ok
sctp chunk init exists;ok
sctp chunk init-ack exists;ok
sctp chunk sack exists;ok
sctp chunk heartbeat exists;ok
sctp chunk heartbeat-ack exists;ok
sctp chunk abort exists;ok
sctp chunk shutdown exists;ok
sctp chunk shutdown-ack exists;ok
sctp chunk error exists;ok
sctp chunk cookie-echo exists;ok
sctp chunk cookie-ack exists;ok
sctp chunk ecne exists;ok
sctp chunk cwr exists;ok
sctp chunk shutdown-complete exists;ok
sctp chunk asconf-ack exists;ok
sctp chunk forward-tsn exists;ok
sctp chunk asconf exists;ok

# test common header fields in random chunk types
sctp chunk data type 0;ok
sctp chunk init flags 23;ok
sctp chunk init-ack length 42;ok

# test one custom field in every applicable chunk type
sctp chunk data stream 1337;ok
sctp chunk init initial-tsn 5;ok
sctp chunk init-ack num-outbound-streams 3;ok
sctp chunk sack a-rwnd 1;ok
sctp chunk shutdown cum-tsn-ack 65535;ok
sctp chunk ecne lowest-tsn 5;ok
sctp chunk cwr lowest-tsn 8;ok
sctp chunk asconf-ack seqno 12345;ok
sctp chunk forward-tsn new-cum-tsn 31337;ok
sctp chunk asconf seqno 12345;ok
