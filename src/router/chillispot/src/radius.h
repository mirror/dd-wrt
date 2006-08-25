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

#ifndef _RADIUS_H
#define _RADIUS_H

#define RADIUS_NONETWORK   0x01
#define RADIUS_NOBROADCAST 0x02

#define RADIUS_SECRETSIZE 128    /* No secrets that long */
#define RADIUS_MD5LEN  16        /* Length of MD5 hash */
#define RADIUS_AUTHLEN 16        /* RFC 2865: Length of authenticator */
#define RADIUS_PWSIZE  128       /* RFC 2865: Max 128 octets in password */
#define RADIUS_QUEUESIZE 256     /* Same size as id address space */
#define RADIUS_TIMEOUT 2500000   /* Time between requests in micro seconds */
#define RADIUS_RETRY1 10          /* Number of times to retry primary */
#define RADIUS_RETRY2 20          /* Total number of retries */

#define RADIUS_ATTR_VLEN 253
#define RADIUS_AUTHPORT 1812
#define RADIUS_ACCTPORT 1813
#define RADIUS_PACKSIZE 4096
#define RADIUS_HDRSIZE 20
#define RADIUS_PASSWORD_LEN 16
#define RADIUS_MPPEKEYSSIZE 32   /* Length of MS_CHAP_MPPE_KEYS attribute */ 

/* Radius packet types */
#define RADIUS_CODE_ACCESS_REQUEST            1
#define RADIUS_CODE_ACCESS_ACCEPT             2
#define RADIUS_CODE_ACCESS_REJECT             3
#define RADIUS_CODE_ACCOUNTING_REQUEST        4
#define RADIUS_CODE_ACCOUNTING_RESPONSE       5
#define RADIUS_CODE_ACCESS_CHALLENGE         11
#define RADIUS_CODE_STATUS_SERVER            12
#define RADIUS_CODE_STATUS_CLIENT            13
#define RADIUS_CODE_DISCONNECT_REQUEST       40
#define RADIUS_CODE_DISCONNECT_ACK           41
#define RADIUS_CODE_DISCONNECT_NAK           42
#define RADIUS_CODE_COA_REQUEST              43
#define RADIUS_CODE_COA_ACK                  44
#define RADIUS_CODE_COA_NAK                  45
#define RADIUS_CODE_STATUS_REQUEST           46
#define RADIUS_CODE_STATUS_ACCEPT            47
#define RADIUS_CODE_STATUS_REJECT            48

