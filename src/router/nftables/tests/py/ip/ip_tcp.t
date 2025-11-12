:input;type filter hook input priority 0

*ip;test-ip;input

# can remove ip dependency -- its redundant in ip family
ip protocol tcp tcp dport 22;ok;tcp dport 22

# but not here
ip protocol tcp meta mark set 1 tcp dport 22;ok;ip protocol 6 meta mark set 0x00000001 tcp dport 22
