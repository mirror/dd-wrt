#the following line combines thecm last line to prevent this file from being sourced twice
#. /usr/sbin/cfgmgr.sh

# this file is target depedent, it provids
# 1. following variables :
#      RawEth=         # name of raw eth NIF. not assigned means 2 eth NIFs, otherwise 1 eth NIF.
#      RawEthLan=      # name of raw eth NIF for lan.
#      RawEthWan=      # name of raw eth NIF for wan.
#      WanIndepPhy=    # 0 means RawEthWan doesn't have independent phy (ie. connects to switch).
#                      # 1 means RawEthWan has independent phy (ie. doesn't connects to switch).
# 2. following functions :
#      et_init()       # initialize ethernet & switch.
#      sw_configvlan() # configure switch vlan for all kind of opmode.
#

RawEth=
RawEthLan=eth1
RawEthWan=eth2
RawSfp=eth0
WanIndepPhy=0
enable_bond=0
BondEth=bond0
#[ "$($CONFIG get ipv6_type)" = "bridge" ] && enable_bond=0

# for R9000 Two Switch Design(AR8337 switch):
# Switch-A:
# sw port 0 -> Trunk to CPU(eth1)
# sw port 5 -> Trunk to CPU(eth2)
# sw port 4 -> Trunk to Switch-B sw port 0
# sw port 6 -> Trunk to Switch-B sw port 5
# sw port 3 -> WAN
# sw port 2 -> LAN1
# sw port 1 -> LAN2
# Switch-B:
# sw port 0 -> Trunk to Switch-A sw port 4
# sw port 5 -> Trunk to Switch-A sw port 6
# sw port 4 -> LAN3
# sw port 3 -> LAN4
# sw port 2 -> LAN5
# sw port 1 -> LAN6
# sw port 6 -> No Used

ssdk_sh=/usr/sbin/ssdk_sh.old
ssdk_sh_new=/usr/sbin/ssdk_sh
ssdk_sh_id=/usr/sbin/ssdk_sh_id
swconfig=/sbin/swconfig
swconf=/tmp/sw.conf
ssdk_cmds_file=/tmp/ssdk.sh
ssdk_cmds_new_file=/tmp/ssdk_new.sh

et_landefmac()
{
	[ -f /tmp/lan_mac ] && cat /tmp/lan_mac || \
	echo "00:03:7f:$(hexdump -n 4 /dev/urandom | awk 'NR==1 {print $2$3}' \
	                 | sed 's/../&:/g' | cut -c 1-8)"
}

et_wandefmac()
{
	[ -f /tmp/wan_mac ] && cat /tmp/wan_mac || \
	echo "00:03:7f:$(hexdump -n 4 /dev/urandom | awk 'NR==1 {print $2$3}' \
	                 | sed 's/../&:/g' | cut -c 1-8)"
}


et_sfpdefmac()
{
	[ -f /tmp/sfp_mac ] && cat /tmp/sfp_mac || \
	echo "00:03:7f:$(hexdump -n 4 /dev/urandom | awk 'NR==1 {print $2$3}' \
	                 | sed 's/../&:/g' | cut -c 1-8)"
}

et_irq_affinity()
{
	local eth_name="$1"
	local irq

	for irq in `ls /sys/class/net/$eth_name/device/msi_irqs`; do
		echo $irq
		echo `cat /proc/irq/$irq/affinity_hint` > /proc/irq/$irq/smp_affinity
	done
}

bond_init(){
	[ "$enable_bond" != "1" ] && return
	
	[ -f /tmp/bond0_done ] && return
	touch /tmp/bond0_done

	echo "balance-xor" > /sys/devices/virtual/net/bond0/bonding/mode
	echo "layer2+3" > /sys/devices/virtual/net/bond0/bonding/xmit_hash_policy
	echo "1000" > /sys/devices/virtual/net/bond0/bonding/miimon
	echo "+$RawEthWan" > /sys/devices/virtual/net/bond0/bonding/slaves
	echo "+$RawEthLan" > /sys/devices/virtual/net/bond0/bonding/slaves

	ethtool -A $RawEthLan rx off
	ethtool -A $RawEthLan tx off
	ethtool -A $RawEthWan rx off
	ethtool -A $RawEthWan tx off

	ethtool -C $RawEthLan rx-usecs 100
	ethtool -C $RawEthLan tx-usecs 200
	ethtool -C $RawEthWan rx-usecs 100
	ethtool -C $RawEthWan tx-usecs 200

	echo 0 > /proc/sys/net/ipv4/conf/$RawEthLan/rp_filter
	echo 0 > /proc/sys/net/ipv4/conf/$RawEthWan/rp_filter
	echo 0 > /proc/sys/net/core/netdev_skb_tstamp
	echo 1 > /proc/sys/net/ipv4/route/fib_trie/fib_local_acc
	echo 1 > /proc/sys/net/core/hh_output_relaxed
	#echo 1 > /proc/sys/net/ipv4/ip_forward

	et_irq_affinity $RawEthLan
	et_irq_affinity $RawEthWan

	sw_init
}

