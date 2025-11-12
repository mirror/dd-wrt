:input;type filter hook input priority 0

*ip6;test-ip6;input

exthdr hbh exists;ok
exthdr rt exists;ok
exthdr frag exists;ok
exthdr dst exists;ok
exthdr mh exists;ok

exthdr hbh missing;ok

exthdr hbh == exists;ok;exthdr hbh exists
exthdr hbh == missing;ok;exthdr hbh missing
exthdr hbh != exists;ok
exthdr hbh != missing;ok

exthdr hbh 1;ok;exthdr hbh exists
exthdr hbh 0;ok;exthdr hbh missing
