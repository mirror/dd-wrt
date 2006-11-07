
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
#include <dlfcn.h>



#include <broadcom.h>
#include <cyutils.h>

#define MIN_BUF_SIZE    4096

#define SERVICE_MODULE "/usr/lib/services.so"
#define cprintf(fmt, args...)

/*
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)
*/


void *
load_service (char *name)
{
  cprintf ("load service %s\n", name);
  void *handle = dlopen (SERVICE_MODULE, RTLD_LAZY);
  cprintf ("done()\n");
  if (handle == NULL && name != NULL)
    {
      cprintf ("not found, try to load alternate\n");
      char dl[64];
      sprintf (dl, "/usr/lib/%s_service.so", name);
      cprintf ("try to load %s\n", dl);
      handle = dlopen (dl, RTLD_LAZY);
      if (handle == NULL)
	{
	  fprintf (stderr, "cannot load %s\n", dl);
	  return NULL;
	}
    }
  cprintf ("found it, returning handle\n");
  return handle;
}


void *fhandle;
void *
getPointer (char *name)
{
  fhandle = load_service (name);
  if (fhandle == NULL)
    {
      return -1;
    }
  void *fptr;
  fptr = (int (*)(int, char **)) dlsym (fhandle, name);
  return fptr;
}

void
closePointer (void)
{
  if (fhandle)
    dlclose (fhandle);
}


static int restore_ret;


static char *filter[] = { "lan_ifnames",
  "lan_ifname",
  "wan_ifnames",
  "wan_ifname",
  "et0macaddr",
  "il0macaddr",
  "boardnum",
  "boardtype",
  "boardrev",
  "melco_id",
  "product_name",
  "phyid_num",
  "cardbus",
  "CFEver",
  NULL
};
void
nv_file_in (char *url, webs_t wp, int len, char *boundary)
{
  char buf[1024];
  restore_ret = EINVAL;
  char sign[7];
  sign[6] = 0;
  char *nvram_ver = NULL;

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

  // fprintf (stderr, "file write");
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
    again:;
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
	  if (!strcmp (name, "nvram_ver"))
	    nvram_ver = value;
	  int a = 0;
	  while (filter[a] != NULL)
	    {
	      if (!strcmp (name, filter[a++]))
		{
		  free (value);
		  free (name);
		  goto again;
		}
	    }
	  nvram_set (name, value);
	  free (value);
	  free (name);
	}
      nvram_commit ();
      restore_ret = 0;
    }
  else if (!strcmp (sign, "XX-WRT"))
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
	  int a;
	  for (a = 0; a < c; a++)
	    name[a] ^= 37;
	  name[c] = 0;
	  len -= (c + 1);
	  wfread (&l, 2, 1, wp);
	  char *value = (char *) malloc (l + 1);
	  wfread (value, l, 1, wp);
	  for (a = 0; a < l; a++)
	    value[a] ^= 37;
	  len -= (l + 2);
	  value[l] = 0;
	  //cprintf("setting %s to %s\n",name,value);
	  if (!strcmp (name, "nvram_ver"))
	    nvram_ver = value;
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
  if (nvram_ver == NULL)
    {
      nvram_set ("http_passwd", zencrypt (nvram_safe_get ("http_passwd")));
      nvram_set ("http_username",
		 zencrypt (nvram_safe_get ("http_username")));
      if (nvram_get ("newhttp_passwd") != NULL)
	{
	  nvram_set ("newhttp_passwd",
		     zencrypt (nvram_safe_get ("newhttp_passwd")));
	  nvram_set ("newhttp_username",
		     zencrypt (nvram_safe_get ("newhttp_username")));
	}
    }

  chdir ("/www");
}

void
sr_config_cgi (char *path, webs_t wp)
{
  if (restore_ret != 0)
    do_ej ("Fail.asp", wp);
  else
    do_ej ("Success_rest.asp", wp);

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

  // struct nvram_tuple *router_defaults =
//    (struct nvram_tuple *) getPointer ("srouter_defaults");
//  if (router_defaults == NULL)
//    {
//      closePointer ();
//      return;
//    }
//  struct nvram_tuple *v;
  int backupcount = 0;
#ifdef HAVE_NEWMEDIA
  char sign[7] = { "XX-WRT" };
#else
  char sign[7] = { "DD-WRT" };
#endif

  char *buf = (char *) malloc (NVRAM_SPACE);
  nvram_getall (buf, NVRAM_SPACE);
  char *p = buf;
  int i;
  for (i = 0; i < NVRAM_SPACE; i++)
    {
      if (buf[i] == '=')
	backupcount++;
    }
  wfwrite (sign, 6, 1, wp);
  wfputc (backupcount & 255, wp);	//high byte
  wfputc (backupcount >> 8, wp);	//low byte
  while (strlen (p) != 0)
    {
      int len = strlen (p);
      for (i = 0; i < len; i++)
	if (p[i] == '=')
	  p[i] = 0;
      char *name = p;
      wfputc (strlen (name), wp);

#ifdef HAVE_NEWMEDIA
      for (i = 0; i < strlen (name); i++)
	wfputc (name[i] ^ 37, wp);
#else
      for (i = 0; i < strlen (name); i++)
	wfputc (name[i], wp);
#endif
      char *val = nvram_safe_get (name);
      wfputc (strlen (val) & 255, wp);
      wfputc (strlen (val) >> 8, wp);
#ifdef HAVE_NEWMEDIA
      for (i = 0; i < strlen (val); i++)
	wfputc (val[i] ^ 37, wp);
#else
      for (i = 0; i < strlen (val); i++)
	wfputc (val[i], wp);
#endif

      p += len+1;
    }
    free(buf);

//  closePointer ();
  return;
}
