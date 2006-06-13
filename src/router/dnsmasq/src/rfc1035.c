/* dnsmasq is Copyright (c) 2000 - 2005 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

static int add_resource_record(HEADER *header, char *limit, int *truncp, 
			       unsigned int nameoffset, unsigned char **pp, 
			       unsigned long ttl, unsigned int *offset, unsigned short type, 
			       unsigned short class, char *format, ...);

static int extract_name(HEADER *header, size_t plen, unsigned char **pp, 
			char *name, int isExtract)
{
  unsigned char *cp = (unsigned char *)name, *p = *pp, *p1 = NULL;
  unsigned int j, l, hops = 0;
  int retvalue = 1;
  
  if (isExtract)
    *cp = 0;

  while ((l = *p++))
    {
      unsigned int label_type = l & 0xc0;
      if (label_type == 0xc0) /* pointer */
	{ 
	  if ((size_t)(p - (unsigned char *)header + 1) >= plen)
	    return 0;
	      
	  /* get offset */
	  l = (l&0x3f) << 8;
	  l |= *p++;
	  if (l >= plen) 
	    return 0;
	  
	  if (!p1) /* first jump, save location to go back to */
	    p1 = p;
	      
	  hops++; /* break malicious infinite loops */
	  if (hops > 255)
	    return 0;
	  
	  p = l + (unsigned char *)header;
	}
      else if (label_type == 0x80)
	return 0; /* reserved */
      else if (label_type == 0x40)
	{ /* ELT */
	  unsigned int count, digs;
	  
	  if ((l & 0x3f) != 1)
	    return 0; /* we only understand bitstrings */

	  if (!isExtract)
	    return 0; /* Cannot compare bitsrings */
	  
	  count = *p++;
	  if (count == 0)
	    count = 256;
	  digs = ((count-1)>>2)+1;
	  
	  /* output is \[x<hex>/siz]. which is digs+9 chars */
	  if (cp - (unsigned char *)name + digs + 9 >= MAXDNAME)
	    return 0;
	  if ((size_t)(p - (unsigned char *)header + ((count-1)>>3) + 1) >= plen)
	    return 0;

	  *cp++ = '\\';
	  *cp++ = '[';
	  *cp++ = 'x';
	  for (j=0; j<digs; j++)
	    {
	      unsigned int dig;
	      if (j%2 == 0)
		dig = *p >> 4;
	      else
		dig = *p++ & 0x0f;
	      
	      *cp++ = dig < 10 ? dig + '0' : dig + 'A' - 10;
	    } 
	  cp += sprintf((char *)cp, "/%d]", count);
	  /* do this here to overwrite the zero char from sprintf */
	  *cp++ = '.';
	}
      else 
	{ /* label_type = 0 -> label. */
	  if (cp - (unsigned char *)name + l + 1 >= MAXDNAME)
	    return 0;
	  if ((size_t)(p - (unsigned char *)header + 1) >= plen)
	    return 0;
	  for(j=0; j<l; j++, p++)
	    if (isExtract)
	      {
		if (legal_char(*p))
		  *cp++ = *p;
		else
		  return 0;
	      }
	    else 
	      {
		unsigned char c1 = *cp, c2 = *p;
		
		if (c1 == 0)
		  retvalue = 2;
		else 
		  {
		    cp++;
		    if (c1 >= 'A' && c1 <= 'Z')
		      c1 += 'a' - 'A';
		    if (c2 >= 'A' && c2 <= 'Z')
		      c2 += 'a' - 'A';
		    
		    if (c1 != c2)
		      retvalue =  2;
		  }
	      }
	  
	  if (isExtract)
	    *cp++ = '.';
	  else if (*cp != 0 && *cp++ != '.')
	    retvalue = 2;
	}
      
      if ((unsigned int)(p - (unsigned char *)header) >= plen)
	return 0;
    }

  if (isExtract)
    {
      if (cp != (unsigned char *)name)
	cp--;
      *cp = 0; /* terminate: lose final period */
    }
  else if (*cp != 0)
    retvalue = 2;
    
  if (p1) /* we jumped via compression */
    *pp = p1;
  else
    *pp = p;

  return retvalue;
}
 
