#ifdef HAVE_NSTX
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void
stop_nstxd (void)
{
  if (pidof ("nstxd") > 0)
    syslog (LOG_INFO, "nstxd : nstx daemon successfully stopped\n");
  killall ("nstxd", SIGTERM);
}

void
start_nstxd (void)
{
  if (nvram_match ("nstxd_enable", "1"))
    {
      stop_nstxd ();
      eval ("nstxd");
    }
  syslog (LOG_INFO, "nstxd daemon successfully started\n");
}

#endif

