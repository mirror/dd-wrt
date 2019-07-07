#the following line combines the last line to prevent this file from being sourced twice
if [ "x$opmode_sh" = "x" ]; then opmode_sh="sourced"
. /usr/sbin/cfgmgr.sh
. /usr/sbin/enet.sh

# name of NIF :
#   Eth Lan NIF will always be "ethlan"
#   Eth Wan NIF will always be "ethwan"
#   Lan NIF will always be "br0"
#   Wan NIF will be "brwan" or "ppp0"

# vlan-iptv related configs :
#
#   enable_vlan (1/0) : enable / disable
#     vlan_type (1/0) : vlan group / bridge group (iptv)
#
#   vlan group configs :
#     vlan_tag_1 ~ vlan_tag_10 :
#         Enable   Name      VID     Priority   lan-ports    wireless
#          1/0    xxxxxx    1-4094     0-7      x (4bits)    x (4bits)
#        lan-ports : 
#            bit0 -> lan1       belongs to vlan group or not
#            bit1 -> lan2       belongs to vlan group or not
#            bit2 -> lan3       belongs to vlan group or not
#            bit3 -> lan4       belongs to vlan group or not
#            bit4 -> lan5       belongs to vlan group or not
#            bit5 -> lan6       belongs to vlan group or not
#        wireless :
#            bit0 -> 2.4G       belongs to vlan group or not
#            bit1 -> 5G         belongs to vlan group or not
#            bit3 -> 2.4G-Guest belongs to vlan group or not
#            bit4 -> 5G-Guest   belongs to vlan group or not
#            bit4 -> 60G        belongs to vlan group or not
#
#   bridge group (iptv) configs :
#     wan_brig_ssid1 (1/0)       : 2.4G       belongs to bridge group or not
#     wan_brig_ssid2 (1/0)       : 5G         belongs to bridge group or not
#     wan_brig_guest_ssid1 (1/0) : 2.4G-Guest belongs to bridge group or not
#     wan_brig_guest_ssid2 (1/0) : 5G-Guest   belongs to bridge group or not
#     wan_brig_ssid_ad (1/0)     : 60G        belongs to bridge group or not
#     iptv_mask (4 bits) : 
#            bit0 -> lan1                     belongs to bridge group or not
#            bit1 -> lan2                     belongs to bridge group or not
#            bit2 -> lan3                     belongs to bridge group or not
#            bit3 -> lan4                     belongs to bridge group or not
#            bit4 -> lan5                     belongs to bridge group or not
#            bit5 -> lan6                     belongs to bridge group or not
#     iptv_mask_change :
#
#   wlg1_endis_guestNet (1/0) : 2.4G-Guest enable or not
#   wla1_endis_guestNet (1/0) :   5G-Guest enable or not
#
#   opmode related induced configs : 
#     i_opmode (normal/iptv/vlan/apmode/brmode) : device operation mode
#     i_wlg_br                                  : the bridge wlg belongs to
#     i_wla_br                                  : the bridge wla belongs to
#     i_wlg_guest_br                            : the bridge wlg_guest belongs to
#     i_wla_guest_br                            : the bridge wla_guest belongs to
#     i_wlg_pri                                 : the priority of wlg
#     i_wla_pri                                 : the priority of wla
#     i_wlg_guest_pri                           : the priority of wlg_guest
#     i_wla_guest_pri                           : the priority of wla_guest
#     i_wig_br                                  : the bridge wig belongs to
#     i_wig_pri                                 : the priority of wig
#
Switch_set()
{

	[ -f /tmp/switch_havset  ] && return
	touch /tmp/switch_haveset
	
	et_irq_affinity $RawEthLan
	et_irq_affinity $RawEthWan

	sw_init
}

