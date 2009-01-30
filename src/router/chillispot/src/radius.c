/* 
 * Radius client functions.
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * 
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h> /* in_addr */
#include <stdlib.h>     /* calloc */
#include <stdio.h>      /* sscanf */
#include <string.h>     /* memcpy */
#include <syslog.h>
#include <sys/time.h>

#include "syserr.h"
#include "radius.h"
#include "md5.h"

int radius_printqueue(struct radius_t *this) {
  int n;
  printf("next %d, first %d, last %d\n", 
	 this->next, this->first, this ->last);

  for(n=0; n<256; n++) {
    if (this->queue[n].state) {
      printf("%3d %3d %3d %3d %8d %8d %d\n",
	     n, this->queue[n].state,
	     this->queue[n].next,
	     this->queue[n].prev,
	     (int) this->queue[n].timeout.tv_sec,
	     (int) this->queue[n].timeout.tv_usec,
	     (int) this->queue[n].retrans);
    }
  }
  return 0;
}

/* 
 * radius_hmac_md5()
 * Calculate HMAC MD5 on a radius packet. 
 */
int radius_hmac_md5(struct radius_t *this, struct radius_packet_t *pack,
		uint8_t *dst) {
  unsigned char digest[RADIUS_MD5LEN];
  int length;

  MD5_CTX context;

  uint8_t *key;
  int key_len;

  unsigned char k_ipad[65];
  unsigned char k_opad[65];
  unsigned char tk[RADIUS_MD5LEN];
  int i;

  if (this->secretlen > 64) { /* TODO: If Microsoft truncate to 64 instead */
    MD5Init(&context);
    MD5Update(&context, (uint8_t*) this->secret, this->secretlen);
    MD5Final(tk, &context);
    key = tk;
    key_len = 16;
  }
  else {
    key = (uint8_t*) this->secret;
    key_len = this->secretlen;
  }
  

  length = ntohs(pack->length);

  memset(k_ipad, 0x36, sizeof k_ipad);
  memset(k_opad, 0x5c, sizeof k_opad);

  for (i=0; i<key_len; i++) {
    k_ipad[i] ^= key[i];
    k_opad[i] ^= key[i];
  }

  /* Perform inner MD5 */
  MD5Init(&context);
  MD5Update(&context, k_ipad, 64);
  MD5Update(&context, (uint8_t*) pack, length);
  MD5Final(digest, &context);

  /* Perform outer MD5 */
  MD5Init(&context);
  MD5Update(&context, k_opad, 64);
  MD5Update(&context, digest, 16);
  MD5Final(digest, &context);
  
  memcpy(dst, digest, RADIUS_MD5LEN);

  return 0;
}

/* 
 * radius_acctreq_authenticator()
 * Update a packet with an accounting request authenticator
 */
int radius_acctreq_authenticator(struct radius_t *this,
				 struct radius_packet_t *pack) {

  /* From RFC 2866: Authenticator is the MD5 hash of:
     Code + Identifier + Length + 16 zero octets + request attributes +
     shared secret */
  
  MD5_CTX context;

  memset(pack->authenticator, 0, RADIUS_AUTHLEN);

  /* Get MD5 hash on secret + authenticator */
  MD5Init(&context);
  MD5Update(&context, (void*) pack, ntohs(pack->length));
  MD5Update(&context, (uint8_t*) this->secret, this->secretlen);
  MD5Final(pack->authenticator, &context);
  
  return 0;
}


/* 
 * radius_authresp_authenticator()
 * Update a packet with an authentication response authenticator
 */
int radius_authresp_authenticator(struct radius_t *this,
				 struct radius_packet_t *pack,
				 uint8_t *req_auth,
				 char *secret, int secretlen) {

  /* From RFC 2865: Authenticator is the MD5 hash of:
     Code + Identifier + Length + request authenticator + request attributes +
     shared secret */
  
  MD5_CTX context;

  memcpy(pack->authenticator, req_auth, RADIUS_AUTHLEN);

  /* Get MD5 hash on secret + authenticator */
  MD5Init(&context);
  MD5Update(&context, (void*) pack, ntohs(pack->length));
  MD5Update(&context, (uint8_t*) secret, secretlen);
  MD5Final(pack->authenticator, &context);
  
  return 0;
}


/* 
 * radius_queue_in()
 * Place data in queue for later retransmission.
 */
int radius_queue_in(struct radius_t *this, struct radius_packet_t *pack,
		    void *cbp) {
  struct timeval *tv;
  struct radius_attr_t *ma = NULL; /* Message authenticator */

  if (this->debug) printf("radius_queue_in\n");

  if (this->debug) {
    printf("radius_queue_in\n");
    radius_printqueue(this);
  }

  if (this->queue[this->next].state == 1) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius queue is full!");
    /* Queue is not really full. It only means that the next space
       in queue is not available, but there might be space elsewhere */
    return -1;
  }

  pack->id = this->next;

  /* If packet contains message authenticator: Calculate it! */
  if (!radius_getattr(pack, &ma, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 0,0,0)) {
    radius_hmac_md5(this, pack, ma->v.t);
  }
  
  /* If accounting request: Calculate authenticator */
  if (pack->code == RADIUS_CODE_ACCOUNTING_REQUEST)
    radius_acctreq_authenticator(this, pack);

  memcpy(&this->queue[this->next].p, pack, RADIUS_PACKSIZE);
  this->queue[this->next].state = 1;
  this->queue[this->next].cbp = cbp;
  this->queue[this->next].retrans = 0;
  tv = &this->queue[this->next].timeout;
  gettimeofday(tv, NULL);
  tv->tv_usec += RADIUS_TIMEOUT;
  tv->tv_sec  += tv->tv_usec / 1000000;
  tv->tv_usec = tv->tv_usec % 1000000;
  this->queue[this->next].lastsent = this->lastreply;

  /* Insert in linked list for handling timeouts */
  this->queue[this->next].next = -1;         /* Last in queue */
  this->queue[this->next].prev = this->last; /* Link to previous */

  if (this->last != -1)
    this->queue[this->last].next=this->next; /* Link previous to us */
  this->last = this->next;                   /* End of queue */

  if (this->first == -1)
    this->first = this->next; /* First and last */

  this->next++; /* next = next % RADIUS_QUEUESIZE */

  if (this->debug) {
    printf("radius_queue_out end\n");
    radius_printqueue(this);
  }

  return 0;
}


