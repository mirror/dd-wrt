#!/bin/sh

#------------------------------------------------------------------
# © 2016 Belkin International, Inc. and/or its affiliates. All rights reserved.
#------------------------------------------------------------------

#source /etc/init.d/interface_functions.sh
#source /etc/init.d/ulog_functions.sh

# get the relevant syscfg parameters
#PARAM=`utctx_cmd get lan_ifname lan_ethernet_physical_ifnames lan_mac_addr wan_physical_ifname wan_mac_addr bridge_mode ipv6::passthrough_enable ipv6::passthrough_done_in_hw`
#eval $PARAM

#WRAITH-128
#ssdk_sh debug reg set 0x624 0x7f7f7f 4 > /dev/null 2>&1 #global forward control 1 reg

#ssdk_sh debug reg set 0x620 0x1004f0 4 > /dev/null 2>&1 #global forward control 0 reg
#ssdk_sh debug reg set 0x200 0x31 4 > /dev/null 2>&1 # drop frame when IPv4 header length check fails
#ssdk_sh debug reg set 0x204 0x5800 4 > /dev/null 2>&1 # drop the ICMP checksum error
#acl_rule_index=1
#acl_rule_index=`printf '%02x\n' $acl_rule_index`

## QCA case #04937005 -- The MTU of LAN and WAN (eth0.1, eth0.2) can't be 1500.
## config ipq5018 internal MAC frameMaxSize to 0x800 (2048) instead of 0x5ee (1518) for VLAN tagged packets.
echo 0 > /sys/ssdk/dev_id ;

ssdk_sh port frameMaxSize set 2 0x800 ;

# QCA case #05153007 enable flowctrl to prevent low performance of PPTP connection with Cisco 7301.
ssdk_sh port flowctrlforcemode set 2 enable
ssdk_sh port flowctrl set 2 enable

## config port.5 to VLAN(1) and port.1/2/3/4 to VLAN(2)
#
echo 1 > /sys/ssdk/dev_id

ssdk_sh vlan entry flush

        # ssdk_sh vlan entry create 1
        # ssdk_sh vlan member add 1 6 tagged
        # ssdk_sh vlan member add 1 2 untagged
        # ssdk_sh portvlan defaultcVid set 2 1
        # ssdk_sh portvlan ingress set 2 secure

        # ssdk_sh vlan entry create 2
        # ssdk_sh vlan member add 2 6 tagged
        # ssdk_sh vlan member add 2 3 untagged
        # ssdk_sh vlan member add 2 4 untagged
        # ssdk_sh vlan member add 2 5 untagged
        # ssdk_sh portvlan defaultcVid set 3 2
        # ssdk_sh portvlan defaultcVid set 4 2
        # ssdk_sh portvlan defaultcVid set 5 2
        # ssdk_sh portvlan ingress set 3 secure
        # ssdk_sh portvlan ingress set 4 secure
        # ssdk_sh portvlan ingress set 5 secure

        # ssdk_sh portvlan defaultcVid set 6 2
        # ssdk_sh portvlan ingress set 6 secure

ssdk_sh vlan entry append 1 1 6,4 6 4 default default default;
ssdk_sh vlan entry append 2 2 6,1,2,3 6 1,2,3 default default default;

ssdk_sh portVlan ingress set 1 fallback;
ssdk_sh portVlan ingress set 2 fallback;
ssdk_sh portVlan ingress set 3 fallback;
ssdk_sh portVlan ingress set 4 fallback;
ssdk_sh portVlan ingress set 5 fallback;
ssdk_sh portVlan ingress set 6 fallback;

ssdk_sh portVlan defaultSVid set 1 2;
ssdk_sh portVlan defaultSVid set 2 2;
ssdk_sh portVlan defaultSVid set 3 2;
ssdk_sh portVlan defaultSVid set 4 1;
ssdk_sh portVlan defaultSVid set 6 1;
ssdk_sh portVlan egress set 1 unmodified;
ssdk_sh portVlan vlanPropagation set 1 disable;
ssdk_sh portVlan tlsMode set 1 enable;

ssdk_sh portVlan egress set 2 unmodified;
ssdk_sh portVlan vlanPropagation set 2 disable;
ssdk_sh portVlan tlsMode set 2 enable;

ssdk_sh portVlan egress set 3 unmodified;
ssdk_sh portVlan vlanPropagation set 3 disable;
ssdk_sh portVlan tlsMode set 3 enable;

