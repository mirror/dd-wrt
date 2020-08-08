:INPUT,FORWARD,OUTPUT
-m mh;;FAIL
-p mobility-header -m mh;=;OK
-p mobility-header -m mh --mh-type 1;=;OK
-p mobility-header -m mh ! --mh-type 4;=;OK
-p mobility-header -m mh --mh-type 4:123;=;OK
