/* dnsmasq is Copyright (c) 2000-2006 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

static struct dhcp_lease *leases;
static int dns_dirty, file_dirty, leases_left;

void
lease_init (struct daemon *daemon, time_t now)
{
//printf("stored lease left"); 

leases = NULL;
leases_left = daemon->dhcp_max;
//printf("load dhcp leases"); 
FILE *lease_file=load_dhcp(daemon,now);
rewind(lease_file);
//printf("done"); 
file_dirty=0;
dns_dirty=1;
daemon->lease_stream = lease_file;

}


void lease_update_from_configs(struct daemon *daemon)
{
  /* changes to the config may change current leases. */
  
  struct dhcp_lease *lease;
  struct dhcp_config *config;
  char *name;

  for (lease = leases; lease; lease = lease->next)
    if ((config = find_config(daemon->dhcp_conf, NULL, lease->clid, lease->clid_len, 
			      lease->hwaddr, lease->hwaddr_len, lease->hwaddr_type, NULL)) && 
	(config->flags & CONFIG_NAME) &&
	(!(config->flags & CONFIG_ADDR) || config->addr.s_addr == lease->addr.s_addr))
      lease_set_hostname(lease, config->hostname, daemon->domain_suffix, 1);
    else if ((name = host_from_dns(daemon, lease->addr)))
      lease_set_hostname(lease, name, daemon->domain_suffix, 1); /* updates auth flag only */
}


struct lease_t
{
  unsigned char chaddr[16];
  unsigned int yiaddr;
  unsigned int expires;
  char hostname[64];
};
#define EXPIRES_NEVER 0xFFFFFFFF	/* static lease */

void
lease_update_file (struct daemon *daemon)
{
  struct dhcp_lease *lease;
//  int i = always;		/* avoid warning */
  unsigned long expires;
  struct lease_t l;
  int a;
/* DD-WRT udhcpd lease file compatibility */
fprintf(stderr,"update lease file\n");

  if (file_dirty != 0)
    {
      rewind (daemon->lease_stream);
      ftruncate (fileno (daemon->lease_stream), 0);
fprintf(stderr,"rewindet\n");

      for (lease = leases; lease; lease = lease->next)
	{
	 if (lease->expires) 
	  expires = (unsigned long) lease->expires;
	  
	  memset (&l, 0, sizeof (l));
	  if (lease->hwaddr != NULL)
	    {
	      for (a = 0; a < 6; a++)
		l.chaddr[a] = lease->hwaddr[a];
	    }

	  l.yiaddr = lease->addr.s_addr;	//ip
	  if (!lease->expires)
	    expires = EXPIRES_NEVER;
	  
	  l.expires = htonl (expires);
	  if (lease->hostname != NULL)
	    {
	      strcpy (l.hostname, lease->hostname);
	    }
	  else
	    {
	      if (lease->fqdn != NULL)
		strcpy (l.hostname, lease->fqdn);
	    }
fprintf(stderr,"write lease\n");


	  fwrite (&l, sizeof (l), 1, daemon->lease_stream);
	}
fprintf(stderr,"done()\n");

      fflush (daemon->lease_stream);
      fsync (fileno (daemon->lease_stream));
fprintf(stderr,"sync()\n");

      file_dirty = 0;
/* DD-WRT end */

    }
}



