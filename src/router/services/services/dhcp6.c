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

void start_dhcp6c(void)
{
	FILE *fp, *fpc;
	char *buf;
	int prefix_len;
	char ea[ETHER_ADDR_LEN];
	unsigned long iaid = 0;
	struct {
		uint16 type;
		uint16 hwtype;
	} __attribute__((__packed__)) duid;
	uint16 duid_len = 0;

	if (!nvram_match("ipv6_typ", "ipv6pd"))
		return;
	nvram_unset("ipv6_prefix");
	char mac[18];
	getLANMac(mac);
	if (strlen(mac) == 0)
		strcpy(mac, nvram_safe_get("et0macaddr_safe"));

	if (ether_atoe(mac, ea)) {
		/* Generate IAID from the last 7 digits of WAN MAC */
		iaid = ((unsigned long)(ea[3] & 0x0f) << 16) | ((unsigned long)(ea[4]) << 8) | ((unsigned long)(ea[5]));

		/* Generate DUID-LL */
		duid_len = sizeof(duid) + ETHER_ADDR_LEN;
		duid.type = htons(3);	/* DUID-LL */
		duid.hwtype = htons(1);	/* Ethernet */
	}

	unlink("/var/dhcp6c_duid");
	if ((duid_len != 0) && (fp = fopen("/var/dhcp6c_duid", "w")) != NULL) {
		fwrite(&duid_len, sizeof(duid_len), 1, fp);
		fwrite(&duid, sizeof(duid), 1, fp);
		fwrite(&ea, ETHER_ADDR_LEN, 1, fp);
		fclose(fp);
	}

	if (nvram_match("dhcp6c_custom", "1")) {
		buf = nvram_get("dhcp6c_conf");
		if (buf != NULL)
			writenvram("dhcp6c_conf", "/tmp/dhcp6c.conf");
	} else {

		prefix_len = 64 - (atoi(nvram_get("ipv6_pf_len")) ? : 64);
		if (prefix_len < 0)
			prefix_len = 0;

		if ((fpc = fopen("/etc/dhcp6c.conf", "w"))) {
			fprintf(fpc,
				"interface %s {\n"
				" send ia-pd 0;\n"
				" send rapid-commit;\n"
				" request domain-name-servers;\n"
				" script \"/sbin/dhcp6c-state\";\n"
				"};\n" "id-assoc pd 0 {\n" " prefix-interface %s {\n" "  sla-id 0;\n" "  sla-len %d;\n" " };\n" "};\n" "id-assoc na 0 { };\n", get_wan_face(), nvram_safe_get("lan_ifname"), prefix_len);
			fclose(fpc);
		}
	}

	eval("dhcp6c", "-c", "/tmp/dhcp6c.conf", "-T", "LL", get_wan_face());
}

void stop_dhcp6c(void)
{
	stop_process("dhcp6c", "dhcp6c");
}

void start_dhcp6s(void)
{
	FILE *fp;
	pid_t pid;
	const char *p;
	char *buf;
	char ipv6_dns_str[1024] = "";

	char ea[ETHER_ADDR_LEN];
	unsigned long iaid = 0;
	struct {
		uint16 type;
		uint16 hwtype;
	} __attribute__((__packed__)) duid;
	uint16 duid_len = 0;

	if (!nvram_match("ipv6_enable", "1"))
		return;
	if (!nvram_match("dhcp6s_enable", "1"))
		return;

	if (ether_atoe(nvram_safe_get("lan_hwaddr"), ea)) {
		/* Generate IAID from the last 7 digits of WAN MAC */
		iaid = ((unsigned long)(ea[3] & 0x0f) << 16) | ((unsigned long)(ea[4]) << 8) | ((unsigned long)(ea[5]));

		/* Generate DUID-LL */
		duid_len = sizeof(duid) + ETHER_ADDR_LEN;
		duid.type = htons(3);	/* DUID-LL */
		duid.hwtype = htons(1);	/* Ethernet */
	}

	unlink("/var/dhcp6s_duid");
	if ((duid_len != 0) && (fp = fopen("/var/dhcp6s_duid", "w")) != NULL) {
		fwrite(&duid_len, sizeof(duid_len), 1, fp);
		fwrite(&duid, sizeof(duid), 1, fp);
		fwrite(&ea, ETHER_ADDR_LEN, 1, fp);
		fclose(fp);
	}

	if (nvram_match("dhcp6s_custom", "1")) {
		buf = nvram_get("dhcp6s_conf");
		if (buf != NULL)
			writenvram("dhcp6s_conf", "/tmp/dhcp6s.conf");
	} else {

		if ((fp = fopen("/tmp/dhcp6s.conf", "w")) == NULL)
			return;

		fprintf(fp, "option refreshtime %d;\n", 900);	/* 15 minutes for now */
		fprintf(fp, "option domain-name-servers %s", nvram_safe_get("ipv6_get_dns"));
		/* dhcp6s won't start if there are duplicate dns ips */
		if (!strstr(nvram_safe_get("ipv6_get_dns"), nvram_get("ipv6_dns1")))
			fprintf(fp, " %s", nvram_get("ipv6_dns1"));
		if (!strstr(nvram_safe_get("ipv6_get_dns"), nvram_get("ipv6_dns2")))
			fprintf(fp, " %s", nvram_get("ipv6_dns2"));
		fprintf(fp, ";\n");
		if (nvram_invmatch("ipv6_get_domain", ""))
			fprintf(fp, "option domain-name \"%s\";\n", nvram_safe_get("ipv6_get_domain"));
		if (nvram_match("dhcp6s_seq_ips", "1")) {
			dd_syslog(LOG_INFO, "kong: dhcp6c ipv6_prefix: %s\n", nvram_get("ipv6_prefix"));
			fprintf(fp, "\ninterface %s {\n", nvram_get("lan_ifname"));
			fprintf(fp, "\tallow rapid-commit;\n\taddress-pool pool1 30 86400;\n};\n");
			fprintf(fp, "pool pool1 {\n \t range %s1000 to %sffff;\n};\n", nvram_get("ipv6_prefix"), nvram_get("ipv6_prefix"));
		} else {
			fprintf(fp, "\ninterface %s {\n", nvram_get("lan_ifname"));
			fprintf(fp, "\tallow rapid-commit;\n};\n");
		}
		if (nvram_invmatch("dhcp6s_hosts", "")) {
			fprintf(fp, "\n%s\n", nvram_get("dhcp6s_hosts"));
		}
		fclose(fp);
	}

	eval("dhcp6s", "-c", "/tmp/dhcp6s.conf", "-D", nvram_get("lan_ifname"));
}

void stop_dhcp6s(void)
{
	stop_process("dhcp6s", "dhcp6s");
}
