:INPUT,FORWARD,OUTPUT
-j NFLOG --nflog-group 1;=;OK
-j NFLOG --nflog-group 65535;=;OK
-j NFLOG --nflog-group 65536;;FAIL
-j NFLOG --nflog-group 0;-j NFLOG;OK
-j NFLOG --nflog-range 1;=;OK
-j NFLOG --nflog-range 4294967295;=;OK
-j NFLOG --nflog-range 4294967296;;FAIL
-j NFLOG --nflog-range -1;;FAIL
-j NFLOG --nflog-size 0;=;OK
-j NFLOG --nflog-size 1;=;OK
-j NFLOG --nflog-size 4294967295;=;OK
-j NFLOG --nflog-size 4294967296;;FAIL
-j NFLOG --nflog-size -1;;FAIL
# ERROR: cannot find: iptables -I INPUT -j NFLOG --nflog-prefix  xxxxxx [...]
# -j NFLOG --nflog-prefix xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;=;OK
# ERROR: should fail: iptables -A INPUT -j NFLOG --nflog-prefix  xxxxxxx [...]
#  -j NFLOG --nflog-prefix xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;;FAIL
-j NFLOG --nflog-threshold 1;=;OK
# ERROR: line 13 (should fail: iptables -A INPUT -j NFLOG --nflog-threshold 0
# -j NFLOG --nflog-threshold 0;;FAIL
-j NFLOG --nflog-threshold 65535;=;OK
-j NFLOG --nflog-threshold 65536;;FAIL
-j NFLOG;=;OK
