
/*
 * Secure OLSR plugin
 * http://www.olsr.org
 *
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or 
 * without modification, are permitted provided that the following 
 * conditions are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsrd, olsr.org nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: olsrd_secure.c,v 1.12 2005/03/10 19:57:48 kattemat Exp $
 */


/*
 * Dynamic linked library for the olsr.org olsr daemon
 */

#include "olsrd_secure.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef linux
#include <linux/in_route.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#ifdef USE_OPENSSL
/* OpenSSL stuff */
#include <openssl/sha.h>

#define CHECKSUM SHA1
#define SCHEME   SHA1_INCLUDING_KEY

#else
/* Homebrewn checksuming */
#include "md5.h"

static void
MD5_checksum(char *, olsr_u16_t, char *);

static void
MD5_checksum(char *data, olsr_u16_t data_len, char *hashbuf)
{
  MD5_CTX context;

  MD5Init(&context);
  MD5Update(&context, data, data_len);
  MD5Final(hashbuf, &context);
}

#define CHECKSUM MD5_checksum
#define SCHEME   MD5_INCLUDING_KEY

#endif

#ifdef OS
#undef OS
#endif

#ifdef WIN32
#define close(x) closesocket(x)
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#define OS "Windows"
#endif
#ifdef linux
#define OS "GNU/Linux"
#endif
#ifdef __FreeBSD__
#define OS "FreeBSD"
#endif

#ifndef OS
#define OS "Undefined"
#endif


#ifdef WIN32

static char *inet_ntop4(const unsigned char *src, char *dst, int size)
{
  static const char fmt[] = "%u.%u.%u.%u";
  char tmp[sizeof "255.255.255.255"];

  if (sprintf(tmp, fmt, src[0], src[1], src[2], src[3]) > size)
    return (NULL);

  return strcpy(dst, tmp);
}

char *inet_ntop(int af, void *src, char *dst, int size)
{
  switch (af)
  {
  case AF_INET:
    return (inet_ntop4(src, dst, size));

  default:
    return (NULL);
  }
}

#endif


/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */

int
olsr_plugin_init()
{
  struct interface *ints;
  int i;


  /* Initialize the timestamp database */
  for(i = 0; i < HASHSIZE; i++)
    {
      timestamps[i].next = &timestamps[i];
      timestamps[i].prev = &timestamps[i];
    }
  olsr_printf(1, "Timestamp database initialized\n");

  if(!strlen(keyfile))
    strcpy(keyfile, KEYFILE);

  i = read_key_from_file(keyfile);

  if(i < 0)
    {
      olsr_printf(1, "[ENC]Could not read key from file %s!\nExitting!\n\n", keyfile);
      exit(1);
    }
  if(i == 0)
    {
      olsr_printf(1, "[ENC]There was a problem reading key from file %s. Is the key long enough?\nExitting!\n\n", keyfile);
      exit(1);
    }

  /* Register the packet transform function */
  add_ptf(&add_signature);

  /* register ifchange function */
  add_ifchgf(&ifchange);

  /* Hijack OLSR socket parser functions */
  ints = ifs;
  while(ints)
    {
      olsr_printf(1, "[ENC]Hijacking %s socket %d\n", ints->int_name, ints->olsr_socket);
      fflush(stdout);
      remove_olsr_socket(ints->olsr_socket, olsr_input);
      add_olsr_socket(ints->olsr_socket, &packet_parser);
     
      /* Reducing maxmessagesize */
      net_reserve_bufspace(ints, sizeof(struct olsrmsg));

      ints = ints->int_next;
    }

  /* Register timeout - poll every 2 seconds */
  olsr_register_scheduler_event(&timeout_timestamps, NULL, 2, 0 , NULL);

  return 1;
}

int
plugin_ipc_init()
{
  return 1;
}


/*
 * destructor - called at unload
 */
void
olsr_plugin_exit()
{
}



/* Mulitpurpose funtion */
int
plugin_io(int cmd, void *data, size_t size)
{

  switch(cmd)
    {
    default:
      return 0;
    }
  
  return 1;
}



/**
 *Scheduled event
 */
void
olsr_event()
{

}


int
ipc_send(char *data, int size)
{
  return 1;
}


/* XXX - ToDo */
int
ifchange(struct interface *ifn, int action)
{

  switch(action)
    {
    case(IFCHG_IF_ADD):
      printf("SEC: interface %s added\n\n", ifn->int_name);
      olsr_printf(1, "[ENC]Hijacking %s socket %d\n", ifn->int_name, ifn->olsr_socket);
      remove_olsr_socket(ifn->olsr_socket, olsr_input);
      add_olsr_socket(ifn->olsr_socket, &packet_parser);
      /* Reducing maxmessagesize */
      net_reserve_bufspace(ifn, sizeof(struct olsrmsg));
      break;

    case(IFCHG_IF_REMOVE):
      printf("SEC: interface %s removed\n\n", ifn->int_name);
      olsr_printf(1, "[ENC]Removing %s socket %d\n", ifn->int_name, ifn->olsr_socket);
      remove_olsr_socket(ifn->olsr_socket, &packet_parser);
      break;

    case(IFCHG_IF_UPDATE):
      printf("SEC: interface %s updated\n\n", ifn->int_name);
      break;
      
    default:
      break;
    }
  return 0;
  
}


