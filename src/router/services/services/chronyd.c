#ifdef HAVE_CHRONY

#include <stdio.h>
#include <unistd.h>

#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char *chronyd_deps(void)
{
	return "chronyd_enable chronyd_conf";
}

char *chronyd_proc(void)
{
	return "chronyd";
}

void stop_chronyd(void)
{
	stop_process("chronyd", "chronyd daemon");
	nvram_delstates(chronyd_deps());

	return;
}

static int write_default_config(FILE *fp)
{
	int valid;
	char cidr[24];
	struct in_addr addr, mask;

	valid = inet_aton(nvram_safe_get("lan_ipaddr"), &addr);
	if (valid == -EINVAL) {
		fprintf(stderr, "invalid address\n");
		return 1;
	}
	valid = inet_aton(nvram_safe_get("lan_netmask"), &mask);
	if (valid == -EINVAL) {
		fprintf(stderr, "invalid mask\n");
		return 1;
	}
	inet_addr_to_cidr(addr, mask, cidr);

	fprintf(fp, "pool pool.ntp.org offline\n");
	fprintf(fp, "driftfile /var/lib/chrony/drift\n");
	fprintf(fp, "makestep 1.0 3\n");
	fprintf(fp, "local stratum 8\n");
	fprintf(fp, "noclientlog\n");
	fprintf(fp, "allow %s\n", cidr);

	return 0;
}

void start_chronyd(void)
{
	int ret = 0;
	char *chronyd_argv[] = { "chronyd", NULL };

	stop_chronyd();
	if (!nvram_invmatchi("chronyd_enable", 0))
		return;

	FILE *fp = fopen("/tmp/chrony.conf", "w+");
	if (fp == NULL)
		return;

	if (nvram_invmatch("chronyd_conf", ""))
		fprintf(fp, nvram_get("chronyd_conf"));
	else
		ret = write_default_config(fp);
	fclose(fp);

	if (!ret)
		_log_evalpid(chronyd_argv, NULL, 0, NULL);
}

#endif
