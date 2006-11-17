
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>




static void
watchdog (void)
{
  int fd = open ("/dev/misc/watchdog", O_WRONLY);
  if (fd == -1)
    {
      return;
    }
  while (1)
    {
      write (fd, "\0", 1);
      fsync (fd);
      sleep (10);
    }
}

int
watchdog_main (int argc, char *argv[])
{

  /* Run it under background */
  switch (fork ())
    {
    case -1:
      exit (0);
      break;
    case 0:
      /* child process */
      watchdog ();
      exit (0);
      break;
    default:
      /* parent process should just die */
      _exit (0);
    }
  return 0;
}
