:output;type filter hook output priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;output
*ip6;test-ip6;output
*inet;test-inet;output
*arp;test-arp;output
*bridge;test-bridge;output
*netdev;test-netdev;ingress,egress

limit rate 400/minute;ok;limit rate 400/minute burst 5 packets
limit rate 20/second;ok;limit rate 20/second burst 5 packets
limit rate 400/hour;ok;limit rate 400/hour burst 5 packets
limit rate 40/day;ok;limit rate 40/day burst 5 packets
limit rate 400/week;ok;limit rate 400/week burst 5 packets
limit rate 1023/second burst 10 packets;ok
limit rate 1023/second burst 10 bytes;fail

limit rate 1 kbytes/second;ok
limit rate 2 kbytes/second;ok
limit rate 1025 kbytes/second;ok
limit rate 1023 mbytes/second;ok
limit rate 10230 mbytes/second;ok
limit rate 512 kbytes/second burst 5 packets;fail

limit rate 1 bytes / second;ok;limit rate 1 bytes/second
limit rate 1 kbytes / second;ok;limit rate 1 kbytes/second
limit rate 1 mbytes / second;ok;limit rate 1 mbytes/second
limit rate 1 gbytes / second;fail

limit rate 1025 bytes/second burst 512 bytes;ok
limit rate 1025 kbytes/second burst 1023 kbytes;ok
limit rate 1025 mbytes/second burst 1025 kbytes;ok

limit rate over 400/minute;ok;limit rate over 400/minute burst 5 packets
limit rate over 20/second;ok;limit rate over 20/second burst 5 packets
limit rate over 400/hour;ok;limit rate over 400/hour burst 5 packets
limit rate over 40/day;ok;limit rate over 40/day burst 5 packets
limit rate over 400/week;ok;limit rate over 400/week burst 5 packets
limit rate over 1023/second burst 10 packets;ok

limit rate over 1 kbytes/second;ok
limit rate over 2 kbytes/second;ok
limit rate over 1025 kbytes/second;ok
limit rate over 1023 mbytes/second;ok
limit rate over 10230 mbytes/second;ok

limit rate over 1025 bytes/second burst 512 bytes;ok
limit rate over 1025 kbytes/second burst 1023 kbytes;ok
limit rate over 1025 mbytes/second burst 1025 kbytes;ok

limit rate over 1025bytes/second burst 512bytes;ok;limit rate over 1025 bytes/second burst 512 bytes
limit rate over 1025bytes /second burst 512bytes;ok;limit rate over 1025 bytes/second burst 512 bytes
limit rate over 1025bytes/ second burst 512bytes;ok;limit rate over 1025 bytes/second burst 512 bytes
limit rate over 1025bytes / second burst 512bytes;ok;limit rate over 1025 bytes/second burst 512 bytes
limit rate over 1025 bytes / second burst 512bytes;ok;limit rate over 1025 bytes/second burst 512 bytes
