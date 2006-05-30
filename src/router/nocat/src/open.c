# include <glib.h>
# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include "gateway.h"

gchar *splash_page = NULL;

/************* Capture and splash *************/

void capture_peer ( http_request *h ) {
    gchar *dest, *orig = target_redirect(h);
    gchar *redir = url_encode(orig);

    dest   = g_strdup_printf( "http://%s:%s/?redirect=%s",
	h->sock_ip, CONF("GatewayPort"), redir ); 

    http_send_redirect( h, dest );

    g_message( "Captured peer %s", h->peer_ip );

    g_free( orig  );
    g_free( redir );
    g_free( dest  );
}

void splash_peer ( http_request *h ) {
    GHashTable *data;
    gchar *path = NULL, *file, *action, *host;
    GIOError r;
   
    host = local_host( h );
    action = g_strdup_printf("http://%s/", host);
    data = g_hash_dup( nocat_conf );
    g_hash_merge( data, h->query );
    g_hash_set( data, "action", action );

    if (splash_page) {
	file = splash_page;
    } else {
	path = http_fix_path( CONF("SplashForm"), CONF("DocumentRoot") );
	file = load_file( path );
    } 

    r = http_serve_template( h, file, data );
    if (r == G_IO_ERROR_NONE)
	g_message( "Splashed peer %s", h->peer_ip );

    g_hash_free( data );
    g_free( action );
    g_free( host );
    if ( path != NULL ) {
	g_free( file );
	g_free( path );
    }
}

void handle_request( http_request *h ) {
    gchar *hostname = HEADER("Host");
    gchar *sockname = local_host(h);

    g_assert( sockname != NULL );

    if (hostname == NULL || strcmp( hostname, sockname ) != 0) {
	capture_peer(h);
    } else if (strcmp( h->uri, "/" ) == 0) {
	if ( QUERY("mode_login") != NULL || QUERY("mode_login.x") != NULL ) {
	    accept_peer(h);
	    sleep(2);
	    http_send_redirect( h, QUERY("redirect") );
	} else if ( QUERY("redirect") != NULL ) {
	    splash_peer(h);
	} else {
	    capture_peer(h);
	}
    } else if (strcmp( h->uri, "/status" ) == 0) {
	status_page( h );
    } else {
	http_serve_file( h, CONF("DocumentRoot") );
    }

    g_free( sockname );
}

/*** Dynamic splash page fetch ***/
# ifdef HAVE_LIBGHTTP
# include <ghttp.h>

static struct ghttp_process {
    ghttp_request *req;
    gchar *uri;
    gchar **buffer;
    int active;
    } ghttp_action;

void fetch_http_uri ( struct ghttp_process *proc, gchar *uri );

gboolean process_http_fetch (struct ghttp_process *proc) {
    ghttp_request *req = proc->req;
    int r;

    // g_message("processing HTTP request");
    r = ghttp_process(req);

    if (r == ghttp_not_done) {
	return TRUE;
    }

    else if (r == ghttp_error) {
	g_warning( "Can't load URL %s, retrying: %s", 
	    proc->uri, ghttp_get_error(req));
	
	/* We could retry at this point...
	ghttp_clean( req );
	proc->active = 0;
	fetch_http_uri( proc, proc->uri );
	return FALSE;
	*/
    }
    
    else if (ghttp_status_code(req) != 200) {
	g_warning( "Can't load URL %s: %d %s", proc->uri,
		    ghttp_status_code(req), ghttp_reason_phrase(req) );
    }

    else {
	ssize_t n = ghttp_get_body_len(proc->req);
	gchar **buffer = proc->buffer;

	if (*buffer) {
	    *buffer = g_renew( gchar, *buffer, n + 1 );
	} else {
	    *buffer = g_new( gchar, n + 1 );
	}

	g_strncpy( *buffer, ghttp_get_body(proc->req), n );
	(*buffer)[n] = '\0';

	g_message( "finished loading HTTP request" );
    }

    g_free(proc->uri);
    ghttp_request_destroy(proc->req);
    proc->active = 0;
    return FALSE;
}

gboolean http_fetch_callback( 
    GIOChannel *sock, GIOCondition cond, struct ghttp_process *proc ) {

    if (process_http_fetch(proc)) 
	return TRUE;

    g_io_channel_unref(sock);
    return FALSE;
}

void fetch_http_uri ( struct ghttp_process *proc, gchar *uri ) {
    ghttp_request *req = ghttp_request_new();
    GIOChannel *sock;
    int r;

    if (proc->active) {
	g_warning("attempt to interrupt existing HTTP request");
	return;
    }    

    proc->req = req;
    proc->uri = uri;
    proc->active = 1;

    ghttp_set_uri(req, uri);

    /* Close the connection after you are done. */
    ghttp_set_header(req, http_hdr_Connection, "close");

    r = ghttp_prepare(req);
    if (r < 0) {
	g_warning( "Can't request splash page from %s", uri );
	ghttp_request_destroy(req);
	return;
    }

    ghttp_set_sync(req, ghttp_async);

    if (process_http_fetch(proc)) {
	sock = g_io_channel_unix_new( ghttp_get_socket(req) );
	g_io_add_watch( sock, G_IO_IN, (GIOFunc) http_fetch_callback, proc);
    }
}

gboolean fetch_splash_page (struct ghttp_process *proc) {
    gchar *uri;
    if (! proc->active) {
	uri = parse_template( CONF("SplashURL"), nocat_conf );
	g_message( "fetching remote splash page: %s", uri );
	fetch_http_uri( proc, uri ); 
    }
    return TRUE;
}

void initialize_driver (void) {
    g_message("initializing dynamic splash page");
    if (CONF("SplashURL") != NULL) { 
	ghttp_action.buffer = &splash_page;
	fetch_splash_page( &ghttp_action );
	g_timeout_add( CONFd("SplashTimeout") * 1000, 
		(GSourceFunc) fetch_splash_page, &ghttp_action );
    }
}

# else /* don't HAVE_LIBGHTTP */

void initialize_driver (void) {
    g_message("initializing static splash page");
    return;
}

# endif /* HAVE_LIBGHTTP */
