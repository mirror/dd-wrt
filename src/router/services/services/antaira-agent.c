#ifdef HAVE_ANTAIRA_AGENT

#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

char *antaira_agent_deps(void)
{
	return "antaira_agent_enable antaira_agent_wan_ifname antaira_agent_cloud_url \
		antaira_agent_connect_url antaira_agent_configuration_url antaira_agent_token";
}

char *antaira_agent_proc(void)
{
	return "antaira-quick-vpn-agent";
}

void start_antaira_agent(void)
{
	char wan_if_buffer[33];
	char *antaira_agent_argv[] = { "antaira-quick-vpn-agent", NULL };

	stop_antaira_agent();

	if (!nvram_invmatchi("antaira_agent_enable", 0))
		return;
	if (nvram_match("wan_proto", "disabled") || !*(safe_get_wan_face(wan_if_buffer))) // todo: add upstream
		return;

	_log_evalpid(antaira_agent_argv, NULL, 0, NULL);

	cprintf("done\n");
	return;
}

void stop_antaira_agent(void)
{
	stop_process("antaira-quick-vpn-agent", "antaira-agent");

	nvram_delstates(antaira_agent_deps());

	cprintf("done\n");
	return;
}

#endif //HAVE_ANTAIRA_AGENT
