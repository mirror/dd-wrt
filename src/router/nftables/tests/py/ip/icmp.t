# BUG: There is a bug with icmp protocol and inet family.
# *inet;test-inet
:input;type filter hook input priority 0

*ip;test-ip4;input

icmp type echo-reply accept;ok
icmp type destination-unreachable accept;ok
icmp type source-quench accept;ok
icmp type redirect accept;ok
icmp type echo-request accept;ok
icmp type time-exceeded accept;ok
icmp type parameter-problem accept;ok
icmp type timestamp-request accept;ok
icmp type timestamp-reply accept;ok
icmp type info-request accept;ok
icmp type info-reply accept;ok
icmp type address-mask-request accept;ok
icmp type address-mask-reply accept;ok
icmp type router-advertisement accept;ok
icmp type router-solicitation accept;ok
icmp type {echo-reply, destination-unreachable, source-quench, redirect, echo-request, time-exceeded, parameter-problem, timestamp-request, timestamp-reply, info-request, info-reply, address-mask-request, address-mask-reply, router-advertisement, router-solicitation} accept;ok
icmp type != {echo-reply, destination-unreachable, source-quench};ok

icmp code 111 accept;ok
icmp code != 111 accept;ok
icmp code 33-55;ok
icmp code != 33-55;ok
icmp code { 2, 4, 54, 33, 56};ok
icmp code != { prot-unreachable, frag-needed, 33, 54, 56};ok;icmp code != { 2, 4, 33, 54, 56}

icmp checksum 12343 accept;ok
icmp checksum != 12343 accept;ok
icmp checksum 11-343 accept;ok
icmp checksum != 11-343 accept;ok
icmp checksum { 1111, 222, 343} accept;ok
icmp checksum != { 1111, 222, 343} accept;ok

icmp id 1245 log;ok;icmp type { echo-reply, echo-request} icmp id 1245 log
icmp id 22;ok;icmp type { echo-reply, echo-request} icmp id 22
icmp id != 233;ok;icmp type { echo-reply, echo-request} icmp id != 233
icmp id 33-45;ok;icmp type { echo-reply, echo-request} icmp id 33-45
icmp id != 33-45;ok;icmp type { echo-reply, echo-request} icmp id != 33-45

icmp id { 22, 34, 333};ok;icmp type { echo-request, echo-reply} icmp id { 22, 34, 333}
icmp id != { 22, 34, 333};ok;icmp type { echo-request, echo-reply} icmp id != { 22, 34, 333}

icmp sequence 22;ok;icmp type { echo-reply, echo-request} icmp sequence 22
icmp sequence != 233;ok;icmp type { echo-reply, echo-request} icmp sequence != 233
icmp sequence 33-45;ok;icmp type { echo-reply, echo-request} icmp sequence 33-45
icmp sequence != 33-45;ok;icmp type { echo-reply, echo-request} icmp sequence != 33-45
icmp sequence { 33, 55, 67, 88};ok;icmp type { echo-request, echo-reply} icmp sequence { 33, 55, 67, 88}
icmp sequence != { 33, 55, 67, 88};ok;icmp type { echo-request, echo-reply} icmp sequence != { 33, 55, 67, 88}
icmp id 1 icmp sequence 2;ok;icmp type { echo-reply, echo-request} icmp id 1 icmp sequence 2
icmp type { echo-reply, echo-request} icmp id 1 icmp sequence 2;ok
icmp type echo-reply icmp id 1;ok

icmp mtu 33;ok
icmp mtu 22-33;ok
icmp mtu 22;ok
icmp mtu != 233;ok
icmp mtu 33-45;ok
icmp mtu != 33-45;ok
icmp mtu { 33, 55, 67, 88};ok
icmp mtu != { 33, 55, 67, 88};ok

icmp gateway 22;ok
icmp gateway != 233;ok
icmp gateway 33-45;ok
icmp gateway != 33-45;ok
icmp gateway { 33, 55, 67, 88};ok
icmp gateway != { 33, 55, 67, 88};ok
icmp gateway != 34;ok
icmp gateway != { 333, 334};ok

icmp code 1 icmp type 2;ok;icmp type 2 icmp code 1
icmp code != 1 icmp type 2 icmp mtu 5;fail
