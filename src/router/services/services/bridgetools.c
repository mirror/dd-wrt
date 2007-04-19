#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#ifdef HAVE_MICRO
extern int br_add_bridge (const char *brname);
extern int br_del_bridge (const char *brname);
extern int br_add_interface (const char *br, const char *dev);
extern int br_del_interface (const char *br, const char *dev);
extern int br_set_stp_state (const char *br, int stp_state);

int
brctl_main (int argc, char **argv)
{
  if (argc == 1)
    {
      fprintf (stderr, "try to be professional!\n");
      return -1;
    }
  br_init ();
  if (!strcmp (argv[1], "addif"))
    {
      if (ifexists (argv[3]))
	br_add_interface (argv[2], argv[3]);
    }
  if (!strcmp (argv[1], "delif"))
    {
      if (ifexists (argv[3]))
	br_del_interface (argv[2], argv[3]);
    }
  if (!strcmp (argv[1], "addbr"))
    {
      br_add_bridge (argv[2]);
    }
  if (!strcmp (argv[1], "delbr"))
    {
      if (!ifexists (argv[2]))
	return -1;
      br_del_bridge (argv[2]);
    }
  br_shutdown ();
}
#else
int
br_add_bridge (const char *brname)
{
  syslog (LOG_INFO, "bridge added successfully\n");
  return eval ("/usr/sbin/brctl", "addbr", brname);
}

int
br_del_bridge (const char *brname)
{
  if (!ifexists (brname))
    return -1;
  syslog (LOG_INFO, "bridge deleted successfully\n");
  return eval ("/usr/sbin/brctl", "delbr", brname);
}

int
br_add_interface (const char *br, const char *dev)
{
  if (!ifexists (dev))
    return -1;
  syslog (LOG_INFO, "interface added successfully\n");
  return eval ("/usr/sbin/brctl", "addif", br, dev);
}

int
br_del_interface (const char *br, const char *dev)
{
  if (!ifexists (dev))
    return -1;
  syslog (LOG_INFO, "interface deleted successfully\n");
  return eval ("/usr/sbin/brctl", "delif", br, dev);
}

int
br_set_stp_state (const char *br, int stp_state)
{
  if (!ifexists (br))
    return -1;
  if (stp_state == 1)
    {
//      syslog (LOG_INFO, "stp is set to on\n");
      return eval ("/usr/sbin/brctl", "stp", br, "on");
    }
  else
    {
//      syslog (LOG_INFO, "stp is set to off\n");
      return eval ("/usr/sbin/brctl", "stp", br, "off");
    }
}
#endif