/* Max size of input string (for IPv6) is 75 chars.) */
#define MAXARPANAME 75
static int in_arpa_name_2_addr(char *namein, struct all_addr *addrp)
{
  int j;
  char name[MAXARPANAME+1], *cp1;
  unsigned char *addr = (unsigned char *)addrp;
  char *lastchunk = NULL, *penchunk = NULL;
  
  if (strlen(namein) > MAXARPANAME)
    return 0;

  memset(addrp, 0, sizeof(struct all_addr));

  /* turn name into a series of asciiz strings */
  /* j counts no of labels */
  for(j = 1,cp1 = name; *namein; cp1++, namein++)
    if (*namein == '.')
      {
	penchunk = lastchunk;
        lastchunk = cp1 + 1;
	*cp1 = 0;
	j++;
      }
    else
      *cp1 = *namein;
  
  *cp1 = 0;

  if (j<3)
    return 0;

  if (hostname_isequal(lastchunk, "arpa") && hostname_isequal(penchunk, "in-addr"))
    {
      /* IP v4 */
      /* address arives as a name of the form
	 www.xxx.yyy.zzz.in-addr.arpa
	 some of the low order address octets might be missing
	 and should be set to zero. */
      for (cp1 = name; cp1 != penchunk; cp1 += strlen(cp1)+1)
	{
	  /* check for digits only (weeds out things like
	     50.0/24.67.28.64.in-addr.arpa which are used 
	     as CNAME targets according to RFC 2317 */
	  char *cp;
	  for (cp = cp1; *cp; cp++)
	    if (!isdigit((int)*cp))
	      return 0;
	  
	  addr[3] = addr[2];
	  addr[2] = addr[1];
	  addr[1] = addr[0];
	  addr[0] = atoi(cp1);
	}

      return F_IPV4;
    }
#ifdef HAVE_IPV6
  else if (hostname_isequal(penchunk, "ip6") && 
	   (hostname_isequal(lastchunk, "int") || hostname_isequal(lastchunk, "arpa")))
    {
      /* IP v6:
         Address arrives as 0.1.2.3.4.5.6.7.8.9.a.b.c.d.e.f.ip6.[int|arpa]
    	 or \[xfedcba9876543210fedcba9876543210/128].ip6.[int|arpa]
      
	 Note that most of these the various reprentations are obsolete and 
	 left-over from the many DNS-for-IPv6 wars. We support all the formats
	 that we can since there is no reason not to.
      */

      if (*name == '\\' && *(name+1) == '[' && 
	  (*(name+2) == 'x' || *(name+2) == 'X'))
	{	  
	  for (j = 0, cp1 = name+3; *cp1 && isxdigit(*cp1) && j < 32; cp1++, j++)
	    {
	      char xdig[2];
	      xdig[0] = *cp1;
	      xdig[1] = 0;
	      if (j%2)
		addr[j/2] |= strtol(xdig, NULL, 16);
	      else
		addr[j/2] = strtol(xdig, NULL, 16) << 4;
	    }
	  
	  if (*cp1 == '/' && j == 32)
	    return F_IPV6;
	}
      else
	{
	  for (cp1 = name; cp1 != penchunk; cp1 += strlen(cp1)+1)
	    {
	      if (*(cp1+1) || !isxdigit((int)*cp1))
		return 0;
	      
	      for (j = sizeof(struct all_addr)-1; j>0; j--)
		addr[j] = (addr[j] >> 4) | (addr[j-1] << 4);
	      addr[0] = (addr[0] >> 4) | (strtol(cp1, NULL, 16) << 4);
	    }
	  
	  return F_IPV6;
	}
    }
#endif
  
  return 0;
}

static unsigned char *skip_name(unsigned char *ansp, HEADER *header, size_t plen)
{
  while(1)
    {
      unsigned int label_type = (*ansp) & 0xc0;
      
      if ((unsigned int)(ansp - (unsigned char *)header) >= plen)
	return NULL;
      
      if (label_type == 0xc0)
	{
	  /* pointer for compression. */
	  ansp += 2;	
	  break;
	}
      else if (label_type == 0x80)
	return NULL; /* reserved */
      else if (label_type == 0x40)
	{
	  /* Extended label type */
	  unsigned int count;
	  
	  if (((*ansp++) & 0x3f) != 1)
	    return NULL; /* we only understand bitstrings */
	  
	  count = *(ansp++); /* Bits in bitstring */
	  
	  if (count == 0) /* count == 0 means 256 bits */
	    ansp += 32;
	  else
	    ansp += ((count-1)>>3)+1;
	}
      else
	{ /* label type == 0 Bottom six bits is length */
	  unsigned int len = (*ansp++) & 0x3f;
	  if (len == 0)
	    break; /* zero length label marks the end. */
	  
	  ansp += len;
	}
    }
  
  return ansp;
}

static unsigned char *skip_questions(HEADER *header, size_t plen)
{
  int q, qdcount = ntohs(header->qdcount);
  unsigned char *ansp = (unsigned char *)(header+1);

  for (q = 0; q<qdcount; q++)
    {
      if (!(ansp = skip_name(ansp, header, plen)))
	return NULL;
      ansp += 4; /* class and type */
    }
  if ((unsigned int)(ansp - (unsigned char *)header) > plen) 
     return NULL;
  
  return ansp;
}

static unsigned char *skip_section(unsigned char *ansp, int count, HEADER *header, size_t plen)
{
  int i, rdlen;
  
  for (i = 0; i < count; i++)
    {
      if (!(ansp = skip_name(ansp, header, plen)))
	return NULL; 
      ansp += 8; /* type, class, TTL */
      GETSHORT(rdlen, ansp);
      if ((unsigned int)(ansp + rdlen - (unsigned char *)header) > plen) 
	return NULL;
      ansp += rdlen;
    }

  return ansp;
}

/* CRC the question section. This is used to safely detect query 
   retransmision and to detect answers to questions we didn't ask, which 
   might be poisoning attacks. Note that we decode the name rather 
   than CRC the raw bytes, since replies might be compressed differently. 
   We ignore case in the names for the same reason. */
