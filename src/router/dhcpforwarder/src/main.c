// Copyright (C) 2002, 2003, 2004, 2008, 2012, 2014
//               Enrico Scholz <enrico.scholz@ensc.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "splint_compat.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/param.h>
#include <errno.h>
#include <alloca.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>

#include <netpacket/packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include "cfg.h"
#include "wrappers.h"
#include "dhcp.h"
#include "inet.h"
#include "recvfromflags.h"

#include "assertions.h"
#include "logging.h"

  //#define ENABLE_AGENT_REPLACE		1

typedef enum {
  acIGNORE,		//< Do nothing...
  acREMOVE_AGENT_INFO,	//< Remove an already existing agent-id field
  acADD_AGENT_INFO,	//< Add an agent-id field if such a field does not
			//< already exists
  acREPLACE_AGENT_INFO,	//< Replace an already existing agent-id field
} OptionFillAction;

/*@checkmod@*/static struct ServerInfoList	servers;
/*@checkmod@*/static struct FdInfoList		fds;

unsigned long	g_compat_hacks;

static void
fillFDSet(/*@out@*/fd_set			*fdset,
	  /*@out@*/int				*max)
    /*@globals fds@*/
    /*@modifies *fdset, *max@*/
{
    /*@-nullptrarith@*/
  struct FdInfo const *		fdinfo;
  struct FdInfo const * const	end_fdinfo = fds.dta+fds.len;
    /*@=nullptrarith@*/

  assert(fds.dta!=0 || fds.len==0);

  FD_ZERO(fdset);
  *max = -1;

  for (fdinfo=fds.dta; fdinfo<end_fdinfo; ++fdinfo) {
    assert(fdinfo!=0);

    *max = MAX(*max, fdinfo->fd);
    FD_SET(fdinfo->fd, fdset);
  }
}

static /*@exposed@*//*@null@*/struct FdInfo const *
lookupFD(/*@in@*/struct in_addr const addr)
    /*@globals fds@*/
{
    /*@-nullptrarith@*/
  struct FdInfo const *			fdinfo;
  struct FdInfo const * const		end_fdinfo = fds.dta+fds.len;
    /*@=nullptrarith@*/

  assert(fds.dta!=0 || fds.len==0);

  for (fdinfo=fds.dta; fdinfo<end_fdinfo; ++fdinfo) {
    assert(fdinfo!=0);

      /* We must check for both the real and "faked" giaddr IP here */
    if (fdinfo->iface->if_real_ip==addr.s_addr ||
	fdinfo->iface->if_ip     ==addr.s_addr) return fdinfo;
  }

  return 0;
}

static size_t
determineMaxMTU()
    /*@globals fds@*/
    /*@modifies@*/
{
  size_t				result = 1500;
    /*@-nullptrarith@*/
  struct FdInfo const *			fdinfo;
  struct FdInfo const * const		end_fdinfo = fds.dta+fds.len;
    /*@=nullptrarith@*/

  assert(fds.dta!=0 || fds.len==0);

  for (fdinfo=fds.dta; fdinfo<end_fdinfo; ++fdinfo) {
    assert(fdinfo!=0);

    result = MAX(result, fdinfo->iface->if_mtu);
  }

  return result;
}

static bool
isValidHeader(/*@in@*/struct DHCPHeader *header)
    /*@globals internalState@*/
    /*@modifies internalState@*/
{
  char const		*reason = 0;

    /*@-strictops@*/
  if ((header->flags&~flgDHCP_BCAST)!=0) { reason = "Invalid flags field"; }
    /*@=strictops@*/
  else if (header->hops>=MAX_HOPS)       { reason = "Looping detected"; }
#if 0
  else switch (header->htype) {
    case ARPHRD_ETHER	:
      if (header->hlen!=ETH_ALEN) {  reason = "Invalid hlen for ethernet"; }
      break;
    default		:
      break;	// Not active handled by us; will be forwarded as-is
  }
#else
  else {}
#endif

  if (reason==0) switch (header->op) {
    case opBOOTREPLY	:
    case opBOOTREQUEST	:  break;
    default		:  reason = "Unknown operation"; break;
  };

  if (reason!=0) LOGSTR(reason);

  return reason==0;
}

