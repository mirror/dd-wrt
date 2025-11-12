:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

tcp dport set {1, 2, 3};fail

tcp dport 22;ok
tcp dport != 233;ok
tcp dport 33-45;ok
tcp dport != 33-45;ok
tcp dport { 33, 55, 67, 88};ok
tcp dport != { 33, 55, 67, 88};ok
tcp dport {telnet, http, https} accept;ok;tcp dport { 443, 23, 80} accept
tcp dport vmap { 22 : accept, 23 : drop };ok
tcp dport vmap { 25:accept, 28:drop };ok
tcp dport { 22, 53, 80, 110 };ok
tcp dport != { 22, 53, 80, 110 };ok
# BUG: invalid expression type set
# nft: src/evaluate.c:975: expr_evaluate_relational: Assertion '0' failed.

tcp sport 22;ok
tcp sport != 233;ok
tcp sport 33-45;ok
tcp sport != 33-45;ok
tcp sport { 33, 55, 67, 88};ok
tcp sport != { 33, 55, 67, 88};ok
tcp sport vmap { 25:accept, 28:drop };ok

tcp sport 8080 drop;ok
tcp sport 1024 tcp dport 22;ok
tcp sport 1024 tcp dport 22 tcp sequence 0;ok

tcp sequence 0 tcp sport 1024 tcp dport 22;ok
tcp sequence 0 tcp sport { 1024, 1022} tcp dport 22;ok;tcp sequence 0 tcp sport { 1022, 1024} tcp dport 22

tcp sequence 22;ok
tcp sequence != 233;ok
tcp sequence 33-45;ok
tcp sequence != 33-45;ok
tcp sequence { 33, 55, 67, 88};ok
tcp sequence != { 33, 55, 67, 88};ok

tcp ackseq 42949672 drop;ok
tcp ackseq 22;ok
tcp ackseq != 233;ok
tcp ackseq 33-45;ok
tcp ackseq != 33-45;ok
tcp ackseq { 33, 55, 67, 88};ok
tcp ackseq != { 33, 55, 67, 88};ok

- tcp doff 22;ok
- tcp doff != 233;ok
- tcp doff 33-45;ok
- tcp doff != 33-45;ok
- tcp doff { 33, 55, 67, 88};ok
- tcp doff != { 33, 55, 67, 88};ok

# BUG reserved
# BUG: It is accepted but it is not shown then. tcp reserver

tcp flags { fin, syn, rst, psh, ack, urg, ecn, cwr} drop;ok
tcp flags != { fin, urg, ecn, cwr} drop;ok
tcp flags cwr;ok
tcp flags != cwr;ok
tcp flags == syn;ok
tcp flags fin,syn / fin,syn;ok;tcp flags & (fin | syn) == fin | syn
tcp flags != syn / fin,syn;ok;tcp flags & (fin | syn) != syn
tcp flags & syn != 0;ok;tcp flags syn
tcp flags & syn == 0;ok;tcp flags ! syn
tcp flags & (syn | ack) != 0;ok;tcp flags syn,ack
tcp flags & (syn | ack) == 0;ok;tcp flags ! syn,ack
# it should be possible to transform this to: tcp flags syn
tcp flags & syn == syn;ok
tcp flags & syn != syn;ok
tcp flags & (fin | syn | rst | ack) syn;ok;tcp flags & (fin | syn | rst | ack) == syn
tcp flags & (fin | syn | rst | ack) == syn;ok
tcp flags & (fin | syn | rst | ack) != syn;ok
tcp flags & (fin | syn | rst | ack) == syn | ack;ok
tcp flags & (fin | syn | rst | ack) != syn | ack;ok
tcp flags & (syn | ack) == syn | ack;ok
tcp flags & (fin | syn | rst | psh | ack | urg | ecn | cwr) == fin | syn | rst | psh | ack | urg | ecn | cwr;ok;tcp flags == 0xff
tcp flags { syn, syn | ack };ok
tcp flags & (fin | syn | rst | psh | ack | urg) == { fin, ack, psh | ack, fin | psh | ack };ok
tcp flags ! fin,rst;ok
tcp flags & (fin | syn | rst | ack) ! syn;fail

tcp window 22222;ok
tcp window 22;ok
tcp window != 233;ok
tcp window 33-45;ok
tcp window != 33-45;ok
tcp window { 33, 55, 67, 88};ok
tcp window != { 33, 55, 67, 88};ok

tcp checksum 22;ok
tcp checksum != 233;ok
tcp checksum 33-45;ok
tcp checksum != 33-45;ok
tcp checksum { 33, 55, 67, 88};ok
tcp checksum != { 33, 55, 67, 88};ok

tcp urgptr 1234 accept;ok
tcp urgptr 22;ok
tcp urgptr != 233;ok
tcp urgptr 33-45;ok
tcp urgptr != 33-45;ok
tcp urgptr { 33, 55, 67, 88};ok
tcp urgptr != { 33, 55, 67, 88};ok

tcp doff 8;ok
