# include <glib.h>
# include <string.h>
# include <unistd.h>
# include <time.h>
# include "gateway.h"

GHashTable *peer_tab; 
unsigned long int total_connections = 0;
time_t last_connection = 0;

void increment_total_connections( void ) {
    total_connections++;
    time(&last_connection);
}

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

peer *find_peer ( const char *ip ) {
    peer *p = NULL; 
    
    p = g_hash_table_lookup( peer_tab, ip );
    if ( NULL == p ) {
	if(CONFd("Verbosity") >= 6) g_message("Peer %s not found in table, adding...", ip);
        p = peer_new( nocat_conf, ip );
        if ( NULL != p ) {
            g_hash_table_insert( peer_tab, (gpointer) p->ip, p );
        }
	else g_warning("Error allocating new peer %s to table.", ip);
    }
    else if(CONFd("Verbosity") >= 6) g_message("Peer %s found.", ip);
    return p;
}

/****** status page! hooray!!  ********/

static gboolean 
	get_peer_status ( gpointer key, gpointer val, gpointer data ) {    

    peer *p	 = (peer *) val;
    GString *tab = (GString *) data;
    time_t now	 = time(NULL);
    gchar *mac, *macsearch;

    // removed because we allow a 0 in passive_radius.c
    //if (p->expire <= now)
	//return TRUE;

    mac	       = g_strdup( p->hw );
    macsearch  = g_strdup_printf( "%c%c%c%c%c%c",
	mac[0], mac[1], mac[3], mac[4], mac[6], mac[7] );

    mac[12] = mac[13] = mac[15] = mac[16] = 'X';

    g_string_sprintfa( tab, "<tr><td>%s</td>", p->ip );
    g_string_sprintfa( tab,     "<td>%s</td>", ctime(&(p->connected)) );
    g_string_sprintfa( tab,     "<td>%d</td>", 
				    (int) ((now - p->connected) / 60) );
    g_string_sprintfa( tab,     "<td><a href=\""
	"http://standards.ieee.org/cgi-bin/ouisearch?%s\">%s</a></td></tr>\n", 
	macsearch, mac );
    
    g_free(mac);
    g_free(macsearch);
    
    return TRUE;
}

void status_page ( http_request *h ) {
    GHashTable *status = g_hash_dup( nocat_conf );
    GString *peers;
    gchar *conn, *path, *file;
    time_t now;

    time(&now);
    g_hash_set( status, "LocalTime", ctime(&now) );

    conn = g_strdup_printf("%u", g_hash_table_size(peer_tab) );
    g_hash_set( status, "ConnectionCount", conn );
    g_free(conn);

    conn = g_strdup_printf("%lu", total_connections);
    g_hash_set( status, "TotalConnections", conn );
    g_free(conn);

    g_hash_set( status, "LastConnectionTime",
		last_connection ? ctime(&last_connection) : "Never" );

    /* still need UserTable */
    peers = g_string_sized_new(4096); 
	/* magic # to keep GLib from segfaulting :-/ */
    g_hash_table_foreach( peer_tab, (GHFunc) get_peer_status, peers );
    g_hash_set( status, "UserTable", peers->str );
    g_string_free(peers, TRUE);
    
    /* now parse the bloody template .... */
    path = http_fix_path( CONF("StatusForm"), CONF("DocumentRoot") );
    file = load_file( path );
    http_serve_template( h, file, status );

    g_free( path );
    g_free( file );
    g_hash_free( status );
}
