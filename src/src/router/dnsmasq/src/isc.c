/* dnsmasq is Copyright (c) 2000 - 2004 by Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


/* Code in this file is based on contributions by John Volpe. */
/* DHCP altered to process udhcp format by W.J. van der Laan 2003 */
/* Binary lease file code copied from Wifi-box WRT54G firmware source code */
/* Binary lease file code merged into this file by Rod Whitby */

#include "dnsmasq.h"

#ifdef HAVE_ISC_READER

struct lease_t {
	unsigned char chaddr[16];
	u_int32_t yiaddr;
	u_int32_t expires;
	char hostname[64];
};
#define EXPIRES_NEVER 0xFFFFFFFF

struct isc_lease {
  char *name, *fqdn;
  time_t expires;
  struct in_addr addr;
  struct isc_lease *next;
};

static struct isc_lease *leases = NULL;
static int logged_lease = 0;

//void load_dhcp(char *file, char *suffix, time_t now, char *hostname)
//{
FILE *load_dhcp(struct daemon *daemon, time_t now)
{
  char *hostname = daemon->namebuff;
  char *file = daemon->lease_file;
  char *suffix = daemon->domain_suffix;
  char *dot;
  struct in_addr host_address;
  time_t ttd;
  FILE *fp;
  struct lease_t binlease;
  struct isc_lease *lease, *tmp, **up;
  struct stat statbuf;

  logged_lease = 0;
  
  
//  if (!(fp = fopen (file, "r+b")))
//    {
fprintf(stderr,"opening %s\n",file);
      if (!(fp = fopen (file, "wb")))
      {
fprintf(stderr,"error while opening %s\n",file);
      syslog (LOG_ERR, "failed to load %s: %m", file);
      return NULL;
      }
//    }
fprintf(stderr,"done()\n");
  
  syslog (LOG_INFO, "reading %s", file);

  while (fread(&binlease, sizeof(binlease), 1, fp))
    {
      /* Skip empty hostnames */
      if(!binlease.hostname[0])
	continue;
  	
      strncpy(hostname, binlease.hostname, MAXDNAME);
      hostname[MAXDNAME-1] = 0;
  	
      if (!canonicalise(hostname))
	{
	  *hostname = 0;
	  syslog(LOG_ERR, "bad name in %s", file); 
	}
      
      if ((dot = strchr(hostname, '.')))
	{ 
	  if (!suffix || hostname_isequal(dot+1, suffix))
	    {
	      syslog(LOG_WARNING, 
		     "Ignoring DHCP lease for %s because it has an illegal domain part", 
		     hostname);
	      continue;
	    }
	  *dot = 0;
	}

      /* Address */
      host_address.s_addr = binlease.yiaddr;

      /* Lease */
      binlease.expires = ntohl(binlease.expires);

      /* Calculate time to death */
      ttd = (time_t)(0); /* infinite */
      if (binlease.expires != EXPIRES_NEVER) {
	/* WRT54G uses time remaining */
	ttd = now + binlease.expires;
	if (ttd < now)
	  /* expired already */
	  continue;
      }
      
      syslog(LOG_INFO, "found lease for %s", hostname); 

      for (lease = leases; lease; lease = lease->next)
	if (hostname_isequal(lease->name, hostname))
	  {
	    lease->expires = ttd;
	    lease->addr = host_address;
	    break;
	  }
      
      if (!lease && (lease = malloc(sizeof(struct isc_lease))))
	{
	  lease->expires = ttd;
	  lease->addr = host_address;
	  lease->fqdn =  NULL;
	  lease->next = leases;
	  if (!(lease->name = malloc(strlen(hostname)+1)))
	    free(lease);
	  else
	    {
	      leases = lease;
	      strcpy(lease->name, hostname);
	      if (suffix && (lease->fqdn = malloc(strlen(hostname) + strlen(suffix) + 2)))
		{
		  strcpy(lease->fqdn, hostname);
		  strcat(lease->fqdn, ".");
		  strcat(lease->fqdn, suffix);
		}
	    }
	}
    }

  
  /* prune expired leases */
  for (lease = leases, up = &leases; lease; lease = tmp)
     {
       tmp = lease->next;
       if (lease->expires != (time_t)0 && difftime(now, lease->expires) > 0)
	 {
	   syslog(LOG_INFO, "expired lease for %s", lease->name); 
	   *up = lease->next; /* unlink */
	   free(lease->name);
	   if (lease->fqdn)
	     free(lease->fqdn);
	   free(lease);
	 }
       else
	 up = &lease->next;
     }

     
  /* remove all existing DHCP cache entries */
  cache_unhash_dhcp();

  for (lease = leases; lease; lease = lease->next)
    {
      if (lease->fqdn)
	{
	  cache_add_dhcp_entry(daemon,lease->fqdn, &lease->addr, lease->expires);
	  cache_add_dhcp_entry(daemon,lease->name, &lease->addr, lease->expires);
	}
      else 
	cache_add_dhcp_entry(daemon,lease->name, &lease->addr, lease->expires);

      syslog(LOG_INFO, "stored lease for %s", lease->name); 

    }
return fp;
}

#endif

