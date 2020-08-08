:INPUT,OUTPUT
-p ipcomp -m ipcomp --ipcompspi 18 -j DROP;=;OK
-p ipcomp -m ipcomp ! --ipcompspi 18 -j ACCEPT;=;OK
