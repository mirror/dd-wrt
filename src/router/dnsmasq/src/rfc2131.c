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

#define BOOTREQUEST              1
#define BOOTREPLY                2
#define DHCP_COOKIE              0x63825363

/* The Linux in-kernel DHCP client silently ignores any packet 
   smaller than this. Sigh...........   */
#define MIN_PACKETSZ             300

#define OPTION_PAD               0
#define OPTION_NETMASK           1
#define OPTION_ROUTER            3
#define OPTION_DNSSERVER         6
#define OPTION_HOSTNAME          12
#define OPTION_DOMAINNAME        15
#define OPTION_BROADCAST         28
#define OPTION_VENDOR_CLASS_OPT  43
#define OPTION_REQUESTED_IP      50 
#define OPTION_LEASE_TIME        51
#define OPTION_OVERLOAD          52
#define OPTION_MESSAGE_TYPE      53
#define OPTION_SERVER_IDENTIFIER 54
#define OPTION_REQUESTED_OPTIONS 55
#define OPTION_MESSAGE           56
#define OPTION_MAXMESSAGE        57
#define OPTION_T1                58
#define OPTION_T2                59
#define OPTION_VENDOR_ID         60
#define OPTION_CLIENT_ID         61
#define OPTION_USER_CLASS        77
#define OPTION_CLIENT_FQDN       81
#define OPTION_AGENT_ID          82
#define OPTION_SUBNET_SELECT     118
#define OPTION_END               255

#define SUBOPT_SUBNET_SELECT     5     /* RFC 3527 */

#define DHCPDISCOVER             1
#define DHCPOFFER                2
#define DHCPREQUEST              3
#define DHCPDECLINE              4
#define DHCPACK                  5
#define DHCPNAK                  6
#define DHCPRELEASE              7
#define DHCPINFORM               8

#define have_config(config, mask) ((config) && ((config)->flags & (mask))) 
#define option_len(opt) ((int)(((unsigned char *)(opt))[1]))
#define option_ptr(opt) ((void *)&(((unsigned char *)(opt))[2]))

static unsigned int calc_time(struct dhcp_context *context, struct dhcp_config *config, 
			      struct dhcp_lease *lease, unsigned char *opt, time_t now);
static unsigned char *option_put(unsigned char *p, unsigned char *end, int opt, int len, unsigned int val);
static unsigned char *option_end(unsigned char *p, unsigned char *end, 
				 unsigned char *agent_id, struct dhcp_packet *start);
static unsigned char *option_put_string(unsigned char *p, unsigned char *end, 
					int opt, char *string, int null_term);
static void bootp_option_put(struct dhcp_packet *mess, 
			     struct dhcp_boot *boot_opts, struct dhcp_netid *netids);
static struct in_addr option_addr(unsigned char *opt);
static unsigned int option_uint(unsigned char *opt, int size);
static void log_packet(struct daemon *daemon, char *type, struct in_addr *addr, 
		       struct dhcp_packet *mess, char *interface, char *string);
static unsigned char *option_find(struct dhcp_packet *mess, size_t size, int opt_type, int minsize);
static unsigned char *option_find1(unsigned char *p, unsigned char *end, int opt, int minsize);
static unsigned char *do_req_options(struct dhcp_context *context,
				     unsigned char *p, unsigned char *end, 
				     unsigned char *req_options, 
				     struct daemon *daemon,
				     char *hostname,
				     struct dhcp_netid *netid,
				     struct in_addr subnet_addr,
				     unsigned char fqdn_flags,
				     int null_term);


