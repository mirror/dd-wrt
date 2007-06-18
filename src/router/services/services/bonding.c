#ifdef HAVE_BONDING
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>

void
start_bonding (void)
{
  char mode[64];
  char count[64];
  sprintf (mode, "mode=%s", nvram_default_get ("bonding_type", "balance-rr"));
  sprintf (count, "max_bonds=%s", nvram_default_get ("bonding_number", "1"));
  eval ("insmod", "bonding", "miimon=100", mode, count);

  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("bondings");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    if (!tag || !port)
      {
	break;
      }
    eval ("ifconfig", tag, "0.0.0.0", "up");
    eval ("ifenslave", tag, port);
  }
  int c = atoi (nvram_safe_get ("bonding_number"));
  int i;
  for (i = 0; i < c; i++)
    {
      sprintf (word, "bond%d", i);
      char *br = getRealBridge (word);
      if (br)
	eval ("brctl", "addif", br, word);

    }
}
void
stop_bonding (void)
{
  int i;
  for (i = 0; i < 10; i++)
    {
      char bond[32];
      sprintf (bond, "bond%d", i);
      if (ifexists (bond))
	{
	  char *br = getRealBridge (bond);
	  if (br)
	    eval ("brctl", "delif", br, bond);
	  eval ("ifconfig", bond, "down");
	}
    }
  eval ("rmmod", "bonding");
}




#endif
