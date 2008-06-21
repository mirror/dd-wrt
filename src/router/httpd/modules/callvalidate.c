/*
 * DD-WRT servicemanager.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <sebastian.gottschall@blueline-ag.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License
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
 */
#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <cyutils.h>
#include <support.h>
#include <cy_conf.h>
//#ifdef EZC_SUPPORT
#include <ezc.h>
//#endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>


#include <dlfcn.h>
#include <stdio.h>
//#include <shutils.h>

#define SERVICE_MODULE "/usr/lib/validate.so"
//#define SERVICE_MODULE "/tmp/validate.so"
#define cprintf(fmt, args...)


#ifndef cprintf
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp,"%s (%d):%s ",__FILE__,__LINE__,__func__); \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)
#endif

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
      sprintf (dl, "/usr/lib/%s_validate.so", name);
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
extern struct wl_client_mac *wl_client_macs;

void
start_gozila (char *name, webs_t wp)
{
//  lcdmessaged("Starting Service",name);
  cprintf ("start_gozila %s\n", name);
  char service[64];
  void *handle = load_service (name);
  if (handle == NULL)
    {
      return;
    }


  void (*init) (char *(*web) (webs_t wp, char *var, char *d),
		int (*filter) (char *old_name, char *new_name, size_t size,
			       int type),struct wl_client_mac *macs,int (*write) (webs_t wp, char *fmt, ...));
  init =
    (void (*)
     (char *(*web) (webs_t wp, char *var, char *d),
      int (*filter) (char *old_name, char *new_name, size_t size,
		     int type),struct wl_client_mac *macs,int (*write) (webs_t wp, char *fmt, ...))) dlsym (handle, "initWeb");
  if (!init)
    {
      fprintf (stderr, "error, initWeb not found\n");
      return;
    }
  init (websGetVar, httpd_filter_name,wl_client_macs,websWrite);

  int (*fptr) (webs_t wp);
  sprintf (service, "%s", name);
  cprintf ("resolving %s\n", service);
  fptr = (int (*)(webs_t wp)) dlsym (handle, service);
  if (fptr)
    (*fptr) (wp);
  else
    fprintf (stderr, "function %s not found \n", service);
  dlclose (handle);
  cprintf ("start_sevice done()\n");
}

int
start_validator (char *name, webs_t wp, char *value, struct variable *v)
{
//  lcdmessaged("Starting Service",name);
  cprintf ("start_validator %s\n", name);
  char service[64];
  void *handle = load_service (name);
  if (handle == NULL)
    {
      return FALSE;
    }


  int ret = FALSE;
  void (*init) (char *(*web) (webs_t wp, char *var, char *d),
		int (*filter) (char *old_name, char *new_name, size_t size,
			       int type),struct wl_client_mac *macs,int (*write) (webs_t wp, char *fmt, ...));

  init =
    (void (*)
     (char *(*web) (webs_t wp, char *var, char *d),
      int (*filter) (char *old_name, char *new_name, size_t size,
		     int type),struct wl_client_mac *macs,int (*write) (webs_t wp, char *fmt, ...))) dlsym (handle, "initWeb");
  if (!init)
    {
      fprintf (stderr, "error, initWeb not found\n");
      return ret;
    }
  init (websGetVar, httpd_filter_name,wl_client_macs,websWrite);

  int (*fptr) (webs_t wp, char *value, struct variable * v);
  sprintf (service, "%s", name);
  cprintf ("resolving %s\n", service);
  fptr =
    (int (*)(webs_t wp, char *value, struct variable * v)) dlsym (handle,
								  service);
  if (fptr)
    ret = (*fptr) (wp, value, v);
  else
    fprintf (stderr, "function %s not found \n", service);
  dlclose (handle);
  cprintf ("start_sevice done()\n");
  return ret;
}



void *
start_validator_nofree (char *name, void *handle, webs_t wp, char *value,
			struct variable *v)
{
//  lcdmessaged("Starting Service",name);
  cprintf ("start_service_nofree %s\n", name);
  char service[64];
  int nohandle = 0;
  if (!handle)
    {
      handle = load_service (name);
      nohandle = 1;
    }
  if (handle == NULL)
    {
      return NULL;
    }
  void (*fptr) (webs_t wp, char *value, struct variable * v);
  sprintf (service, "%s", name);
  if (nohandle)
    {
      cprintf ("resolve init\n");
  void (*init) (char *(*web) (webs_t wp, char *var, char *d),
		int (*filter) (char *old_name, char *new_name, size_t size,
			       int type),struct wl_client_mac *macs,int (*write) (webs_t wp, char *fmt, ...));

      init =
	(void (*)
	 (char *(*web) (webs_t wp, char *var, char *d),
	  int (*filter) (char *old_name, char *new_name, size_t size,
			 int type),struct wl_client_mac *macs,int (*write) (webs_t wp, char *fmt, ...))) dlsym (handle, "initWeb");
      if (!init)
	{
	  fprintf (stderr, "error, initWeb not found\n");
	  return NULL;
	}
      cprintf ("call init");
      init (websGetVar, httpd_filter_name,wl_client_macs,websWrite);
    }
  cprintf ("resolving %s\n", service);
  fptr =
    (void (*)(webs_t wp, char *value, struct variable * v)) dlsym (handle,
								   service);
  cprintf ("found. pointer is %p\n", fptr);
  if (fptr)
    (*fptr) (wp, value, v);
  else
    fprintf (stderr, "function %s not found \n", service);
  cprintf ("start_sevice_nofree done()\n");
  return handle;
}
