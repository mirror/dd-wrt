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
			FILE *fp = fopen("/tmp/wiviz2-cfg", "wb");
				if (strstr (hopseq, ","))
      				fprintf (fp, "channelsel=hop&");
    			else
      				fprintf (fp, "channelsel=%s&", nvram_safe_get ("hopseq"));
			fprintf (fp, "hopdwell=%s&hopseq=%s\n", nvram_safe_get ("hopdwell"), nvram_safe_get ("hopseq"));
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
