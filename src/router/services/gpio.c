#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>


int
gpio_main (int argc, char **argv)
{

  unsigned int gpio;
  unsigned int old_gpio;
  unsigned int pin;

  int gpioouten = open ("/dev/gpio/outen", O_RDWR);

  if (argc != 3)
    {
      fprintf(stdout,"%s <poll | enable | disable> <pin>\n", argv[0]);
      exit (1);
    }

  pin = 1 << atoi (argv[2]);
  if (!strcmp (argv[1], "poll"))
    {
      int gpioin = open ("/dev/gpio/in", O_RDWR);
      read (gpioouten, &gpio, sizeof (gpio));
      gpio &= ~pin;
      write (gpioouten, &gpio, sizeof (gpio));
      while (1)
	{
	  read (gpioin, &gpio, sizeof (gpio));
	  gpio = (gpio & pin) ? 1 : 0;
	  if (gpio != old_gpio)
	fprintf (stdout,"%02X\n", gpio);
	  old_gpio = gpio;
	}
    }
  else if (!strcmp (argv[1], "enable") || !strcmp (argv[1], "disable"))
    {
      int gpioout = open ("/dev/gpio/out", O_RDWR);

      read (gpioouten, &gpio, sizeof (gpio));
      gpio |= pin;
      write (gpioouten, &gpio, sizeof (gpio));

      read (gpioout, &gpio, sizeof (gpio));
      if (!strcmp (argv[1], "enable"))
	{
	  gpio |= pin;
	}
      else
	{
	  gpio &= ~pin;
	}
      write (gpioout, &gpio, sizeof (gpio));
    }

  return 0;
}