unsigned int questions_crc(HEADER *header, size_t plen, char *name)
{
  int q;
  unsigned int crc = 0xffffffff;
  unsigned char *p1, *p = (unsigned char *)(header+1);

  for (q = 0; q < ntohs(header->qdcount); q++) 
    {
      if (!extract_name(header, plen, &p, name, 1))
	return crc; /* bad packet */
      
      for (p1 = (unsigned char *)name; *p1; p1++)
	{
	  int i = 8;
	  char c = *p1;

	  if (c >= 'A' && c <= 'Z')
	    c += 'a' - 'A';

	  crc ^= c << 24;
	  while (i--)
	    crc = crc & 0x80000000 ? (crc << 1) ^ 0x04c11db7 : crc << 1;
	}
      
      /* CRC the class and type as well */
      for (p1 = p; p1 < p+4; p1++)
	{
	  int i = 8;
	  crc ^= *p1 << 24;
	  while (i--)
	    crc = crc & 0x80000000 ? (crc << 1) ^ 0x04c11db7 : crc << 1;
	}

      p += 4;
      if ((unsigned int)(p - (unsigned char *)header) > plen)
	return crc; /* bad packet */
    }

  return crc;
}


size_t resize_packet(HEADER *header, size_t plen, unsigned char *pheader, size_t hlen)
{
  unsigned char *ansp = skip_questions(header, plen);
    
  if (!ansp)
    return 0;
  
  if (!(ansp = skip_section(ansp, ntohs(header->ancount) + ntohs(header->nscount) + ntohs(header->arcount),
			    header, plen)))
    return 0;
    
  /* restore pseudoheader */
  if (pheader && ntohs(header->arcount) == 0)
    {
      /* must use memmove, may overlap */
      memmove(ansp, pheader, hlen);
      header->arcount = htons(1);
      ansp += hlen;
    }

  return ansp - (unsigned char *)header;
}

unsigned char *find_pseudoheader(HEADER *header, size_t plen, size_t  *len, unsigned char **p)
{
  /* See if packet has an RFC2671 pseudoheader, and if so return a pointer to it. 
     also return length of pseudoheader in *len and pointer to the UDP size in *p */
  
  int i, arcount = ntohs(header->arcount);
  unsigned char *ansp;
  unsigned short rdlen, type;

  if (arcount == 0 || !(ansp = skip_questions(header, plen)))
    return NULL;
  
  if (!(ansp = skip_section(ansp, ntohs(header->ancount) + ntohs(header->nscount), header, plen)))
    return NULL; 
    
  for (i = 0; i < arcount; i++)
    {
      unsigned char *save, *start = ansp;
      if (!(ansp = skip_name(ansp, header, plen)))
	return NULL; 

      GETSHORT(type, ansp);
      save = ansp;
      ansp += 6; /* class, TTL */
      GETSHORT(rdlen, ansp);
      if ((size_t)(ansp + rdlen - (unsigned char *)header) > plen) 
	return NULL;
       ansp += rdlen;
       if (type == T_OPT)
	{
	  if (len)
	    *len = ansp - start;
	  if (p)
	    *p = save;
	  return start;
	}
    }
  
  return NULL;
}
      
  
/* is addr in the non-globally-routed IP space? */ 
static int private_net(struct all_addr *addrp) 
{
  struct in_addr addr = *(struct in_addr *)addrp;
  if (inet_netof(addr) == 0xA ||
      (inet_netof(addr) >= 0xAC10 && inet_netof(addr) < 0xAC20) ||
      (inet_netof(addr) >> 8) == 0xC0A8) 
    return 1;
  else 
    return 0;
}
 
static void dns_doctor(HEADER *header, struct doctor *doctor, struct in_addr *addr)
{
  for (; doctor; doctor = doctor->next)
    if (is_same_net(doctor->in, *addr, doctor->mask))
      {
	addr->s_addr &= ~doctor->mask.s_addr;
	addr->s_addr |= (doctor->out.s_addr & doctor->mask.s_addr);
	/* Since we munged the data, the server it came from is no longer authoritative */
	header->aa = 0;
	break;
      }
}

static int find_soa(HEADER *header, struct doctor *doctor, size_t qlen)
{
  unsigned char *p;
  int qtype, qclass, rdlen;
  unsigned long ttl, minttl = ULONG_MAX;
  int i, found_soa = 0;
  
  /* first move to NS section and find TTL from any SOA section */
  if (!(p = skip_questions(header, qlen)) ||
      !(p = skip_section(p, ntohs(header->ancount), header, qlen)))
    return 0; /* bad packet */
  
  for (i=0; i<ntohs(header->nscount); i++)
    {
      if (!(p = skip_name(p, header, qlen)))
	return 0; /* bad packet */
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlen, p);
      
      if ((qclass == C_IN) && (qtype == T_SOA))
	{
	  found_soa = 1;
	  if (ttl < minttl)
	    minttl = ttl;

	  /* MNAME */
	  if (!(p = skip_name(p, header, qlen)))
	    return 0;
	  /* RNAME */
	  if (!(p = skip_name(p, header, qlen)))
	    return 0;
	  p += 16; /* SERIAL REFRESH RETRY EXPIRE */
	  
	  GETLONG(ttl, p); /* minTTL */
	  if (ttl < minttl)
	    minttl = ttl;
	}
      else
	p += rdlen;
      
      if ((size_t)(p - (unsigned char *)header) > qlen)
	return 0; /* bad packet */
    }
 
  if (doctor)
    for (i=0; i<ntohs(header->arcount); i++)
      {
	if (!(p = skip_name(p, header, qlen)))
	  return 0; /* bad packet */
      
	GETSHORT(qtype, p); 
	GETSHORT(qclass, p);
	GETLONG(ttl, p);
	GETSHORT(rdlen, p);
	
	if ((qclass == C_IN) && (qtype == T_A))
	  dns_doctor(header, doctor, (struct in_addr *)p);
	
	p += rdlen;
	
	if ((size_t)(p - (unsigned char *)header) > qlen)
	  return 0; /* bad packet */
      }
 
  return found_soa ? minttl : 0;
}

