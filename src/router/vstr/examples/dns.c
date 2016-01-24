#define DNS_C
/*
 *  Copyright (C) 2004, 2005  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */
/* dns resolving, meant for async calls ... TCP only atm. */

#define VSTR_COMPILE_INCLUDE 1
#include <vstr.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

/* #define NDEBUG 1 -- not done via. configure atm. */
#include <assert.h>

#include <err.h>


#include <socket_poll.h>
#include <timer_q.h>

#include "dns.h"

#include "vlg.h"

#define TRUE 1
#define FALSE 0

#include "app.h"

#define MAP_MAKE_ENTRY(x) [ DNS_CLASS_ ## x ] = #x
static const char *dns_map_class2name[DNS_CLASS_ALL + 1] = {
 MAP_MAKE_ENTRY(IN),
 MAP_MAKE_ENTRY(CH),
 MAP_MAKE_ENTRY(HS),
 [DNS_CLASS_ALL] = "*"
};
#undef MAP_MAKE_ENTRY

#define MAP_MAKE_ENTRY(x) [ DNS_TYPE_IN_ ## x ] = #x
static const char *dns_map_type_in2name[DNS_TYPE_ALL + 1] = {
 MAP_MAKE_ENTRY(A),
 MAP_MAKE_ENTRY(NS),
 MAP_MAKE_ENTRY(CNAME),
 MAP_MAKE_ENTRY(SOA),
 MAP_MAKE_ENTRY(PTR),
 MAP_MAKE_ENTRY(MX),
 MAP_MAKE_ENTRY(TXT),
 MAP_MAKE_ENTRY(AAAA),
 MAP_MAKE_ENTRY(SRV),
 MAP_MAKE_ENTRY(AXFR),
 MAP_MAKE_ENTRY(IXFR),
 [DNS_TYPE_ALL] = "*"
};
#undef MAP_MAKE_ENTRY

#define MAP_MAKE_ENTRY(x) [ DNS_TYPE_CH_ ## x ] = #x
static const char *dns_map_type_ch2name[DNS_TYPE_ALL + 1] = {
 MAP_MAKE_ENTRY(A),
 MAP_MAKE_ENTRY(TXT),
 [DNS_TYPE_ALL] = "*"
};
#undef MAP_MAKE_ENTRY

#define MAP_MAKE_ENTRY(x) [ DNS_HDR_R_ ## x ] = #x
static const char *dns_map_hdr_r2name[DNS_HDR_RSZ] = {
 MAP_MAKE_ENTRY(NONE),
 MAP_MAKE_ENTRY(BFMT),
 MAP_MAKE_ENTRY(SERV),
 MAP_MAKE_ENTRY(NAME),
 MAP_MAKE_ENTRY(NSUP),
 MAP_MAKE_ENTRY(REFU),
};
#undef MAP_MAKE_ENTRY

unsigned int dns_get_msg_len(Vstr_base *s1, size_t pos)
{
  if (s1->len < pos + 1)
    return (0);

  return (2 + get_b_uint16(s1, pos));
}

const char *dns_name_type_ch(unsigned int num)
{
  if (num > DNS_TYPE_ALL)
    return ("");

  if (!dns_map_type_ch2name[num])
    return ("");

  return (dns_map_type_ch2name[num]);
}

const char *dns_name_type_in(unsigned int num)
{
  if (num > DNS_TYPE_ALL)
    return ("");

  if (!dns_map_type_in2name[num])
    return ("");

  return (dns_map_type_in2name[num]);
}

const char *dns_name_class(unsigned int num)
{
  if (num > DNS_CLASS_ALL)
    return ("");

  if (!dns_map_class2name[num])
    return ("");

  return (dns_map_class2name[num]);
}

const char *dns_name_hdr_r(unsigned int num)
{
  if (num > DNS_HDR_RSZ)
    return ("");

  if (!dns_map_hdr_r2name[num])
    return ("");

  return (dns_map_hdr_r2name[num]);
}

