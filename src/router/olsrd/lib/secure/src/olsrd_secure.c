/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
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
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/* Adjustments made to ensure data going out is converted to network
 * byte ordering.  Also, to ensure incoming data is converted before
 * it is used and before checksums are calculated as well.
 * Rusty Haddock AE5AE -- for the HSMM-MESH project.
 */

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */

#include "olsrd_secure.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#include "defs.h"
#include "ipcalc.h"
#include "olsr.h"
#include "parser.h"
#include "scheduler.h"
#include "net_olsr.h"
#include "olsr_random.h"

#ifdef USE_OPENSSL

/* OpenSSL stuff */
#include <openssl/sha.h>

#define CHECKSUM SHA1
#define SCHEME   SHA1_INCLUDING_KEY

#else /* USE_OPENSSL */

/* Homebrewn checksuming */
#include "md5.h"

static void
MD5_checksum(const uint8_t * data, const uint16_t data_len, uint8_t * hashbuf)
{
  MD5_CTX context;

  MD5Init(&context);
  MD5Update(&context, data, data_len);
  MD5Final(hashbuf, &context);
}

#define CHECKSUM MD5_checksum
#define SCHEME   MD5_INCLUDING_KEY

#endif /* USE_OPENSSL */

#ifdef OS
#undef OS
#endif /* OS */

#ifdef _WIN32
#define close(x) closesocket(x)
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#define OS "Windows"
#endif /* _WIN32 */
#ifdef __linux__
#define OS "GNU/Linux"
#endif /* __linux__ */
#if defined __FreeBSD__ || defined __FreeBSD_kernel__
#define OS "FreeBSD"
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ */

#ifndef OS
#define OS "Undefined"
#endif /* OS */

static struct timeval now;

/* Timestamp node */
struct stamp {
  union olsr_ip_addr addr;
  /* Timestamp difference */
  int diff;
  uint32_t challenge;
  uint8_t validated;
  uint32_t valtime;                     /* Validity time */
  uint32_t conftime;                    /* Reconfiguration time */
  struct stamp *prev;
  struct stamp *next;
};

/* Seconds to cache a valid timestamp entry */
#define TIMESTAMP_HOLD_TIME 30

/* Seconds to cache a not verified timestamp entry */
#define EXCHANGE_HOLD_TIME 5

static struct stamp timestamps[HASHSIZE];

char keyfile[FILENAME_MAX + 1];
char aes_key[16];

/* Event function to register with the sceduler */
static int send_challenge(struct interface_olsr *olsr_if, const union olsr_ip_addr *);
static int send_cres(struct interface_olsr *olsr_if, union olsr_ip_addr *, union olsr_ip_addr *, uint32_t, struct stamp *);
static int send_rres(struct interface_olsr *olsr_if, union olsr_ip_addr *, union olsr_ip_addr *, uint32_t);
static int parse_challenge(struct interface_olsr *olsr_if, char *);
static int parse_cres(struct interface_olsr *olsr_if, char *);
static int parse_rres(char *);
static int check_auth(struct interface_olsr *olsr_if, char *, int *);
static int add_signature(uint8_t *, int *);
static int validate_packet(struct interface_olsr *olsr_if, const char *, int *);
static char *secure_preprocessor(char *packet, struct interface_olsr *olsr_if, union olsr_ip_addr *from_addr, int *length);
static void timeout_timestamps(void *);
static int check_timestamp(struct interface_olsr *olsr_if, const union olsr_ip_addr *, time_t);
static struct stamp *lookup_timestamp_entry(const union olsr_ip_addr *);
static int read_key_from_file(const char *);

/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */

int
secure_plugin_init(void)
{
  int i;

  /* Initialize the timestamp database */
  for (i = 0; i < HASHSIZE; i++) {
    timestamps[i].next = &timestamps[i];
    timestamps[i].prev = &timestamps[i];
  }
  olsr_printf(1, "Timestamp database initialized\n");

  if (!strlen(keyfile))
    strscpy(keyfile, KEYFILE, sizeof(keyfile));

  i = read_key_from_file(keyfile);

  if (i < 0) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "SECURE: Could not read key from file %s", keyfile);
    olsr_exit(buf, EXIT_FAILURE);
  }
  if (i == 0) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "SECURE: There was a problem reading key from file %s. Is the key long enough?", keyfile);
    olsr_exit(buf, EXIT_FAILURE);
  }

  /* Register the packet transform function */
  add_ptf(&add_signature);

  olsr_preprocessor_add_function(&secure_preprocessor);

  /* Register timeout - poll every 2 seconds */
  olsr_start_timer(2 * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC, &timeout_timestamps, NULL, 0);

  return 1;
}

