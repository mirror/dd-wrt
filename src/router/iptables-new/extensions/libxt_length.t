:INPUT,FORWARD,OUTPUT
-m length --length 1;=;OK
-m length --length :2;-m length --length 0:2;OK
-m length --length 0:3;=;OK
-m length --length 4:;=;OK
-m length --length 0:65535;=;OK
-m length ! --length 0:65535;=;OK
-m length --length 0:65536;;FAIL
-m length --length -1:65535;;FAIL
-m length;;FAIL
