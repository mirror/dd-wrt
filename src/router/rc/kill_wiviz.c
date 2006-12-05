#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <shutils.h>
#include <utils.h>

int
kill_wiviz_main (void)
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
	killall("wiviz",SIGUSR1);
//		eval ("killall", "-USR1", "wiviz", ">/dev/null", "2>&1");	// then kill it to get data
      exit (0);
      break;
    default:
      _exit (0);
      break;
    }
}