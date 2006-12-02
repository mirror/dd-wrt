#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <shutils.h>

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
    	eval ("killall", "wiviz");
    	eval ("/usr/sbin/wiviz", ">/dev/null", "</dev/null", "2>&1", "&");
      exit (0);
      break;
    default:
      _exit (0);
      break;
    }
}