et_init()
{
	#sw_init
	$CONFIG set lan_factory_mac="$(et_landefmac)"
	$CONFIG set wan_factory_mac="$(et_wandefmac)"
	$CONFIG set sfp_factory_mac="$(et_sfpdefmac)"
	ethtool -K $RawEthLan gro on # enable ethlan GRO to improve smb write performance
}

bond_switch_set() {
	sw_printconf_add_switch > $swconf
	$swconfig dev switch0 load $swconf

	$ssdk_sh vlan entry create 1
	$ssdk_sh vlan entry create 2
	$ssdk_sh vlan member add 1 0 tagged
	$ssdk_sh vlan member add 1 6 tagged
	$ssdk_sh vlan member add 2 0 tagged
	$ssdk_sh vlan member add 2 6 tagged
	$ssdk_sh vlan member add 1 1 untagged
	$ssdk_sh vlan member add 1 2 untagged
	$ssdk_sh vlan member add 1 3 untagged
	$ssdk_sh vlan member add 1 4 untagged
	$ssdk_sh vlan member add 2 5 untagged
	$ssdk_sh portVlan defaultCvid set 1 1
	$ssdk_sh portVlan defaultCvid set 2 1
	$ssdk_sh portVlan defaultCvid set 3 1
	$ssdk_sh portVlan defaultCvid set 4 1
	$ssdk_sh portVlan defaultCvid set 5 2
	$ssdk_sh misc cpuVid set enable
	$ssdk_sh portvlan ingress set 1 check
	$ssdk_sh portvlan ingress set 2 check
	$ssdk_sh portvlan ingress set 3 check
	$ssdk_sh portvlan ingress set 4 check
	$ssdk_sh portvlan ingress set 5 check
	$ssdk_sh trunk group set 0 enable 0x41
	$ssdk_sh trunk hashmode set 0xf
}

bond_hashstop(){
	$ssdk_sh misc cpuVid set disable
	$ssdk_sh trunk group set 0 disable 0x41
	$ssdk_sh trunk hashmode set 0xf
}

