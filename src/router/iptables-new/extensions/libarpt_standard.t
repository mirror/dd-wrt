:INPUT
-s 192.168.0.1;=;OK
-s 0.0.0.0/8;=;OK
-s ! 0.0.0.0;! -s 0.0.0.0;OK
-d 192.168.0.1;=;OK
! -d 0.0.0.0;=;OK
-d 0.0.0.0/24;=;OK
-j DROP -i lo;=;OK
-j ACCEPT ! -i lo;=;OK
-i ppp+;=;OK
! -i ppp+;=;OK
-i lo --destination-mac 11:22:33:44:55:66;-i lo --dst-mac 11:22:33:44:55:66;OK
--source-mac Unicast;--src-mac 00:00:00:00:00:00/01:00:00:00:00:00;OK
! --src-mac Multicast;! --src-mac 01:00:00:00:00:00/01:00:00:00:00:00;OK
