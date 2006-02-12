/*
 *  $Id: libnet-functions.h,v 1.1 2004/04/27 01:30:54 dyang Exp $
 *
 *  libnet-functions.h - Network routine library function prototype header file
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef __LIBNET_FUNCTIONS_H
#define __LIBNET_FUNCTIONS_H


int                                     /* 1 if good, -1 if bad */
libnet_plist_chain_new(
    struct libnet_plist_chain **,       /* pointer to the head of the list */
    char *                              /* token list pointer */
    );


int                                     /* 1 if more nodes, 0 if not */
libnet_plist_chain_next_pair(
    struct libnet_plist_chain *,        /* pointer to the head of the list */
    u_short *bport,                     /* holds bport */
    u_short *eport                      /* holds eport */
    );


int
libnet_plist_chain_dump(
    struct libnet_plist_chain *         /* pointer to the head of the list */
    );


u_char *
libnet_plist_chain_dump_string(
    struct libnet_plist_chain *         /* pointer to the head of the list */
    );


int
libnet_plist_chain_free(
    struct libnet_plist_chain *         /* pointer to the head of the list */
    );

/*
 *  Standard error handling code.
 */

void
libnet_error(
    int,                /* severity */
    char *,             /* error message */
    ...                 /* varargs */
    );


/*
 *  Seeds the pseudorandom number generator with gettimeofday.
 */

int
libnet_seed_prand();


/*
 *  Returns a psuedorandom positive integer.
 */

u_long
libnet_get_prand(
    int                 /* One of the PR* constants */
    );


/*
 *  Calculates IPv4 family checksum on packet headers.
 */

int                     /* 1 on success, -1 on failure */
libnet_do_checksum(
    u_char *,           /* Pointer to the packet buffer */
    int,                /* Protocol */
    int                 /* Packet size */
    );


/*
 *  Network byte order into IP address
 *  Previous versions had a memory leak (returned a strdup'd pointer -- strdup
 *  has an implicit malloc which wasn't getting freed).  This static var hack
 *  thingy was used to preserve existing code without having to change much.
 *  You can simply use the return value of the function directly allowing you
 *  to write tighter, more obvious code (rather then having to do allocate an
 *  additional buffer for the output).
 *  Thanks to Red for the idea.
 */

u_char *                /* Pointer to hostname or dotted decimal IP address */
libnet_host_lookup(
    u_long,             /* Network byte ordered (big endian) IP address */
    u_short             /* Use domain names or no */
    );


/*
 *  Network byte order into IP address
 *  Threadsafe version.
 */

void
libnet_host_lookup_r(
    u_long,             /* Network byte ordered (big endian) IP address */
    u_short,            /* Use domain names or no */
    u_char *            /* Pointer to hostname or dotted decimal IP address */
    );


/*
 *  IP address into network byte order
 */

u_long                  /* Network byte ordered IP address or -1 on error */
libnet_name_resolve(
    u_char *,           /* Pointer the hostname or dotted decimal IP address */
    u_short             /* Use domain names or no */
    );


/*
 *  IP checksum wrapper.
 */

u_short                 /* Standard IP checksum of header and data */
libnet_ip_check(
    u_short *,          /* Pointer to the buffer to be summed */
    int                 /* Packet length */
    );


/*
 *  IP checksum.
 */

int                     /* Standard IP checksum */
libnet_in_cksum(
    u_short *,          /* Pointer to the buffer to be summed */
    int                 /* Packet length */
    );


/*
 *  Opens a socket for writing raw IP datagrams to.  Set IP_HDRINCL to let the 
 *  kernel know we've got it all under control.
 */

int                     /* Opened file desciptor, or -1 on error */
libnet_open_raw_sock(
    int                 /* Protocol of raw socket (from /etc/protocols) */
    );


int                     /* 1 upon success, or -1 on error */
libnet_close_raw_sock(
    int                 /* File descriptor */
    );


int
libnet_select_device(
    struct sockaddr_in *sin,
    u_char **device,
    u_char *ebuf
    );

/*
 *  Ethernet packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_ethernet(
    u_char *,           /* Pointer to a 6 byte ethernet address */
    u_char *,           /* Pointer to a 6 byte ethernet address */
    u_short,            /* Packet IP type */
    const u_char *,     /* Payload (or NULL) */
    int,                /* Payload size */
    u_char *            /* Packet header buffer */
    );


/*
 *  ARP packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_arp(
    u_short,            /* hardware address type */
    u_short,            /* protocol address type */
    u_char,             /* hardware address length */
    u_char,             /* protocol address length */
    u_short,            /* ARP operation type */
    u_char *,           /* sender hardware address */
    u_char *,           /* sender protocol address */
    u_char *,           /* target hardware address */
    u_char *,           /* target protocol address */
    const u_char *,     /* payload or NULL if none */
    int,                /* payload length */
    u_char *            /* packet buffer memory */
    );