int
plugin_ipc_init(void)
{
  return 1;
}

/*
 * destructor - called at unload
 */
void
secure_plugin_exit(void)
{
  olsr_preprocessor_remove_function(&secure_preprocessor);
}

static char *
secure_preprocessor(char *packet, struct interface_olsr *olsr_if, union olsr_ip_addr *from_addr, int *length)
{
  struct olsr *olsr = (struct olsr *)packet;
  struct ipaddr_str buf;

  /*
   * Check for challenge/response messages
   */
  check_auth(olsr_if, packet, length);

  /*
   * Check signature
   */

  if (!validate_packet(olsr_if, packet, length)) {
    olsr_printf(1, "[ENC]Rejecting packet from %s\n", olsr_ip_to_string(&buf, from_addr));
    return NULL;
  }

  olsr_printf(1, "[ENC]Packet from %s OK size %d\n", olsr_ip_to_string(&buf, from_addr), *length);

  /* Fix OLSR packet header */
  olsr->olsr_packlen = htons(*length);
  return packet;
}

/**
 * Check a incoming OLSR packet for
 * challenge/responses.
 * They need not be verified as they
 * are signed in the message.
 *
 */
static int
check_auth(struct interface_olsr *olsr_if, char *pck, int *size __attribute__ ((unused)))
{

  olsr_printf(3, "[ENC]Checking packet for challenge response message...\n");

  switch (pck[4]) {
  case (TYPE_CHALLENGE):
    parse_challenge(olsr_if, &pck[4]);
    break;

  case (TYPE_CRESPONSE):
    parse_cres(olsr_if, &pck[4]);
    break;

  case (TYPE_RRESPONSE):
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
add_signature(uint8_t * pck, int *size)
{
  struct s_olsrmsg *msg;
#ifdef DEBUG
  unsigned int i;
  int j;
  const uint8_t *sigmsg;
#endif /* DEBUG */

  olsr_printf(2, "[ENC]Adding signature for packet size %d\n", *size);
  fflush(stdout);

  msg = (struct s_olsrmsg *)ARM_NOWARN_ALIGN(&pck[*size]);
  /* Update size */
  ((struct olsr *)pck)->olsr_packlen = htons(*size + sizeof(struct s_olsrmsg));

  /* Fill packet header */
  msg->olsr_msgtype = MESSAGE_TYPE;
  msg->olsr_vtime = 0;
  msg->olsr_msgsize = htons(sizeof(struct s_olsrmsg));
  memcpy(&msg->originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
  msg->ttl = 1;
  msg->hopcnt = 0;
  msg->seqno = htons(get_msg_seqno());

  /* Fill subheader */
  msg->sig.type = ONE_CHECKSUM;
  msg->sig.algorithm = SCHEME;
  memset(&msg->sig.reserved, 0, 2);

  /* Add timestamp */
  msg->sig.timestamp = htonl(now.tv_sec);
#ifndef _WIN32
  olsr_printf(3, "[ENC]timestamp: %lld\n", (long long)now.tv_sec);
#endif /* _WIN32 */
  /* Set the new size */
  *size += sizeof(struct s_olsrmsg);

  {
    uint8_t checksum_cache[1512 + KEYLENGTH];
    /* Create packet + key cache */
    /* First the OLSR packet + signature message - digest */
    memcpy(checksum_cache, pck, *size - SIGNATURE_SIZE);
    /* Then the key */
    memcpy(&checksum_cache[*size - SIGNATURE_SIZE], aes_key, KEYLENGTH);

    /* Create the hash */
    CHECKSUM(checksum_cache, (*size - SIGNATURE_SIZE) + KEYLENGTH, &pck[*size - SIGNATURE_SIZE]);
  }

#ifdef DEBUG
  olsr_printf(1, "Signature message:\n");

  j = 0;
  sigmsg = (uint8_t *) msg;

  for (i = 0; i < sizeof(struct s_olsrmsg); i++) {
    olsr_printf(1, "  %3i", sigmsg[i]);
    j++;
    if (j == 4) {
      olsr_printf(1, "\n");
      j = 0;
    }
  }
#endif /* DEBUG */

  olsr_printf(3, "[ENC] Message signed\n");

  return 1;
}

static int
validate_packet(struct interface_olsr *olsr_if, const char *pck, int *size)
{
  int packetsize;
  uint8_t sha1_hash[SIGNATURE_SIZE];
  const struct s_olsrmsg *sig;
  time_t rec_time;

#ifdef DEBUG
  unsigned int i;
  int j;
  const uint8_t *sigmsg;
#endif /* DEBUG */

  /* Find size - signature message */
  packetsize = *size - sizeof(struct s_olsrmsg);

  if (packetsize < 4)
    return 0;

  sig = (const struct s_olsrmsg *)CONST_ARM_NOWARN_ALIGN(&pck[packetsize]);

  //olsr_printf(1, "Size: %d\n", packetsize);

#ifdef DEBUG
  olsr_printf(1, "Input message:\n");

  j = 0;
  sigmsg = (const uint8_t *)sig;

  for (i = 0; i < sizeof(struct s_olsrmsg); i++) {
    olsr_printf(1, "  %3i", sigmsg[i]);
    j++;
    if (j == 4) {
      olsr_printf(1, "\n");
      j = 0;
    }
  }
#endif /* DEBUG */

  /* Sanity check first */
  if ((sig->olsr_msgtype != MESSAGE_TYPE) || (sig->olsr_vtime != 0)
      || (sig->olsr_msgsize != ntohs(sizeof(struct s_olsrmsg))) || (sig->ttl != 1) || (sig->hopcnt != 0)) {
    olsr_printf(1, "[ENC]Packet not sane!\n");
    return 0;
  }

  /* Check scheme and type */
  switch (sig->sig.type) {
  case (ONE_CHECKSUM):
    switch (sig->sig.algorithm) {
    case (SCHEME):
      goto one_checksum_SHA;    /* Ahhh... fix this */
      break;
    default:
    	break;
    }
    break;

  default:
    olsr_printf(1, "[ENC]Unsupported sceme: %d enc: %d!\n", sig->sig.type, sig->sig.algorithm);
    return 0;
  }
  //olsr_printf(1, "Packet sane...\n");

one_checksum_SHA:

  {
    uint8_t checksum_cache[1512 + KEYLENGTH];
    /* Create packet + key cache */
    /* First the OLSR packet + signature message - digest */
    memcpy(checksum_cache, pck, *size - SIGNATURE_SIZE);
    /* Then the key */
    memcpy(&checksum_cache[*size - SIGNATURE_SIZE], aes_key, KEYLENGTH);

    /* generate SHA-1 */
    CHECKSUM(checksum_cache, *size - SIGNATURE_SIZE + KEYLENGTH, sha1_hash);
  }

#ifdef DEBUG
  olsr_printf(1, "Recevied hash:\n");

  sigmsg = (const uint8_t *)sig->sig.signature;

  for (i = 0; i < SIGNATURE_SIZE; i++) {
    olsr_printf(1, " %3i", sigmsg[i]);
  }
  olsr_printf(1, "\n");

  olsr_printf(1, "Calculated hash:\n");

  sigmsg = sha1_hash;

  for (i = 0; i < SIGNATURE_SIZE; i++) {
    olsr_printf(1, " %3i", sigmsg[i]);
  }
  olsr_printf(1, "\n");
#endif /* DEBUG */

  if (memcmp(sha1_hash, sig->sig.signature, SIGNATURE_SIZE) != 0) {
    olsr_printf(1, "[ENC]Signature missmatch\n");
    return 0;
  }

  /* Check timestamp */
  rec_time = ntohl(sig->sig.timestamp);

  if (!check_timestamp(olsr_if, (const union olsr_ip_addr *)&sig->originator, rec_time)) {
    struct ipaddr_str buf;
    olsr_printf(1, "[ENC]Timestamp missmatch in packet from %s!\n",
                olsr_ip_to_string(&buf, (const union olsr_ip_addr *)&sig->originator));
    return 0;
  }
#ifndef _WIN32
  olsr_printf(1, "[ENC]Received timestamp %lld diff: %lld\n", (long long)rec_time, (long long)now.tv_sec - (long long)rec_time);
#endif /* _WIN32 */
  /* Remove signature message */
  *size = packetsize;
  return 1;
}

int
check_timestamp(struct interface_olsr *olsr_if, const union olsr_ip_addr *originator, time_t tstamp)
{
  struct stamp *entry;
  int diff;

  entry = lookup_timestamp_entry(originator);

  if (!entry) {
    /* Initiate timestamp negotiation */

    send_challenge(olsr_if, originator);

    return 0;
  }

  if (!entry->validated) {
    olsr_printf(1, "[ENC]Message from non-validated host!\n");
    return 0;
  }

  diff = entry->diff - (now.tv_sec - tstamp);

  olsr_printf(3, "[ENC]Timestamp slack: %d\n", diff);

  if ((diff > UPPER_DIFF) || (diff < LOWER_DIFF)) {
    olsr_printf(1, "[ENC]Timestamp scew detected!!\n");
    return 0;
  }

  /* ok - update diff */
  entry->diff = ((now.tv_sec - tstamp) + entry->diff) ? ((now.tv_sec - tstamp) + entry->diff) / 2 : 0;

  olsr_printf(3, "[ENC]Diff set to : %d\n", entry->diff);

  /* update validtime */

  entry->valtime = GET_TIMESTAMP(TIMESTAMP_HOLD_TIME * 1000);

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
send_challenge(struct interface_olsr *olsr_if, const union olsr_ip_addr *new_host)
{
  struct challengemsg cmsg;
  struct stamp *entry;
  uint32_t challenge, hash;
  struct ipaddr_str buf;

  olsr_printf(1, "[ENC]Building CHALLENGE message\n");

  /* Set the size including OLSR packet size */

  challenge = (uint32_t)olsr_random() << 16;
  challenge |= olsr_random();

  /* initialise rrmsg */
  memset(&cmsg, 0, sizeof(cmsg));

  /* Fill challengemessage */
  cmsg.olsr_msgtype = TYPE_CHALLENGE;
  cmsg.olsr_msgsize = htons(sizeof(struct challengemsg));
  memcpy(&cmsg.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
  cmsg.ttl = 1;
  cmsg.seqno = htons(get_msg_seqno());

  /* Fill subheader */
  assert(olsr_cnf->ipsize == sizeof(cmsg.destination));
  memcpy(&cmsg.destination, new_host, olsr_cnf->ipsize);
  cmsg.challenge = htonl(challenge);

  olsr_printf(3, "[ENC]Size: %lu\n", (unsigned long)sizeof(struct challengemsg));

  {
    uint8_t checksum_cache[(sizeof(cmsg) - sizeof(cmsg.signature)) + KEYLENGTH];
    /* Create packet + key cache */
    /* First the OLSR packet + signature message - digest */
    memcpy(checksum_cache, &cmsg, sizeof(cmsg) - sizeof(cmsg.signature));
    /* Then the key */
    memcpy(&checksum_cache[sizeof(cmsg) - sizeof(cmsg.signature)], aes_key, KEYLENGTH);

    /* Create the hash */
    CHECKSUM(checksum_cache, (sizeof(cmsg) - sizeof(cmsg.signature)) + KEYLENGTH, cmsg.signature);
  }
  olsr_printf(3, "[ENC]Sending timestamp request to %s challenge 0x%x\n",
	      olsr_ip_to_string(&buf, new_host), challenge);

  /* Add to buffer */
  net_outbuffer_push(olsr_if, &cmsg, sizeof(struct challengemsg));

  /* Send the request */
  net_output(olsr_if);

  /* Create new entry */
  entry = malloc(sizeof(struct stamp));

  entry->diff = 0;
  entry->validated = 0;
  entry->challenge = challenge;

  memcpy(&entry->addr, new_host, olsr_cnf->ipsize);

  /* update validtime - not validated */
  entry->conftime = GET_TIMESTAMP(EXCHANGE_HOLD_TIME * 1000);

  hash = olsr_ip_hashing(new_host);

  /* Queue */
  timestamps[hash].next->prev = entry;
  entry->next = timestamps[hash].next;
  timestamps[hash].next = entry;
  entry->prev = &timestamps[hash];

  return 1;

}

int
parse_cres(struct interface_olsr *olsr_if, char *in_msg)
{
  struct c_respmsg *msg;
  uint8_t sha1_hash[SIGNATURE_SIZE];
  struct stamp *entry;
  struct ipaddr_str buf;

  msg = (struct c_respmsg *)ARM_NOWARN_ALIGN(in_msg);

  olsr_printf(1, "[ENC]Challenge-response message received\n");
  olsr_printf(3, "[ENC]To: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->destination));

  if (if_ifwithaddr((union olsr_ip_addr *)&msg->destination) == NULL) {
    olsr_printf(3, "[ENC]Not for us...\n");
    return 0;
  }

  olsr_printf(3, "[ENC]Challenge: 0x%lx\n", (unsigned long)ntohl(msg->challenge));      /* ntohl() returns a unsignedlong onwin32 */

  /* Check signature */

  {
    uint8_t checksum_cache[1512 + KEYLENGTH];
    /* Create packet + key cache */
    /* First the OLSR packet + signature message - digest */
    memcpy(checksum_cache, msg, sizeof(struct c_respmsg) - SIGNATURE_SIZE);
    /* Then the key */
    memcpy(&checksum_cache[sizeof(struct c_respmsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);

    /* Create the hash */
    CHECKSUM(checksum_cache, (sizeof(struct c_respmsg) - SIGNATURE_SIZE) + KEYLENGTH, sha1_hash);
  }

  if (memcmp(sha1_hash, &msg->signature, SIGNATURE_SIZE) != 0) {
    olsr_printf(1, "[ENC]Signature missmatch in challenge-response!\n");
    return 0;
  }

  olsr_printf(3, "[ENC]Signature verified\n");

  /* Now to check the digest from the emitted challenge */
  if ((entry = lookup_timestamp_entry((const union olsr_ip_addr *)&msg->originator)) == NULL) {
    olsr_printf(1, "[ENC]Received challenge-response from non-registered node %s!\n",
                olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->originator));
    return 0;
  }

  /* Generate the digest */
  olsr_printf(3, "[ENC]Entry-challenge 0x%x\n", entry->challenge);

  {
    uint8_t checksum_cache[1512 + KEYLENGTH];
    uint32_t netorder_challenge;

    /* First the challenge received */
    /* But we have to calculate our hash with the challenge in
     * network order just like the remote host did!  6-Jun-2011 AE5AE */
    netorder_challenge = htonl(entry->challenge);
    memcpy(checksum_cache, &netorder_challenge, sizeof(uint32_t));
/*     memcpy(checksum_cache, &entry->challenge, 4); */

    /* Then the local IP */
    memcpy(&checksum_cache[sizeof(uint32_t)], &msg->originator, olsr_cnf->ipsize);

    /* Create the hash */
    CHECKSUM(checksum_cache, sizeof(uint32_t) + olsr_cnf->ipsize, sha1_hash);
  }

  if (memcmp(msg->res_sig, sha1_hash, SIGNATURE_SIZE) != 0) {
    olsr_printf(1, "[ENC]Error in challenge signature from %s!\n",
		olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->originator));

    return 0;
  }

  olsr_printf(3, "[ENC]Challenge-response signature ok\n");

  /* Update entry! */

  entry->challenge = 0;
  entry->validated = 1;

  /* Bring timestamp to host order before arith. 2011/05/31 AE5AE */
  entry->diff = now.tv_sec - ntohl(msg->timestamp);

  /* update validtime - validated entry */
  entry->valtime = GET_TIMESTAMP(TIMESTAMP_HOLD_TIME * 1000);

  olsr_printf(1, "[ENC]%s registered with diff %d!\n",
	      olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->originator),
              entry->diff);

  /* Send response-response */
  send_rres(olsr_if, (union olsr_ip_addr *)&msg->originator,
	    (union olsr_ip_addr *)&msg->destination, msg->challenge);
/* 	    (union olsr_ip_addr *)&msg->destination, ntohl(msg->challenge)); */
/* Don't give send_rres() the challenge in host order, as the checksum needs to
 * be calculated in network order.   06-Jun-2011  AE5AE */

  return 1;
}

int
parse_rres(char *in_msg)
{
  struct r_respmsg *msg;
  uint8_t sha1_hash[SIGNATURE_SIZE];
  struct stamp *entry;
  struct ipaddr_str buf;

  msg = (struct r_respmsg *)ARM_NOWARN_ALIGN(in_msg);

  olsr_printf(1, "[ENC]Response-response message received\n");
  olsr_printf(3, "[ENC]To: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->destination));

  if (if_ifwithaddr((union olsr_ip_addr *)&msg->destination) == NULL) {
    olsr_printf(1, "[ENC]Not for us...\n");
    return 0;
  }

  /* Check signature */

  {
    uint8_t checksum_cache[1512 + KEYLENGTH];
    /* Create packet + key cache */
    /* First the OLSR packet + signature message - digest */
    memcpy(checksum_cache, msg, sizeof(struct r_respmsg) - SIGNATURE_SIZE);
    /* Then the key */
    memcpy(&checksum_cache[sizeof(struct r_respmsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);

    /* Create the hash */
    CHECKSUM(checksum_cache, (sizeof(struct r_respmsg) - SIGNATURE_SIZE) + KEYLENGTH, sha1_hash);
  }

  if (memcmp(sha1_hash, &msg->signature, SIGNATURE_SIZE) != 0) {
    olsr_printf(1, "[ENC]Signature missmatch in response-response!\n");
    return 0;
  }

  olsr_printf(3, "[ENC]Signature verified\n");

  /* Now to check the digest from the emitted challenge */
  if ((entry = lookup_timestamp_entry((const union olsr_ip_addr *)&msg->originator)) == NULL) {
    olsr_printf(1, "[ENC]Received response-response from non-registered node %s!\n",
                olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->originator));
    return 0;
  }

  /* Generate the digest */
  olsr_printf(3, "[ENC]Entry-challenge 0x%x\n", entry->challenge);

  {
    uint8_t checksum_cache[1512 + KEYLENGTH];
    uint32_t netorder_challenge;

    /* First the challenge received */
    /* But we have to calculate our hash with the challenge in network order!  6-Jun-2011 AE5AE */
    netorder_challenge = htonl(entry->challenge);
    memcpy(checksum_cache, &netorder_challenge, sizeof(uint32_t));
/*     memcpy(checksum_cache, &entry->challenge, 4); */

    /* Then the local IP */
    memcpy(&checksum_cache[sizeof(uint32_t)], &msg->originator, olsr_cnf->ipsize);

    /* Create the hash */
    CHECKSUM(checksum_cache, sizeof(uint32_t) + olsr_cnf->ipsize, sha1_hash);
  }

  if (memcmp(msg->res_sig, sha1_hash, SIGNATURE_SIZE) != 0) {
    olsr_printf(1, "[ENC]Error in response signature from %s!\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->originator));

    return 0;
  }

  olsr_printf(3, "[ENC]Challenge-response signature ok\n");

  /* Update entry! */

  entry->challenge = 0;
  entry->validated = 1;

  /* Bring timestamp to host order before arith. 2011/05/31 AE5AE */
  entry->diff = now.tv_sec - ntohl(msg->timestamp);


  /* update validtime - validated entry */
  entry->valtime = GET_TIMESTAMP(TIMESTAMP_HOLD_TIME * 1000);

  olsr_printf(1, "[ENC]%s registered with diff %d!\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->originator),
              entry->diff);

  return 1;
}

int
parse_challenge(struct interface_olsr *olsr_if, char *in_msg)
{
  struct challengemsg *msg;
  uint8_t sha1_hash[SIGNATURE_SIZE];
  struct stamp *entry;
  uint32_t hash;
  struct ipaddr_str buf;

  msg = (struct challengemsg *)ARM_NOWARN_ALIGN(in_msg);

  olsr_printf(1, "[ENC]Challenge message received\n");
  olsr_printf(3, "[ENC]To: %s\n", olsr_ip_to_string(&buf, (union olsr_ip_addr *)&msg->destination));

  if (if_ifwithaddr((union olsr_ip_addr *)&msg->destination) == NULL) {
    olsr_printf(1, "[ENC]Not for us...\n");
    return 0;
  }

  /* Create entry if not registered */
  if ((entry = lookup_timestamp_entry((const union olsr_ip_addr *)&msg->originator)) == NULL) {
    entry = malloc(sizeof(struct stamp));
    memcpy(&entry->addr, &msg->originator, olsr_cnf->ipsize);

    hash = olsr_ip_hashing((union olsr_ip_addr *)&msg->originator);

    /* Queue */
    timestamps[hash].next->prev = entry;
    entry->next = timestamps[hash].next;
    timestamps[hash].next = entry;
    entry->prev = &timestamps[hash];
  } else {
    /* Check configuration timeout */
    if (!TIMED_OUT(entry->conftime)) {
      /* If registered - do not accept! */
      olsr_printf(1, "[ENC]Challenge from registered node...dropping!\n");
      return 0;
    } else {
      olsr_printf(1, "[ENC]Challenge from registered node...accepted!\n");
    }
  }

  olsr_printf(3, "[ENC]Challenge: 0x%lx\n", (unsigned long)ntohl(msg->challenge));      /* ntohl() returns a unsignedlong onwin32 */

  /* Check signature */

  {
    uint8_t checksum_cache[1512 + KEYLENGTH];
    /* Create packet + key cache */
    /* First the OLSR packet + signature message - digest */
    memcpy(checksum_cache, msg, sizeof(struct challengemsg) - SIGNATURE_SIZE);
    /* Then the key */
    memcpy(&checksum_cache[sizeof(struct challengemsg) - SIGNATURE_SIZE], aes_key, KEYLENGTH);

    /* Create the hash */
    CHECKSUM(checksum_cache, (sizeof(struct challengemsg) - SIGNATURE_SIZE) + KEYLENGTH, sha1_hash);
  }
  if (memcmp(sha1_hash, &msg->signature, SIGNATURE_SIZE) != 0) {
    olsr_printf(1, "[ENC]Signature missmatch in challenge!\n");
    return 0;
  }

  olsr_printf(3, "[ENC]Signature verified\n");

  entry->diff = 0;
  entry->validated = 0;

  /* update validtime - not validated */
  entry->conftime = GET_TIMESTAMP(EXCHANGE_HOLD_TIME * 1000);

  /* Build and send response */

  send_cres(olsr_if, (union olsr_ip_addr *)&msg->originator,
	    (union olsr_ip_addr *)&msg->destination, msg->challenge, entry);
/* 	    (union olsr_ip_addr *)&msg->destination, ntohl(msg->challenge), entry); */
/* Don't give send_cres() the challenge in host order, as the checksum needs to
 * be calculated with network order.   06-Jun-2011  AE5AE */

  return 1;
}

/**
 * Build and transmit a challenge response
 * message.
 *
 */
int
send_cres(struct interface_olsr *olsr_if, union olsr_ip_addr *to, union olsr_ip_addr *from, uint32_t chal_in, struct stamp *entry)
{
  struct c_respmsg crmsg;
  uint32_t challenge;
  struct ipaddr_str buf;

  olsr_printf(1, "[ENC]Building CRESPONSE message\n");

  challenge = olsr_random() << 16;
  challenge |= olsr_random();

  entry->challenge = challenge;

  olsr_printf(3, "[ENC]Challenge-response: 0x%x\n", challenge);

  /* initialise rrmsg */
  memset(&crmsg, 0, sizeof(crmsg));

  /* Fill challengemessage */
  crmsg.olsr_msgtype = TYPE_CRESPONSE;
  crmsg.olsr_msgsize = htons(sizeof(struct c_respmsg));
  memcpy(&crmsg.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
  crmsg.ttl = 1;
  crmsg.seqno = htons(get_msg_seqno());

  /* set timestamp */
  /* but swap the byte order to the network order before sending!  2011/05/28 AE5AE */
  crmsg.timestamp = htonl(now.tv_sec);
#ifndef _WIN32
  /* Don't print htonl()'d time, use now.tv_sec 2011/05/31 AE5AE */
/*   olsr_printf(3, "[ENC]Timestamp %lld\n", (long long)crmsg.timestamp); */
  olsr_printf(3, "[ENC]Timestamp %lld\n", (long long)now.tv_sec);
#endif /* _WIN32 */

  /* Fill subheader */
  assert(olsr_cnf->ipsize == sizeof(crmsg.destination));
  memcpy(&crmsg.destination, to, olsr_cnf->ipsize);
  crmsg.challenge = htonl(challenge);

  /* Create digest of received challenge + IP */

  {
    uint8_t checksum_cache[sizeof(chal_in) + olsr_cnf->ipsize];
    /* Create packet + key cache */
    /* First the challenge received */
    memcpy(checksum_cache, &chal_in, sizeof(chal_in));
    /* Then the local IP */
    memcpy(&checksum_cache[sizeof(chal_in)], from, olsr_cnf->ipsize);

    /* Create the hash */
    CHECKSUM(checksum_cache, sizeof(chal_in) + olsr_cnf->ipsize, crmsg.res_sig);
  }

  /* Now create the digest of the message and the key */

  {
    uint8_t checksum_cache[(sizeof(crmsg) - sizeof(crmsg.signature)) + KEYLENGTH];
    /* Create packet + key cache */
    /* First the OLSR packet + signature message - digest */
    memcpy(checksum_cache, &crmsg, sizeof(crmsg) - sizeof(crmsg.signature));
    /* Then the key */
    memcpy(&checksum_cache[sizeof(crmsg) - sizeof(crmsg.signature)], aes_key, KEYLENGTH);

    /* Create the hash */
    CHECKSUM(checksum_cache, (sizeof(crmsg) - sizeof(crmsg.signature)) + KEYLENGTH, crmsg.signature);
  }

  olsr_printf(3, "[ENC]Sending challenge response to %s challenge 0x%x\n", olsr_ip_to_string(&buf, to), challenge);

  /* Add to buffer */
  net_outbuffer_push(olsr_if, &crmsg, sizeof(struct c_respmsg));
  /* Send the request */
  net_output(olsr_if);

  return 1;
}

/**
 * Build and transmit a response response
 * message.
 *
 */
static int
send_rres(struct interface_olsr *olsr_if, union olsr_ip_addr *to, union olsr_ip_addr *from, uint32_t chal_in)
{
  struct r_respmsg rrmsg;
  struct ipaddr_str buf;

  olsr_printf(1, "[ENC]Building RRESPONSE message\n");

  /* initialise rrmsg */
  memset(&rrmsg, 0, sizeof(rrmsg));

  /* Fill challengemessage */
  rrmsg.olsr_msgtype = TYPE_RRESPONSE;
  rrmsg.olsr_msgsize = htons(sizeof(struct r_respmsg));
  memcpy(&rrmsg.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
  rrmsg.ttl = 1;
  rrmsg.seqno = htons(get_msg_seqno());

  /* set timestamp */
  /* But swap the byte order to the network order!  2011/05/28 AE5AE */
  rrmsg.timestamp = htonl(now.tv_sec);

#ifndef _WIN32
  /* olsr_printf(3, "[ENC]Timestamp %lld\n", (long long)rrmsg.timestamp); */
  /* don't print htonl()'d time, use now. 2011/05/31 AE5AE */
  olsr_printf(3, "[ENC]Timestamp %lld\n", (long long)now.tv_sec);
#endif /* _WIN32 */
  /* Fill subheader */
  assert(olsr_cnf->ipsize == sizeof(rrmsg.destination));
  memcpy(&rrmsg.destination, to, olsr_cnf->ipsize);

  /* Create digest of received challenge + IP */

  {
    uint8_t checksum_cache[sizeof(chal_in) + sizeof(union olsr_ip_addr)];
    /* Create packet + key cache */
    /* First the challenge received */
    memcpy(checksum_cache, &chal_in, sizeof(chal_in));
    /* Then the local IP */
    memcpy(&checksum_cache[sizeof(chal_in)], from, olsr_cnf->ipsize);

    /* Create the hash */
    CHECKSUM(checksum_cache, sizeof(chal_in) + olsr_cnf->ipsize, rrmsg.res_sig);
  }

  /* Now create the digest of the message and the key */

  {
    uint8_t checksum_cache[(sizeof(rrmsg) - sizeof(rrmsg.signature)) + KEYLENGTH];
    /* Create packet + key cache */
    /* First the OLSR packet + signature message - digest */
    memcpy(checksum_cache, &rrmsg, sizeof(rrmsg) - sizeof(rrmsg.signature));
    /* Then the key */
    memcpy(&checksum_cache[sizeof(rrmsg) - sizeof(rrmsg.signature)], aes_key, KEYLENGTH);

    /* Create the hash */
    CHECKSUM(checksum_cache, (sizeof(rrmsg) - sizeof(rrmsg.signature)) + KEYLENGTH, rrmsg.signature);
  }

  olsr_printf(3, "[ENC]Sending response response to %s\n", olsr_ip_to_string(&buf, to));

  /* add to buffer */
  net_outbuffer_push(olsr_if, &rrmsg, sizeof(struct r_respmsg));

  /* Send the request */
  net_output(olsr_if);

  return 1;
}

static struct stamp *
lookup_timestamp_entry(const union olsr_ip_addr *adr)
{
  uint32_t hash;
  struct stamp *entry;
  struct ipaddr_str buf;

  hash = olsr_ip_hashing(adr);

  for (entry = timestamps[hash].next; entry != &timestamps[hash]; entry = entry->next) {
    if (memcmp(&entry->addr, adr, olsr_cnf->ipsize) == 0) {
      olsr_printf(3, "[ENC]Match for %s\n", olsr_ip_to_string(&buf, adr));
      return entry;
    }
  }

  olsr_printf(1, "[ENC]No match for %s\n", olsr_ip_to_string(&buf, adr));

  return NULL;
}

/**
 *Find timed out entries and delete them
 *
 *@return nada
 */
void
timeout_timestamps(void *foo __attribute__ ((unused)))
{
  struct stamp *tmp_list;
  struct stamp *entry_to_delete;
  int idx;

  /* Update our local timestamp */
  gettimeofday(&now, NULL);

  for (idx = 0; idx < HASHSIZE; idx++) {
    tmp_list = timestamps[idx].next;
    /*Traverse MID list */
    while (tmp_list != &timestamps[idx]) {
      /*Check if the entry is timed out */
      if ((TIMED_OUT(tmp_list->valtime)) && (TIMED_OUT(tmp_list->conftime))) {
        struct ipaddr_str buf;
        entry_to_delete = tmp_list;
        tmp_list = tmp_list->next;

        olsr_printf(1, "[ENC]timestamp info for %s timed out.. deleting it\n",
		    olsr_ip_to_string(&buf, &entry_to_delete->addr));

        /*Delete it */
        entry_to_delete->next->prev = entry_to_delete->prev;
        entry_to_delete->prev->next = entry_to_delete->next;

        free(entry_to_delete);
      } else
        tmp_list = tmp_list->next;
    }
  }

  return;
}

static int
read_key_from_file(const char *file)
{
  FILE *kf;
  size_t keylen;

  keylen = 16;
  kf = fopen(file, "r");

  olsr_printf(1, "[ENC]Reading key from file \"%s\"\n", file);

  if (kf == NULL) {
    olsr_printf(1, "[ENC]Could not open keyfile %s!\nError: %s\n", file, strerror(errno));
    return -1;
  }

  if (fread(aes_key, 1, keylen, kf) != keylen) {
    olsr_printf(1, "[ENC]Could not read key from keyfile %s!\nError: %s\n", file, strerror(errno));
    fclose(kf);
    return 0;
  }

  fclose(kf);
  return 1;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
