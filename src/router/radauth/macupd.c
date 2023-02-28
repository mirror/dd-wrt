// ***************************************************************
// **       macupd   Programm developed by Schlegel Stephan     **
// ***************************************************************


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define BSIZE 256
// Switchin DEBUG Mode for Developer !!!!
#define DBGPRINT if (0) printf
//#define DBGPRINT printf       

// Varibales difin fpr workin

static int delay = 45;		// Interval for sending udp paket
static int times = 1;		// count of upd copies


// static int inFile;
static char inBuf[BSIZE];
static char outBuf[BSIZE];
static int inLen = 0;
static int sock;
struct sockaddr_in addr;
struct in_addr ip;
static int portnum;

const char bash0[] = "rm -f /tmp/rssiclients.lst";
const char bash1[] =
  "echo rssi client list by macupd-DD-WRT >/tmp/rssiclients.lst";
#ifdef HAVE_MADWIFI
const char bash2[] =
  "for mac in $(wl_atheros assoclist | awk '{print$2}')\ndo\nsignal=$(wl_atheros rssi $mac | awk {'print$3'})\necho -e rssi 0 0 $mac +$signal+ wlan>>/tmp/rssiclients.lst\ndone\n";
const char bash3[] =
  "for mac in $(wl_atheros wds | awk '{print$2}')\ndo\nsignal=$(wl_atheros rssi $mac | awk {'print$3'})\necho -e rssi 0 0 $mac +$signal+ wds>>/tmp/rssiclients.lst\ndone\n";
#else  
#ifdef NEW_RSSI
const char bash2[] =
  "for mac in $(wl assoclist | awk '{print$2}')\ndo\nsignal=$(wl rssi $mac | awk {'print$1'})\necho -e rssi 0 0 $mac +$signal+ wlan>>/tmp/rssiclients.lst\ndone\n";
const char bash3[] =
  "for mac in $(wl wds | awk '{print$2}')\ndo\nsignal=$(wl rssi $mac | awk {'print$1'})\necho -e rssi 0 0 $mac +$signal+ wds>>/tmp/rssiclients.lst\ndone\n";
#else
const char bash2[] =
  "for mac in $(wl assoclist | awk '{print$2}')\ndo\nsignal=$(wl rssi $mac | awk {'print$3'})\necho -e rssi 0 0 $mac +$signal+ wlan>>/tmp/rssiclients.lst\ndone\n";
const char bash3[] =
  "for mac in $(wl wds | awk '{print$2}')\ndo\nsignal=$(wl rssi $mac | awk {'print$3'})\necho -e rssi 0 0 $mac +$signal+ wds>>/tmp/rssiclients.lst\ndone\n";
#endif
#endif

static int
indexOf (char *haystack, char needle)
{
  // DBGPRINT("doing search...  ");
  int ind = 0;
  while (haystack[ind] != 0)
    {
      if (haystack[ind] == needle)
	{
	  // DBGPRINT("  found (%dmacudp <server> <port>)", ind);
	  return ind;
	}
      ind++;
    }
  // DBGPRINT("  not found !");
  return -1;
}