op_opmode() # ret: factory / brmode / apmode / extmode / normal / vlan / iptv
{
	[ "$($CONFIG get factory_mode)" = "1" ] && echo "factory" && return
	[ "$($CONFIG get extender_mode)" = "1" ] && echo "extmode" && return
	[ $($CONFIG get bridge_mode) = "1" ] && echo "brmode" && return
	[ $($CONFIG get ap_mode) = "1" ] && echo "apmode" && return
	! [ $($CONFIG get enable_vlan) = "1" ] && echo "normal" && return
	[ $($CONFIG get vlan_type) = "1" ] && echo "vlan" || echo "iptv"
}

op_set_induced_configs()
{
	local opmode=$(op_opmode)
	local i_wlg_br="br0" i_wla_br="br0" i_wlg_guest_br="br0" i_wla_guest_br="br0" i_wig_br="br0"
	local i_wlg_pri="" i_wla_pri="" i_wlg_guest_pri="" i_wla_guest_pri="" i_wig_pri=""
	local i tv i_vid

	oc echo "opmode = $opmode"
	$CONFIG set i_opmode=$opmode

	case "$opmode" in
	iptv)
		[ "$($CONFIG get wan_brig_ssid1)" = "1" ] && i_wlg_br="brwan"
		[ "$($CONFIG get wan_brig_ssid2)" = "1" ] && i_wla_br="brwan"
		[ "$($CONFIG get wan_brig_guest_ssid1)" = "1" ] && i_wlg_guest_br="brwan"
		[ "$($CONFIG get wan_brig_guest_ssid2)" = "1" ] && i_wla_guest_br="brwan"
		[ "$($CONFIG get wan_brig_ssid_ad)" = "1" ] && i_wig_br="brwan"
		;;

	vlan)
		for i in 1 2 3 4 5 6 7 8 9 10; do
			tv=$($CONFIG get vlan_tag_$i)
			[ -n "$tv" ] || continue
			set - $(echo $tv)
			# $1: enable, $2: name, $3: vid, $4: pri, $5:wports, $6:wlports
			[ "$1" = "1" ] || continue
			[ "$2" = "Internet" -o "$2" = "Intranet" ] && i_vid=$3 && continue
			[ $(( 1 & $6 )) -ne 0 ] && i_wlg_br="br$3" && i_wlg_pri="$4"
			[ $(( 2 & $6 )) -ne 0 ] && i_wla_br="br$3" && i_wla_pri="$4"
			[ $(( 4 & $6 )) -ne 0 ] && i_wlg_guest_br="br$3" && i_wlg_guest_pri="$4"
			[ $(( 8 & $6 )) -ne 0 ] && i_wla_guest_br="br$3" && i_wla_guest_pri="$4"
			[ $(( 16 & $6 )) -ne 0 ] && i_wig_br="br$3" && i_wig_pri="$4"
		done
		[ "$i_vid" != "0" ] && {
			[ "$i_wlg_br" = "br$i_vid" ] && i_wlg_br="brwan"
			[ "$i_wla_br" = "br$i_vid" ] && i_wla_br="brwan"
			[ "$i_wlg_guest_br" = "br$i_vid" ] && i_wlg_guest_br="brwan"
			[ "$i_wla_guest_br" = "br$i_vid" ] && i_wla_guest_br="brwan"
			[ "$i_wig_br" = "br$i_vid" ] && i_wig_br="brwan"
		}
		;;
	esac

	$CONFIG set i_wlg_br="$i_wlg_br"
	$CONFIG set i_wla_br="$i_wla_br"
	$CONFIG set i_wlg_guest_br="$i_wlg_guest_br"
	$CONFIG set i_wla_guest_br="$i_wla_guest_br"
	$CONFIG set i_wlg_pri="$i_wlg_pri"
	$CONFIG set i_wla_pri="$i_wla_pri"
	$CONFIG set i_wlg_guest_pri="$i_wlg_guest_pri"
	$CONFIG set i_wla_guest_pri="$i_wla_guest_pri"
	$CONFIG set i_wig_br="$i_wig_br"
	$CONFIG set i_wig_pri="$i_wig_pri"
}

