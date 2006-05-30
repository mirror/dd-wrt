# include <glib.h>
# include <sys/time.h>

typedef struct peer_st {
    char ip[16]; /* 111.222.333.444   */
    char hw[18]; /* 11:22:33:44:55:66 */
    char token[35];
    char *request;
    time_t connected;
    time_t expire;
    enum { PEER_DENY, PEER_ACCEPT } status;
} peer;

/*** Function prototypes start here ***/
peer *peer_new ( GHashTable *conf, const gchar *ip );
void peer_free ( peer *p );
int peer_permit ( GHashTable *conf, peer *p );
int peer_deny ( GHashTable *conf, peer *p );
gchar *get_peer_token ( peer *p );

/*** Function prototypes start here ***/
int fw_perform( gchar *action, GHashTable *conf, peer *p );
int fw_init ( GHashTable *conf );
gchar *peer_arp( peer *p );
