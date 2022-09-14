# include <glib.h>
# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include <time.h>
# include "gateway.h"

gchar *splash_page = NULL;



/************* Permit and deny peers *************/



void accept_peer ( http_request *h ) {
    peer *p;
   
    p  = find_peer( h->peer_ip );
    if ( NULL != p ) {
	if (CONFd("Verbosity") >= 1) g_message( "accept_peer: adding %s", p->ip );

        increment_total_connections();

        peer_permit( nocat_conf, p );

        if (CONFd("ForcedRedirect") >= 1 ) {
            http_add_header ( h, "Location", CONF("HomePage") );
        } else {
            http_add_header ( h, "Location", QUERY("redirect") );
        }
        http_send_header( h, 302, "Moved" );
        http_send_redirect( h, QUERY("redirect") );

        if (CONF("LeaseFile")) peer_file_sync(CONF("LeaseFile"));

    }
}

void remove_peer ( peer *p ) {
    if (CONFd("Verbosity") >= 1) g_message( "remove_peer: removing %s", p->ip );
    peer_deny( nocat_conf, p );
    peer_free(p);
    if (CONF("LeaseFile")) peer_file_sync(CONF("LeaseFile"));
}

gboolean check_peer_expire ( gchar *ip, peer *p, time_t *now ) {
// IDLE CHECK
    if (0 != p->idle_check) {
        if (CONFd("Verbosity") >= 9) g_message( "check_peer_expire: IDLE check: %s: %lu sec. remain", ip, p->idle_check - *now );
        if (p->idle_check <= *now) {
            if (CONFd("Verbosity") >= 9) g_message("check_peer_expire: MISSING check: %s: missing count = %lu", ip, p->missing_count);
            if ( !find_peer_arp(p->ip) ) {
                p->missing_count++;
                if (p->missing_count > CONFd("MaxMissedARP")) {
                    if (CONFd("Verbosity") >= 1) g_message( "check_peer_expire: removing IDLE peer %s", ip );
                    remove_peer( p );
                    return TRUE;
                }
            }
	    else p->missing_count = 0;
            p->idle_check = *now + CONFd("IdleTimeout");
        }
    }
// TIME OUT EXPIRED
    if (CONFd("Verbosity") >= 9) g_message( "check_peer_expire: EXPIRED check: %s: %ld sec. remain", ip, p->expire - *now );
    if (p->expire <= *now) {
        if (CONFd("Verbosity") >= 1) g_message( "check_peer_expire: removing EXPIRED peer %s", ip );
	remove_peer( p );
	return TRUE;
    } else {
	return FALSE;
    }
} 

/************* Capture and splash *************/

void capture_peer ( http_request *h ) {
    gchar *dest, *orig = target_redirect(h);
    gchar *redir = url_encode(orig);

    dest   = g_strdup_printf( "http://%s:%s/?redirect=%s",
	h->sock_ip, CONF("GatewayPort"), redir ); 

    http_send_redirect( h, dest );

    if(CONFd("Verbosity") >= 4) g_message( "capture_peer: %s requested http://%s%s, CAPTURED, REDIRECTED to: http://%s:%s/?redirect=%s", 
                                           h->peer_ip, HEADER("Host"), h->uri, h->sock_ip, CONF("GatewayPort"), orig);

    g_free( orig  );
    g_free( redir );
    g_free( dest  );
}