/* Note that the following code can create CNAME chains that don't point to a real record,
   either because of lack of memory, or lack of SOA records.  These are treated by the cache code as 
   expired and cleaned out that way. */
void extract_addresses(HEADER *header, size_t qlen, char *name, time_t now, struct daemon *daemon)
{
  unsigned char *p, *p1, *endrr;
  int i, j, qtype, qclass, aqtype, aqclass, ardlen, res, searched_soa = 0;
  unsigned long ttl = 0;

  cache_start_insert();

  /* find_soa is needed for dns_doctor side-effects, so don't call it lazily if there are any. */
  if (daemon->doctors)
    {
      searched_soa = 1;
      ttl = find_soa(header, daemon->doctors, qlen);
    }
  
  /* go through the questions. */
  p = (unsigned char *)(header+1);
  
  for (i = 0; i<ntohs(header->qdcount); i++)
    {
      int found = 0, cname_count = 5;
      struct crec *cpp = NULL;
      int flags = header->rcode == NXDOMAIN ? F_NXDOMAIN : 0;
      unsigned long cttl = ULONG_MAX, attl;
      
      if (!extract_name(header, qlen, &p, name, 1))
	return; /* bad packet */
           
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      
      if (qclass != C_IN)
	continue;

      /* PTRs: we chase CNAMEs here, since we have no way to 
	 represent them in the cache. */
      if (qtype == T_PTR)
	{ 
	  struct all_addr addr;
	  int name_encoding = in_arpa_name_2_addr(name, &addr);
	  
	  if (!name_encoding)
	    continue;

	  if (!(flags & F_NXDOMAIN))
	    {
	    cname_loop:
	      if (!(p1 = skip_questions(header, qlen)))
		return;
	      
	      for (j = 0; j<ntohs(header->ancount); j++) 
		{
		  if (!(res = extract_name(header, qlen, &p1, name, 0)))
		    return; /* bad packet */
		  
		  GETSHORT(aqtype, p1); 
		  GETSHORT(aqclass, p1);
		  GETLONG(attl, p1);
		  GETSHORT(ardlen, p1);
		  endrr = p1+ardlen;
		  
		  /* TTL of record is minimum of CNAMES and PTR */
		  if (attl < cttl)
		    cttl = attl;

		  if (aqclass == C_IN && res != 2 && (aqtype == T_CNAME || aqtype == T_PTR))
		    {
		      if (!extract_name(header, qlen, &p1, name, 1))
			return;
		      
		      if (aqtype == T_CNAME)
			{
			  if (!cname_count--)
			    return; /* looped CNAMES */
			  goto cname_loop;
			}
		      
		      cache_insert(name, &addr, now, cttl, name_encoding | F_REVERSE);
		      found = 1; 
		    }
		  
		  p1 = endrr;
		  if ((size_t)(p1 - (unsigned char *)header) > qlen)
		    return; /* bad packet */
		}
	    }
	  
	   if (!found && !(daemon->options & OPT_NO_NEG))
	    {
	      if (!searched_soa)
		{
		  searched_soa = 1;
		  ttl = find_soa(header, NULL, qlen);
		}
	      if (ttl)
		cache_insert(name, &addr, now, ttl, name_encoding | F_REVERSE | F_NEG | flags);	
	    }
	}
      else
	{
	  /* everything other than PTR */
	  struct crec *newc;
	  
	  if (qtype == T_A)
	    flags |= F_IPV4;
#ifdef HAVE_IPV6
	  else if (qtype == T_AAAA)
	    flags |= F_IPV6;
#endif
	  else 
	    continue;
	    
	  if (!(flags & F_NXDOMAIN))
	    {
	    cname_loop1:
	      if (!(p1 = skip_questions(header, qlen)))
		return;
	      
	      for (j = 0; j<ntohs(header->ancount); j++) 
		{
		  if (!(res = extract_name(header, qlen, &p1, name, 0)))
		    return; /* bad packet */
		  
		  GETSHORT(aqtype, p1); 
		  GETSHORT(aqclass, p1);
		  GETLONG(attl, p1);
		  GETSHORT(ardlen, p1);
		  endrr = p1+ardlen;
		  
		  if (aqclass == C_IN && res != 2 && (aqtype == T_CNAME || aqtype == qtype))
		    {
		      if (aqtype == T_CNAME)
			{
			  if (!cname_count--)
			    return; /* looped CNAMES */
			  newc = cache_insert(name, NULL, now, attl, F_CNAME | F_FORWARD);
			  if (newc && cpp)
			    {
			      cpp->addr.cname.cache = newc;
			      cpp->addr.cname.uid = newc->uid;
			    }

			  cpp = newc;
			  if (attl < cttl)
			    cttl = attl;
			  
			  if (!extract_name(header, qlen, &p1, name, 1))
			    return;
			  goto cname_loop1;
			}
		      else
			{
			  found = 1;
			  if (aqtype == T_A)
			    dns_doctor(header, daemon->doctors, (struct in_addr *)p1);
			  newc = cache_insert(name, (struct all_addr *)p1, now, attl, flags | F_FORWARD);
			  if (newc && cpp)
			    {
			      cpp->addr.cname.cache = newc;
			      cpp->addr.cname.uid = newc->uid;
			    }
			  cpp = NULL;
			}
		    }
		  
		  p1 = endrr;
		  if ((size_t)(p1 - (unsigned char *)header) > qlen)
		    return; /* bad packet */
		}
	    }
	  
	  if (!found && !(daemon->options & OPT_NO_NEG))
	    {
	      if (!searched_soa)
		{
		  searched_soa = 1;
		  ttl = find_soa(header, NULL, qlen);
		}
	      /* If there's no SOA to get the TTL from, but there is a CNAME 
		 pointing at this, inherit it's TTL */
	      if (ttl || cpp)
		{
		  newc = cache_insert(name, (struct all_addr *)p, now, ttl ? ttl : cttl, F_FORWARD | F_NEG | flags);	
		  if (newc && cpp)
		    {
		      cpp->addr.cname.cache = newc;
		      cpp->addr.cname.uid = newc->uid;
		    }
		}
	    }
	}
    }
  
  cache_end_insert();
}

