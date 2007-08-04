#ifdef HAVE_SPUTNIK_APD
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

/* Sputnik APD Service Handling */
int
start_sputnik (void)
{
  int ret;

  // Only start if enabled
  if (!nvram_invmatch ("apd_enable", "0"))
    return 0;

  ret = eval ("sputnik");
  syslog (LOG_INFO, "sputnik : sputnik daemon successfully started\n");
  cprintf ("done\n");
  return ret;
}

int
stop_sputnik (void)
{
  if (pidof ("sputnik") > 0)
    syslog (LOG_INFO, "sputnik : sputnik daemon successfully stopped\n");
  int ret = killall ("sputnik", SIGTERM);

  cprintf ("done\n");
  return ret;
}

/* END Sputnik Service Handling */

#endif
