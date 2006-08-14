/* CVS $Id: radius.c,v 1.1.1.1 2006/03/28 01:13:43 tjaqua Exp $
   0.1 : Initial version based on mod_auth_radius code from the freeradius.org
*/
#include "radius.h"
#include <malloc.h>
#include "md5.h"
#include <netdb.h>
#include "includes.h"
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>

/*
  RFC 2138 says that this port number is wrong, but everyone's using it.
  Use " AddRadiusAuth server:port secret " to change the port manually.
  */
#define RADIUS_AUTH_UDP_PORT	     1812

#define RADIUS_PASSWORD_LEN	         16
#define RADIUS_RANDOM_VECTOR_LEN     16

/* Per-attribute structure */
typedef struct attribute_t {
  unsigned char attribute;
  unsigned char length;
  unsigned char data[1];
} attribute_t;

/* Packet header structure */
typedef struct radius_packet_t {
  unsigned char code;
  unsigned char id;
  unsigned short length;
  unsigned char vector[RADIUS_RANDOM_VECTOR_LEN];
  attribute_t first;
} radius_packet_t;

#define RADIUS_HEADER_LEN             20

/* RADIUS ID definitions. See RFC 2138 */
#define	RADIUS_ACCESS_REQUEST   1
#define	RADIUS_ACCESS_ACCEPT    2
#define	RADIUS_ACCESS_REJECT    3
#define RADIUS_ACCESS_CHALLENGE 11

/* RADIUS accounting ID definitions. See RFC 2866 */
#define RADIUS_ACCOUNTING_REQUEST  4
#define RADIUS_ACCOUNTING_RESPONSE 5

/* RADIUS attribute definitions. Also from RFC 2138 */
#define	RADIUS_USER_NAME	       1
#define	RADIUS_PASSWORD		       2
#define	RADIUS_NAS_IP_ADDRESS	   4
#define	RADIUS_NAS_PORT_ID	       5
#define RADIUS_SERVICE_TYPE        6
#define RADIUS_REPLY_MESSAGE       18
#define RADIUS_STATE		       24
#define RADIUS_SESSION_TIMEOUT     27
#define	RADIUS_CALLED_STATION_ID   30
#define	RADIUS_CALLING_STATION_ID  31
#define	RADIUS_NAS_IDENTIFIER	   32

/* RADIUS accounting attribute definitions. Also from RFC 2866 */
#define	RADIUS_ACCT_STATUS_TYPE	     40
#define	RADIUS_ACCT_STATUS_TYPE_START 1
#define	RADIUS_ACCT_STATUS_TYPE_STOP  2
#define	RADIUS_ACCT_INPUT_OCTETS	 42
#define	RADIUS_ACCT_OUTPUT_OCTETS	 43
#define	RADIUS_ACCT_SESSION_ID	     44
#define	RADIUS_ACCT_SESSION_TIME	 46
#define	RADIUS_ACCT_TERMINATE_CAUSE	 49

/* service types : authenticate only for now */
#define RADIUS_AUTHENTICATE_ONLY      8

/* How large the packets may be */
#define RADIUS_PACKET_RECV_SIZE       1024
#define RADIUS_PACKET_SEND_SIZE       1024

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

/* per-server configuration create */
radius_server_config_rec *create_radius_server_config()
{
   
  radius_server_config_rec *scr = (radius_server_config_rec *) g_new0(radius_server_config_rec,1);

  scr->radius_ip = NULL;	            /* no server yet */
  scr->port = RADIUS_AUTH_UDP_PORT;     /* set the default port */
  scr->nas_id = NULL;		            /* no nas id */
  scr->secret = NULL;		            /* no secret yet */
  scr->secret_len = 0;
  scr->wait = 50;		                /* wait 50 sec before giving up on the packet */
  scr->retries = 0;		                /* no additional retries */
  scr->timeout = 0;		                /* */
  scr->bind_address = INADDR_ANY;
  scr->next = NULL;

  return scr;
}

/* RADIUS utility functions */
struct in_addr *
get_ip_addr(const char *hostname)
{
  struct hostent *hp;

  if ((hp = gethostbyname(hostname)) != NULL) {
    struct in_addr *ipaddr = malloc(sizeof(struct in_addr));
    *ipaddr = *(struct in_addr *) hp->h_addr; /* make a local copy */
    return ipaddr;
  } else {
    return NULL;
  }
}

