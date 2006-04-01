/* dnsmasq is Copyright (c) 2000-2006 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* Author's email: simon@thekelleys.org.uk */

#include "dnsmasq.h"

static struct dhcp_lease *leases;
static int dns_dirty;
enum
{ no, yes, force } file_dirty;
static int leases_left;

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
file_dirty=no;
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
	  file_dirty = yes;

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
  lease->expires = 1;
  
  if (!lease_set_hwaddr(lease, hwaddr, clid, hw_len, hw_type, clid_len))
    {
      free(lease);
      return NULL;
    }

  lease->next = leases;
  leases = lease;
  
  file_dirty = force;
  leases_left--;

  return lease;
}

void lease_set_expires(struct dhcp_lease *lease, time_t exp)
{
  if (exp != lease->expires)
    {
      file_dirty = yes;
      dns_dirty = 1;
    }
  lease->expires = exp;
}

int lease_set_hwaddr(struct dhcp_lease *lease, unsigned char *hwaddr,
		      unsigned char *clid, int hw_len, int hw_type, int clid_len)
{
  if (hw_len != lease->hwaddr_len ||
      hw_type != lease->hwaddr_type || 
      memcmp(lease->hwaddr, hwaddr, hw_len) != 0)
    {
      file_dirty = force;
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
	  file_dirty = force;
	  if (lease->clid)
	    free(lease->clid);
	  if (!(lease->clid = malloc(clid_len)))
	    return 0;
	}
      else if (memcmp(lease->clid, clid, clid_len) != 0)
	file_dirty = force;

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
  
  file_dirty = force;
  dns_dirty = 1;
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
lease_update_file (struct daemon *daemon, int always, time_t now)
{
  struct dhcp_lease *lease;
  int i = always;		/* avoid warning */
  unsigned long expires;
  struct lease_t l;
  int a;
/* DD-WRT udhcpd lease file compatibility */
fprintf(stderr,"update lease file\n");

//  if (file_dirty != no)
    {
      rewind (daemon->lease_stream);
      ftruncate (fileno (daemon->lease_file), 0);
fprintf(stderr,"rewindet\n");

      for (lease = leases; lease; lease = lease->next)
	{
	 if (lease->expires) 
	  expires = (unsigned long) lease->expires - now;
	  
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

      file_dirty = no;
/* DD-WRT end */

    }
}

