#ifdef HAVE_RSTATS
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void
stop_rstats (void)
{
  if (pidof ("rstats") > 0)
    syslog (LOG_INFO, "rstats : rstats daemon successfully stopped\n");
  killall ("rstats", SIGTERM);
}

void
start_rstats (void)
{
  // If jffs has been disabled force rstats files to temp memory
  if (nvram_match ("rstats_path", "/jffs/")
      && nvram_match ("enable_jffs2", "1"))
    {
      nvram_set ("rstats_path", "");
      nvram_commit ();
    }

  if (nvram_match ("rstats_enable", "1"))
    {
      stop_rstats ();
      eval ("rstats");
    }
  syslog (LOG_INFO, "rstats daemon successfully started\n");
}

#endif
