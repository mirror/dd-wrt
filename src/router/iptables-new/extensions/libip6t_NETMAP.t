:PREROUTING,INPUT,OUTPUT,POSTROUTING
*nat
-j NETMAP --to dead::/64;=;OK
-j NETMAP --to dead::beef;=;OK
