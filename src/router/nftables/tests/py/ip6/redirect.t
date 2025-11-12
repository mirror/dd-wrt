:output;type nat hook output priority 0

*ip6;test-ip6;output

# with no arguments
redirect;ok
udp dport 954 redirect;ok
ip6 saddr fe00::cafe counter redirect;ok

# nf_nat flags combination
udp dport 53 redirect random;ok
udp dport 53 redirect random,persistent;ok
udp dport 53 redirect random,persistent,fully-random;ok;udp dport 53 redirect random,fully-random,persistent
udp dport 53 redirect random,fully-random;ok
udp dport 53 redirect random,fully-random,persistent;ok
udp dport 53 redirect persistent;ok
udp dport 53 redirect persistent,random;ok;udp dport 53 redirect random,persistent
udp dport 53 redirect persistent,random,fully-random;ok;udp dport 53 redirect random,fully-random,persistent
udp dport 53 redirect persistent,fully-random;ok;udp dport 53 redirect fully-random,persistent
udp dport 53 redirect persistent,fully-random,random;ok;udp dport 53 redirect random,fully-random,persistent

# port specification
udp dport 1234 redirect to :1234;ok
ip6 daddr fe00::cafe udp dport 9998 redirect to :6515;ok
ip6 nexthdr tcp redirect to :100-200;ok;ip6 nexthdr 6 redirect to :100-200
tcp dport 39128 redirect to :993;ok
redirect to :1234;fail
redirect to :12341111;fail

# both port and nf_nat flags
tcp dport 9128 redirect to :993 random;ok
tcp dport 9128 redirect to :993 fully-random,persistent;ok

# nf_nat flags are the last argument
tcp dport 9128 redirect persistent to 123;fail
tcp dport 9128 redirect random,persistent to 123;fail

# redirect is a terminal statement
tcp dport 22 redirect counter packets 0 bytes 0 accept;fail
tcp sport 22 redirect accept;fail
ip6 saddr ::1 redirect drop;fail

# redirect with sets
tcp dport { 1, 2, 3, 4, 5, 6, 7, 8, 101, 202, 303, 1001, 2002, 3003} redirect;ok
ip6 daddr fe00::1-fe00::200 udp dport 53 counter redirect;ok
iifname "eth0" ct state established,new tcp dport vmap {22 : drop, 222 : drop } redirect;ok

# redirect with maps
redirect to :tcp dport map { 22 : 8000, 80 : 8080};ok
