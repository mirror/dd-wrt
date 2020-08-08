:INPUT,FORWARD,OUTPUT
-m icmpv6;;FAIL
-p ipv6-icmp -m icmp6 --icmpv6-type 1/0;=;OK
-p ipv6-icmp -m icmp6 --icmpv6-type 2;=;OK
# cannot use option twice:
-p ipv6-icmp -m icmp6 --icmpv6-type no-route --icmpv6-type packet-too-big;;FAIL
