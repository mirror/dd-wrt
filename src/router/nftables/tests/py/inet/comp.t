:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

# BUG: nft: payload.c:88: payload_expr_pctx_update: Assertion `left->payload.base + 1 <= (__PROTO_BASE_MAX - 1)' failed.
- comp nexthdr esp;ok;comp nexthdr 50
comp nexthdr != esp;ok;comp nexthdr != 50

- comp nexthdr {esp, ah, comp, udp, udplite, tcp, tcp, dccp, sctp};ok
# comp flags ## 8-bit field.  Reserved for future use.  MUST be set to zero.

# Bug comp flags: to list. List the decimal value.
comp flags 0x0;ok
comp flags != 0x23;ok
comp flags 0x33-0x45;ok
comp flags != 0x33-0x45;ok
comp flags {0x33, 0x55, 0x67, 0x88};ok
comp flags != {0x33, 0x55, 0x67, 0x88};ok

comp cpi 22;ok
comp cpi != 233;ok
comp cpi 33-45;ok
comp cpi != 33-45;ok
comp cpi {33, 55, 67, 88};ok
comp cpi != {33, 55, 67, 88};ok