sw_init()                                                                                            
{                                                                                                    
	echo "switch init"
	# workaround of switch hw issue on r7500                                                      
	#$ssdk_sh debug reg set 0x04 0x07700000 4 >/dev/null                                           
	#$ssdk_sh debug reg set 0xe4 0x0006a545 4 >/dev/null                                           

	if [ "x$(lsmod | grep qca_ssdk)" != "x" ]; then
		return
	fi

	insmod /lib/modules/3.10.20/qca-ssdk.ko

#	$ssdk_sh_id 0 debug reg set 0x0 0x80000000 4
	$ssdk_sh_id 0 debug reg set 0x10 0x002613a0 4
	$ssdk_sh_id 0 debug reg set 0xe0 0xc74164de 4
	$ssdk_sh_id 0 debug reg set 0xe4 0x000ea545 4
	$ssdk_sh_id 0 debug reg set 0x4 0x07680000 4
	$ssdk_sh_id 0 debug reg set 0x8 0x07600000 4
	$ssdk_sh_id 0 debug reg set 0xc 0x80 4
	$ssdk_sh_id 0 debug reg set 0x624 0x007f7f7f 4
	$ssdk_sh_id 0 debug reg set 0x7c 0x4e 4
	$ssdk_sh_id 0 debug reg set 0x90 0x4e 4
	$ssdk_sh_id 0 debug reg set 0x94 0x4e 4

	#CPU --> (P0/5)QCA8337A(P4/6)--->(P0/5)QCA8337B
	#remove trunking on -0/5
	$ssdk_sh_id 0 debug reg set 0x700 0xd000 4
	$ssdk_sh_id 0 debug reg set 0x704 0x00ec0000 4

	$ssdk_sh_id 0 debug reg set 0x660 0x0014017e 4
	$ssdk_sh_id 0 debug reg set 0x66c 0x0014017d 4
	$ssdk_sh_id 0 debug reg set 0x678 0x0014017b 4
	$ssdk_sh_id 0 debug reg set 0x684 0x00140177 4
	$ssdk_sh_id 0 debug reg set 0x690 0x0014016f 4
	$ssdk_sh_id 0 debug reg set 0x69c 0x0014015f 4
	$ssdk_sh_id 0 debug reg set 0x6a8 0x0014013f 4

	$ssdk_sh_id 0 debug reg set 0x420 0x00010001 4
	#change p5 vid -->2
	$ssdk_sh_id 0 debug reg set 0x448 0x00020001 4
	$ssdk_sh_id 0 debug reg set 0x428 0x00010001 4
	$ssdk_sh_id 0 debug reg set 0x430 0x00010001 4
	$ssdk_sh_id 0 debug reg set 0x440 0x00010001 4
	$ssdk_sh_id 0 debug reg set 0x450 0x00010001 4

	$ssdk_sh_id 0 debug reg set 0x438 0x00020001 4

	$ssdk_sh_id 0 debug reg set 0x424 0x00002040 4
	$ssdk_sh_id 0 debug reg set 0x44c 0x00002040 4
	$ssdk_sh_id 0 debug reg set 0x42c 0x00001040 4
	$ssdk_sh_id 0 debug reg set 0x434 0x00001040 4
	$ssdk_sh_id 0 debug reg set 0x43c 0x00001040 4
	$ssdk_sh_id 0 debug reg set 0x444 0x00001040 4
	$ssdk_sh_id 0 debug reg set 0x454 0x00001040 4

	#VLAN1-0t/1/2/4/5t/6,VLAN2-0t/3/5t
	#vlan1-0t/1/2/4/6 vlan2-3/5t
	$ssdk_sh_id 0 debug reg set 0x610 0x0019dd50 4
	$ssdk_sh_id 0 debug reg set 0x614 0x80010002 4
	$ssdk_sh_id 0 debug reg set 0x610 0x001b77f0 4
	$ssdk_sh_id 0 debug reg set 0x614 0x80020002 4

	# do not learn mac address on internal trunk 5
	#$ssdk_sh_id 0 fdb portLearn set 5 disable
	#$ssdk_sh_id 0 fdb fdb entry flush 0

	# For rfc packet lose issue
	$ssdk_sh_id 0 debug reg set 0x94 0x7e 4
	$ssdk_sh_id 1 debug reg set 0x7c 0x7e 4

	$ssdk_sh_id 0 debug reg set 0x700 0xd000 4
	$ssdk_sh_id 0 debug reg set 0x704 0xec0000 4
	$ssdk_sh_id 1 debug reg set 0x620 0x1000f0 4

	$ssdk_sh_id 0 debug reg set 0x808 0x7f004e 4
	$ssdk_sh_id 1 debug reg set 0x808 0x7f004e 4

	$ssdk_sh_id 0 debug phy set 0x3 0xd 0x7
	$ssdk_sh_id 0 debug phy set 0x3 0xe 0x3c
	$ssdk_sh_id 0 debug phy set 0x3 0xd 0x4007
	$ssdk_sh_id 0 debug phy set 0x3 0xe 0x00
	$ssdk_sh_id 0 debug phy set 0x3 0x00 0x1200

	$ssdk_sh_id 1 debug phy set 0x4 0xd 0x7
	$ssdk_sh_id 1 debug phy set 0x4 0xe 0x3c
	$ssdk_sh_id 1 debug phy set 0x4 0xd 0x4007
	$ssdk_sh_id 1 debug phy set 0x4 0xe 0x00
	$ssdk_sh_id 1 debug phy set 0x4 0x00 0x1200
}                                                                                                    

sw_printconf_add_switch()
{
	cat <<EOF
config switch
	option name 'switch0'
	option reset '1'
	option enable_vlan '1'

EOF
}

sw_printconf_add_vlan() # $1: device, $2: vlan, $3: vid, $4: ports
{
	cat <<EOF
config switch_vlan
	option device '$1'
	option vlan '$2'
	option vid '$3'
	option ports '$4'

EOF
}

sw_tmpconf_start()
{
	rm -f $swconf.tmp*
}

