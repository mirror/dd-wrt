/*
 * scdp - Send one CDP packet. Use it with a cron job.
 * Copyright (C) 2001 Mikael Wedlin & Kent Engström, Linköping University.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <conf.h>
#include <libnet.h>
#include <net/ethernet.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

void create_version(char *buf, struct utsname *uts);
int qflag = 0; /* TRUE if we should be quiet about errors. */

/*************************************************
                   Defines
 *************************************************/

/* Ethernet protocol type. */
#define ETHERTYPE_CDP	0x2000

/* CDP data types. */
#define CDP_TYPE_DEVID		0x0001
#define CDP_TYPE_ADDRESS	0x0002
#define CDP_TYPE_PORTID		0x0003
#define CDP_TYPE_CAPABILITIES	0x0004
#define CDP_TYPE_VERSION	0x0005
#define CDP_TYPE_PLATFORM	0x0006
#define CDP_TYPE_IPPREFIX	0x0007

/* CDP Capabilities */
#define CDP_CAP_ROUTER		0x01  /* R */
#define CDP_CAP_TRANSP_BRIDGE	0x02  /* T */
#define CDP_CAP_SOURCE_BRIDGE	0x04  /* B */
#define CDP_CAP_SWITCH		0x08  /* S */
#define CDP_CAP_HOST		0x10  /* H */
#define CDP_CAP_IGMP		0x20  /* I */
#define CDP_CAP_REPEATER	0x40  /* r */

/* Buffer sizes */

#define HOSTBUFSIZE 128
#define TEMPBUFSIZE 512

/***********************************************
                     Usage
***********************************************/

void usage(int uts_valid, struct utsname *uts, int host_valid, char *hostbuf) {
  char verbuf[TEMPBUFSIZE];

  fprintf(stderr,"scdp -i <interface> [params]\n");
  fprintf(stderr,"  Send one cdp package to the specified interface\n");

  fprintf(stderr,"  -i <interface>\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --interface=<interface>\n");
  fprintf(stderr,"  --portid=<portid>\n");
#endif
  fprintf(stderr,"     Send to this interface. Also portid\n");
  /************************************************************/
  fprintf(stderr,"  -d <devid>\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --devid=<devid>\n");
#endif
  fprintf(stderr,"     Device ID (hostname)\n");
  if (host_valid)
    fprintf(stderr,"     Default is: '%s'\n", hostbuf);
  /************************************************************/
  fprintf(stderr,"  -a <address>\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --address=<address>\n");
#endif
  fprintf(stderr,"     One IP address.\n");
  /************************************************************/
  fprintf(stderr,"  -c <capabilities>\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --capabilities=<capabilities>\n");
#endif
  fprintf(stderr,"     Report the device's functionel capabilities\n");
  fprintf(stderr,"     as a string with one character for each capability.\n");
  fprintf(stderr,"     R: Router, T: Transparent bridge, B: Source route\n");
  fprintf(stderr,"     bridge, S: Switch, H: host, I: IGMP, r: Repeater.\n");
  fprintf(stderr,"     Default is: 'H' (host)\n");
  /************************************************************/
  fprintf(stderr,"  -v <version>\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --version=<version>\n");
#endif
  fprintf(stderr,"     Report a version string\n");
  if (uts_valid)
  {
    create_version(verbuf, uts);
    fprintf(stderr,"     Default is: '%s'\n", verbuf);
  }
  /************************************************************/
  fprintf(stderr,"  -p <platform>\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --platform=<platform>\n");
#endif
  fprintf(stderr,"     Report a platform string\n");
  if (uts_valid)
    fprintf(stderr,"     Default is: '%s'\n", uts->sysname);
  /************************************************************/
  fprintf(stderr,"  -x <prefix>\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --prefix=<prefix>\n");
#endif
  fprintf(stderr,"     Report a prefix string\n");
  /************************************************************/
  fprintf(stderr,"  -t <seconds>\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --ttl=<seconds>\n");
#endif
  fprintf(stderr,"     Set a time to live value. (Default 180 s)\n");
  /************************************************************/
  fprintf(stderr,"  -q\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --quiet\n");
#endif
  fprintf(stderr,"     Be quiet and don't print any runtime error messages.\n");
  /************************************************************/
  fprintf(stderr,"  -g\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --debug\n");
#endif
  fprintf(stderr,"     Print a lot of debug messages.\n");
  /************************************************************/
  fprintf(stderr,"  -h\n");
#ifdef HAVE_GETOPT_LONG
  fprintf(stderr,"  --help\n");
#endif
  fprintf(stderr,"     Print this text and exit.\n");

  exit(-1);
};

/**********************************************/
/*      Create default version.               */
/**********************************************/

void create_version(char *buf, struct utsname *uts)
{
  sprintf(buf, "%s %s %s %s", 
	  uts->sysname,
	  uts->release,
	  uts->version,
	  uts->machine);
}

