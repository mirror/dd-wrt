# include "http.h"
# include "conf.h"
# include "util.h"
# include "firewall.h"

gchar *target_redirect ( http_request *h );
gchar *local_host( http_request *h );
peer *find_peer ( const gchar *ip );
void accept_peer ( http_request *h );
void remove_peer ( peer *p );
gboolean check_peer_expire ( gchar *ip, peer *p, time_t *now );
void peer_file_sync( const char *leasefile );
void peer_file_reinit( const char *leasefile );
int peer_re_permit ( GHashTable *conf, peer *p );

void increment_total_connections ( void );
void status_page ( http_request *h );

/*** actually defined in either open.c or passive.c ***/
void initialize_driver( void );
void handle_request( http_request *h );