// send a file on udp Stream to server
static int
sendfile (char filename[])
{

  DBGPRINT ("opening ... %s\n", filename);
  int filehandle = -1;

  filehandle = open (filename, O_RDONLY);

  // only send if file exists
  if (filehandle > -1)
    {


      int lineNo = 0;
      int r = 0;
      DBGPRINT ("reading...\n");
      while ((r = read (filehandle, &inBuf[inLen], BSIZE - inLen - 1)))
	{
	  // DBGPRINT("  ok (%d)\n", r);
	  inLen += r;
	  // DBGPRINT("setting end0 [%d]...\n", inLen);
	  inBuf[inLen] = 0;
	  // DBGPRINT("  ok\n");
	  int lineSize;
	  // DBGPRINT("finding \\n...\n");
	  while ((lineSize = indexOf (inBuf, '\n')) >= 0)
	    {
	      // DBGPRINT("  ok (%d)\n", lineSize);

	      // work with inBuf[0]...inBuf[lineSize-1]
	      inBuf[lineSize] = 0;
	      DBGPRINT ("The line is: >%s<", inBuf);

	      if (lineNo > 0)
		{
		  int wdh;
		  for (wdh = times; wdh > 0; wdh--)
		    {
		      sprintf (outBuf, "%4d    ", lineNo);
		      memcpy (&outBuf[8], inBuf, lineSize);
		      DBGPRINT ("\t\tsending line...\n");
		      int ret = sendto (sock, outBuf, lineSize + 8, 0,
					(struct sockaddr *) &addr,
					sizeof (addr));
		      if (!ret)
			{
			  printf ("error sending packet\n");
			  return -1;
			}
		      //DBGPRINT("  ok\n");
		      usleep (30 * 1000);
		    }
		}
	      else
		DBGPRINT ("\n");

	      inLen -= lineSize + 1;
	      // DBGPRINT("doing memcpy [lineSize %d, left %d]...\n", lineSize, inLen);
	      memcpy (&(inBuf[0]), &(inBuf[lineSize + 1]), inLen + 1);
	      // DBGPRINT("  ok\n");
	      lineNo++;
	      // DBGPRINT("finding \\n...\n");
	    }
	  //DBGPRINT("reading...\n");
	};
      close (filehandle);
      // success
      return 1;
    }
  else
    return 0;
}









int
main (int argc, char **argv)
{

  printf ("\n");
  printf
    ("******************************************************************************\n");
  printf
    ("*  macupd v2 | send all known Clients (and WDS) from this machine by UDP     *\n");
  printf
    ("******************************************************************************\n");
  printf ("\n");
  printf ("\n");
  printf ("rssi value of all WLAN Clients and MAC-IP combination\n");
  printf ("developed by Schlegel Stephan (DD-WRT)\n");
  printf ("Ctl+C Abort this programm\n");

  if (argc < 4)
    {
      printf ("use this command line: rssiupd <server> <port> <interval>\n");
      printf ("\n");
      printf ("<server>   : IP Adress of server to recive messages\n");
      printf ("<port>     : Port\n");
      printf ("<interval> : delay between udp pakets in sec\n");
      printf ("\n");
      return 1;
    }

  DBGPRINT ("parse delay...\n");
  delay = atoi (argv[3]);
  if (delay < 1)
    delay = 1;

  printf ("\n");
  printf ("is working on: %s:%s  (Interval=%d sec, %d times)...\n", argv[1],
	  argv[2], delay, times);
  printf ("\n");

  DBGPRINT ("parse address...\n");
  if (!inet_aton (argv[1], &ip))
    {
      printf ("address not recognized\n");
    }
  DBGPRINT ("  ok\n");
  DBGPRINT ("parse port...\n");
  portnum = atoi (argv[2]);
  DBGPRINT ("  ok (%d)\n", portnum);

  addr.sin_family = AF_INET;
  addr.sin_addr = ip;
  addr.sin_port = htons (portnum);

  DBGPRINT ("create socket...\n");
  sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock)
    {
      DBGPRINT ("  ok\n");
    }
  else
    {
      DBGPRINT ("  failed\n");
      return 1;
    }

  // *******************  begin with read and process loop   *****************************************
  while (1)
    {
      int test;

      DBGPRINT ("open file...\n");

      // bashabfrage erstellen !!!!!!
      system ("rm -rf /tmp/snmp_cache");
      system (bash1);		// delete and create rssiclients.lst
      system (bash2);		// append rssiclients.lst WLAN Clients
      system (bash3);		// append rssiclients.lst WDS Clients

      test = sendfile ("/proc/net/arp");
      DBGPRINT ("succes=%d\n", test);
      test = sendfile ("/tmp/rssiclients.lst");
      DBGPRINT ("success=%d\n", test);


      DBGPRINT ("sleep secs...%d\n", delay);
      sleep (delay);
    }
  // ****************************************************************************************************************

  return 0;
}