void
packet_parser(int fd)
{
  /* sockaddr_in6 is bigger than sockaddr !!!! */
  struct sockaddr_storage from;
  size_t fromlen;
  int cc;
  union olsr_ip_addr from_addr;
  union
  {
    char	buf[MAXMESSAGESIZE+1];
    struct	olsr olsr;
  } inbuf;


  for (;;) 
    {
      fromlen = sizeof(struct sockaddr_storage);

      cc = recvfrom(fd, (void *)&inbuf, sizeof (inbuf), 0, (struct sockaddr *)&from, &fromlen);

      if (cc <= 0) 
	{
	  if (cc < 0 && errno != EWOULDBLOCK)
	    {
	      olsr_printf(1, "[ENC]error recvfrom: %s", strerror(errno));
	    }
	  break;
	}

      if(ipversion == AF_INET)
	{
	  /* IPv4 sender address */
	  memcpy(&from_addr, &((struct sockaddr_in *)&from)->sin_addr.s_addr, ipsize);
	}
      else
	{
	  /* IPv6 sender address */
	  memcpy(&from_addr, &((struct sockaddr_in6 *)&from)->sin6_addr, ipsize);
	}

      /*
      olsr_printf(1, "[ENC]message from %s size %d\n",
		  olsr_ip_to_string(&from_addr),
		  cc);
      */

      /* are we talking to ourselves? */
      if(if_ifwithaddr(&from_addr) != NULL)
      	return;


      /*
       *setting global from addr
       */
      //printf("Recieved data on socket %d\n", socknr);


      if((olsr_in_if = if_ifwithsock(fd)) == NULL)
	{
	  olsr_printf(1, "[ENC]Could not find input interface for message from %s size %d\n",
		      olsr_ip_to_string(&from_addr),
		      cc);
	  return ;
	}

      /*
       * Check for challenge/response messages
       */
      check_auth(inbuf.buf, &cc);

      /*
       * Check signature
       */

      if(!validate_packet(inbuf.buf, &cc))
	{
	  olsr_printf(1, "[ENC]Rejecting packet from %s\n", olsr_ip_to_string((union olsr_ip_addr *)&((struct sockaddr_in *)&from)->sin_addr.s_addr));
	  return;
	}


      olsr_printf(1, "[ENC]Packet from %s OK size %d\n", olsr_ip_to_string((union olsr_ip_addr *)&((struct sockaddr_in *)&from)->sin_addr.s_addr), cc);


      /* Fix OLSR packet header */
      inbuf.olsr.olsr_packlen = htons(cc);

      
      //olsr_printf(1, "Recieved a packet from %s\n", olsr_ip_to_string((union olsr_ip_addr *)&((struct sockaddr_in *)&from)->sin_addr.s_addr));

      //printf("\nCC: %d FROMLEN: %d\n\n", cc, fromlen);
      if ((ipversion == AF_INET) && (fromlen != sizeof (struct sockaddr_in)))
	break;
      else if ((ipversion == AF_INET6) && (fromlen != sizeof (struct sockaddr_in6)))
	break;


      /*
       * &from - sender
       * &inbuf.olsr 
       * cc - bytes read
       */
      parse_packet(&inbuf.olsr, cc, olsr_in_if, &from_addr);
    
    }
}



/**
 * Check a incoming OLSR packet for
 * challenge/responses.
 * They need not be verified as they
 * are signed in the message.
 *
 */
int
check_auth(char *pck, int *size)
{

  olsr_printf(3, "[ENC]Checking packet for challenge response message...\n");

  switch(pck[4])
    {
    case(TYPE_CHALLENGE):
      parse_challenge(&pck[4]);
      break;

    case(TYPE_CRESPONSE):
      parse_cres(&pck[4]);
      break;

    case(TYPE_RRESPONSE):
      parse_rres(&pck[4]);
      break;

    default:
      return 0;
    }

  return 1;
}



/**
 * Packet transform function
 * Build a SHA-1/MD5 hash of the original message
 * + the signature message(-digest) + key
 *
 * Then add the signature message to the packet and
 * increase the size
 */
