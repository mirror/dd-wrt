#include <sveaconf.h>

char* get_wdev(void)
{
	if(!check_vlan_support())
	  return "eth2";
	else
	  return "eth1";

}

char* get_ldev(void)
{
	if(!check_vlan_support())
	  return "eth1";
	else
	  return "vlan1";

}

int del_ebtables(void)
{
	char *lan_mac = nvram_safe_get("et0macaddr");
	char *lan_ip = nvram_safe_get("lan_ipaddr");
	char  cmd[1024] = {0};

	snprintf(cmd, 1023, "/usr/sbin/ebtables -t broute -D BROUTING -p IPv4 -i eth0 --ip-dst %s -j DROP 2>&1 > /dev/null", lan_ip);
	system(cmd);

	snprintf(cmd, 1023, "/usr/sbin/ebtables -t broute -D BROUTING -p ARP -i eth0 -d %s -j DROP 2>&1 > /dev/null 2>&1 > /dev/null", lan_mac);
	system(cmd);

	snprintf(cmd, 1023, "/usr/sbin/ebtables -t broute -D BROUTING -p IPv4 -i %s --ip-dst %s -j DROP 2>&1 > /dev/null 2>&1 > /dev/null", get_wdev(), lan_ip);
	system(cmd);

	snprintf(cmd, 1023, "/usr/sbin/ebtables -t broute -D BROUTING -p ARP -i %s -d %s -j DROP 2>&1 > /dev/null 2>&1 > /dev/null", get_wdev(), lan_mac);
	system(cmd);
	
	return 0;

}

int add_ebtables(void)
{
	char *lan_mac = nvram_safe_get("et0macaddr");
	char *lan_ip = nvram_safe_get("lan_ipaddr");
	char  cmd[1024] = {0};

	del_ebtables();

	snprintf(cmd, 1023, "/usr/sbin/ebtables -t broute -A BROUTING -p IPv4 -i eth0 --ip-dst %s -j DROP 2>&1 > /dev/null 2>&1 > /dev/null", lan_ip);
	system(cmd);

	snprintf(cmd, 1023, "/usr/sbin/ebtables -t broute -A BROUTING -p ARP -i eth0 -d %s -j DROP 2>&1 > /dev/null 2>&1 > /dev/null", lan_mac);
	system(cmd);

	snprintf(cmd, 1023, "/usr/sbin/ebtables -t broute -A BROUTING -p IPv4 -i %s --ip-dst %s -j DROP 2>&1 > /dev/null 2>&1 > /dev/null", get_wdev(), lan_ip);
	system(cmd);

	snprintf(cmd, 1023, "/usr/sbin/ebtables -t brouting -A BROUTING -p ARP -i %s -d %s -j DROP 2>&1 > /dev/null 2>&1 > /dev/null", get_wdev(), lan_mac);
	system(cmd);

	return 0;
}
