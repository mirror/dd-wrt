:input;type filter hook input priority 0
:ingress;type filter hook ingress device lo priority 0
:egress;type filter hook egress device lo priority 0

*ip6;test-ip6;input
*inet;test-inet;input
*netdev;test-netdev;ingress,egress

ip6 saddr vmap { abcd::3 : accept };ok
ip6 saddr 1234:1234:1234:1234:1234:1234:1234:1234:1234;fail

# Ipv6 address combinations
# from src/scanner.l
ip6 saddr vmap { 1234:1234:1234:1234:1234:1234:1234:1234 : accept};ok
ip6 saddr vmap { ::1234:1234:1234:1234:1234:1234:1234 : accept};ok;ip6 saddr vmap { 0:1234:1234:1234:1234:1234:1234:1234 : accept}
ip6 saddr vmap { 1234::1234:1234:1234:1234:1234:1234 : accept};ok;ip6 saddr vmap { 1234:0:1234:1234:1234:1234:1234:1234 : accept}
ip6 saddr vmap { 1234:1234::1234:1234:1234:1234:1234 : accept};ok;ip6 saddr vmap { 1234:1234:0:1234:1234:1234:1234:1234 : accept}
ip6 saddr vmap { 1234:1234:1234::1234:1234:1234:1234 : accept};ok;ip6 saddr vmap { 1234:1234:1234:0:1234:1234:1234:1234 : accept}
ip6 saddr vmap { 1234:1234:1234:1234::1234:1234:1234 : accept};ok;ip6 saddr vmap { 1234:1234:1234:1234:0:1234:1234:1234 : accept}
ip6 saddr vmap { 1234:1234:1234:1234:1234::1234:1234 : accept};ok;ip6 saddr vmap { 1234:1234:1234:1234:1234:0:1234:1234 : accept}
ip6 saddr vmap { 1234:1234:1234:1234:1234:1234::1234 : accept};ok;ip6 saddr vmap { 1234:1234:1234:1234:1234:1234:0:1234 : accept}
ip6 saddr vmap { 1234:1234:1234:1234:1234:1234:1234:: : accept};ok;ip6 saddr vmap { 1234:1234:1234:1234:1234:1234:1234:0 : accept}
ip6 saddr vmap { ::1234:1234:1234:1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234::1234:1234:1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234::1234:1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234::1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234:1234::1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234:1234:1234::1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234:1234:1234:1234:: : accept};ok
ip6 saddr vmap { ::1234:1234:1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234::1234:1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234::1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234::1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234:1234::1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234:1234:1234:: : accept};ok
ip6 saddr vmap { ::1234:1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234::1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234::1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234::1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234:1234:: : accept};ok
ip6 saddr vmap { ::1234:1234:1234 : accept};ok
ip6 saddr vmap { 1234::1234:1234 : accept};ok
ip6 saddr vmap { 1234:1234::1234 : accept};ok
ip6 saddr vmap { 1234:1234:1234:: : accept};ok
ip6 saddr vmap { ::1234:1234 : accept};ok;ip6 saddr vmap { ::18.52.18.52 : accept}
ip6 saddr vmap { 1234::1234 : accept};ok
ip6 saddr vmap { 1234:1234:: : accept};ok
ip6 saddr vmap { ::1234 : accept};ok
ip6 saddr vmap { 1234:: : accept};ok
ip6 saddr vmap { ::/64 : accept};ok

ip6 saddr vmap {1234:1234:1234:1234:1234:1234:aaaa:: : accept, ::aaaa : drop};ok;ip6 saddr vmap {1234:1234:1234:1234:1234:1234:aaaa:0 : accept, ::aaaa : drop}
ip6 saddr vmap {1234:1234:1234:1234:1234:1234:aaaa:::accept, ::bbbb : drop};ok;ip6 saddr vmap {1234:1234:1234:1234:1234:1234:aaaa:0 : accept, ::bbbb : drop}
ip6 saddr vmap {1234:1234:1234:1234:1234:1234:aaaa:::accept,::cccc : drop};ok;ip6 saddr vmap {1234:1234:1234:1234:1234:1234:aaaa:0 : accept, ::cccc : drop}
ip6 saddr vmap {1234:1234:1234:1234:1234:1234:aaaa:::accept,::dddd: drop};ok;ip6 saddr vmap {1234:1234:1234:1234:1234:1234:aaaa:0 : accept, ::dddd: drop}

# rule without comma:
filter-input ip6 saddr vmap { 1234:1234:1234:1234:1234:1234:bbbb:::accept::adda : drop};fail