int
add_signature(char *pck, int *size)
{
  struct olsrmsg *msg;
#ifdef DEBUG
  int i, j;
  char *sigmsg;                                                                                        
#endif
  
  olsr_printf(2, "[ENC]Adding signature for packet size %d\n", *size);
  fflush(stdout);
  
  msg = (struct olsrmsg *)&pck[*size];
  /* Update size */
  ((struct olsr*)pck)->olsr_packlen = htons(*size + sizeof(struct olsrmsg));
  
  /* Fill packet header */
  msg->olsr_msgtype = MESSAGE_TYPE;
  msg->olsr_vtime = 0;
  msg->olsr_msgsize = htons(sizeof(struct olsrmsg));
  memcpy(&msg->originator, main_addr, ipsize);
  msg->ttl = 1;
  msg->hopcnt = 0;
  msg->seqno = htons(get_msg_seqno());
  
  /* Fill subheader */
  msg->sig.type = ONE_CHECKSUM;
  msg->sig.algorithm = SCHEME;
  memset(&msg->sig.reserved, 0, 2);
  
  /* Add timestamp */
  msg->sig.timestamp = htonl(now->tv_sec);
  olsr_printf(3, "[ENC]timestamp: %d\n", now->tv_sec);
  
  /* Set the new size */
  *size = *size + sizeof(struct olsrmsg);
  
  /* Create packet + key cache */
  /* First the OLSR packet + signature message - digest */
  memcpy(checksum_cache, pck, *size - SIGNATURE_SIZE);
  /* Then the key */
  memcpy(&checksum_cache[*size - SIGNATURE_SIZE], aes_key, KEYLENGTH);
  
  /* Create the hash */
  CHECKSUM(checksum_cache, (*size - SIGNATURE_SIZE) + KEYLENGTH, &pck[*size - SIGNATURE_SIZE]);
 

#ifdef DEBUG
  olsr_printf(1, "Signature message:\n");

  j = 0;
  sigmsg = (char *)msg;

  for(i = 0; i < sizeof(struct olsrmsg); i++)
    {
      olsr_printf(1, "  %3i", (u_char) sigmsg[i]);
      j++;
      if(j == 4)
	{
	  olsr_printf(1, "\n");
	  j = 0;
	}
    }
#endif

  olsr_printf(3, "[ENC] Message signed\n");

  return 1;
}



int
validate_packet(char *pck, int *size)
{
  int packetsize;
  char sha1_hash[SIGNATURE_SIZE];
  struct olsrmsg *sig;
  time_t rec_time;

#ifdef DEBUG
  int i, j;
  char *sigmsg;
#endif

  /* Find size - signature message */
  packetsize = *size - sizeof(struct olsrmsg);

  if(packetsize < 4)
    return 0;

  sig = (struct olsrmsg *)&pck[packetsize];

  //olsr_printf(1, "Size: %d\n", packetsize);

#ifdef DEBUG
  olsr_printf(1, "Input message:\n");
  
  j = 0;
  sigmsg = (char *)sig;

  for(i = 0; i < sizeof(struct olsrmsg); i++)
    {
      olsr_printf(1, "  %3i", (u_char) sigmsg[i]);
      j++;
      if(j == 4)
	{
	  olsr_printf(1, "\n");
	  j = 0;
	}
    }
#endif

  /* Sanity check first */
  if((sig->olsr_msgtype != MESSAGE_TYPE) || 
     (sig->olsr_vtime) ||
     (sig->olsr_msgsize != ntohs(sizeof(struct olsrmsg))) ||
     (sig->ttl != 1) ||
     (sig->hopcnt != 0))
    {
      olsr_printf(1, "[ENC]Packet not sane!\n");
      return 0;
    }

  /* Check scheme and type */
  switch(sig->sig.type)
    {
    case(ONE_CHECKSUM):
      switch(sig->sig.algorithm)
	{
	case(SCHEME):
	  goto one_checksum_SHA; /* Ahhh... fix this */
	  break;
	  
	}
      break;

    default:
      olsr_printf(1, "[ENC]Unsupported sceme: %d enc: %d!\n", sig->sig.type, sig->sig.algorithm);
      return 0;
      break;

    }
  //olsr_printf(1, "Packet sane...\n");

 one_checksum_SHA:

  /* Create packet + key cache */
  /* First the OLSR packet + signature message - digest */
  memcpy(checksum_cache, pck, *size - SIGNATURE_SIZE);
  /* Then the key */
  memcpy(&checksum_cache[*size - SIGNATURE_SIZE], aes_key, KEYLENGTH);


  /* generate SHA-1 */
  CHECKSUM(checksum_cache, *size - SIGNATURE_SIZE + KEYLENGTH, sha1_hash);


#ifdef DEBUG
  olsr_printf(1, "Recevied hash:\n");
  
  sigmsg = (char *)sig->sig.signature;

  for(i = 0; i < SIGNATURE_SIZE; i++)
    {
      olsr_printf(1, " %3i", (u_char) sigmsg[i]);
    }
  olsr_printf(1, "\n");

  olsr_printf(1, "Calculated hash:\n");
  
  sigmsg = sha1_hash;

  for(i = 0; i < SIGNATURE_SIZE; i++)
    {
      olsr_printf(1, " %3i", (u_char) sigmsg[i]);
    }
  olsr_printf(1, "\n");
#endif

  if(memcmp(sha1_hash, sig->sig.signature, SIGNATURE_SIZE) != 0)
    {
      olsr_printf(1, "[ENC]Signature missmatch\n");
      return 0;
    }

  /* Check timestamp */
  rec_time = ntohl(sig->sig.timestamp);

  if(!check_timestamp((union olsr_ip_addr *)&sig->originator, rec_time))
    {
      olsr_printf(1, "[ENC]Timestamp missmatch in packet from %s!\n",
		  olsr_ip_to_string((union olsr_ip_addr *)&sig->originator));
      return 0;
    }

  olsr_printf(1, "[ENC]Received timestamp %d diff: %d\n", rec_time, now->tv_sec - rec_time);

  /* Remove signature message */
  *size = packetsize;
  return 1;
}