/*
void lease_update_file(struct daemon *daemon)
{
  struct dhcp_lease *lease;
  int i;
  
  if (file_dirty != 0)
    {
      errno = 0;
      rewind(daemon->lease_stream);
      if (errno != 0 || ftruncate(fileno(daemon->lease_stream), 0) != 0)
	{
	write_err:
	  syslog(LOG_ERR, _("failed to write %s: %m (retry in %ds)"), daemon->lease_file, LEASE_RETRY);
	  alarm(LEASE_RETRY);
	  return;
	}
      
      for (lease = leases; lease; lease = lease->next)
	{
#ifdef HAVE_BROKEN_RTC
	  if (fprintf(daemon->lease_stream, "%u ", lease->length) < 0)
	    goto write_err;
#else
	  if (fprintf(daemon->lease_stream, "%lu ", (unsigned long)lease->expires) < 0)
	    goto write_err;
#endif
	  if ((lease->hwaddr_type != ARPHRD_ETHER || lease->hwaddr_len == 0) && 
	      fprintf(daemon->lease_stream, "%.2x-", lease->hwaddr_type) < 0)
	    goto write_err;
	  for (i = 0; i < lease->hwaddr_len; i++)
	    {
	      if (fprintf(daemon->lease_stream, "%.2x", lease->hwaddr[i]) < 0)
		goto write_err;
	      if (i != lease->hwaddr_len - 1 &&
		  fprintf(daemon->lease_stream, ":") < 0)
		goto write_err;
	    }
	  if (fprintf(daemon->lease_stream, " %s %s ", inet_ntoa(lease->addr),
		      lease->hostname && strlen(lease->hostname) != 0 ? lease->hostname : "*") < 0)
	    goto write_err;
	  
	  if (lease->clid && lease->clid_len != 0)
	    {
	      for (i = 0; i < lease->clid_len - 1; i++)
		if (fprintf(daemon->lease_stream, "%.2x:", lease->clid[i]) < 0)
		  goto write_err;
	      if (fprintf(daemon->lease_stream, "%.2x\n", lease->clid[i]) < 0)
		goto write_err;
	    }
	  else
	    if (fprintf(daemon->lease_stream, "*\n") < 0)
	      goto write_err;
	  
	}

      if (fflush(daemon->lease_stream) != 0)
	goto write_err;
      if (fsync(fileno(daemon->lease_stream)) < 0)
	goto write_err;
      file_dirty = 0;
    }
}
*/
void lease_update_dns(struct daemon *daemon)
{
  struct dhcp_lease *lease;
  
  if (dns_dirty)
    {
      cache_unhash_dhcp();
      
      for (lease = leases; lease; lease = lease->next)
	{
	  cache_add_dhcp_entry(daemon, lease->fqdn, &lease->addr, lease->expires);
	  cache_add_dhcp_entry(daemon, lease->hostname, &lease->addr, lease->expires);
	}
      
      dns_dirty = 0;
    }
}

void lease_prune(struct dhcp_lease *target, time_t now)
{
  struct dhcp_lease *lease, *tmp, **up;

  for (lease = leases, up = &leases; lease; lease = tmp)
    {
      tmp = lease->next;
      if ((lease->expires != 0 && difftime(now, lease->expires) > 0) || lease == target)
	{
	  file_dirty = 1;

	  *up = lease->next; /* unlink */
	  if (lease->hostname)
	    {
	      free(lease->hostname); 
	      dns_dirty = 1;
	    }
	  if (lease->fqdn)
	    free(lease->fqdn);
	  if (lease->clid)
	    free(lease->clid);
	  free(lease);
	  leases_left++;
	}
      else
	up = &lease->next;
    }
} 
	
  
struct dhcp_lease *lease_find_by_client(unsigned char *hwaddr, int hw_len, int hw_type,
					unsigned char *clid, int clid_len)
{
  struct dhcp_lease *lease;

  if (clid)
    for (lease = leases; lease; lease = lease->next)
      if (lease->clid && clid_len == lease->clid_len &&
	  memcmp(clid, lease->clid, clid_len) == 0)
	return lease;
  
  for (lease = leases; lease; lease = lease->next)	
    if ((!lease->clid || !clid) && 
	hw_len != 0 && 
	lease->hwaddr_len == hw_len &&
	lease->hwaddr_type == hw_type &&
	memcmp(hwaddr, lease->hwaddr, hw_len) == 0)
      return lease;
  
  return NULL;
}

struct dhcp_lease *lease_find_by_addr(struct in_addr addr)
{
  struct dhcp_lease *lease;

  for (lease = leases; lease; lease = lease->next)
    if (lease->addr.s_addr == addr.s_addr)
      return lease;
  
  return NULL;
}


struct dhcp_lease *lease_allocate(unsigned char *hwaddr, unsigned char *clid, 
				  int hw_len, int hw_type, int clid_len, struct in_addr addr)
{
  struct dhcp_lease *lease;
  if (!leases_left || !(lease = malloc(sizeof(struct dhcp_lease))))
    return NULL;

  lease->clid = NULL;
  lease->hostname = lease->fqdn = NULL;  
  lease->addr = addr;
  memset(lease->hwaddr, 0, DHCP_CHADDR_MAX);
  lease->hwaddr_len = 0;
  lease->hwaddr_type = 0;
  lease->expires = 1;
#ifdef HAVE_BROKEN_RTC
  lease->length = 0xffffffff; /* illegal value */
#endif
  
  if (!lease_set_hwaddr(lease, hwaddr, clid, hw_len, hw_type, clid_len))
    {
      free(lease);
      return NULL;
    }

  lease->next = leases;
  leases = lease;
  
  file_dirty = 1;
  leases_left--;

  return lease;
}

