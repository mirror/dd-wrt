:input;type filter hook input priority 0

*ip;test-ip4;input

# counter
%cnt1 type counter;ok
%cnt2 type counter;ok

ip saddr 192.168.1.3 counter name "cnt2";ok
ip saddr 192.168.1.3 counter name "cnt3";fail
counter name tcp dport map {443 : "cnt1", 80 : "cnt2", 22 : "cnt1"};ok

# quota
%qt1 type quota 25 mbytes;ok
%qt2 type quota over 1 kbytes;ok

ip saddr 192.168.1.3 quota name "qt1";ok
ip saddr 192.168.1.3 quota name "qt3";fail
quota name tcp dport map {443 : "qt1", 80 : "qt2", 22 : "qt1"};ok

# ct helper
%cthelp1 type ct helper { type "ftp" protocol tcp; };ok
%cthelp2 type ct helper { type "ftp" protocol tcp; l3proto ip6; };fail

ct helper set "cthelp1";ok
ct helper set tcp dport map {21 : "cthelp1", 2121 : "cthelp1" };ok

# limit
%lim1 type limit rate 400/minute;ok
%lim2 type limit rate over 1024 bytes/second burst 512 bytes;ok

ip saddr 192.168.1.3 limit name "lim1";ok
ip saddr 192.168.1.3 limit name "lim3";fail
limit name tcp dport map {443 : "lim1", 80 : "lim2", 22 : "lim1"};ok

# ct timeout
%cttime1 type ct timeout { protocol tcp; policy = { established:122 } ;};ok
%cttime2 type ct timeout { protocol udp; policy = { syn_sent:122 } ;};fail
%cttime3 type ct timeout { protocol tcp; policy = { established:132, close:16, close_wait:16 } ; l3proto ip ;};ok
%cttime4 type ct timeout { protocol udp; policy = { replied:14, unreplied:19 } ;};ok
%cttime5 type ct timeout {protocol tcp; policy = { estalbished:100 } ;};fail

ct timeout set "cttime1";ok

# ct expectation
%ctexpect1 type ct expectation { protocol tcp; dport 1234; timeout 2m; size 12; };ok
%ctexpect2 type ct expectation { protocol udp; };fail
%ctexpect3 type ct expectation { protocol tcp; dport 4321; };fail
%ctexpect4 type ct expectation { protocol tcp; dport 4321; timeout 2m; };fail
%ctexpect5 type ct expectation { protocol udp; dport 9876; timeout 2m; size 12; l3proto ip; };ok

ct expectation set "ctexpect1";ok

# synproxy
%synproxy1 type synproxy mss 1460 wscale 7;ok
%synproxy2 type synproxy mss 1460 wscale 7 timestamp sack-perm;ok

synproxy name tcp dport map {443 : "synproxy1", 80 : "synproxy2"};ok