static bool
isValidOptions(/*@in@*/struct DHCPOptions const	*options,
	       size_t				o_len)
    /*@*/
{
  bool				seen_end     = false;
  struct DHCPSingleOption const	*opt         = reinterpret_cast(struct DHCPSingleOption const *)(options->data);
  struct DHCPSingleOption const	*end_options = reinterpret_cast(struct DHCPSingleOption const *)(&reinterpret_cast(uint8_t const *)(options)[o_len]);

    /* Is this really ok? Is an empty option-field RFC compliant? */
  if (o_len==0) return true;
  if (o_len<=4) return false;
  if (options->cookie != optDHCP_COOKIE) return false;

  do {
    switch (opt->code) {
      case cdEND	:  seen_end = true; break;
      default		:  break;
    }

    opt = DHCP_nextSingleOptionConst(opt);
  } while (opt < end_options);

  return (seen_end && opt==end_options);
}

static void *fillSuboption(struct DHCPSingleOption *out,
			   struct DHCPSubOption const *opt)
{
	out->code = opt->code;
	out->len  = opt->len;
	memcpy(out->data, opt->data, opt->len);

	return &out->data[opt->len];
}

struct DHCPOptionList {
	struct DHCPSingleOption const	*start;
	struct DHCPSingleOption		*cursor;
	size_t				len;
};

static void validateDHCPOptionList(struct DHCPOptionList const *options)
{
  assert(options->cursor >= options->start);
  assert(options->len >= 1u);
  assert((uintptr_t)(options->cursor) <
	 (uintptr_t)(options->start) + options->len);
}

static void
addAgentOption(/*@in@*/struct InterfaceInfo const * const	iface,
	       struct DHCPOptionList				*options)
{
  struct DHCPSubOption const	*subopt = &iface->suboptions.dta[0];
  struct DHCPSingleOption	*pos = options->cursor;
  size_t			len = options->len;
  size_t			i;
  size_t			opt_len;
  void				*ptr;

  /* Replace the end-tag
   *  - - - - - - -----
   * |           | END |
   *  - - - - - - -----
   * with
   *  - - - - - - ----- ----- ----- ----- ------------ -----
   * |           | 82  | len | sub | sub | ... id ... | END |
   * |           |     |     | opt | len |            |     |
   *  - - - - - - ----- ----- ----- ----- ------------ -----
   *                          \                     /
   *                           `   repeat x times  '
   * */

  validateDHCPOptionList(options);
  assert(pos->code == cdEND);

  /* no suboptions specified; exit immediately */
  if (iface->suboptions.len == 0)
    return;

  opt_len = 2;			       /* the two initial '82' + 'len' fields */
  for (i = 0; i < iface->suboptions.len; ++i)
    opt_len += 2 + subopt[i].len;

  /* 'len' should now have the length of the complete option-field. RFC 2131
   * sets a lower limit of 312 octets, so we are checking against this
   * value. Since the function got only the real options without the
   * magic-cookie, 4 octets must be added.
   *
   * Because the underlying buffer was declared to hold more than this
   * minimum amount, we can exclude overflows here.
   *
   * Further versions of this software should make it possible to configure
   * the maximum size at runtime. */
  if (len + opt_len + 4 >= 312 || opt_len > 255) {
    LOG("New DHCP packet would be too large; dropping configured suboptions");
    return;
  }

  pos->code = cdRELAY_AGENT;
  pos->len  = opt_len - 2;
  ptr       = pos->data;

  for (i = 0; i < iface->suboptions.len; ++i)
    ptr = fillSuboption(ptr, &subopt[i]);

  pos = DHCP_nextSingleOption(pos);
  assert(ptr == pos);

  pos->code = cdEND;

  options->len += opt_len;
  options->cursor = pos;

  validateDHCPOptionList(options);
}

static void removeAgentOption(struct DHCPOptionList *options)
    /*@requires (opt+1) <= end_opt@*/
    /*@modifies *opt@*/
{
  struct DHCPSingleOption const		*in = options->cursor;
  struct DHCPSingleOption		*out = options->cursor;
  size_t				len = options->len;

  validateDHCPOptionList(options);
  assert(in->code == cdRELAY_AGENT);

  do {
    struct DHCPSingleOption const	*next;
    size_t				l;

    /* content of 'in' can be overwritten in this loop; save the pointer to the
     * next option */
    next = DHCP_nextSingleOptionConst(in);
    l	 = DHCP_getOptionLength(in);

    if (in->code == cdRELAY_AGENT) {
      /* we have at least a 'cdEND' option so that 'len' should never become
	 zero*/
      assert(len > l);
      len -= l;
    } else {
      memmove(out, in, l);
      out = DHCP_nextSingleOption(out);
    }

    in = next;
  } while (in->code != cdEND);

  out->code = cdEND;

  options->len    = len;
  options->cursor = out;

  validateDHCPOptionList(options);
}

  /*@-mustmod@*/