bond_create_ifs()
{
	local landefmac=$($CONFIG get lan_factory_mac)
	local wandefmac=$($CONFIG get cur_wanmac)

	bond_init
	[ "x$wandefmac" = "x" ] && wandefmac=$($CONFIG get wan_factory_mac)
	ifconfig $RawEthLan hw ether $landefmac
	ifconfig $RawEthWan hw ether $landefmac
	ifconfig $BondEth hw ether $landefmac
	ifconfig $BondEth up
	vconfig add $BondEth 1 && ifconfig $BondEth.1 down
	vconfig add $BondEth 2 && ifconfig $BondEth.2 down
	ip link set dev $BondEth.1 name ethlan
	ip link set dev $BondEth.2 name ethwan
}

br_create() # $1: brname
{
	brctl addbr $1
	brctl setfd $1 0
	brctl stp $1 0
	echo 0 > /sys/devices/virtual/net/$1/bridge/multicast_snooping
}

br_allbrs()
{
	awk '/br[0-9wo]/ {print $1}' /proc/net/dev |sed 's/://g'
}

br_allnifs() # $1: brx
{
	brctl show $1 | awk '!/bridge/ {print $NF}' | grep "eth\|sfp\|ath\|host0.\|bond0."
}

op_del_all_brs_vifs()
{
	local brx nif
	local landefmac=$($CONFIG get lan_factory_mac)
	local wandefmac=$($CONFIG get wan_factory_mac)

	for brx in $(br_allbrs); do
		ifconfig $brx down
		for nif in $(br_allnifs $brx); do
			ifconfig $nif down
			brctl delif $brx $nif
			case "$nif" in
			ethlan|ethwan)
				;;
			eth0.*|eth1.*|eth2.*|bond0.*)
				vconfig rem $nif
				;;
			sfpwan)
				ifconfig sfpwan down
				vconfig rem sfpwan || ip link set dev sfpwan name eth0
				;;	
			esac
		done
		[ "$brx" != br0 -a $brx != "brwan" ] && brctl delbr $brx
	done

	ifconfig ethlan down
	vconfig rem ethlan || ip link set dev ethlan name $RawEthLan
	ifconfig ethwan down
	vconfig rem ethwan || ip link set dev ethwan name $RawEthWan

	#[ -n "$RawEth" ] && ifconfig $RawEth down || {
	#	ifconfig $RawEthLan down
	#	ifconfig $RawEthWan down
	#}
        #echo "-$RawEthWan" > /sys/devices/virtual/net/bond0/bonding/slaves
        #echo "-$RawEthLan" > /sys/devices/virtual/net/bond0/bonding/slaves
	#ifconfig $RawEthLan hw ether $landefmac
	#ifconfig $RawEthWan hw ether $wandefmac 
	#ifconfig $BondEth down
}

op_create_br0_brwan()
{
	br_create br0
	br_create brwan
}

factory_mode_create_brs_and_vifs()
{
	local landefmac=$($CONFIG get lan_factory_mac)
	local wandefmac=$($CONFIG get wan_factory_mac)
	local sfpdefmac=$($CONFIG get sfp_factory_mac)

	if [ -n "$RawEth" ]; then
		ifconfig $RawEth hw ether $landefmac
		ifconfig $RawEth up
		vconfig add $RawEth 0 && ifconfig $RawEth.0 down
		vconfig add $RawEth 1 && ifconfig $RawEth.1 down
		ip link set dev $RawEth.0 name ethlan
		ip link set dev $RawEth.1 name ethwan
	elif [ "$enable_bond" = "1" ]; then
		bond_create_ifs
	else
		ifconfig $RawEthLan hw ether $landefmac
		ifconfig $RawEthWan hw ether $wandefmac
		ifconfig $RawEthLan up
		ifconfig $RawEthWan up
		vconfig add $RawEthLan 1 && ifconfig $RawEthLan.1 down
		vconfig add $RawEthWan 2 && ifconfig $RawEthWan.2 down
		ip link set dev $RawEthLan.1 name ethlan
		ip link set dev $RawEthWan.2 name ethwan
	fi

	Switch_set
	brctl addif br0 ethlan

	# SFP+ Interface
	ifconfig eth0 hw ether $sfpdefmac
	brctl addif br0 eth0
	ifconfig eth0 up

	brctl addif br0 ethwan
	ifconfig br0 hw ether $landefmac
	sw_configvlan "factory"
}