/* If the packet holds exactly one query
   return 1 and leave the name from the query in name. */

unsigned short extract_request(HEADER *header, size_t qlen, char *name, unsigned short *typep)
{
  unsigned char *p = (unsigned char *)(header+1);
  int qtype, qclass;

  if (typep)
    *typep = 0;

  if (ntohs(header->qdcount) != 1 || header->opcode != QUERY)
    return 0; /* must be exactly one query. */
  
  if (!extract_name(header, qlen, &p, name, 1))
    return 0; /* bad packet */
   
  GETSHORT(qtype, p); 
  GETSHORT(qclass, p);

  if (typep)
    *typep = qtype;

  if (qclass == C_IN)
    {
      if (qtype == T_A)
	return F_IPV4;
      if (qtype == T_AAAA)
	return F_IPV6;
      if (qtype == T_ANY)
	return  F_IPV4 | F_IPV6;
    }
  
  return F_QUERY;
}


size_t setup_reply(HEADER *header, size_t qlen,
		struct all_addr *addrp, unsigned short flags, unsigned long ttl)
{
  unsigned char *p = skip_questions(header, qlen);
  
  header->qr = 1; /* response */
  header->aa = 0; /* authoritive */
  header->ra = 1; /* recursion if available */
  header->tc = 0; /* not truncated */
  header->nscount = htons(0);
  header->arcount = htons(0);
  header->ancount = htons(0); /* no answers unless changed below */
  if (flags == F_NEG)
    header->rcode = SERVFAIL; /* couldn't get memory */
  else if (flags == F_NOERR || flags == F_QUERY)
    header->rcode = NOERROR; /* empty domain */
  else if (flags == F_NXDOMAIN)
    header->rcode = NXDOMAIN;
  else if (p && flags == F_IPV4)
    { /* we know the address */
      header->rcode = NOERROR;
      header->ancount = htons(1);
      header->aa = 1;
      add_resource_record(header, NULL, NULL, sizeof(HEADER), &p, ttl, NULL, T_A, C_IN, "4", addrp);
    }
#ifdef HAVE_IPV6
  else if (p && flags == F_IPV6)
    {
      header->rcode = NOERROR;
      header->ancount = htons(1);
      header->aa = 1;
      add_resource_record(header, NULL, NULL, sizeof(HEADER), &p, ttl, NULL, T_AAAA, C_IN, "6", addrp);
    }
#endif
  else /* nowhere to forward to */
    header->rcode = REFUSED;
 
  return p - (unsigned char *)header;
}

/* check if name matches local names ie from /etc/hosts or DHCP or local mx names. */
int check_for_local_domain(char *name, time_t now, struct daemon *daemon)
{
  struct crec *crecp;
  struct mx_srv_record *mx;
  struct txt_record *txt;
    
  if ((crecp = cache_find_by_name(NULL, name, now, F_IPV4 | F_IPV6)) &&
      (crecp->flags & (F_HOSTS | F_DHCP)))
    return 1;
  
  for (mx = daemon->mxnames; mx; mx = mx->next)
    if (hostname_isequal(name, mx->name))
      return 1;

  for (txt = daemon->txt; txt; txt = txt->next)
    if (hostname_isequal(name, txt->name))
      return 1;
  
  return 0;
}

/* Is the packet a reply with the answer address equal to addr?
   If so mung is into an NXDOMAIN reply and also put that information
   in the cache. */
int check_for_bogus_wildcard(HEADER *header, size_t qlen, char *name, 
			     struct bogus_addr *baddr, time_t now)
{
  unsigned char *p;
  int i, qtype, qclass, rdlen;
  unsigned long ttl;
  struct bogus_addr *baddrp;

  /* skip over questions */
  if (!(p = skip_questions(header, qlen)))
    return 0; /* bad packet */

  for (i=0; i<ntohs(header->ancount); i++)
    {
      if (!extract_name(header, qlen, &p, name, 1))
	return 0; /* bad packet */
  
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlen, p);
      
      if (qclass == C_IN && qtype == T_A)
	for (baddrp = baddr; baddrp; baddrp = baddrp->next)
	  if (memcmp(&baddrp->addr, p, INADDRSZ) == 0)
	    {
	      /* Found a bogus address. Insert that info here, since there no SOA record
		 to get the ttl from in the normal processing */
	      cache_start_insert();
	      cache_insert(name, NULL, now, ttl, F_IPV4 | F_FORWARD | F_NEG | F_NXDOMAIN | F_CONFIG);
	      cache_end_insert();
	      
	      return 1;
	    }
      
