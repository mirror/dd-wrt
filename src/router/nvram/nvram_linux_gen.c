/*
 * nvram_linux_gen.c
 *
 * Copyright (C) 2005 - 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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
 * $Id:
 * NVRAM Emulation Library for platforms who cannot support nvram based settings in any way
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <malloc.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <nvram_convert.h>

/* Globals */

static char value[65536];

static int isni = 0;

struct nvrams
{
  char *name;
  char *value;
};

struct nvramdb
{
  int offsets[('z' + 1) - 'A'];
  int nov;			//number of values;
  struct nvrams *values;
};

static struct nvramdb values;

int
nvram_init (void *unused)
{
//not needed for generic implementation
  return 0;
}

#define cprintf(fmt, args...)

/*#define cprintf(fmt, args...) do { \
	FILE *fp = stderr; \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
	} \
} while (0)
*/

//simply sort algorithm
void
sort (void)
{
  cprintf ("sort()\n");
  int i, a;
  for (i = 0; i < values.nov; i++)
    for (a = i; a < values.nov; a++)
      {
	if (values.values[a].name && values.values[i].name)
	  {
	    if (values.values[a].name[0] < values.values[i].name[0])
	      {
		struct nvrams b = values.values[a];
		values.values[a] = values.values[i];
		values.values[i] = b;
	      }
	  }
      }
  cprintf ("sort done()\n");
}



void
lock (void)
{
  FILE *in;
  int lockwait=0;
  while ((in = fopen ("/tmp/nvram/.lock", "rb")) != NULL)
    {
      fclose (in);
      cprintf ("nvram lock, waiting....\n");
      lockwait++;
      if (lockwait==3)
        unlink("/tmp/nvram/.lock"); //something crashed, we fix it
      sleep (1);
    }
  in = fopen ("/tmp/nvram/.lock", "wb");
  fprintf (in, "lock");
  fclose (in);
}


void
unlock (void)
{
  unlink ("/tmp/nvram/.lock");
}


void
writedb (void)
{
  FILE *in;
  in = fopen ("/tmp/nvram/nvram.db", "wb");
  if (in == NULL)
    return;
  int c = 0;
  int i;
  for (i = 0; i < ('z' + 1) - 'A'; i++)
    values.offsets[i] = -1;
  for (i = 0; i < values.nov; i++)
    {
      if (values.values[i].name)
	c++;
    }
  sort ();
  putc (c >> 8, in);
  putc (c & 255, in);
  for (i = 0; i < values.nov; i++)
    {
      if (values.values[i].name)
	{
	  //take a look in our offset table
	  int a;
	  if (values.offsets[values.values[i].name[0] - 'A'] == -1)
	    values.offsets[values.values[i].name[0] - 'A'] = ftell (in);
	  int len = strlen (values.values[i].name);
	  int fulllen = len + strlen (values.values[i].value) + 3;
	  putc (fulllen >> 8, in);
	  putc (fulllen & 255, in);
	  putc (len, in);
	  fwrite (values.values[i].name, len, 1, in);
	  len = strlen (values.values[i].value);
	  putc (len >> 8, in);
	  putc (len & 255, in);
	  fwrite (values.values[i].value, len, 1, in);
	}
    }
  fclose (in);
  in = fopen ("/tmp/nvram/offsets.db", "wb");
  fwrite (values.offsets, (('z' + 1) - 'A') * 4, 1, in);
  fclose (in);
}
void
readdb (void)
{
  FILE *in;
  in = fopen ("/tmp/nvram/nvram.db", "rb");
  if (in == NULL)
    {
      values.nov = 0;
      return;			//first time init;
    }
  values.nov = getc (in) << 8;
  values.nov += getc (in);
  values.values =
    (struct nvrams *) malloc (values.nov * sizeof (struct nvrams));
  int i;
  for (i = 0; i < values.nov; i++)
    {
      getc (in);
      getc (in);
      //fseek (in, 2, SEEK_CUR);
      int len = getc (in);
      values.values[i].name = (char *) malloc (len + 1);
      int a;
      fread (values.values[i].name, len, 1, in);
      values.values[i].name[len] = 0;
      len = getc (in) << 8;
      len += getc (in);
      values.values[i].value = (char *) malloc (len + 1);
      fread (values.values[i].value, len, 1, in);
      values.values[i].value[len] = 0;
    }
  fclose (in);
}

void
closedb (void)
{
  int i;
  for (i = 0; i < values.nov; i++)
    {
      if (values.values[i].name)
	free (values.values[i].name);
      if (values.values[i].value)
	free (values.values[i].value);
    }
  if (values.values)
    free (values.values);
  values.nov = -1;
  isni = 0;
}