static size_t dns_app_class_type(Vstr_base *out, Vstr_base *pkt,
                                 size_t pos, size_t msg_len,
                                 unsigned int *dns_class,
                                 unsigned int *dns_type)
{
  unsigned int cnum = 0;
  unsigned int tnum = 0;
  
  if (4 > vstr_sc_posdiff(pos, msg_len))
    return (msg_len);

  tnum = get_b_uint16(pkt, pos); pos += 2;
  cnum = get_b_uint16(pkt, pos); pos += 2;

  if (dns_class) *dns_class = cnum;
  if (dns_type)  *dns_type  = tnum;
  
  if (out)
  {
    if (dns_name_class(cnum))
      app_cstr_buf(out, dns_name_class(cnum));
    else
      app_fmt(out, "%u", cnum);
    app_cstr_buf(out, ".");
  }
  
  if (out) switch (cnum)
  {
    case DNS_CLASS_IN:
      app_cstr_buf(out, dns_name_type_in(tnum));
      break;
    case DNS_CLASS_CH:
      app_cstr_buf(out, dns_name_type_ch(tnum));
      break;
    case DNS_CLASS_ALL:
      if (tnum == DNS_TYPE_ALL)
      {
        app_cstr_buf(out, "*");
        break;
      }
      /* FALL THROUGH */
    default:
      app_fmt(out, "%u", tnum);
      break;
  }

  return (pos);
}

static size_t dns_app_txt(Vstr_base *out,
                          Vstr_base *pkt, size_t pos, size_t msg_len)
{
  unsigned char tmp = 0;
  
  while ((pos < msg_len) && (tmp = vstr_export_chr(pkt, pos)))
  {
    ++pos;
    if (tmp > vstr_sc_posdiff(pos, msg_len))
      tmp = vstr_sc_posdiff(pos, msg_len);
    if (out) app_fmt(out, "${vstr:%p%zu%zu%u}",
                     pkt, pos, tmp, VSTR_TYPE_ADD_ALL_BUF);
    pos += tmp;
  }
  if (pos <= msg_len)
    ++pos;

  return (pos);
}

#define OUT_EQ_VLOG() (out == base->io_dbg->out_vstr)

static size_t dns_app_label(Dns_base *base, Vstr_base *out,
                            Vstr_base *pkt, size_t pos, size_t msg_len)
{
  unsigned char tmp = 0;
  
  while ((pos < msg_len) && (tmp = vstr_export_chr(pkt, pos)))
  {
    if (DNS_LABEL_IS_PTR(tmp))
    { /* ptr */
      if (OUT_EQ_VLOG()) app_cstr_buf(out, " <BAD END>");
      return (msg_len);
    }

    /* label */
    ++pos;
    if (tmp > vstr_sc_posdiff(pos, msg_len))
      tmp = vstr_sc_posdiff(pos, msg_len);
    if (out) app_fmt(out, "${vstr:%p%zu%zu%u}.",
                     pkt, pos, tmp, VSTR_TYPE_ADD_ALL_BUF);
    pos += tmp;
  }
  if (pos <= msg_len)
    ++pos;

  return (pos);
}

static size_t dns_app_name(Dns_base *base, Vstr_base *out,
                           Vstr_base *pkt, size_t pos, size_t msg_len)
{
  size_t orig_pos = pos;
  unsigned char tmp = 0;
  
  while ((pos < msg_len) && (tmp = vstr_export_chr(pkt, pos)))
  {
    if (DNS_LABEL_IS_PTR(tmp))
    {
      unsigned int off = get_b_uint16(pkt, pos);

      off &= ~0xC000; /* remove top bits that specifiy it's a ptr */
      ++off; /* offset is 0 indexed, position is 1 indexed */
      
      if ((off != orig_pos) && (off < pos))
        dns_app_name(base, out, pkt, off, msg_len);
      else if (OUT_EQ_VLOG()) app_cstr_buf(out, " <BAD END>");
      
      return (pos + 2);
    }

    ++pos;
    if (tmp > vstr_sc_posdiff(pos, msg_len))
      tmp = vstr_sc_posdiff(pos, msg_len);
    if (out) app_fmt(out, "${vstr:%p%zu%zu%u}.",
                     pkt, pos, tmp, VSTR_TYPE_ADD_ALL_BUF);
    pos += tmp;
  }
  if (pos <= msg_len)
    ++pos;

  return (pos);
}