size_t dhcp_reply(struct daemon *daemon, struct dhcp_context *context, char *iface_name, 
	       size_t sz, time_t now, int unicast_dest)
{
  unsigned char *opt, *clid = NULL;
  struct dhcp_lease *ltmp, *lease = NULL;
  struct dhcp_vendor *vendor;
  struct dhcp_mac *mac;
  struct dhcp_netid_list *id_list;
  int clid_len = 0, ignore = 0;
  struct dhcp_packet *mess = daemon->dhcp_packet.iov_base;
  unsigned char *p, *end = (unsigned char *)(mess + 1);
  char *hostname = NULL, *offer_hostname = NULL, *client_hostname = NULL;
  int hostname_auth = 0, borken_opt = 0;
  unsigned char *req_options = NULL;
  char *message = NULL;
  unsigned int time;
  struct dhcp_config *config;
  struct dhcp_netid *netid = NULL;
  struct in_addr subnet_addr, fallback;
  unsigned short fuzz = 0;
  unsigned int mess_type = 0;
  unsigned char fqdn_flags = 0;
  unsigned char *agent_id = NULL;

  subnet_addr.s_addr = 0;
  
  if (mess->op != BOOTREQUEST || mess->hlen > DHCP_CHADDR_MAX)
    return 0;
   
  if (mess->htype == 0 && mess->hlen != 0)
    return 0;

  /* check for DHCP rather than BOOTP */
  if ((opt = option_find(mess, sz, OPTION_MESSAGE_TYPE, 1)))
    {
      mess_type = option_uint(opt, 1);

      /* only insist on a cookie for DHCP. */
      if (*((u32 *)&mess->options) != htonl(DHCP_COOKIE))
	return 0;

      /* two things to note here: expand_buf may move the packet,
	 so reassign mess from daemon->packet. Also, the size
	 sent includes the IP and UDP headers, hence the magic "-28" */
      if ((opt = option_find(mess, sz, OPTION_MAXMESSAGE, 2)))
	{
	  size_t size = (size_t)option_uint(opt, 2) - 28;
	  
	  if (size > DHCP_PACKET_MAX)
	    size = DHCP_PACKET_MAX;
	  else if (size < sizeof(struct dhcp_packet))
	    size = sizeof(struct dhcp_packet);
	  
	  if (expand_buf(&daemon->dhcp_packet, size))
	    {
	      mess = daemon->dhcp_packet.iov_base;
	      end = ((unsigned char *)mess) + size;
	    }
	}

      /* Some buggy clients set ciaddr when they shouldn't, so clear that here since
	 it can affect the context-determination code. */
      if ((option_find(mess, sz, OPTION_REQUESTED_IP, INADDRSZ) || mess_type == DHCPDISCOVER))
	mess->ciaddr.s_addr = 0;

      if ((opt = option_find(mess, sz, OPTION_AGENT_ID, 1)))
	{
	  /* Any agent-id needs to be copied back out, verbatim, as the last option
	     in the packet. Here, we shift it to the very end of the buffer, if it doesn't
	     get overwritten, then it will be shuffled back at the end of processing.
	     Note that the incoming options must not be overwritten here, so there has to 
	     be enough free space at the end of the packet to copy the option. */
	  unsigned int total = option_len(opt) + 2;
	  unsigned char *last_opt = option_find(mess, sz, OPTION_END, 0);
	  if (last_opt && last_opt < end - total)
	    {
	      agent_id = end - total;
	      memcpy(agent_id, opt, total);
	    }

	  /* look for RFC3527 Link selection sub-option */
	  if ((opt = option_find1(option_ptr(opt), option_ptr(opt) + option_len(opt), SUBOPT_SUBNET_SELECT, INADDRSZ)))
	    subnet_addr = option_addr(opt);
	}
      
      /* Check for RFC3011 subnet selector - only if RFC3527 one not present */
      if (subnet_addr.s_addr == 0 && (opt = option_find(mess, sz, OPTION_SUBNET_SELECT, INADDRSZ)))
	subnet_addr = option_addr(opt);
      
      /* If there is no client identifier option, use the hardware address */
      if ((opt = option_find(mess, sz, OPTION_CLIENT_ID, 1)))
	{
	  clid_len = option_len(opt);
	  clid = option_ptr(opt);
	}
      
      /* do we have a lease in store? */
      lease = lease_find_by_client(mess->chaddr, mess->hlen, mess->htype, clid, clid_len);

      /* If this request is missing a clid, but we've seen one before, 
	 use it again for option matching etc. */
      if (lease && !clid && lease->clid)
	{
	  clid_len = lease->clid_len;
	  clid = lease->clid;
	}
    }
  
  for (mac = daemon->dhcp_macs; mac; mac = mac->next)
    if (mac->hwaddr_len == mess->hlen &&
	(mac->hwaddr_type == mess->htype || mac->hwaddr_type == 0) &&
	memcmp_masked(mac->hwaddr, mess->chaddr, mess->hlen, mac->mask))
      {
	mac->netid.next = netid;
	netid = &mac->netid;
      }
  
  /* Determine network for this packet. Our caller will have already linked all the 
     contexts which match the addresses of the receiving interface but if the 
     machine has an address already, or came via a relay, or we have a subnet selector, 
     we search again. If we don't have have a giaddr or explicit subnet selector, 
     use the ciaddr. This is necessary because a  machine which got a lease via a 
     relay won't use the relay to renew. If matching a ciaddr fails but we have a context 
     from the physical network, continue using that to allow correct DHCPNAK generation later. */
  if (mess->giaddr.s_addr || subnet_addr.s_addr || mess->ciaddr.s_addr)
    {
      struct dhcp_context *context_tmp, *context_new = NULL;
      struct in_addr addr = mess->ciaddr;
      int force = 0;
      
      if (subnet_addr.s_addr)
	{
	  addr = subnet_addr;
	  force = 1;
	}
      else if (mess->giaddr.s_addr)
	{
	  addr = mess->giaddr;
	  force = 1;
	}
      
      for (context_tmp = daemon->dhcp; context_tmp; context_tmp = context_tmp->next)
	if (context_tmp->netmask.s_addr  && 
	    is_same_net(addr, context_tmp->start, context_tmp->netmask) &&
	    is_same_net(addr, context_tmp->end, context_tmp->netmask))
	  {
	    context_tmp->current = context_new;
	    context_new = context_tmp;
	  }

      if (context_new || force)
	context = context_new;

    }
  
  if (!context)
    {
      syslog(LOG_WARNING, _("no address range available for DHCP request %s %s"), 
             subnet_addr.s_addr ? _("with subnet selector") : _("via"),
             subnet_addr.s_addr ? inet_ntoa(subnet_addr) : (mess->giaddr.s_addr ? inet_ntoa(mess->giaddr) : iface_name));
      return 0;
    }
  
  /* keep _a_ local address available. */
  fallback = context->local;
  
  mess->op = BOOTREPLY;
  p = mess->options + sizeof(u32); /* skip cookie */
  
  config = find_config(daemon->dhcp_conf, context, clid, clid_len, 
		       mess->chaddr, mess->hlen, mess->htype, NULL);
  
  if (mess_type == 0)
    {
      /* BOOTP request */
      struct dhcp_netid id;
      char save = mess->file[128];
      struct in_addr *logaddr = NULL;

      /* must have a MAC addr for bootp */
      if (mess->htype == 0 || mess->hlen == 0)
	return 0;
      
      if (have_config(config, CONFIG_DISABLE))
	message = _("disabled");

      end = mess->options + 64; /* BOOTP vend area is only 64 bytes */
            
      if (have_config(config, CONFIG_NAME))
	hostname = config->hostname;
      
      if (have_config(config, CONFIG_NETID))
	{
	  config->netid.next = netid;
	  netid = &config->netid;
	}

      /* Match incoming filename field as a netid. */
      if (mess->file[0])
	{
	  mess->file[128] = 0; /* ensure zero term. */
	  id.net = (char *)mess->file;
	  id.next = netid;
	  netid = &id;
	}
      
      for (id_list = daemon->dhcp_ignore; id_list; id_list = id_list->next)
	if (match_netid(id_list->list, netid, 0))
	  message = _("disabled");
      
      if (!message)
	{
	  if (have_config(config, CONFIG_ADDR))
	    {
	      logaddr = &config->addr;
	      mess->yiaddr = config->addr;
	      if ((lease = lease_find_by_addr(config->addr)) &&
		  (lease->hwaddr_len != mess->hlen ||
		   lease->hwaddr_type != mess->htype ||
		   memcmp(lease->hwaddr, mess->chaddr, lease->hwaddr_len) != 0))
		message = _("address in use");
	    }
	  else if (!(daemon->options & OPT_BOOTP_DYNAMIC))
	    message = _("no address configured");
	  else
	    {
	      if (!(lease = lease_find_by_client(mess->chaddr, mess->hlen, mess->htype, NULL, 0)) ||
		  !address_available(context, lease->addr))
		{
		   if (lease)
		     {
		       /* lease exists, wrong network. */
		       lease_prune(lease, now);
		       lease = NULL;
		     }
		   if (!address_allocate(context, daemon, &mess->yiaddr, mess->chaddr, mess->hlen, netid, now))
		     message = _("no address available");
		}
	      else
		mess->yiaddr = lease->addr;
	    }
	  
	  if (!message && 
	      !lease && 
	      (!(lease = lease_allocate(mess->yiaddr))))
	    message = _("no leases left");
	  
	  if (!message && !(context = narrow_context(context, mess->yiaddr)))
	    message = _("wrong network");
	      
	  if (!message)
	    {
	      logaddr = &mess->yiaddr;
		
	      if (context->netid.net)
		{
		  context->netid.next = netid;
		  netid = &context->netid;
		}	 
	      
	      lease_set_hwaddr(lease, mess->chaddr, NULL, mess->hlen, mess->htype, 0);
	      if (hostname)
		lease_set_hostname(lease, hostname, daemon->domain_suffix, 1); 
	      lease_set_expires(lease, 0xffffffff, now); /* infinite lease */
	      
	      p = do_req_options(context, p, end, NULL, daemon, 
				 hostname, netid, subnet_addr, fqdn_flags, 0);
	      /* must do this after do_req_options since it overwrites filename field. */
	      mess->siaddr = context->local;
	      bootp_option_put(mess, daemon->boot_config, netid);
	      p = option_end(p, end, NULL, mess);
	    }
	}
      
      log_packet(daemon, NULL, logaddr, mess, iface_name, message);
      mess->file[128] = save;

      if (message)
	return 0;
      else
	return p - (unsigned char *)mess; 
    }
  
  if ((opt = option_find(mess, sz, OPTION_CLIENT_FQDN, 4)))
    {
      /* http://tools.ietf.org/wg/dhc/draft-ietf-dhc-fqdn-option/draft-ietf-dhc-fqdn-option-10.txt */
      int len = option_len(opt);
      char *pq = daemon->dhcp_buff;
      unsigned char *pp, *op = option_ptr(opt);
      
      fqdn_flags = *op;
      len -= 3;
      op += 3;
      pp = op;
      
      /* Always force update, since the client has no way to do it itself. */
      if (!(fqdn_flags & 0x01))
	fqdn_flags |= 0x02;
      
      fqdn_flags &= ~0x08;
      fqdn_flags |= 0x01;
      
      if (fqdn_flags & 0x04)
	while (*op != 0 && ((op + (*op) + 1) - pp) < len)
	  {
	    memcpy(pq, op+1, *op);
	    pq += *op;
	    op += (*op)+1;
	    *(pq++) = '.';
	  }
      else
	{
	  memcpy(pq, op, len);
	  if (len > 0 && op[len-1] == 0)
	    borken_opt = 1;
	  pq += len + 1;
	}
      
      if (pq != daemon->dhcp_buff)
	pq--;
      
      *pq = 0;
      
      if (canonicalise(daemon->dhcp_buff))
	offer_hostname = client_hostname = daemon->dhcp_buff;
    }
  else if ((opt = option_find(mess, sz, OPTION_HOSTNAME, 1)))
    {
      int len = option_len(opt);
      memcpy(daemon->dhcp_buff, option_ptr(opt), len);
      /* Microsoft clients are broken, and need zero-terminated strings
	 in options. We detect this state here, and do the same in
	 any options we send */
      if (len > 0 && daemon->dhcp_buff[len-1] == 0)
	borken_opt = 1;
      else
	daemon->dhcp_buff[len] = 0;
      if (canonicalise(daemon->dhcp_buff))
	client_hostname = daemon->dhcp_buff;
    }

  if (have_config(config, CONFIG_NAME))
    {
      hostname = config->hostname;
      hostname_auth = 1;
      /* be careful not to send an OFFER with a hostname not 
	 matching the DISCOVER. */
      if (fqdn_flags != 0 || !client_hostname || hostname_isequal(hostname, client_hostname))
	offer_hostname = hostname;
    }
  else if (client_hostname && (hostname = strip_hostname(daemon, client_hostname)) && !config)
    {
      /* Search again now we have a hostname. 
	 Only accept configs without CLID and HWADDR here, (they won't match)
	 to avoid impersonation by name. */
      struct dhcp_config *new = find_config(daemon->dhcp_conf, context, NULL, 0,
					    mess->chaddr, mess->hlen, 
					    mess->htype, hostname);
      if (!have_config(new, CONFIG_CLID) && !have_config(new, CONFIG_HWADDR))
	config = new;
    }
  
  if (have_config(config, CONFIG_NETID))
    {
      config->netid.next = netid;
      netid = &config->netid;
    }
  
  /* user-class options are, according to RFC3004, supposed to contain
     a set of counted strings. Here we check that this is so (by seeing
     if the counts are consistent with the overall option length) and if
     so zero the counts so that we don't get spurious matches between 
     the vendor string and the counts. If the lengths don't add up, we
     assume that the option is a single string and non RFC3004 compliant 
     and just do the substring match. dhclient provides these broken options. */

  if ((opt = option_find(mess, sz, OPTION_USER_CLASS, 1)))
    {
      unsigned char *ucp = option_ptr(opt);
      int tmp, j;
      for (j = 0; j < option_len(opt); j += ucp[j] + 1);
      if (j == option_len(opt))
	for (j = 0; j < option_len(opt); j = tmp)
	  {
	    tmp = j + ucp[j] + 1;
	    ucp[j] = 0;
	  }
    }
  
  for (vendor = daemon->dhcp_vendors; vendor; vendor = vendor->next)
    if ((opt = option_find(mess, sz, vendor->is_vendor ? OPTION_VENDOR_ID : OPTION_USER_CLASS, 1)))
      {
	int i;
	for (i = 0; i <= (option_len(opt) - vendor->len); i++)
	  if (memcmp(vendor->data, option_ptr(opt)+i, vendor->len) == 0)
	    {
	      vendor->netid.next = netid;
	      netid = &vendor->netid;
	      break;
	    }
      }
  
  /* if all the netids in the ignore list are present, ignore this client */
  for (id_list = daemon->dhcp_ignore; id_list; id_list = id_list->next)
    if (match_netid(id_list->list, netid, 0))
      ignore = 1;
  
  /* Can have setting to ignore the client ID for a particular MAC address or hostname */
  if (have_config(config, CONFIG_NOCLID))
    clid = NULL;
          
  if ((opt = option_find(mess, sz, OPTION_REQUESTED_OPTIONS, 0)))
    {
      req_options = (unsigned char *)daemon->dhcp_buff2;
      memcpy(req_options, option_ptr(opt), option_len(opt));
      req_options[option_len(opt)] = OPTION_END;
    }
  
  switch (mess_type)
    {
    case DHCPDECLINE:
      if (!(opt = option_find(mess, sz, OPTION_SERVER_IDENTIFIER, INADDRSZ)) ||
	  (context->local.s_addr != option_addr(opt).s_addr))
	return 0;

      /* sanitise any message. Paranoid? Moi? */
      if ((opt = option_find(mess, sz, OPTION_MESSAGE, 1)))
	{ 
	  char *p = option_ptr(opt), *q = daemon->dhcp_buff;
	  int i;
	  
	  for (i = option_len(opt); i > 0; i--)
	    {
	      char c = *p++;
	      if (isprint(c))
		*q++ = c;
	    }
	  *q++ = 0; /* add terminator */
	}
      
      if (!(opt = option_find(mess, sz, OPTION_REQUESTED_IP, INADDRSZ)))
	return 0;
      
      log_packet(daemon, "DECLINE", option_ptr(opt), mess, iface_name, daemon->dhcp_buff);
      
      if (lease && lease->addr.s_addr == option_addr(opt).s_addr)
	lease_prune(lease, now);
      
      if (have_config(config, CONFIG_ADDR) && 
	  config->addr.s_addr == option_addr(opt).s_addr)
	{
	  prettyprint_time(daemon->dhcp_buff, DECLINE_BACKOFF);
	  syslog(LOG_WARNING, _("disabling DHCP static address %s for %s"), 
		 inet_ntoa(config->addr), daemon->dhcp_buff);
	  config->flags |= CONFIG_DECLINED;
	  config->decline_time = now;
	}
      else
	/* make sure this host gets a different address next time. */
	for (; context; context = context->current)
	  context->addr_epoch++;
      
      return 0;

    case DHCPRELEASE:
      if (!(opt = option_find(mess, sz, OPTION_SERVER_IDENTIFIER, INADDRSZ)) ||
	  (context->local.s_addr != option_addr(opt).s_addr))
	return 0;
      
      if (lease && lease->addr.s_addr == mess->ciaddr.s_addr)
	lease_prune(lease, now);
      else
	message = _("unknown lease");

      log_packet(daemon, "RELEASE", &mess->ciaddr, mess, iface_name, message);
	
      return 0;
      
    case DHCPDISCOVER:
      if (ignore || have_config(config, CONFIG_DISABLE))
	{
	  message = _("ignored");
	  opt = NULL;
	}
      else 
	{
	  struct in_addr addr, conf;
	  
	  if ((opt = option_find(mess, sz, OPTION_REQUESTED_IP, INADDRSZ)))	 
	    addr = option_addr(opt);
	  
	  conf.s_addr = 0;
	  if (have_config(config, CONFIG_ADDR))
	    {
	      char *addrs = inet_ntoa(config->addr);
	      
	      if ((ltmp = lease_find_by_addr(config->addr)) && ltmp != lease)
		syslog(LOG_WARNING, _("not using configured address %s because it is leased to %s"),
		       addrs, print_mac(daemon, ltmp->hwaddr, ltmp->hwaddr_len));
	      else
		{
		  struct dhcp_context *tmp;
		  for (tmp = context; tmp; tmp = tmp->current)
		    if (context->router.s_addr == config->addr.s_addr)
		      break;
		  if (tmp)
		    syslog(LOG_WARNING, _("not using configured address %s because it is in use by the server or relay"), addrs);
		  else if (have_config(config, CONFIG_DECLINED) &&
			   difftime(now, config->decline_time) < (float)DECLINE_BACKOFF)
		    syslog(LOG_WARNING, _("not using configured address %s because it was previously declined"), addrs);
		  else
		    conf = config->addr;
		}
	    }
	  
	  if (conf.s_addr)
	    mess->yiaddr = conf;
	  else if (lease && address_available(context, lease->addr))
	    mess->yiaddr = lease->addr;
	  else if (opt && address_available(context, addr) && !lease_find_by_addr(addr) && 
		   !config_find_by_address(daemon->dhcp_conf, addr))
	    mess->yiaddr = addr;
	  else if (!address_allocate(context, daemon, &mess->yiaddr, mess->chaddr, mess->hlen, netid, now))
	    message = _("no address available");      
	}
      
      log_packet(daemon, "DISCOVER", opt ? (struct in_addr *)option_ptr(opt) : NULL, mess, iface_name, message); 

      if (message || !(context = narrow_context(context, mess->yiaddr)))
	return 0;

      if (context->netid.net)
	{
	  context->netid.next = netid;
	  netid = &context->netid;
	}
       
      time = calc_time(context, config, lease, option_find(mess, sz, OPTION_LEASE_TIME, 4), now);
      mess->siaddr = context->local;
      bootp_option_put(mess, daemon->boot_config, netid);
      p = option_put(p, end, OPTION_MESSAGE_TYPE, 1, DHCPOFFER);
      p = option_put(p, end, OPTION_SERVER_IDENTIFIER, INADDRSZ, ntohl(context->local.s_addr));
      p = option_put(p, end, OPTION_LEASE_TIME, 4, time);
      /* T1 and T2 are required in DHCPOFFER by HP's wacky Jetdirect client. */
      if (time != 0xffffffff)
	{
	  p = option_put(p, end, OPTION_T1, 4, (time/2));
	  p = option_put(p, end, OPTION_T2, 4, (time*7)/8);
	}
      p = do_req_options(context, p, end, req_options, daemon, 
			 offer_hostname, netid, subnet_addr, fqdn_flags, borken_opt);
      p = option_end(p, end, agent_id, mess);
      
      log_packet(daemon, "OFFER" , &mess->yiaddr, mess, iface_name, NULL);
      return p - (unsigned char *)mess;
      
    case DHCPREQUEST:
      if (ignore || have_config(config, CONFIG_DISABLE))
	return 0;
      if ((opt = option_find(mess, sz, OPTION_REQUESTED_IP, INADDRSZ)))
	{
	  /* SELECTING  or INIT_REBOOT */
	  mess->yiaddr = option_addr(opt);
	  
	  if ((opt = option_find(mess, sz, OPTION_SERVER_IDENTIFIER, INADDRSZ)))
	    {
	      /* SELECTING */
	      for (; context; context = context->current)
		if (context->local.s_addr == option_addr(opt).s_addr)
		  break;

	      if (!context)
		return 0;
	      
	      /* If a lease exists for this host and another address, squash it. */
	      if (lease && lease->addr.s_addr != mess->yiaddr.s_addr)
		{
		  lease_prune(lease, now);
		  lease = NULL;
		}
	    }
	  else
	    {
	      /* INIT-REBOOT */
	      if (!lease && !(daemon->options & OPT_AUTHORITATIVE))
		return 0;
	      
	      if (lease && lease->addr.s_addr != mess->yiaddr.s_addr)
		message = _("wrong address");
	    }
	}
      else
	{
	  /* RENEWING or REBINDING */ 
	  /* Check existing lease for this address.
	     We allow it to be missing if dhcp-authoritative mode
	     as long as we can allocate the lease now - checked below.
	     This makes for a smooth recovery from a lost lease DB */
	  if ((lease && mess->ciaddr.s_addr != lease->addr.s_addr) ||
	      (!lease && !(daemon->options & OPT_AUTHORITATIVE)))
	    {
	      message = _("lease not found");
	      /* ensure we broadcast NAK */
	      unicast_dest = 0;
	    }
	  /* desynchronise renewals */
	  fuzz = rand16();
	  mess->yiaddr = mess->ciaddr;
	}
      
      log_packet(daemon, "REQUEST", &mess->yiaddr, mess, iface_name, NULL);
 
      if (!message)
	{
	  struct dhcp_config *addr_config;
	  struct dhcp_context *tmp = NULL;
	  
	  if (have_config(config, CONFIG_ADDR))
	    for (tmp = context; tmp; tmp = tmp->current)
	      if (context->router.s_addr == config->addr.s_addr)
		break;
	  
	  if (!(context = narrow_context(context, mess->yiaddr)))
	    {
	      /* If a machine moves networks whilst it has a lease, we catch that here. */
	      message = _("wrong network");
	      /* ensure we broadcast NAK */
	      unicast_dest = 0;
	    }
	  
	  /* Check for renewal of a lease which is outside the allowed range. */
	  else if (!address_available(context, mess->yiaddr) &&
		   (!have_config(config, CONFIG_ADDR) || config->addr.s_addr != mess->yiaddr.s_addr))
	    message = _("address not available");
	  
	  /* Check if a new static address has been configured. Be very sure that
	     when the client does DISCOVER, it will get the static address, otherwise
	     an endless protocol loop will ensue. */
	  else if (!tmp &&
		   have_config(config, CONFIG_ADDR) && 
		   (!have_config(config, CONFIG_DECLINED) ||
		    difftime(now, config->decline_time) > (float)DECLINE_BACKOFF) &&
		   config->addr.s_addr != mess->yiaddr.s_addr &&
		   (!(ltmp = lease_find_by_addr(config->addr)) || ltmp == lease))
	    message = _("static lease available");

	  /* Check to see if the address is reserved as a static address for another host */
	  else if ((addr_config = config_find_by_address(daemon->dhcp_conf, mess->yiaddr)) && addr_config != config)
	    message = _("address reserved");

	  else if ((ltmp = lease_find_by_addr(mess->yiaddr)) && ltmp != lease)
	    message = _("address in use");
	  
	  else if (!clid && mess->hlen == 0)
	    message = _("no unique-id");
	  
	  else if (!lease && 
		   !(lease = lease_allocate(mess->yiaddr)))
	    message = _("no leases left");
	}
      
      if (message)
	{
	  log_packet(daemon, "NAK", &mess->yiaddr, mess, iface_name, message);
	  
	  mess->siaddr.s_addr = mess->yiaddr.s_addr = 0;
	  bootp_option_put(mess, NULL, NULL);
	  p = option_put(p, end, OPTION_MESSAGE_TYPE, 1, DHCPNAK);
	  p = option_put(p, end, OPTION_SERVER_IDENTIFIER, INADDRSZ, 
			 ntohl(context ? context->local.s_addr : fallback.s_addr));
	  p = option_put_string(p, end, OPTION_MESSAGE, message, borken_opt);
	  /* This fixes a problem with the DHCP spec, broadcasting a NAK to a host on 
	     a distant subnet which unicast a REQ to us won't work. */
	  if (!unicast_dest || mess->giaddr.s_addr != 0 || 
	      mess->ciaddr.s_addr == 0 || is_same_net(context->local, mess->ciaddr, context->netmask))
	    {
	      mess->flags |= htons(0x8000); /* broadcast */
	      mess->ciaddr.s_addr = 0;
	    }
	}
      else
	{
	  if (!hostname_auth && (client_hostname = host_from_dns(daemon, mess->yiaddr)))
	    {
	      hostname = client_hostname;
	      hostname_auth = 1;
	    }

	  log_packet(daemon, "ACK", &mess->yiaddr, mess, iface_name, hostname);
      
	  if (context->netid.net)
	    {
	      context->netid.next = netid;
	      netid = &context->netid;
	    }
	
	  time = calc_time(context, config, NULL, option_find(mess, sz, OPTION_LEASE_TIME, 4), now);
	  lease_set_hwaddr(lease, mess->chaddr, clid, mess->hlen, mess->htype, clid_len);
	  if (hostname)
	    lease_set_hostname(lease, hostname, daemon->domain_suffix, hostname_auth);
	  lease_set_expires(lease, time, now);
	  	  
	  mess->siaddr = context->local;
	  bootp_option_put(mess, daemon->boot_config, netid);
	  p = option_put(p, end, OPTION_MESSAGE_TYPE, 1, DHCPACK);
	  p = option_put(p, end, OPTION_SERVER_IDENTIFIER, INADDRSZ, ntohl(context->local.s_addr));
	  p = option_put(p, end, OPTION_LEASE_TIME, 4, time);
	  if (time != 0xffffffff)
	    {
	      while (fuzz > (time/16))
		fuzz = fuzz/2; 
	      p = option_put(p, end, OPTION_T1, 4, (time/2) - fuzz);
	      p = option_put(p, end, OPTION_T2, 4, ((time/8)*7) - fuzz);
	    }
	  p = do_req_options(context, p, end, req_options, daemon, 
			     hostname, netid, subnet_addr, fqdn_flags, borken_opt);
	}

      p = option_end(p, end, agent_id, mess);
      return p - (unsigned char *)mess; 
      
    case DHCPINFORM:
      if (ignore || have_config(config, CONFIG_DISABLE))
	message = _("ignored");
      
      log_packet(daemon, "INFORM", &mess->ciaddr, mess, iface_name, message);
     
      if (message || mess->ciaddr.s_addr == 0 || 
	  !(context = narrow_context(context, mess->ciaddr)))
	return 0;
      
      if (context->netid.net)
	{
	  context->netid.next = netid;
	  netid = &context->netid;
	}
       
      mess->siaddr = context->local;
      bootp_option_put(mess, daemon->boot_config, netid);
      p = option_put(p, end, OPTION_MESSAGE_TYPE, 1, DHCPACK);
      p = option_put(p, end, OPTION_SERVER_IDENTIFIER, INADDRSZ, ntohl(context->local.s_addr));
      if (!hostname)
	hostname = host_from_dns(daemon, mess->yiaddr);
      p = do_req_options(context, p, end, req_options, daemon, 
			 hostname, netid, subnet_addr, fqdn_flags, borken_opt);
      p = option_end(p, end, agent_id, mess);
      
      log_packet(daemon, "ACK", &mess->ciaddr, mess, iface_name, hostname);
      return p - (unsigned char *)mess; 
    }
  
  return 0;
}

