/*
 * overclock_atheros.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * desc:
 * overclocks 2313 and 2316/17 or compatible wisoc boards with redboot or wistron zLoader to 220mhz/200mhz
 * just a dirty hack i found while playing with code 
 */
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <shutils.h>


/*
to prevent bricks and other troubles you should use this tool only manually. it will not be implemented in the webgui
*/

void
start_overclock
 (void)		// hidden feature. must be called with "startservice overlock". then reboot the unit
{
  long len;
  long i;
#ifdef HAVE_CA8  
  FILE *in = fopen ("/dev/mtdblock/2", "rb");  // zloader Board Data access. the board data section contains also the cpu programming code
#else
  FILE *in = fopen ("/dev/mtdblock/0", "rb");
#endif
  FILE *out = fopen ("/tmp/boot", "wb");
  fseek (in, 0, SEEK_END);
  len = ftell (in);
  fseek (in, 0, SEEK_SET);
  for (i = 0; i < len; i++)
    putc (getc (in), out);
  fclose (in);
  fclose (out);
  in = fopen ("/tmp/boot", "r+b");
  fseek (in, 0xe64b, SEEK_SET);				
  int zmul = getc(in);
  fseek (in, 0xcb, SEEK_SET);				
  int vipermul = getc (in);
  fseek (in, 0x1e3, SEEK_SET);
  int div = getc (in);
  fseek (in, 0x1ef, SEEK_SET);
  int mul = getc (in);
 // fprintf(stderr,"vipermul %X, div %X, mul %X\n",vipermul,div,mul);
  if (div == 0x3 && mul == 0x5c)
    {
      fprintf (stderr, "ap51/ap61 (ar2315 or ar2317) found\n");
      fseek (in, 0x1e3, SEEK_SET);
      putc (0x1, in);
      fseek (in, 0x1ef, SEEK_SET);
      putc (0x28, in); //0x2c for 220 mhz 0x30 for 240 mhz
      fclose (in);
      eval ("mtd", "-f", "write", "/tmp/boot", "RedBoot");
      fprintf (stderr, "board is now clocked at 200 mhz, please reboot\n");
    }
  else if (div == 0x01 && mul == 0x28)
    {
      fprintf (stderr, "board already clocked to 200mhz\n");
      fclose (in);
    }
  else if (div == 0x01 && mul == 0x2c)
    {
      fprintf (stderr, "board already clocked to 220mhz\n");
      fclose (in);
    }
  else if (div == 0x01 && mul == 0x30)
    {
      fprintf (stderr, "board already clocked to 220mhz\n");
      fclose (in);
    }
  else if (vipermul == 0xb)
    {
      fprintf (stderr, "board already clocked to 220mhz\n");
      fclose (in);
    }
  else if (vipermul == 0xc)
    {
      fprintf (stderr, "board already clocked to 240mhz\n");
      fclose (in);
    }
  else if (vipermul == 0x9)
    {
      fprintf (stderr, "viper (ar2313) found\n");
      fseek (in, 0xcb, SEEK_SET);
      putc (0xb, in);
      fclose (in);
      eval ("mtd", "-f", "write", "/tmp/boot", "RedBoot");
      fprintf (stderr, "board is now clocked at 220 mhz, please reboot\n");
    }
  else if (zmul == 0xb)
    {
      fprintf (stderr, "board already clocked to 220mhz\n");
      fclose (in);
    }
  else if (zmul == 0xc)
    {
      fprintf (stderr, "board already clocked to 240mhz\n");
      fclose (in);
    }
  else if (zmul == 0x9) // special handling for zLoader based boards
    {
      fprintf (stderr, "viper (ar2313) found (zLoader)\n");
      fseek (in, 0xe64b, SEEK_SET);				
      putc (0xb, in);
      fclose (in);
      eval ("mtd", "-f", "write", "/tmp/boot", "bdata");
      fprintf (stderr, "board is now clocked at 220 mhz, please reboot\n");
    }
  else
    {
      fprintf (stderr, "unknown board or no redboot found\n");
      fclose (in);
    }
}