      p += rdlen;
    }
  
  return 0;
}

static int add_resource_record(HEADER *header, char *limit, int *truncp, unsigned int nameoffset, unsigned char **pp, 
			       unsigned long ttl, unsigned int *offset, unsigned short type, unsigned short class, char *format, ...)
{
  va_list ap;
  unsigned char *sav, *p = *pp;
  int j;
  unsigned short usval;
  long lval;
  char *sval;

  if (truncp && *truncp)
    return 0;

  PUTSHORT(nameoffset | 0xc000, p);
  PUTSHORT(type, p);
  PUTSHORT(class, p);
  PUTLONG(ttl, p);      /* TTL */

  sav = p;              /* Save pointer to RDLength field */
  PUTSHORT(0, p);       /* Placeholder RDLength */

  va_start(ap, format);   /* make ap point to 1st unamed argument */
  
  for (; *format; format++)
    switch (*format)
      {
#ifdef HAVE_IPV6
      case '6':
	sval = va_arg(ap, char *); 
	memcpy(p, sval, IN6ADDRSZ);
	p += IN6ADDRSZ;
	break;
#endif
	
      case '4':
	sval = va_arg(ap, char *); 
	memcpy(p, sval, INADDRSZ);
	p += INADDRSZ;
	break;
	
      case 's':
	usval = va_arg(ap, int);
	PUTSHORT(usval, p);
	break;
	
      case 'l':
	lval = va_arg(ap, long);
	PUTLONG(lval, p);
	break;
	
      case 'd':
	/* get domain-name answer arg and store it in RDATA field */
	if (offset)
	  *offset = p - (unsigned char *)header;
	p = do_rfc1035_name(p, va_arg(ap, char *));
	*p++ = 0;
	break;
	
      case 't':
	usval = va_arg(ap, int);
	sval = va_arg(ap, char *);
	memcpy(p, sval, usval);
	p += usval;
	break;
      }

  va_end(ap);	/* clean up variable argument pointer */
  
  j = p - sav - 2;
  PUTSHORT(j, sav);     /* Now, store real RDLength */
  
  /* check for overflow of buffer */
  if (limit && ((unsigned char *)limit - p) < 0)
    {
      if (truncp)
	*truncp = 1;
      return 0;
    }
  
  *pp = p;
  return 1;
}

/* return zero if we can't answer from cache, or packet size if we can */
size_t answer_request(HEADER *header, char *limit, size_t qlen, struct daemon *daemon, 
		      struct in_addr local_addr, struct in_addr local_netmask, time_t now) 
{
  char *name = daemon->namebuff;
  unsigned char *p, *ansp, *pheader;
  int qtype, qclass, is_arpa;
  struct all_addr addr;
  unsigned int nameoffset;
  unsigned short flag;
  int qdcount = ntohs(header->qdcount); 
  int q, ans, anscount = 0, addncount = 0;
  int dryrun = 0, sec_reqd = 0;
  struct crec *crecp;
  int nxdomain = 0, auth = 1, trunc = 0;
  struct mx_srv_record *rec;
 
  if (!qdcount || header->opcode != QUERY )
    return 0;

  /* If there is an RFC2671 pseudoheader then it will be overwritten by
     partial replies, so we have to do a dry run to see if we can answer
     the query. We check to see if the do bit is set, if so we always
     forward rather than answering from the cache, which doesn't include
     security information. */

  if (find_pseudoheader(header, qlen, NULL, &pheader))
    { 
      unsigned short udpsz, ext_rcode, flags;
      unsigned char *psave = pheader;

      GETSHORT(udpsz, pheader);
      GETSHORT(ext_rcode, pheader);
      GETSHORT(flags, pheader);
      
      sec_reqd = flags & 0x8000; /* do bit */ 

      /* If our client is advertising a larger UDP packet size
	 than we allow, trim it so that we don't get an overlarge
	 response from upstream */

      if (udpsz > daemon->edns_pktsz)
	PUTSHORT(daemon->edns_pktsz, psave); 

      dryrun = 1;
    }

  for (rec = daemon->mxnames; rec; rec = rec->next)
    rec->offset = 0;
  
 rerun:
  /* determine end of question section (we put answers there) */
  if (!(ansp = skip_questions(header, qlen)))
    return 0; /* bad packet */
   
  /* now process each question, answers go in RRs after the question */
  p = (unsigned char *)(header+1);

  for (q=0; q<qdcount; q++)
    {
      /* save pointer to name for copying into answers */
      nameoffset = p - (unsigned char *)header;

      /* now extract name as .-concatenated string into name */
      if (!extract_name(header, qlen, &p, name, 1))
	return 0; /* bad packet */
      
      /* see if it's w.z.y.z.in-addr.arpa format */

      is_arpa = in_arpa_name_2_addr(name, &addr);
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);

      ans = 0; /* have we answered this question */
      