static size_t dns_app_ttl(Vstr_base *out,
                          Vstr_base *pkt, size_t pos, size_t msg_len)
{
  unsigned int num = 0;
  
  if (4 > vstr_sc_posdiff(pos, msg_len))
    return (msg_len);
  
  num = get_b_uint32(pkt, pos);
  if (out) app_fmt(out, " for %ud %02u:%02u:%02u",
                   (num / (1 * 60 * 60 * 24)),
                   (num / (1 * 60 * 60)) % 24,
                   (num / (1 * 60)) % 60,
                   (num / (1)) % 60);
  
  return (pos + 4);
}

static size_t dns_app_rr_unknown_data(Dns_base *base,
                                      Vstr_base *pkt, size_t pos,
                                      size_t msg_len, unsigned int len)
{
  (void)pkt;
  
  vlg_dbg2(base->io_dbg, "  RD: %u %zu\n", len, vstr_sc_posdiff(pos, msg_len));
  
  if (len <= vstr_sc_posdiff(pos, msg_len))
    return (pos + len);
  
  return (msg_len);
}

static size_t dns_app_rr_data(Dns_base *base, Vstr_base *out,
                              Vstr_base *pkt, size_t pos, size_t msg_len,
                              unsigned int dns_class,
                              unsigned int dns_type)
{
  unsigned int len = 0;
  
  if (2 > vstr_sc_posdiff(pos, msg_len))
    return (msg_len);

  len = get_b_uint16(pkt, pos); pos += 2;
  
  if (!len)
    return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));

  if (len > (vstr_sc_posdiff(pos, msg_len)))
    return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));

  if ((dns_class != DNS_CLASS_IN) && (dns_class != DNS_CLASS_CH))
    return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));

  msg_len = vstr_sc_poslast(pos, len);
  if (dns_class == DNS_CLASS_CH)
  {
    if (0) { }
    else if (dns_type == DNS_TYPE_CH_A)
    {
      unsigned int num  = 0;

      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  NAME: ");
      pos = dns_app_name(base, out, pkt, pos, msg_len);
      
      if (2 > vstr_sc_posdiff(pos, msg_len))
        return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));
      
      num  = get_b_uint16(pkt, pos); pos += 2;
      if (OUT_EQ_VLOG()) app_cstr_buf(out, " A: ");
      if (out) app_fmt(out, " %u", num);
    }
    else if (dns_type == DNS_TYPE_CH_TXT)
    {
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  TXT: ");
      pos = dns_app_txt(out, pkt, pos, msg_len);
    }
    else
      return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));
  }
  
  if (dns_class == DNS_CLASS_IN)
  {
    if (0) { }
    else if (dns_type == DNS_TYPE_IN_A)
    {
      unsigned char buf[4];

      if (len != 4)
        return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));
      
      vstr_export_buf(pkt, pos, 4, buf, sizeof(buf)); pos += 4;
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  A: ");
      if (out) app_fmt(out, "%u.%u.%u.%u", buf[0], buf[1], buf[2], buf[3]);
    }
    else if (dns_type == DNS_TYPE_IN_NS)
    {
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  NS: ");
      pos = dns_app_name(base, out, pkt, pos, msg_len);
    }
    else if (dns_type == DNS_TYPE_IN_CNAME)
    {
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  CNAME: ");
      pos = dns_app_name(base, out, pkt, pos, msg_len);
    }
    else if (dns_type == DNS_TYPE_IN_SOA)
    {
      unsigned int num_serial  = 0;
      unsigned int num_refresh = 0;
      unsigned int num_retry   = 0;
      unsigned int num_expire  = 0;
      unsigned int num_min     = 0;
      
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  SOA:\n");
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "    NS: ");
      pos = dns_app_name(base, out, pkt, pos, msg_len);
      if (OUT_EQ_VLOG())
        app_cstr_buf(out, "\n");
      else if (out)
        app_cstr_buf(out, " ");
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "    ROOT: ");
      pos = dns_app_name(base, out, pkt, pos, msg_len);
      if (OUT_EQ_VLOG())
        app_cstr_buf(out, "\n");
      else if (out)
        app_cstr_buf(out, " ");
      
      if (16 > vstr_sc_posdiff(pos, msg_len))
        return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));
      
      num_serial  = get_b_uint32(pkt, pos); pos += 4;
      num_refresh = get_b_uint32(pkt, pos); pos += 4;
      num_retry   = get_b_uint32(pkt, pos); pos += 4;
      num_expire  = get_b_uint32(pkt, pos); pos += 4;
      num_min     = get_b_uint32(pkt, pos); pos += 4;
      if (OUT_EQ_VLOG())
        app_fmt(out, "    SERIAL: %u REFRESH: %u"
                " RETRY: %u EXPIRE: %u MIN: %u",
                num_serial, num_refresh, num_retry, num_expire, num_min);
      else if (out)
        app_fmt(out, " %u %u %u %u %u",
                num_serial, num_refresh, num_retry, num_expire, num_min);
    }
    else if (dns_type == DNS_TYPE_IN_PTR)
    {
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  PTR: ");
      pos = dns_app_name(base, out, pkt, pos, msg_len);
    }
    else if (dns_type == DNS_TYPE_IN_HINFO)
    {
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  CPU: ");
      pos = dns_app_txt(out, pkt, pos, msg_len);
      if (out) app_cstr_buf(out, " ");
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "OS: ");
      pos = dns_app_txt(out, pkt, pos, msg_len);
    }
    else if (dns_type == DNS_TYPE_IN_MX)
    {
      unsigned int num = 0;

      if (len < 4)
        return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));
      
      num = get_b_uint16(pkt, pos); pos += 2;
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  PREF: ");
      if (out) app_fmt(out, "%u ", num);
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "NAME: ");
      pos = dns_app_name(base, out, pkt, pos, msg_len);
    }
    else if (dns_type == DNS_TYPE_IN_TXT)
    {
      if (OUT_EQ_VLOG()) app_cstr_buf(out, "  TXT: ");
      pos = dns_app_txt(out, pkt, pos, msg_len);
    }
    else if (dns_type == DNS_TYPE_IN_SRV)
    {
      unsigned int num_pri = 0;
      unsigned int num_weight = 0;
      unsigned int num_port = 0;

      if (len < 8)
        return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));
      
      num_pri    = get_b_uint16(pkt, pos); pos += 2;
      num_weight = get_b_uint16(pkt, pos); pos += 2;
      num_port   = get_b_uint16(pkt, pos); pos += 2;
      
      if (OUT_EQ_VLOG())
        app_fmt(out, "    PRI: %u WEIGHT: %u PORT: %u NAME: ",
                num_pri, num_weight, num_port);
      else if (out)
        app_fmt(out, " %u %u %u ",
                num_pri, num_weight, num_port);
      
      pos = dns_app_name(base, out, pkt, pos, msg_len);
    }
    else
      return (dns_app_rr_unknown_data(base, pkt, pos, msg_len, len));
  }
  
  return (pos);
}