sw_tmpconf_add_vlan() # $1: vlanindex, $2: vid, $3: ports
{
	cat <<EOF > "$swconf.tmp$1"
vid="$2"
ports="$3"
EOF
}

sw_tmpconf_adjust_vlan() # $1: vlanindex, $2: vid, $3: ports
{
	local vid ports i=1

	while [ $i -le $1 ]; do
		. "$swconf.tmp$i"
		if [ "$vid" = "$2" ]; then
			for p in $3; do
				echo $ports | grep -q '\<'$p'\>' && continue
				ports="$ports $p"
			done
			sw_tmpconf_add_vlan "$i" "$vid" "$ports"
			return 0
		fi
		i=$(($i + 1))
	done

	return 1
}

sw_tmpconf_generate_swconf() # $1: vlanindex
{
	local vid ports i=1

	sw_printconf_add_switch
	while [ $i -le $1 ]; do
		. "$swconf.tmp$i"
		sw_printconf_add_vlan "switch0" "$i" "$vid" "$ports"
		i=$(($i + 1))
	done
}

sw_print_ssdk_cmds_start()
{
	echo -n
}

sw_print_ssdk_cmds_flush_vlan()
{
	# Flush Switch0 VLAN Table
	cat <<EOF
$ssdk_sh_id 0 vlan entry flush
EOF
	# Flush Switch1 VLAN Table
	cat <<EOF
$ssdk_sh_id 1 vlan entry flush
EOF
}

sw_print_ssdk_cmds_add_vlan() # $1: swid, $2: vlanindext, $3: vlanid, $4: trunk_ports $5: access_ports $6: vlan_mode
{
	local swid="$1"
	local vlanindext="$2"
	local vlanid="$3"
	local trunk_ports="$4"
	local access_ports="$5"
	local vlan_mode="$6"
	local vlan_enable_bridge=$($CONFIG get vlan_tag_enable_bridge)
	
	# Set Prot VLAN ID, Ingress Mode, Egress Mode.
	local pp
	for pp in $trunk_ports; do
		[ "x$pp" = "x" ] && continue
		if [ "$vlan_mode" = "lan" -o "$vlan_mode" = "factory" -o "$vlan_mode" = "normal_lan" -o "$vlan_mode" = "iptv_lan" -o "$vlan_mode" = "wan" ]; then
		cat <<EOF
$ssdk_sh_id $swid portvlan defaultCVid set $pp $vlanid
$ssdk_sh_id $swid portvlan ingress set $pp check
$ssdk_sh_id $swid portvlan egress set $pp tagged
EOF
		elif [ "$vlan_mode" = "vlan_br" -o "$vlan_mode" = "vlan_wan" ]; then
			if [ "$swid" = "$WAN_SID" -a "$pp" = "$WAN_PID" ]; then
				if [ "$vlanid" != "838" -a "$vlanid" != "840" ]; then
			cat <<EOF
$ssdk_sh_id $swid portvlan defaultCVid set $pp $vlanid
$ssdk_sh_id $swid portvlan ingress set $pp check
$ssdk_sh_id $swid portvlan egress set $pp unmodified
EOF
#				else
#			cat <<EOF
#$ssdk_sh_id $swid portvlan defaultCVid set $pp $vlanid
#$ssdk_sh_id $swid portvlan ingress set $pp check
#$ssdk_sh_id $swid portvlan egress set $pp tagged
#EOF
				fi
			fi
		elif [ "$vlan_mode" = "iptv" ]; then
			if [ "$swid" = "$WAN_SID" -a "$pp" = "5" -a "$vlan_enable_bridge" != "1"]; then
			cat <<EOF
$ssdk_sh_id $swid portvlan defaultCVid set $pp $vlanid
$ssdk_sh_id $swid portvlan ingress set $pp check
$ssdk_sh_id $swid portvlan egress set $pp tagged
EOF
			fi
		fi
	done

	for pp in $access_ports; do
		[ "x$pp" = "x" ] && continue
		if [ "$vlan_mode" = "lan" -o "$vlan_mode" = "factory" -o "$vlan_mode" = "normal_lan" -o "$vlan_mode" = "iptv_lan" -o "$vlan_mode" = "wan" -o "$vlan_mode" = "iptv" -o "$vlan_mode" = "vlan" ]; then
		cat <<EOF
$ssdk_sh_id $swid portvlan defaultCVid set $pp $vlanid
$ssdk_sh_id $swid portvlan ingress set $pp check
$ssdk_sh_id $swid portvlan egress set $pp untagged
EOF
		fi
		if [ "$vlan_mode" != "vlan_br" -a "$vlan_mode" = "vlan_wan" ]; then
		cat <<EOF
$ssdk_sh_id $swid portvlan defaultCVid set $pp $vlanid
$ssdk_sh_id $swid portvlan ingress set $pp check
$ssdk_sh_id $swid portvlan egress set $pp untagged
EOF
		fi
	done

	# Crreat VLAN Entry in VLAN Table
	cat <<EOF
$ssdk_sh_id $swid vlan entry create $vlanid
EOF
	for pp in $trunk_ports; do
		[ "x$pp" = "x" ] && continue
		cat <<EOF
$ssdk_sh_id $swid vlan member add $vlanid $pp tagged
EOF
	done
	for pp in $access_ports; do
		[ "x$pp" = "x" ] && continue
		cat <<EOF
$ssdk_sh_id $swid vlan member add $vlanid $pp untagged
EOF
	done
}