static char cache[512];
char *
nvram_get (const char *name)
{
  int i;
  if (!name)
    return NULL;
  cprintf ("get nvram %s\n", name);
  int len = strlen (name);
  if (len == 0)
    return NULL;
  lock();

  FILE *in = fopen ("/tmp/nvram/offsets.db", "rb");
  if (in == NULL)
    {
      unlock ();
      cprintf ("nvram_get NULL (offsets)\n");
      return NULL;
    }
  setvbuf (in, &cache[0], _IOFBF, 4);
  int offset;
  fseek (in, (name[0] - 'A') * 4, SEEK_SET);
  fread (&offset, 4, 1, in);
  fclose (in);
  if (offset == -1)
    {
      unlock ();
      cprintf ("nvram_get NULL (offset = -1)\n");
      return NULL;
    }
  in = fopen ("/tmp/nvram/nvram.db", "rb");
  if (in == NULL)
    {
      unlock ();
      cprintf ("nvram_get NULL (offsets)\n");
      return NULL;
    }
  setvbuf (in, &cache[0], _IOFBF, 32);
  fseek (in, offset, SEEK_SET);

begin:;
  while (!feof (in))
    {
      int fullen = getc (in);
      if (fullen == EOF)
	break;
      fullen = fullen << 8;
      fullen += getc (in);
      int namelen = getc (in);
      if (namelen == EOF)
	break;
      if (namelen != len)
	{
	  offset += fullen + 2;
	  fseek (in, fullen - 1, SEEK_CUR);
	  goto begin;
	}
      if (getc (in) != name[0])
	{
	  fclose (in);
	  unlock ();
	  return NULL;		//readed over boundaries   
	}
      for (i = 1; i < namelen; i++)
	if (getc (in) != name[i])
	  {
	    offset += fullen + 2;
	    fseek (in, fullen - (i + 2), SEEK_CUR);
	    goto begin;
	  }
      fullen = getc (in);
      if (fullen == EOF)
	break;
      fullen = fullen << 8;
      fullen += getc (in);
      fread (&value[offset], fullen, 1, in);
      value[offset + fullen] = 0;
      fclose (in);
      unlock ();
      return &value[offset];
    }
  fclose (in);
  unlock ();
  cprintf ("nvram_get NULL (eof)\n");

  return NULL;
}

int
nvram_getall (char *b, int count)
{
  lock ();
  readdb ();
  char *buf = b;
  memset (buf, 0, NVRAM_SPACE);
  int i;
  for (i = 0; i < values.nov; i++)
    {
    if (values.values[i].name!=NULL)
    {
      strcat (buf, values.values[i].name);
      strcat (buf, "=");
      strcat (buf, values.values[i].value);
      int len = strlen (buf);
      buf[len] = 0;
      buf += len + 1;
    }
    }
  closedb ();
  unlock ();
  return 0;
}


void nvram_open(void)
{
lock();
  readdb ();
}

void nvram_close(void)
{
  writedb ();
  closedb ();
unlock();
}

int nvram_immed_set (const char *name, const char *value)
{
  cprintf ("nvram_set %s %s\n", name, value);

  if (name[0] < 'A' || name[0] > 'z')
    {
      fprintf (stderr, "nvram parameter %s starts with a illegal character\n",
	       name);
      return NULL;
    }

  int i;
  for (i = 0; i < values.nov; i++)
    {
      if (values.values[i].name!=NULL && !strcmp (values.values[i].name, name))
	{
	  if (value == NULL)
	    {
	      free (values.values[i].name);
	      free (values.values[i].value);
	      values.values[i].name = NULL;
	      values.values[i].value = NULL;
	    }
	  else
	    {
	      free (values.values[i].value);
	      values.values[i].value = strdup (value);
	      value = NULL;
	    }
	}
    }
  if (value)
    {
      values.values =
	(struct nvrams *) realloc (values.values,
				   (values.nov + 1) * sizeof (struct nvrams));
      values.values[values.nov].name = strdup (name);
      values.values[values.nov].value = strdup (value);
      values.nov++;
    }
  cprintf ("nvram_set done()\n");

  return 0;
}

int
nvram_set (const char *name, const char *value)
{
nvram_open();
#ifdef HAVE_NOWIFI
  if (!strcmp (name, "ip_conntrack_max") && value != NULL)
    {
      int val = atoi (value);
      if (val > 4096)
	{
	  int ret = nvram_immed_set (name, "4096");
	  nvram_close();
	  return ret;
	}

    }
#endif

  cprintf ("set nvram %s to %s\n", name, value);
  extern struct nvram_convert nvram_converts[];
  struct nvram_convert *v;
  int ret;

  ret = nvram_immed_set (name, value);

  for (v = nvram_converts; v->name; v++)
    {
      if (!strcmp (v->name, name))
	{
	  if (strcmp (v->wl0_name, ""))
	    nvram_immed_set (v->wl0_name, value);
	  if (strcmp (v->d11g_name, ""))
	    nvram_immed_set (v->d11g_name, value);
	}
    }
nvram_close();
  cprintf ("nvram_set done\n");
  return ret;
}

