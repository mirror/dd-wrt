:INPUT,FORWARD,OUTPUT
--log;=;OK
--log-level crit;=;OK
--log-level 1;--log-level alert --log-prefix "";OK
--log-level emerg --log-ip --log-arp --log-ip6;--log-level emerg --log-prefix "" --log-ip --log-arp --log-ip6 -j CONTINUE;OK
--log-level crit --log-ip --log-arp --log-ip6 --log-prefix foo;--log-level crit --log-prefix "foo" --log-ip --log-arp --log-ip6 -j CONTINUE;OK
