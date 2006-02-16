#ifdef HAVE_WOL

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <snmp.h>
#include <signal.h>

#define WOL_INTERVAL 15

int
stop_wol (void)
{
  int ret;

  ret = eval ("killall", "-9", "wol");

  cprintf ("done\n");

  return ret;
}

int
start_wol (void)
{
  int ret;
  pid_t pid;
  char *wol_argv[] = { "wol",
    NULL
  };

  stop_wol ();

  if (nvram_match ("wol_enable", "0"))
    return 0;

  ret = _eval (wol_argv, NULL, 0, &pid);

  return ret;
}

#endif /* HAVE_WOL */
