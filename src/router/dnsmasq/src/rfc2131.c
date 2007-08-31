/* dnsmasq is Copyright (c) 2000-2007 Simon Kelley

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
#define OPTION_SNAME             66
#define OPTION_FILENAME          67
#define OPTION_USER_CLASS        77
#define OPTION_CLIENT_FQDN       81
#define OPTION_AGENT_ID          82
#define OPTION_SUBNET_SELECT     118
#define OPTION_END               255

#define SUBOPT_CIRCUIT_ID        1
#define SUBOPT_REMOTE_ID         2
#define SUBOPT_SUBNET_SELECT     5     /* RFC 3527 */
#define SUBOPT_SUBSCR_ID         6     /* RFC 3393 */

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

static int sanitise(unsigned char *opt, char *buf);
static unsigned int calc_time(struct dhcp_context *context, struct dhcp_config *config, 
			      struct dhcp_lease *lease, unsigned char *opt, time_t now);
static void option_put(struct dhcp_packet *mess, unsigned char *end, int opt, int len, unsigned int val);
static void option_put_string(struct dhcp_packet *mess, unsigned char *end, 
			      int opt, char *string, int null_term);
static struct in_addr option_addr(unsigned char *opt);
static unsigned int option_uint(unsigned char *opt, int size);
static void log_packet(char *type, void *addr, 
		       unsigned char *ext_mac, int mac_len, char *interface, char *string);
static unsigned char *option_find(struct dhcp_packet *mess, size_t size, int opt_type, int minsize);
static unsigned char *option_find1(unsigned char *p, unsigned char *end, int opt, int minsize);
static size_t dhcp_packet_size(struct dhcp_packet *mess, struct dhcp_netid *netid);
static void clear_packet(struct dhcp_packet *mess, unsigned char *end);
static void do_options(struct dhcp_context *context,
		       struct dhcp_packet *mess,
		       unsigned char *real_end, 
		       unsigned char *req_options,
		       char *hostname,
		       struct dhcp_netid *netid,
		       struct in_addr subnet_addr,
		       unsigned char fqdn_flags,
		       int null_term,
		       unsigned char *agent_id);
static unsigned char *extended_hwaddr(int hwtype, int hwlen, unsigned char *hwaddr, 
				      int clid_len, unsigned char *clid, int *len_out);
