

grep -q iptable_filter /proc/modules || /etc/init.d/ipt_modules start

[ -e /sys/module/xt_ndpi ] || insmod ../src/xt_ndpi.ko xt_debug=1

echo "add_custom TEST" >/proc/net/xt_ndpi/proto

echo "reset" >/proc/net/xt_ndpi/host_proto
X=${1:-5}
for i in `seq 0 $X`; do
	[ -f "p$i" ] || continue
	echo "$i"
	sync
	if cat p$i >/proc/net/xt_ndpi/host_proto 2>/dev/zero ; then
		if [ -e "p$i.err" ]; then echo ERROR; else echo OK ; fi
	else
		if [ -e "p$i.err" ]; then echo OK; else echo ERROR ; fi
	fi
	diff -u /proc/net/xt_ndpi/host_proto p$i.res
done
#iptables-restore <.iptables.cmd
if [ "$1" = "0" ]; then
	echo
#  sleep 1
#  sync
#  echo rmmod xt_ndpi
#  rmmod xt_ndpi
fi