/* get a random vector */
void
get_random_vector(unsigned char vector[RADIUS_RANDOM_VECTOR_LEN])
{
  struct timeval tv;
  struct timezone tz;
  static unsigned int session = 1; /* make the random number harder to guess */
  MD5_CTX my_md5;
  
  /* Use the time of day with the best resolution the system can
     give us -- often close to microsecond accuracy. */
  gettimeofday(&tv,&tz);

  tv.tv_sec ^= getpid() * session++; /* add some secret information: session */

  /* Hash things to get some cryptographically strong pseudo-random numbers */
  MD5Init(&my_md5);
  MD5Update(&my_md5, (unsigned char *) &tv, sizeof(tv));
  MD5Update(&my_md5, (unsigned char *) &tz, sizeof(tz));
  MD5Final(vector, &my_md5);	      /* set the final vector */
}


unsigned char *
xor(unsigned char *p, unsigned char *q, int length)
{
  int i;
  unsigned char *response = p;
  
  for (i = 0; i < length; i++)
    *(p++) ^= *(q++);
  return response;
}

int
verify_packet( radius_server_config_rec *scr, radius_packet_t *packet, unsigned char vector[RADIUS_RANDOM_VECTOR_LEN])
{
  MD5_CTX my_md5;
  unsigned char	calculated[RADIUS_RANDOM_VECTOR_LEN];
  unsigned char	reply[RADIUS_RANDOM_VECTOR_LEN];
  
  /*
   * We could dispense with the memcpy, and do MD5's of the packet
   * + vector piece by piece.  This is easier understand, and probably faster.
   */
  memcpy(reply, packet->vector, RADIUS_RANDOM_VECTOR_LEN); /* save the reply */
  memcpy(packet->vector, vector, RADIUS_RANDOM_VECTOR_LEN); /* sent vector */
   
  /* MD5(packet header + vector + packet data + secret) */
  MD5Init(&my_md5);
  MD5Update(&my_md5, (unsigned char *) packet, ntohs(packet->length));
  MD5Update(&my_md5, scr->secret, scr->secret_len);
  MD5Final(calculated, &my_md5);      /* set the final vector */

  /* Did he use the same random vector + shared secret? */
  if(memcmp(calculated, reply, RADIUS_RANDOM_VECTOR_LEN) != 0) {
    return -1;
  }
  return 0;
}

void
add_attribute(radius_packet_t *packet, int type, const unsigned char *data, int length)
{
  attribute_t *p;

  p = (attribute_t *) ((unsigned char *)packet + packet->length);
  p->attribute = type;
  p->length = length + 2;		/* the total size of the attribute */
  packet->length += p->length;
  memcpy(p->data, data, length);
}

attribute_t *
find_attribute(radius_packet_t *packet, unsigned char type)
{
  attribute_t *attr = &packet->first;
  int len = ntohs(packet->length) - RADIUS_HEADER_LEN;

  while (attr->attribute != type) {
    if ((len -= attr->length) <= 0) {
      return NULL;		/* not found */
    }
    attr = (attribute_t *) ((char *) attr + attr->length);
  }
  return attr;
}