/* Radius attributes */
#define RADIUS_ATTR_USER_NAME                 1     /* string */
#define RADIUS_ATTR_USER_PASSWORD             2     /* string (encrypt) */
#define RADIUS_ATTR_CHAP_PASSWORD             3     /* octets */
#define RADIUS_ATTR_NAS_IP_ADDRESS            4     /* ipaddr */
#define RADIUS_ATTR_NAS_PORT                  5     /* integer */
#define RADIUS_ATTR_SERVICE_TYPE              6     /* integer */
#define RADIUS_ATTR_FRAMED_PROTOCOL           7     /* integer */
#define RADIUS_ATTR_FRAMED_IP_ADDRESS         8     /* ipaddr */
#define RADIUS_ATTR_FRAMED_IP_NETMASK         9     /* ipaddr */
#define RADIUS_ATTR_FRAMED_ROUTING           10     /* integer */
#define RADIUS_ATTR_FILTER_ID                11     /* string */
#define RADIUS_ATTR_FRAMED_MTU               12     /* integer */
#define RADIUS_ATTR_FRAMED_COMPRESSION       13     /* integer */
#define RADIUS_ATTR_LOGIN_IP_HOST            14     /* ipaddr */
#define RADIUS_ATTR_LOGIN_SERVICE            15     /* integer */
#define RADIUS_ATTR_LOGIN_TCP_PORT           16     /* integer */
#define RADIUS_ATTR_REPLY_MESSAGE            18     /* string */
#define RADIUS_ATTR_CALLBACK_NUMBER          19     /* string */
#define RADIUS_ATTR_CALLBACK_ID              20     /* string */
#define RADIUS_ATTR_FRAMED_ROUTE             22     /* string */
#define RADIUS_ATTR_FRAMED_IPX_NETWORK       23     /* ipaddr */
#define RADIUS_ATTR_STATE                    24     /* octets */
#define RADIUS_ATTR_CLASS                    25     /* octets */
#define RADIUS_ATTR_VENDOR_SPECIFIC          26     /* octets */
#define RADIUS_ATTR_SESSION_TIMEOUT          27     /* integer */
#define RADIUS_ATTR_IDLE_TIMEOUT             28     /* integer */
#define RADIUS_ATTR_TERMINATION_ACTION       29     /* integer */
#define RADIUS_ATTR_CALLED_STATION_ID        30     /* string */
#define RADIUS_ATTR_CALLING_STATION_ID       31     /* string */
#define RADIUS_ATTR_NAS_IDENTIFIER           32     /* string */
#define RADIUS_ATTR_PROXY_STATE              33     /* octets */
#define RADIUS_ATTR_LOGIN_LAT_SERVICE        34     /* string */
#define RADIUS_ATTR_LOGIN_LAT_NODE           35     /* string */
#define RADIUS_ATTR_LOGIN_LAT_GROUP          36     /* octets */
#define RADIUS_ATTR_FRAMED_APPLETALK_LINK    37     /* integer */
#define RADIUS_ATTR_FRAMED_APPLETALK_NETWORK 38     /* integer */
#define RADIUS_ATTR_FRAMED_APPLETALK_ZONE    39     /* string */
#define RADIUS_ATTR_ACCT_STATUS_TYPE         40     /* integer */
#define RADIUS_ATTR_ACCT_DELAY_TIME          41     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_OCTETS        42     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_OCTETS       43     /* integer */
#define RADIUS_ATTR_ACCT_SESSION_ID          44     /* string */
#define RADIUS_ATTR_ACCT_AUTHENTIC           45     /* integer */
#define RADIUS_ATTR_ACCT_SESSION_TIME        46     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_PACKETS       47     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_PACKETS      48     /* integer */
#define RADIUS_ATTR_ACCT_TERMINATE_CAUSE     49     /* integer */
#define RADIUS_ATTR_ACCT_MULTI_SESSION_ID    50     /* string */
#define RADIUS_ATTR_ACCT_LINK_COUNT          51     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_GIGAWORDS     52     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS    53     /* integer */
#define RADIUS_ATTR_EVENT_TIMESTAMP          55     /* date */
#define RADIUS_ATTR_CHAP_CHALLENGE           60     /* string */
#define RADIUS_ATTR_NAS_PORT_TYPE            61     /* integer */
#define RADIUS_ATTR_PORT_LIMIT               62     /* integer */
#define RADIUS_ATTR_LOGIN_LAT_PORT           63     /* integer */
#define RADIUS_ATTR_ACCT_TUNNEL_CONNECTION   68     /* string */
#define RADIUS_ATTR_ARAP_PASSWORD            70     /* string */
#define RADIUS_ATTR_ARAP_FEATURES            71     /* string */
#define RADIUS_ATTR_ARAP_ZONE_ACCESS         72     /* integer */
#define RADIUS_ATTR_ARAP_SECURITY            73     /* integer */
#define RADIUS_ATTR_ARAP_SECURITY_DATA       74     /* string */
#define RADIUS_ATTR_PASSWORD_RETRY           75     /* integer */
#define RADIUS_ATTR_PROMPT                   76     /* integer */
#define RADIUS_ATTR_CONNECT_INFO             77     /* string */
#define RADIUS_ATTR_CONFIGURATION_TOKEN      78     /* string */
#define RADIUS_ATTR_EAP_MESSAGE              79     /* string */
#define RADIUS_ATTR_MESSAGE_AUTHENTICATOR    80     /* octets */
#define RADIUS_ATTR_ARAP_CHALLENGE_RESPONSE  84     /* string # 10 octets */
#define RADIUS_ATTR_ACCT_INTERIM_INTERVAL    85     /* integer */
#define RADIUS_ATTR_NAS_PORT_ID              87     /* string */
#define RADIUS_ATTR_FRAMED_POOL              88     /* string */
#define RADIUS_ATTR_NAS_IPV6_ADDRESS         95     /* octets (IPv6) */
#define RADIUS_ATTR_FRAMED_INTERFACE_ID      96     /* octets # 8 octets */
#define RADIUS_ATTR_FRAMED_IPV6_PREFIX       97     /* octets ??? */
#define RADIUS_ATTR_LOGIN_IPV6_HOST          98     /* octets (IPv6) */
#define RADIUS_ATTR_FRAMED_IPV6_ROUTE        99     /* string */
#define RADIUS_ATTR_FRAMED_IPV6_POOL        100     /* string */
#define RADIUS_ATTR_DIGEST_RESPONSE         206     /* string */
#define RADIUS_ATTR_DIGEST_ATTRIBUTES       207     /* octets  ??? */