void lease_set_expires(struct dhcp_lease *lease, unsigned int len, time_t now)
{
  time_t exp = now + (time_t)len;
  
  if (len == 0xffffffff)
    {
      exp = 0;
      len = 0;
    }
  
  if (exp != lease->expires)
    {
      dns_dirty = 1;
      lease->expires = exp;
#ifndef HAVE_BROKEN_RTC
      file_dirty = 1;
#endif
    }
  
#ifdef HAVE_BROKEN_RTC
  if (len != lease->length)
    {
      lease->length = len;
      file_dirty = 1;
    }
#endif
} 

int lease_set_hwaddr(struct dhcp_lease *lease, unsigned char *hwaddr,
		      unsigned char *clid, int hw_len, int hw_type, int clid_len)
{
  /* must have some sort of unique-id */
  if (hw_len == 0 && (clid_len == 0 || !clid))
    return 0;

  if (hw_len != lease->hwaddr_len ||
      hw_type != lease->hwaddr_type || 
      hw_len == 0 ||
      memcmp(lease->hwaddr, hwaddr, hw_len) != 0)
    {
      file_dirty = 1;
      memcpy(lease->hwaddr, hwaddr, hw_len);
      lease->hwaddr_len = hw_len;
      lease->hwaddr_type = hw_type;
    }

  /* only update clid when one is available, stops packets
     without a clid removing the record. Lease init uses
     clid_len == 0 for no clid. */
  if (clid_len != 0 && clid)
    {
      if (!lease->clid)
	lease->clid_len = 0;

      if (lease->clid_len != clid_len)
	{
	  file_dirty = 1;
	  if (lease->clid)
	    free(lease->clid);
	  if (!(lease->clid = malloc(clid_len)))
	    return 0;
	}
      else if (memcmp(lease->clid, clid, clid_len) != 0)
	file_dirty = 1;
      
      lease->clid_len = clid_len;
      memcpy(lease->clid, clid, clid_len);
    }

  return 1;
}

void lease_set_hostname(struct dhcp_lease *lease, char *name, char *suffix, int auth)
{
  struct dhcp_lease *lease_tmp;
  char *new_name = NULL, *new_fqdn = NULL;

  if (lease->hostname && name && hostname_isequal(lease->hostname, name))
    {
      lease->auth_name = auth;
      return;
    }

  if (!name && !lease->hostname)
    return;

  /* If a machine turns up on a new net without dropping the old lease,
     or two machines claim the same name, then we end up with two interfaces with
     the same name. Check for that here and remove the name from the old lease.
     Don't allow a name from the client to override a name from dnsmasq config. */
  
  if (name)
    {
      for (lease_tmp = leases; lease_tmp; lease_tmp = lease_tmp->next)
	if (lease_tmp->hostname && hostname_isequal(lease_tmp->hostname, name))
	  {
	    if (lease_tmp->auth_name && !auth)
	      return;
	    new_name = lease_tmp->hostname;
	    lease_tmp->hostname = NULL;
	    if (lease_tmp->fqdn)
	      {
		new_fqdn = lease_tmp->fqdn;
		lease_tmp->fqdn = NULL;
	      }
	    break;
	  }
     
      if (!new_name && (new_name = malloc(strlen(name) + 1)))
	strcpy(new_name, name);
      
      if (suffix && !new_fqdn && (new_fqdn = malloc(strlen(name) + strlen(suffix) + 2)))
	{
	  strcpy(new_fqdn, name);
	  strcat(new_fqdn, ".");
	  strcat(new_fqdn, suffix);
	}
    }

  if (lease->hostname)
    free(lease->hostname);
  if (lease->fqdn)
    free(lease->fqdn);
  
  lease->hostname = new_name;
  lease->fqdn = new_fqdn;
  lease->auth_name = auth;
  
  file_dirty = 1;
  dns_dirty = 1;
}