static size_t
fillOptions(/*@in@*/struct InterfaceInfo const* const	iface,
	    /*@dependent@*/void				*option_ptr,
	    OptionFillAction				action)
    /*@modifies *option_ptr@*/
{
  /*@dependent@*/struct DHCPSingleOption	*opt       = option_ptr;
  /*@dependent@*/struct DHCPSingleOption	*end_opt   = 0;
  /*@dependent@*/struct DHCPSingleOption	*relay_opt = 0;
  struct DHCPOptionList		options = {
	  .start	= option_ptr,
  };

  do {
    switch (opt->code) {
      case cdRELAY_AGENT	:
	/* we are interested in the first relay agent option only */
	if (!relay_opt)
	  relay_opt = opt;
	break;
      case cdEND		:  end_opt   = opt; break;
      default			:  break;
    }

    opt = DHCP_nextSingleOption(opt);
  } while (end_opt==0);

  assert(static_cast(void*)(end_opt)>=option_ptr);
  assert(end_opt>=relay_opt || relay_opt==0);

    /* Determine used space until end-tag and add space for the end-tag itself
     * (1 octet). */
  options.len  = DHCP_ptrdiff(end_opt, option_ptr) + 1u;

  if (relay_opt)
    options.cursor = relay_opt;
  else
    options.cursor = end_opt;

  switch (action) {
    case acREMOVE_AGENT_INFO:
      if (relay_opt)
	removeAgentOption(&options);
      break;

    case acADD_AGENT_INFO:
    case acREPLACE_AGENT_INFO:
      if (!relay_opt) {
	addAgentOption(iface, &options);
      } else if (action == acREPLACE_AGENT_INFO) {
	removeAgentOption(&options);
	addAgentOption(iface, &options);
      } else {
	LOG("DCHP relay agent info already set");
      }

      break;

    case acIGNORE:
      break;

    default:
      assert(false);
  }

  return options.len;
}
  /*@=mustmod@*/

static uint16_t
calculateCheckSum(/*@in@*/void const * const	dta,
		  size_t size,
		  uint32_t sum)
    /*@*/
{
  size_t		i;
  uint8_t const		*data = dta;

  for (i=0; i+1 < size; i += 2)
    sum += (data[i] << 8) + data[i + 1];

  if (size%2 != 0)
    sum += data[size-1] << 8;

  while ( (sum>>16)!=0 )
    sum = (sum & 0xFFFF) + (sum >> 16);

  return sum;
}

static void
fixCheckSumIP(struct iphdr * const	ip)
    /*@modifies *ip@*/
{
  ip->check = 0;
  ip->check = htons(~calculateCheckSum(ip, sizeof(*ip), 0));
}

static void
fixCheckSumUDP(struct udphdr * const			udp,
	       /*@in@*/struct iphdr const * const	ip,
	       /*@in@*/void const * const		data)
    /*@modifies *udp@*/
{
  uint32_t		sum;
  struct {
      uint32_t		src;
      uint32_t		dst;
      uint8_t		mbz;
      uint8_t		proto;
      uint16_t		len;
  } __attribute__((__packed__))	pseudo_hdr;

  pseudo_hdr.src   = ip->saddr;
  pseudo_hdr.dst   = ip->daddr;
  pseudo_hdr.mbz   = 0;
  pseudo_hdr.proto = ip->protocol;
  pseudo_hdr.len   = udp->len;

  udp->check = 0;
  sum = calculateCheckSum(&pseudo_hdr, sizeof pseudo_hdr, 0);
  sum = calculateCheckSum(udp,         sizeof(*udp),      sum);
  sum = calculateCheckSum(data,        ntohs(udp->len)-sizeof(*udp), sum);

  sum = ~ntohs(sum);
  if (sum==0) sum=~sum;
  udp->check = sum;
}

