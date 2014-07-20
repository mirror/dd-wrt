#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
#include <net/route.h>
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>
#include <syslog.h>
#include <services.h>
#include <time.h>



void start_dhcp6c() {
	FILE *fp, *fpc;
	int prefix_len;
	unsigned char ea[ETHER_ADDR_LEN];
	unsigned long iaid = 0;
	struct {
		uint16 type;
		uint16 hwtype;
	} __attribute__ ((__packed__)) duid;
	uint16 duid_len = 0;

	if(!nvram_match("ipv6_typ", "ipv6pd"))
		return 0;

/*	nvram_set("ipv6_get_dns", "");
	nvram_set("ipv6_get_domain", "");
	if (nvram_get_int("ipv6_dhcp_pd")) {
		nvram_set("ipv6_rtr_addr", "");
		nvram_set("ipv6_prefix", "");
	}*/

	prefix_len = 64 - ( atoi(nvram_get("ipv6_pf_len")) ? : 64);
	if (prefix_len < 0)
		prefix_len = 0;

	if (ether_atoe(nvram_safe_get("et0macaddr"), ea)) {
		/* Generate IAID from the last 7 digits of WAN MAC */
		iaid =	((unsigned long)(ea[3] & 0x0f) << 16) |
			((unsigned long)(ea[4]) << 8) |
			((unsigned long)(ea[5]));

		/* Generate DUID-LL */
		duid_len = sizeof(duid) + ETHER_ADDR_LEN;
		duid.type = htons(3);	/* DUID-LL */
		duid.hwtype = htons(1);	/* Ethernet */
	}

	/* Create dhcp6c_duid */
	unlink("/var/dhcp6c_duid");
	if ((duid_len != 0) &&
	    (fp = fopen("/var/dhcp6c_duid", "w")) != NULL) {
		fwrite(&duid_len, sizeof(duid_len), 1, fp);
		fwrite(&duid, sizeof(duid), 1, fp);
		fwrite(&ea, ETHER_ADDR_LEN, 1, fp);
		fclose(fp);
	}
	if ((fpc = fopen("/etc/dhcp6c.conf", "w"))) {
		fprintf(fpc,
			"interface %s {\n"
			" send ia-pd 0;\n"
			" send rapid-commit;\n"
			" request domain-name-servers;\n"
			" script \"/sbin/dhcp6c-state\";\n"
			"};\n"
			"id-assoc pd 0 {\n"
			" prefix-interface %s {\n"
			"  sla-id 0;\n"
			"  sla-len %d;\n"
			" };\n"
			"};\n"
			"id-assoc na 0 { };\n",
			nvram_safe_get("wan_ifname"),
			nvram_safe_get("lan_ifname"),
			prefix_len);
		fclose(fpc);
	}


	///tmp/strace dhcp6c -c /tmp/dhcp6c.conf -T LL -D vlan2
	eval("dhcp6c", "-c" , "/tmp/dhcp6c.conf" , "-T", "LL", "-D" , nvram_safe_get("wan_ifname"));
}

void stop_dhcp6c() {
      stop_process("dhcp6c", "dhcp6c");
}
