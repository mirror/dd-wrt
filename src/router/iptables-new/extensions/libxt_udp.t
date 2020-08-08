:INPUT,OUTPUT,FORWARD
-p udp -m udp --sport 1;=;OK
-p udp -m udp --sport 65535;=;OK
-p udp -m udp --dport 1;=;OK
-p udp -m udp --dport 65535;=;OK
-p udp -m udp --sport 1:1023;=;OK
-p udp -m udp --sport 1024:65535;=;OK
-p udp -m udp --sport 1024:;-p udp -m udp --sport 1024:65535;OK
-p udp -m udp ! --sport 1;=;OK
-p udp -m udp ! --sport 65535;=;OK
-p udp -m udp ! --dport 1;=;OK
-p udp -m udp ! --dport 65535;=;OK
-p udp -m udp --sport 1 --dport 65535;=;OK
-p udp -m udp --sport 65535 --dport 1;=;OK
-p udp -m udp ! --sport 1 --dport 65535;=;OK
-p udp -m udp ! --sport 65535 --dport 1;=;OK
# ERRROR: should fail: iptables -A INPUT -p udp -m udp --sport 65536
# -p udp -m udp --sport 65536;;FAIL
-p udp -m udp --sport -1;;FAIL
-p udp -m udp --dport -1;;FAIL
# should we accept this below?
-p udp -m udp;=;OK
