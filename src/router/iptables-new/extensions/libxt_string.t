:INPUT,FORWARD,OUTPUT
# ERROR: cannot find: iptables -I INPUT -m string --algo bm --string "test"
# -m string --algo bm --string "test";=;OK
# ERROR: cannot find: iptables -I INPUT -m string --algo kmp --string "test")
# -m string --algo kmp --string "test";=;OK
# ERROR: cannot find: iptables -I INPUT -m string --algo kmp ! --string "test"
# -m string --algo kmp ! --string "test";=;OK
# cannot find: iptables -I INPUT -m string --algo bm --string "xxxxxxxxxxx" ....]
# -m string --algo bm --string "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";=;OK
# ERROR: cannot load: iptables -A INPUT -m string --algo bm --string "xxxx"
# -m string --algo bm --string "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";=;OK
# ERROR: cannot load: iptables -A INPUT -m string --algo bm --hexstring "|0a0a0a0a|"
# -m string --algo bm --hexstring "|0a0a0a0a|";=;OK
# ERROR: cannot find: iptables -I INPUT -m string --algo bm --from 0 --to 65535 --string "test"
# -m string --algo bm --from 0 --to 65535 --string "test";=;OK
-m string --algo wrong;;FAIL
-m string --algo bm;;FAIL
-m string;;FAIL
