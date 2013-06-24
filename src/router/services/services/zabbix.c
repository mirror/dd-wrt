 
#ifdef HAVE_ZABBIX
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

void start_zabbix(void)
{
	if (!nvram_match("zabbix_enable", "1"))
		return;
	if (pidof("zabbix_agentd") > 0) {
	  //syslog(LOG_INFO, "dlna : minidlna already running\n");
	}else{
	  eval("sh" , "/etc/config/zabbix.startup");
	  syslog(LOG_INFO, "zabbix : monitoring client started\n");
	}
	
	return;
}

void stop_zabbix(void)
{

	stop_process("zabbix_agentd", "zabbix_agentd");
	/* kill it once again since stop_process does not close all instances */
	if (pidof("zabbix_agentd") > 0) {
	  eval("kill", "-9", pidof("zabbix_agentd"));
	}
	
	return;
}
#endif