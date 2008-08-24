/*
 * NVRAM variable manipulation (Linux user mode half)
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram_linux.c,v 1.9 2003/12/03 10:14:06 honor Exp $
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

#include <typedefs.h>
#include <bcmnvram.h>
#include <nvram_convert.h>

#define PATH_DEV_NVRAM "/dev/nvram"

/* Globals */
static int nvram_fd = -1;
static char *nvram_buf = NULL;

int
nvram_init (void *unused)
{


  if ((nvram_fd = open (PATH_DEV_NVRAM, O_RDWR)) < 0)
    goto err;

  /* Map kernel string buffer into user space */
  if ((nvram_buf =
       mmap (NULL, NVRAM_SPACE, PROT_READ, MAP_SHARED, nvram_fd,
	     0)) == MAP_FAILED)
    {
      close (nvram_fd);
      fprintf (stderr, "nvram_init(): failed\n");
      nvram_fd = -1;
      goto err;
    }

  return 0;

err:
  perror (PATH_DEV_NVRAM);
  return errno;
}

void
lock (void)
{
  FILE *in;
  int lockwait=0;
  while ((in = fopen ("/tmp/.nvlock", "rb")) != NULL)
    {
      fclose (in);
      //cprintf ("nvram lock, waiting....\n");
      lockwait++;
      if (lockwait==3)
        unlink("/tmp/.nvlock"); //something crashed, we fix it
      sleep (1);
    }
  in = fopen ("/tmp/.nvlock", "wb");
  fprintf (in, "lock");
  fclose (in);
}


void
unlock (void)
{
  unlink ("/tmp/.nvlock");
}

char *
nvram_get (const char *name)
{
//lock();
  size_t count = strlen (name) + 1;
  char tmp[100], *value;
  unsigned long *off = (unsigned long *) tmp;

  if (nvram_fd < 0)
    if (nvram_init (NULL))
      {
	//unlock();
	return NULL;
      }
  if (count > sizeof (tmp))
    {
      if (!(off = malloc (count)))
	{
	  //unlock();
	  return NULL;
	}
    }

  /* Get offset into mmap() space */
  strcpy ((char *) off, name);

  count = read (nvram_fd, off, count);

  if (count == sizeof (unsigned long))
    value = &nvram_buf[*off];
  else
    value = NULL;

  if (count < 0)
    perror (PATH_DEV_NVRAM);

  if (off != (unsigned long *) tmp)
    free (off);
  //unlock();

//fprintf(stderr,"nvram_get %s = %s\n",name,value!=NULL?value:"");
//fprintf(stderr,"NVRAM_GET(%s)=%s\n",name,value);
  return value;
}

int
nvram_getall (char *buf, int count)
{
//fprintf(stderr,"getall\n");
//lock();
  int ret;

  if (nvram_fd < 0)
    if ((ret = nvram_init (NULL)))
      {
	//unlock();
	return ret;
      }
  if (count == 0)
    {
      //unlock();
      return 0;
    }
  /* Get all variables */
  *buf = '\0';

  ret = read (nvram_fd, buf, count);

  if (ret < 0)
    perror (PATH_DEV_NVRAM);
  //unlock();
  return (ret == count) ? 0 : ret;
}

void nvram_open(void) // dummy
{
}
void nvram_close(void) //dummy
{
}

static int
_nvram_set (const char *name, const char *value)
{
  size_t count = strlen (name) + 1;
  char tmp[100], *buf = tmp;
  int ret;

  if (nvram_fd < 0)
    if ((ret = nvram_init (NULL)))
      return ret;

  /* Wolf add - keep nvram varname to sane len - may prevent corruption */
  if (strlen (name) > 64)
    return -ENOMEM;

  /* Unset if value is NULL */
  if (value)
    count += strlen (value) + 1;

  if (count > sizeof (tmp))
    {
      if (!(buf = malloc (count)))
	return -ENOMEM;
    }

  if (value)
    sprintf (buf, "%s=%s", name, value);
  else
    strcpy (buf, name);

  ret = write (nvram_fd, buf, count);

  if (ret < 0)
    perror (PATH_DEV_NVRAM);

  if (buf != tmp)
    free (buf);

  return (ret == count) ? 0 : ret;
}

int
nvram_set (const char *name, const char *value)
{
//fprintf(stderr,"nvram_set %s=%s\n",name,value);

  extern struct nvram_convert nvram_converts[];
  struct nvram_convert *v;
  int ret;
//lock();
  ret = _nvram_set (name, value);

  for (v = nvram_converts; v->name; v++)
    {
      if (!strcmp (v->name, name))
	{
	  if (strcmp (v->wl0_name, ""))
	    _nvram_set (v->wl0_name, value);
	  if (strcmp (v->d11g_name, ""))
	    _nvram_set (v->d11g_name, value);
	//Jemmy add for dual band wireless 2008.3.1
//      if (!nvram_match("restore_defaults", "0") || !nvram_match("os_name", "linux"))	 
//    {
//	  if(strcmp(v->wl1_name,""))
//	    _nvram_set(v->wl1_name, value);
//	}
//	  if ((!strcmp(v->wl1_name, "wl1_maclist")) || (!strcmp(v->wl1_name, "wl1_macmode")))
//		_nvram_set(v->wl1_name, value);	    
	}
    }
//unlock();
  return ret;
}
int nvram_immed_set (const char *name, const char *value)
{
return nvram_set(name,value);
}


int
nvram_unset (const char *name)
{
//lock();
//fprintf(stderr,"nvram_unset %s\n",name);
  int v = _nvram_set (name, NULL);
//unlock();
  return v;
}

int
nvram_commit (void)
{
system("/sbin/ledtool 1");
//fprintf(stderr,"nvram_commit \n");
  lock ();
  int ret;
  //fprintf (stderr, "nvram_commit(): start\n");
  if (nvram_fd < 0)
    {
      if ((ret = nvram_init (NULL)))
	{
	  fprintf (stderr, "nvram_commit(): failed\n");
	  unlock ();
	  return ret;
	}
    }
  ret = ioctl (nvram_fd, NVRAM_MAGIC, NULL);

  if (ret < 0)
    {
      fprintf (stderr, "nvram_commit(): failed\n");
      perror (PATH_DEV_NVRAM);
    }

  fprintf (stderr, "nvram_commit(): end\n");
  unlock ();
  return ret;
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