static void match_vendor_opts(unsigned char *opt, struct dhcp_opt *dopt); 

	  
size_t dhcp_reply(struct dhcp_context *context, char *iface_name, 
		  size_t sz, time_t now, int unicast_dest, int *is_inform)
{
  unsigned char *opt, *clid = NULL;
  struct dhcp_lease *ltmp, *lease = NULL;
  struct dhcp_vendor *vendor;
  struct dhcp_mac *mac;
  struct dhcp_netid_list *id_list;
  int clid_len = 0, ignore = 0, do_classes = 0, selecting = 0;
  struct dhcp_packet *mess = daemon->dhcp_packet.iov_base;
  unsigned char *end = (unsigned char *)(mess + 1); 
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
  unsigned char *emac = NULL;
  int emac_len;
  struct dhcp_netid known_id;

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
	  unsigned char *sopt;
	  unsigned int total = option_len(opt) + 2;
	  unsigned char *last_opt = option_find(mess, sz, OPTION_END, 0);
	  if (last_opt && last_opt < end - total)
	    {
	      agent_id = end - total;
	      memcpy(agent_id, opt, total);
	    }

	  /* look for RFC3527 Link selection sub-option */
	  if ((sopt = option_find1(option_ptr(opt), option_ptr(opt) + option_len(opt), SUBOPT_SUBNET_SELECT, INADDRSZ)))
	    subnet_addr = option_addr(sopt);
	  
	  /* if a circuit-id or remote-is option is provided, exact-match to options. */ 
	  for (vendor = daemon->dhcp_vendors; vendor; vendor = vendor->next)
	    {
	      int search;
	      
	      if (vendor->match_type == MATCH_CIRCUIT)
		search = SUBOPT_CIRCUIT_ID;
	      else if (vendor->match_type == MATCH_REMOTE)
		search = SUBOPT_REMOTE_ID;
	      else if (vendor->match_type == MATCH_SUBSCRIBER)
		search = SUBOPT_SUBSCR_ID;
	      else 
		continue;

	      if ((sopt = option_find1(option_ptr(opt), option_ptr(opt) + option_len(opt), search, 1)) &&
		  vendor->len == option_len(sopt) &&
		  memcmp(option_ptr(sopt), vendor->data, vendor->len) == 0)
		{
		  vendor->netid.next = netid;
		  netid = &vendor->netid;
		  break;
		} 
	    }
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

      /* find mac to use for logging and hashing */
      emac = extended_hwaddr(mess->htype, mess->hlen, mess->chaddr, clid_len, clid, &emac_len);
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
      struct in_addr addr;
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
      else
	{
	  /* If ciaddr is in the hardware derived set of contexts, leave that unchanged */
	  addr = mess->ciaddr;
	  for (context_tmp = context; context_tmp; context_tmp = context_tmp->current)
	    if (context_tmp->netmask.s_addr && 
		is_same_net(addr, context_tmp->start, context_tmp->netmask) &&
		is_same_net(addr, context_tmp->end, context_tmp->netmask))
	      {
		context_new = context;
		break;
	      }
	} 
		
      if (!context_new)
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
      my_syslog(LOG_WARNING, _("no address range available for DHCP request %s %s"), 
		subnet_addr.s_addr ? _("with subnet selector") : _("via"),
		subnet_addr.s_addr ? inet_ntoa(subnet_addr) : (mess->giaddr.s_addr ? inet_ntoa(mess->giaddr) : iface_name));
      return 0;
    }

  /* keep _a_ local address available. */
  fallback = context->local;
  
  if (daemon->options & OPT_LOG_OPTS)
    {
      struct dhcp_context *context_tmp;
      my_syslog(LOG_INFO, _("DHCP packet: transaction-id is %u"), mess->xid);
      for (context_tmp = context; context_tmp; context_tmp = context_tmp->current)
	{
	  strcpy(daemon->namebuff, inet_ntoa(context_tmp->start));
	  if (context_tmp->flags & CONTEXT_STATIC)
	    my_syslog(LOG_INFO, _("Available DHCP subnet: %s/%s"), daemon->namebuff, inet_ntoa(context_tmp->netmask));
	  else
	    my_syslog(LOG_INFO, _("Available DHCP range: %s -- %s"), daemon->namebuff, inet_ntoa(context_tmp->end));
	}
    }

  mess->op = BOOTREPLY;
  
  config = find_config(daemon->dhcp_conf, context, clid, clid_len, 
		       mess->chaddr, mess->hlen, mess->htype, NULL);

  /* set "known" tag for known hosts */
  if (config)
    {
      known_id.net = "known";
      known_id.next = netid;
      netid = &known_id;
    }
  
  if (mess_type == 0)
    {
      /* BOOTP request */
      struct dhcp_netid id, bootp_id;
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
	  memcpy(daemon->dhcp_buff2, mess->file, sizeof(mess->file));
	  daemon->dhcp_buff2[sizeof(mess->file) + 1] = 0; /* ensure zero term. */
	  id.net = (char *)daemon->dhcp_buff2;
	  id.next = netid;
	  netid = &id;
	}

      /* Add "bootp" as a tag to allow different options, address ranges etc
	 for BOOTP clients */
      bootp_id.net = "bootp";
      bootp_id.next = netid;
      netid = &bootp_id;
      
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
		   if (!address_allocate(context, &mess->yiaddr, mess->chaddr, mess->hlen, netid, now))
		     message = _("no address available");
		}
	      else
		mess->yiaddr = lease->addr;
	    }
	  
	  if (!message && 
	      !lease && 
	      (!(lease = lease_allocate(mess->yiaddr))))
	    {
	      my_syslog(LOG_WARNING, _("Limit of %d leases exceeded."), daemon->dhcp_max);
	      message = _("no leases left");
	    }
	  
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
	      
	      clear_packet(mess, end);
	      do_options(context, mess, end, NULL,  
			 hostname, netid, subnet_addr, 0, 0, NULL);
	    }
	}
      
      log_packet(NULL, logaddr, mess->chaddr, mess->hlen, iface_name, message);
      
      return message ? 0 : dhcp_packet_size(mess, netid);
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
      /* be careful not to send an OFFER with a hostname not matching the DISCOVER. */
      if (fqdn_flags != 0 || !client_hostname || hostname_isequal(hostname, client_hostname))
        offer_hostname = hostname;
    }
  else if (client_hostname)
    {
      char *d = strip_hostname(client_hostname);
      if (d)
	my_syslog(LOG_WARNING, _("Ignoring domain %s for DHCP host name %s"), d, client_hostname);
      
      if (strlen(client_hostname) != 0)
	{
	  hostname = client_hostname;
	  if (!config)
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
	}
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
     and just do the substring match. dhclient provides these broken options.
     The code, later, which sends user-class data to the lease-change script
     relies on the transformation done here.
  */

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
    {
      int mopt;
      
      if (vendor->match_type == MATCH_VENDOR)
	mopt = OPTION_VENDOR_ID;
      else if (vendor->match_type == MATCH_USER)
	mopt = OPTION_USER_CLASS; 
      else
	continue;

      if ((opt = option_find(mess, sz, mopt, 1)))
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
    }

  /* mark vendor-encapsulated options which match the client-supplied vendor class */
  match_vendor_opts(option_find(mess, sz, OPTION_VENDOR_ID, 1), daemon->dhcp_opts);
    
  if (daemon->options & OPT_LOG_OPTS)
    {
      if (sanitise(option_find(mess, sz, OPTION_VENDOR_ID, 1), daemon->namebuff))
	my_syslog(LOG_INFO, _("Vendor class: %s"), daemon->namebuff);
      if (sanitise(option_find(mess, sz, OPTION_USER_CLASS, 1), daemon->namebuff))
	my_syslog(LOG_INFO, _("User class: %s"), daemon->namebuff);
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
      sanitise(option_find(mess, sz, OPTION_MESSAGE, 1), daemon->dhcp_buff);
      
      if (!(opt = option_find(mess, sz, OPTION_REQUESTED_IP, INADDRSZ)))
	return 0;
      
      log_packet("DECLINE", option_ptr(opt), emac, emac_len, iface_name, daemon->dhcp_buff);
      
      if (lease && lease->addr.s_addr == option_addr(opt).s_addr)
	lease_prune(lease, now);
      
      if (have_config(config, CONFIG_ADDR) && 
	  config->addr.s_addr == option_addr(opt).s_addr)
	{
	  prettyprint_time(daemon->dhcp_buff, DECLINE_BACKOFF);
	  my_syslog(LOG_WARNING, _("disabling DHCP static address %s for %s"), 
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
      if (!(context = narrow_context(context, mess->ciaddr)) ||
	  !(opt = option_find(mess, sz, OPTION_SERVER_IDENTIFIER, INADDRSZ)) ||
	  (context->local.s_addr != option_addr(opt).s_addr))
	return 0;
      
      if (lease && lease->addr.s_addr == mess->ciaddr.s_addr)
	lease_prune(lease, now);
      else
	message = _("unknown lease");

      log_packet("RELEASE", &mess->ciaddr, emac, emac_len, iface_name, message);
	
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
		{
		  int len;
		  unsigned char *mac = extended_hwaddr(ltmp->hwaddr_type, ltmp->hwaddr_len,
						       ltmp->hwaddr, ltmp->clid_len, ltmp->clid, &len);
		  my_syslog(LOG_WARNING, _("not using configured address %s because it is leased to %s"),
			    addrs, print_mac(daemon->namebuff, mac, len));
		}
	      else
		{
		  struct dhcp_context *tmp;
		  for (tmp = context; tmp; tmp = tmp->current)
		    if (context->router.s_addr == config->addr.s_addr)
		      break;
		  if (tmp)
		    my_syslog(LOG_WARNING, _("not using configured address %s because it is in use by the server or relay"), addrs);
		  else if (have_config(config, CONFIG_DECLINED) &&
			   difftime(now, config->decline_time) < (float)DECLINE_BACKOFF)
		    my_syslog(LOG_WARNING, _("not using configured address %s because it was previously declined"), addrs);
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
	  else if (emac_len == 0)
	    message = _("no unique-id");
	  else if (!address_allocate(context, &mess->yiaddr, emac, emac_len, netid, now))
	    message = _("no address available");      
	}
      
      log_packet("DISCOVER", opt ? option_ptr(opt) : NULL, emac, emac_len, iface_name, message); 

      if (message || !(context = narrow_context(context, mess->yiaddr)))
	return 0;

      log_packet("OFFER" , &mess->yiaddr, emac, emac_len, iface_name, NULL);

      if (context->netid.net)
	{
	  context->netid.next = netid;
	  netid = &context->netid;
	}
       
      time = calc_time(context, config, lease, option_find(mess, sz, OPTION_LEASE_TIME, 4), now);
      clear_packet(mess, end);
      option_put(mess, end, OPTION_MESSAGE_TYPE, 1, DHCPOFFER);
      option_put(mess, end, OPTION_SERVER_IDENTIFIER, INADDRSZ, ntohl(context->local.s_addr));
      option_put(mess, end, OPTION_LEASE_TIME, 4, time);
      /* T1 and T2 are required in DHCPOFFER by HP's wacky Jetdirect client. */
      if (time != 0xffffffff)
	{
	  option_put(mess, end, OPTION_T1, 4, (time/2));
	  option_put(mess, end, OPTION_T2, 4, (time*7)/8);
	}
      do_options(context, mess, end, req_options, offer_hostname, 
		 netid, subnet_addr, fqdn_flags, borken_opt, agent_id);
      
      return dhcp_packet_size(mess, netid);
      
    case DHCPREQUEST:
      if (ignore || have_config(config, CONFIG_DISABLE))
	return 0;
      if ((opt = option_find(mess, sz, OPTION_REQUESTED_IP, INADDRSZ)))
	{
	  /* SELECTING  or INIT_REBOOT */
	  mess->yiaddr = option_addr(opt);
	  
	  /* send vendor and user class info for new or recreated lease */
	  do_classes = 1;
	  
	  if ((opt = option_find(mess, sz, OPTION_SERVER_IDENTIFIER, INADDRSZ)))
	    {
	      /* SELECTING */
	      selecting = 1;
	      
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
      
      log_packet("REQUEST", &mess->yiaddr, emac, emac_len, iface_name, NULL);
 
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
	  else if (!tmp && !selecting &&
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
	  
	  else if (emac_len == 0)
	    message = _("no unique-id");
	  
	  else if (!lease)
	    {	     
	      if ((lease = lease_allocate(mess->yiaddr)))
		do_classes = 1;
	      else
		message = _("no leases left");
	    }
	}

      if (message)
	{
	  log_packet("NAK", &mess->yiaddr, emac, emac_len, iface_name, message);
	  
	  mess->yiaddr.s_addr = 0;
	  clear_packet(mess, end);
	  option_put(mess, end, OPTION_MESSAGE_TYPE, 1, DHCPNAK);
	  option_put(mess, end, OPTION_SERVER_IDENTIFIER, INADDRSZ, 
		     ntohl(context ? context->local.s_addr : fallback.s_addr));
	  option_put_string(mess, end, OPTION_MESSAGE, message, borken_opt);
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
	   if (do_classes)
	     {
	       lease->changed = 1;
	       /* copy user-class and vendor class into new lease, for the script */
	       if ((opt = option_find(mess, sz, OPTION_USER_CLASS, 1)))
		 {
		   int len = option_len(opt);
		   unsigned char *ucp = option_ptr(opt);
		   /* If the user-class option started as counted strings, the first byte will be zero. */
		   if (len != 0 && ucp[0] == 0)
		     ucp++, len--;
		   free(lease->userclass);
		   if ((lease->userclass = whine_malloc(len+1)))
		     {
		       memcpy(lease->userclass, ucp, len);
		       lease->userclass[len] = 0;
		       lease->userclass_len = len+1;
		     }
		 }
	       if ((opt = option_find(mess, sz, OPTION_VENDOR_ID, 1)))
		 {
		   int len = option_len(opt);
		   unsigned char *ucp = option_ptr(opt);
		   free(lease->vendorclass);
		   if ((lease->vendorclass = whine_malloc(len+1)))
		     {
		       memcpy(lease->vendorclass, ucp, len);
		       lease->vendorclass[len] = 0;
		       lease->vendorclass_len = len+1;
		     }
		 }
	     }
	   
	   if (!hostname_auth && (client_hostname = host_from_dns(mess->yiaddr)))
	     {
	      hostname = client_hostname;
	      hostname_auth = 1;
	    }
      
	  if (context->netid.net)
	    {
	      context->netid.next = netid;
	      netid = &context->netid;
	    }
	
	  time = calc_time(context, config, NULL, option_find(mess, sz, OPTION_LEASE_TIME, 4), now);
	  lease_set_hwaddr(lease, mess->chaddr, clid, mess->hlen, mess->htype, clid_len);
	   
	  /* if all the netids in the ignore_name list are present, ignore client-supplied name */
	  if (!hostname_auth)
	    {
	      for (id_list = daemon->dhcp_ignore_names; id_list; id_list = id_list->next)
		if ((!id_list->list) || match_netid(id_list->list, netid, 0))
		  break;
	      if (id_list)
		hostname = NULL;
	    }
	  if (hostname)
	    lease_set_hostname(lease, hostname, daemon->domain_suffix, hostname_auth);
	  
	  lease_set_expires(lease, time, now);
	  	
	  log_packet("ACK", &mess->yiaddr, emac, emac_len, iface_name, hostname);  
	  
	  clear_packet(mess, end);
	  option_put(mess, end, OPTION_MESSAGE_TYPE, 1, DHCPACK);
	  option_put(mess, end, OPTION_SERVER_IDENTIFIER, INADDRSZ, ntohl(context->local.s_addr));
	  option_put(mess, end, OPTION_LEASE_TIME, 4, time);
	  if (time != 0xffffffff)
	    {
	      while (fuzz > (time/16))
		fuzz = fuzz/2; 
	      option_put(mess, end, OPTION_T1, 4, (time/2) - fuzz);
	      option_put(mess, end, OPTION_T2, 4, ((time/8)*7) - fuzz);
	    }
	  do_options(context, mess, end, req_options, hostname, 
		     netid, subnet_addr, fqdn_flags, borken_opt, agent_id);
	}

      return dhcp_packet_size(mess, netid); 
      
    case DHCPINFORM:
      if (ignore || have_config(config, CONFIG_DISABLE))
	message = _("ignored");
      
      log_packet("INFORM", &mess->ciaddr, emac, emac_len, iface_name, message);
     
      if (message || mess->ciaddr.s_addr == 0 || 
	  !(context = narrow_context(context, mess->ciaddr)))
	return 0;
      
      /* Find a least based on IP address if we didn't
	 get one from MAC address/client-d */
      if (!lease &&
	  (lease = lease_find_by_addr(mess->ciaddr)) && 
	  lease->hostname)
	hostname = lease->hostname;
      
      if (!hostname)
	hostname = host_from_dns(mess->ciaddr);

      log_packet("ACK", &mess->ciaddr, emac, emac_len, iface_name, hostname);
      
      if (context->netid.net)
	{
	  context->netid.next = netid;
	  netid = &context->netid;
	}
       
      clear_packet(mess, end);
      option_put(mess, end, OPTION_MESSAGE_TYPE, 1, DHCPACK);
      option_put(mess, end, OPTION_SERVER_IDENTIFIER, INADDRSZ, ntohl(context->local.s_addr));

      if (lease)
	{
	  if (lease->expires == 0)
	    time = 0xffffffff;
	  else
	    time = (unsigned int)difftime(lease->expires, now);
	  option_put(mess, end, OPTION_LEASE_TIME, 4, time);
	}
      do_options(context, mess, end, req_options, hostname, 
		 netid, subnet_addr, fqdn_flags, borken_opt, agent_id);
      
      *is_inform = 1; /* handle reply differently */
      return dhcp_packet_size(mess, netid); 
    }
  
  return 0;
}

/* find a good value to use as MAC address for logging and address-allocation hashing.
   This is normally just the chaddr field from the DHCP packet,
   but eg Firewire will have hlen == 0 and use the client-id instead. 
   This could be anything, but will normally be EUI64 for Firewire.
   We assume that if the first byte of the client-id equals the htype byte
   then the client-id is using the usual encoding and use the rest of the 
   client-id: if not we can use the whole client-id. This should give
   sane MAC address logs. */
static unsigned char *extended_hwaddr(int hwtype, int hwlen, unsigned char *hwaddr, 
				      int clid_len, unsigned char *clid, int *len_out)
{
  if (hwlen == 0 && clid && clid_len > 3)
    {
      if (clid[0]  == hwtype)
	{
	  *len_out = clid_len - 1 ;
	  return clid + 1;
	}

#if defined(ARPHRD_EUI64) && defined(ARPHRD_IEEE1394)
      if (clid[0] ==  ARPHRD_EUI64 && hwtype == ARPHRD_IEEE1394)
	{
	  *len_out = clid_len - 1 ;
	  return clid + 1;
	}
#endif
      
      *len_out = clid_len;
      return clid;
    }
  
  *len_out = hwlen;
  return hwaddr;
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

static int sanitise(unsigned char *opt, char *buf)
{
  char *p;
  int i;
  
  *buf = 0;
  
  if (!opt)
    return 0;

  p = option_ptr(opt);

  for (i = option_len(opt); i > 0; i--)
    {
      char c = *p++;
      if (isprint(c))
	*buf++ = c;
    }
  *buf = 0; /* add terminator */
  
  return 1;
}

static void log_packet(char *type, void *addr, unsigned char *ext_mac, 
		       int mac_len, char *interface, char *string)
{
  struct in_addr a;

  /* addr may be misaligned */
  if (addr)
    memcpy(&a, addr, sizeof(a));
  
  my_syslog(LOG_INFO, "%s%s(%s) %s%s%s %s",
	    type ? "DHCP" : "BOOTP",
	    type ? type : "",
	    interface, 
	    addr ? inet_ntoa(a) : "",
	    addr ? " " : "",
	    print_mac(daemon->namebuff, ext_mac, mac_len),
	    string ? string : "");
}

static void log_options(unsigned char *start)
{
  while (*start != OPTION_END)
    {
      char *text = option_string(start[0]);
      unsigned char trunc = start[1] < 13 ? start[1] : 13;
      my_syslog(LOG_INFO, "sent size:%3d option:%3d%s%s%s%s%s", 
		start[1], start[0],
		text ? ":" : "", text ? text : "",
		start[1] == 0 ? "" : "  ",
		start[1] == 0 ? "" : print_mac(daemon->namebuff, &start[2], trunc),
		trunc == start[1] ? "" : "...");
      start += start[1] + 2;
    }
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
  unsigned char *ret, *overload;
  
  /* skip over DHCP cookie; */
  if ((ret = option_find1(&mess->options[0] + sizeof(u32), ((unsigned char *)mess) + size, opt_type, minsize)))
    return ret;

  /* look for overload option. */
  if (!(overload = option_find1(&mess->options[0] + sizeof(u32), ((unsigned char *)mess) + size, OPTION_OVERLOAD, 1)))
    return NULL;
  
  /* Can we look in filename area ? */
  if ((overload[2] & 1) &&
      (ret = option_find1(&mess->file[0], &mess->file[128], opt_type, minsize)))
    return ret;

  /* finally try sname area */
  if ((overload[2] & 2) &&
      (ret = option_find1(&mess->sname[0], &mess->sname[64], opt_type, minsize)))
    return ret;

  return NULL;
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

static unsigned char *dhcp_skip_opts(unsigned char *start)
{
  while (*start != 0)
    start += start[1] + 2;
  return start;
}

/* only for use when building packet: doesn't check for bad data. */ 
static unsigned char *find_overload(struct dhcp_packet *mess)
{
  unsigned char *p = &mess->options[0] + sizeof(u32);
  
  while (*p != 0)
    {
      if (*p == OPTION_OVERLOAD)
	return p;
      p += p[1] + 2;
    }
  return NULL;
}

static size_t dhcp_packet_size(struct dhcp_packet *mess, struct dhcp_netid *netid)
{
  unsigned char *p = dhcp_skip_opts(&mess->options[0] + sizeof(u32));
  unsigned char *overload;
  size_t ret;

  /* We do logging too */
  if (netid && (daemon->options & OPT_LOG_OPTS))
    {
      char *p = daemon->namebuff;
      *p = 0;
      for (; netid; netid = netid->next)
	{
	  strncat (p, netid->net, MAXDNAME);
	  if (netid->next)
	    strncat (p, ", ", MAXDNAME);
	}
      p[MAXDNAME - 1] = 0;
      my_syslog(LOG_INFO, _("tags: %s"), p);
    } 
   
  /* add END options to the regions. */
  if ((overload = find_overload(mess)))
    {
      if (option_uint(overload, 1) & 1)
	{
	  *dhcp_skip_opts(mess->file) = OPTION_END;
	  if (daemon->options & OPT_LOG_OPTS)
	    log_options(mess->file);
	}
      if (option_uint(overload, 1) & 2)
	{
	  *dhcp_skip_opts(mess->sname) = OPTION_END;
	  if (daemon->options & OPT_LOG_OPTS)
	    log_options(mess->sname);
	}
    }

  *p++ = OPTION_END;
  if (daemon->options & OPT_LOG_OPTS)
    log_options(&mess->options[0] + sizeof(u32));
  
  ret = (size_t)(p - (unsigned char *)mess);
  
  if (ret < MIN_PACKETSZ)
    ret = MIN_PACKETSZ;

  return ret;
}

static unsigned char *free_space(struct dhcp_packet *mess, unsigned char *end, int opt, int len)
{
  unsigned char *p = dhcp_skip_opts(&mess->options[0] + sizeof(u32));
  
  if (p + len + 3 >= end)
    /* not enough space in options area, try and use overload, if poss */
    {
      unsigned char *overload;
      
      if (!(overload = find_overload(mess)) &&
	  (mess->file[0] == 0 || mess->sname[0] == 0))
	{
	  /* attempt to overload fname and sname areas, we've reserved space for the
	     overflow option previuously. */
	  overload = p;
	  *(p++) = OPTION_OVERLOAD;
	  *(p++) = 1;
	}
      
      p = NULL;
      
      /* using filename field ? */
      if (overload)
	{
	  if (mess->file[0] == 0)
	    overload[2] |= 1;
	  
	  if (overload[2] & 1)
	    {
	      p = dhcp_skip_opts(mess->file);
	      if (p + len + 3 >= mess->file + sizeof(mess->file))
		p = NULL;
	    }
	  
	  if (!p)
	    {
	      /* try to bring sname into play (it may be already) */
	      if (mess->sname[0] == 0)
		overload[2] |= 2;
	      
	      if (overload[2] & 2)
		{
		  p = dhcp_skip_opts(mess->sname);
		  if (p + len + 3 >= mess->sname + sizeof(mess->file))
		    p = NULL;
		}
	    }
	}
      
      if (!p)
	my_syslog(LOG_WARNING, _("cannot send DHCP/BOOTP option %d: no space left in packet"), opt);
    }
 
  if (p)
    {
      *(p++) = opt;
      *(p++) = len;
    }

  return p;
}
	      
static void option_put(struct dhcp_packet *mess, unsigned char *end, int opt, int len, unsigned int val)
{
  int i;
  unsigned char *p = free_space(mess, end, opt, len);
  
  if (p) 
    for (i = 0; i < len; i++)
      *(p++) = val >> (8 * (len - (i + 1)));
}

static void option_put_string(struct dhcp_packet *mess, unsigned char *end, int opt, 
			      char *string, int null_term)
{
  unsigned char *p;
  size_t len = strlen(string);

  if (null_term && len != 255)
    len++;

  if ((p = free_space(mess, end, opt, len)))
    memcpy(p, string, len);
}

/* return length, note this only does the data part */
static int do_opt(struct dhcp_opt *opt, unsigned char *p, struct in_addr local, int null_term)
{
  int len = opt->len;
  
  if ((opt->flags & DHOPT_STRING) && null_term && len != 255)
    len++;

  if (p && len != 0)
    {
      if ((opt->flags & DHOPT_ADDR) && !(opt->flags & DHOPT_ENCAPSULATE))
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
	memcpy(p, opt->val, len);
    }  
  return len;
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
    if (tmp->opt == opt && !(tmp->flags & DHOPT_ENCAPSULATE))
      if (match_netid(tmp->netid, netid, 1) || match_netid(tmp->netid, netid, 0))
	return tmp;
	      
  return netid ? option_find2(NULL, opts, opt) : NULL;
}

/* mark vendor-encapsulated options which match the client-supplied  or
   config-supplied vendor class */
static void match_vendor_opts(unsigned char *opt, struct dhcp_opt *dopt)
{
  for (; dopt; dopt = dopt->next)
    {
      dopt->flags &= ~DHOPT_VENDOR_MATCH;
      if (opt && (dopt->flags & DHOPT_ENCAPSULATE))
	{
	  int i, len = 0;
	  if (dopt->vendor_class)
	    len = strlen((char *)dopt->vendor_class);
	  for (i = 0; i <= (option_len(opt) - len); i++)
	    if (len == 0 || memcmp(dopt->vendor_class, option_ptr(opt)+i, len) == 0)
	      {
		dopt->flags |= DHOPT_VENDOR_MATCH;
		break;
	      }
	}
    }
}

static void clear_packet(struct dhcp_packet *mess, unsigned char *end)
{
  memset(mess->sname, 0, sizeof(mess->sname));
  memset(mess->file, 0, sizeof(mess->file));
  memset(&mess->options[0] + sizeof(u32), 0, end - (&mess->options[0] + sizeof(u32)));
  mess->siaddr.s_addr = 0;
}

static void do_options(struct dhcp_context *context,
		       struct dhcp_packet *mess,
		       unsigned char *real_end, 
		       unsigned char *req_options,
		       char *hostname,
		       struct dhcp_netid *netid,
		       struct in_addr subnet_addr,
		       unsigned char fqdn_flags,
		       int null_term,
		       unsigned char *agent_id)
{
  struct dhcp_opt *opt, *config_opts = daemon->dhcp_opts;
  struct dhcp_boot *boot;
  unsigned char *p, *end = agent_id ? agent_id : real_end;
  int i, len, force_encap = 0;
  unsigned char f0 = 0, s0 = 0;

  /* logging */
  if ((daemon->options & OPT_LOG_OPTS) && req_options)
    {
      char *q = daemon->namebuff;
      for (i = 0; req_options[i] != OPTION_END; i++)
	{
	  char *s = option_string(req_options[i]);
	  q +=snprintf(q, MAXDNAME - (q - daemon->namebuff),
		       "%d%s%s%s", 
		       req_options[i],
		       s ? ":" : "",
		       s ? s : "", 
		       req_options[i+1] == OPTION_END ? "" : ", ");
	  if (req_options[i+1] == OPTION_END || (q - daemon->namebuff) > 40)
	    {
	      q = daemon->namebuff;
	      my_syslog(LOG_INFO, _("requested options: %s"), daemon->namebuff);
	    }
	}
    }
      
  /* decide which dhcp-boot option we're using */
  for (boot = daemon->boot_config; boot; boot = boot->next)
    if (match_netid(boot->netid, netid, 0))
      break;
  if (!boot)
    /* No match, look for one without a netid */
    for (boot = daemon->boot_config; boot; boot = boot->next)
      if (match_netid(boot->netid, netid, 1))
	break;
  
  mess->siaddr = context->local;
  
  /* See if we can send the boot stuff as options.
     To do this we need a requested option list, BOOTP
     and very old DHCP clients won't have this. 
     Some PXE ROMs have bugs (surprise!) and need zero-terminated 
     names, so we always send those. */
  if (boot)
    {
      if (boot->sname)
	{
	  if (req_options && in_list(req_options, OPTION_SNAME))
	    option_put_string(mess, end, OPTION_SNAME, boot->sname, 1);
	  else
	    {
	      if (daemon->options & OPT_LOG_OPTS)
		my_syslog(LOG_INFO, _("server name: %s"), boot->sname);
	      strncpy((char *)mess->sname, boot->sname, sizeof(mess->sname)-1);
	    }
	}
      
      if (boot->file)
	{
	  if (req_options && in_list(req_options, OPTION_FILENAME))
	    option_put_string(mess, end, OPTION_FILENAME, boot->file, 1);
	  else
	    {
	      if (daemon->options & OPT_LOG_OPTS)
		my_syslog(LOG_INFO, _("bootfile name: %s"), boot->file);
	      strncpy((char *)mess->file, boot->file, sizeof(mess->file)-1);
	    }
	}
      
      if (boot->next_server.s_addr)
	mess->siaddr = boot->next_server;
	
      if (daemon->options & OPT_LOG_OPTS)
	my_syslog(LOG_INFO, _("next server: %s"), inet_ntoa(mess->siaddr));
    }
  
  /* We don't want to do option-overload for BOOTP, so make the file and sname
     fields look like they are in use, even when they aren't. This gets restored
     at the end of this function. */

  if (!req_options)
    {
      f0 = mess->file[0];
      mess->file[0] = 1;
      s0 = mess->sname[0];
      mess->sname[0] = 1;
    }
      
  /* At this point, if mess->sname or mess->file are zeroed, they are available
     for option overload, reserve space for the overload option. */
  if (mess->file[0] == 0 || mess->sname[0] == 0)
    end -= 3;

  /* rfc3011 says this doesn't need to be in the requested options list. */
  if (subnet_addr.s_addr)
    option_put(mess, end, OPTION_SUBNET_SELECT, INADDRSZ, ntohl(subnet_addr.s_addr));

  if (!option_find2(netid, config_opts, OPTION_NETMASK))
    option_put(mess, end, OPTION_NETMASK, INADDRSZ, ntohl(context->netmask.s_addr));
  
  /* May not have a "guessed" broadcast address if we got no packets via a relay
     from this net yet (ie just unicast renewals after a restart */
  if (context->broadcast.s_addr &&
      !option_find2(netid, config_opts, OPTION_BROADCAST))
    option_put(mess, end, OPTION_BROADCAST, INADDRSZ, ntohl(context->broadcast.s_addr));
  
  /* Same comments as broadcast apply, and also may not be able to get a sensible
     default when using subnet select.  User must configure by steam in that case. */
  if (context->router.s_addr &&
      in_list(req_options, OPTION_ROUTER) &&
      !option_find2(netid, config_opts, OPTION_ROUTER))
    option_put(mess, end, OPTION_ROUTER, INADDRSZ, ntohl(context->router.s_addr));

  if (in_list(req_options, OPTION_DNSSERVER) &&
      !option_find2(netid, config_opts, OPTION_DNSSERVER))
    option_put(mess, end, OPTION_DNSSERVER, INADDRSZ, ntohl(context->local.s_addr));
  
  if (daemon->domain_suffix && in_list(req_options, OPTION_DOMAINNAME) && 
      !option_find2(netid, config_opts, OPTION_DOMAINNAME))
    option_put_string(mess, end, OPTION_DOMAINNAME, daemon->domain_suffix, null_term);
 
  /* Note that we ignore attempts to set the hostname using 
     --dhcp-option=12,<name> and the fqdn using
     --dhc-option=81,<name> */
  if (hostname)
    {
      if (in_list(req_options, OPTION_HOSTNAME))
	option_put_string(mess, end, OPTION_HOSTNAME, hostname, null_term);
      
      if (fqdn_flags != 0)
	{
	  int len = strlen(hostname) + 3;
	  if (fqdn_flags & 0x04)
	    len += 2;
	  else if (null_term)
	    len++;

	  if (daemon->domain_suffix)
	    len += strlen(daemon->domain_suffix) + 1;
	  
	  if ((p = free_space(mess, end, OPTION_CLIENT_FQDN, len)))
	    {
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

  for (opt = config_opts; opt; opt = opt->next)
    {
      /* was it asked for, or are we sending it anyway? */
      if (!(opt->flags & DHOPT_FORCE) && !in_list(req_options, opt->opt))
	continue;
      
      /* prohibit some used-internally options */
      if (opt->opt == OPTION_HOSTNAME ||
	  opt->opt == OPTION_CLIENT_FQDN ||
	  opt->opt == OPTION_MAXMESSAGE ||
	  opt->opt == OPTION_OVERLOAD ||
	  opt->opt == OPTION_PAD ||
	  opt->opt == OPTION_END)
	continue;
      
      /* netids match and not encapsulated? */
      if (opt != option_find2(netid, config_opts, opt->opt))
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

      len = do_opt(opt, NULL, context->local, null_term);
      if ((p = free_space(mess, end, opt->opt, len)))
	{
	  do_opt(opt, p, context->local, null_term);

	  /* If we send a vendor-id, revisit which vendor-ops we consider 
	     it appropriate to send. */
	  if (opt->opt == OPTION_VENDOR_ID)
	    match_vendor_opts(p - 2, config_opts);
	}  
    }

  /* prune encapsulated options based on netid, and look if we're forcing them to be sent */
  for (opt = config_opts; opt; opt = opt->next)
    if (opt->flags & DHOPT_VENDOR_MATCH)
      {
	if (!match_netid(opt->netid, netid, 1) && !match_netid(opt->netid, netid, 0))
	  opt->flags &= ~DHOPT_VENDOR_MATCH;
	else if (opt->flags & DHOPT_FORCE)
	  force_encap = 1;
      }
  
  if (force_encap || in_list(req_options, OPTION_VENDOR_CLASS_OPT))
    {
      int enc_len = 0;
      struct dhcp_opt *start;

      /* find size in advance */
      for (start = opt = config_opts; opt; opt = opt->next)
	if (opt->flags & DHOPT_VENDOR_MATCH)
	  {
	    int new = do_opt(opt, NULL, context->local, null_term) + 2;
	    if (enc_len + new <= 255)
	      enc_len += new;
	    else
	      {
		p = free_space(mess, end, OPTION_VENDOR_CLASS_OPT, enc_len);
		for (; start && start != opt; start = start->next)
		  if (p && (start->flags & DHOPT_VENDOR_MATCH))
		    {
		      len = do_opt(start, p + 2, context->local, null_term);
		      *(p++) = start->opt;
		      *(p++) = len;
		      p += len;
		    }
		enc_len = new;
		start = opt;
	      }
	  }
	      
      if (enc_len != 0 &&
	  (p = free_space(mess, end, OPTION_VENDOR_CLASS_OPT, enc_len + 1)))
	{
	  for (; start; start = start->next)
	    if (start->flags & DHOPT_VENDOR_MATCH)
	      {
		 len = do_opt(start, p + 2, context->local, null_term);
		 *(p++) = start->opt;
		 *(p++) = len;
		 p += len;
	      }
	  *p = OPTION_END;
	}
    }
  
  /* move agent_id back down to the end of the packet */
  if (agent_id)
    {
      p = dhcp_skip_opts(&mess->options[0] + sizeof(u32));
      memmove(p, agent_id, real_end - agent_id);
      p += real_end - agent_id;
      memset(p, 0, real_end - p); /* in case of overlap */
    }

  /* restore BOOTP anti-overload hack */
  if (!req_options)
    {
      mess->file[0] = f0;
      mess->sname[0] = s0;
    }
}