static unsigned int calc_time(struct dhcp_context *context, struct dhcp_config *config, 
			      struct dhcp_lease *lease, unsigned char *opt, time_t now)
{
  unsigned int time = have_config(config, CONFIG_TIME) ? config->lease_time : context->lease_time;
  
  if (opt)
    { 
      unsigned int req_time = option_uint(opt, 4);
      if (req_time < 120 )
	req_time = 120; /* sanity */
      if (time == 0xffffffff || (req_time != 0xffffffff && req_time < time))
	time = req_time;
    }
  else if (lease && lease->expires != 0 && difftime(lease->expires, now) > 0.0)
    {
      unsigned int lease_time = (unsigned int)difftime(lease->expires, now);
      
      /* put a floor on lease-remaining time. */
      if (lease_time < 360 )
	lease_time = 360;
      
      if (time > lease_time)
	time = lease_time;
    }

  return time;
}

static void log_packet(struct daemon *daemon, char *type, struct in_addr *addr, 
		       struct dhcp_packet *mess, char *interface, char *string)
{
  syslog(LOG_INFO, "%s%s(%s) %s%s%s %s",
	 type ? "DHCP" : "BOOTP",
	 type ? type : "",
	 interface, 
	 addr ? inet_ntoa(*addr) : "",
	 addr ? " " : "",
	 print_mac(daemon, mess->chaddr, mess->hlen),
	 string ? string : "");
}