/* 
 * radius_queue_in()
 * Remove data from queue.
 */
int radius_queue_out(struct radius_t *this, struct radius_packet_t *pack,
		     int id, void **cbp) {

  if (this->debug) if (this->debug) printf("radius_queue_out\n");

  if (this->queue[id].state != 1) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No such id in radius queue: %d!", id);
    return -1;
  }

  if (this->debug) {
    printf("radius_queue_out\n");
    radius_printqueue(this);
  }
  
  memcpy(pack, &this->queue[id].p, RADIUS_PACKSIZE);
  *cbp = this->queue[id].cbp;

  this->queue[id].state = 0;

  /* Remove from linked list */
  if (this->queue[id].next == -1) /* Are we the last in queue? */
    this->last = this->queue[id].prev;
  else
    this->queue[this->queue[id].next].prev = this->queue[id].prev;
    
  if (this->queue[id].prev == -1) /* Are we the first in queue? */
    this->first = this->queue[id].next;
  else
    this->queue[this->queue[id].prev].next = this->queue[id].next;

  if (this->debug) {
    printf("radius_queue_out end\n");
    radius_printqueue(this);
  }

  return 0;
}

/* 
 * radius_queue_reschedule()
 * Recalculate the timeout value of a packet in the queue.
 */
int radius_queue_reschedule(struct radius_t *this, int id) {
  struct timeval *tv;
  if (this->debug) printf("radius_queue_reschedule\n");

  if (this->queue[id].state != 1) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No such id in radius queue: %d!", id);
    return -1;
  }

  if (this->debug) {
    printf("radius_reschedule\n");
    radius_printqueue(this);
  }

  this->queue[id].retrans++;
  tv = &this->queue[id].timeout;
  gettimeofday(tv, NULL);
  tv->tv_usec += RADIUS_TIMEOUT;
  tv->tv_sec  += tv->tv_usec / 1000000;
  tv->tv_usec = tv->tv_usec % 1000000;

  /* Remove from linked list */
  if (this->queue[id].next == -1) /* Are we the last in queue? */
    this->last = this->queue[id].prev;
  else
    this->queue[this->queue[id].next].prev = this->queue[id].prev;
    
  if (this->queue[id].prev == -1) /* Are we the first in queue? */
    this->first = this->queue[id].next;
  else
    this->queue[this->queue[id].prev].next = this->queue[id].next;

  /* Insert in linked list for handling timeouts */
  this->queue[id].next = -1;         /* Last in queue */
  this->queue[id].prev = this->last; /* Link to previous (could be -1) */

  if (this->last != -1)
    this->queue[this->last].next=id; /* If not empty: link previous to us */
  this->last = id;                   /* End of queue */

  if (this->first == -1)
    this->first = id;                /* First and last */

  if (this->debug) {
    radius_printqueue(this);
  }

  return 0;
}


/* 
 * radius_cmptv()
 * Returns an integer less than, equal to or greater than zero if tv1
 * is found, respectively, to be less than, to match or be greater than tv2.
 */
int 
radius_cmptv(struct timeval *tv1, struct timeval *tv2)
{
  struct timeval diff;

  if (0) {
    printf("tv1 %8d %8d tv2 %8d %8d\n", 
	   (int) tv1->tv_sec, (int) tv1->tv_usec, 
	   (int) tv2->tv_sec, (int) tv2->tv_usec);
  }
  

  /* First take the difference with |usec| < 1000000 */
  diff.tv_sec = (tv1->tv_usec  - tv2->tv_usec) / 1000000 +
                (tv1->tv_sec   - tv2->tv_sec);
  diff.tv_usec = (tv1->tv_usec - tv2->tv_usec) % 1000000;

  if (0) {
    printf("tv1 %8d %8d tv2 %8d %8d diff %8d %8d\n", 
	   (int) tv1->tv_sec, (int) tv1->tv_usec, 
	   (int) tv2->tv_sec, (int) tv2->tv_usec, 
	   (int) diff.tv_sec, (int) diff.tv_usec);
  }

  /* If sec and usec have different polarity add or subtract 1 second */
  if ((diff.tv_sec > 0) & (diff.tv_usec < 0)) {
    diff.tv_sec--;
    diff.tv_usec += 1000000;
  }
  if ((diff.tv_sec < 0) & (diff.tv_usec > 0)) {
    diff.tv_sec++;
    diff.tv_usec -= 1000000;
  }
  if (0) {
    printf("tv1 %8d %8d tv2 %8d %8d diff %8d %8d\n", 
	   (int) tv1->tv_sec, (int) tv1->tv_usec, 
	   (int) tv2->tv_sec, (int) tv2->tv_usec, 
	   (int) diff.tv_sec, (int) diff.tv_usec);
  }

  if (diff.tv_sec < 0) {if (0) printf("-1\n"); return -1; }
  if (diff.tv_sec > 0) {if (0) printf("1\n"); return  1; }

  if (diff.tv_usec < 0) {if (0) printf("-1\n"); return -1;}
  if (diff.tv_usec > 0) {if (0) printf("1\n"); return  1;}
  if (0) printf("0 \n");
  return 0;

}


/* 
 * radius_timeleft()
 * Determines how nuch time is left until we need to call 
 * radius_timeout().
 * Only modifies timeout if new value is lower than current value.
 */