ssdk_sh portVlan egress set 4 unmodified;
ssdk_sh portVlan vlanPropagation set 4 disable;
ssdk_sh portVlan tlsMode set 4 enable;

ssdk_sh portVlan egress set 5 unmodified;
ssdk_sh portVlan vlanPropagation set 5 disable;
ssdk_sh portVlan tlsMode set 5 enable;

ssdk_sh portVlan egress set 6 tagged;
ssdk_sh portVlan qinqrole set 6 core;
ssdk_sh portVlan vlanPropagation set 6 replace;
ssdk_sh portVlan tlsMode set 6 disable;

ssdk_sh portVlan qinqmode set stag;
ssdk_sh portVlan svlanTPID set 0x8100;

usleep 50000;

# create vlan group 
#if [ "`ifconfig eth0.1 2>/dev/null | grep HWaddr | cut -d' ' -f1`" != "eth0.1" ]; then
#
## config eth0.1 and eth0.2 by vconfig with VLAN ID 1 and 2
#
#    ifconfig eth0 up;
#    ifconfig eth1 up;
    #vconfig set_name_type VLAN_PLUS_VID_NO_PAD;
#    vconfig add eth1 1; # vlan1(WAN)
#    vconfig add eth1 2; # vlan2(LAN)
#fi

# Due to ssdk_init.c disable port(5), so it needs to enable port(5) after vlan is ready.
ssdk_sh port poweron set 5
usleep 50000;



#if [ "$SYSCFG_ipv6_passthrough_enable" = "1" ] && [ "$SYSCFG_ipv6_passthrough_done_in_hw" = 1 ]; then
#    brctl addif $SYSCFG_lan_ifname $SYSCFG_wan_physical_ifname
#    echo $SYSCFG_wan_physical_ifname > /proc/sys/nss/bridge_mgr/add_wanif
#fi

# prevent switch from dropping packets with TTL == 0.
ssdk_sh sec l3parser set 2 2

#there are some garbage fdb entries produced during switch initialization, flush them
ssdk_sh fdb entry flush 0 > /dev/null 2>&1
#ulog vlan tagging "ACL has $acl_rule_index rules"

# for QCA8337
# drop invalid tcp
#ssdk_sh debug reg set 0x200 0x2000 4
# drop tcp/udp checksum errors
#ssdk_sh debug reg set 0x204 0x0842 4
# enable pppoe
#ssdk_sh debug reg set 0x214 0x2000000 4

# QCA case #05153007 enable flowctrl to prevent low performance of PPTP connection with Cisco 7301.
ssdk_sh port flowctrlforcemode set 6 enable
ssdk_sh port flowctrl set 6 enable

#set igmp snooping configurations
#ssdk_sh igmp portJoin set 1 enable > /dev/null 2>&1
#ssdk_sh igmp portJoin set 2 enable > /dev/null 2>&1
#ssdk_sh igmp portJoin set 3 enable > /dev/null 2>&1
#ssdk_sh igmp portJoin set 4 enable > /dev/null 2>&1
#ssdk_sh igmp portLeave set 1 enable > /dev/null 2>&1
#ssdk_sh igmp portLeave set 2 enable > /dev/null 2>&1
#ssdk_sh igmp portLeave set 3 enable > /dev/null 2>&1
#ssdk_sh igmp portLeave set 4 enable > /dev/null 2>&1
#ssdk_sh igmp createStatus set enable > /dev/null 2>&1
#ssdk_sh igmp version3 set enable > /dev/null 2>&1
#if [ "$SYSCFG_bridge_mode" != "0" ]; then
#    ssdk_sh igmp portJoin set 5 enable > /dev/null 2>&1
#    ssdk_sh igmp portLeave set 5 enable > /dev/null 2>&1
#    ssdk_sh igmp portJoin set 6 enable > /dev/null 2>&1
#    ssdk_sh igmp portLeave set 6 enable > /dev/null 2>&1
#    ssdk_sh igmp rp set 0x60 > /dev/null 2>&1
#else
#    ssdk_sh igmp portJoin set 5 disable > /dev/null 2>&1
#    ssdk_sh igmp portLeave set 5 disable > /dev/null 2>&1
#    ssdk_sh igmp portJoin set 6 disable > /dev/null 2>&1
#    ssdk_sh igmp portLeave set 6 disable > /dev/null 2>&1
#    ssdk_sh igmp rp set 0x40 > /dev/null 2>&1
#fi