static struct in_addr option_addr(unsigned char *opt)
{
  /* this worries about unaligned data in the option. */
  /* struct in_addr is network byte order */
  struct in_addr ret;

  memcpy(&ret, option_ptr(opt), INADDRSZ);

  return ret;
}

static unsigned int option_uint(unsigned char *opt, int size)
{
  /* this worries about unaligned data and byte order */
  unsigned int ret = 0;
  int i;
  unsigned char *p = option_ptr(opt);
  
  for (i = 0; i < size; i++)
    ret = (ret << 8) | *p++;

  return ret;
}

static void bootp_option_put(struct dhcp_packet *mess, 
			     struct dhcp_boot *boot_opts, struct dhcp_netid *netids)
{
  struct dhcp_boot *tmp;
  
  for (tmp = boot_opts; tmp; tmp = tmp->next)
    if (match_netid(tmp->netid, netids, 0))
      break;
  if (!tmp)
    /* No match, look for one without a netid */
    for (tmp = boot_opts; tmp; tmp = tmp->next)
      if (match_netid(tmp->netid, netids, 1))
	break;

  /* Do this _after_ the matching above, since in 
     BOOTP mode, one if the things we match is the filename. */
  
  memset(mess->sname, 0, sizeof(mess->sname));
  memset(mess->file, 0, sizeof(mess->file));
  