int
nvram_unset (const char *name)
{
  nvram_open();
  nvram_immed_set (name, NULL);
  nvram_close();
  return 0;
}

/*
 * write back the changes to flash memory or filesystem (platform depending)
 */
int
nvram_commit (void)
{
  lock ();
#ifdef HAVE_MAGICBOX
  system
    ("tar -czf /tmp/nvram/nvram.tar.gz /tmp/nvram/nvram.db /tmp/nvram/offsets.db");
  system ("mtd -f write /tmp/nvram/nvram.tar.gz nvram");
#elif HAVE_GATEWORX
  system
    ("tar -czf /tmp/nvram/nvram.tar.gz /tmp/nvram/nvram.db /tmp/nvram/offsets.db");
  system ("mtd -f write /tmp/nvram/nvram.tar.gz nvram");
#elif HAVE_FONERA
  system
    ("tar -czf /tmp/nvram/nvram.tar.gz /tmp/nvram/nvram.db /tmp/nvram/offsets.db");
  system ("mtd -f write /tmp/nvram/nvram.tar.gz nvram");
#elif HAVE_WHRAG108
  system
    ("tar -czf /tmp/nvram/nvram.tar.gz /tmp/nvram/nvram.db /tmp/nvram/offsets.db");
  system ("mtd -f write /tmp/nvram/nvram.tar.gz nvram");
#elif HAVE_TW6600
  system
    ("tar -czf /tmp/nvram/nvram.tar.gz /tmp/nvram/nvram.db /tmp/nvram/offsets.db");
  system ("mtd -f write /tmp/nvram/nvram.tar.gz nvram");
#else
  system ("mount /usr/local -o remount,rw");
  system ("cp /tmp/nvram/nvram.db /etc/nvram");
  system ("cp /tmp/nvram/offsets.db /etc/nvram");
  system ("mount /usr/local -o remount,ro");
#endif
  unlock ();
  return 0;
}

int
file2nvram (char *filename, char *varname)
{
  FILE *fp;
  int c, count;
  int i = 0, j = 0;
  char mem[10000], buf[30000];

  if (!(fp = fopen (filename, "rb")))
    return 0;

  count = fread (mem, 1, sizeof (mem), fp);
  fclose (fp);
  for (j = 0; j < count; j++)
    {
      if (i > sizeof (buf) - 3)
	break;
      c = mem[j];
      if (c >= 32 && c <= 126 && c != '~')
	{
	  buf[i++] = (unsigned char) c;
	}
      else if (c == 13)
	{
	  buf[i++] = (unsigned char) c;
	}
      else if (c == 0)
	{
	  buf[i++] = '~';
	}
      else if (c == 10)
	{
	  buf[i++] = (unsigned char) c;
	}
      else
	{
	  buf[i++] = '\\';
	  sprintf (buf + i, "%02X", c);
	  i += 2;
	}
    }
  if (i == 0)
    return 0;
  buf[i] = 0;
  //fprintf(stderr,"================ > file2nvram %s = [%s] \n",varname,buf); 
  nvram_set (varname, buf);

}

int
nvram2file (char *varname, char *filename)
{
  FILE *fp;
  int c, tmp;
  int i = 0, j = 0;
  char *buf;
  char mem[10000];

  if (!(fp = fopen (filename, "wb")))
    return 0;

  buf = strdup (nvram_safe_get (varname));
  //fprintf(stderr,"=================> nvram2file %s = [%s] \n",varname,buf);
  while (buf[i] && j < sizeof (mem) - 3)
    {
/*        if (buf[i] == '\\')  {
                i++;
                tmp=buf[i+2];
                buf[i+2]=0;
                sscanf(buf+i,"%02X",&c);
                buf[i+2]=tmp;
                i+=2;
                mem[j]=c;j++;
        } else */
      if (buf[i] == '~')
	{
	  mem[j] = 0;
	  j++;
	  i++;
	}
      else
	{
	  mem[j] = buf[i];
	  j++;
	  i++;
	}
    }
  if (j <= 0)
    return j;
  j = fwrite (mem, 1, j, fp);
  fclose (fp);
  free (buf);
  return j;
}

#include "nvram_generics.h"