#define RADIUS_VENDOR_MS                    311
#define RADIUS_ATTR_MS_CHAP_RESPONSE          1
#define RADIUS_ATTR_MS_MPPE_ENCRYPTION_POLICY 7
#define RADIUS_ATTR_MS_MPPE_ENCRYPTION_TYPES  8
#define RADIUS_ATTR_MS_CHAP_CHALLENGE        11
#define RADIUS_ATTR_MS_CHAP_MPPE_KEYS        12
#define RADIUS_ATTR_MS_MPPE_SEND_KEY         16
#define RADIUS_ATTR_MS_MPPE_RECV_KEY         17
#define RADIUS_ATTR_MS_CHAP2_RESPONSE        25
#define RADIUS_ATTR_MS_CHAP2_SUCCESS         26


#define RADIUS_SERVICE_TYPE_LOGIN             1

#define RADIUS_STATUS_TYPE_START              1
#define RADIUS_STATUS_TYPE_STOP               2
#define RADIUS_STATUS_TYPE_INTERIM_UPDATE     3

#define RADIUS_NAS_PORT_TYPE_VIRTUAL          5
#define RADIUS_NAS_PORT_TYPE_WIRELESS_802_11 19
#define RADIUS_NAS_PORT_TYPE_WIRELESS_UMTS   23

#define RADIUS_TERMINATE_CAUSE_USER_REQUEST          1
#define RADIUS_TERMINATE_CAUSE_LOST_CARRIER          2
#define RADIUS_TERMINATE_CAUSE_LOST_SERVICE          3
#define RADIUS_TERMINATE_CAUSE_IDLE_TIMEOUT          4
#define RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT       5
#define RADIUS_TERMINATE_CAUSE_ADMIN_RESET           6
#define RADIUS_TERMINATE_CAUSE_ADMIN_REBOOT          7
#define RADIUS_TERMINATE_CAUSE_PORT_ERROR            8
#define RADIUS_TERMINATE_CAUSE_NAS_ERROR             9
#define RADIUS_TERMINATE_CAUSE_NAS_REQUEST          10
#define RADIUS_TERMINATE_CAUSE_NAS_REBOOT           11
#define RADIUS_TERMINATE_CAUSE_PORT_UNNEEDED        12
#define RADIUS_TERMINATE_CAUSE_PORT_PREEMPTED       13
#define RADIUS_TERMINATE_CAUSE_PORT_SUSPEND         14
#define RADIUS_TERMINATE_CAUSE_SERVICE_UNAVAILABLE  15
#define RADIUS_TERMINATE_CAUSE_CALLBACK             16
#define RADIUS_TERMINATE_CAUSE_USER_ERROR           17
#define RADIUS_TERMINATE_CAUSE_HOST_REQUEST         18