  if (tmp)
    {
      if (tmp->sname)
	strncpy((char *)mess->sname, tmp->sname, sizeof(mess->sname)-1);
      if (tmp->file)
	strncpy((char *)mess->file, tmp->file, sizeof(mess->file)-1);
      if (tmp->next_server.s_addr)
	mess->siaddr = tmp->next_server;
    }
}

static int check_space(unsigned char *p, unsigned char *end, int len, int opt)
{
  /* always keep one octet space for the END option. */ 
  if (p + len + 3 >= end)
    {
      syslog(LOG_WARNING, _("cannot send DHCP option %d: no space left in packet"), opt);
      return 0;
    }

  return 1;
}

   
static unsigned char *option_put(unsigned char *p, unsigned char *end, int opt, int len, unsigned int val)
{
  int i;
  
  if (check_space(p, end, len, opt)) 
    {
      *(p++) = opt;
      *(p++) = len;
      
      for (i = 0; i < len; i++)
	*(p++) = val >> (8 * (len - (i + 1)));
    }
  return p;
}

static unsigned char *option_end(unsigned char *p, unsigned char *end, 
				 unsigned char *agent_id, struct dhcp_packet *start)
{
  /* shuffle agent-id back down if we have room for it */
  if (agent_id && p < agent_id)
    {
      memmove(p, agent_id, end - agent_id);
      p += end - agent_id;
    }

  *(p++) = OPTION_END;
  while ((p < end) && (p - ((unsigned char *)start) < MIN_PACKETSZ))
    *p++ = 0;
  
  return p;
}