apmode_create_brs_and_vifs()
{
	local landefmac=$($CONFIG get lan_factory_mac)
	local wandefmac=$($CONFIG get wan_factory_mac)
	local sfpdefmac=$($CONFIG get sfp_factory_mac)

	if [ -n "$RawEth" ]; then
		ifconfig $RawEth hw ether $landefmac
		ifconfig $RawEth up
		vconfig add $RawEth 0 && ifconfig $RawEth.0 down
		vconfig add $RawEth 1 && ifconfig $RawEth.1 down
		ip link set dev $RawEth.0 name ethlan
		ip link set dev $RawEth.1 name ethwan
	elif [ "$enable_bond" = "1" ]; then
		bond_create_ifs
	else
		ifconfig $RawEthLan hw ether $landefmac
		ifconfig $RawEthWan hw ether $wandefmac
		ifconfig $RawEthLan up
		ifconfig $RawEthWan up
		vconfig add $RawEthLan 1 && ifconfig $RawEthLan.1 down
		vconfig add $RawEthWan 2 && ifconfig $RawEthWan.2 down
		ip link set dev $RawEthLan.1 name ethlan
		ip link set dev $RawEthWan.2 name ethwan
	fi
	Switch_set

	brctl addif br0 ethlan

	# SFP+ Interface
	ifconfig eth0 hw ether $sfpdefmac
	brctl addif br0 eth0
	ifconfig eth0 up

	brctl addif br0 ethwan
	ifconfig br0 hw ether $landefmac
	sw_configvlan "apmode"
}

normal_create_brs_and_vifs()
{
	local landefmac=$($CONFIG get lan_factory_mac)
	local wandefmac=$($CONFIG get wan_factory_mac)
	local sfpdefmac=$($CONFIG get sfp_factory_mac)

	if [ -n "$RawEth" ]; then
		ifconfig $RawEth hw ether $landefmac
		ifconfig $RawEth up
		vconfig add $RawEth 0 && ifconfig $RawEth.0 down
		vconfig add $RawEth 1 && ifconfig $RawEth.1 down
		ip link set dev $RawEth.0 name ethlan
		ip link set dev $RawEth.1 name ethwan
	elif [ "$enable_bond" = "1" ]; then
		bond_create_ifs
	else
		ifconfig $RawEthLan hw ether $landefmac
		ifconfig $RawEthWan hw ether $wandefmac
		ifconfig $RawEthLan up
		ifconfig $RawEthWan up
		vconfig add $RawEthLan 1 && ifconfig $RawEthLan.1 down
		vconfig add $RawEthWan 2 && ifconfig $RawEthWan.2 down
		ip link set dev $RawEthLan.1 name ethlan
		ip link set dev $RawEthWan.2 name ethwan
	fi

	Switch_set
	brctl addif br0 ethlan

	ifconfig eth0 hw ether $sfpdefmac

	# WAN Preference: 0 - ethwan as WAN port; 1 - eth0 as WAN port
	if [ "$($CONFIG get wan_preference)" = "0" ]; then
		# SFP+ LAN Interface
		brctl addif br0 eth0
		ifconfig eth0 up

		ifconfig br0 hw ether $landefmac
		brctl addif brwan ethwan
	else
		# Since WAN port is SFP+ interface, use ethwan as LAN interface
		brctl addif br0 ethwan
		ifconfig ethwan up

		# SFP+ WAN Interface
		brctl addif brwan eth0
		ifconfig br0 hw ether $landefmac
	fi

	sw_configvlan "normal"
}