int
check_timestamp(union olsr_ip_addr *originator, time_t tstamp)
{
  struct stamp *entry;
  int diff;

  entry = lookup_timestamp_entry(originator);

  if(!entry)
    {
      /* Initiate timestamp negotiation */

      send_challenge(originator);

      return 0;
    }

  if(!entry->validated)
    {
      olsr_printf(1, "[ENC]Message from non-validated host!\n");
      return 0;
    }

  diff = entry->diff - (now->tv_sec - tstamp);

  olsr_printf(3, "[ENC]Timestamp slack: %d\n", diff);

  if((diff > UPPER_DIFF) || (diff < LOWER_DIFF))
    {
      olsr_printf(1, "[ENC]Timestamp scew detected!!\n");
      return 0;
    }

  /* ok - update diff */
  entry->diff = ((now->tv_sec - tstamp) + entry->diff) ?
    ((now->tv_sec - tstamp) + entry->diff) / 2 : 0;

  olsr_printf(3, "[ENC]Diff set to : %d\n", entry->diff);

  /* update validtime */
  olsr_get_timestamp((olsr_u32_t) TIMESTAMP_HOLD_TIME*1000, &entry->valtime);

  return 1;
}


/**
 * Create and send a timestamp
 * challenge message to new_host
 *
 * The host is registered in the timestamps
 * repository with valid=0
 */

