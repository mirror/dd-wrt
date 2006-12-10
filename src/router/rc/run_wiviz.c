#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>


int
run_wiviz_main (void)
{
  pid_t pid;
  pid = fork ();
  switch (pid)
    {
    case -1:
      perror ("fork failed");
      exit (1);
      break;
    case 0:
        if (pidof("wiviz") > 0)
    	    killall("wiviz", SIGUSR1);
	else
		{	
#ifdef HAVE_MSSID
  			char *channel = nvram_safe_get ("wl0_channel");
#else
			char *channel = nvram_safe_get ("wl_channel");
#endif

			char *hopdwell = nvram_safe_get ("hopdwell");
				if (!strlen (hopdwell)) 
					nvram_set ("hopdwell", "1000"));
					
			char *hopseq = nvram_safe_get ("hopseq");
				if (!strlen (hopseq)) 
					nvram_set ("hopseq", channel));

			FILE *fp = fopen("/tmp/wiviz2-cfg", "wb");
			fprintf (fp, "channelsel=hop&hopdwell=%s&hopseq=%s\n", 
				nvram_safe_get ("hopdwell"), nvram_safe_get ("hopseq"));
			fclose (fp);
    	    eval ("/usr/sbin/wiviz", ">/dev/null", "</dev/null", "2>&1", "&");
	    }
      exit (0);
      break;
    default:
      _exit (0);
      break;
    }
}
