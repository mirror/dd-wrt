#ifdef HAVE_TELNET
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
/* begin Sveasoft additon */
int
start_telnetd (void)
{
  int ret = 0;
  pid_t pid;

  char *telnetd_argv[] = { "/usr/sbin/telnetd", NULL };

  stop_telnetd ();

  if (!nvram_invmatch ("telnetd_enable", "0"))
    return 0;

  ret = _eval (telnetd_argv, NULL, 0, &pid);
  syslog (LOG_INFO, "telnetd : telnet daemon successfully started\n");

  cprintf ("done\n");
  return ret;
}

int
stop_telnetd (void)
{
  int ret;
  if (pidof ("telnetd") > 0)
    syslog (LOG_INFO, "telnetd : telnet daemon successfully stopped\n");
  ret = killall ("telnetd", SIGTERM);

  cprintf ("done\n");
  return ret;
}
#endif