/**********************************************/
/*      Create the 802.2 and CDP packets.     */
/**********************************************/

/* Add a text CDP value to the packet. */
int cdp_str_add(void *packet, unsigned short type, char *string) {
  unsigned short length = strlen(string) + 4;
  unsigned short plen = htons(length);
  unsigned short ptype = htons(type);

  memcpy(packet, &ptype, 2);
  memcpy(packet+2, &plen, 2);
  memcpy(packet+4, string, length-4);

  return(length);
}

/* Add a CDP capabilities to the packet. */
int cdp_cap_add(void *packet, char *cap) {
  int c, l = strlen(cap);
  register unsigned long tcap = 0;
  struct cap_s_t {
    unsigned short type;
    unsigned short length;
    unsigned long  capabilities;
  } cap_s;

  cap_s.type = htons(CDP_TYPE_CAPABILITIES);
  cap_s.length = htons(sizeof(cap_s));

  for (c=0; c<l; c++) {
    switch (cap[c]) {
    case 'R': tcap |= CDP_CAP_ROUTER; break;
    case 'T': case 't': tcap |= CDP_CAP_TRANSP_BRIDGE; break;
    case 'B': case 'b': tcap |= CDP_CAP_SOURCE_BRIDGE; break;
    case 'S': case 's': tcap |= CDP_CAP_SWITCH; break;
    case 'H': case 'h': tcap |= CDP_CAP_HOST; break;
    case 'I': case 'i': tcap |= CDP_CAP_IGMP; break;
    case 'r': tcap |= CDP_CAP_REPEATER; break;
    default:
      if (!qflag) libnet_error(LIBNET_ERR_WARNING, "Unkown capability %c.\n", cap[c]);
    };
  };
  cap_s.capabilities = htonl(tcap);
  memcpy(packet, (void *)&cap_s, sizeof(cap_s));
  return sizeof(cap_s);
}

/* Make an IP address from an ASCII representation. */
unsigned long
atoip(char *string) {
  if (strspn(string, "0123456789.") == strlen(string)) {
    /* This is an IP address. */
    return libnet_name_resolve(string, 0);
  } else {
    return libnet_name_resolve(string, 1);
  };
}

/* Add IP addresses to the CDP packet. */
int
cdp_ip_add(
	   unsigned char* packet,	/* Place data here. */
	   unsigned long * ip,	/* Table of IP addresses in network order. */
	   int length		/* Number of IP addresses in ip. */
	   ) {
  struct header_t { /* Address header */
    unsigned short type;
    unsigned short length;
    unsigned long addresses;
  } header;

#if 0     /* Not used due to alignement problems. */
  struct aptr_t { /* Address struct. 9 bytes long. */
    unsigned char protocol_1;
    unsigned char length;
    unsigned char protocol_2;
    unsigned short addrlen;
    unsigned long address;
  } *aptr;
#endif
  int i, ptr;
  unsigned short ipadrlen = htons(4);
  unsigned long ipadr;

  header.type = htons(CDP_TYPE_ADDRESS);
  header.addresses = htonl(length);

  ptr = sizeof(header);
  for (i=0; i<length; i++) {
    packet[ptr++] = 1; /* Protocol: NLPID */
    packet[ptr++] = 1; /* Length */
    packet[ptr++] = 0xcc; /* Protocol: IP */
    memcpy(&packet[ptr],  &ipadrlen, 2); ptr += 2;
    memcpy(&packet[ptr], &ip[i], 4); ptr +=4;
  };
  header.length = htons(ptr);
  memcpy(packet, (void *)&header, sizeof(header));

  return ptr;
}
  
/* Calculate a cdp checksum. */
unsigned short
cdp_checksum(u_char *ptr, int length) {
  if (length % 2 == 0) {
    /* The doc says 'standard IP checksum', so this is what we do. */
    return libnet_ip_check((u_short *)ptr, length);
  } else {
    /* An IP checksum is not defined for an odd number of bytes... */
    /* Tricky. */
    /* Treat the last byte as an unsigned short in network order. */

    int c = ptr[length-1];
    unsigned short *sp = (unsigned short *)(&ptr[length-1]);
    unsigned short ret;

    *sp = htons(c);
    ret = libnet_ip_check((u_short *)ptr, length+1);
    ptr[length-1] = c;
    return ret;
  };
}

/**********************************************/
/*                   exexcdp                  */
/**********************************************/

int
execcdp(char *interface, int argc, char *argv[]) {
  char **new_argv = (char **)malloc((argc+3)*sizeof(char));
  int i, pid;

  if ((pid = fork()) == 0) {
    /* This is the child. */

    new_argv[0] = argv[0];
    new_argv[1] = "-i";
    new_argv[2] = interface;
    for (i = 1; i<argc+3; i++) {
      new_argv[i+2] = argv[i];
    };

    if (execvp(argv[0], new_argv) == -1) {
      if (!qflag) perror("SCDP: ");
    };
    exit(-1);
  } else {
    /* This is the mother process. */
    (void) waitpid(pid, &i, 0);
    return i;
  };
}