int 
radius_timeleft(struct radius_t *this, struct timeval *timeout) 
{
  struct timeval now, later, diff;

  if (this->first == -1) /* Queue is empty */
    return 0;

  gettimeofday(&now, NULL);
  later.tv_sec = this->queue[this->first].timeout.tv_sec;
  later.tv_usec = this->queue[this->first].timeout.tv_usec;

  /* First take the difference with |usec| < 1000000 */
  diff.tv_sec = (later.tv_usec  - now.tv_usec) / 1000000 +
                (later.tv_sec   - now.tv_sec);
  diff.tv_usec = (later.tv_usec - now.tv_usec) % 1000000;

  /* If sec and usec have different polarity add or subtract 1 second */
  if ((diff.tv_sec > 0) & (diff.tv_usec < 0)) {
    diff.tv_sec--;
    diff.tv_usec += 1000000;
  }
  if ((diff.tv_sec < 0) & (diff.tv_usec > 0)) {
    diff.tv_sec++;
    diff.tv_usec -= 1000000;
  }

  /* If negative set to zero */
  if ((diff.tv_sec < 0) || (diff.tv_usec < 0)) {
    diff.tv_sec = 0;
    diff.tv_usec = 0;
  }

  /* If original was smaller do nothing */
  if (radius_cmptv(timeout, &diff) <=0) 
    return 0;

  timeout->tv_sec = diff.tv_sec;
  timeout->tv_usec = diff.tv_usec;
  return 0;
}

/* 
 * radius_timeout()
 * Retransmit any outstanding packets. This function should be called at
 * regular intervals. Use radius_timeleft() to determine how much time is 
 * left before this function should be called.
 */
int radius_timeout(struct radius_t *this) {
  /* Retransmit any outstanding packets */
  /* Remove from queue if maxretrans exceeded */
  struct timeval now;
  struct sockaddr_in addr;
  struct radius_packet_t pack_req;
  void *cbp;

  /*printf("Retrans: New beginning %d\n", (int) now);*/

  gettimeofday(&now, NULL);

  if (this->debug) {
    printf("radius_timeout %8d %8d\n", 
	   (int) now.tv_sec, (int) now.tv_usec);
    radius_printqueue(this);
  }

  while ((this->first!=-1) && 
	 (radius_cmptv(&now, &this->queue[this->first].timeout) >= 0)) {
    
    if (this->queue[this->first].retrans < RADIUS_RETRY2) {
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      
      if (this->queue[this->first].retrans == (RADIUS_RETRY1-1)) {
	/* Use the other server for next retransmission */
	if (this->queue[this->first].lastsent) {
	  addr.sin_addr = this->hisaddr0;
	  this->queue[this->first].lastsent = 0;
	}
	else {
	  addr.sin_addr = this->hisaddr1;
	  this->queue[this->first].lastsent = 1;
	}
      } 
      else {
	/* Use the same server for next retransmission */
	if (this->queue[this->first].lastsent) {
	  addr.sin_addr = this->hisaddr1;
	}
	else {
	  addr.sin_addr = this->hisaddr0;
	}
      }
      
      /* Use the correct port for accounting and authentication */
      if (this->queue[this->first].p.code == RADIUS_CODE_ACCOUNTING_REQUEST)
	addr.sin_port = htons(this->acctport);
      else
	addr.sin_port = htons(this->authport);
      
      
      if (sendto(this->fd, &this->queue[this->first].p,
		 ntohs(this->queue[this->first].p.length), 0,
		 (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno,
		"sendto() failed!");
	radius_queue_reschedule(this, this->first);
	return -1;
      }
      radius_queue_reschedule(this, this->first);
    }
    else { /* Finished retrans */
      if (radius_queue_out(this, &pack_req, this->first, &cbp)) {
	sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
		"Matching request was not found in queue: %d!", this->first);
	return -1;
      }

      if ((pack_req.code == RADIUS_CODE_ACCOUNTING_REQUEST) &&
	  (this->cb_acct_conf))
	  return this->cb_acct_conf(this, NULL, &pack_req, cbp);

      if ((pack_req.code == RADIUS_CODE_ACCESS_REQUEST) &&
	  (this->cb_auth_conf))
	return this->cb_auth_conf(this, NULL, &pack_req, cbp);
    }    
  }
  
  if (this->debug) {
    printf("radius_timeout\n");
    if (this->first > 0) {
      printf("first %d, timeout %8d %8d\n", this->first, 
	     (int) this->queue[this->first].timeout.tv_sec, 
	     (int) this->queue[this->first].timeout.tv_usec); 
    }
    radius_printqueue(this);
  }

  return 0;
}



/* 
 * radius_addattr()
 * Add an attribute to a packet. The packet length is modified 
 * accordingly.
 * If data==NULL and dlen!=0 insert null attribute.
 */
int 
radius_addattr(struct radius_t *this, struct radius_packet_t *pack, 
	       uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
	       uint32_t value, uint8_t *data, uint16_t dlen) {
  struct radius_attr_t *a;
  uint16_t length = ntohs(pack->length);
  uint8_t vlen;
  char passwd[RADIUS_PWSIZE];
  int pwlen;

  a = (struct radius_attr_t*) ((void*) pack + length);


  if (type == RADIUS_ATTR_USER_PASSWORD) {
    radius_pwencode(this, (uint8_t*) passwd, RADIUS_PWSIZE, &pwlen, 
		    data, dlen, pack->authenticator,
		    this->secret, this->secretlen);
    data = (uint8_t*) passwd;
    dlen = pwlen;
  }

  if (type != RADIUS_ATTR_VENDOR_SPECIFIC) {
    if (dlen) { /* If dlen != 0 it is a text/string attribute */
      vlen = dlen;
    }
    else {
      vlen = 4; /* address, integer or time */
    }
    
    if (vlen > RADIUS_ATTR_VLEN) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Data too long!");
      return -1;
    }

    if ((length+vlen+2) > RADIUS_PACKSIZE) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "No more space!");
      return -1;
    }

    length += vlen + 2;

    pack->length = htons(length);

    a->t = type;
    a->l = vlen+2;
    if (data)
      memcpy(&a->v, data, dlen);
    else if (dlen)
      memset(&a->v, 0, dlen);
    else
      a->v.i = htonl(value);
  }
  else { /* Vendor specific */
    if (dlen) { /* If dlen != 0 it is a text/string attribute */
      vlen = dlen;
    }
    else {
      vlen = 4; /* address, integer or time */
    }

    if (vlen > RADIUS_ATTR_VLEN) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Data too long!");
      return -1;
    }

    if ((length+vlen+2) > RADIUS_PACKSIZE) { 
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "No more space!");
      return -1;
    }

    length += vlen + 8;

    pack->length = htons(length);

    a->t = type;
    a->l = vlen+8;

    a->v.vv.i = htonl(vendor_id);
    a->v.vv.t = vendor_type;
    a->v.vv.l = vlen+2;

    if (data)
      memcpy(((void*) a)+8, data, dlen); /* TODO */
    else if (dlen)
      memset(((void*) a)+8, 0, dlen); /* TODO */
    else
      a->v.vv.i = htonl(value);
  }

  return 0;
}