static void
sendEtherFrame(/*@in@*/struct InterfaceInfo const	*iface,
	       /*@dependent@*/struct DHCPllPacket	*frame,
	       /*@dependent@*//*@in@*/char const	*buffer,
	       size_t					size)
    /*@globals internalState, fds@*/
    /*@modifies internalState, *frame@*/
{
  struct sockaddr_ll		sock;
  struct msghdr			msg;
  /*@temp@*/struct iovec	iovec_data[2];
    /*@-sizeoftype@*/
  size_t const			szUDPHDR = sizeof(struct udphdr);
    /*@=sizeoftype@*/

    /* We support ethernet only and the config-part shall return ethernet-macs
     * only... */
  assert(iface->if_maclen == sizeof(frame->eth.ether_dhost));

  memset(&sock, 0, sizeof sock);
  sock.sll_family    = static_cast(sa_family_t)(AF_PACKET);
  sock.sll_ifindex   = static_cast(int)(iface->if_idx);
    /* We do not need to initialize the other attributes of rcpt_sock since
     * dst-hwaddr et.al. are determined by the ethernet-frame defined below */

  memcpy(frame->eth.ether_shost, iface->if_mac,  iface->if_maclen);

  frame->ip.version  = 4u;
  frame->ip.ihl      = sizeof(frame->ip)/4u;
  frame->ip.tos      = 0;
  frame->ip.tot_len  = htons(sizeof(frame->ip) + szUDPHDR + size);
  frame->ip.id       = 0;
  frame->ip.frag_off = htons(IP_DF);
  frame->ip.ttl      = 64;
  frame->ip.protocol = IPPROTO_UDP;
  frame->ip.saddr    = iface->if_ip;

  frame->udp.len     = htons(szUDPHDR + size);

  fixCheckSumIP(&frame->ip);
  fixCheckSumUDP(&frame->udp, &frame->ip, buffer);

  iovec_data[0].iov_base = frame;
  iovec_data[0].iov_len  = sizeof(*frame);
  iovec_data[1].iov_base = const_cast(char *)(buffer);
  iovec_data[1].iov_len  = size;

  msg.msg_name       = &sock;
  msg.msg_namelen    = sizeof(sock);
  msg.msg_iov        = iovec_data;
  msg.msg_iovlen     = 2;
  msg.msg_control    = 0;
  msg.msg_controllen = 0;
  msg.msg_flags      = 0;

  assertDefined(msg.msg_iov);

  Wsendmsg(fds.raw_fd, &msg, 0);
}

static void
sendToClient(/*@in@*/struct FdInfo const * const	fd,
	     /*@in@*/struct DHCPHeader const * const	header,
	     /*@in@*//*@dependent@*/char const * const	buffer,
	     size_t const				size)
    /*@globals internalState, fds@*/
    /*@modifies internalState@*/
{
  struct DHCPllPacket		frame;
  struct InterfaceInfo const	*iface = fd->iface;

  assert(header->op   == opBOOTREPLY);

  memset(&frame, 0, sizeof frame);
  frame.eth.ether_type = htons(ETHERTYPE_IP);

  if (g_compat_hacks & (1Lu << COMPAT_HACK_CLIENT_ADDRESSING)) {
    /* \todo: when there are no complaints, remove me after 2015 */

    /* Check whether header contains an ethernet MAC or something else (e.g. a
     * PPP tag). In the first case send to this MAC, in the latter one, send a
     * ethernet-broadcast message */
    if (header->htype==ARPHRD_ETHER && header->hlen==ETH_ALEN)
      memcpy(frame.eth.ether_dhost, header->chaddr, sizeof frame.eth.ether_dhost);
    else
      memset(frame.eth.ether_dhost, 255,            sizeof frame.eth.ether_dhost);

    if ((header->flags&flgDHCP_BCAST)!=0 && header->ciaddr!=0)
      frame.ip.daddr  = header->ciaddr;
    else if (iface->allow_bcast)
      frame.ip.daddr  = INADDR_BROADCAST;
    else
      return;	//< \todo
  } else {
    /*
     * From RFC-2131:
     *   A server or relay agent sending or relaying a DHCP message directly
     *   to a DHCP client (i.e., not to a relay agent specified in the
     *   'giaddr' field) SHOULD examine the BROADCAST bit in the 'flags'
     *   field.  If this bit is set to 1, the DHCP message SHOULD be sent as
     *   an IP broadcast using an IP broadcast address (preferably 0xffffffff)
     *   as the IP destination address and the link-layer broadcast address as
     *   the link-layer destination address.  If the BROADCAST bit is cleared
     *   to 0, the message SHOULD be sent as an IP unicast to the IP address
     *   specified in the 'yiaddr' field and the link-layer address specified
     *   in the 'chaddr' field.  If unicasting is not possible, the message
     *   MAY be sent as an IP broadcast using an IP broadcast address
     *   (preferably 0xffffffff) as the IP destination address and the link-
     *   layer broadcast address as the link-layer destination address.
     */

    /* unicast if possible */
    if ((header->flags & flgDHCP_BCAST) == 0 &&
	header->yiaddr != 0 &&
	header->htype == ARPHRD_ETHER && header->hlen == ETH_ALEN) {
      memcpy(frame.eth.ether_dhost, header->chaddr, sizeof frame.eth.ether_dhost);
      frame.ip.daddr = header->yiaddr;
    } else if (iface->allow_bcast) {
      /* broadcast otherwise */
      memset(frame.eth.ether_dhost, 255, sizeof frame.eth.ether_dhost);
      frame.ip.daddr = INADDR_BROADCAST;
    } else
      return;	//< \todo
  }

  frame.udp.source  = htons(DHCP_PORT_SERVER);
  frame.udp.dest    = htons(DHCP_PORT_CLIENT);

  sendEtherFrame(iface, &frame, buffer, size);
}

