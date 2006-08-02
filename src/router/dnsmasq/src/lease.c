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

static struct dhcp_lease *leases, *old_leases;
static int dns_dirty, file_dirty, leases_left;

void lease_init(struct daemon *daemon, time_t now)
{
  unsigned long ei;
  struct in_addr addr;
  struct dhcp_lease *lease;
  int flags, clid_len, hw_len, hw_type;
  FILE *leasestream;
  
  leases = old_leases = NULL;
  leases_left = daemon->dhcp_max;

  if (daemon->options & OPT_LEASE_RO)
    {
      /* run "<lease_change_script> init" once to get the
	 initial state of the database. If leasefile-ro is
	 set without a script, we just do without any 
	 lease database. */
      if (!daemon->lease_change_command)
	{
	  file_dirty = dns_dirty = 0;
	  return;
	}
      strcpy(daemon->dhcp_buff, daemon->lease_change_command);
      strcat(daemon->dhcp_buff, " init");
      leasestream = popen(daemon->dhcp_buff, "r");
    }
  else
    {
      /* NOTE: need a+ mode to create file if it doesn't exist */
      leasestream = daemon->lease_stream = fopen(daemon->lease_file, "a+");
      
      if (!leasestream)
	die(_("cannot open or create lease file %s: %s"), daemon->lease_file);
      
      flags = fcntl(fileno(leasestream), F_GETFD);
      if (flags != -1)
	fcntl(fileno(leasestream), F_SETFD, flags | FD_CLOEXEC); 
      
      /* a+ mode lease pointer at end. */
      rewind(leasestream);
    }
  
  /* client-id max length is 255 which is 255*2 digits + 254 colons 
     borrow DNS packet buffer which is always larger than 1000 bytes */
  if (leasestream)
    while (fscanf(leasestream, "%lu %255s %16s %255s %764s",
		  &ei, daemon->dhcp_buff2, daemon->namebuff, 
		  daemon->dhcp_buff, daemon->packet) == 5)
      {
	hw_len = parse_hex(daemon->dhcp_buff2, (unsigned char *)daemon->dhcp_buff2, DHCP_CHADDR_MAX, NULL, &hw_type);
	/* For backwards compatibility, no explict MAC address type means ether. */
	if (hw_type == 0 && hw_len != 0)
	  hw_type = ARPHRD_ETHER;
	
	addr.s_addr = inet_addr(daemon->namebuff);
	
	/* decode hex in place */
	clid_len = 0;
	if (strcmp(daemon->packet, "*") != 0)
	  clid_len = parse_hex(daemon->packet, (unsigned char *)daemon->packet, 255, NULL, NULL);
	
	if (!(lease = lease_allocate(addr)))
	  die (_("too many stored leases"), NULL);
	/* not actually new */
	lease->new = 0;
	
#ifdef HAVE_BROKEN_RTC
	if (ei != 0)
	  lease->expires = (time_t)ei + now;
	else
	  lease->expires = (time_t)0;
	lease->length = ei;
#else
	/* strictly time_t is opaque, but this hack should work on all sane systems,
	   even when sizeof(time_t) == 8 */
	lease->expires = (time_t)ei;
#endif
	
	lease_set_hwaddr(lease, (unsigned char *)daemon->dhcp_buff2, (unsigned char *)daemon->packet, hw_len, hw_type, clid_len);
	
	if (strcmp(daemon->dhcp_buff, "*") !=  0)
	  lease_set_hostname(lease, daemon->dhcp_buff, daemon->domain_suffix, 0);
      }
  
  if (!daemon->lease_stream)
    {
      int rc = 0;

      /* shell returns 127 for "command not found", 126 for bad permissions. */
      if (!leasestream || (rc = pclose(leasestream)) == -1 || WEXITSTATUS(rc) == 127 || WEXITSTATUS(rc) == 126)
	{
	  if (WEXITSTATUS(rc) == 127)
	    errno = ENOENT;
	  else if (WEXITSTATUS(rc) == 126)
	    errno = EACCES;
	  die(_("cannot run lease-init script %s: %s"), daemon->lease_change_command);
	}
      
      if (WEXITSTATUS(rc) != 0)
	{
	  sprintf(daemon->dhcp_buff, "%d", WEXITSTATUS(rc));
	  die(_("lease-init script returned exit code %s"), daemon->dhcp_buff);
	}
    }

  /* Some leases may have expired */
  file_dirty = 0;
  lease_prune(NULL, now);
  dns_dirty = 1;
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

static void ourprintf(struct daemon *daemon, int *errp, char *format, ...)
{
  va_list ap;
  
  va_start(ap, format);
  if (!(*errp) && vfprintf(daemon->lease_stream, format, ap) < 0)
    *errp = errno;
  va_end(ap);
}

void lease_update_file(struct daemon *daemon, time_t now)
{
  struct dhcp_lease *lease;
  time_t next_event;
  int i, err = 0;

  if (file_dirty != 0 && daemon->lease_stream)
    {
      errno = 0;
      rewind(daemon->lease_stream);
      if (errno != 0 || ftruncate(fileno(daemon->lease_stream), 0) != 0)
	err = errno;
      
      for (lease = leases; lease; lease = lease->next)
	{
#ifdef HAVE_BROKEN_RTC
	  ourprintf(daemon, &err, "%u ", lease->length);
#else
	  ourprintf(daemon, &err, "%lu ", (unsigned long)lease->expires);
#endif
	  if (lease->hwaddr_type != ARPHRD_ETHER || lease->hwaddr_len == 0) 
	    ourprintf(daemon, &err, "%.2x-", lease->hwaddr_type);
	  for (i = 0; i < lease->hwaddr_len; i++)
	    {
	      ourprintf(daemon, &err, "%.2x", lease->hwaddr[i]);
	      if (i != lease->hwaddr_len - 1)
		ourprintf(daemon, &err, ":");
	    }
	  ourprintf(daemon, &err, " %s %s ", inet_ntoa(lease->addr),
		    lease->hostname && strlen(lease->hostname) != 0 ? lease->hostname : "*");
	  
	  if (lease->clid && lease->clid_len != 0)
	    {
	      for (i = 0; i < lease->clid_len - 1; i++)
		ourprintf(daemon, &err, "%.2x:", lease->clid[i]);
	      ourprintf(daemon, &err, "%.2x\n", lease->clid[i]);
	    }
	  else
	    ourprintf(daemon, &err, "*\n");	  
	}
      
      if (fflush(daemon->lease_stream) != 0 ||
	  fsync(fileno(daemon->lease_stream)) < 0)
	err = errno;
      
      if (!err)
	file_dirty = 0;
    }
  
  /* Set alarm for when the first lease expires + slop. */
  for (next_event = 0, lease = leases; lease; lease = lease->next)
    if (lease->expires != 0 &&
	(next_event == 0 || difftime(next_event, lease->expires + 10) > 0.0))
      next_event = lease->expires + 10;
   
  if (err)
    {
      if (next_event == 0 || difftime(next_event, LEASE_RETRY + now) > 0.0)
	next_event = LEASE_RETRY + now;
      
      syslog(LOG_ERR, _("failed to write %s: %s (retry in %us)"), 
	     daemon->lease_file, strerror(err),
	     (unsigned int)difftime(next_event, now));
    }

  if (next_event != 0)
    alarm((unsigned)difftime(next_event, now)); 
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
	  file_dirty = 1;
	  if (lease->hostname)
	    dns_dirty = 1;
	  
	  *up = lease->next; /* unlink */
	  
	  /* Put on old_leases list 'till we
	     can run the script */
	  lease->next = old_leases;
	  old_leases = lease;
	  
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


struct dhcp_lease *lease_allocate(struct in_addr addr)
{
  struct dhcp_lease *lease;
  if (!leases_left || !(lease = malloc(sizeof(struct dhcp_lease))))
    return NULL;

  memset(lease, 0, sizeof(struct dhcp_lease));
  lease->new = 1;
  lease->addr = addr;
  lease->hwaddr_len = 225; /* illegal value */
  lease->expires = 1;
#ifdef HAVE_BROKEN_RTC
  lease->length = 0xffffffff; /* illegal value */
#endif
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
      lease->aux_changed = file_dirty = 1;
#endif
    }
  
#ifdef HAVE_BROKEN_RTC
  if (len != lease->length)
    {
      lease->length = len;
      lease->aux_changed = file_dirty = 1; 
    }
#endif
} 

void lease_set_hwaddr(struct dhcp_lease *lease, unsigned char *hwaddr,
		      unsigned char *clid, int hw_len, int hw_type, int clid_len)
{
  if (hw_len != lease->hwaddr_len ||
      hw_type != lease->hwaddr_type || 
      (hw_len != 0 && memcmp(lease->hwaddr, hwaddr, hw_len) != 0))
    {
      memcpy(lease->hwaddr, hwaddr, hw_len);
      lease->hwaddr_len = hw_len;
      lease->hwaddr_type = hw_type;
      lease->changed = file_dirty = 1; /* run script on change */
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
	  lease->aux_changed = file_dirty = 1;
	  if (lease->clid)
	    free(lease->clid);
	  if (!(lease->clid = malloc(clid_len)))
	    return;
	}
      else if (memcmp(lease->clid, clid, clid_len) != 0)
	lease->aux_changed = file_dirty = 1;
	  
      lease->clid_len = clid_len;
      memcpy(lease->clid, clid, clid_len);
    }

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
	    lease_tmp->changed = 1; /* call script on change */
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
  lease->changed = 1; /* run script on change */
}