static unsigned char *option_put_string(unsigned char *p, unsigned char *end, int opt, 
					char *string, int null_term)
{
  size_t len = strlen(string);

  if (null_term && len != 255)
    len++;

  if (check_space(p, end, len, opt))
    {
      *(p++) = opt;
      *(p++) = len;
      memcpy(p, string, len);
      p += len;
    }

  return p;
}
 
static unsigned char *option_find1(unsigned char *p, unsigned char *end, int opt, int minsize)
{
  while (*p != OPTION_END) 
    {
      if (p >= end)
	return NULL; /* malformed packet */
      else if (*p == OPTION_PAD)
	p++;
      else 
	{ 
	  int opt_len;
	  if (p >= end - 2)
	    return NULL; /* malformed packet */
	  opt_len = option_len(p);
	  if (p >= end - (2 + opt_len))
	    return NULL; /* malformed packet */
	  if (*p == opt && opt_len >= minsize)
	    return p;
	  p += opt_len + 2;
	}
    }
  
  return opt == OPTION_END ? p : NULL;
}
 
static unsigned char *option_find(struct dhcp_packet *mess, size_t size, int opt_type, int minsize)
{
  unsigned char *ret, *opt;
  
  /* skip over DHCP cookie; */
  if ((ret = option_find1(&mess->options[0] + sizeof(u32), ((unsigned char *)mess) + size, opt_type, minsize)))
    return ret;

  /* look for overload option. */
  if (!(opt = option_find1(&mess->options[0] + sizeof(u32), ((unsigned char *)mess) + size, OPTION_OVERLOAD, 1)))
    return NULL;
  
  /* Can we look in filename area ? */
  if ((option_uint(opt, 1) & 1) &&
      (ret = option_find1(&mess->file[0], &mess->file[128], opt_type, minsize)))
    return ret;

  /* finally try sname area */
  if ((option_uint(opt, 1) & 2) &&
      (ret = option_find1(&mess->sname[0], &mess->sname[64], opt_type, minsize)))
    return ret;

  return NULL;
}

