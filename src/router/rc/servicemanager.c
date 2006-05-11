/*
 * DD-WRT servicemanager.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <sebastian.gottschall@blueline-ag.de>
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
 */


#include <dlfcn.h>
#include <stdio.h>
//#include <shutils.h>

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



int
start_service (char *name)
{
  cprintf ("start_service\n");
  char service[64];
  sprintf (service, "/etc/config/%s", name);
  FILE *ck = fopen (service, "rb");
  if (ck != NULL)
    {
      fclose (ck);
      cprintf ("found shell based service %s\n", service);
      return system (service);
    }
  void *handle = load_service (name);
  if (handle == NULL)
    {
      return -1;
    }
  void (*fptr) (void);
  sprintf (service, "start_%s", name);
  cprintf ("resolving %s\n", service);
  fptr = (void (*)(void)) dlsym (handle, service);
  if (fptr)
    (*fptr) ();
  else
    fprintf (stderr, "function %s not found \n", service);
  dlclose (handle);
  return 0;
  cprintf ("start_sevice done()\n");
}

int
start_servicep (char *name, char *param)
{
  cprintf ("start_servicep\n");
  void *handle = load_service (name);
  if (handle == NULL)
    {
      return -1;
    }
  void (*fptr) (char *);
  char service[64];
  sprintf (service, "start_%s", name);
  cprintf ("resolving %s\n", service);
  fptr = (void (*)(char *)) dlsym (handle, service);
  if (fptr)
    (*fptr) (param);
  else
    fprintf (stderr, "function %s not found \n", service);
  dlclose (handle);
  cprintf ("start_sevicep done()\n");
  return 0;
}

int
start_servicei (char *name, int param)
{
  cprintf ("start_servicei\n");
  void *handle = load_service (name);
  if (handle == NULL)
    {
      return -1;
    }
  void (*fptr) (int);
  char service[64];
  sprintf (service, "start_%s", name);
  cprintf ("resolving %s\n", service);
  fptr = (void (*)(int)) dlsym (handle, service);
  if (fptr)
    (*fptr) (param);
  else
    fprintf (stderr, "function %s not found \n", service);
  dlclose (handle);
  cprintf ("start_sevicei done()\n");
  return 0;
}


int
start_main (char *name, int argc, char **argv)
{
  cprintf ("start_main\n");
  void *handle = load_service (name);
  if (handle == NULL)
    {
      return -1;
    }
  int (*fptr) (int, char **);
  char service[64];
  sprintf (service, "%s_main", name);
  cprintf ("resolving %s\n", service);
  fptr = (int (*)(int, char **)) dlsym (handle, service);
  if (fptr)
    (*fptr) (argc, argv);
  else
    fprintf (stderr, "function %s not found \n", service);
  dlclose (handle);
  cprintf ("start_main done()\n");
  return 0;
}

int
stop_service (char *name)
{
  cprintf ("stop service()\n");
  void *handle = load_service (name);
  if (handle == NULL)
    {
      return -1;
    }
  void (*fptr) (void);
  char service[64];
  sprintf (service, "stop_%s", name);
  cprintf ("resolving %s\n", service);
  fptr = (void (*)(void)) dlsym (handle, service);
  if (fptr)
    (*fptr) ();
  else
    fprintf (stderr, "function %s not found \n", service);
  dlclose (handle);
  cprintf ("stop_service done()\n");

  return 0;
}


void
startstop (char *name)
{
  cprintf ("stop and start service\n");
  stop_service (name);
  start_service (name);
}
