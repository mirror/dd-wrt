. ./enet.sh

	sw_user_lan_ports_vlan_config "2" "1" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "3" "2" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "4" "3" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "5" "4" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "6" "5" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "7" "6" "0" "0" "0" "normal_lan"
	sw_user_lan_ports_vlan_config "1" "" "0" "1" "0" "wan"