/* 
 * radius_getattr()
 * Search for an attribute in a packet. Returns -1 if attribute is not found.
 * The first instance matching attributes will be skipped
 */
int
radius_getattr(struct radius_packet_t *pack, struct radius_attr_t **attr,
	       uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
	       int instance) {
  struct radius_attr_t *t;
/*  struct radius_attr_t *v;  TODO: Loop through vendor specific */
  int offset = 0;
  int count = 0;

  if (0) {
    printf("radius_getattr \n");
    printf("radius_getattr payload %.2x %.2x %.2x %.2x\n",
	   pack->payload[0], pack->payload[1], pack->payload[2], 
	   pack->payload[3]);
  }


  if (type == RADIUS_ATTR_VENDOR_SPECIFIC) {
    do {
      t = (struct radius_attr_t*) (((void*) &(pack->payload)) + offset);
      if (0) {
	printf("radius_getattr %d %d %d %.2x %.2x \n", t->t, t->l, 
	       ntohl(t->v.vv.i), (int) t->v.vv.t, (int) t->v.vv.l);
      }
      if ((t->t == type) && (ntohl(t->v.vv.i) == vendor_id) && 
	  (t->v.vv.t == vendor_type)) {
	if (count == instance) {
	  *attr = (struct radius_attr_t *) &t->v.vv.t;
	  if (0) printf("Found\n");
	  return 0;
	}
	else {
	  count++;
	}
      }
      offset +=  t->l;
    } while (offset < (ntohs(pack->length)-RADIUS_HDRSIZE)); /* TODO */
  }

  else {     /* Need to check pack -> length */
    do {
      t = (struct radius_attr_t*) (((void*) &(pack->payload)) + offset);
      if (t->t == type) {
	if (count == instance) {
	  *attr = t;
	  return 0;
	}
	else {
	  count++;
	}
      }
      offset +=  t->l;
    } while (offset < (ntohs(pack->length)-RADIUS_HDRSIZE)); /* TODO */
  }

  return -1; /* Not found */

}

/* 
 * radius_countattr()
 * Count the number of instances of an attribute in a packet.
 */
int 
radius_countattr(struct radius_packet_t *pack, uint8_t type) {
  struct radius_attr_t *t;
  int offset = 0;
  int count = 0;
  
  /* Need to check pack -> length */
  
  do {
    t = (struct radius_attr_t*) (((void*) &(pack->payload)) + offset);
    if (t->t == type) {
      count++;
    }
    offset +=  2 + t->l;
  } while (offset < ntohs(pack->length));
  
  if (0) printf("Count %d\n", count);
  return count;
}


/* 
 * radius_cmpattr()
 * Compare two attributes to see if they are the same.
 */
int 
radius_cmpattr(struct radius_attr_t *t1, struct radius_attr_t *t2) {
  if (t1->t != t2->t  ) return -1;
  if (t1->l != t2->l) return -1;
  if (memcmp(t1->v.t, t2->v.t, t1->l)) return -1; /* Also int/time/addr */
  return 0;
}


/*
 * radius_keydecode()
 * Decode an MPPE key using MD5.
 */
int radius_keydecode(struct radius_t *this, uint8_t *dst, int dstsize,
		     int *dstlen, uint8_t *src, int srclen,
		     uint8_t *authenticator, char *secret, int secretlen) {
  int i, n;
  MD5_CTX context;
  unsigned char b[RADIUS_MD5LEN];
  int blocks;

  blocks = (srclen - 2) / RADIUS_MD5LEN;

  if ((blocks * RADIUS_MD5LEN +2) != srclen) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_keydecode: srclen must be 2 plus n*16");
    return -1;
  }

  if (blocks < 1) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_keydecode srclen must be at least 18");
    return -1;
  }

  /* Get MD5 hash on secret + authenticator (First 16 octets) */
  MD5Init(&context);
  MD5Update(&context, (uint8_t*) secret, secretlen);
  MD5Update(&context, authenticator, RADIUS_AUTHLEN);
  MD5Update(&context, src, 2);
  MD5Final(b, &context);

  if ((src[2] ^ b[0]) > dstsize) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_keydecode dstsize too small");
    return -1; 
  }

  if ((src[2] ^ b[0]) > (srclen - 3)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_keydecode dstlen > srclen -3");
    return -1; 
  }

  *dstlen = src[2] ^ b[0];

  for (i = 1; i < RADIUS_MD5LEN; i++)
    if ((i-1) < *dstlen)
      dst[i-1] = src[i+2] ^ b[i];

  /* Next blocks of 16 octets */
  for (n=1; n<blocks; n++) {
    MD5Init(&context);
    MD5Update(&context, (uint8_t*) secret, secretlen);
    MD5Update(&context, src + 2 + ((n-1) * RADIUS_MD5LEN), RADIUS_MD5LEN);
    MD5Final(b, &context);
    for (i = 0; i < RADIUS_MD5LEN; i++)
      if ((i-1+n*RADIUS_MD5LEN) < *dstlen)
	dst[i-1+n*RADIUS_MD5LEN] = src[i+2+n*RADIUS_MD5LEN] ^ b[i];
  }

  return 0;

}