static void
sendServerBcast(/*@in@*/struct ServerInfo const	* const		server,
		/*@dependent@*//*@in@*/char const * const	buffer,
		size_t const					size)
    /*@globals internalState, fds@*/
    /*@modifies internalState@*/
{
  struct DHCPllPacket		frame;
  struct InterfaceInfo const	*iface = server->iface;

  memset(&frame, 0, sizeof frame);
  memset(frame.eth.ether_dhost, 255, sizeof frame.eth.ether_dhost);

  frame.eth.ether_type = htons(ETHERTYPE_IP);

  frame.ip.daddr       = INADDR_BROADCAST;

  frame.udp.source     = htons(DHCP_PORT_CLIENT);
  frame.udp.dest       = htons(DHCP_PORT_SERVER);

  sendEtherFrame(iface, &frame, buffer, size);
}

static void
sendServerUnicast(/*@in@*/struct ServerInfo const * const	server,
		  /*@in@*/char const * const			buffer,
		  size_t const					size)
    /*@globals internalState@*/
    /*@modifies internalState@*/
{
  struct sockaddr_in	sock;

  memset(&sock, 0, sizeof sock);
  sock.sin_family = AF_INET;

  sock.sin_addr = server->info.unicast.ip;
  sock.sin_port = htons(DHCP_PORT_SERVER);

  Wsendto(server->info.unicast.fd, buffer, size, 0,
	  reinterpret_cast(struct sockaddr *)(&sock),
	  sizeof sock);
}

static void
sendToServer(/*@in@*//*@dependent@*/char const * const	buffer,
	     size_t const				size)
    /*@globals servers, fds, internalState@*/
    /*@modifies internalState@*/
{
    /*@-nullptrarith@*/
  struct ServerInfo const *		server;
  struct ServerInfo const * const	end_server = servers.dta+servers.len;
    /*@=nullptrarith@*/

  assert(servers.len==0 || servers.dta!=0);

  for (server=servers.dta; server<end_server; ++server) {
    assert(server!=0);

    switch (server->type) {
      case svUNICAST	:
	sendServerUnicast(server, buffer, size);
	break;
      case svBCAST	:
	sendServerBcast(server, buffer, size);
	break;
      default		:
	assert(false);
    }
  }
}

static void
handlePacket(/*@in@*/struct FdInfo const * const		fd,
	     /*@in@*/struct InterfaceInfo const * const		iface_orig,
	     /*@dependent@*/char * const			buffer,
	     size_t						size)
    /*@globals fds, servers, internalState@*/
    /*@modifies internalState@*/
{
  struct DHCPHeader * const	header  = reinterpret_cast(struct DHCPHeader *)(buffer);
  struct DHCPOptions * const	options = reinterpret_cast(struct DHCPOptions *)(&buffer[sizeof(*header)]);
  size_t			options_len = size - sizeof(*header);


    /* Discard broken header (e.g. too much hops or bad values) */
  if (!isValidHeader(header)) return;
  ++header->hops;

    /* Check if we are the first agent or if we handle a packet indented for us;
     * if so, set 'giaddr' and the relay-agent field, else do not touch the
     * packet */
  if (header->giaddr==0 || header->giaddr==fd->iface->if_ip) {
    header->giaddr = fd->iface->if_ip;

    if (!isValidOptions(options, options_len)) {
      LOG("Invalid options");
      return;
    }

      /* Do not fill the option-field if this field does not exists. */
    if (options_len!=0) {
      OptionFillAction		action;

      switch (header->op) {
	case opBOOTREPLY:	action = acREMOVE_AGENT_INFO; break;
	case opBOOTREQUEST:	action = acADD_AGENT_INFO;    break;
	default:		assert(false); action = acIGNORE; break;
      }

	/* Fill agent-info and adjust size-information */
      options_len  = fillOptions(fd->iface, options->data, action);
      options_len += sizeof(options->cookie);
      size         = options_len + sizeof(*header);
    }

    assert(isValidOptions(options, options_len));
  }

  switch (header->op) {
    case opBOOTREPLY	:
      if      (!iface_orig->has_servers) LOG("BOOTREPLY request from interface without servers");
      else if (!fd->iface->has_clients)  LOG("BOOTREPLY request for interface without clients");
      else sendToClient(fd, header, buffer, size);
      break;
    case opBOOTREQUEST	:
      if (!iface_orig->has_clients)      LOG("BOOTREQUEST request from interface without clients");
      else sendToServer(buffer, size);
      break;

	/* isValidHeader() checked the correctness of header->op already and it
	 * should be impossible to reach this code... */
    default		:  assert(false);
  }
}

