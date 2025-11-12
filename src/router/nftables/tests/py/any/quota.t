:output;type filter hook output priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;output
*ip6;test-ip6;output
*inet;test-inet;output
*arp;test-arp;output
*bridge;test-bridge;output
*netdev;test-netdev;ingress,egress

quota 1025 bytes;ok
quota 1 kbytes;ok
quota 2 kbytes;ok
quota 1025 kbytes;ok
quota 1023 mbytes;ok
quota 10230 mbytes;ok
quota 1023000 mbytes;ok

quota over 1 kbytes;ok
quota over 2 kbytes;ok
quota over 1025 kbytes;ok
quota over 1023 mbytes;ok
quota over 10230 mbytes;ok
quota over 1023000 mbytes;ok