/* 
 * radius_keyencode()
 * Encode an MPPE key using MD5.
 */
int radius_keyencode(struct radius_t *this, uint8_t *dst, int dstsize,
		     int *dstlen, uint8_t *src, int srclen,
		     uint8_t *authenticator, char *secret, int secretlen) {
  int i, n;
  MD5_CTX context;
  unsigned char b[RADIUS_MD5LEN];
  int blocks;

  blocks = (srclen + 1) / RADIUS_MD5LEN;
  if ((blocks * RADIUS_MD5LEN) < (srclen +1)) blocks++;
  
  if (((blocks * RADIUS_MD5LEN) +2 ) > dstsize) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_keyencode dstsize too small");
    return -1;
  }

  *dstlen = (blocks * RADIUS_MD5LEN) + 2;

  /* Read two salt octets */
  if (fread(dst, 1, 2, this->urandom_fp) != 2) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "fread() failed");
    return -1;
  }

  /* Get MD5 hash on secret + authenticator (First 16 octets) */
  MD5Init(&context);
  MD5Update(&context, (uint8_t*) secret, secretlen);
  MD5Update(&context, authenticator, RADIUS_AUTHLEN);
  MD5Update(&context, dst, 2);
  MD5Final(b, &context);
  dst[2] = (uint8_t) srclen ^ b[0]; /* Length of key */
  for (i = 1; i < RADIUS_MD5LEN; i++)
    if ((i-1) < srclen)
      dst[i+2] = src[i-1] ^ b[i];
    else
      dst[i+2] = b[i];

  /* Get MD5 hash on secret + c(n-1) (Next j 16 octets) */
  for (n=1; n<blocks; n++) {
    MD5Init(&context);
    MD5Update(&context, (uint8_t*) secret, secretlen);
    MD5Update(&context, dst+2+((n-1) * RADIUS_MD5LEN), RADIUS_MD5LEN);
    MD5Final(b, &context);
    for (i = 0; i < RADIUS_MD5LEN; i++)
      if ((i-1) < srclen)
	dst[i+2+n*RADIUS_MD5LEN] = src[i-1+n*RADIUS_MD5LEN] ^ b[i];
      else
	dst[i+2+n*RADIUS_MD5LEN] = b[i];
  }

  return 0;
}


/* 
 * radius_pwdecode()
 * Decode a password using MD5. Also used for MSCHAPv1 MPPE keys.
 */
int radius_pwdecode(struct radius_t *this, uint8_t *dst, int dstsize,
		    int *dstlen, uint8_t *src, int srclen, 
		    uint8_t *authenticator, char *secret, int secretlen) {
  int i, n;
  MD5_CTX context;
  unsigned char output[RADIUS_MD5LEN];

  if (srclen > dstsize) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_pwdecode srclen larger than dstsize");
    return -1;
  }

  if (srclen % RADIUS_MD5LEN) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_pwdecode srclen is not multiple of 16 octets");
    return -1;
  }


  *dstlen = srclen;

  if (this->debug) {
    printf("pwdecode srclen %d\n", srclen);
    for (n=0; n< srclen; n++) {
      printf("%.2x ", src[n]);
      if ((n % 16) == 15)
	printf("\n");
    }
    printf("\n");

    printf("pwdecode authenticator \n");
    for (n=0; n< RADIUS_AUTHLEN; n++) {
      printf("%.2x ", authenticator[n]);
      if ((n % 16) == 15)
	printf("\n");
    }
    printf("\n");

    printf("pwdecode secret \n");
    for (n=0; n< secretlen; n++) {
      printf("%.2x ", secret[n]);
      if ((n % 16) == 15)
	printf("\n");
    }
    printf("\n");
  }

  /* Get MD5 hash on secret + authenticator */
  MD5Init(&context);
  MD5Update(&context, (uint8_t*) secret, secretlen);
  MD5Update(&context, authenticator, RADIUS_AUTHLEN);
  MD5Final(output, &context);

  /* XOR first 16 octets of passwd with MD5 hash */
  for (i = 0; i < RADIUS_MD5LEN; i++)
    dst[i] = src[i] ^ output[i];

  /* Continue with the remaining octets of passwd if any */
  for (n = 0; n < 128 && n < (*dstlen - RADIUS_AUTHLEN); n += RADIUS_AUTHLEN) {
    MD5Init(&context);
    MD5Update(&context, (uint8_t*) secret, secretlen);
    MD5Update(&context, src + n, RADIUS_AUTHLEN);
    MD5Final(output, &context);
    for (i = 0; i < RADIUS_AUTHLEN; i++)
      dst[i + n + RADIUS_AUTHLEN] = src[i + n + RADIUS_AUTHLEN] ^ output[i];
  }    

  if (this->debug) {
    printf("pwdecode dest \n");
    for (n=0; n< 32; n++) {
      printf("%.2x ", dst[n]);
      if ((n % 16) == 15)
	printf("\n");
    }
    printf("\n");
  }

  return 0;
}


/* 
 * radius_pwencode()
 * Encode a password using MD5.
 */
