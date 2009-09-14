/*
 *	Rest in pieces - RIP protocol
 *
 *	Copyright (c) 1999 Pavel Machek <pavel@ucw.cz>
 *	Copyright (c) 2004 Ondrej Filip <feela@network.cz>
 *
 *	Bug fixes by Eric Leblond <eleblond@init-sys.com>, April 2003
 * 
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "lib/socket.h"
#include "lib/resource.h"
#include "lib/lists.h"
#include "lib/timer.h"
#include "lib/md5.h"
#include "lib/string.h"

#include "rip.h"

#define P ((struct rip_proto *) p)
#define P_CF ((struct rip_proto_config *)p->cf)

#define PACKETLEN(num) (num * sizeof(struct rip_block) + sizeof(struct rip_packet_heading))

/*
 * rip_incoming_authentication - check authentication of incomming packet and return 1 if there's problem.
 */
int
rip_incoming_authentication( struct proto *p, struct rip_block_auth *block, struct rip_packet *packet, int num, ip_addr whotoldme )
{
  DBG( "Incoming authentication: " );
  switch (ntohs(block->authtype)) {	/* Authentication type */
  case AT_PLAINTEXT: 
    {
      struct password_item *passwd = password_find(P_CF->passwords, 1);
      DBG( "Plaintext passwd" );
      if (!passwd) {
	log( L_AUTH "No passwords set and password authentication came" );
	return 1;
      }
      if (strncmp( (char *) (&block->packetlen), passwd->password, 16)) {
	log( L_AUTH "Passwd authentication failed!" );
	DBG( "Expected %s, got %.16s\n", passwd->password, &block->packetlen );
	return 1;
      }
    }
    break;
  case AT_MD5:
    DBG( "md5 password" );
    {
      struct password_item *pass = NULL, *ptmp;
      struct rip_md5_tail *tail;
      struct MD5Context ctxt;
      char md5sum_packet[16];
      char md5sum_computed[16];
      struct neighbor *neigh = neigh_find(p, &whotoldme, 0);
      list *l = P_CF->passwords;

      if (ntohs(block->packetlen) != PACKETLEN(num) - sizeof(struct rip_md5_tail) ) {
	log( L_ERR "Packet length in MD5 does not match computed value" );
	return 1;
      }

      tail = (struct rip_md5_tail *) ((char *) packet + (ntohs(block->packetlen) ));
      if ((tail->mustbeFFFF != 0xffff) || (tail->mustbe0001 != 0x0100)) {
	log( L_ERR "MD5 tail signature is not there" );
	return 1;
      }

      WALK_LIST(ptmp, *l)
      {
        if (block->keyid != ptmp->id) continue;
        if ((ptmp->genfrom > now_real) || (ptmp->gento < now_real)) continue;
        pass = ptmp;
        break;
      }

      if(!pass) return 1;

      if (!neigh) {
        log( L_AUTH "Non-neighbour MD5 checksummed packet?" );
      } else {
	if (neigh->aux > block->seq) {
	  log( L_AUTH "MD5 protected packet with lower numbers" );
	  return 1;
        }
	neigh->aux = block->seq;
      }

      memcpy(md5sum_packet, tail->md5, 16);
      password_cpy(tail->md5, pass->password, 16);

      MD5Init(&ctxt);
      MD5Update(&ctxt, (char *) packet, ntohs(block->packetlen) +  sizeof(struct rip_block_auth) );
      MD5Final(md5sum_computed, &ctxt);
      if (memcmp(md5sum_packet, md5sum_computed, 16))
        return 1;
    }
  }
    
  return 0;
}

/*
 * rip_outgoing_authentication - append authentication information to the packet.
 * %num: number of rip_blocks already in packets. This function returns size of packet to send.
 */
int
rip_outgoing_authentication( struct proto *p, struct rip_block_auth *block, struct rip_packet *packet, int num )
{
  struct password_item *passwd = password_find(P_CF->passwords, 1);

  if (!P_CF->authtype)
    return PACKETLEN(num);

  DBG( "Outgoing authentication: " );

  if (!passwd) {
    log( L_ERR "No suitable password found for authentication" );
    return PACKETLEN(num);
  }

  block->authtype = htons(P_CF->authtype);
  block->mustbeFFFF = 0xffff;
  switch (P_CF->authtype) {
  case AT_PLAINTEXT:
    password_cpy( (char *) (&block->packetlen), passwd->password, 16);
    return PACKETLEN(num);
  case AT_MD5:
    {
      struct rip_md5_tail *tail;
      struct MD5Context ctxt;
      static u32 sequence = 0;

      if (num > PACKET_MD5_MAX)
	bug(  "We can not add MD5 authentication to this long packet" );

      /* need to preset the sequence number to a sane value */
      if (!sequence)
	sequence = (u32) time(NULL);

      block->keyid = passwd->id;
      block->authlen = sizeof(struct rip_block_auth);
      block->seq = sequence++;
      block->zero0 = 0;
      block->zero1 = 0;
      block->packetlen = htons(PACKETLEN(num));
      tail = (struct rip_md5_tail *) ((char *) packet + PACKETLEN(num) );
      tail->mustbeFFFF = 0xffff;
      tail->mustbe0001 = 0x0100;

      password_cpy(tail->md5, passwd->password, 16);
      MD5Init(&ctxt);
      MD5Update(&ctxt, (char *) packet, PACKETLEN(num) + sizeof(struct  rip_md5_tail));
      MD5Final(tail->md5, &ctxt);
      return PACKETLEN(num) + block->authlen;
    }
  default:
    bug( "Unknown authtype in outgoing authentication?" );
  }
}
