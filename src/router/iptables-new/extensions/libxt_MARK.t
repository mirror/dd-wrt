:INPUT,FORWARD,OUTPUT
-j MARK --set-xmark 0xfeedcafe/0xfeedcafe;=;OK
-j MARK --set-xmark 0;=;OK
-j MARK --set-xmark 4294967295;-j MARK --set-xmark 0xffffffff;OK
-j MARK --set-xmark 4294967296;;FAIL
-j MARK --set-xmark -1;;FAIL
-j MARK;;FAIL