iptv_create_brs_and_vifs()
{
	local landefmac=$($CONFIG get lan_factory_mac)
	local wandefmac=$($CONFIG get wan_factory_mac)
	local sfpdefmac=$($CONFIG get sfp_factory_mac)
	local iptv_vlan_enable=$($CONFIG get iptv_vlan_enable)
	local iptv_vlan=$($CONFIG get iptv_vlan)

	if [ -n "$RawEth" ]; then
		ifconfig $RawEth hw ether $landefmac
		ifconfig $RawEth up
		vconfig add $RawEth 0 && ifconfig $RawEth.0 down
		vconfig add $RawEth 1 && ifconfig $RawEth.1 down
		ip link set dev $RawEth.0 name ethlan
		ip link set dev $RawEth.1 name ethwan
	elif [ "$enable_bond" = "1" ]; then
		bond_create_ifs
	else
		ifconfig $RawEthLan hw ether $landefmac
		ifconfig $RawEthWan hw ether $wandefmac
		if [ "$WanIndepPhy" = "0" ]; then
			ifconfig $RawEthLan up
			ifconfig $RawEthWan up

			if [ "$iptv_vlan_enable" = "1" ]; then
				vconfig add $RawEthLan 1 && ifconfig $RawEthLan.1 down
				vconfig add $RawEthWan $iptv_vlan && ifconfig $RawEthWan.$iptv_vlan down
				ip link set dev $RawEthLan.1 name ethlan
				#ip link set dev $RawEthWan.$iptv_vlan name ethwan
			else
				vconfig add $RawEthLan 1 && ifconfig $RawEthLan.1 down
				vconfig add $RawEthWan 2 && ifconfig $RawEthWan.2 down
				ip link set dev $RawEthLan.1 name ethlan
				ip link set dev $RawEthWan.2 name ethwan
			fi
		else
			ifconfig $RawEthLan up
			vconfig add $RawEthLan 0 && ifconfig $RawEthLan.0 down
			vconfig add $RawEthLan 1 && ifconfig $RawEthLan.1 up
			brctl addif brwan $RawEthLan.1
			ip link set dev $RawEthLan.0 name ethlan
			ip link set dev $RawEthWan name ethwan
		fi
	fi

	Switch_set
	brctl addif br0 ethlan

	# SFP+ Interface
	ifconfig eth0 hw ether $sfpdefmac

	if [ "$iptv_vlan_enable" = "1" ]; then
		vlan_create_internet_vif 10 0 
	else
		# WAN Preference: 0 - ethwan as WAN port; 1 - eth0 as WAN port
		if [ "$($CONFIG get wan_preference)" = "0" ]; then
			brctl addif br0 eth0
			ifconfig eth0 up
			ifconfig br0 hw ether $landefmac

			brctl addif brwan ethwan
		else
			# Since WAN port is SFP+ interface, use ethwan as LAN interface
			brctl addif br0 ethwan
			ifconfig ethwan up
			ifconfig br0 hw ether $landefmac

			brctl addif brwan eth0
			ifconfig eth0 up
		fi
	fi

	sw_configvlan "iptv" $($CONFIG get iptv_mask)
}

vlan_set_vif_pri() # $1: vif, $2: pri
{
	local p

	for p in 0 1 2 3 4 5 6 7; do
		vconfig set_ingress_map $1 $p $p
		vconfig set_egress_map $1 $p $2
	done
}

nif_existed() # $1: nif
{
	ifconfig $1 >/dev/null 2>&1
}

