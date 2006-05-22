/* dnsmasq is Copyright (c) 2000 - 2005 by Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


/* Code in this file is based on contributions by John Volpe. */

#include "dnsmasq.h"

#ifdef HAVE_ISC_READER

#define MAXTOK 50

struct isc_lease {
  char *name, *fqdn;
  time_t expires;
  struct in_addr addr;
  struct isc_lease *next;
};

static struct isc_lease *leases = NULL;
static off_t lease_file_size = (off_t)0;
static ino_t lease_file_inode = (ino_t)0;
static int logged_lease = 0;

static int next_token (char *token, int buffsize, FILE * fp)
{
  int c, count = 0;
  char *cp = token;
  
  while((c = getc(fp)) != EOF)
    {
      if (c == '#')
	do { c = getc(fp); } while (c != '\n' && c != EOF);
      
      if (c == ' ' || c == '\t' || c == '\n' || c == ';')
	{
	  if (count)
	    break;
	}
      else if ((c != '"') && (count<buffsize-1))
	{
	  *cp++ = c;
	  count++;
	}
    }
  
  *cp = 0;
  return count ? 1 : 0;
}

void load_dhcp(struct daemon *daemon, time_t now)
{
  char *hostname = daemon->namebuff;
  char token[MAXTOK], *dot;
  struct in_addr host_address;
  time_t ttd, tts;
  FILE *fp;
  struct isc_lease *lease, *tmp, **up;
  struct stat statbuf;

  if (stat(daemon->lease_file, &statbuf) == -1)
    {
      if (!logged_lease)
	syslog(LOG_WARNING, _("failed to access %s: %m"), daemon->lease_file);
      logged_lease = 1;
      return;
    }
  
  logged_lease = 0;
  
  if ((statbuf.st_size <= lease_file_size) &&
      (statbuf.st_ino == lease_file_inode))
    return;
  
  lease_file_size = statbuf.st_size;
  lease_file_inode = statbuf.st_ino;
  
  if (!(fp = fopen (daemon->lease_file, "r")))
    {
      syslog (LOG_ERR, _("failed to load %s: %m"), daemon->lease_file);
      return;
    }
  
  syslog (LOG_INFO, _("reading %s"), daemon->lease_file);

  while ((next_token(token, MAXTOK, fp)))
    {
      if (strcmp(token, "lease") == 0)
        {
          hostname[0] = '\0';
	  ttd = tts = (time_t)(-1);
	  if (next_token(token, MAXTOK, fp) && 
	      (host_address.s_addr = inet_addr(token)) != (in_addr_t) -1)
            {
              if (next_token(token, MAXTOK, fp) && *token == '{')
                {
                  while (next_token(token, MAXTOK, fp) && *token != '}')
                    {
                      if ((strcmp(token, "client-hostname") == 0) ||
			  (strcmp(token, "hostname") == 0))
			{
			  if (next_token(hostname, MAXDNAME, fp))
			    if (!canonicalise(hostname))
			      {
				*hostname = 0;
				syslog(LOG_ERR, _("bad name in %s"), daemon->lease_file); 
			      }
			}
                      else if ((strcmp(token, "ends") == 0) ||
			       (strcmp(token, "starts") == 0))
                        {
                          struct tm lease_time;
			  int is_ends = (strcmp(token, "ends") == 0);
			  if (next_token(token, MAXTOK, fp) &&  /* skip weekday */
			      next_token(token, MAXTOK, fp) &&  /* Get date from lease file */
			      sscanf (token, "%d/%d/%d", 
				      &lease_time.tm_year,
				      &lease_time.tm_mon,
				      &lease_time.tm_mday) == 3 &&
			      next_token(token, MAXTOK, fp) &&
			      sscanf (token, "%d:%d:%d:", 
				      &lease_time.tm_hour,
				      &lease_time.tm_min, 
				      &lease_time.tm_sec) == 3)
			    {
			      /* There doesn't seem to be a universally available library function
				 which converts broken-down _GMT_ time to seconds-in-epoch.
				 The following was borrowed from ISC dhcpd sources, where
                                 it is noted that it might not be entirely accurate for odd seconds.
				 Since we're trying to get the same answer as dhcpd, that's just
				 fine here. */
			      static const int months [11] = { 31, 59, 90, 120, 151, 181,
							       212, 243, 273, 304, 334 };
			      time_t time = ((((((365 * (lease_time.tm_year - 1970) + /* Days in years since '70 */
						  (lease_time.tm_year - 1969) / 4 +   /* Leap days since '70 */
						  (lease_time.tm_mon > 1                /* Days in months this year */
						   ? months [lease_time.tm_mon - 2]
						   : 0) +
						  (lease_time.tm_mon > 2 &&         /* Leap day this year */
						   !((lease_time.tm_year - 1972) & 3)) +
						  lease_time.tm_mday - 1) * 24) +   /* Day of month */
						lease_time.tm_hour) * 60) +
					      lease_time.tm_min) * 60) + lease_time.tm_sec;
			      if (is_ends)
				ttd = time;
			      else
				tts = time;			    }
                        }
		    }
		  
		  /* missing info? */
		  if (!*hostname)
		    continue;
		  if (ttd == (time_t)(-1))
		    continue;
		  
		  /* We use 0 as infinite in ttd */
		  if ((tts != -1) && (ttd == tts - 1))
		    ttd = (time_t)0;
		  else if (difftime(now, ttd) > 0)
		    continue;

		  if ((dot = strchr(hostname, '.')))
		    { 
		      if (!daemon->domain_suffix || hostname_isequal(dot+1, daemon->domain_suffix))
			{
			  syslog(LOG_WARNING, 
				 _("Ignoring DHCP lease for %s because it has an illegal domain part"), 
				 hostname);
			  continue;
			}
		      *dot = 0;
		    }

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
			  if (daemon->domain_suffix && 
			      (lease->fqdn = malloc(strlen(hostname) + strlen(daemon->domain_suffix) + 2)))
			    {
			      strcpy(lease->fqdn, hostname);
			      strcat(lease->fqdn, ".");
			      strcat(lease->fqdn, daemon->domain_suffix);
			    }
			}
		    }
		}
	    }
	}
    }

  fclose(fp);
  
  /* prune expired leases */
  for (lease = leases, up = &leases; lease; lease = tmp)
     {
       tmp = lease->next;
       if (lease->expires != (time_t)0 && difftime(now, lease->expires) > 0)
	 {
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
      cache_add_dhcp_entry(daemon, lease->fqdn, &lease->addr, lease->expires);
      cache_add_dhcp_entry(daemon, lease->name, &lease->addr, lease->expires);
    }
}

#endif

