:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip;test-ip4;input
*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

esp spi 100;ok
esp spi != 100;ok
esp spi 111-222;ok
esp spi != 111-222;ok
esp spi { 100, 102};ok
esp spi != { 100, 102};ok

esp sequence 22;ok
esp sequence 22-24;ok
esp sequence != 22-24;ok
esp sequence { 22, 24};ok
esp sequence != { 22, 24};ok