void dns_dbg_prnt_pkt(Dns_base *base, Vstr_base *pkt)
{
  size_t pos = 1;
  unsigned int id = 0;
  unsigned int flags = 0;
  unsigned int qdc = 0;
  unsigned int anc = 0;
  unsigned int nsc = 0;
  unsigned int arc = 0;
  unsigned int scan= 0;
  const size_t msg_len = pkt->len;
  int prefix = FALSE;
  
  if (!base->io_dbg->out_dbg)
    return;

  prefix = vlg_prefix_set(base->io_dbg, FALSE);
  
  if (12 <= msg_len)
  {
    id    = get_b_uint16(pkt, pos); pos += 2;
    flags = get_b_uint16(pkt, pos); pos += 2;
    qdc   = get_b_uint16(pkt, pos); pos += 2;
    anc   = get_b_uint16(pkt, pos); pos += 2;
    nsc   = get_b_uint16(pkt, pos); pos += 2;
    arc   = get_b_uint16(pkt, pos); pos += 2;
    
    vlg_dbg1(base->io_dbg, " id=%u\n", id);
    vlg_dbg1(base->io_dbg, " %*s: op=%u |%s|%s|%s|%s| z=%d ret=%d ->"
             " qd=%u an=%u ns=%u ar=%u\n",
             (int)strlen("Response"),
             ((flags & DNS_HDR_QR) ? "Response" : "Query"),
             ((flags & DNS_HDR_OPCMASK) >> DNS_HDR_OPCOFF),
             ((flags & DNS_HDR_AA) ? "AA" : "  "),
             ((flags & DNS_HDR_TC) ? "TC" : "  "),
             ((flags & DNS_HDR_RD) ? "RD" : "  "),
             ((flags & DNS_HDR_RA) ? "RA" : "  "),
             ((flags & DNS_HDR_ZMASK) >> DNS_HDR_ZOFF),
             ((flags & DNS_HDR_RMASK) >> DNS_HDR_ROFF),
             qdc, anc, nsc, arc);
  }
  else
    vlg_dbg1(base->io_dbg, "<NO HDR>\n");
  
  scan = 0;
  while ((scan++ < qdc) && (6 <= vstr_sc_posdiff(pos, msg_len)))
  {
    vlg_dbg1(base->io_dbg, " QUERY(%u/%u): ", scan, qdc);
    pos = dns_app_label(base, base->io_dbg->out_vstr, pkt, pos, msg_len);
    vlg_dbg1(base->io_dbg, " ");
    pos = dns_app_class_type(base->io_dbg->out_vstr, pkt, pos, msg_len,
                             NULL, NULL);
    vlg_dbg1(base->io_dbg, "\n");
  }
      
  scan = 0;
  while ((scan++ < (anc + nsc + arc)) && (12 <= vstr_sc_posdiff(pos, msg_len)))
  {
    unsigned int dns_class = 0;
    unsigned int dns_type  = 0;

    if (0) { }
    else if (anc && (scan <= (anc)))
      vlg_dbg1(base->io_dbg, " AN-RR(%u/%u): ", scan, anc);
    else if (nsc && (scan <= (anc + nsc)))
      vlg_dbg1(base->io_dbg, " NS-RR(%u/%u): ", scan - anc, nsc);
    else
      vlg_dbg1(base->io_dbg, " AR-RR(%u/%u): ", scan - (anc + nsc), arc);
    pos = dns_app_name(base, base->io_dbg->out_vstr, pkt, pos, msg_len);
    vlg_dbg1(base->io_dbg, " ");
    pos = dns_app_class_type(base->io_dbg->out_vstr, pkt, pos, msg_len,
                             &dns_class, &dns_type);
    pos = dns_app_ttl(base->io_dbg->out_vstr, pkt, pos, msg_len);
    vlg_dbg1(base->io_dbg, "\n");
    pos = dns_app_rr_data(base, base->io_dbg->out_vstr, pkt, pos, msg_len,
                          dns_class, dns_type);
    vlg_dbg1(base->io_dbg, "\n");
  }
  
  vlg_prefix_set(base->io_dbg, prefix);
}