LAN1_SID=0
LAN1_PID=2
LAN2_SID=0
LAN2_PID=1
LAN3_SID=1
LAN3_PID=4
LAN4_SID=1
LAN4_PID=3
LAN5_SID=1
LAN5_PID=2
LAN6_SID=1
LAN6_PID=1
WAN_SID=0
WAN_PID=3

sw_user_lan_ports_vlan_config() # $1: vlanid $2: LAN Ports, $3: Is_Bridge_VLAN $4: Is_WAN_VLAN $5: Is_IPTV $6: Mode
{
	local vlanid="$1"
	local lan_ports="$2"
	local is_bridge_vlan="$3"
	local is_wan_vlan="$4"
	local is_iptv_vlan="$5"
	local vlan_mode="$6"
	local vlan_enable_bridge=$($CONFIG get enable_orange)
	local enable_spvoda_iptv=$($CONFIG get spain_voda_iptv)
	local iptv_vlan_enable=$($CONFIG get iptv_vlan_enable)
	local wan_preference=$($CONFIG get wan_preference)

	local sw0_trunk_ports="0 5 4 6"
	local sw0_access_ports=""
	local sw1_trunk_ports="0 5"
	local sw1_access_ports=""

	local lan_pp
	for lan_pp in $lan_ports; do
		[ "x$lan_pp" = "x" ] && continue
		[ $lan_pp -lt 1 -o $lan_pp -gt 6 ] && continue
		[ "$iptv_vlan_enable" = "1" -a "$vlan_mode" = "iptv" ] && continue
		if [ "$(eval echo "$""LAN"$lan_pp"_SID")" = "0" ]; then
			sw0_access_ports="$sw0_access_ports $(eval echo "$""LAN"$lan_pp"_PID")"
		else
			sw1_access_ports="$sw1_access_ports $(eval echo "$""LAN"$lan_pp"_PID")"
		fi
	done

	if [ "$is_bridge_vlan" = "1" ]; then
		if [ "$wan_preference" = "0" ]; then
			sw0_trunk_ports="$sw0_trunk_ports 3"
		else
			sw0_trunk_ports="0 5 4 6"
			sw0_access_ports="$sw0_access_ports 3"
		fi
	fi

	if [ "$is_wan_vlan" = "1" ]; then
		if [ "$is_bridge_vlan" = "1" ]; then
			sw0_trunk_ports="0 5 4 6 3"
			sw0_access_ports=""
			sw1_trunk_ports="0 5"
			sw1_access_ports=""
		else
			sw0_trunk_ports="0 5 4 6"
			sw0_access_ports="3"
			sw1_trunk_ports="0 5"
			sw1_access_ports=""
		fi
	fi

	if [ "$is_iptv_vlan" = "1" ]; then	
		if [ "$iptv_vlan_enable" = "1" -o "$vlan_enable_bridge" = "1" -o "$enable_spvoda_iptv" = "1" ]; then
			if [ "$vlan_enable_bridge" = "1" -o "$enable_spvoda_iptv" = "1" ]; then
				sw0_trunk_ports="3 4 6"
			else	
				sw0_trunk_ports="3 5 4 6"
			fi
			if [ "$vlan_mode" = "iptv" ]; then
				for lan_pp in $lan_ports; do
					[ "x$lan_pp" = "x" ] && continue
					[ $lan_pp -lt 1 -o $lan_pp -gt 6 ] && continue
					if [ "$(eval echo "$""LAN"$lan_pp"_SID")" = "0" ]; then
						sw0_trunk_ports="$sw0_trunk_ports $(eval echo "$""LAN"$lan_pp"_PID")"
					else
						sw1_trunk_ports="$sw1_trunk_ports $(eval echo "$""LAN"$lan_pp"_PID")"
					fi
				done
			fi
		else
			sw0_trunk_ports="5 4 6"
			sw0_access_ports="$sw0_access_ports 3"
		fi
	fi

	if [ "$vlan_mode" = "iptv_lan" ]; then
		sw0_trunk_ports="0 4 6"

		if [ "$iptv_vlan_enable" = "1" ]; then
			echo "*** FreeISP access ports settings"
			if [ "$wan_preference" = "0" ]; then
				sw0_access_ports="2 1"
			else
				sw0_access_ports="3 2 1"
			fi
			sw1_access_ports="4 3 2 1"
		fi
	fi

	if [ "$vlan_mode" = "normal_lan" ]; then
			sw0_trunk_ports="0 4 6"
			#sw1_trunk_ports="0 5"
	fi

	if [ "$vlan_mode" = "wan" ]; then
			sw0_trunk_ports="5"
			sw1_trunk_ports=""
	fi


	if [ "$vlan_mode" = "factory" ]; then
		sw0_trunk_ports="0 5 4 6"
		sw0_access_ports="1 2 3"
		sw1_trunk_ports="0 5"
		sw1_access_ports="1 2 3 4 6"
	fi

	sw_print_ssdk_cmds_add_vlan "0" "$vlanid" "$vlanid" "$sw0_trunk_ports" "$sw0_access_ports" "$vlan_mode" >> $ssdk_cmds_new_file
	sw_print_ssdk_cmds_add_vlan "1" "$vlanid" "$vlanid" "$sw1_trunk_ports" "$sw1_access_ports" "$vlan_mode" >> $ssdk_cmds_new_file
}

