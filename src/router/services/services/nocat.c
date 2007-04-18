#ifdef HAVE_NOCAT
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

int
start_splashd (void)
{
  int ret = 0;
  FILE *fp;

  if (!nvram_match ("NC_enable", "1"))
    return 0;

  /* Irving - make sure our WAN link is up first.
     if not, check_ps will start us later */
  if (nvram_match ("wan_ipaddr", "0.0.0.0"))
    return 0;

  mk_nocat_conf ();

  if (!(fp = fopen ("/tmp/start_splashd.sh", "w")))
    {
      perror ("/tmp/start_splashd.sh");
      return errno;
    }
  fprintf (fp, "#!/bin/sh\n");
  fprintf (fp, "sleep 20\n");
  fprintf (fp, "splashd >> /tmp/nocat.log 2>&1 &\n");
  fclose (fp);
  chmod ("/tmp/start_splashd.sh", 0700);
  system2 ("/tmp/start_splashd.sh&");
  syslog (LOG_INFO, "splashd : splash daemon successfully started\n");

  cprintf ("done\n");
  return ret;
}

int
stop_splashd (void)
{
  int ret;
  if (pidof ("splashd") > 0)
    syslog (LOG_INFO, "splashd : splash daemon successfully stopped\n");
  //ret = killps("splashd",NULL);
  ret = killall ("splashd", SIGTERM);

  cprintf ("done\n");
  return ret;
}

#endif

