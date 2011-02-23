/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/aiccu_test.c - AICCU Test function
***********************************************************
 $Author: jeroen $
 $Id: aiccu_test.c,v 1.9 2007-01-15 12:00:46 jeroen Exp $
 $Date: 2007-01-15 12:00:46 $
**********************************************************/

#include "aiccu.h"

#ifndef _WIN32
#define PING4 "ping -c %d -v %s 2>&1"
#define PING6 "ping6 -c %d -v %s 2>&1"
#define TRACEROUTE4 "traceroute %s 2>&1"
#define TRACEROUTE6 "traceroute6 %s 2>&1"
#else
#define PING4 "ping -4 -n %d %s"
#define PING6 "ping -6 -n %d %s"
#define TRACEROUTE4 "tracert %s"
#define TRACEROUTE6 "tracert6 %s"
#endif

void system_arg(const char *fmt, ...);
void system_arg(const char *fmt, ...)
{
	char	buf[1024];
	int	ret;
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	fflush(stdout);
	ret = system(buf);
	if (ret == -1) dolog(LOG_WARNING, "Execution of \"%s\" failed!? (Please check if the command is available)\n", buf);
}

#define PINGCOUNT 3

bool test_ask(bool automatic);
bool test_ask(bool automatic)
{
	char buf[100];

	if (!g_aiccu->running) return false;

	printf("\n######\n");
	printf("\n");

	if (automatic) return true;
	
	printf("Did this work? [Y/n] ");

	if (fgets(buf, sizeof(buf), stdin) == NULL) return false;

	printf("\n");

	return (buf[0] == 'N' || buf[0] == 'n' ? false : true);
}

void aiccu_os_test(struct TIC_Tunnel *hTunnel, bool automatic)
{
	unsigned int t = 1;
	unsigned int tottests = 8;

	/* Make sure we have a correct local IPv4 address for some tests */
	if (strcmp(hTunnel->sType, "6in4-static") != 0)
	{
		heartbeat_socket(NULL, 0, "",
			&hTunnel->sIPv4_Local,
			hTunnel->sIPv4_POP,
			NULL);
	}

	if (!g_aiccu->running) return;

	printf("#######\n");
	printf("####### AICCU Quick Connectivity Test\n");
	printf("#######\n\n");

	printf("####### [%u/%u] Ping the IPv4 Local/Your Outer Endpoint (%s)\n",
		t++, tottests, hTunnel->sIPv4_Local);
	printf("### This should return so called 'echo replies'\n");
	printf("### If it doesn't then check your firewall settings\n");
	printf("### Your local endpoint should always be pingable\n");
	printf("### It could also indicate problems with your IPv4 stack\n");
	printf("\n");
	system_arg(PING4, PINGCOUNT, hTunnel->sIPv4_Local);
	if (!test_ask(automatic) || !g_aiccu->running) return;

	printf("####### [%u/%u] Ping the IPv4 Remote/PoP Outer Endpoint (%s)\n",
		t++, tottests, hTunnel->sIPv4_POP);
	printf("### These pings should reach the PoP and come back to you\n");
	printf("### In case there are problems along the route between your\n");
	printf("### host and the PoP this could not return replies\n");
	printf("### Check your firewall settings if problems occur\n");
	printf("\n");
	system_arg(PING4, PINGCOUNT, hTunnel->sIPv4_POP);
	if (!test_ask(automatic) || !g_aiccu->running) return;

	printf("####### [%u/%u] Traceroute to the PoP (%s) over IPv4\n",
		t++, tottests, hTunnel->sIPv4_POP);
	printf("### This traceroute should reach the PoP\n");
	printf("### In case this traceroute fails then you have no connectivity\n");
	printf("### to the PoP and this is most probably the problem\n");
	printf("\n");
	system_arg(TRACEROUTE4, hTunnel->sIPv4_POP);
	if (!test_ask(automatic) || !g_aiccu->running) return;

	printf("###### [%u/%u] Checking if we can ping IPv6 localhost (::1)\n",
		t++, tottests);
	printf("### This confirms if your IPv6 is working\n");
	printf("### If ::1 doesn't reply then something is wrong with your IPv6 stack\n");
	printf("\n");
	system_arg(PING6, PINGCOUNT, "::1");
	if (!test_ask(automatic) || !g_aiccu->running) return;

	printf("###### [%u/%u] Ping the IPv6 Local/Your Inner Tunnel Endpoint (%s)\n",
		t++, tottests, hTunnel->sIPv6_Local);
	printf("### This confirms that your tunnel is configured\n");
	printf("### If it doesn't reply then check your interface and routing tables\n");
	printf("\n");
	system_arg(PING6, PINGCOUNT, hTunnel->sIPv6_Local);
	if (!test_ask(automatic) || !g_aiccu->running) return;

	printf("###### [%u/%u] Ping the IPv6 Remote/PoP Inner Tunnel Endpoint (%s)\n",
		t++, tottests, hTunnel->sIPv6_POP);
	printf("### This confirms the reachability of the other side of the tunnel\n");
	printf("### If it doesn't reply then check your interface and routing tables\n");
	printf("### Don't forget to check your firewall of course\n");
	printf("### If the previous test was succesful then this could be both\n");
	printf("### a firewalling and a routing/interface problem\n");
	printf("\n");
	system_arg(PING6, PINGCOUNT, hTunnel->sIPv6_POP);
	if (!test_ask(automatic) || !g_aiccu->running) return;

	printf("###### [%u/%u] Traceroute6 to the central SixXS machine (noc.sixxs.net)\n",
		t++, tottests);
	printf("### This confirms that you can reach the central machine of SixXS\n");
	printf("### If that one is reachable you should be able to reach most IPv6 destinations\n");
	printf("### Also check http://www.sixxs.net/ipv6calc/ which should show an IPv6 connection\n");
	printf("### If your browser supports IPv6 and uses it of course.\n");
	printf("\n");
	system_arg(TRACEROUTE6, "noc.sixxs.net");
	if (!test_ask(automatic) || !g_aiccu->running) return;

	printf("###### [%u/%u] Traceroute6 to (www.kame.net)\n",
		t++, tottests);
	printf("### This confirms that you can reach a Japanese IPv6 destination\n");
	printf("### If that one is reachable you should be able to reach most IPv6 destinations\n");
	printf("### You should also check http://www.kame.net which should display\n");
	printf("### a animated kame (turtle), of course only when your browser supports and uses IPv6\n");
	printf("\n");
	system_arg(TRACEROUTE6, "www.kame.net");
	if (!test_ask(automatic) || !g_aiccu->running) return;

	printf("###### ACCU Quick Connectivity Test (done)\n\n");

	printf("### Either the above all works and gives no problems\n");
	printf("### or it shows you where what goes wrong\n");
	printf("### Check the SixXS FAQ (http://www.sixxs.net/faq/\n");
	printf("### for more information and possible solutions or hints\n");
	printf("### Don't forget to check the Forums (http://www.sixxs.net/forum/)\n");
	printf("### for a helping hand.\n");
	printf("### Passing the output of 'aiccu autotest >aiccu.log' is a good idea.\n");

	if (!automatic)
	{
		/* Wait for a keypress */
		printf("\n\n*** press a key to continue ***\n");
		getchar();
	}
}