/*@noreturn@*/
static void
execRelay()
    /*@globals fds, servers, internalState@*/
    /*@modifies internalState@*/
{
    /*@-sizeoftype@*/
  size_t const			szDHCPHDR = sizeof(struct DHCPHeader);
    /*@=sizeoftype@*/
  size_t const			max_mtu   = determineMaxMTU();
  size_t const			len_total = max_mtu + IFNAMSIZ + 4;
  char				*buffer   = static_cast(char *)(alloca(len_total));

  FatalErrnoError(buffer==0, 1, "alloca()");

  assert(fds.dta!=0 || fds.len==0);

  while (true) {
    fd_set			fdset;
    int				max;
      /*@-nullptrarith@*/
    struct FdInfo const *	fdinfo;
    struct FdInfo const * const end_fdinfo = fds.dta+fds.len;
      /*@=nullptrarith@*/

    fillFDSet(&fdset, &max);
    if /*@-type@*/(Wselect(max+1, &fdset, 0, 0, 0)==-1)/*@=type@*/ continue;

    for (fdinfo=fds.dta; fdinfo<end_fdinfo; ++fdinfo) {
      size_t				size;
      struct sockaddr_in		addr;
      struct in_pktinfo			pkinfo;
      int				flags = 0;

      assert(fdinfo!=0);

      if (!FD_ISSET(fdinfo->fd, &fdset)) /*@innercontinue@*/continue;

	/* Is this really correct? Can we receive fragmented IP datagrams being
	 * assembled larger than the MTU of the attached interfaces? */
      size = WrecvfromFlagsInet4(fdinfo->fd, buffer, len_total, &flags,
				 &addr, &pkinfo);

#ifdef WITH_LOGGING
      logDHCPPackage(buffer, size, &pkinfo, &addr);
#endif

      if (static_cast(ssize_t)(size)==-1) {
	char const *	msg = strerror(errno);

	LOG("recvfrom() failed");
	LOGSTR(msg);
      }
      else if (size < szDHCPHDR) {
	LOG("Malformed package");
      }
      else {
	struct InterfaceInfo const * const	iface_orig = fdinfo->iface;
	struct FdInfo const *			fd_real    = fdinfo;

	  /* Broadcast messages are designated for the interface, so lookup the
	   * "real" dest-interface for unicast-messages only.
	   *
	   * \todo: Can we setup the routing that datagrams from the server are
	   *        received by the giaddr-interface? */
	if (pkinfo.ipi_addr.s_addr!=INADDR_BROADCAST)
	  fd_real = lookupFD(pkinfo.ipi_addr);

	if (fd_real==0) LOG("Received package on unknown interface");
	else if (size > fd_real->iface->if_mtu) {
	  LOG("Unexpected large packet");
	}
	else
	  handlePacket(fd_real, iface_orig, buffer, size);
      }
    }
  }
}

int main(int argc, char *argv[])
    /*@globals fds, servers, internalState, fileSystem@*/
    /*@modifies fds, servers, internalState, fileSystem@*/
{
  struct InterfaceInfoList		ifs;

  checkCompileTimeAssertions();

    /* We create a mem-leak here, but this is not important because the parent
     * exits immediately and frees the resources and the child never exits */
    /*@-compdestroy@*//*@-superuser@*/
  switch (initializeSystem(argc, argv, &ifs, &servers, &fds)) {
    /*@=superuser@*/
    case -1	:  return 5;
    case 0	:  execRelay();
    default	:  return 0;
  }
    /*@=compdestroy@*/
}

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