void dns_app_recq_pkt(Dns_base *base, unsigned int qcount, ...)
{
  Vstr_base *io_w = base->io_w_serv;
  va_list ap;
  size_t pos1 = 0;
  size_t len1 = 0;
  unsigned int id = 0;
  Vstr_base *s1 = vstr_make_base(io_w->conf);
  size_t srch_pos = 0;
  size_t srch_len = 0;
  
  if (!s1)
    errno = ENOMEM, err(EXIT_FAILURE, __func__);

  pos1 = io_w->len + 1;
  app_b_uint16(io_w, 0); /* TCP length */

  id = rand(); id &= 0xFFFF;
  app_b_uint16(io_w, id);
  app_b_uint16(io_w, DNS_HDR_OPC_QUERY | (base->opt_recur ? DNS_HDR_RD : 0));
  
  app_b_uint16(io_w, qcount);
  app_b_uint16(io_w, 0);
  app_b_uint16(io_w, 0);
  app_b_uint16(io_w, 0);

  va_start(ap, qcount);
  while (qcount--)
  {
    const char *name       = va_arg(ap, const char *);
    unsigned int dns_class = va_arg(ap, unsigned int);
    unsigned int dns_type  = va_arg(ap, unsigned int);

    assert(name);

    vstr_sub_cstr_ptr(s1, 1, s1->len, name);

    if ((dns_class == DNS_CLASS_IN) && (dns_type == DNS_TYPE_IN_PTR))
    { /* magic remapping for ptr */
      unsigned char ipv4[4];
      unsigned int  ipv6[8];
      unsigned int ern = 0;
      
      if (0) { }
      else if (vstr_parse_ipv4(s1, 1, s1->len, ipv4, NULL,
                               VSTR_FLAG_PARSE_IPV4_FULL |
                               VSTR_FLAG_PARSE_IPV4_ONLY, NULL, &ern) && !ern)
      {
        vstr_del(s1, 1, s1->len);
        vstr_add_fmt(s1, s1->len, "%u.%u.%u.%u.in-addr.arpa",
                     ipv4[3], ipv4[2], ipv4[1], ipv4[0]);
      }
      else if (vstr_parse_ipv6(s1, 1, s1->len, ipv6, NULL,
                               VSTR_FLAG_PARSE_IPV6_ONLY, NULL, &ern) && !ern)
      {
        vstr_del(s1, 1, s1->len);
# define IP6_INT2BYTES(x)                                           \
        (ipv6[(x)] >>  0) & 0xF,                                    \
        (ipv6[(x)] >>  4) & 0xF,                                    \
        (ipv6[(x)] >>  8) & 0xF,                                    \
        (ipv6[(x)] >> 12) & 0xF
        vstr_add_fmt(s1, s1->len,
                     "%x.%x.%x.%x." "%x.%x.%x.%x."
                     "%x.%x.%x.%x." "%x.%x.%x.%x."
                     "%x.%x.%x.%x." "%x.%x.%x.%x."
                     "%x.%x.%x.%x." "%x.%x.%x.%x."
                     "ip6.int",
                     IP6_INT2BYTES(7), IP6_INT2BYTES(6),                     
                     IP6_INT2BYTES(5), IP6_INT2BYTES(4),                     
                     IP6_INT2BYTES(3), IP6_INT2BYTES(2),                     
                     IP6_INT2BYTES(1), IP6_INT2BYTES(0));

# undef IP6_INT2BYTES
      }
      
    }
    
    /* question */
    srch_pos = vstr_csrch_cstr_chrs_fwd(s1, 1, s1->len, ".");
    srch_len = vstr_sc_posdiff(srch_pos, s1->len);
    while (srch_len)
    {
      size_t difflen = vstr_cspn_cstr_chrs_fwd(s1, srch_pos, srch_len, ".");
      
      if (!difflen) /* ignore spurious '.' */
        difflen = vstr_spn_cstr_chrs_fwd(s1, srch_pos, srch_len, ".");
      else
      {
        app_b_uint8(io_w, difflen);
        app_vstr(io_w, s1, srch_pos, difflen, VSTR_TYPE_ADD_ALL_BUF);
        
        if (difflen != srch_len)
          ++difflen;
      }
      assert(difflen <= srch_len);
      
      srch_len -= difflen;
      srch_pos += difflen;
    }
    app_b_uint8(io_w, 0); /* 0 length label is terminator */

    app_b_uint16(io_w, dns_type);
    app_b_uint16(io_w, dns_class);
  }
  va_end(ap);
  
  if (io_w->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, __func__);

  /* make the lengths correct */
  len1 = io_w->len - (pos1 - 1);
  sub_b_uint16(io_w, pos1, len1 - 2);
  
  vlg_dbg1(base->io_dbg,
           "\n${rep_chr:%c%zu} send ${BKMG.u:%u} ${rep_chr:%c%zu}\n",
           '=', 33, len1 - 2, '=', 33);
  if (base->io_dbg->out_dbg >= 1)
    vstr_sub_vstr(s1, 1, s1->len,
                  io_w, pos1 + 2, len1 - 2, VSTR_TYPE_ADD_BUF_PTR);
  dns_dbg_prnt_pkt(base, s1);
  vlg_dbg1(base->io_dbg, "\n${rep_chr:%c%zu}\n", '-', 79);

  vstr_free_base(s1);
  io_w->conf->malloc_bad = FALSE;
}

