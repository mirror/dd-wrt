# filter chains available are: input, output, forward, forward, prerouting and postrouting.
:filter-input;type filter hook input priority 0
:filter-prer;type filter hook prerouting priority 0
:filter-forw-t;type filter hook forward priority 0
:filter-out-t;type filter hook output priority 0
:filter-post-t;type filter hook postrouting priority 0

# nat chains available are: input, output, forward, prerouting and postrouting.
:nat-input;type nat hook input priority 0
:nat-prerouting;type nat hook prerouting priority 0
:nat-output;type nat hook output priority 0
:nat-postrou;type nat hook postrouting priority 0

# route chain available is output.
:route-out;type route hook output priority 0

*ip6;test-ip6;filter-input,filter-prer,filter-forw-t,filter-out-t,filter-post-t,nat-input,nat-prerouting,nat-output,nat-postrou,route-out
