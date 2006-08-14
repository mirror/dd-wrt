# include <glib.h>
# include <string.h>
# include <unistd.h>
# include <stdio.h>
# include <time.h>
# include "gateway.h"

static gboolean peer_write_ea ( gchar *ip, peer *p, FILE *fp ); 
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
	orig = g_strdup(CONF("HomePage"));
    }

    return orig;
}


gchar *local_host( http_request *h ) {
    return g_strdup_printf( "%s:%s", h->sock_ip, CONF("GatewayPort") );
}

peer *find_peer ( const char *ip ) {
    peer *p = NULL; 
    gchar *hw = find_peer_arp(ip);
    
    p = g_hash_table_lookup( peer_tab, ip );
    if ( NULL == p ) {
        p = peer_new( nocat_conf, ip );
        if ( NULL != p ) {
            g_hash_table_insert( peer_tab, (gpointer) p->ip, p );
	    if(CONFd("Verbosity") >= 6) g_message("find_peer: %s not in table, adding...", ip);
        }
	else g_warning("Error allocating new peer %s to table.", ip);
    }
    else if( strncmp(hw,p->hw,sizeof(p->hw)+1) ) {
	if(CONFd("Verbosity") >= 6) g_message("find_peer: %s is in table with MAC not matching %s, replacing...", ip, hw);
	peer_deny( nocat_conf, p );
        g_strncpy( p->hw, hw, 18 );
	p->connected = time( NULL );
    }
    else if(CONFd("Verbosity") >= 6) g_message("find_peer: Peer %s found, with MAC: %s.", ip, p->hw);

    free(hw);
    return p;
}

/************* Write out a static peer leases-type file *********/
/* For static startup re-initialization! -TJ */
void peer_file_sync( const char *leasefile) {
    FILE *fp = fopen(leasefile, "w");

    if (!fp) {
        g_warning("Cannot open lease file %s for writing!", leasefile);
        return;
    }

    g_hash_table_foreach(peer_tab, (GHFunc)peer_write_ea, fp);

    fclose(fp);

    if (CONFd("Verbosity") >= 5) g_message("Synced peer leases to file: %s.", leasefile);
    return;
}

static gboolean peer_write_ea ( gchar *ip, peer *p, FILE *fp ) {
    fwrite(p,sizeof(peer),1,fp);
    return TRUE;
}

void peer_file_reinit(const char *leasefile) {
    FILE *fp = fopen(leasefile, "r");
    peer *p = g_new0(peer,1);
    time_t now = time(NULL);

    if (!fp) {
        g_warning("Cannot open lease file %s for re-initialization!", leasefile);
        return;
    }
    else if (CONFd("Verbosity") >= 5) g_message("Re-initializing leases from file: %s.", leasefile);

    while (fread(p,sizeof(peer),1,fp)==1) {
        if (p->expire >= now) {
            g_hash_table_insert( peer_tab, (gpointer) p->ip, p );
            peer_re_permit(nocat_conf, p);
            if(CONFd("Verbosity") >= 6) g_message("peer_file_reinit: %s added, expires at: %u", p->ip, p->expire);
            p = g_new0(peer,1);
        }
    }
    if (CONFd("Verbosity") >= 5) g_message("Finished re-initializing leases, re-syncing leasefile.");

    fclose(fp);
    peer_file_sync(leasefile);
}

// re-initialization of the firewall
// sIngle rules can be there or not, we first try to remove them and then
// add them again
int peer_re_permit ( GHashTable *conf, peer *p ) {
    g_assert( p != NULL );
    if (p->status != PEER_DENY) {
        if (fw_perform_exec( "DenyCmd", conf, p, TRUE ) == 0) {
            p->status = PEER_DENY;
        }
        if (p->status != PEER_ACCEPT) {
            if (fw_perform_exec( "PermitCmd", conf, p, TRUE ) == 0) {
                p->status = PEER_ACCEPT;
            }
        }
    }
	return p->status == PEER_ACCEPT;
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
