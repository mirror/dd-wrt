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
  if (pidof ("milkfish") > 0)
    syslog (LOG_INFO, "Milkfish service successfully stopped\n");
  killall ("milkfish", SIGTERM);
}

void
start_milkfish (void)
{
	if (nvram_match ("milkfish_enabled", "1"))
    {
// here comes all the Milkfish stuff, cfg files, etc

//      eval ("milkfish", "blabla");
    }
  syslog (LOG_INFO, "Milkfish service successfully started\n");
}

#endif
