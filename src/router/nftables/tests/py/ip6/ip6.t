:input;type filter hook input priority 0

*ip6;test-ip6;input
*inet;test-inet;input

# BUG: Problem with version, priority
# <cmdline>:1:1-38: Error: Could not process rule: Invalid argument
# add rule ip6 test6 input ip6 version 1
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- ip6 version 6;ok

ip6 dscp cs1;ok
ip6 dscp != cs1;ok
ip6 dscp 0x38;ok;ip6 dscp cs7
ip6 dscp != 0x20;ok;ip6 dscp != cs4
ip6 dscp {cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7, af11, af12, af13, af21, af22, af23, af31, af32, af33, af41, af42, af43, ef};ok
ip6 dscp vmap { 0x04 : accept, 0x3f : continue } counter;ok

!map1 type dscp : mark;ok
meta mark set ip6 dscp map @map1;ok
!map2 type dscp . ipv6_addr : mark;ok
meta mark set ip6 dscp . ip6 daddr map @map2;ok
!map3 type dscp : mark;ok
ip6 dscp @map3;ok
!map4 type dscp . ipv6_addr : mark;ok
ip6 dscp . ip6 daddr @map4;ok

ip6 flowlabel 22;ok
ip6 flowlabel != 233;ok
- ip6 flowlabel 33-45;ok
- ip6 flowlabel != 33-45;ok
ip6 flowlabel { 33, 55, 67, 88};ok
# BUG ip6 flowlabel { 5046528, 2883584, 13522432 }
ip6 flowlabel != { 33, 55, 67, 88};ok
ip6 flowlabel vmap { 0 : accept, 2 : continue };ok

ip6 length 22;ok
ip6 length != 233;ok
ip6 length 33-45;ok
ip6 length != 33-45;ok
ip6 length { 33, 55, 67, 88};ok
ip6 length != {33, 55, 67, 88};ok

ip6 nexthdr {udp, ah, comp, udplite, tcp, dccp, sctp};ok;ip6 nexthdr { 132, 51, 108, 136, 17, 33, 6}
ip6 nexthdr {esp, ah, comp, udp, udplite, tcp, dccp, sctp, icmpv6};ok;ip6 nexthdr { 6, 136, 108, 33, 50, 17, 132, 58, 51}
ip6 nexthdr != {esp, ah, comp, udp, udplite, tcp, dccp, sctp, icmpv6};ok;ip6 nexthdr != { 6, 136, 108, 33, 50, 17, 132, 58, 51}
ip6 nexthdr esp;ok;ip6 nexthdr 50
ip6 nexthdr != esp;ok;ip6 nexthdr != 50
ip6 nexthdr 33-44;ok
ip6 nexthdr != 33-44;ok

ip6 hoplimit 1;ok
ip6 hoplimit != 233;ok
ip6 hoplimit 33-45;ok
ip6 hoplimit != 33-45;ok
ip6 hoplimit {33, 55, 67, 88};ok
ip6 hoplimit != {33, 55, 67, 88};ok

