#ifdef HAVE_MILKFISH
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void
stop_milkfish (void)
{
  if (pidof ("rtpproxy") > 0 || pidof ("openserctl") > 0)
    syslog (LOG_INFO, "Milkfish service successfully stopped\n");
    
  killall ("rtpproxy", SIGTERM);
  killall ("openserctl", SIGTERM) 
}

void
start_milkfish (void)
{
	if (nvram_match ("milkfish_enabled", "1"))
    {
		eval ("/etc/config/milkfish.startup");
		eval ("/etc/config/milkfish.netup");  //start rtpproxy and openserctl
    
		syslog (LOG_INFO, "Milkfish service successfully started\n");
	}
}

#endif