#ifndef NO_FORK
static pid_t run_script(struct daemon *daemon, char *action, struct dhcp_lease *lease)
{
  if (daemon->lease_change_command)
    {
      char *addr = inet_ntoa(lease->addr);
      char *com = strrchr(daemon->lease_change_command, '/');
      char *p;
      pid_t pid;
      int i;

      /* stringify MAC into dhcp_buff */
      p = daemon->dhcp_buff;
      if (lease->hwaddr_type != ARPHRD_ETHER || lease->hwaddr_len == 0) 
	p += sprintf(p, "%.2x-", lease->hwaddr_type);
      for (i = 0; i < lease->hwaddr_len; i++)
	{
	  p += sprintf(p, "%.2x", lease->hwaddr[i]);
	  if (i != lease->hwaddr_len - 1)
	    p += sprintf(p, ":");
	}
      
      /* and CLID into namebuff */
      p = daemon->namebuff;
      if (lease->clid)
	for (i = 0; i < lease->clid_len; i++)
	  {
	    p += sprintf(p, "%.2x", lease->clid[i]);
	    if (i != lease->clid_len - 1) 
	      p += sprintf(p, ":");
	  }
      
      /* and expiry or length into dhcp_buff2 */
#ifdef HAVE_BROKEN_RTC
      sprintf(daemon->dhcp_buff2, "%u ", lease->length);
#else
      sprintf(daemon->dhcp_buff2, "%lu ", (unsigned long)lease->expires);
#endif

      pid = fork();
      
      if (pid == -1)
	return 0; /* fork error */
      else if (pid != 0)
	return pid;
      
      if (lease->clid && lease->clid_len != 0)
	setenv("DNSMASQ_CLIENT_ID", daemon->namebuff, 1);
      else
	unsetenv("DNSMASQ_CLIENT_ID");

#ifdef HAVE_BROKEN_RTC
      setenv("DNSMASQ_LEASE_LENGTH", daemon->dhcp_buff2, 1);
      unsetenv("DNSMASQ_LEASE_EXPIRES");
#else
      setenv("DNSMASQ_LEASE_EXPIRES", daemon->dhcp_buff2, 1); 
      unsetenv("DNSMASQ_LEASE_LENGTH");
#endif

      execl(daemon->lease_change_command, 
	    com ? com+1 : daemon->lease_change_command,
	    action, daemon->dhcp_buff, addr, lease->hostname, (char*)NULL);
      
      /* log socket should still be open, right? */
      syslog(LOG_ERR, _("failed to execute %s: %m"), 
	     daemon->lease_change_command);
      _exit(0);	
    }

  return 0;
}
#endif

/* deleted leases get transferred to the old_leases list.
   remove them here, after calling the lease change
   script. Also run the lease change script on new leases */
void lease_collect(struct daemon *daemon)
{
  struct dhcp_lease *lease;

  while (old_leases)
    {
      if (daemon->script_pid != 0)
	return; /* busy */
      
      lease = old_leases;
      old_leases = lease->next;

#ifndef NO_FORK
      daemon->script_pid = run_script(daemon, "del", lease);
#endif
            
      if (lease->hostname)
	free(lease->hostname); 
      if (lease->fqdn)
	free(lease->fqdn);
      if (lease->clid)
	free(lease->clid);
      free(lease);
    }

  for (lease = leases; lease; lease = lease->next)
    if (lease->new || lease->changed || 
	(lease->aux_changed && (daemon->options & OPT_LEASE_RO)))
      {
	if (daemon->script_pid != 0)
	  return; /* busy */

#ifndef NO_FORK
	daemon->script_pid = run_script(daemon, lease->new ? "add" : "old", lease);
#endif

	lease->new = lease->changed = lease->aux_changed = 0;
      }
}
	  

      