int send_and_receive_packet( radius_server_config_rec *  scr, int sockfd, radius_packet_t *packet, int retries, const char* user, char *recv_buffer) {
  int                   salen, total_length;
  fd_set                set;
  struct sockaddr_in*   sin;
  struct sockaddr       saremote;
  struct timeval        tv;
  int                   rcode = -1;
  
  total_length = packet->length;
  packet->length = htons(packet->length);
  
  sin = (struct sockaddr_in *) &saremote;
  memset ((char *) sin, '\0', sizeof(saremote));
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = scr->radius_ip->s_addr;
  sin->sin_port = htons(scr->port);
  
  g_debug("Sending packet on %s:%i", inet_ntoa(*scr->radius_ip), scr->port);
  
  while (retries >= 0) {
        if (sendto(sockfd, (char *) packet, total_length, 0,
               &saremote, sizeof(struct sockaddr_in)) < 0) {
          g_warning("Error sending RADIUS packet for user %s: %s", user, strerror(errno));
          return FALSE;
        }
        
      wait_again:
        /* ************************************************************ */
        /* Wait for the response, and verify it. */
        salen = sizeof (saremote);
        tv.tv_sec = scr->wait;	/* wait for the specified time */
        tv.tv_usec = 0;
        FD_ZERO(&set);		/* clear out the set */
        FD_SET(sockfd, &set);	/* wait only for the RADIUS UDP socket */
        
        rcode = select(sockfd + 1, &set, NULL, NULL, &tv);
        if ((rcode < 0) && (errno == EINTR)) {
          goto wait_again;		/* signal, ignore it */
        }
        
        if (rcode == 0) {		/* done the select, with no data ready */
          retries--;
        } else {
          break;			/* exit from the 'while retries' loop */
        }
  } /* loop over the retries */

  /*
   *  Error.  Die.
   */
  if (rcode < 0) {
    g_warning("Error waiting for RADIUS response: %s", strerror(errno));
    return FALSE;
  }
  
  /*
   *  Time out.
   */
  if (rcode == 0) {
    g_warning( "RADIUS server %s failed to respond within %d seconds after each of %d retries", inet_ntoa(*scr->radius_ip), scr->wait, scr->retries);
    return FALSE;
  }
  
  if ((total_length = recvfrom(sockfd, (char *) recv_buffer,
			       RADIUS_PACKET_RECV_SIZE,
			       0, &saremote, &salen)) < 0) {
    g_warning( "Error reading RADIUS packet: %s", strerror(errno));
    return FALSE;
  } else {
    packet = (radius_packet_t *) recv_buffer; /* we have a new packet */
    if ((ntohs(packet->length) > total_length) ||
	     (ntohs(packet->length) > RADIUS_PACKET_RECV_SIZE)) {
         g_warning("RADIUS packet corrupted");
         return FALSE;
    }
  }
  
  return TRUE;
}  

#define radcpy(STRING, ATTR) {memcpy(STRING, ATTR->data, ATTR->length - 2); \
                              (STRING)[ATTR->length - 2] = 0;}