static int in_list(unsigned char *list, int opt)
{
  int i;
  
  /* If no requested options, send everything, not nothing. */
  if (!list)
    return 1;
  
  for (i = 0; list[i] != OPTION_END; i++)
    if (opt == list[i])
      return 1;

  return 0;
}

static struct dhcp_opt *option_find2(struct dhcp_netid *netid, struct dhcp_opt *opts, int opt)
{
  struct dhcp_opt *tmp;  
  for (tmp = opts; tmp; tmp = tmp->next)
    if (tmp->opt == opt)
      if (match_netid(tmp->netid, netid, 1) || match_netid(tmp->netid, netid, 0))
	return tmp;
	      
  return netid ? option_find2(NULL, opts, opt) : NULL;
}

static unsigned char *do_opt(struct dhcp_opt *opt, unsigned char *p, unsigned char *end,  
			     struct in_addr local, int null_term)
{
  int len = opt->len;

  if ((opt->flags & DHOPT_STRING) && null_term && len != 255)
    len++;

  if (!check_space(p, end, len, opt->opt))
    return p;
    
  *(p++) = opt->opt;
  *(p++) = len;
  
  if (len == 0)
    return p;
      
  if (opt->flags & DHOPT_ADDR)
    {
      int j;
      struct in_addr *a = (struct in_addr *)opt->val;
      for (j = 0; j < opt->len; j+=INADDRSZ, a++)
	{
	  /* zero means "self" (but not in vendorclass options.) */
	  if (a->s_addr == 0)
	    memcpy(p, &local, INADDRSZ);
	  else
	    memcpy(p, a, INADDRSZ);
	  p += INADDRSZ;
	}
    }
  else
    {
      memcpy(p, opt->val, len);
      p += len;
    }

  return p;
}  

