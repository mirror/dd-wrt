#ifdef HAVE_UPNP
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

int
start_upnp (void)
{
  char *wan_ifname = get_wan_face ();
  int ret;

  if (nvram_match ("upnp_enable", "0"))
    {
      stop_upnp ();
      return 0;
    }
  /* Make sure its not running first */
  ret = killall ("upnp", SIGUSR1);
  if (ret != 0)
    {
      ret = eval ("upnp", "-D",
		  "-L", nvram_safe_get ("lan_ifname"),
		  "-W", wan_ifname,
		  "-I", nvram_safe_get ("upnp_ssdp_interval"),
		  "-A", nvram_safe_get ("upnp_max_age"));
      syslog (LOG_INFO, "upnp : upnp daemon successfully started\n");
    }

  cprintf ("done\n");
  return ret;
}

int
stop_upnp (void)
{
  if (pidof ("upnp") > 0)
    syslog (LOG_INFO, "upnp : upnp daemon successfully stopped\n");
  killall ("upnp", SIGUSR1);
  killall ("upnp", SIGTERM);

  cprintf ("done\n");
  return 0;
}
#endif