struct radius_packet_t {
  uint8_t code;
  uint8_t id;
  uint16_t length;
  uint8_t authenticator[RADIUS_AUTHLEN];
  uint8_t payload[RADIUS_PACKSIZE-RADIUS_HDRSIZE];
} __attribute__((packed));


struct radius_queue_t {      /* Holder for queued packets */
  int state;                 /* 0=empty, 1=full */
  void *cbp;                 /* Pointer used for callbacks */
  struct timeval timeout;    /* When do we retransmit this packet? */
  int retrans;               /* How many times did we retransmit this? */
  int lastsent;              /* 0 or 1 indicates last server used */
  struct sockaddr_in peer;   /* Address packet was sent to / received from */
  struct radius_packet_t p;  /* The packet stored */
  uint16_t seq;              /* The sequence number */
  uint8_t type;             /* The type of packet */
  int l;                     /* Length of the packet */
  struct qmsg_t *seqnext;    /* Pointer to next in sequence hash list */
  int next;                  /* Pointer to the next in queue. -1: Last */
  int prev;                  /* Pointer to the previous in queue. -1: First */
  int this;                  /* Pointer to myself */
};


struct radius_t {
  int fd;                        /* Socket file descriptor */
  FILE *urandom_fp;              /* /dev/urandom FILE pointer */
  struct in_addr ouraddr;        /* Address to listen to */
  uint16_t ourport;              /* Port to listen to */
  int coanocheck;                /* Accept coa from all IP addresses */
  int lastreply;                 /* 0 or 1 indicates last server reply */
  uint16_t authport;             /* His port for authentication */
  uint16_t acctport;             /* His port for accounting */
  struct in_addr hisaddr0;       /* Server address */
  struct in_addr hisaddr1;       /* Server address */
  char secret[RADIUS_SECRETSIZE];/* Shared secret */
  int secretlen;                 /* Length of sharet secret */
  int proxyfd;                   /* Proxy socket file descriptor */
  struct in_addr proxylisten;    /* Proxy address to listen to */
  uint16_t proxyport;            /* Proxy port to listen to */
  struct in_addr proxyaddr;      /* Proxy client address */
  struct in_addr proxymask;      /* Proxy client mask */
  char proxysecret[RADIUS_SECRETSIZE]; /* Proxy secret */
  int proxysecretlen;            /* Length of sharet secret */

  int debug;                     /* Print debug messages */
  struct radius_queue_t queue[RADIUS_QUEUESIZE]; /* Outstanding replies */
  uint8_t next;                  /* Next location in queue to use */
  int first;                     /* First packet in queue (oldest timeout) */
  int last;                      /* Last packet in queue (youngest timeout) */

  int listsize;                  /* Total number of addresses */
  int hashsize;                  /* Size of hash table */
  int hashlog;                   /* Log2 size of hash table */
  int hashmask;                  /* Bitmask for calculating hash */
  int (*cb_ind)  (struct radius_t *radius, struct radius_packet_t *pack,
		  struct sockaddr_in *peer);
  int (*cb_auth_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		       struct radius_packet_t *pack_req, void *cbp);
  int (*cb_acct_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		       struct radius_packet_t *pack_req, void *cbp);
  int (*cb_coa_ind)   (struct radius_t *radius, struct radius_packet_t *pack,
		       struct sockaddr_in *peer);
};

struct radiusm_t {
  struct in_addr addr;           /* IP address of this member */
  int inuse;                     /* 0=available; 1= inuse */
  struct RADIUSm_t *nexthash;    /* Linked list part of hash table */
  struct RADIUSm_t *prev, *next; /* Double linked list of available members */
  struct RADIUS_t *parent;       /* Pointer to parent */
  void *peer;                    /* Pointer to peer protocol handler */
};


