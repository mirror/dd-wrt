#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#ifdef HAVE_VLANTAGGING

void
start_vlantagging (void)
{
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("vlan_tags");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    if (!tag || !port)
      {
	break;
      }
    eval ("vconfig", "add", tag, port);
    char vlan_name[32];
    sprintf (vlan_name, "%s.%s", tag, port);
    eval ("ifconfig", vlan_name, "0.0.0.0", "up");
  }
}

void
stop_vlantagging (void)
{
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("vlan_tags");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    if (!tag || !port)
      break;
    char vlan_name[32];
    sprintf (vlan_name, "%s.%s", tag, port);
    if (ifexists (vlan_name))
      {
      eval ("vconfig", "rem", vlan_name);
      }
  }
}
void
start_bridgesif (void)
{
  if (nvram_match ("lan_stp", "0"))
    eval ("brctl", "stp", "br0", "off");
  else
    eval ("brctl", "stp", "br0", "on");
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("bridgesif");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    char *prio = port;
    strsep (&prio, ">");
    if (!tag || !port)
      break;
    if (strncmp (tag, "EOP", 3))
      {
	eval ("brctl", "addif", tag, port);
	if (prio)
	  eval ("brctl", "setportprio", tag, port, prio);
      }
  }

}

void
start_bridging (void)
{
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("bridges");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    char *prio = port;
    strsep (&prio, ">");
    if (!tag || !port)
      break;
    char ipaddr[32];
    sprintf (ipaddr, "%s_ipaddr", tag);
    char netmask[32];
    sprintf (netmask, "%s_netmask", tag);

    eval ("brctl", "addbr", tag);
    if (!strcmp (port, "On"))
      br_set_stp_state (tag, 1);
    else
      br_set_stp_state (tag, 0);
    if (prio)
      eval ("brctl", "setbridgeprio", tag, prio);
    if (!nvram_match (ipaddr, "0.0.0.0") && !nvram_match (netmask, "0.0.0.0"))
      {
	eval ("ifconfig", tag, nvram_safe_get (ipaddr), "netmask",
	      nvram_safe_get (netmask), "up");
      }
    else
      eval ("ifconfig", tag, "0.0.0.0", "up");
  }
}

char *
getBridge (char *ifname)
{
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("bridgesif");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    char *prio = port;
    strsep (&prio, ">");
    if (!tag || !port)
      break;
    if (!strcmp (port, ifname))
      return tag;
  }
  return nvram_safe_get ("lan_ifname");
}

char *
getRealBridge (char *ifname)
{
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("bridgesif");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    char *prio = port;
    strsep (&prio, ">");
    if (!tag || !port)
      break;
    if (!strcmp (port, ifname))
      return tag;
  }
  return NULL;
}

char *
getBridgePrio (char *ifname)
{
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("bridgesif");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    char *prio = port;
    strsep (&prio, ">");
    if (!tag || !port)
      break;
    if (!strcmp (port, ifname))
      return port;
  }
  return "0";
}

void
stop_bridgesif (void)
{
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("bridgesif");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    char *prio = port;
    strsep (&prio, ">");
    if (!tag || !port)
      break;
    if (ifexists (port))
      eval ("brctl", "delif", tag, port);
  }
}

void
stop_bridging (void)
{
  static char word[256];
  char *next, *wordlist;
  wordlist = nvram_safe_get ("bridges");
  foreach (word, wordlist, next)
  {
    char *port = word;
    char *tag = strsep (&port, ">");
    char *prio = port;
    strsep (&prio, ">");
    if (!tag || !port)
      break;
    if (ifexists (tag))
      {
	eval ("ifconfig", tag, "down");
	eval ("brctl", "delbr", tag);
      }
  }
}


#else
char *
getBridge (char *ifname)
{
  return nvram_safe_get ("lan_ifname");
}

char *
getRealBridge (char *ifname)
{
  return NULL;
}

char *
getBridgePrio (char *ifname)
{
  return "0";
}
#endif

int
getbridge_main (int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf (stderr, "syntax: getbridge [ifname]\n");
      return -1;
    }
  char *bridge = getBridge (argv[1]);
  fprintf (stdout, "%s\n", bridge);
  return 0;
}

int
getbridgeprio_main (int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf (stderr, "syntax: getbridgeprio [ifname]\n");
      return -1;
    }
  char *bridge = getBridgePrio (argv[1]);
  fprintf (stdout, "%s\n", bridge);
  return 0;
}