void dns_sc_ui_out(Dns_base *base, Vstr_base *pkt)
{
  Vstr_base *io_w = base->io_w_user;
  size_t pos = 1;
  unsigned int id = 0;
  unsigned int flags = 0;
  unsigned int qdc = 0;
  unsigned int anc = 0;
  unsigned int nsc = 0;
  unsigned int arc = 0;
  unsigned int scan = 0;
  const size_t msg_len = pkt->len;
  
  if (12 <= msg_len)
  {
    id    = get_b_uint16(pkt, pos); pos += 2;
    flags = get_b_uint16(pkt, pos); pos += 2;
    qdc   = get_b_uint16(pkt, pos); pos += 2;
    anc   = get_b_uint16(pkt, pos); pos += 2;
    nsc   = get_b_uint16(pkt, pos); pos += 2;
    arc   = get_b_uint16(pkt, pos); pos += 2;
  }

  /* can't check id atm. */
  if (!(flags & DNS_HDR_QR))
    return;

  if (!anc)
  {
    scan = 0;
    while ((scan++ < qdc) && (6 <= vstr_sc_posdiff(pos, msg_len)))
    {
      unsigned int hdr_r_code = ((flags & DNS_HDR_RMASK) >> DNS_HDR_ROFF);
      
      pos = dns_app_label(base, io_w, pkt, pos, msg_len);
      app_cstr_buf(io_w, " ");
      pos = dns_app_class_type(io_w, pkt, pos, msg_len, NULL, NULL);
      app_fmt(io_w, ":ret=%d#%s\n", hdr_r_code, dns_name_hdr_r(hdr_r_code));
    }
    return;
  }

  scan = 0;
  while ((scan++ < qdc) && (6 <= vstr_sc_posdiff(pos, msg_len)))
  {
    pos = dns_app_label(base, NULL, pkt, pos, msg_len);
    pos = dns_app_class_type(NULL, pkt, pos, msg_len, NULL, NULL);
  }
  
  scan = 0;
  while ((scan++ < anc) && (12 <= vstr_sc_posdiff(pos, msg_len)))
  {
    unsigned int dns_class = 0;
    unsigned int dns_type  = 0;

    pos = dns_app_name(base, io_w, pkt, pos, msg_len);
    app_cstr_buf(io_w, " ");
    pos = dns_app_class_type(io_w, pkt, pos, msg_len, &dns_class, &dns_type);
    app_cstr_buf(io_w, " ");
    pos = dns_app_ttl(NULL, pkt, pos, msg_len);
    pos = dns_app_rr_data(base, io_w, pkt, pos, msg_len, dns_class, dns_type);
    app_cstr_buf(io_w, "\n");
  }
}