struct radius_attr_t {
  uint8_t t;
  uint8_t l;
  union {
    uint32_t i;
    uint8_t  t[RADIUS_ATTR_VLEN];
    struct {
      uint32_t i;
      uint8_t t;
      uint8_t l;
      union {
	uint32_t i;
	uint8_t  t[RADIUS_ATTR_VLEN-4];
      } v;
    } vv;
  } v; 
} __attribute__((packed));


/* Create new radius instance */
extern int radius_new(struct radius_t **this, 
		      struct in_addr *listen, uint16_t port, int coanocheck,
		      struct in_addr *proxylisten, uint16_t proxyport,
		      struct in_addr *proxyaddr, struct in_addr *proxymask,
		      char* proxysecret);

/* Delete existing radius instance */
extern int radius_free(struct radius_t *this);

/* Set radius parameters which can later be changed */
extern void radius_set(struct radius_t *this, int debug,
		       struct in_addr *server0, struct in_addr *server1,
		       uint16_t authport, uint16_t acctport, char* secret);


/* Callback function for received request */
extern int radius_set_cb_ind(struct radius_t *this,
  int (*cb_ind) (struct radius_t *radius, struct radius_packet_t *pack,
		 struct sockaddr_in *peer));

/* Callback function for response to access request */
extern int radius_set_cb_auth_conf(struct radius_t *this,
int (*cb_auth_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		     struct radius_packet_t *pack_req, void *cbp));

/* Callback function for response to accounting request */
extern int radius_set_cb_acct_conf(struct radius_t *this,
int (*cb_acct_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		     struct radius_packet_t *pack_req, void *cbp));

/* Send of a request */
extern int radius_req(struct radius_t *this, 
		      struct radius_packet_t *pack,
		      void *cbp);

/* Send of a response */
extern int radius_resp(struct radius_t *this,
		       struct radius_packet_t *pack,
		       struct sockaddr_in *peer, uint8_t *req_auth);

/* Send of a coa response */
extern int radius_coaresp(struct radius_t *this,
			  struct radius_packet_t *pack,
			  struct sockaddr_in *peer, uint8_t *req_auth);

/* Process an incoming packet */
extern int radius_decaps(struct radius_t *this);

/* Process an incoming packet */
extern int radius_proxy_ind(struct radius_t *this);

/* Add an attribute to a packet */
extern int radius_addattr(struct radius_t *this, struct radius_packet_t *pack, 
	       uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
	       uint32_t value, uint8_t *data, uint16_t dlen);

/* Generate a packet for use with radius_addattr() */
extern int radius_default_pack(struct radius_t *this,
			       struct radius_packet_t *pack,
			       int code);

/* Extract an attribute from a packet */
extern int 
radius_getattr(struct radius_packet_t *pack, struct radius_attr_t **attr,
	       uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
	       int instance);


/* Encode a password */
extern int 
radius_pwencode(struct radius_t *this, uint8_t *dst, int dstsize,
		int *dstlen, uint8_t *src, int srclen, 
		uint8_t *authenticator, char *secret, int secretlen);
  

/* Decode a password (also used for MSCHAPv1 MPPE keys) */
extern int 
radius_pwdecode(struct radius_t *this, uint8_t *dst, int dstsize,
		    int *dstlen, uint8_t *src, int srclen, 
		uint8_t *authenticator, char *secret, int secretlen);


/* Decode MPPE key */
extern int 
radius_keydecode(struct radius_t *this, uint8_t *dst, int dstsize,
		 int *dstlen, uint8_t *src, int srclen, 
		 uint8_t *authenticator, char *secret, int secretlen);

/* Encode MPPE key */
extern int 
radius_keyencode(struct radius_t *this, uint8_t *dst, int dstsize,
		     int *dstlen, uint8_t *src, int srclen,
		 uint8_t *authenticator, char *secret, int secretlen);



/* Call this function to process packets needing retransmission */
extern int radius_timeout(struct radius_t *this);

/* Figure out when to call radius_calltimeout() */
extern int radius_timeleft(struct radius_t *this, struct timeval *timeout);

#endif	/* !_RADIUS_H */
