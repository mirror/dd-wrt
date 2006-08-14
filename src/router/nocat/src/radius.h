
#define	RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST     1
#define	RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT     4
#define	RADIUS_ACCT_TERMINATE_CAUSE_SESSION_TIMEOUT  5
#define	RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_RESET      6
#define	RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_REBOOT     7
#define	RADIUS_ACCT_TERMINATE_CAUSE_NAS_REBOOT       11


/* per-server configuration structure */
typedef struct radius_server_config_struct {
  struct in_addr *radius_ip;	  /* server IP address */
  unsigned char *nas_id;	      /* nas id to be send*/
  unsigned char *secret;	      /* server shared secret */
  int secret_len;		          /* length of the secret (to save time later) */
  int timeout;			          /* cookie valid time */
  int wait;			              /* wait for RADIUS server responses */
  int retries;			          /*  number of retries on timeout */
  unsigned short port;		      /* RADIUS port number */
  unsigned long bind_address;	  /* bind socket to this local address */
  struct radius_server_config_struct *next; /* fallback server(s) */
} radius_server_config_rec;

/* per-server configuration create */
radius_server_config_rec* create_radius_server_config();


struct in_addr * get_ip_addr(const char *hostname);

int
radius_auth( radius_server_config_rec *scr, 
    const char *user, 
    const char *passwd_in,
    const char *called_station_id,
    const char *calling_station_id,
    char *message, 
    char *errstr, 
    unsigned int* session_timeout );

int
radius_acct_start( radius_server_config_rec *scr, 
    const char *user, 
    const char *acct_session_id,
    const char *called_station_id,
    const char *calling_station_id,
    char *errstr);
    
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
    char *errstr);


