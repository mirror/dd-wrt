
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


#include <broadcom.h>
#include <cyutils.h>

#define MIN_BUF_SIZE    4096

extern struct nvram_tuple router_defaults[];
static int restore_ret;


void
nv_file_in (char *url, webs_t wp, int len, char *boundary)
{
  struct nvram_tuple *v;
  char buf[1024], *var;
  int nvrtotal = 0;
  FILE *fp = NULL;
  restore_ret = EINVAL;
  char sign[7];
  sign[6] = 0;
  /* Look for our part */
  while (len > 0)
    {
      if (!wfgets (buf, MIN (len + 1, sizeof (buf)), wp))
	return;
      len -= strlen (buf);
      if (!strncasecmp (buf, "Content-Disposition:", 20))
	{
	  if (strstr (buf, "name=\"file\""))
	    {
	      break;
	    }
	}
    }
  /* Skip boundary and headers */
  while (len > 0)
    {
      if (!wfgets (buf, sizeof (buf), wp))
	return;
      len -= strlen (buf);
      if (!strcmp (buf, "\n") || !strcmp (buf, "\r\n"))
	break;
    }

  // fprintf (stderr, "file wirte");
  unsigned short count;
  wfread (sign, 6, 1, wp);
  len -= 6;
  if (!strcmp (sign, "DD-WRT"))
    {
      wfread (&count, 2, 1, wp);
      len -= 2;
      int i;
      for (i = 0; i < count; i++)
	{
	  unsigned short l = 0;
	  unsigned char c = 0;
	  wfread (&c, 1, 1, wp);
	  char *name = (char *) malloc (c + 1);
	  wfread (name, c, 1, wp);
	  name[c] = 0;
	  len -= (c + 1);
	  wfread (&l, 2, 1, wp);
	  char *value = (char *) malloc (l + 1);
	  wfread (value, l, 1, wp);
	  len -= (l + 2);
	  value[l] = 0;
	  //cprintf("setting %s to %s\n",name,value);
	  nvram_set (name, value);
	  free (value);
	  free (name);
	}
      nvram_commit ();
      restore_ret = 0;
    }
  else
    {
      restore_ret = 99;
    }
  /* Slurp anything remaining in the request */
  while (len--)
    {
#ifdef HAVE_HTTPS
      if (do_ssl)
	{
#ifdef HAVE_OPENSSL
	  BIO_gets ((BIO *) wp, buf, 1);
#elif defined(HAVE_MATRIXSSL)
	  matrixssl_gets (wp, buf, 1);
#else

#endif
	}
      else
#endif
	(void) fgetc (wp);
    }
  chdir ("/www");
}

void
sr_config_cgi (char *path, webs_t wp)
{
  if (restore_ret != 0)
    do_ej ("Fail.asp", wp);
  else
    do_ej ("Success.asp", wp);

  websDone (wp, 200);

  /* Reboot if successful */
  if (restore_ret == 0)
    {
      nvram_commit ();
      sleep (1);
      sys_reboot ();
    }
}

void
nv_file_out (char *path, webs_t wp)
{
  FILE *fp = NULL;
  int ret = 0;
  char *var, *vertag;
  pid_t pid;
  struct nvram_tuple *v;
  char *backup_argv[5];
  struct sysinfo info;
  int backupcount = 0;
  char sign[7] = { "DD-WRT" };
  for (v = router_defaults; v->name; v++)
    {
      backupcount++;
    }
  wfwrite (sign, 6, 1, wp);
  wfputc (backupcount & 255, wp);	//high byte
  wfputc (backupcount >> 8, wp);	//low byte
  for (v = router_defaults; v->name; v++)
    {
      wfputc (strlen (v->name), wp);
      int i;
      for (i = 0; i < strlen (v->name); i++)
	wfputc (v->name[i], wp);
      char *val = nvram_safe_get (v->name);
      wfputc (strlen (val) & 255, wp);
      wfputc (strlen (val) >> 8, wp);
      for (i = 0; i < strlen (val); i++)
	wfputc (val[i], wp);
    }
  return;
}