sw_print_ssdk_cmds_set_ports_pri() # $1: ports, $2: pri
{
        local pp swid pid
	local lan_ports="$1"
	local vlan_pri="$2"

        for pp in $lan_ports; do
		[ "x$pp" = "x" ] && continue
		[ $pp -lt 1 -o $pp -gt 6 ] && continue
		swid="$(eval echo "$""LAN"$pp"_SID")"
		pid=$(eval echo "$""LAN"$pp"_PID")

                cat <<EOF
$ssdk_sh_id $swid qos ptDefaultCpri set $pid $vlan_pri
EOF
        done
}

sw_print_ssdk_cmds_set_wan_port_pri() #$1: pri
{
	local vlan_pri="$1"
	local swid="$WAN_SID"
	local pid="$WAN_PID"
	cat <<EOF
$ssdk_sh_id $swid qos ptDefaultCpri set $pid $vlan_pri
EOF
}


sw_configvlan_factory()
{
	sw_print_ssdk_cmds_start > $ssdk_cmds_new_file
	sw_print_ssdk_cmds_flush_vlan >> $ssdk_cmds_new_file

	if [ "x$($CONFIG get factory_tt3)" = "x1" ]; then
		sw_user_lan_ports_vlan_config "1" "" "0" "0" "0" "factory"

		# This LED will be shut down later by other modules,
		# so I run this again in init.d/done to make sure it's on
		ledcontrol -n usb1 -c amber -s on
	else
		sw_user_lan_ports_vlan_config "1" "" "0" "0" "0" "factory"
		#$ssdk_sh_id 0 debug reg set 0x448 0x00010001 4
	
		#$ssdk_sh_id 0 debug reg set 0x610 0x00195550 4
		#$ssdk_sh_id 0 debug reg set 0x610 0x001bfff0 4

		#$ssdk_sh_id 0 debug reg set 0x618 0xd0e0002b 4
		#$ssdk_sh_id 1 debug reg set 0x618 0xd0e0002b 4
	fi

	qt sh $ssdk_cmds_new_file
}

