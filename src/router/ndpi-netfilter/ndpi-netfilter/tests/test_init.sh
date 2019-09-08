

grep -q iptable_filter /proc/modules || /etc/init.d/ipt_modules start

[ -e /sys/module/xt_ndpi ] || insmod ../src/xt_ndpi.ko lib_trace=0 xt_debug=0

echo "add_custom rkn" >/proc/net/xt_ndpi/proto
echo "add_custom help" >/proc/net/xt_ndpi/proto
#echo "rkn debug 3" >/proc/net/xt_ndpi/proto
#echo "DNS debug 3" >/proc/net/xt_ndpi/proto
#echo "0 debug 0" >/proc/net/xt_ndpi/proto
#sync
sync
s=""
for i in "aanet.ru" ".aanet.ru" "guap.ru" ".guap.ru"; do
  for j in "" `seq 1 10` ; do
	s="+rkn:$j$i $s"
  done
done
echo "$s" >/proc/net/xt_ndpi/host_proto
echo -n "+rkn:test.ru,.test.ru,bad.ru help:help.com,help.org" >/proc/net/xt_ndpi/host_proto
echo '+1.1.1.1 any:81:rkn' >/proc/net/xt_ndpi/ip_proto
egrep -i '^(rkn|help):' /proc/net/xt_ndpi/host_proto
egrep -i '(rkn|help)' /proc/net/xt_ndpi/ip_proto
iptables-restore <.iptables.cmd
#rmmod xt_ndpi