/*
 *  TCP packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_tcp(
    u_short,            /* Source port */
    u_short,            /* Destination port */
    u_long,             /* Sequence Number */
    u_long,             /* Acknowledgement Number */
    u_char,             /* Control bits */
    u_short,            /* Advertised Window Size */
    u_short,            /* Urgent Pointer */
    const u_char *,     /* Pointer to packet data (or NULL) */
    int,                /* Packet payload size */
    u_char *            /* Pointer to packet header memory */
    );


/*
 * UDP packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_udp(
    u_short,            /* Source port */
    u_short,            /* Destination port */
    const u_char *,     /* Pointer to packet data (or NULL) */
    int,                /* Packet payload size */
    u_char *            /* Pointer to packet header memory */
    );

/*
 *  ICMP_ECHO packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_icmp_echo(
    u_char,             /* icmp type */
    u_char,             /* icmp code */
    u_short,            /* id */
    u_short,            /* sequence number */
    const u_char *,     /* Pointer to packet data (or NULL) */
    int,                /* Packet payload size */
    u_char *            /* Pointer to packet header memory */
    );

/*
 *  ICMP_MASK packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_icmp_mask(
    u_char,             /* icmp type */
    u_char,             /* icmp code */
    u_short,            /* id */
    u_short,            /* sequence number */
    u_long,             /* address mask */
    const u_char *,     /* Pointer to packet data (or NULL) */
    int,                /* Packet payload size */
    u_char *            /* Pointer to packet header memory */
    );


/*
 *  ICMP_UNREACH packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_icmp_unreach(
    u_char,             /* icmp type */
    u_char,             /* icmp code */
    u_short,            /* Original Length of packet data */
    u_char,             /* Original IP tos */
    u_short,            /* Original IP ID */
    u_short,            /* Original Fragmentation flags and offset */
    u_char,             /* Original TTL */
    u_char,             /* Original Protocol */
    u_long,             /* Original Source IP Address */
    u_long,             /* Original Destination IP Address */
    const u_char *,     /* Pointer to original packet data (or NULL) */
    int,                /* Packet payload size (or 0) */
    u_char *            /* Pointer to packet header memory */
    );

/*
 *  ICMP_REDIRECT packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_icmp_redirect(
    u_char,             /* icmp type */
    u_char,             /* icmp code */
    u_long,             /* Gateway host that should be used */
    u_short,            /* Original Length of packet data */
    u_char,             /* Original IP tos */
    u_short,            /* Original IP ID */
    u_short,            /* Original Fragmentation flags and offset */
    u_char,             /* Original TTL */
    u_char,             /* Original Protocol */
    u_long,             /* Original Source IP Address */
    u_long,             /* Original Destination IP Address */
    const u_char *,     /* Pointer to original packet data (or NULL) */
    int,                /* Packet payload size (or 0) */
    u_char *            /* Pointer to packet header memory */
    );


/*
 *  ICMP_TIMXCEED packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_icmp_timeexceed(
    u_char,             /* icmp type */
    u_char,             /* icmp code */
    u_short,            /* Original Length of packet data */
    u_char,             /* Original IP tos */
    u_short,            /* Original IP ID */
    u_short,            /* Original Fragmentation flags and offset */
    u_char,             /* Original TTL */
    u_char,             /* Original Protocol */
    u_long,             /* Original Source IP Address */
    u_long,             /* Original Destination IP Address */
    const u_char *,     /* Pointer to original packet data (or NULL) */
    int,                /* Packet payload size (or 0) */
    u_char *            /* Pointer to packet header memory */
    );

/*
 *  ICMP_TIMESTAMP packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_icmp_timestamp(
    u_char,             /* icmp type */
    u_char,             /* icmp code */
    u_short,            /* id */
    u_short,            /* sequence number */
    n_time,             /* original timestamp */
    n_time,             /* receive timestamp */
    n_time,             /* transmit timestamp */
    const u_char *,     /* Pointer to packet data (or NULL) */
    int,                /* Packet payload size */
    u_char *            /* Pointer to packet header memory */
    );

/*
 *  IGMP packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_igmp(
    u_char,             /* igmp type */
    u_char,             /* igmp code */
    u_long,             /* ip addr */
    const u_char *,     /* Pointer to packet data (or NULL) */
    int,                /* Packet payload size */
    u_char *            /* Pointer to packet header memory */
    );


/*
 *  IP packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_ip(
    u_short,            /* Length of packet data */
    u_char,             /* IP tos */
    u_short,            /* IP ID */
    u_short,            /* Fragmentation flags and offset */
    u_char,             /* TTL */
    u_char,             /* Protocol */
    u_long,             /* Source IP Address */
    u_long,             /* Destination IP Address */
    const u_char *,     /* Pointer to packet data (or NULL) */
    int,                /* Packet payload size */
    u_char *            /* Pointer to packet header memory */
    );


/*
 *  DNS pacekt assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_dns(
    u_short,            /* Packet ID */
    u_short,            /* Flags */
    u_short,            /* Number of questions */
    u_short,            /* Number of answer resource records */
    u_short,            /* Number of authority resource records */
    u_short,            /* Number of additional resource records */
    const u_char *,     /* Payload (or NULL) */
    int,                /* Payload size */
    u_char *            /* Header memory */
    );


