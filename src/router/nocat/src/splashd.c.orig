# include <glib.h>
# include <stdio.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <signal.h>
# include <string.h>
# include <time.h>
# include "gateway.h"
# include "config.h"

extern GHashTable *peer_tab;

/************ Check peer timeouts **************/

gboolean check_peers( void *dummy ) {
    time_t now = time(NULL);
    g_message("Checking peers for expiration");
    g_hash_table_foreach_remove( peer_tab, (GHRFunc)check_peer_expire, &now );
    return TRUE;
}

/************* Connection handlers ************/

gboolean handle_read( GIOChannel *sock, GIOCondition cond, http_request *h ) {
    g_debug( "entering handle_read" );
    http_request_read( h );

    if (! http_request_ok(h))
	return TRUE;

    handle_request(h);

    g_io_channel_close( h->sock );
    g_io_channel_unref( h->sock );
    http_request_free( h );
    g_debug( "exiting handle_read" );
    return FALSE;
}

gboolean handle_accept( GIOChannel *sock, GIOCondition cond, void *dummy ) {
    GIOChannel *conn;
    http_request *req;
    int fd;

    fd = accept( g_io_channel_unix_get_fd(sock), NULL, NULL );
    g_assert( fd != -1 );

    conn = g_io_channel_unix_new( fd );
    req  = http_request_new( conn );

    g_io_add_watch( conn, G_IO_IN, (GIOFunc) handle_read, req );
    return TRUE;
}


/************* main ************/

static int caught_sigint = 0;

gboolean check_sigint ( GMainLoop *loop ) {
    if (caught_sigint) {
	g_message( "Caught SIGINT!" );
	g_main_quit( loop );
    }
    return TRUE;
}

void handle_sigint ( int sig ) {
    caught_sigint++;
}

int main (int argc, char **argv) {
    GMainLoop  *loop;
    GIOChannel *sock;

    /* Initialize signal handlers */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT,  handle_sigint);

    /* read nocat.conf */
    read_conf_file( NC_CONF_PATH "/nocat.conf" );

    initialize_driver();

    /* initialize the firewall */
    fw_init( nocat_conf );

    /* initialize the peer table */
    peer_tab = g_hash_new();

    /* initialize the listen socket */
    sock = http_bind_socket( 
	    CONF("GatewayAddr"), CONFd("GatewayPort"), CONFd("ListenQueue") );

    /* initialize the main loop and handlers */
    loop = g_main_new(FALSE);
    g_io_add_watch( sock, G_IO_IN,  (GIOFunc) handle_accept, NULL );
    g_timeout_add( 30000, (GSourceFunc) check_peers, NULL );
    g_timeout_add( 1000, (GSourceFunc) check_sigint, loop );
    
    /* Go! */
    g_message("starting main loop");
    g_main_run( loop );
    g_message("exiting main loop");
    return 0;
}
