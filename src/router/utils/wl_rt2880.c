#include <stdio.h>
#include <wlutils.h>
#include <shutils.h>


void
showinterface (char *ifname)
{
  char *buf=malloc(8192);
  memset(buf,0,8192);
  int cnt = getassoclist (ifname, buf);
  int count;
  memcpy (&count, buf, 4);
  unsigned char *p = &buf[4];
  int a;
  int pos = 0;
  for (a = 0; a < cnt; a++)
    {
      fprintf (stdout, "assoclist %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
	       p[pos], p[pos + 1], p[pos + 2], p[pos + 3], p[pos + 4],
	       p[pos + 5]);
      pos += 6;
    }
 free(buf);


}
int
main (int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf (stderr, "invalid argument\n");
      return 0;
    }
  char *ifname = "wl0";
  int i;
  for (i = 1; i < argc; i++)
    {
      if (!strcmp (argv[i], "-i"))
        {
	    ifname = argv[i + 1];    
        }
      if (!strcmp (argv[i], "assoclist"))
	{
	    showinterface (ifname);
	}
      if (!strcmp (argv[i], "rssi"))
	{
	  unsigned char rmac[6];
	  ether_atoe (argv[++i], rmac);
	  int rssi = getRssi (ifname, rmac);
	  fprintf (stdout, "rssi is %d\n", rssi);
	}
      if (!strcmp (argv[i], "noise"))
	{
	  unsigned char rmac[6];
	  ether_atoe (argv[++i], rmac);
	  int rssi=-1;
	  rssi = getNoise (ifname, rmac);
	  fprintf (stdout, "noise is %d\n", rssi);
	}
    }
}
