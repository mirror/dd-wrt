:INPUT,FORWARD,OUTPUT
-m rt --rt-type 0 --rt-segsleft 1:23 --rt-len 42 --rt-0-res;=;OK
-m rt --rt-type 0 ! --rt-segsleft 1:23 ! --rt-len 42 --rt-0-res;=;OK
-m rt ! --rt-type 1 ! --rt-segsleft 12:23 ! --rt-len 42;=;OK
-m rt;=;OK