      if (qtype == T_TXT || qtype == T_ANY)
	{
	  struct txt_record *t;
	  for(t = daemon->txt; t ; t = t->next)
	    {
	      if (t->class == qclass && hostname_isequal(name, t->name))
		{
		  ans = 1;
		  if (!dryrun)
		    {
		      log_query(F_CNAME | F_FORWARD | F_CONFIG | F_NXDOMAIN, name, NULL, 0, NULL, 0);
		      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
					      daemon->local_ttl, NULL,
					      T_TXT, t->class, "t", t->len, t->txt))
			anscount++;

		    }
		}
	    }
	}

      if (qclass == C_IN)
	{
	  if (qtype == T_PTR || qtype == T_ANY)
	    {
	      if (!(crecp = cache_find_by_addr(NULL, &addr, now, is_arpa)))
		{ 
		  if (is_arpa == F_IPV4 && (daemon->options & OPT_BOGUSPRIV) && private_net(&addr))
		    {
		      /* if not in cache, enabled and private IPV4 address, return NXDOMAIN */
		      ans = 1;
		      nxdomain = 1;
		      if (!dryrun)
			log_query(F_CONFIG | F_REVERSE | F_IPV4 | F_NEG | F_NXDOMAIN, name, &addr, 0, NULL, 0);
		    }
		}
	      else do 
		{ 
		  /* don't answer wildcard queries with data not from /etc/hosts or dhcp leases */
		  if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
		    continue;
		  
		  if (crecp->flags & F_NEG)
		    {
		      ans = 1;
		      auth = 0;
		      if (crecp->flags & F_NXDOMAIN)
			nxdomain = 1;
		      if (!dryrun)
			log_query(crecp->flags & ~F_FORWARD, name, &addr, 0, NULL, 0);
		    }
		  else if ((crecp->flags & (F_HOSTS | F_DHCP)) || !sec_reqd)
		    {
		      ans = 1;
		      if (!(crecp->flags & (F_HOSTS | F_DHCP)))
			auth = 0;
		      if (!dryrun)
			{
			  unsigned long ttl;
			  /* Return 0 ttl for DHCP entries, which might change
			     before the lease expires. */
			  if  (crecp->flags & (F_IMMORTAL | F_DHCP))
			    ttl = daemon->local_ttl;
			  else
			    ttl = crecp->ttd - now;
			  
			  log_query(crecp->flags & ~F_FORWARD, cache_get_name(crecp), &addr,
				    0, daemon->addn_hosts, crecp->uid);

			  if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, ttl, NULL,
						  T_PTR, C_IN, "d", cache_get_name(crecp)))
			    anscount++;
			}
		    }
		} while ((crecp = cache_find_by_addr(crecp, &addr, now, is_arpa)));
	    }

	  for (flag = F_IPV4; flag; flag = (flag == F_IPV4) ? F_IPV6 : 0)
	    {
	      unsigned short type = T_A;

	      if (flag == F_IPV6)
#ifdef HAVE_IPV6
		type = T_AAAA;
#else
	        break;
#endif
	      
	      if (qtype != type && qtype != T_ANY)
		continue;
	      
	      /* Check for "A for A"  queries. */
	      if (qtype == T_A && (addr.addr.addr4.s_addr = inet_addr(name)) != (in_addr_t) -1)
		{
		  ans = 1;
		  if (!dryrun)
		    {
		      log_query(F_FORWARD | F_CONFIG | F_IPV4, name, &addr, 0, NULL, 0);
		      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, 
					      daemon->local_ttl, NULL, type, C_IN, "4", &addr))
			anscount++;
		    }
		  continue;
		}

	    cname_restart:
	      if ((crecp = cache_find_by_name(NULL, name, now, flag | F_CNAME)))
		{
		  int localise = 0;
		  
		  /* See if a putative address is on the network from which we recieved
		     the query, is so we'll filter other answers. */
		  if (local_addr.s_addr != 0 && (daemon->options & OPT_LOCALISE) && flag == F_IPV4)
		    {
		      struct crec *save = crecp;
		      do {
			if ((crecp->flags & F_HOSTS) &&
			    is_same_net(*((struct in_addr *)&crecp->addr), local_addr, local_netmask))
			  {
			    localise = 1;
			    break;
			  } 
			} while ((crecp = cache_find_by_name(crecp, name, now, flag | F_CNAME)));
		      crecp = save;
		    }
			  
		  do
		    { 
		      /* don't answer wildcard queries with data not from /etc/hosts
			 or DHCP leases */
		      if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
			break;
		      
		      if (crecp->flags & F_CNAME)
			{
			  if (!dryrun)
			    {
			      log_query(crecp->flags, name, NULL, 0, daemon->addn_hosts, crecp->uid);
			      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, crecp->ttd - now, &nameoffset,
						      T_CNAME, C_IN, "d", cache_get_name(crecp->addr.cname.cache)))
				anscount++;
			    }
			  
			  strcpy(name, cache_get_name(crecp->addr.cname.cache));
			  goto cname_restart;
			}
		      
		      if (crecp->flags & F_NEG)
			{
			  ans = 1;
			  auth = 0;
			  if (crecp->flags & F_NXDOMAIN)
			    nxdomain = 1;
			  if (!dryrun)
			    log_query(crecp->flags, name, NULL, 0, NULL, 0);
			}
		      else if ((crecp->flags & (F_HOSTS | F_DHCP)) || !sec_reqd)
			{
			  /* If we are returning local answers depending on network,
			     filter here. */
			  if (localise && 
			      (crecp->flags & F_HOSTS) &&
			      !is_same_net(*((struct in_addr *)&crecp->addr), local_addr, local_netmask))
			    continue;
       
			  if (!(crecp->flags & (F_HOSTS | F_DHCP)))
			    auth = 0;
			  
			  ans = 1;
			  if (!dryrun)
			    {
			      unsigned long ttl;
			      
			      if  (crecp->flags & (F_IMMORTAL | F_DHCP))
				ttl = daemon->local_ttl;
			      else
				ttl = crecp->ttd - now;
			      
			      log_query(crecp->flags & ~F_REVERSE, name, &crecp->addr.addr,
					0, daemon->addn_hosts, crecp->uid);
			      
			      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, ttl, NULL, type, C_IN, 
						      type == T_A ? "4" : "6", &crecp->addr))
				anscount++;
			    }
			}
		    } while ((crecp = cache_find_by_name(crecp, name, now, flag | F_CNAME)));
		}
	    }
	  
	  if (qtype == T_MX || qtype == T_ANY)
	    {
	      int found = 0;
	      for (rec = daemon->mxnames; rec; rec = rec->next)
		if (!rec->issrv && hostname_isequal(name, rec->name))
		  {
		  ans = found = 1;
		  if (!dryrun)
		    {
		      unsigned int offset;
		      log_query(F_CNAME | F_FORWARD | F_CONFIG | F_IPV4, name, NULL, 0, NULL, 0);
		      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, daemon->local_ttl,
					      &offset, T_MX, C_IN, "sd", rec->weight, rec->target))
			{
			  anscount++;
			  if (rec->target)
			    rec->offset = offset;
			}
		    }
		  }
	      
	      if (!found && (daemon->options & (OPT_SELFMX | OPT_LOCALMX)) && 
		  cache_find_by_name(NULL, name, now, F_HOSTS | F_DHCP))
		{ 
		  ans = 1;
		  if (!dryrun)
		    {
		      log_query(F_CNAME | F_FORWARD | F_CONFIG | F_IPV4, name, NULL, 0, NULL, 0);
		      if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, daemon->local_ttl, NULL, 
					      T_MX, C_IN, "sd", 1, 
					      (daemon->options & OPT_SELFMX) ? name : daemon->mxtarget))
			anscount++;
		    }
		}
	    }
	  	  
	  if (qtype == T_SRV || qtype == T_ANY)
	    {
	      int found = 0;
	      
	      for (rec = daemon->mxnames; rec; rec = rec->next)
		if (rec->issrv && hostname_isequal(name, rec->name))
		  {
		    found = ans = 1;
		    if (!dryrun)
		      {
			unsigned int offset;
			log_query(F_CNAME | F_FORWARD | F_CONFIG | F_IPV6, name, NULL, 0, NULL, 0);
			if (add_resource_record(header, limit, &trunc, nameoffset, &ansp, daemon->local_ttl, 
						&offset, T_SRV, C_IN, "sssd", 
						rec->priority, rec->weight, rec->srvport, rec->target))
			  {
			    anscount++;
			    if (rec->target)
			      rec->offset = offset;
			  }
		      }
		  }
	      
	      if (!found && (daemon->options & OPT_FILTER) &&  (qtype == T_SRV || (qtype == T_ANY && strchr(name, '_'))))
		{
		  ans = 1;
		  if (!dryrun)
		    log_query(F_CONFIG | F_NEG, name, NULL, 0, NULL, 0);
		}
	    }
	  
	  if (qtype == T_MAILB)
	    ans = 1, nxdomain = 1;

	  if (qtype == T_SOA && (daemon->options & OPT_FILTER))
	    {
	      ans = 1; 
	      if (!dryrun)
		log_query(F_CONFIG | F_NEG, name, &addr, 0, NULL, 0);
	    }
	}

      if (!ans)
	return 0; /* failed to answer a question */
    }
  
  if (dryrun)
    {
      dryrun = 0;
      goto rerun;
    }
  
  /* create an additional data section, for stuff in SRV and MX record replies. */
  for (rec = daemon->mxnames; rec; rec = rec->next)
    if (rec->offset != 0)
      {
	/* squash dupes */
	struct mx_srv_record *tmp;
	for (tmp = rec->next; tmp; tmp = tmp->next)
	  if (tmp->offset != 0 && hostname_isequal(rec->target, tmp->target))
	    tmp->offset = 0;
	
	crecp = NULL;
	while ((crecp = cache_find_by_name(crecp, rec->target, now, F_IPV4 | F_IPV6)))
	  {
	    unsigned long ttl;
#ifdef HAVE_IPV6
	    int type =  crecp->flags & F_IPV4 ? T_A : T_AAAA;
#else
	    int type = T_A;
#endif
	    if (crecp->flags & F_NEG)
	      continue;

	    if  (crecp->flags & (F_IMMORTAL | F_DHCP))
	      ttl = daemon->local_ttl;
	    else
	      ttl = crecp->ttd - now;
	    
	    if (add_resource_record(header, limit, NULL, rec->offset, &ansp, ttl, NULL, type, C_IN, 
				    crecp->flags & F_IPV4 ? "4" : "6", &crecp->addr))
	      addncount++;
	  }
      }
  
  /* done all questions, set up header and return length of result */
  header->qr = 1; /* response */
  header->aa = auth; /* authoritive - only hosts and DHCP derived names. */
  header->ra = 1; /* recursion if available */
  header->tc = trunc; /* truncation */
  if (anscount == 0 && nxdomain)
    header->rcode = NXDOMAIN;
  else
    header->rcode = NOERROR; /* no error */
  header->ancount = htons(anscount);
  header->nscount = htons(0);
  header->arcount = htons(addncount);
  return ansp - (unsigned char *)header;
}