/*
 *  RIP packet assembler.
 */

int                     /* -1 on failure (null buf passed in), 1 on success */
libnet_build_rip(
    u_char,             /* Command */
    u_char,             /* Version */
    u_short,            /* Zero (v1) or Routing Domain (v2) */
    u_short,            /* Address family */
    u_short,            /* Zero (v1) or Route Tag (v2) */
    u_long,             /* IP address */
    u_long,             /* Zero (v1) or Subnet Mask (v2) */
    u_long,             /* Zero (v1) or Next hop IP address (v2) */
    u_long,             /* Metric */
    const u_char *,     /* Payload (or NULL) */
    int,                /* Payload size */
    u_char *            /* Header memory */
    );


/*
 *  Insert IP options to a prebuilt IP packet.
 */

int                     /* 1 on success, -1 on failure */
libnet_insert_ipo(
    struct ipoption *,  /* Pointer to the ip options structure */ 
    u_char,             /* IP option list size */
    u_char *            /* Pointer to packet buf */
    );

/*
 *  Insert TCP options to a prebuilt IP packet.
 */

int                     /* 1 on success, -1 on failure */
libnet_insert_tcpo(
    struct tcpoption *, /* Pointer to the tcp options structure */ 
    u_char,             /* TCP option list size */
    u_char *            /* Pointer to packet buf */
    );

/*
 *  Writes a prebuild IP packet to the network with a supplied raw socket.
 *  To write a link layer packet, use the write_link_layer function.
 */

int                     /* number of bytes written if successful, -1 on error */
libnet_write_ip(
    int sock,           /* Previously opened raw socket */
    u_char *,           /* Pointer a complete IP datagram */
    int                 /* Packet size */
    );

/*
 *  Writes a prebuild IP/ethernet packet to the network with a supplied
 *  link_layer interface.  To write just an IP packet, use the write_link_layer
 *  function.
 */

int                     /* number of bytes written if successful, -1 on error */
libnet_write_link_layer(
    struct libnet_link_int *,  /* Pointer to a link interface structure */
    const u_char *,     /* Pointer to the device */
    u_char *,           /* Pointer the u_char buf (the packet)to be written */
    int                 /* Packet length */
    );


/*
 *  Opens a link layer interface.  Analogous to open_raw_sock.
 */

struct libnet_link_int *       /* Pointer to a link layer interface struct */
libnet_open_link_interface(
    char *,             /* Device name */
    char *              /* Error buffer */
    );


int                     /* 1 on success, -1 on failure */
libnet_close_link_interface(
    struct libnet_link_int *   /* Pointer to a link layer interface struct */
    );


char *                  /* String error message */
ll_strerror(
    int                 /* Errno */
    );


/*
 *  Returns the IP address of the interface.
 */

u_long                  /* 0 upon error, address upon success */
libnet_get_ipaddr(
    struct libnet_link_int *,  /* Pointer to a link interface structure */
    const u_char *,     /* Device */
    char *              /* Error buf */
    );


/*
 *  Returns the MAC address of the interface.
 */

struct ether_addr *     /* 0 upon error, address upon success */
libnet_get_hwaddr(
    struct libnet_link_int *,  /* Pointer to a link interface structure */
    const u_char *,     /* Device */
    char *              /* Error buf */
    );


/*
 *  Simple interface for initializing a packet.
 *  Basically a malloc wrapper.  
 */

int                     /* -1 on error, 1 on ok */
libnet_init_packet(
    int,                /* 0 and we make a good guess, otherwise you choose. */
    u_char **           /* Pointer to the pointer to the packet */
    );      


/*
 *  Simple interface for destoying a packet.
 *  Don't call this without a corresponding call to init_packet() first.
 */

int                         /* -1 if arena is NULL, 1 if ok */
libnet_destroy_packet(
    u_char **               /* Pointer to the packet addr. */
    );


/*
 *  Memory pool initialization routine.
 */

int
libnet_init_packet_arena(
    struct libnet_arena **, /* Pointer to an arena pointer */
    int,                /* 0 and we make a good guess, otherwise you choose. */
    u_short
    );


/*
 *  Returns the next chunk of memory from the pool.
 */

u_char *
libnet_next_packet_from_arena(
    struct libnet_arena **, /* Pointer to an arena pointer */
    int                 /* 0 and we make a good guess, otherwise you choose. */
    );


/*
 *  Memory pool destructor routine.
 */

int                         /* -1 if arena is NULL, 1 if ok */
libnet_destroy_packet_arena(
    struct libnet_arena **  /* Pointer to an arena pointer */
    );


/* 
 *  More or less taken from tcpdump code.
 */

void
libnet_hex_dump(
    u_char *,               /* Packet to be dumped */
    int,                    /* Packet size (in bytes */
    int,                    /* To swap or not to swap */
    FILE *                  /* Stream pointer to dump to */
    );


#endif  /* __LIBNET_FUNCTIONS_H */

/* EOF */
