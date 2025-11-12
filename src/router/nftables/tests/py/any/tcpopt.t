:input;type filter hook input priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input

tcp option eol exists;ok
tcp option nop exists;ok
tcp option maxseg exists;ok
tcp option maxseg length 1;ok
tcp option maxseg size 1;ok
tcp option window length 1;ok
tcp option window count 1;ok
tcp option sack-perm exists;ok
tcp option sack-perm length 1;ok
tcp option sack exists;ok
tcp option sack length 1;ok
tcp option sack left 1;ok
tcp option sack0 left 1;ok;tcp option sack left 1
tcp option sack1 left 1;ok
tcp option sack2 left 1;ok
tcp option sack3 left 1;ok
tcp option sack right 1;ok
tcp option sack0 right 1;ok;tcp option sack right 1
tcp option sack1 right 1;ok
tcp option sack2 right 1;ok
tcp option sack3 right 1;ok
tcp option timestamp exists;ok
tcp option timestamp length 1;ok
tcp option timestamp tsval 1;ok
tcp option timestamp tsecr 1;ok
tcp option 255 missing;ok
tcp option 6 exists;ok
tcp option @255,8,8 255;ok

tcp option foobar;fail
tcp option foo bar;fail
tcp option eol left;fail
tcp option eol left 1;fail
tcp option sack window;fail
tcp option sack window 1;fail
tcp option 256 exists;fail
tcp option @255,8,8 256;fail

tcp option window exists;ok
tcp option window missing;ok

tcp option maxseg size set 1360;ok

tcp option md5sig exists;ok
tcp option fastopen exists;ok
tcp option mptcp exists;ok

tcp option mptcp subtype mp-capable;ok
tcp option mptcp subtype 1;ok;tcp option mptcp subtype mp-join
tcp option mptcp subtype { mp-capable, mp-join, remove-addr, mp-prio, mp-fail, mp-fastclose, mp-tcprst };ok
tcp option mptcp subtype . tcp dport { mp-capable . 10, mp-join . 100, add-addr . 200, remove-addr . 300, mp-prio . 400, mp-fail . 500, mp-fastclose . 600, mp-tcprst . 700 };ok

reset tcp option mptcp;ok
reset tcp option 2;ok;reset tcp option maxseg
reset tcp option 123;ok
reset tcp option meh;fail
reset tcp option 256;fail
