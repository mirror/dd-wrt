:postrouting;type nat hook postrouting priority 0

*ip;test-ip4;postrouting

# nf_nat flags combination
udp dport 53 masquerade;ok
udp dport 53 masquerade random;ok
udp dport 53 masquerade random,persistent;ok
udp dport 53 masquerade random,persistent,fully-random;ok;udp dport 53 masquerade random,fully-random,persistent
udp dport 53 masquerade random,fully-random;ok
udp dport 53 masquerade random,fully-random,persistent;ok
udp dport 53 masquerade persistent;ok
udp dport 53 masquerade persistent,random;ok;udp dport 53 masquerade random,persistent
udp dport 53 masquerade persistent,random,fully-random;ok;udp dport 53 masquerade random,fully-random,persistent
udp dport 53 masquerade persistent,fully-random;ok;udp dport 53 masquerade fully-random,persistent
udp dport 53 masquerade persistent,fully-random,random;ok;udp dport 53 masquerade random,fully-random,persistent

# using ports
ip protocol 6 masquerade to :1024;ok
ip protocol 6 masquerade to :1024-2048;ok

# masquerade is a terminal statement
tcp dport 22 masquerade counter packets 0 bytes 0 accept;fail
tcp sport 22 masquerade accept;fail
ip saddr 10.1.1.1 masquerade drop;fail

# masquerade with sets
tcp dport { 1,2,3,4,5,6,7,8,101,202,303,1001,2002,3003} masquerade;ok
ip daddr 10.0.0.0-10.2.3.4 udp dport 53 counter masquerade;ok
iifname "eth0" ct state established,new tcp dport vmap {22 : drop, 222 : drop } masquerade;ok