vlan_create_br_and_vif() # $1: vid, $2: pri
{
	local brx="br$1"
	local wandefmac=$($CONFIG get wan_factory_mac)
	local sfpdefmac=$($CONFIG get sfp_factory_mac)

	nif_existed $brx && return

	br_create $brx
	if [ -n "$RawEth" ]; then
		vconfig add $RawEth $1 && ifconfig $RawEth.$1 up
		brctl addif $brx $RawEth.$1
		vlan_set_vif_pri $RawEth.$1 $2
	elif [ "$enable_bond" = "1" ]; then
		vconfig add $BondEth $1 && ifconfig $BondEth.$1 up
		brctl addif $brx $BondEth.$1
		vlan_set_vif_pri $BondEth.$1 $2
	else
		if [ "$($CONFIG get wan_preference)" = "0" ]; then
		#if [ "$WanIndepPhy" = "0" ]; then
		#	vconfig add ethlan $1 && ifconfig ethlan.$1 up
		#	vconfig add $RawEthWan $1 && ifconfig $RawEthWan.$1 up
		#	brctl addif $brx ethlan.$1
		#	brctl addif $brx $RawEthWan.$1
		#else
			vconfig add $RawEthLan $1 && ifconfig $RawEthLan.$1 up
			vconfig add $RawEthWan $1 && ifconfig $RawEthWan.$1 up
			brctl addif $brx $RawEthLan.$1
			brctl addif $brx $RawEthWan.$1
		#fi
			vlan_set_vif_pri $RawEthWan.$1 $2
			vlan_set_vif_pri $RawEthLan.$1 $2
			ifconfig $brx hw ether $wandefmac	
			brctl stp $brx on
		else
			vconfig add $RawEthLan $1 && ifconfig $RawEthLan.$1 up
			vconfig add $RawSfp $1 && ifconfig $RawSfp.$1 up
			brctl addif $brx $RawEthLan.$1
			brctl addif $brx $RawSfp.$1
			vlan_set_vif_pri $RawSfp.$1 $2
			vlan_set_vif_pri $RawEthLan.$1 $2
			ifconfig $brx hw ether $sfpdefmac
			brctl stp $brx on
		fi
	fi
	ifconfig $brx up
}

vlan_create_internet_vif() # $1: vid, $2: pri
{
	local brx="br$1"
	local landefmac=$($CONFIG get lan_factory_mac)

	if nif_existed $brx; then
		ifconfig $brx down
		if [ -n "$RawEth" ]; then
			ifconfig $RawEth.$1 down && brctl delif $brx $RawEth.$1
			ip link set dev $RawEth.$1 name ethwan
		elif [ "$enable_bond" = "1" ]; then
			ifconfig $BondEth.$1 down && brctl delif $brx $BondEth.$1
			ip link set dev $BondEth.$1 name ethwan
		else
			if [ "$WanIndepPhy" = "0" ]; then
				ifconfig $RawEthWan.$1 down && brctl delif $brx $RawEthWan.$1
			else
				ifconfig $RawEthLan.$1 down && brctl delif $brx $RawEthLan.$1
				ifconfig $RawEthWan.$1 down && brctl delif $brx $RawEthWan.$1
				brctl addif brwan $RawEthLan.$1
			fi
			ip link set dev $RawEthWan.$1 name ethwan
		fi
		brctl delbr $brx
	else
		if [ -n "$RawEth" ]; then
			vconfig add $RawEth $1 && ifconfig $RawEth.$1 down
			ip link set dev $RawEth.$1 name ethwan
		elif [ "$enable_bond" = "1" ]; then
			vconfig add $BondEth $1 && ifconfig $BondEth.$1 down
			ip link set dev $BondEth.$1 name ethwan
		else
			vconfig add $RawEthWan $1 && ifconfig $RawEthWan.$1 down
			ip link set dev $RawEthWan.$1 name ethwan
		fi
	fi

	# WAN Preference(WAN interface): 0 - ethwan; 1 - sfpwan(eth0.vid)
	if [ "$($CONFIG get wan_preference)" = "0" ]; then
		brctl addif br0 eth0
		ifconfig eth0 up
		ifconfig br0 hw ether $landefmac

		vlan_set_vif_pri ethwan $2
		brctl addif brwan ethwan
	else
		brctl addif br0 ethwan
		ifconfig ethwan up
		ifconfig br0 hw ether $landefmac

		# While VLAN is enabled for bridge group, add eth0 into brwan
		if [ "$($CONFIG get vlan_type)" = "0" -a "$($CONFIG get iptv_vlan_enable)" = "1" ]; then
			brctl addif brwan eth0
			ifconfig eth0 up
		else
			ifconfig eth0 up
			vconfig add eth0 $1 && ifconfig eth0.$1 down
			ip link set dev eth0.$1 name sfpwan

			vlan_set_vif_pri sfpwan $2
			brctl addif brwan sfpwan
		fi
	fi
}

