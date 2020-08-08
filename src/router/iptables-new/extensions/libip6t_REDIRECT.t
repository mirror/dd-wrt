:PREROUTING,OUTPUT
*nat
-p tcp -j REDIRECT --to-ports 42;=;OK
-p udp -j REDIRECT --to-ports 42-1234;=;OK
-p tcp -j REDIRECT --to-ports 42-1234 --random;=;OK
-j REDIRECT --to-ports 42;;FAIL