void splash_peer ( http_request *h ) {
    GHashTable *data;
    gchar *hostname = HEADER("Host");
    gchar *path = NULL, *file, *action, *localhost;
    GIOError r;
   
    localhost = local_host( h );
    action = g_strdup_printf("http://%s/", localhost);
    data = g_hash_dup( nocat_conf );
    g_hash_merge( data, h->query );
    g_hash_set( data, "action", action );

    if (splash_page) {
	if (CONFd("Verbosity") >= 4) g_message( "splash_peer: %s got SERVED remote SplashURL: %s (cached)", h->peer_ip, CONF("SplashURL") );
	file = splash_page;
    } else {
	path = http_fix_path( CONF("SplashForm"), CONF("DocumentRoot") );
	if (CONFd("Verbosity") >= 4) g_message( "splash_peer: %s got SERVED: %s", h->peer_ip, path );
	file = load_file( path );
    } 

    r = http_serve_template( h, file, data );
    if (r == G_IO_ERROR_NONE) {
	// if (CONFd("Verbosity") >= 9) g_message( "splash_peer: %s ", file );
    }

    g_hash_free( data );
    g_free( action );
    g_free( localhost );
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
	if (CONFd("Verbosity") >= 4) g_message( "handle_request: %s requested http://%s%s on CAPTURED port %s.", 
                                                h->peer_ip, hostname, h->uri, CONF("GatewayPort") );
	capture_peer(h);
    } else if (strcmp( h->uri, "/" ) == 0) { // i.e. this was a POST with only QUERY items (e.g. redirect, accept_terms, login_mode, 3tc)
	/* Irving - Force addition of an accept_terms checkbox */
	if ( QUERY("accept_terms") && (strncmp(QUERY("accept_terms"),"yes",3) == 0)  
	     && ( QUERY("mode_login") != NULL || QUERY("mode_login.x") != NULL ) )
	{
	    if (CONFd("Verbosity") >= 4) g_message( "handle_request: %s has ACCEPTED on our port: %s, REDIRECTED to: %s", h->peer_ip, CONF("GatewayPort"), url_decode(QUERY("redirect")) );
	    accept_peer(h);
	    sleep(2);
	} else if ( QUERY("redirect") != NULL ) {
	    if (CONFd("Verbosity") >= 4) g_message( "handle_request: %s is being SPLASHED.", h->peer_ip );
	    splash_peer(h);
	} else {
	    if (CONFd("Verbosity") >= 4) g_message( "handle_request: %s is being RECAPTURED.", h->peer_ip );
	    capture_peer(h);
	}
    } else if (strcmp( h->uri, "/status" ) == 0) {
	if (CONFd("Verbosity") >= 4) g_message( "handle_request: %s requested STATUS page, SERVED.", h->peer_ip );
        status_page( h );
    } else {
	if (CONFd("Verbosity") >= 4) g_message( "handle_request: %s gets SERVED %s%s.", h->peer_ip, CONF("DocumentRoot"), h->uri );
	http_serve_file( h, CONF("DocumentRoot") );
    }

    g_free( sockname );
}

/*** Dynamic splash page fetch ***/
# ifdef HAVE_LIBGHTTP
# include "ghttp.h"

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
	g_warning( "process_http_fetch: Can't load URL %s, retrying: %s", 
	    proc->uri, ghttp_get_error(req));
	
	/* We could retry at this point...
	ghttp_clean( req );
	proc->active = 0;
	fetch_http_uri( proc, proc->uri );
	return FALSE;
	*/
    }
    
    else if (ghttp_status_code(req) != 200) {
	g_warning( "process_http_fetch: Can't load URL %s: %d %s", proc->uri,
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

	g_message( "process_http_fetch: finished loading HTTP request" );
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
	g_warning("fetch_http_uri: attempt to interrupt existing HTTP request");
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
	g_warning( "fetch_http_uri: Can't request splash page from %s", uri );
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
	g_message( "fetch_splash_page: %s", uri );
	fetch_http_uri( proc, uri ); 
    }
    return TRUE;
}

void initialize_driver (void) {
    g_message("initialize_driver: Retrieving dynamic splash page");
    if (CONF("SplashURL") != NULL) { 
	ghttp_action.buffer = &splash_page;
	fetch_splash_page( &ghttp_action );
	g_timeout_add( CONFd("SplashURLTimeout") * 1000, 
		(GSourceFunc) fetch_splash_page, &ghttp_action );
    }
}

# else /* don't HAVE_LIBGHTTP */

void initialize_driver (void) {
    g_message("initialize_driver: No fetch required (static splash page)");
    return;
}

# endif /* HAVE_LIBGHTTP */
