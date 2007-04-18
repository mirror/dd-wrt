#ifdef HAVE_PPPOERELAY
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
void
start_pppoerelay (void)
{
  killall ("pppoe-relay", SIGTERM);
  if (nvram_match ("pppoerelay_enable", "1"))
    {
#ifdef HAVE_MADWIFI
      if (nvram_match ("ath0_mode", "sta"))
	eval ("pppoe-relay", "-S", getSTA (), "-C", "br0");
      else
#else
      if (nvram_match ("wl_mode", "sta"))
	eval ("pppoe-relay", "-S", get_wdev (), "-C", "br0");
      else
#endif
	eval ("pppoe-relay", "-S", nvram_safe_get ("wan_ifname"), "-C",
	      "br0");

      syslog (LOG_INFO, "pppoe-relay successfully started\n");
    }
}
void
stop_pppoerelay (void)
{
  if (pidof ("pppoe-relay") > 0)
    syslog (LOG_INFO, "pppoe-relay successfully stopped\n");
  killall ("pppoe-relay", SIGTERM);
}
#endif

