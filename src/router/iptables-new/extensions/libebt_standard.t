:INPUT,FORWARD,OUTPUT
-d de:ad:be:ef:00:00;=;OK
-s 0:0:0:0:0:0;-s 00:00:00:00:00:00;OK
-d 00:00:00:00:00:00;=;OK
-s de:ad:be:ef:0:00 -j RETURN;-s de:ad:be:ef:00:00 -j RETURN;OK
-d de:ad:be:ef:00:00 -j CONTINUE;=;OK
-d de:ad:be:ef:0:00/ff:ff:ff:ff:0:0 -j DROP;-d de:ad:be:ef:00:00/ff:ff:ff:ff:00:00 -j DROP;OK
-p ARP -j ACCEPT;=;OK
-p ! ARP -j ACCEPT;=;OK
-p 0 -j ACCEPT;=;FAIL
-p ! 0 -j ACCEPT;=;FAIL
:INPUT
-i foobar;=;OK
-o foobar;=;FAIL
:FORWARD
-i foobar;=;OK
-o foobar;=;OK
:OUTPUT
-i foobar;=;FAIL
-o foobar;=;OK
:PREROUTING
*nat
-i foobar;=;OK
-o foobar;=;FAIL
:POSTROUTING
*nat
-i foobar;=;FAIL
-o foobar;=;OK