/**********************************************/
/*                    Main                    */
/**********************************************/

int
main(int argc, char *argv[]) {

  /***************** Variables ****************/

  int rs, ret, c;
  u_char *lpacket;
  int lpack_size;
  struct ether_addr *eth_src;	/* My ethernet address. */
  u_char eth_dst[6] =	/* Destination ethernet address. */
  {0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc};

  struct libnet_link_int *link;

  struct header_t {
    /* 802.2 LLC */
    unsigned char	DSAP;
    unsigned char	SSAP;
    unsigned char	cntl;
    /* 802.2 SNAP */
    unsigned char	org_code[3];
    unsigned short	type;
  } *header;

  struct cdp_header_t {
    unsigned char	version;
    unsigned char	ttl;
    short		checksum;
  } *cdp_header;

  unsigned char packet[8192];
  unsigned char ebuf[LIBNET_ERRBUF_SIZE];
  int plen = 0;
  int cdp_start;

  struct utsname uts;
  int uts_valid = 0;
  char tempbuf[TEMPBUFSIZE];
  char hostbuf[HOSTBUFSIZE];
  int host_valid = 0;

  char *interface=NULL, *devid=NULL, *version=NULL,
    *platform=NULL, *capabilities=NULL;
  int ttl = 180;

  struct libnet_ifaddr_list *ifaddr;

#define MAX_ADDR 200 /* This shuld be allowed. */
  unsigned long addr[MAX_ADDR];
  int addrlen = 0;

#ifdef HAVE_GETOPT_LONG
  int opt_ind;
  static struct option l_opt[] = {
    /* name		has_arg	flag	val */
    {"interface",	1,	0,	'i'},
    {"portid",		1,	0,	'i'},
    {"devid",		1,	0,	'd'},
    {"address",		1,	0,	'a'},
    {"capabilities",	1,	0,	'c'},
    {"version",		1,	0,	'v'},
    {"platform",	1,	0,	'p'},
    {"prefix",		1,	0,	'x'},
    {"ttl",		1,	0,	't'},
    {"debug",		0,	0,	'g'},
    {"quiet",		0,	0,	'q'},
    {"help",		0,	0,	  0},
    {NULL,			0,	0,	  0}};
#endif

  int debug = 0;

  /*************** Get uname data for later use. ****************/
  if (uname(&uts) >= 0)
  {
    uts_valid = 1; 
    if (gethostname(hostbuf, HOSTBUFSIZE) == 0)
    {
      host_valid = 1;
    }    
  }
 
  /*************** Parse the arguments.****************/

#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, "i:d:a:v:p:x:t:hgq", l_opt, &opt_ind)) != -1) {
#else
  while ((c = getopt(argc, argv, "i:d:a:c:v:p:x:t:hgq")) != -1) {
#endif
    switch(c) {
    case 'i': /* Interface */
      interface = optarg;
      break;
    case 'd':
      devid = optarg;
      break;
    case 'a':
      if (addrlen >= MAX_ADDR) break; /* Silently truncate these. */
      addr[addrlen] = atoip(optarg);
      if (addr[addrlen] == 0) {
	if (!qflag) libnet_error(LIBNET_ERR_WARNING,
				 "Invalid ip address %s, ignored.\n",
				 optind);
      } else {
	addrlen++;
      };
      break;
    case 'c':
      capabilities = optarg;
      break;
    case 'v':
      version = optarg;
      break;
    case 'p':
      platform = optarg;
      break;
    case 'x':
      fprintf(stderr, "scdp: -x NOT YET IMPLEMENTED!\n");
      usage(uts_valid, &uts, host_valid, hostbuf);
      break;
    case 't':
      ttl = atoi(optarg);
      break;
    case 'g':
      debug = 1;
      break;
    case 'q':
      qflag = 1;
      break;
    case 'h':
    default:
      usage(uts_valid, &uts, host_valid, hostbuf);
    };
  };

  /**************** Check for interfaces ***************/

  if (interface == NULL) { /* Try to find open interfaces. */
    int i;

    libnet_ifaddrlist(&ifaddr, ebuf);
    for (i=0; ifaddr[i].device != NULL; i++) {
      if (debug) {
	printf("Interface %d: %lu, %s\n", i, ifaddr[i].addr, ifaddr[i].device);
      };
      (void)execcdp(ifaddr[i].device, argc, argv);
    };
    exit(0);
  };

  /**************** Create CDP packet ***************/

  /* Place pointers to the headers in packet. */
  header = (struct header_t *)&packet[0];
  plen = sizeof(*header);
  cdp_header = (struct cdp_header_t *)&packet[plen];
  memset(&packet[0], 0, sizeof(packet));

  /* IEEE 802.2 header. */
  header->DSAP = 0xaa; /* SNAP */
  header->SSAP = 0xaa; /* SNAP */
  header->cntl = 0x03;
  header->org_code[2] = 0x0c;
  header->type = htons(ETHERTYPE_CDP);

  /* CDP header. */
  cdp_header->version = 1;
  cdp_header->ttl = ttl;

  cdp_start = plen; /* Index to the start of the CDP packet. */
  plen += sizeof(*cdp_header); /* plen is an index to the first free
				  byte in the packet. */

  /**************** Network initialization ***************/

  if (interface == NULL) { /* Try to find open interfaces. */
    struct sockaddr_in sin;
    if (libnet_select_device(&sin, (u_char **)&interface, ebuf) == -1) {
      if (qflag) {
	exit(0);
      } else {
	libnet_error(LIBNET_ERR_FATAL,
		     "Failed to find default interface: %s\n",
		     ebuf);
      };
    };
  };
  if (debug) { printf("Interface: %s\n", interface); }

  if ((link = libnet_open_link_interface(interface, ebuf)) == NULL) {
    if (qflag) { exit(0); } else {
      libnet_error(LIBNET_ERR_FATAL, "Could not open interface %s: %s\n",
		   interface, ebuf);
    };
  };

  /***************** Create CDP packet ********************/

  /* Device ID. */
  if (devid == NULL && host_valid)
  {
    devid = hostbuf;
  }
  if (devid != NULL) {
    plen += cdp_str_add(&packet[plen], CDP_TYPE_DEVID, devid);
  };

  /* Addresses */
  if (addrlen == 0) {
    /* Try to find at least one address. */
    addr[0] = htonl(libnet_get_ipaddr(link, interface, ebuf));
    if (addr[0] == 0) {
      if (!qflag) libnet_error(LIBNET_ERR_WARNING,
			       "Could not find address on interface %s: %s\n",
			       interface, ebuf);
    } else {
      addrlen++;
    };
  };
  plen += cdp_ip_add(&packet[plen], &addr[0], addrlen);

  /* Port ID */
  plen += cdp_str_add(&packet[plen], CDP_TYPE_PORTID, interface);

  /* Capabilities */
  if (capabilities == NULL) capabilities = "H";
  plen += cdp_cap_add(&packet[plen], capabilities);

  /* Version */
  if (version == NULL && uts_valid)
  {
    create_version(tempbuf, &uts);
    version = tempbuf;
  }
  if (version != NULL) {
    plen += cdp_str_add(&packet[plen], CDP_TYPE_VERSION, version);
  };

  /* Platform */
  if (platform == NULL && uts_valid)
  {
    strcpy(tempbuf, uts.sysname);
    platform = tempbuf;
  }
  if (platform != NULL) {
    plen += cdp_str_add(&packet[plen], CDP_TYPE_PLATFORM, platform);
  };

  /* IP prefix. */
  /* NOT YET IMPLEMENTED! */


  /* The checksum is an IP checksum over the entire CDP packet */
  cdp_header->checksum =
    cdp_checksum((void *)cdp_header, plen - cdp_start);

  /**************** Create the packet *******************/
  lpack_size = plen + LIBNET_ETH_H;
  
  if (libnet_init_packet (lpack_size, &lpacket) == -1) {
    if (qflag) exit(0); else {
      libnet_error(LIBNET_ERR_FATAL, "Could not init packet.\n");
    };
  };

  /* Find my ethernet address. */
  if ((eth_src = libnet_get_hwaddr(link, interface, ebuf)) == 0) {
    if (qflag) exit(0); else {
      libnet_error(LIBNET_ERR_FATAL,
		   "Could not find the MAC address of %s: %s\n",
		   interface, ebuf);
    };
  };

  /* Assemble the packet. */
  libnet_build_ethernet(eth_dst, eth_src->ether_addr_octet,plen, (u_char *)header, plen,lpacket);

  if (debug)
  {
    libnet_hex_dump(lpacket, lpack_size, 1, stdout);
  }

  /*************** Send the packet ********************/
  if ((c = libnet_write_link_layer(link, interface, lpacket, lpack_size))
      < lpack_size) {
    if (!qflag) libnet_error(LIBNET_ERR_WARNING,
			     "Only wrote %d bytes instead of %d.\n",
			     c, lpack_size);
  };


  /*************** Clean up ******************/
  if (libnet_close_link_interface(link) == -1) {
    if (!qflag) libnet_error(LIBNET_ERR_WARNING,
			     "Failed to close interface.\n");
  };

  libnet_destroy_packet(&lpacket);

  exit(EXIT_SUCCESS);
}





