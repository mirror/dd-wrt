
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>



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
/* software wlan led control */      
      int radiooff = -1;

#ifdef HAVE_MADWIFI
		//????;  no idea how to check this
#else
		wl_ioctl (get_wdev (), WLC_GET_RADIO, &radiooff, sizeof (int));
#endif

  		switch (radiooff)
    		{
    		case 0:
      			led_control (LED_WLAN, LED_ON);
      			break;
    		default:
	  			led_control (LED_WLAN, LED_OFF);
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
