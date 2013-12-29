
#ifdef HAVE_PRIVOXY
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>

void start_privoxy(void)
{
	if (!nvram_match("privoxy_enable", "1"))
		return;

	int mode = 0;
	char *ip = nvram_safe_get("lan_ipaddr");

	sysprintf("grep -q nobody /etc/passwd || echo \"nobody:*:65534:65534:nobody:/var:/bin/false\" >> /etc/passwd");
	mkdir("/var/log/privoxy", 0777);
	
	char *wan = get_wan_ipaddr();
	if (nvram_match("privoxy_transp_enable", "1")) {
		sysprintf("iptables -t nat -D PREROUTING -p tcp -d ! %s --dport 80 -j DNAT --to %s:8118", wan, ip);
		sysprintf("iptables -t nat -I PREROUTING -p tcp -d ! %s --dport 80 -j DNAT --to %s:8118", wan, ip);
		mode = 1;
	}

	FILE *fp = fopen("/tmp/privoxy.conf", "wb");

	if (nvram_match("privoxy_advanced", "1") && nvram_invmatch("privoxy_conf", "")) {
		fprintf(fp, "%s", nvram_safe_get("privoxy_conf"));
	} else {
		fprintf(fp, "confdir /etc/privoxy\n"
			"logdir /var/log/privoxy\n"
			"actionsfile match-all.action\n"
			"actionsfile default.action\n"
			"actionsfile user.action\n"
			"filterfile default.filter\n"
			"logfile logfile\n"
			"listen-address  %s:8118\n"
			"toggle  1\n"
			"enable-remote-toggle  0\n"
			"enable-remote-http-toggle  0\n"
			"enable-edit-actions 0\n"
			"buffer-limit 4096\n"
			"accept-intercepted-requests %d\n" 
			"split-large-forms 0\n" 
			"keep-alive-timeout 5\n" 
			"socket-timeout 300\n" 
			"max-client-connections 64\n" 
			"handle-as-empty-doc-returns-ok 1\n", ip, mode);
	}
	fclose(fp);
	eval("privoxy", "/tmp/privoxy.conf");
	syslog(LOG_INFO, "Privoxy : privoxy started\n");
	return;
}

void stop_privoxy(void)
{
	char *wan = get_wan_ipaddr();
	sysprintf("iptables -t nat -D PREROUTING -p tcp -d ! %s --dport 80 -j REDIRECT --to-port 8118", wan);
	stop_process("privoxy", "privoxy");
}
#endif