/* helper to open a socket */
int
radius_open_socket( radius_server_config_rec * scr, const char* user, int* sockfd ) {
  struct sockaddr_in *sin;
  struct sockaddr salocal;
  unsigned short local_port;
  
  /* ************************************************************ */
  /* connect to a port */
  if ((*sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
    g_warning("error opening RADIUS socket for user %s: %s", user, strerror(errno));
    return FALSE;
  }
  
  sin = (struct sockaddr_in *) &salocal;
  memset((char *) sin, '\0', sizeof(salocal));
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = scr->bind_address;
  
  local_port = 1025;
  do {
    local_port++;
    sin->sin_port = htons((unsigned short) local_port);
  } while((bind(*sockfd, &salocal, sizeof(struct sockaddr_in)) < 0) &&
	  (local_port < 64000));
  if(local_port >= 64000) {
    close(*sockfd);
    g_warning("cannot bind to RADIUS socket for user %s", user);
    return FALSE;
  }
  return TRUE;
}


int
radius_build_and_send_authenticate(
            radius_server_config_rec *  scr, 
		    int                         sockfd, 
            int                         code, 
            char *                      recv_buffer,
		    const char *                user, 
            const char *                passwd_in, 
				const char * 				  called_station_id,
				const char *              calling_station_id,
		    unsigned char *             vector, 
            char *                      errstr)
{
  int retries = scr->retries;
  struct in_addr *ip_addr = NULL;
  
  unsigned char misc[RADIUS_RANDOM_VECTOR_LEN];
  int password_len, i;
  unsigned char password[128];
  MD5_CTX md5_secret, my_md5;
  uint32_t service;

  unsigned char send_buffer[RADIUS_PACKET_SEND_SIZE];
  radius_packet_t *packet = (radius_packet_t *) send_buffer;

  i = strlen(passwd_in);
  password_len = (i + 0x0f) & 0xfffffff0; /* round off to 16 */
  if (password_len == 0) {
    password_len = 16;		/* it's at least 15 bytes long */
  } else if (password_len > 128) { /* password too long, from RFC2138, p.22 */
    g_warning("password given by user %s is too long for RADIUS", user);
    return FALSE;
  }
  
  memset(password, 0, password_len);
  memcpy(password, passwd_in, i); /* don't use strcpy! */
  
  /* ************************************************************ */
  /* generate a random authentication vector */
  get_random_vector(vector);

  /* ************************************************************ */
  /* Fill in the packet header */
  memset(send_buffer, 0, sizeof(send_buffer));

  packet->code = code;
  packet->id = vector[0];	/* make a random request id */
  packet->length = RADIUS_HEADER_LEN;
  memcpy(packet->vector, vector, RADIUS_RANDOM_VECTOR_LEN);
  
  /* Fill in the user name attribute */
  add_attribute(packet, RADIUS_USER_NAME, user, strlen(user));
  
  /* ************************************************************ */
  /* encrypt the password */
  /* password : e[0] = p[0] ^ MD5(secret + vector) */
  MD5Init(&md5_secret);
  MD5Update(&md5_secret, scr->secret, scr->secret_len);
  my_md5 = md5_secret;		/* so we won't re-do the hash later */
  MD5Update(&my_md5, vector, RADIUS_RANDOM_VECTOR_LEN);
  MD5Final(misc, &my_md5);      /* set the final vector */
  xor(password, misc, RADIUS_PASSWORD_LEN);
  
  /* For each step through, e[i] = p[i] ^ MD5(secret + e[i-1]) */
  for (i = 1; i < (password_len >> 4); i++) {
    my_md5 = md5_secret;	/* grab old value of the hash */
    MD5Update(&my_md5, &password[(i-1) * RADIUS_PASSWORD_LEN], RADIUS_PASSWORD_LEN);
    MD5Final(misc, &my_md5);      /* set the final vector */
    xor(&password[i * RADIUS_PASSWORD_LEN], misc, RADIUS_PASSWORD_LEN);
  }
  add_attribute(packet, RADIUS_PASSWORD, password, password_len);
  
  /* ************************************************************ */
  /* Tell the RADIUS server that we only want to authenticate */
  service = htonl(RADIUS_AUTHENTICATE_ONLY);
  add_attribute(packet, RADIUS_SERVICE_TYPE, (unsigned char *) &service, sizeof(service));
  /* ************************************************************ */
  /* Tell the RADIUS server which virtual server we're coming from */
  if ( NULL != scr->nas_id  ) {
      add_attribute(packet, RADIUS_NAS_IDENTIFIER, scr->nas_id, strlen(scr->nas_id));
  }
  /* ************************************************************ */
  /* Tell the RADIUS server which IP address we're coming from */
  if (scr->radius_ip->s_addr == htonl(0x7f000001)) {
    ip_addr = scr->radius_ip; /* go to localhost through localhost */
  
    add_attribute(packet, RADIUS_NAS_IP_ADDRESS, (unsigned char *)&ip_addr->s_addr, sizeof(ip_addr->s_addr));
  } 
  /* ************************************************************ */
  /* Tell the RADIUS server MAC address of client and access point  */
  if ( NULL != called_station_id  )  add_attribute(packet, RADIUS_CALLED_STATION_ID, called_station_id, strlen(called_station_id));
  if ( NULL != calling_station_id  ) add_attribute(packet, RADIUS_CALLING_STATION_ID, calling_station_id, strlen(calling_station_id));

  if ( send_and_receive_packet( scr, sockfd, packet, retries, user, recv_buffer) ) {
      /* Check if we've got everything OK.  We should also check packet->id...*/
      packet = (radius_packet_t *) recv_buffer; 
      if (verify_packet(scr, packet, vector)) {
        g_warning("RADIUS packet fails verification");
        return FALSE;
      }
      return TRUE;
  } else {
      return FALSE;
  }
}


int
radius_auth( radius_server_config_rec *scr, const char *user, const char *passwd_in, const char* called_station_id, const char* calling_station_id, char *message, char *errstr, unsigned int* session_timeout ) {
    
    unsigned char vector[RADIUS_RANDOM_VECTOR_LEN];
    unsigned char recv_buffer[RADIUS_PACKET_RECV_SIZE];
    radius_packet_t *packet;
    int sockfd;
    int rcode;
    
    if ( !radius_open_socket(scr, user, &sockfd) ) return FALSE;
    
    rcode = radius_build_and_send_authenticate( scr, sockfd, RADIUS_ACCESS_REQUEST, recv_buffer, user, passwd_in, called_station_id, calling_station_id, vector, errstr);
    
    close(sockfd);
    
    if (!rcode) return FALSE;
    
    // check result
    packet = (radius_packet_t *) recv_buffer;
    
    switch (packet->code) {
        case RADIUS_ACCESS_ACCEPT: 
          {
                attribute_t *a_timeout;
                unsigned int i;
            
                a_timeout = find_attribute(packet, RADIUS_SESSION_TIMEOUT);
                if (a_timeout) {
                  memcpy(&i, a_timeout->data, 4);
                  i = ntohl(i);
		  if ( NULL != session_timeout ) *session_timeout = i;
                  g_debug("RADIUS returned timeout %u", i);
                }
          }
          *message = 0;		/* no message*/
          return TRUE;		/* he likes you! */
          break;
          
        case RADIUS_ACCESS_REJECT:
          g_warning("RADIUS authentication failed for user %s", user);
          break;
           
        default:			/* don't know what else to do */
          g_warning("RADIUS server returned unknown response %02x",packet->code);
          break;
    }
    return FALSE;			/* default to failing authentication */
}


int
radius_build_and_send_accounting(
    radius_server_config_rec *  scr, 
    int                         sockfd, 
    char *                      recv_buffer,
    int                         acct_status_type,
    const char *                user, 
    const char *                acct_session_id,
    const char *                called_station_id,
    const char *                calling_station_id,
    unsigned int                acct_input_octets,
    unsigned int                acct_output_octets,
    unsigned int                acct_session_time,
    unsigned int                acct_terminate_cause,
    unsigned char *             vector, 
    char *                      errstr)
{
  int retries = scr->retries;
  int converted;
  struct in_addr *ip_addr = NULL;
  MD5_CTX md5_secret;
  int packet_length;
  
  unsigned char send_buffer[RADIUS_PACKET_SEND_SIZE];
  radius_packet_t *packet = (radius_packet_t *) send_buffer;
  
  
  /* ************************************************************ */
  /* generate a random authentication vector */
  get_random_vector(vector);

  /* ************************************************************ */
  /* Fill in the packet header */
  memset(send_buffer, 0, sizeof(send_buffer));
  
  packet->code = RADIUS_ACCOUNTING_REQUEST;
  
  packet->id = vector[0];	/* make a random request id */
  packet->length = RADIUS_HEADER_LEN;
  memset(packet->vector, 0, RADIUS_RANDOM_VECTOR_LEN);
  // vector is calculated not like the authentification vector, see below
  
  /* ************************************************************ */
  converted = htonl(acct_status_type);
  add_attribute(packet, RADIUS_ACCT_STATUS_TYPE, (const unsigned char *)&converted, 4);
  add_attribute(packet, RADIUS_USER_NAME, user, strlen(user));
  if ( NULL != acct_session_id  )    add_attribute(packet, RADIUS_ACCT_SESSION_ID, acct_session_id, strlen(acct_session_id));
  if ( NULL != called_station_id  )  add_attribute(packet, RADIUS_CALLED_STATION_ID, called_station_id, strlen(called_station_id));
  if ( NULL != calling_station_id  ) add_attribute(packet, RADIUS_CALLING_STATION_ID, calling_station_id, strlen(calling_station_id));
  if ( NULL != scr->nas_id  )        add_attribute(packet, RADIUS_NAS_IDENTIFIER , scr->nas_id, strlen(scr->nas_id));
  if ( RADIUS_ACCT_STATUS_TYPE_STOP == acct_status_type ) {
    
    if ( 0 != acct_input_octets    ) { converted = htonl(acct_input_octets);    add_attribute(packet, RADIUS_ACCT_INPUT_OCTETS,    (const unsigned char *)&converted, 4); }
    if ( 0 != acct_output_octets   ) { converted = htonl(acct_output_octets);   add_attribute(packet, RADIUS_ACCT_OUTPUT_OCTETS,   (const unsigned char *)&converted, 4); }
    if ( 0 != acct_session_time    ) { converted = htonl(acct_session_time);    add_attribute(packet, RADIUS_ACCT_SESSION_TIME,    (const unsigned char *)&converted, 4); }
    if ( 0 != acct_terminate_cause ) { converted = htonl(acct_terminate_cause); add_attribute(packet, RADIUS_ACCT_TERMINATE_CAUSE, (const unsigned char *)&converted, 4); }
  }
  /* ************************************************************ */
  /* Tell the RADIUS server which IP address we're coming from */
  if (scr->radius_ip->s_addr == htonl(0x7f000001)) {
    ip_addr = scr->radius_ip; /* go to localhost through localhost */
    add_attribute(packet, RADIUS_NAS_IP_ADDRESS, (unsigned char *)&ip_addr->s_addr, sizeof(ip_addr->s_addr));
 } 
  // calculate the Request Authenticator
  /*  
      The NAS and RADIUS accounting server share a secret.  The Request
      Authenticator field in Accounting-Request packets contains a one-
      way MD5 hash calculated over a stream of octets consisting of the
      Code + Identifier + Length + 16 zero octets + request attributes +
      shared secret (where + indicates concatenation).  The 16 octet MD5
      hash value is stored in the Authenticator field of the
      Accounting-Request packet.

      Note that the Request Authenticator of an Accounting-Request can
      not be done the same way as the Request Authenticator of a RADIUS
      Access-Request, because there is no User-Password attribute in an
      Accounting-Request.
  */
  /* MD5(packet header + vector + packet data + secret) */
  packet_length = packet->length;
  packet->length = htons(packet->length); // convert as for send to do md5 
  MD5Init(&md5_secret);
  MD5Update(&md5_secret, (unsigned char *) packet, packet_length);
  MD5Update(&md5_secret, scr->secret, scr->secret_len);
  MD5Final(vector, &md5_secret);  /* set the final vector */
  memcpy(packet->vector, vector, RADIUS_RANDOM_VECTOR_LEN);
  packet->length = packet_length; // convert back
  
  if ( send_and_receive_packet( scr, sockfd, packet, retries, user, recv_buffer) ) {
      
      /* Check if we've got everything OK.  We should also check packet->id...*/
      packet = (radius_packet_t *) recv_buffer; 
      if (verify_packet(scr, packet, vector)) {
        g_warning("RADIUS packet fails verification");
        return FALSE;
      }
      return TRUE;
  } else {
      return FALSE;
  }
}

int
radius_acct( radius_server_config_rec *scr, 
    int acct_status_type,
    const char *user, 
    const char *acct_session_id,
    const char *called_station_id,
    const char *calling_station_id,
    unsigned int acct_input_octets,
    unsigned int acct_output_octets,
    unsigned int acct_session_time,
    unsigned int acct_terminate_cause,
    char *errstr) 
{
    unsigned char vector[RADIUS_RANDOM_VECTOR_LEN];
    unsigned char recv_buffer[RADIUS_PACKET_RECV_SIZE];
    radius_packet_t *packet;
    int sockfd;
    int rcode;
    
    if ( !radius_open_socket(scr, user, &sockfd) ) return FALSE;
    
    rcode = radius_build_and_send_accounting( scr, sockfd, recv_buffer, 
                acct_status_type, 
                user, 
                acct_session_id, 
                called_station_id, calling_station_id, 
                acct_input_octets, acct_output_octets,
                acct_session_time,
                acct_terminate_cause,
                vector, errstr);
    
    close(sockfd);
    
    if (!rcode) return FALSE;
    
    // check result
    packet = (radius_packet_t *) recv_buffer;
    
    switch (packet->code) {
        case RADIUS_ACCOUNTING_RESPONSE:
          return TRUE;
          break;
        
        default:
          g_warning("RADIUS server returned unknown response %02x",packet->code);
          break;
    }
    return FALSE;
}



int
radius_acct_start( radius_server_config_rec *scr, 
    const char *user, 
    const char *acct_session_id,
    const char *called_station_id,
    const char *calling_station_id,
    char *errstr) {
        return radius_acct( scr, RADIUS_ACCT_STATUS_TYPE_START,
                   user, acct_session_id, 
                   called_station_id, calling_station_id, 
                   0,0,0,0, 
                   errstr ); 
}
    
int
radius_acct_stop( radius_server_config_rec *scr, 
    const char *user, 
    const char *acct_session_id,
    const char *called_station_id,
    const char *calling_station_id,
    unsigned int acct_input_octets,
    unsigned int acct_output_octets,
    unsigned int acct_session_time,
    unsigned int acct_terminate_cause,
    char *errstr) {
        return radius_acct( scr, RADIUS_ACCT_STATUS_TYPE_STOP,
                            user, acct_session_id, 
                            called_station_id, calling_station_id, 
                            acct_input_octets,acct_output_octets,
                            acct_session_time,acct_terminate_cause, 
                            errstr ); 
}
