:PREROUTING,FORWARD,OUTPUT,POSTROUTING
*mangle
-j CONNMARK --restore-mark;=;OK
-j CONNMARK --save-mark;=;OK
-j CONNMARK --save-mark --nfmask 0xfffffff --ctmask 0xffffffff;-j CONNMARK --save-mark;OK
-j CONNMARK --restore-mark --nfmask 0xfffffff --ctmask 0xffffffff;-j CONNMARK --restore-mark;OK
-j CONNMARK;;FAIL