vlan_create_intranet_vif() # $1: vid, $2: pri
{
	local brx="br$1"

	nif_existed $brx && return

	br_create $brx
	if [ -n "$RawEth" ]; then
		vconfig add $RawEth $1 && ifconfig $RawEth.$1 up
		brctl addif $brx $RawEth.$1
		vlan_set_vif_pri $RawEth.$1 $2
	elif [ "$enable_bond" = "1" ]; then
		vconfig add $BondEth $1 && ifconfig $BondEth.$1 up
		brctl addif $brx $BondEth.$1
		vlan_set_vif_pri $BondEth.$1 $2
	else
		vconfig add $RawEthWan $1 && ifconfig $RawEthWan.$1 up
		brctl addif $brx $RawEthWan.$1
		vlan_set_vif_pri $RawEthWan.$1 $2
	fi
	ifconfig $brx up
}

vlan_create_isp_iptv_vif() # $1: vid, $2: pri
{
	local vid=$1
	local pri=$2
	local brx="brotv"

	if nif_existed $brx; then
		echo "*** brotv is existed."
	else
		br_create $brx
	fi

	vconfig add $RawEthWan $vid && ifconfig $RawEthWan.$vid up
	vlan_set_vif_pri $RawEthWan.$vid $pri
	brctl addif $brx $RawEthWan.$vid
}

vlan_get_freevid() # $1: up / down
{
	local updown=$1
	local i tv vids
	local freevid

	for i in 0 1 2 3 4 5 6 7 8 9 10; do
		tv=$($CONFIG get vlan_tag_$i)
		[ -n "$tv" ] || continue
		set - $(echo $tv)
		# $1: enable, $2: name, $3: vid, $4: pri, $5:wports, $6:wlports
		[ "$1" = "1" ] || continue
		[ "$2" = "Internet" -o "$2" = "Intranet" ] && continue
		[ "$vids" = "" ] && vids="x${3}x" || vids="$vids x${3}x"
	done

	[ "$updown" = "up" ] && freevid=1 || freevid=4094
	while true; do
		echo $vids | grep -q "x${freevid}x" || break
		[ "$updown" = "up" ] && freevid=$(($freevid + 1)) || freevid=$(($freevid - 1))
	done
	echo $freevid
}

vlan_get_lanvid()
{
	vlan_get_freevid up
}

vlan_get_wanvid()
{
	vlan_get_freevid down
}