int radius_pwencode(struct radius_t *this, uint8_t *dst, int dstsize,
		    int *dstlen, uint8_t *src, int srclen, 
		    uint8_t *authenticator, char *secret, int secretlen) {
  int i, n;
  MD5_CTX context;
  unsigned char output[RADIUS_MD5LEN];

  memset(dst, 0, dstsize);

  /* Make dstlen multiple of 16 */
  if (srclen & 0x0f) 
    *dstlen = (srclen & 0xf0) + 0x10; /* Padding 1 to 15 zeros */
  else
    *dstlen = srclen;                 /* No padding */

  /* Is dstsize too small ? */
  if (dstsize <= *dstlen) {
    *dstlen = 0;
    return -1;
  }

  /* Copy first 128 octets of src into dst */
  if (srclen > 128) 
    memcpy(dst, src, 128);
  else
    memcpy(dst, src, srclen);

  /* Get MD5 hash on secret + authenticator */
  MD5Init(&context);
  MD5Update(&context, (uint8_t*) secret, secretlen);
  MD5Update(&context, authenticator, RADIUS_AUTHLEN);
  MD5Final(output, &context);

  /* XOR first 16 octets of dst with MD5 hash */
  for (i = 0; i < RADIUS_MD5LEN; i++)
    dst[i] ^= output[i];

  /* if (*dstlen <= RADIUS_MD5LEN) return 0;  Finished */

  /* Continue with the remaining octets of dst if any */
  for (n = 0; n < 128 && n < (*dstlen - RADIUS_AUTHLEN); n += RADIUS_AUTHLEN) {
    MD5Init(&context);
    MD5Update(&context, (uint8_t*) secret, secretlen);
    MD5Update(&context, dst + n, RADIUS_AUTHLEN);
    MD5Final(output, &context);
    for (i = 0; i < RADIUS_AUTHLEN; i++)
      dst[i + n + RADIUS_AUTHLEN] ^= output[i];
  }    
  return 0;
}


/* 
 * radius_new()
 * Allocate a new radius instance.
 */
int radius_new(struct radius_t **this,
	       struct in_addr *listen, uint16_t port, int coanocheck,
	       struct in_addr *proxylisten, uint16_t proxyport,
	       struct in_addr *proxyaddr, struct in_addr *proxymask,
	       char* proxysecret)
{
  struct sockaddr_in addr;

  /* sys_err(LOG_INFO, __FILE__, __LINE__, 0,
     "Radius client started"); */

  /* Allocate storage for instance */
  if (!(*this = calloc(sizeof(struct radius_t), 1))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "calloc() failed");
    return -1;
  }

  (*this)->coanocheck = coanocheck;

  /* Radius parameters */
  (*this)->ouraddr.s_addr = listen->s_addr;
  (*this)->ourport = port;


  /* Proxy parameters */
  if (proxysecret) {
    (*this)->proxylisten.s_addr = proxylisten->s_addr;
    (*this)->proxyport = proxyport;
    
    if (proxyaddr)
      (*this)->proxyaddr.s_addr = proxyaddr->s_addr;
    else
      (*this)->proxyaddr.s_addr = ~0;
    
    if (proxymask)
      (*this)->proxymask.s_addr = proxymask->s_addr;
    else
      (*this)->proxymask.s_addr = 0;
    
    if (((*this)->proxysecretlen = strlen(proxysecret)) > RADIUS_SECRETSIZE) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "proxysecret longer than %d", RADIUS_SECRETSIZE);
      return -1;
    }
    
    if (((*this)->proxysecretlen = strlen(proxysecret)) > RADIUS_SECRETSIZE) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "proxy secret longer than %d", RADIUS_SECRETSIZE);
      return -1;
    }
    memcpy((*this)->proxysecret, proxysecret, (*this)->proxysecretlen);
  }

  /* Initialise queue */
  (*this)->next = 0;
  (*this)->first = -1;
  (*this)->last = -1;
  
  if (((*this)->urandom_fp = fopen("/dev/urandom", "r")) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "fopen(/dev/urandom, r) failed");
  }
  
  /* Initialise radius socket */
  if (((*this)->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "socket() failed!");
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr = (*this)->ouraddr;
  addr.sin_port = htons((*this)->ourport);
  
  if (bind((*this)->fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "bind() failed!");
    return -1;
  }

  /* Initialise proxy socket */
  if (proxysecret) {
    if (((*this)->proxyfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "socket() failed!");
      return -1;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = (*this)->proxylisten;
    addr.sin_port = htons((*this)->proxyport);
    
    if (bind((*this)->proxyfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "bind() failed!");
      return -1;
    }
  }
  else {
    (*this)->proxyfd = -1; /* Indicate that proxy is not used */
  }

  return 0;
}


/* 
 * radius_free()
 * Free a radius instance. (Undo radius_new() 
 */
int 
radius_free(struct radius_t *this)
{
  if (fclose(this->urandom_fp)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "fclose() failed!");
  }
  if (close(this->fd)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "close() failed!");
  }
  free(this);
  return 0;
}


void radius_set(struct radius_t *this, int debug,
		struct in_addr *server0, struct in_addr *server1,
		uint16_t authport, uint16_t acctport, char* secret) {
  
  this->debug = debug;

  /* Remote radius server parameters */
  this->hisaddr0.s_addr = server0->s_addr;
  this->hisaddr1.s_addr = server1->s_addr;

  if (authport) {
    this->authport = authport;
  }
  else {
    this->authport = RADIUS_AUTHPORT;
  }
  
  if (acctport) {
    this->acctport = acctport;
  }
  else {
    this->acctport = RADIUS_ACCTPORT;
  }

  if ((this->secretlen = strlen(secret)) > RADIUS_SECRETSIZE) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Radius secret too long. Truncating to %d characters", 
	    RADIUS_SECRETSIZE);
    this->secretlen = RADIUS_SECRETSIZE;
  }
  memcpy(this->secret, secret, this->secretlen);

  this->lastreply = 0; /* Start out using server 0 */  
  return;
}

/* 
 * radius_set_cb_ind()
 * Set callback function received requests
 */
int radius_set_cb_ind(struct radius_t *this,
  int (*cb_ind) (struct radius_t *radius, struct radius_packet_t *pack,
		 struct sockaddr_in *peer)) {

  this->cb_ind = cb_ind;
  return 0;
}


/* 
 * radius_set_cb_auth_conf()
 * Set callback function for responses to access request
 */
int
radius_set_cb_auth_conf(struct radius_t *this,
int (*cb_auth_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		       struct radius_packet_t *pack_req, void *cbp)) {

  this->cb_auth_conf = cb_auth_conf;
  return 0;
}

/* 
 * radius_set_cb_acct_conf()
 * Set callback function for responses to accounting request
 */
