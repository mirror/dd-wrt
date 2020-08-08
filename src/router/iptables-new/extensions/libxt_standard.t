:INPUT,FORWARD,OUTPUT
-s 127.0.0.1/32 -d 0.0.0.0/8 -j DROP;=;OK
! -s 0.0.0.0 -j ACCEPT;! -s 0.0.0.0/32 -j ACCEPT;OK
! -d 0.0.0.0/32 -j ACCEPT;=;OK
-s 0.0.0.0/24 -j RETURN;=;OK
-p tcp -j ACCEPT;=;OK
! -p udp -j ACCEPT;=;OK
-j DROP;=;OK
-j ACCEPT;=;OK
-j RETURN;=;OK
! -p 0 -j ACCEPT;=;FAIL
