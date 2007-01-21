/* ----------------------------------------------------------------------------
    NSTX -- tunneling network-packets over DNS

     (C) 2000 by Julien Oster and Florian Heinz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  -------------------------------------------------------------------------- */

#ifndef NSTX_DNS_H
#define NSTX_DNS_H

struct dnsqueryhdr {
  unsigned short id;

/* be careful with that!
 * this version is documented as the LITTLE endian version, however our
 * processors are having a BIG endian byte order!
 *
 * The solution is simple (but did take as some stressy hours to find):
 * this is NOT the byte order, it is the BIT order!
 *
 * And as most processors (except some very rare ones including the DEC
 * PDP machines) we are having a little endian bit order.
 *
 * So if you are reading something about endianess in this context,
 * be careful if it says BYTE order or BIT order! On Intel processors, for
 * example, byte order is big endian, while bit order is little endian.
 *
 * For additional confusion: RFC1035 really shows the headers in the very
 * uncommon big endian bit order.
 *
 * In the only intention to drive you completely mad: at least in Linux the
 * header-file <arpa/nameser.h> talks about a little endian BYTE order for
 * our working version. Ugh. Anyway, it clearly works this way, and that
 * proves the correctness of my assumption.
 *
 * -- frodo
 */

  unsigned char rd:1;
  unsigned char tc:1;
  unsigned char aa:1;
  unsigned char opcode:4;
  unsigned char qr:1;
  unsigned char rcode:4;
  unsigned char z:3;
  unsigned char ra:1;

  unsigned short qdcount;
  unsigned short ancount;
  unsigned short nscount;
  unsigned short arcount;
#define DNSOP_QUERY 0
#define DNSOP_IQUERY 1
#define DNSOP_STATUS 2
};

struct dnsquestion {
  unsigned char qname[48];
  unsigned short qtype;
  unsigned short qclass;
};

struct dnsanswer {
  unsigned short type;
  unsigned short class;
  unsigned int ttl;
  // unsigned short rdlength;
};


struct dnstxtanswer {
  unsigned char name[256];
};

/* overall functions */


/* client functions */

int nstxc_dns_init (struct sockaddr *, socklen_t,
		    struct sockaddr *, socklen_t);
int nstxc_dns_query (char *, unsigned short);


#endif /*NSTX_DNS_H*/
