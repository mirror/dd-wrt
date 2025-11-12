:input;type filter hook input priority 0

*ip6;test-ip6;input
# BUG: There is a bug with icmpv6 and inet tables
# *inet;test-inet;input

icmpv6 type destination-unreachable accept;ok
icmpv6 type packet-too-big accept;ok
icmpv6 type time-exceeded accept;ok
icmpv6 type echo-request accept;ok
icmpv6 type echo-reply accept;ok
icmpv6 type mld-listener-query accept;ok
icmpv6 type mld-listener-report accept;ok
icmpv6 type mld-listener-done accept;ok
icmpv6 type mld-listener-reduction accept;ok;icmpv6 type mld-listener-done accept
icmpv6 type nd-router-solicit accept;ok
icmpv6 type nd-router-advert accept;ok
icmpv6 type nd-neighbor-solicit accept;ok
icmpv6 type nd-neighbor-advert accept;ok
icmpv6 type nd-redirect accept;ok
icmpv6 type parameter-problem accept;ok
icmpv6 type router-renumbering accept;ok
icmpv6 type ind-neighbor-solicit accept;ok
icmpv6 type ind-neighbor-advert accept;ok
icmpv6 type mld2-listener-report accept;ok
icmpv6 type {destination-unreachable, time-exceeded, nd-router-solicit} accept;ok
icmpv6 type {router-renumbering, mld-listener-done, time-exceeded, nd-router-solicit} accept;ok
icmpv6 type {mld-listener-query, time-exceeded, nd-router-advert} accept;ok
icmpv6 type != {mld-listener-query, time-exceeded, nd-router-advert} accept;ok

icmpv6 code 4;ok
icmpv6 code 3-66;ok
icmpv6 code {5, 6, 7} accept;ok
icmpv6 code != {policy-fail, reject-route, 7} accept;ok;icmpv6 code != {5, 6, 7} accept

icmpv6 checksum 2222 log;ok
icmpv6 checksum != 2222 log;ok
icmpv6 checksum 222-226;ok
icmpv6 checksum != 222-226;ok
icmpv6 checksum { 222, 226};ok
icmpv6 checksum != { 222, 226};ok

# BUG: icmpv6 parameter-problem, pptr
# [ICMP6HDR_PPTR]         = ICMP6HDR_FIELD("parameter-problem", icmp6_pptr),
# $ sudo nft add rule ip6 test6 input icmpv6 parameter-problem 35
# <cmdline>:1:53-53: Error: syntax error, unexpected end of file
# add rule ip6 test6 input icmpv6 parameter-problem 35
#                                                    ^
# $ sudo nft add rule ip6 test6 input icmpv6 parameter-problem
# <cmdline>:1:26-31: Error: Value 58 exceeds valid range 0-0
# add rule ip6 test6 input icmpv6 parameter-problem
#                         ^^^^^^
# $ sudo nft add rule ip6 test6 input icmpv6 parameter-problem 2-4
# <cmdline>:1:54-54: Error: syntax error, unexpected end of file
# add rule ip6 test6 input icmpv6 parameter-problem 2-4

icmpv6 mtu 22;ok
icmpv6 mtu != 233;ok
icmpv6 mtu 33-45;ok
icmpv6 mtu != 33-45;ok
icmpv6 mtu {33, 55, 67, 88};ok
icmpv6 mtu != {33, 55, 67, 88};ok
icmpv6 type packet-too-big icmpv6 mtu 1280;ok;icmpv6 mtu 1280

icmpv6 id 33-45;ok;icmpv6 type { echo-request, echo-reply} icmpv6 id 33-45
icmpv6 id != 33-45;ok;icmpv6 type { echo-request, echo-reply} icmpv6 id != 33-45
icmpv6 id {33, 55, 67, 88};ok;icmpv6 type { echo-request, echo-reply} icmpv6 id { 33, 55, 67, 88}
icmpv6 id != {33, 55, 67, 88};ok;icmpv6 type { echo-request, echo-reply} icmpv6 id != { 33, 55, 67, 88}

icmpv6 id 1;ok;icmpv6 type { echo-request, echo-reply} icmpv6 id 1
icmpv6 type echo-reply icmpv6 id 65534;ok

icmpv6 sequence 2;ok;icmpv6 type { echo-request, echo-reply} icmpv6 sequence 2
icmpv6 sequence {3, 4, 5, 6, 7} accept;ok;icmpv6 type { echo-request, echo-reply} icmpv6 sequence { 3, 4, 5, 6, 7} accept


icmpv6 sequence {2, 4};ok;icmpv6 type { echo-request, echo-reply} icmpv6 sequence { 2, 4}
icmpv6 sequence != {2, 4};ok;icmpv6 type { echo-request, echo-reply} icmpv6 sequence != { 2, 4}
icmpv6 sequence 2-4;ok;icmpv6 type { echo-request, echo-reply} icmpv6 sequence 2-4
icmpv6 sequence != 2-4;ok;icmpv6 type { echo-request, echo-reply} icmpv6 sequence != 2-4

icmpv6 max-delay 33-45;ok
icmpv6 max-delay != 33-45;ok
icmpv6 max-delay {33, 55, 67, 88};ok
icmpv6 max-delay != {33, 55, 67, 88};ok

icmpv6 type parameter-problem icmpv6 code 0;ok

icmpv6 type mld-listener-query icmpv6 taddr 2001:db8::133;ok
icmpv6 type nd-neighbor-solicit icmpv6 taddr 2001:db8::133;ok
icmpv6 type nd-neighbor-advert icmpv6 taddr 2001:db8::133;ok
icmpv6 taddr 2001:db8::133;ok;icmpv6 type { mld-listener-query, mld-listener-report, mld-listener-done, nd-neighbor-solicit, nd-neighbor-advert, nd-redirect} icmpv6 taddr 2001:db8::133

icmpv6 type { mld-listener-query, mld-listener-report, mld-listener-done, nd-neighbor-solicit, nd-neighbor-advert, nd-redirect} icmpv6 taddr 2001:db8::133;ok
icmpv6 type { nd-neighbor-solicit, nd-neighbor-advert } icmpv6 taddr 2001:db8::133;ok
icmpv6 daddr 2001:db8::133;ok
icmpv6 type nd-redirect icmpv6 daddr 2001:db8::133;ok;icmpv6 daddr 2001:db8::133