int
radius_set_cb_acct_conf(struct radius_t *this,
int (*cb_acct_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		     struct radius_packet_t *pack_req, void *cbp)) {

  this->cb_acct_conf = cb_acct_conf;
  return 0;
}

/* 
 * radius_set_cb_coa_ind()
 * Set callback function for coa and disconnect request
 */
int
radius_set_cb_coa_ind(struct radius_t *this,
int (*cb_coa_ind) (struct radius_t *radius, struct radius_packet_t *pack,
		   struct sockaddr_in *peer)) {

  this->cb_coa_ind = cb_coa_ind;
  return 0;
}


/* 
 * radius_req()
 * Send of a packet and place it in the retransmit queue
 */
int radius_req(struct radius_t *this,
	       struct radius_packet_t *pack,
	       void *cbp)
{
  struct sockaddr_in addr;
  int len = ntohs(pack->length);

  /* Place packet in queue */
  if (radius_queue_in(this, pack, cbp)) {
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;

  if (this->debug) printf("Lastreply: %d\n", this->lastreply);

  if (!this->lastreply) {
    addr.sin_addr = this->hisaddr0;
  }
  else {
    addr.sin_addr = this->hisaddr1;
  }

  if (pack->code == RADIUS_CODE_ACCOUNTING_REQUEST)
    addr.sin_port = htons(this->acctport);
  else
    addr.sin_port = htons(this->authport);
      
  
  if (sendto(this->fd, pack, len, 0,
	     (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "sendto() failed!");
    return -1;
  } 
    
  return 0;
}


/* 
 * radius_resp()
 * Send of a packet (no retransmit queue)
 */
int radius_resp(struct radius_t *this,
		struct radius_packet_t *pack,
		struct sockaddr_in *peer, uint8_t *req_auth) {

  int len = ntohs(pack->length);
  struct radius_attr_t *ma = NULL; /* Message authenticator */

  /* Prepare for message authenticator TODO */
  memset(pack->authenticator, 0, RADIUS_AUTHLEN);
  memcpy(pack->authenticator, req_auth, RADIUS_AUTHLEN);

  /* If packet contains message authenticator: Calculate it! */
  if (!radius_getattr(pack, &ma, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 0,0,0)) {
    radius_hmac_md5(this, pack, ma->v.t);
  }

  radius_authresp_authenticator(this, pack, req_auth, this->proxysecret,
			       this->proxysecretlen);
  
  if (sendto(this->proxyfd, pack, len, 0,
	     (struct sockaddr *) peer, sizeof(struct sockaddr_in)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "sendto() failed!");
    return -1;
  } 
  
  return 0;
}

/* 
 * radius_coaresp()
 * Send of a packet (no retransmit queue)
 */
int radius_coaresp(struct radius_t *this,
		   struct radius_packet_t *pack,
		   struct sockaddr_in *peer, uint8_t *req_auth) {

  int len = ntohs(pack->length);
  struct radius_attr_t *ma = NULL; /* Message authenticator */

  /* Prepare for message authenticator TODO */
  memset(pack->authenticator, 0, RADIUS_AUTHLEN);
  memcpy(pack->authenticator, req_auth, RADIUS_AUTHLEN);

  /* If packet contains message authenticator: Calculate it! */
  if (!radius_getattr(pack, &ma, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 0,0,0)) {
    radius_hmac_md5(this, pack, ma->v.t);
  }

  radius_authresp_authenticator(this, pack, req_auth, this->secret,
				this->secretlen);
  
  if (sendto(this->fd, pack, len, 0,
	     (struct sockaddr *) peer, sizeof(struct sockaddr_in)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "sendto() failed!");
    return -1;
  } 
  
  return 0;
}




/* 
 * radius_default_pack()
 * Return an empty packet which can be used in subsequent to 
 * radius_addattr()
 */
int
radius_default_pack(struct radius_t *this,
		    struct radius_packet_t *pack, 
		    int code)
{
  memset(pack, 0, RADIUS_PACKSIZE);
  pack->code = code;
  pack->id = 0; /* Let the send procedure queue the packet and assign id */
  pack->length = htons(RADIUS_HDRSIZE);
  
  if (fread(pack->authenticator, 1, RADIUS_AUTHLEN, this->urandom_fp)
      != RADIUS_AUTHLEN) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "fread() failed");
    return -1;
  }
  return 0;
}


/* 
 * radius_authcheck()
 * Check that the authenticator on a reply is correct.
 */
int radius_authcheck(struct radius_t *this, struct radius_packet_t *pack, 
		     struct radius_packet_t *pack_req)
{
  uint8_t auth[RADIUS_AUTHLEN];
  MD5_CTX context;

  MD5Init(&context);
  MD5Update(&context, (void*) pack, RADIUS_HDRSIZE-RADIUS_AUTHLEN);
  MD5Update(&context, pack_req->authenticator, RADIUS_AUTHLEN);
  MD5Update(&context, ((void*) pack) + RADIUS_HDRSIZE, 
	    ntohs(pack->length) - RADIUS_HDRSIZE);
  MD5Update(&context, (uint8_t*) this->secret, this->secretlen);
  MD5Final(auth, &context);
  
  return memcmp(pack->authenticator, auth, RADIUS_AUTHLEN);
}

/* 
 * radius_acctcheck()
 * Check that the authenticator on an accounting request is correct.
 */
int radius_acctcheck(struct radius_t *this, struct radius_packet_t *pack)
{
  uint8_t auth[RADIUS_AUTHLEN];
  uint8_t padd[RADIUS_AUTHLEN] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  MD5_CTX context;
  
  MD5Init(&context);
  MD5Update(&context, (void*) pack, RADIUS_HDRSIZE-RADIUS_AUTHLEN);
  MD5Update(&context, (uint8_t*) padd, RADIUS_AUTHLEN);
  MD5Update(&context, ((void*) pack) + RADIUS_HDRSIZE, 
	    ntohs(pack->length) - RADIUS_HDRSIZE);
  MD5Update(&context, (uint8_t*) this->secret, this->secretlen);
  MD5Final(auth, &context);
  
  return memcmp(pack->authenticator, auth, RADIUS_AUTHLEN);
}


