# include <glib.h>
# include <sys/time.h>

typedef struct peer_st {
    char ip[16]; /* 111.222.333.444   */
    char hw[18]; /* 11:22:33:44:55:66 */
    char token[35];
    char *request;
    time_t connected;
    time_t expire;
    time_t idle_check;
    time_t auth_check;
    enum { PEER_DENY, PEER_ACCEPT } status;
    char *name;         // passive radius
    char *session_id;   // passive radius: accounting
    unsigned int acct_input_octets; // passive radius: accounting
    unsigned int acct_output_octets; // passive radius: accounting
    time_t acct_session_start; // passive radius: accounting
    unsigned int missing_count; // passive radius: for timeout
    char *password;  // passive radius: for reauthentication
} peer;

/*** Function prototypes start here ***/
peer *peer_new ( GHashTable *conf, const gchar *ip );
void peer_free ( peer *p );
int peer_permit ( GHashTable *conf, peer *p );
int peer_deny ( GHashTable *conf, peer *p );
gchar *get_peer_token ( peer *p );
void peer_set_name( peer *p, const char* name );
void peer_set_password( peer *p, const char* password );
void peer_set_auth_check( peer *p, const time_t auth_check );
void peer_set_session_id( peer *p, const char* session_id );

/*** Function prototypes start here ***/
int fw_perform( gchar *action, GHashTable *conf, peer *p );
int fw_perform_return( gchar *action, GHashTable *conf, peer *p, gchar *result, int size);
int fw_init ( GHashTable *conf );
int fw_re_init ( GHashTable *conf );
gchar *find_peer_arp( gchar *pip );
