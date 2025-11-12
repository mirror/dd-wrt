# filter chains available are: input, output, forward, prerouting, postrouting
:filter-input;type filter hook input priority 0
:filter-pre;type filter hook prerouting priority 0
:filter-forw;type filter hook forward priority 0
:filter-out;type filter hook output priority 0
:filter-post;type filter hook postrouting priority 0
# nat chains available are: input, output, prerouting, postrouting
:nat-input-t;type nat hook input priority 0
:nat-pre-t;type nat hook prerouting priority 0
:nat-out-t;type nat hook output priority 0
:nat-post-t;type nat hook postrouting priority 0
# route chain available are: output
:route-out-t;type route hook output priority 0

*ip;test-ip4;filter-input,filter-pre,filter-forw,filter-out,filter-post,nat-input-t,nat-pre-t,nat-out-t,nat-post-t,route-out-t
