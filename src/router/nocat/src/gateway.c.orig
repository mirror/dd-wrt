# include <glib.h>
# include <string.h>
# include <unistd.h>
# include "gateway.h"

GHashTable *peer_tab; 

gchar *target_redirect ( http_request *h ) {
    gchar *orig, *host   = HEADER("Host"); 
    
    if ( host != NULL ) {
	orig = g_strdup_printf( "http://%s%s", HEADER("Host"), h->uri );
    } else {
	orig = CONF("HomePage");
    }

    return orig;
}


gchar *local_host( http_request *h ) {
    return g_strdup_printf( "%s:%s", h->sock_ip, CONF("GatewayPort") );
}

/************* Permit and deny peers *************/

peer *find_peer ( const char *ip ) {
    peer *p; 
    
    p = g_hash_table_lookup( peer_tab, ip );
    if (p == NULL) {
	p = peer_new( nocat_conf, ip );
	g_hash_table_insert( peer_tab, (gpointer) p->ip, p );
    }
    return p;
}

void accept_peer ( http_request *h ) {
    peer *p;
   
    p  = find_peer( h->peer_ip );
    g_message( "Accepting peer %s", p->ip );
    peer_permit( nocat_conf, p );
}

void remove_peer ( peer *p ) {
    g_message( "Removing peer %s", p->ip );
    peer_deny( nocat_conf, p );
}

gboolean check_peer_expire ( gchar *ip, peer *p, time_t *now ) {
    g_message( "Checking peer %s for expire: %ld sec. remain",
	ip, p->expire - *now );
    if (p->expire <= *now) {
	remove_peer( p );
	return TRUE;
    } else {
	return FALSE;
    }
} 