/* 
 * radius_decaps()
 * Read and process a received radius packet.
 */
int radius_decaps(struct radius_t *this) {
  int status;
  struct radius_packet_t pack;
  struct radius_packet_t pack_req;
  void *cbp;
  struct sockaddr_in addr;
  socklen_t fromlen = sizeof(addr);
  int coarequest = 0;

  if (this->debug) printf("Received radius packet\n");
  
  if ((status = recvfrom(this->fd, &pack, sizeof(pack), 0, 
			 (struct sockaddr *) &addr, &fromlen)) <= 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "recvfrom() failed");
    return -1;
  }

  if (status < RADIUS_HDRSIZE) {
    sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	    "Received radius packet which is too short: %d < %d!",
	    status, RADIUS_HDRSIZE);
    return -1;
  }

  if (ntohs(pack.length) != status) {
    sys_err(LOG_WARNING, __FILE__, __LINE__, errno,
	    "Received radius packet with wrong length field %d != %d!",
	    ntohs(pack.length), status);
    return -1;
  }


  switch (pack.code) {
  case RADIUS_CODE_DISCONNECT_REQUEST:
  case RADIUS_CODE_COA_REQUEST:
    coarequest = 1;
    break;
  default:
    coarequest = 0;
  }

  if (!coarequest) {
    /* Check that reply is from correct address */
    if ((addr.sin_addr.s_addr != this->hisaddr0.s_addr) &&
	(addr.sin_addr.s_addr != this->hisaddr1.s_addr)) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Received radius reply from wrong address %.8x!",
	      addr.sin_addr.s_addr);
      return -1;
    }
    
    /* Check that UDP source port is correct */
    if ((addr.sin_port != htons(this->authport)) &&
	(addr.sin_port != htons(this->acctport))) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Received radius packet from wrong port %.4x!",
	      addr.sin_port);
      return -1;
    }
    
    if (radius_queue_out(this, &pack_req, pack.id, &cbp)) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Matching request was not found in queue: %d!", pack.id);
      return -1;
    }

    if (radius_authcheck(this, &pack, &pack_req)) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Authenticator does not match request!");
      return -1;
    }

    /* Set which radius server to use next */
    if (addr.sin_addr.s_addr == this->hisaddr0.s_addr)
      this->lastreply = 0;
    else
      this->lastreply = 1;
    
  }
  else {
    if (!this->coanocheck) {
      /* Check that request is from correct address */
      if ((addr.sin_addr.s_addr != this->hisaddr0.s_addr) &&
	  (addr.sin_addr.s_addr != this->hisaddr1.s_addr)) {
	sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
		"Received radius request from wrong address %.8x!",
		addr.sin_addr.s_addr);
	return -1;
      }
    }

    if (radius_acctcheck(this, &pack)) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Authenticator did not match MD5 of packet!");
      return -1;
    }
  }
    
  /* TODO: Check consistency of attributes vs packet length */
  
  switch (pack.code) {
  case RADIUS_CODE_ACCESS_ACCEPT:
  case RADIUS_CODE_ACCESS_REJECT:
  case RADIUS_CODE_ACCESS_CHALLENGE:
  case RADIUS_CODE_DISCONNECT_ACK:
  case RADIUS_CODE_DISCONNECT_NAK:
  case RADIUS_CODE_STATUS_ACCEPT:
  case RADIUS_CODE_STATUS_REJECT:
    if (this->cb_auth_conf)
      return this->cb_auth_conf(this, &pack, &pack_req, cbp);
    else
      return 0;
    break;
  case RADIUS_CODE_ACCOUNTING_RESPONSE:
    if (this->cb_acct_conf)
      return this->cb_acct_conf(this, &pack, &pack_req, cbp);
    else
      return 0;
    break;
  case RADIUS_CODE_DISCONNECT_REQUEST:
  case RADIUS_CODE_COA_REQUEST:
    if (this->cb_coa_ind)
      return this->cb_coa_ind(this, &pack, &addr);
    else
      return 0;
    break;
  default:
    sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	    "Received unknown radius packet %d!", pack.code);
    return -1;
  }
  
  sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	  "Received unknown radius packet %d!", pack.code);
  return -1;
}

/* 
 * radius_proxy_ind()
 * Read and process a received radius packet.
 */
int radius_proxy_ind(struct radius_t *this) {
  int status;
  struct radius_packet_t pack;
  struct sockaddr_in addr;
  socklen_t fromlen = sizeof(addr);

  if (this->debug) printf("Received radius packet\n");
  
  if ((status = recvfrom(this->proxyfd, &pack, sizeof(pack), 0, 
			 (struct sockaddr *) &addr, &fromlen)) <= 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "recvfrom() failed");
    return -1;
  }

  if (status < RADIUS_HDRSIZE) {
    sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	    "Received radius packet which is too short: %d < %d!",
	    status, RADIUS_HDRSIZE);
    return -1;
  }

  if (ntohs(pack.length) != status) {
    sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	    "Received radius packet with wrong length field %d != %d!",
	    ntohs(pack.length), status);
    return -1;
  }

  /* Is this a known request? */
  if ((this->cb_ind) &&
      ((pack.code == RADIUS_CODE_ACCESS_REQUEST) ||
       (pack.code == RADIUS_CODE_ACCOUNTING_REQUEST) ||
       (pack.code == RADIUS_CODE_DISCONNECT_REQUEST) ||
       (pack.code == RADIUS_CODE_STATUS_REQUEST))) {

    /* Check that request is from a known client */
    /* Any of the two servers or from one of the clients */
    if ((addr.sin_addr.s_addr & this->proxymask.s_addr)!= 
	this->proxyaddr.s_addr) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Received radius request from wrong address %.8x!",
	      addr.sin_addr.s_addr);
      return -1;
    }
    
    return this->cb_ind(this, &pack, &addr);
  }

  sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	  "Received unknown radius packet %d!", pack.code);
  return -1;
}