static unsigned char *do_req_options(struct dhcp_context *context,
				     unsigned char *p, unsigned char *end, 
				     unsigned char *req_options,
				     struct daemon *daemon,
				     char *hostname,
				     struct dhcp_netid *netid,
				     struct in_addr subnet_addr,
				     unsigned char fqdn_flags,
				     int null_term)
{
  struct dhcp_opt *opt, *config_opts = daemon->dhcp_opts;
  char *vendor_class = NULL;
  
  /* rfc3011 says this doesn't need to be in the requested options list. */
  if (subnet_addr.s_addr)
    p = option_put(p, end, OPTION_SUBNET_SELECT, INADDRSZ, ntohl(subnet_addr.s_addr));

  if (in_list(req_options, OPTION_NETMASK) &&
      !option_find2(netid, config_opts, OPTION_NETMASK))
    p = option_put(p, end, OPTION_NETMASK, INADDRSZ, ntohl(context->netmask.s_addr));
  
  /* May not have a "guessed" broadcast address if we got no packets via a relay
     from this net yet (ie just unicast renewals after a restart */
  if (context->broadcast.s_addr &&
      in_list(req_options, OPTION_BROADCAST) &&
      !option_find2(netid, config_opts, OPTION_BROADCAST))
    p = option_put(p, end, OPTION_BROADCAST, INADDRSZ, ntohl(context->broadcast.s_addr));
  
  /* Same comments as broadcast apply, and also may not be able to get a sensible
     default when using subnet select.  User must configure by steam in that case. */
  if (context->router.s_addr &&
      in_list(req_options, OPTION_ROUTER) &&
      !option_find2(netid, config_opts, OPTION_ROUTER))
    p = option_put(p, end, OPTION_ROUTER, INADDRSZ, ntohl(context->router.s_addr));

  if (in_list(req_options, OPTION_DNSSERVER) &&
      !option_find2(netid, config_opts, OPTION_DNSSERVER))
    p = option_put(p, end, OPTION_DNSSERVER, INADDRSZ, ntohl(context->local.s_addr));
  
  if (daemon->domain_suffix && in_list(req_options, OPTION_DOMAINNAME) && 
      !option_find2(netid, config_opts, OPTION_DOMAINNAME))
    p = option_put_string(p, end, OPTION_DOMAINNAME, daemon->domain_suffix, null_term);
 
  /* Note that we ignore attempts to set the hostname using 
     --dhcp-option=12,<name> and the fqdn using
     --dhc-option=81,<name> */
  if (hostname)
    {
      if (in_list(req_options, OPTION_HOSTNAME))
	p = option_put_string(p, end, OPTION_HOSTNAME, hostname, null_term);
      
      if (fqdn_flags != 0)
	{
	  int len = strlen(hostname) + 3;
	  if (fqdn_flags & 0x04)
	    len += 2;
	  else if (null_term)
	    len++;

	  if (daemon->domain_suffix)
	    len += strlen(daemon->domain_suffix) + 1;
	  
	  if (p + len + 1 < end)
	    {
	      *(p++) = OPTION_CLIENT_FQDN;
	      *(p++) = len;
	      *(p++) = fqdn_flags;
	      *(p++) = 255;
	      *(p++) = 255;

	      if (fqdn_flags & 0x04)
		{
		  p = do_rfc1035_name(p, hostname);
		  if (daemon->domain_suffix)
		    p = do_rfc1035_name(p, daemon->domain_suffix);
		  *p++ = 0;
		}
	      else
		{
		  memcpy(p, hostname, strlen(hostname));
		  p += strlen(hostname);
		  if (daemon->domain_suffix)
		    {
		      *(p++) = '.';
		      memcpy(p, daemon->domain_suffix, strlen(daemon->domain_suffix));
		      p += strlen(daemon->domain_suffix);
		    }
		  if (null_term)
		    *(p++) = 0;
		}
	    }
	}
    }      

  for (opt=config_opts; opt; opt = opt->next)
    {
      if (opt->opt == OPTION_HOSTNAME ||
	  opt->opt == OPTION_CLIENT_FQDN ||
	  opt->opt == OPTION_MAXMESSAGE ||
	  !in_list(req_options, opt->opt) ||
	  opt != option_find2(netid, config_opts, opt->opt))
	continue;
      
      /* For the options we have default values on
	 dhc-option=<optionno> means "don't include this option"
	 not "include a zero-length option" */
      if (opt->len == 0 && 
	  (opt->opt == OPTION_NETMASK ||
	   opt->opt == OPTION_BROADCAST ||
	   opt->opt == OPTION_ROUTER ||
	   opt->opt == OPTION_DNSSERVER))
	continue;

      /* opt->val has terminating zero */
      if (opt->opt == OPTION_VENDOR_ID)
	vendor_class = (char *)opt->val; 
      else
	p = do_opt(opt, p, end, context->local, null_term);
    }  

  if (in_list(req_options, OPTION_VENDOR_ID))
    {      
      for (opt = daemon->vendor_opts; opt; opt = opt->next)
	if (match_netid(opt->netid, netid, 1) || match_netid(opt->netid, netid, 0))
	  {
	    if (vendor_class && strcmp(vendor_class, (char *)opt->vendor_class) != 0)
	      syslog(LOG_WARNING, _("More than one vendor class matches, using %s"), vendor_class);
	    else
	      vendor_class = (char *)opt->vendor_class;
	  }

      if (vendor_class)
	{
	  p = option_put_string(p, end, OPTION_VENDOR_ID, vendor_class, 0);
	  
	  if (in_list(req_options, OPTION_VENDOR_CLASS_OPT))
	    {
	      unsigned char *plen, *oend = end;

	      /* encapsulated options can only be 256 bytes,
		 even of the packet is larger */
	      if (p + 256 < end)
		oend = p + 256;

	      if (p + 3 >= oend)
		return p;
	      
	      *(p++) = OPTION_VENDOR_CLASS_OPT;
	      plen = p++; /* fill in later */
	      
	      for (opt = daemon->vendor_opts; opt; opt = opt->next)
		if ((match_netid(opt->netid, netid, 1) || match_netid(opt->netid, netid, 0)) && 
		    strcmp(vendor_class, (char *)opt->vendor_class) == 0)
		  p = do_opt(opt, p, oend, context->local, null_term);
	      
	      *plen = p - plen - 1;
	    }
	}
    }
  
  return p;
}


