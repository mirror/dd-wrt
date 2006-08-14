# include <glib.h>
# include <string.h>
# include <stdio.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <time.h>
# include "gateway.h"

extern unsigned long int total_connections = 0;
extern time_t last_connection = 0;

void accept_peer ( http_request *h ) {
    peer *p;
   
    p  = find_peer( h->peer_ip );
    g_message( "Accepting peer %s", p->ip );

    increment_total_connections();

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

void capture_peer ( http_request *h, peer *p ) {
    gchar *redir = target_redirect( h );
    gchar *gw_addr = local_host( h );
    GHashTable *args = g_hash_new();
    GString *dest;

    g_hash_set( args, "redirect",   redir );
    g_hash_set( args, "token",	    get_peer_token(p) );
    g_hash_set( args, "mac",	    p->hw );
    g_hash_set( args, "timeout",    CONF("LoginTimeout") );
    g_hash_set( args, "gateway",    gw_addr );

    dest = build_url( CONF("AuthServiceURL"), args );
    http_send_redirect( h, dest->str );

    g_string_free( dest, 1 );
    g_hash_free( args );
    g_free( gw_addr );
    g_free( redir );
}

void logout_peer( http_request *h, peer *p ) {
    remove_peer( p );
    http_send_redirect( h, CONF("LogoutURL") );
}

GHashTable *gpg_decrypt( char *ticket ) {
    int rfd[2], wfd[2], r;
    gchar *gpg, *msg;
    gchar **arg;
    GHashTable *data;
    pid_t pid;

    gpg = parse_template( CONF("DecryptCmd"), nocat_conf );
    arg = g_strsplit( gpg, " ", 0 );

    r = pipe(rfd);
    if (r) {
	g_error("Can't open read pipe to gpg: %m");
	return 0;
    }

    r = pipe(wfd);
    if (r) {
	g_error("Can't open write pipe to gpg: %m");
	return NULL;
    }

    pid = fork();
    if (pid == -1) {
	g_error( "Can't fork to exec gpg: %m" );
	return NULL;

    } else if (pid == 0) {
	dup2( wfd[0], STDIN_FILENO );
	close( wfd[1] );

	dup2( rfd[1], STDOUT_FILENO );
	close( rfd[0] );

	r = execv( *arg, arg );
	g_error( "execv %s (%s) failed: %m", gpg, *arg ); // Shouldn't happen.
	exit(-1);
    } 

    close( wfd[0] );
    close( rfd[1] );

    msg = g_new0(char, BUFSIZ);
    g_snprintf( msg, BUFSIZ,
	"-----BEGIN PGP MESSAGE-----\n\n"
	"%s\n"
	"-----END PGP MESSAGE-----",
	ticket );
    r = write( wfd[1], msg, strlen(msg) ); 
    if (r == -1)
	g_error( "Can't write to gpg pipe: %m" );
    close( wfd[1] );

    r = read( rfd[0], msg, BUFSIZ ); 
    g_assert( r > 0 );
    close( rfd[0] );
    msg[r] = '\0';

    waitpid( pid, &r, 0 );
    if (! WIFEXITED( r ))
	g_warning( "gpg returned error: %d (signal %d)", 
	    WEXITSTATUS(r), WIFSIGNALED(r) ? WTERMSIG(r) : 0 );

    data = parse_conf_string( msg );

    g_strfreev( arg );
    g_free( gpg );
    g_free( msg );

    return data;
}

int verify_peer( http_request *h, peer *p ) {
    GHashTable *msg;
    gchar *action, *mode, *dest;
    gchar *ticket = QUERY("ticket");
    GString *m;

    if (ticket == NULL) {
	g_warning("Invalid ticket from peer %s", p->ip);
	return 0;
    }

    msg = gpg_decrypt( QUERY("ticket") );
    m = g_hash_as_string(msg);
    g_message( "auth message: %s", m->str);

    // Check username if set
    // Check MAC
    // Check token

    // Set user
    // Set groups
    // Set token

    action = g_hash_table_lookup(msg, "Action");
    if (strcmp( action, "Permit" ) == 0) {
	accept_peer( h );
    } else if (strcmp( action, "Deny" ) == 0) {
	remove_peer( p );
    } else {
	g_warning("Can't make sense of action %s!", action);
    }


    mode = g_hash_table_lookup(msg, "Mode");
    dest = g_hash_table_lookup(msg, "Redirect"); 
    if (strncmp(mode, "renew", 5) == 0) {
	http_send_header( h, 304, "No Response" );
    } else {
	http_send_redirect( h, dest );
    }

    g_hash_free( msg );
    return 1;
}

void handle_request( http_request *h ) {
    gchar *hostname = HEADER("Host");
    gchar *sockname = local_host(h);
    peer *p = find_peer( h->peer_ip );
    int r;
    if ( NULL != p ) {
        g_assert( sockname != NULL );
        g_assert( hostname != NULL );
        
        if (hostname == NULL || strcmp( hostname, sockname ) != 0) {
            capture_peer(h, p);
        } else if (strcmp( h->uri, "/logout" ) == 0) {
            // logout
            logout_peer(h, p);
        } else if (strcmp( h->uri, "/status" ) == 0 ) { // && h->password_checked ) {
            status_page( h );
        } else {
            // user with a ticket
            r = verify_peer(h, p);
            if (!r)
                capture_peer(h, p);
        }
    }
    g_free( sockname );
}

void initialize_driver (void) {
    return;
}