sw_configvlan_normal()
{
	sw_print_ssdk_cmds_start > $ssdk_cmds_new_file
	sw_print_ssdk_cmds_flush_vlan >> $ssdk_cmds_new_file
	sw_user_lan_ports_vlan_config "2" "1" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "3" "2" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "4" "3" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "5" "4" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "6" "5" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "7" "6" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "1" "" "0" "1" "0" "wan"

	qt sh $ssdk_cmds_new_file
}

i_mask() # $1: 1 / 2 / 3 / 4 / 5 / 6
{
	case $1 in
	1) echo 1 ;;
	2) echo 2 ;;
	3) echo 4 ;;
	4) echo 8 ;;
	5) echo 16 ;;
	6) echo 32 ;;
	esac
}

sw_configvlan_iptv() # $1: iptv_mask
{
	local mask=$(($1 & 0x3f))
	local lan_ports=""
	local iptv_ports=""
	local i
	local iptv_vlan_enable=$($CONFIG get iptv_vlan_enable)
	local iptv_vlan=$($CONFIG get iptv_vlan)

	for i in 1 2 3 4 5 6; do
		[ $(( $(i_mask $i) & $mask )) -eq 0 ] && lan_ports="$lan_ports $i" || iptv_ports="$iptv_ports $i"
	done

	sw_print_ssdk_cmds_start > $ssdk_cmds_new_file
	sw_print_ssdk_cmds_flush_vlan >> $ssdk_cmds_new_file
	sw_user_lan_ports_vlan_config "1" "$lan_ports" "0" "0" "0" "iptv_lan"
	if [ "$iptv_vlan_enable" = "1" ]; then
		sw_user_lan_ports_vlan_config "$iptv_vlan" "$iptv_ports" "0" "0" "1" "iptv"
		sw_user_lan_ports_vlan_config "10" "" "0" "1" "0" "vlan_wan"
	else
		sw_user_lan_ports_vlan_config "2" "$iptv_ports" "0" "0" "1" "iptv"
	fi
	qt sh $ssdk_cmds_new_file
}

sw_configvlan_vlan()
# $1: start
#     add -> $2: br/wan/lan, $3: vid, $4: mask, $5: pri
#     end
{
	case "$1" in
	start)
		sw_print_ssdk_cmds_start > $ssdk_cmds_new_file
		sw_print_ssdk_cmds_flush_vlan >> $ssdk_cmds_new_file
		;;
	add)
		local vid=$3 mask=$(($4 & 0x3f)) pri=$5
		local i
		local ports=""
		for i in 1 2 3 4 5 6; do
			[ $(( $(i_mask $i) & $mask )) -eq 0 ] || ports="$ports $i"
		done
		case "$2" in
			lan) 
				sw_user_lan_ports_vlan_config "$vid" "$ports" "0" "0" "0" "lan"
				sw_print_ssdk_cmds_set_ports_pri "$ports" "$pri" >> $ssdk_cmds_new_file
				;;
			vlan) 
				sw_user_lan_ports_vlan_config "$vid" "$ports" "1" "0" "0" "vlan"
				sw_print_ssdk_cmds_set_ports_pri "$ports" "$pri" >> $ssdk_cmds_new_file
				;;
			iptv)
				sw_user_lan_ports_vlan_config "$vid" "$ports" "0" "0" "1" "iptv"
				sw_print_ssdk_cmds_set_ports_pri "$ports" "$pri" >> $ssdk_cmds_new_file
				;;
			wan) 
				sw_user_lan_ports_vlan_config "$vid" "" "0" "1" "0" "vlan_wan"
				sw_print_ssdk_cmds_set_wan_port_pri "$pri" >> $ssdk_cmds_new_file
				;;
			br) 
				sw_user_lan_ports_vlan_config "$vid" "" "1" "1" "0" "vlan_br"
				if [ "$vid" != "838" -a "$vid" != "840" ]; then
					sw_print_ssdk_cmds_set_wan_port_pri "$pri" >> $ssdk_cmds_new_file
				fi
				;;
		esac

		;;
	end)
		qt sh $ssdk_cmds_new_file
		;;
	esac
}

sw_configvlan() # $1 : normal/iptv/vlan/apmode/brmode
{
	local opmode=$1

	bond_hashstop
	shift
	case "$opmode" in
	normal) sw_configvlan_normal "$@" ;;
	iptv) sw_configvlan_iptv "$@" ;;
	vlan) sw_configvlan_vlan "$@" ;;
	factory) sw_configvlan_factory "$@" ;;
	*) sw_configvlan_normal "$@" ;;
	esac
}