int
send_challenge(union olsr_ip_addr *new_host)
{
  struct challengemsg cmsg;
  struct stamp *entry;
  olsr_u32_t challenge, hash;

  olsr_printf(1, "[ENC]Building CHALLENGE message\n");

  /* Set the size including OLSR packet size */


  challenge = rand() << 16;
  challenge |= rand();

  /* Fill challengemessage */
  cmsg.olsr_msgtype = TYPE_CHALLENGE;
  cmsg.olsr_vtime = 0;
  cmsg.olsr_msgsize = htons(sizeof(struct challengemsg));
  memcpy(&cmsg.originator, main_addr, ipsize);
  cmsg.ttl = 1;
  cmsg.hopcnt = 0;
  cmsg.seqno = htons(get_msg_seqno());

  /* Fill subheader */
  memcpy(&cmsg.destination, new_host, ipsize);
  cmsg.challenge = htonl(challenge);

  olsr_printf(3, "[ENC]Size: %d\n", sizeof(struct challengemsg));

  /* Create packet + key cache */
  /* First the OLSR packet + signature message - digest */
  memcpy(checksum_cache, &cmsg, sizeof(struct challengemsg) - SIGNATURE_SIZE);
  /* Then the key */
  memcpy(&checksum_cache[sizeof(struct challengemsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);

  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   (sizeof(struct challengemsg) - SIGNATURE_SIZE) + KEYLENGTH, 
	   cmsg.signature);

  olsr_printf(3, "[ENC]Sending timestamp request to %s challenge 0x%x\n", 
	      olsr_ip_to_string(new_host),
	      challenge);

  /* Add to buffer */
  net_outbuffer_push(olsr_in_if, (olsr_u8_t *)&cmsg, sizeof(struct challengemsg));

  /* Send the request */
  net_output(olsr_in_if);

  /* Create new entry */
  entry = malloc(sizeof(struct stamp));
  
  entry->diff = 0;
  entry->validated = 0;
  entry->challenge = challenge;

  memcpy(&entry->addr, new_host, ipsize);

  /* update validtime - not validated */
  olsr_get_timestamp((olsr_u32_t) EXCHANGE_HOLD_TIME*1000, &entry->conftime);
  
  hash = olsr_hashing(new_host);
  
  /* Queue */
  timestamps[hash].next->prev = entry;
  entry->next = timestamps[hash].next;
  timestamps[hash].next = entry;
  entry->prev = &timestamps[hash];


  return 1;

}

int
parse_cres(char *in_msg)
{
  struct c_respmsg *msg;
  char sha1_hash[SIGNATURE_SIZE];
  struct stamp *entry;

  msg = (struct c_respmsg *)in_msg;

  olsr_printf(1, "[ENC]Challenge-response message received\n");
  olsr_printf(3, "[ENC]To: %s\n", olsr_ip_to_string((union olsr_ip_addr *)&msg->destination));

  if(if_ifwithaddr((union olsr_ip_addr *)&msg->destination) == NULL)
    {
      olsr_printf(3, "[ENC]Not for us...\n");
      return 0;
    }

  olsr_printf(3, "[ENC]Challenge: 0x%x\n", ntohl(msg->challenge));

  /* Check signature */

  /* Create packet + key cache */
  /* First the OLSR packet + signature message - digest */
  memcpy(checksum_cache, msg, sizeof(struct c_respmsg) - SIGNATURE_SIZE);
  /* Then the key */
  memcpy(&checksum_cache[sizeof(struct c_respmsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);
  
  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   (sizeof(struct c_respmsg) - SIGNATURE_SIZE) + KEYLENGTH, 
	   sha1_hash);
  
  if(memcmp(sha1_hash, &msg->signature, SIGNATURE_SIZE) != 0)
    {
      olsr_printf(1, "[ENC]Signature missmatch in challenge-response!\n");
      return 0;
    }

  olsr_printf(3, "[ENC]Signature verified\n");


  /* Now to check the digest from the emitted challenge */
  if((entry = lookup_timestamp_entry((union olsr_ip_addr *)&msg->originator)) == NULL)
    {
      olsr_printf(1, "[ENC]Received challenge-response from non-registered node %s!\n",
		  olsr_ip_to_string((union olsr_ip_addr *)&msg->originator));
      return 0;
    }

  /* Generate the digest */
  olsr_printf(3, "[ENC]Entry-challenge 0x%x\n", entry->challenge);

  /* First the challenge received */
  memcpy(checksum_cache, &entry->challenge, 4);
  /* Then the local IP */
  memcpy(&checksum_cache[sizeof(olsr_u32_t)], &msg->originator, ipsize);

  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   sizeof(olsr_u32_t) + ipsize, 
	   sha1_hash);


  if(memcmp(msg->res_sig, sha1_hash, SIGNATURE_SIZE) != 0)
    {
      olsr_printf(1, "[ENC]Error in challenge signature from %s!\n",
		  olsr_ip_to_string((union olsr_ip_addr *)&msg->originator));
      
      return 0;
    }

  olsr_printf(3, "[ENC]Challenge-response signature ok\n");

  /* Update entry! */


  entry->challenge = 0;
  entry->validated = 1;
  entry->diff = now->tv_sec - msg->timestamp;

  /* update validtime - validated entry */
  olsr_get_timestamp((olsr_u32_t) TIMESTAMP_HOLD_TIME*1000, &entry->valtime);

  olsr_printf(1, "[ENC]%s registered with diff %d!\n",
	      olsr_ip_to_string((union olsr_ip_addr *)&msg->originator),
	      entry->diff);

  /* Send response-response */
  send_rres((union olsr_ip_addr *)&msg->originator, 
	    (union olsr_ip_addr *)&msg->destination, 
	    ntohl(msg->challenge));

  return 1;
}


int
parse_rres(char *in_msg)
{
  struct r_respmsg *msg;
  char sha1_hash[SIGNATURE_SIZE];
  struct stamp *entry;

  msg = (struct r_respmsg *)in_msg;

  olsr_printf(1, "[ENC]Response-response message received\n");
  olsr_printf(3, "[ENC]To: %s\n", olsr_ip_to_string((union olsr_ip_addr *)&msg->destination));

  if(if_ifwithaddr((union olsr_ip_addr *)&msg->destination) == NULL)
    {
      olsr_printf(1, "[ENC]Not for us...\n");
      return 0;
    }

  /* Check signature */

  /* Create packet + key cache */
  /* First the OLSR packet + signature message - digest */
  memcpy(checksum_cache, msg, sizeof(struct r_respmsg) - SIGNATURE_SIZE);
  /* Then the key */
  memcpy(&checksum_cache[sizeof(struct r_respmsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);
  
  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   (sizeof(struct r_respmsg) - SIGNATURE_SIZE) + KEYLENGTH, 
	   sha1_hash);
  
  if(memcmp(sha1_hash, &msg->signature, SIGNATURE_SIZE) != 0)
    {
      olsr_printf(1, "[ENC]Signature missmatch in response-response!\n");
      return 0;
    }

  olsr_printf(3, "[ENC]Signature verified\n");


  /* Now to check the digest from the emitted challenge */
  if((entry = lookup_timestamp_entry((union olsr_ip_addr *)&msg->originator)) == NULL)
    {
      olsr_printf(1, "[ENC]Received response-response from non-registered node %s!\n",
		  olsr_ip_to_string((union olsr_ip_addr *)&msg->originator));
      return 0;
    }

  /* Generate the digest */
  olsr_printf(3, "[ENC]Entry-challenge 0x%x\n", entry->challenge);

  /* First the challenge received */
  memcpy(checksum_cache, &entry->challenge, 4);
  /* Then the local IP */
  memcpy(&checksum_cache[sizeof(olsr_u32_t)], &msg->originator, ipsize);

  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   sizeof(olsr_u32_t) + ipsize, 
	   sha1_hash);


  if(memcmp(msg->res_sig, sha1_hash, SIGNATURE_SIZE) != 0)
    {
      olsr_printf(1, "[ENC]Error in response signature from %s!\n",
		  olsr_ip_to_string((union olsr_ip_addr *)&msg->originator));
      
      return 0;
    }

  olsr_printf(3, "[ENC]Challenge-response signature ok\n");

  /* Update entry! */


  entry->challenge = 0;
  entry->validated = 1;
  entry->diff = now->tv_sec - msg->timestamp;

  /* update validtime - validated entry */
  olsr_get_timestamp((olsr_u32_t) TIMESTAMP_HOLD_TIME*1000, &entry->valtime);

  olsr_printf(1, "[ENC]%s registered with diff %d!\n",
	      olsr_ip_to_string((union olsr_ip_addr *)&msg->originator),
	      entry->diff);

  return 1;
}


int
parse_challenge(char *in_msg)
{
  struct challengemsg *msg;
  char sha1_hash[SIGNATURE_SIZE];
  struct stamp *entry;
  olsr_u32_t hash;

  msg = (struct challengemsg *)in_msg;

  olsr_printf(1, "[ENC]Challenge message received\n");
  olsr_printf(3, "[ENC]To: %s\n", olsr_ip_to_string((union olsr_ip_addr *)&msg->destination));

  if(if_ifwithaddr((union olsr_ip_addr *)&msg->destination) == NULL)
    {
      olsr_printf(1, "[ENC]Not for us...\n");
      return 0;
    }

  /* Create entry if not registered */
  if((entry = lookup_timestamp_entry((union olsr_ip_addr *)&msg->originator)) == NULL)
    {
      entry = malloc(sizeof(struct stamp));
      memcpy(&entry->addr, &msg->originator, ipsize);

      hash = olsr_hashing((union olsr_ip_addr *)&msg->originator);
  
      /* Queue */
      timestamps[hash].next->prev = entry;
      entry->next = timestamps[hash].next;
      timestamps[hash].next = entry;
      entry->prev = &timestamps[hash];
    }
  else
    {
      /* Check configuration timeout */
      if(!TIMED_OUT(&entry->conftime))
	{
	  /* If registered - do not accept! */
	  olsr_printf(1, "[ENC]Challenge from registered node...dropping!\n");
	  return 0;
	}
      else
	{
	  olsr_printf(1, "[ENC]Challenge from registered node...accepted!\n");
	}
    }

  olsr_printf(3, "[ENC]Challenge: 0x%x\n", ntohl(msg->challenge));

  /* Check signature */

  /* Create packet + key cache */
  /* First the OLSR packet + signature message - digest */
  memcpy(checksum_cache, msg, sizeof(struct challengemsg) - SIGNATURE_SIZE);
  /* Then the key */
  memcpy(&checksum_cache[sizeof(struct challengemsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);
  
  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   (sizeof(struct challengemsg) - SIGNATURE_SIZE) + KEYLENGTH, 
	   sha1_hash);
  
  if(memcmp(sha1_hash, &msg->signature, SIGNATURE_SIZE) != 0)
    {
      olsr_printf(1, "[ENC]Signature missmatch in challenge!\n");
      return 0;
    }

  olsr_printf(3, "[ENC]Signature verified\n");

  
  entry->diff = 0;
  entry->validated = 0;

  /* update validtime - not validated */
  olsr_get_timestamp((olsr_u32_t) EXCHANGE_HOLD_TIME*1000, &entry->conftime);


  /* Build and send response */

  send_cres((union olsr_ip_addr *)&msg->originator, 
	    (union olsr_ip_addr *)&msg->destination, 
	    ntohl(msg->challenge),
	    entry);

  return 1;
}





/**
 * Build and transmit a challenge response
 * message.
 *
 */
int
send_cres(union olsr_ip_addr *to, union olsr_ip_addr *from, olsr_u32_t chal_in, struct stamp *entry)
{
  struct c_respmsg crmsg;
  olsr_u32_t challenge;

  olsr_printf(1, "[ENC]Building CRESPONSE message\n");

  challenge = rand() << 16;
  challenge |= rand();

  entry->challenge = challenge;

  olsr_printf(3, "[ENC]Challenge-response: 0x%x\n", challenge);

  /* Fill challengemessage */
  crmsg.olsr_msgtype = TYPE_CRESPONSE;
  crmsg.olsr_vtime = 0;
  crmsg.olsr_msgsize = htons(sizeof(struct c_respmsg));
  memcpy(&crmsg.originator, main_addr, ipsize);
  crmsg.ttl = 1;
  crmsg.hopcnt = 0;
  crmsg.seqno = htons(get_msg_seqno());

  /* set timestamp */
  crmsg.timestamp = now->tv_sec;
  olsr_printf(3, "[ENC]Timestamp %d\n", crmsg.timestamp);

  /* Fill subheader */
  memcpy(&crmsg.destination, to, ipsize);
  crmsg.challenge = htonl(challenge);

  /* Create digest of received challenge + IP */

  /* Create packet + key cache */
  /* First the challenge received */
  memcpy(checksum_cache, &chal_in, 4);
  /* Then the local IP */
  memcpy(&checksum_cache[sizeof(olsr_u32_t)], from, ipsize);

  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   sizeof(olsr_u32_t) + ipsize, 
	   crmsg.res_sig);


  /* Now create the digest of the message and the key */

  /* Create packet + key cache */
  /* First the OLSR packet + signature message - digest */
  memcpy(checksum_cache, &crmsg, sizeof(struct c_respmsg) - SIGNATURE_SIZE);
  /* Then the key */
  memcpy(&checksum_cache[sizeof(struct c_respmsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);

  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   (sizeof(struct c_respmsg) - SIGNATURE_SIZE) + KEYLENGTH, 
	   crmsg.signature);

  olsr_printf(3, "[ENC]Sending challenge response to %s challenge 0x%x\n", 
	      olsr_ip_to_string(to),
	      challenge);

  /* Add to buffer */
  net_outbuffer_push(olsr_in_if, (olsr_u8_t *)&crmsg, sizeof(struct c_respmsg));
  /* Send the request */
  net_output(olsr_in_if);

  return 1;
}






/**
 * Build and transmit a response response
 * message.
 *
 */
int
send_rres(union olsr_ip_addr *to, union olsr_ip_addr *from, olsr_u32_t chal_in)
{
  struct r_respmsg rrmsg;

  olsr_printf(1, "[ENC]Building RRESPONSE message\n");


  /* Fill challengemessage */
  rrmsg.olsr_msgtype = TYPE_RRESPONSE;
  rrmsg.olsr_vtime = 0;
  rrmsg.olsr_msgsize = htons(sizeof(struct r_respmsg));
  memcpy(&rrmsg.originator, main_addr, ipsize);
  rrmsg.ttl = 1;
  rrmsg.hopcnt = 0;
  rrmsg.seqno = htons(get_msg_seqno());

  /* set timestamp */
  rrmsg.timestamp = now->tv_sec;
  olsr_printf(3, "[ENC]Timestamp %d\n", rrmsg.timestamp);

  /* Fill subheader */
  memcpy(&rrmsg.destination, to, ipsize);

  /* Create digest of received challenge + IP */

  /* Create packet + key cache */
  /* First the challenge received */
  memcpy(checksum_cache, &chal_in, 4);
  /* Then the local IP */
  memcpy(&checksum_cache[sizeof(olsr_u32_t)], from, ipsize);

  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   sizeof(olsr_u32_t) + ipsize, 
	   rrmsg.res_sig);


  /* Now create the digest of the message and the key */

  /* Create packet + key cache */
  /* First the OLSR packet + signature message - digest */
  memcpy(checksum_cache, &rrmsg, sizeof(struct r_respmsg) - SIGNATURE_SIZE);
  /* Then the key */
  memcpy(&checksum_cache[sizeof(struct r_respmsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);

  /* Create the hash */
  CHECKSUM(checksum_cache, 
	   (sizeof(struct r_respmsg) - SIGNATURE_SIZE) + KEYLENGTH, 
	   rrmsg.signature);

  olsr_printf(3, "[ENC]Sending response response to %s\n", 
	      olsr_ip_to_string(to));

  /* add to buffer */
  net_outbuffer_push(olsr_in_if, (olsr_u8_t *)&rrmsg, sizeof(struct r_respmsg));

  /* Send the request */
  net_output(olsr_in_if);

  return 1;
}



struct stamp *
lookup_timestamp_entry(union olsr_ip_addr *adr)
{
  olsr_u32_t hash;
  struct stamp *entry;

  hash = olsr_hashing(adr);

  for(entry = timestamps[hash].next;
      entry != &timestamps[hash];
      entry = entry->next)
    {
      if(memcmp(&entry->addr, adr, ipsize) == 0)
	{
	  olsr_printf(3, "[ENC]Match for %s\n", olsr_ip_to_string(adr));
	  return entry;
	}
    }

  olsr_printf(1, "[ENC]No match for %s\n", olsr_ip_to_string(adr));

  return NULL;
}



/**
 *Find timed out entries and delete them
 *
 *@return nada
 */
void
timeout_timestamps()
{
  struct stamp *tmp_list;
  struct stamp *entry_to_delete;
  int index;


  for(index=0;index<HASHSIZE;index++)
    {
      tmp_list = timestamps[index].next;
      /*Traverse MID list*/
      while(tmp_list != &timestamps[index])
	{
	  /*Check if the entry is timed out*/
	  if((TIMED_OUT(&tmp_list->valtime)) && (TIMED_OUT(&tmp_list->conftime)))
	    {
	      entry_to_delete = tmp_list;
	      tmp_list = tmp_list->next;

	      olsr_printf(1, "[ENC]timestamp info for %s timed out.. deleting it\n", 
			  olsr_ip_to_string(&entry_to_delete->addr));

	      /*Delete it*/
	      entry_to_delete->next->prev = entry_to_delete->prev;
	      entry_to_delete->prev->next = entry_to_delete->next;

	      free(entry_to_delete);
	    }
	  else
	      tmp_list = tmp_list->next;
	}
    }

  return;
}



int
read_key_from_file(char *file)
{
  FILE *kf;
  size_t keylen;

  keylen = 16;
  kf = fopen(file, "r");

  olsr_printf(1, "[ENC]Reading key from file \"%s\"\n", file);

  if(kf == NULL)
    {
      olsr_printf(1, "[ENC]Could not open keyfile %s!\nError: %s\n", file, strerror(errno));
      return -1;
    }

  if(fread(aes_key, 1, keylen, kf) != keylen)
    {
      olsr_printf(1, "[ENC]Could not read key from keyfile %s!\nError: %s\n", file, strerror(errno));
      fclose(kf);
      return 0;
    }


  fclose(kf);
  return 1;
}


/*************************************************************
 *                 TOOLS DERIVED FROM OLSRD                  *
 *************************************************************/


/**
 *Hashing function. Creates a key based on
 *an 32-bit address.
 *@param address the address to hash
 *@return the hash(a value in the 0-31 range)
 */
olsr_u32_t
olsr_hashing(union olsr_ip_addr *address)
{
  olsr_u32_t hash;
  char *tmp;

  if(ipversion == AF_INET)
    /* IPv4 */  
    hash = (ntohl(address->v4));
  else
    {
      /* IPv6 */
      tmp = (char *) &address->v6;
      hash = (ntohl(*tmp));
    }

  //hash &= 0x7fffffff; 
  hash &= HASHMASK;

  return hash;
}



/**
 *Checks if a timer has times out. That means
 *if it is smaller than present time.
 *@param timer the timeval struct to evaluate
 *@return positive if the timer has not timed out,
 *0 if it matches with present time and negative
 *if it is timed out.
 */
int
olsr_timed_out(struct timeval *timer)
{
  return(timercmp(timer, now, <));
}



/**
 *Initiates a "timer", wich is a timeval structure,
 *with the value given in time_value.
 *@param time_value the value to initialize the timer with
 *@param hold_timer the timer itself
 *@return nada
 */
void
olsr_init_timer(olsr_u32_t time_value, struct timeval *hold_timer)
{ 
  olsr_u16_t  time_value_sec;
  olsr_u16_t  time_value_msec;

  time_value_sec = time_value/1000;
  time_value_msec = time_value-(time_value_sec*1000);

  hold_timer->tv_sec = time_value_sec;
  hold_timer->tv_usec = time_value_msec*1000;   
}





/**
 *Generaties a timestamp a certain number of milliseconds
 *into the future.
 *
 *@param time_value how many milliseconds from now
 *@param hold_timer the timer itself
 *@return nada
 */
void
olsr_get_timestamp(olsr_u32_t delay, struct timeval *hold_timer)
{ 
  olsr_u16_t  time_value_sec;
  olsr_u16_t  time_value_msec;

  time_value_sec = delay/1000;
  time_value_msec= delay - (delay*1000);

  hold_timer->tv_sec = now->tv_sec + time_value_sec;
  hold_timer->tv_usec = now->tv_usec + (time_value_msec*1000);   
}


/**
 *Converts a olsr_ip_addr to a string
 *Goes for both IPv4 and IPv6
 *
 *NON REENTRANT! If you need to use this
 *function twice in e.g. the same printf
 *it will not work.
 *You must use it in different calls e.g.
 *two different printfs
 *
 *@param the IP to convert
 *@return a pointer to a static string buffer
 *representing the address in "dots and numbers"
 *
 */
char *
olsr_ip_to_string(union olsr_ip_addr *addr)
{

  char *ret;
  struct in_addr in;
  
  if(ipversion == AF_INET)
    {
      in.s_addr=addr->v4;
      ret = inet_ntoa(in);
    }
  else
    {
      /* IPv6 */
      ret = (char *)inet_ntop(AF_INET6, &addr->v6, ipv6_buf, sizeof(ipv6_buf));
    }

  return ret;
}