vlan_create_brs_and_vifs()
{
	local landefmac=$($CONFIG get lan_factory_mac)
	local wandefmac=$($CONFIG get wan_factory_mac)

	local sfpdefmac=$($CONFIG get sfp_factory_mac)
	local lanvid=$(vlan_get_lanvid)

	local firmware_region=`cat /tmp/firmware_region | awk '{print $1}'`
	local ru_feature=0

	local vlan_enable_bridge=$($CONFIG get enable_orange)
	local enable_spvoda_iptv=$($CONFIG get spain_voda_iptv)

	if [ "x$firmware_region" = "xWW" ] || [ "x$firmware_region" = "x" ] ;then
		if [ "x$($CONFIG get GUI_Region)" = "xRussian" ] ;then
			ru_feature=1
		fi
	elif [ "x$firmware_region" = "xRU" ] ;then
		ru_feature=1
	fi

	local i tv used_wports=0
	local i_vid i_pri

	if [ -n "$RawEth" ]; then
		ifconfig $RawEth hw ether $landefmac
		ifconfig $RawEth up
		vconfig add $RawEth 0 && ifconfig $RawEth.0 down
		ip link set dev $RawEth.0 name ethlan
	elif [ "$enable_bond" = "1" ]; then
		bond_init
		[ "x$wandefmac" = "x" ] && wandefmac=$($CONFIG get wan_factory_mac)
		ifconfig $RawEthLan hw ether $landefmac
		ifconfig $RawEthWan hw ether $landefmac
		ifconfig $BondEth hw ether $landefmac
		ifconfig $BondEth up
		vconfig add $BondEth $lanvid && ifconfig $BondEth.$lanvid down
		ip link set dev $BondEth.$lanvid name ethlan
	else
		ifconfig $RawEthLan hw ether $landefmac
		ifconfig $RawEthWan hw ether $wandefmac
		#if [ "$WanIndepPhy" = "0" ]; then
		#	ip link set dev $RawEthLan name ethlan
		#else
		ifconfig $RawEthLan up
		if [ "$vlan_enable_bridge" = "1" ]; then
			vconfig add $RawEthLan 1 && ifconfig $RawEthLan.1 down
			ip link set dev $RawEthLan.1 name ethlan
		else
			vconfig add $RawEthLan $lanvid && ifconfig $RawEthLan.$lanvid down
			ip link set dev $RawEthLan.$lanvid name ethlan
		fi
		#fi
		ifconfig $RawEthWan up
	fi
	Switch_set
	brctl addif br0 ethlan
	ifconfig br0 hw ether $landefmac

	# SFP+ Interface
	ifconfig eth0 hw ether $sfpdefmac

	sw_configvlan "vlan" "start"
	for i in 0 1 2 3 4 5 6 7 8 9 10; do
		tv=$($CONFIG get vlan_tag_$i)
		[ -n "$tv" ] || continue
		set - $(echo $tv)
		# $1: enable, $2: name, $3: vid, $4: pri, $5:wports, $6:wlports
		[ "$1" = "1" ] || continue
		if [ "$2" = "Internet" ]; then 
			i_vid=$3
			i_pri=$4
		elif [ "$2" = "Intranet" -a "$ru_feature" = "1" ]; then
				vlan_create_intranet_vif $3 $4
				sw_configvlan "vlan" "add" "br" "$3" "0" "$4"
		else
			used_wports=$(($used_wports | $5))
			if [ "$vlan_enable_bridge" = "1" ]; then
				# Orange VOD: VLAN ID 838; IPTV: VLAN ID 840
				if [ "$3" = "838" -o "$3" = "840" ]; then
					 vlan_create_isp_iptv_vif "838" "4"
					 sw_configvlan "vlan" "add" "br" "838" "0" "4"
					 vlan_create_isp_iptv_vif "840" "5"
					 sw_configvlan "vlan" "add" "br" "840" "0" "5"
				fi
			elif [ "$enable_spvoda_iptv" = "1" -a "$3" = "105" ]; then
				vlan_create_isp_iptv_vif "105" "0"
				sw_configvlan "vlan" "add" "br" "105" "0" "0"
			else
				vlan_create_br_and_vif $3 $4
				sw_configvlan "vlan" "add" "vlan" $3 $5 $4
			fi
		fi
	done
	if [ "$i_vid" = "0" ]; then
		i_vid=$(vlan_get_wanvid) && i_pri=0
		vlan_create_internet_vif $i_vid $i_pri
		sw_configvlan "vlan" "add" "wan" "$i_vid" "0" "$i_pri"
	else
		vlan_create_internet_vif $i_vid $i_pri
		sw_configvlan "vlan" "add" "br" "$i_vid" "0" "$i_pri"
	fi
	# while Spain Voda IPTV is enabled, all LAN ports can be used as IPTV port.
	if [ "$enable_spvoda_iptv" = "1" ]; then
		used_wports=0
	fi
	sw_configvlan "vlan" "add" "lan" "$lanvid" $(($used_wports ^ 0x3f)) "0"
	sw_configvlan "vlan" "end"
}

op_create_brs_and_vifs()
{
	local opmode=$($CONFIG get i_opmode)

	op_del_all_brs_vifs  2>/dev/null
	case "$opmode" in
	normal) normal_create_brs_and_vifs ;;
	iptv) iptv_create_brs_and_vifs ;;
	vlan) vlan_create_brs_and_vifs ;;
	factory) factory_mode_create_brs_and_vifs ;;
	*) apmode_create_brs_and_vifs ;;
	esac
}

fi #-------------------- this must be the last line -----------------------------
