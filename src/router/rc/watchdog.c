
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>



static void
watchdog (void)
{
  int radiostate = -1;
  int oldstate = -1;
  
  int fd = open ("/dev/misc/watchdog", O_WRONLY);
  if (fd == -1)
    {
      return;
    }
  while (1)
    {
      write (fd, "\0", 1);
      fsync (fd);
      
/* software wlan led control */      
#ifdef HAVE_MADWIFI
		//????;  no idea how to check this
#else
		wl_ioctl (get_wdev (), WLC_GET_RADIO, &radiostate, sizeof (int));
#endif
		
	if (radiostate != oldstate)
		{
		if (radiostate == 0)
			led_control (LED_WLAN, LED_ON);
		else
			led_control (LED_WLAN, LED_OFF);
			
		oldstate = radiostate;
		}
/* end software wlan led control */
  
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