# from src/scanner.l
# v680		(({hex4}:){7}{hex4})
ip6 saddr 1234:1234:1234:1234:1234:1234:1234:1234;ok
# v670		((:)(:{hex4}{7}))
ip6 saddr ::1234:1234:1234:1234:1234:1234:1234;ok;ip6 saddr 0:1234:1234:1234:1234:1234:1234:1234
# v671		((({hex4}:){1})(:{hex4}{6}))
ip6 saddr 1234::1234:1234:1234:1234:1234:1234;ok;ip6 saddr 1234:0:1234:1234:1234:1234:1234:1234
# v672		((({hex4}:){2})(:{hex4}{5}))
ip6 saddr 1234:1234::1234:1234:1234:1234:1234;ok;ip6 saddr 1234:1234:0:1234:1234:1234:1234:1234
ip6 saddr 1234:1234:0:1234:1234:1234:1234:1234;ok
# v673		((({hex4}:){3})(:{hex4}{4}))
ip6 saddr 1234:1234:1234::1234:1234:1234:1234;ok;ip6 saddr 1234:1234:1234:0:1234:1234:1234:1234
# v674		((({hex4}:){4})(:{hex4}{3}))
ip6 saddr 1234:1234:1234:1234:0:1234:1234:1234;ok
# v675		((({hex4}:){5})(:{hex4}{2}))
ip6 saddr 1234:1234:1234:1234:1234::1234:1234;ok;ip6 saddr 1234:1234:1234:1234:1234:0:1234:1234
# v676		((({hex4}:){6})(:{hex4}{1}))
ip6 saddr 1234:1234:1234:1234:1234:1234:0:1234;ok
# v677		((({hex4}:){7})(:))
ip6 saddr 1234:1234:1234:1234:1234:1234:1234::;ok;ip6 saddr 1234:1234:1234:1234:1234:1234:1234:0
# v67		({v670}|{v671}|{v672}|{v673}|{v674}|{v675}|{v676}|{v677})
# v660		((:)(:{hex4}{6}))
ip6 saddr ::1234:1234:1234:1234:1234:1234;ok
# v661		((({hex4}:){1})(:{hex4}{5}))
ip6 saddr 1234::1234:1234:1234:1234:1234;ok
# v662		((({hex4}:){2})(:{hex4}{4}))
ip6 saddr 1234:1234::1234:1234:1234:1234;ok
# v663		((({hex4}:){3})(:{hex4}{3}))
ip6 saddr 1234:1234:1234::1234:1234:1234;ok
# v664		((({hex4}:){4})(:{hex4}{2}))
ip6 saddr 1234:1234:1234:1234::1234:1234;ok
# v665		((({hex4}:){5})(:{hex4}{1}))
ip6 saddr 1234:1234:1234:1234:1234::1234;ok
# v666		((({hex4}:){6})(:))
ip6 saddr 1234:1234:1234:1234:1234:1234::;ok
# v66		({v660}|{v661}|{v662}|{v663}|{v664}|{v665}|{v666})
# v650		((:)(:{hex4}{5}))
ip6 saddr ::1234:1234:1234:1234:1234;ok
# v651		((({hex4}:){1})(:{hex4}{4}))
ip6 saddr 1234::1234:1234:1234:1234;ok
# v652		((({hex4}:){2})(:{hex4}{3}))
ip6 saddr 1234:1234::1234:1234:1234;ok
# v653		((({hex4}:){3})(:{hex4}{2}))
ip6 saddr 1234:1234:1234::1234:1234;ok
# v654		((({hex4}:){4})(:{hex4}{1}))
ip6 saddr 1234:1234:1234:1234::1234;ok
# v655		((({hex4}:){5})(:))
ip6 saddr 1234:1234:1234:1234:1234::;ok
# v65		({v650}|{v651}|{v652}|{v653}|{v654}|{v655})
# v640		((:)(:{hex4}{4}))
ip6 saddr ::1234:1234:1234:1234;ok
# v641		((({hex4}:){1})(:{hex4}{3}))
ip6 saddr 1234::1234:1234:1234;ok
# v642		((({hex4}:){2})(:{hex4}{2}))
ip6 saddr 1234:1234::1234:1234;ok
# v643		((({hex4}:){3})(:{hex4}{1}))
ip6 saddr 1234:1234:1234::1234;ok
# v644		((({hex4}:){4})(:))
ip6 saddr 1234:1234:1234:1234::;ok
# v64		({v640}|{v641}|{v642}|{v643}|{v644})
# v630		((:)(:{hex4}{3}))
ip6 saddr ::1234:1234:1234;ok
# v631		((({hex4}:){1})(:{hex4}{2}))
ip6 saddr 1234::1234:1234;ok
# v632		((({hex4}:){2})(:{hex4}{1}))
ip6 saddr 1234:1234::1234;ok
# v633		((({hex4}:){3})(:))
ip6 saddr 1234:1234:1234::;ok
# v63		({v630}|{v631}|{v632}|{v633})
# v620		((:)(:{hex4}{2}))
ip6 saddr ::1234:1234;ok;ip6 saddr ::18.52.18.52
# v621		((({hex4}:){1})(:{hex4}{1}))
ip6 saddr 1234::1234;ok
# v622		((({hex4}:){2})(:))
ip6 saddr 1234:1234::;ok
# v62		({v620}|{v621}|{v622})
# v610		((:)(:{hex4}{1}))
ip6 saddr ::1234;ok
# v611		((({hex4}:){1})(:))
ip6 saddr 1234::;ok
# v61		({v610}|{v611})
# v60		(::)
ip6 saddr ::/64;ok
ip6 saddr ::1 ip6 daddr ::2;ok

ip6 daddr != {::1234:1234:1234:1234:1234:1234:1234, 1234:1234::1234:1234:1234:1234:1234 };ok;ip6 daddr != {0:1234:1234:1234:1234:1234:1234:1234, 1234:1234:0:1234:1234:1234:1234:1234}
ip6 daddr != ::1234:1234:1234:1234:1234:1234:1234-1234:1234::1234:1234:1234:1234:1234;ok;ip6 daddr != 0:1234:1234:1234:1234:1234:1234:1234-1234:1234:0:1234:1234:1234:1234:1234

# limit impact to lo
iif "lo" ip6 daddr set ::1;ok
iif "lo" ip6 hoplimit set 1;ok
iif "lo" ip6 dscp set af42;ok
iif "lo" ip6 dscp set 63;ok;iif "lo" ip6 dscp set 0x3f
iif "lo" ip6 ecn set ect0;ok
iif "lo" ip6 ecn set ce;ok

iif "lo" ip6 flowlabel set 0;ok
iif "lo" ip6 flowlabel set 12345;ok
iif "lo" ip6 flowlabel set 0xfffff;ok;iif "lo" ip6 flowlabel set 1048575

iif "lo" ip6 ecn set 4;fail
iif "lo" ip6 dscp set 64;fail
iif "lo" ip6 flowlabel set 1048576;fail
