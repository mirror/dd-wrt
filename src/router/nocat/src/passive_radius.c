# include <glib.h>
# include <string.h>
# include <unistd.h>
# include <time.h>
# include "http.h"
# include "conf.h"
# include "util.h"

# include "radius.h"
# include "firewall.h"


void increment_total_connections ( void );
void status_page ( http_request *h );
peer *find_peer ( const char *ip );

/************** RADIUS HELPER **********************/



int radius_auth_peer( peer *p, const char* name, const char* password, char* message, char* errstr ) {
    int result;
    unsigned int session_timeout;
    session_timeout = 0;
    
    radius_server_config_rec *scr = create_radius_server_config();
    scr->radius_ip= get_ip_addr(CONF("RADIUSAuthServer"));
    scr->port=CONFd("RADIUSAuthPort");
    scr->secret=CONF("RADIUSAuthSecret");
    scr->secret_len=strlen(scr->secret);
    scr->nas_id=CONF("RADIUSAuthNASIdentifier");
    scr->wait=CONFd("RADIUSAuthWait");
    scr->retries=CONFd("RADIUSAuthRetries");
    scr->next=0;
        
    peer_set_name(p, name); // keep for accounting
    peer_set_password(p, password);

    if (CONFd("Verbosity") >= 3) g_message("try radius_auth:%s", name);
    result = radius_auth( scr, name , password, CONF("GatewayMAC"), p->hw, message, errstr, &session_timeout );

    if (result) { 
      peer_set_auth_check(p,time( NULL ) + CONFd("RenewTimeout"));
      if (CONFd("Verbosity") >= 3) g_message ("radius_auth ok");
	  if ( session_timeout > 0 ) {
            if (CONFd("Verbosity") >= 3) g_message ("session_timeout = %d", session_timeout);
	        p->expire = time ( NULL ) + session_timeout;
      }
    }
    g_free(scr);
    
    return result;
}

int radius_acct_peer_start( peer *p, char* errstr ) {
    int result;
    char* session_id;
    
    radius_server_config_rec *scr = create_radius_server_config();
    scr->radius_ip= get_ip_addr(CONF("RADIUSAcctServer"));
    scr->port=CONFd("RADIUSAcctPort");
    scr->secret=CONF("RADIUSAcctSecret");
    scr->secret_len=strlen(scr->secret);
    scr->nas_id=CONF("RADIUSAcctNASIdentifier");
    scr->wait=CONFd("RADIUSAcctWait");
    scr->retries=CONFd("RADIUSAcctRetries");
    scr->next=0;
    
    session_id = g_strdup_printf("%lu-%s-%lu", (long)p, p->hw, p->connected );
    if (CONFd("Verbosity") >= 3) g_message("create session id=%s", session_id);
    peer_set_session_id(p, session_id ); // set for accounting
    
    result = radius_acct_start(scr, p->name, p->session_id, CONF("GatewayMAC"), p->hw, errstr);
    
    if ( result ) { 
        if (CONFd("Verbosity") >= 2) g_message( "Accounting started for %s,%s at %lu", p->ip, p->hw, time( NULL ) );
        p->acct_session_start = time( NULL );
    } else {
        g_warning( "Accounting start failed for %s,%s at %lu, error: %s", p->ip, p->hw, time( NULL ), errstr );
    }
    
    g_free(scr);
    g_free(session_id);
    
    return result;
}

int radius_acct_peer_stop( peer *p, int terminate_cause, char* errstr ) {
    int result;
    int acct_session_time;
    if (CONFd("Verbosity") >= 3) g_message ("radius_acct_peer_stop peer: %s, %s", p->ip, p->hw);
    acct_session_time = time( NULL ) - p->acct_session_start;

    // don't count idle time
    if ( RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT == terminate_cause ) {
        acct_session_time -= CONFd("IdleTimeout") * CONFd("MaxMissedARP");
    }
   
    if (acct_session_time < 0) acct_session_time = 0;
    if (CONFd("Verbosity") >= 3) g_message ("peer %s, %s acct_session_time = %d", p->ip, p->hw, acct_session_time);
    radius_server_config_rec *scr = create_radius_server_config();
    scr->radius_ip= get_ip_addr(CONF("RADIUSAcctServer"));
    scr->port=CONFd("RADIUSAcctPort");
    scr->secret=CONF("RADIUSAcctSecret");
    scr->secret_len=strlen(scr->secret);
    scr->nas_id=CONF("RADIUSAcctNASIdentifier");
    scr->wait=CONFd("RADIUSAcctWait");
    scr->retries=CONFd("RADIUSAcctRetries");
    scr->next=0;
    if (CONFd("Verbosity") >= 3) g_message("try radius_acct_stop peer %s, %s, session %s",p->ip,p->hw,p->session_id);
    result = radius_acct_stop(scr, p->name, p->session_id, CONF("GatewayMAC"), p->hw, p->acct_input_octets, p->acct_output_octets, acct_session_time, terminate_cause, errstr);
    if ( result ) { 
        if (CONFd("Verbosity") >= 2) g_message( "Accounting stopped for %s,%s at %lu with input:%u, output:%u, session time:%u", p->ip, p->hw, time( NULL ), p->acct_input_octets, p->acct_output_octets, acct_session_time );
    } else {
        g_warning( "Accounting stop failed for %s,%s at %lu with input:%u, output:%u, session time:%u, error: %s", p->ip, p->hw, time( NULL ), p->acct_input_octets, p->acct_output_octets, acct_session_time, errstr );
    }
    
    g_free(scr);
    
    return result;
}



/******* accept, logout, remove, check_expire ***************/

void accept_peer ( http_request *h ) {
    peer *p;
    gchar *dest,*redirect,*encoded_redirect;
    char message[256];
    char errstr[256];
    gchar *redirect_page;
    
    //G_LOCK(peer_tab);
    p  = find_peer( h->peer_ip );
    if ( NULL != p ) {
        
        // encode the orginal site
        
        redirect = QUERY("redirect");
        if ( redirect ) {
	    g_message( "original page %s", redirect );
            encoded_redirect  = url_encode( redirect );
	    g_message( "encoded original page %s", encoded_redirect );
        } else 
            encoded_redirect = NULL;
        
        if (CONFd("Verbosity") >= 5)  g_message("try radius auth");  
        
        if (  radius_auth_peer( p, QUERY("name"), QUERY("password"),message, errstr ) ) {
            // goto confirm page
            if (CONFd("Verbosity") >= 1) g_message( "Accepting peer %s", p->ip );
            if (CONFd("Verbosity") >= 5) g_message( "RADIUS Message %s", message );
            
            increment_total_connections();
            
            peer_permit( nocat_conf, p );
            
            if ( PEER_ACCEPT == p->status ) {
                // start accounting
                if ( !radius_acct_peer_start( p, errstr) ) {
                    g_message( "RADIUS Accounting Error %s", errstr );
                }
            }
            
            // redirect to confirm page
            redirect_page = CONF("ConfirmPage");
            
            dest   = g_strdup_printf( "%s%sgateway_ip=%s&gateway_port=%s&MC=%s&OS=%s", redirect_page, ( NULL == rindex(redirect_page,'?') ? "?" : "&" ), h->sock_ip, CONF("GatewayPort"), p->hw, ( encoded_redirect != NULL ? encoded_redirect : "" ) );
            
        } else {
            // back to login page retry login?
            
            g_message( "Not accepted peer %s", p->ip );
            g_message( "RADIUS Message %s", message );
            g_message( "RADIUS Error %s", errstr );
            redirect_page = CONF("ConfirmPage");
            
            QUERY("redirect"), 
            
            dest   = g_strdup_printf( "%s%sgateway_ip=%s&gateway_port=%s&radius=%s&OS=%s", redirect_page, ( NULL == rindex(redirect_page,'?') ? "?" : "&" ), h->sock_ip, CONF("GatewayPort"), errstr, ( encoded_redirect != NULL ? encoded_redirect : "" ) );
            
        }
        
        //G_UNLOCK(peer_tab);
        
        http_add_header ( h, "Location", dest );
        http_send_header( h, 302, "Moved" );
        g_message( "Redirect %s", dest );
        g_free(dest);
        g_free(encoded_redirect);
    }
}

void traffic_count_peer( peer *p ) {
	gchar line[30];
	memset(line,sizeof(line),0);
    fw_perform_return("TrafficInputCountCmd", nocat_conf, p, line, sizeof(line) );
	p->acct_input_octets = atoi(line);
	memset(line,sizeof(line),0);
    fw_perform_return("TrafficOutputCountCmd", nocat_conf, p, line, sizeof(line) );
	p->acct_output_octets = atoi(line);
}

int arp_peer_found( peer *p ) {
    gchar line[30];
    int exit = -1;
	memset(line,sizeof(line),0);
    fw_perform_return("CheckARPCmd", nocat_conf, p, line, sizeof(line) );
	exit = atoi(line);
    if (CONFd("Verbosity") >= 5) g_message( "CheckARPCmd returned: '%s'->%u for peer %s", line, exit, p->ip );
    return (exit == 0); // success
}

void remove_peer ( peer *p, int terminate_cause ) {
    char errstr[256];
	if (CONFd("Verbosity") >= 1) g_message( "Removing peer %s", p->ip );
    
    if ( PEER_ACCEPT == p->status ) {
		 traffic_count_peer(p);
		 if( !radius_acct_peer_stop( p, terminate_cause, errstr) ) {
          g_message( "RADIUS Accounting Error %s", errstr );
		 }
    }
    
    peer_deny( nocat_conf, p );
    
    // remove from hashtable is done always via check_peer_expire
}

void logout_peer ( http_request *h ) {
    peer *p;
    gchar *dest;
    gchar *redirect_page = CONF("LogoutPage");
    
    //G_LOCK(peer_tab);
    p  = find_peer( h->peer_ip );
    if ( NULL != p ) {
        dest   = g_strdup_printf( "%s%sMC=%s", redirect_page, ( NULL == rindex(redirect_page,'?') ? "?" : "&" ), p->hw );
        
        remove_peer(p, RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST );
        
        //G_UNLOCK(peer_tab);
    
        http_add_header ( h, "Location", dest );
        http_send_header( h, 302, "Moved" );
        
        g_free(dest);
    }
}


gboolean check_peer_expire ( gchar *ip, peer *p, time_t *now ) {
  
    int remove = FALSE;
    char message[256];
    char errstr[256];
    
    if ( PEER_DENY == p->status ) {
       // already accounting stopped and denied by explicit logout, remove now
       remove = TRUE;
    };
    // RE AUTHORIZATION
    if ( !remove ) { // not yet removed test for idle
        if (0 != p->auth_check) {
             if (CONFd("Verbosity") >= 3) g_message( "Checking peer %s for reauthorization: %lu sec. remain", ip, p->auth_check - *now );
        }    
        if (0 != p->auth_check && p->auth_check <= *now) {
          if (CONFd("Verbosity") >= 2) g_message("Checking peer %s for reauthorization.", ip);
          if ( PEER_ACCEPT == p->status && !radius_auth_peer( p, p->name, p->password, message, errstr ) ){
            remove_peer( p, RADIUS_ACCT_TERMINATE_CAUSE_SESSION_TIMEOUT );
            remove = TRUE;
          } 
        }
    }
    // IDLE
    if ( !remove ) { // not yet removed test for idle
        if (0 != p->idle_check) {
             if (CONFd("Verbosity") >= 3) g_message( "Checking peer %s for idle: %lu sec. remain", ip, p->idle_check - *now );
        }    
        if (0 != p->idle_check && p->idle_check <= *now) {
           if (CONFd("Verbosity") >= 2) g_message("Checking peer %s for idle: missing count = %lu", ip, p->missing_count);
           if ( arp_peer_found(p) ) {
               p->missing_count = 0;
           } else {
               p->missing_count++;
               if (p->missing_count > CONFd("MaxMissedARP")) { 
                   remove_peer( p, RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT  );
                   remove = TRUE;
               }
           }
           p->idle_check = *now + CONFd("IdleTimeout");
        }
    }
    // TIME OUT EXPIRED
    if ( !remove ) { // not yet removed test for expire
        if (0 != p->expire) {
             if (CONFd("Verbosity") >= 3) g_message( "Checking peer %s for expire: %lu sec. remain", ip, p->expire - *now );
        }    
        if (0 != p->expire && p->expire <= *now) {
            if (CONFd("Verbosity") >= 3) g_message( "Peer %s expired. Try an reauth first", ip); // maybe the user became some extra time within the last expire time span
            if ( PEER_ACCEPT == p->status && !radius_auth_peer( p, p->name, p->password, message, errstr ) ){
                if (CONFd("Verbosity") >= 2) g_message( "Peer %s expired. Reauth failed, thus try a removal.", ip);
                remove_peer( p, RADIUS_ACCT_TERMINATE_CAUSE_SESSION_TIMEOUT );
                remove = TRUE;
            } // elso do not remove !
        }
    }
    return remove;
} 

/************* Capture *************/

void capture_peer ( http_request *h ) {
    peer *p = find_peer ( h->peer_ip );;
    gchar *host   = HEADER("Host"); 
    gchar *orig, *redir, *dest = NULL;
    gchar *login_page   = CONF("LoginPage");
    if ( NULL != p ) {
        if ( host != NULL ) {
            orig = g_strdup_printf( "http://%s%s", HEADER("Host"), h->uri );
        } else {
            orig = CONF("HomePage");
        }
        
	
        g_message( "original page %s", orig );
	redir  = url_encode( orig );
        g_message( "encoded original page %s", redir );
	
        // login page, should perfrom redirect to logal gate way with resulting name and password
        // original site moved to end to assuer correct passing of mac adresses
        dest = g_strdup_printf( "%s%sgateway_ip=%s&gateway_port=%s&gateway_mac=%s&MC=%s&OS=%s", login_page, ( NULL == rindex(login_page,'?') ? "?" : "&" ), h->sock_ip, CONF("GatewayPort"), CONF("GatewayMAC"), p->hw, redir );
        g_free( orig  );
        g_free( redir );
    } else {
        // no real peer because its a local test
        dest = g_strdup_printf( "%s%sgateway_ip=%s&gateway_port=%s&gateway_mac=%s", login_page, ( NULL == rindex(login_page,'?') ? "?" : "&" ), h->sock_ip, CONF("GatewayPort"), CONF("GatewayMAC"));
    }    
    http_add_header ( h, "Location", dest );
    http_send_header( h, 302, "Moved" );
    g_message( "Redirect %s", dest );
    g_free( dest  );
}

void handle_request( http_request *h ) {
    gchar *hostname = HEADER("Host");
    gchar *sockname = g_strdup_printf( "%s:%s", h->sock_ip, CONF("GatewayPort") ); 
    
    g_assert( sockname != NULL );
    // g_message( "got sockname: '%s'", sockname );
    g_assert( hostname != NULL );
    // g_message( "got hostname: '%s'", hostname );
    
    // g_message( "%s == %s = %d", hostname, sockname, n );
    if ( hostname == NULL || strcmp( h->uri, "/test" ) == 0 || ( strcmp( hostname, sockname ) != 0  && strcmp( hostname, h->sock_ip ) != 0 ) ) {
        // a firewall redirect occured
        capture_peer(h);
    } else if ( QUERY("login") != NULL ) {
        accept_peer(h);
    } else if ( QUERY("logout") != NULL ) {
        logout_peer(h);
    } else if (strcmp( h->uri, "/status" ) == 0 ) {  // && h->password_checked ) {
        status_page( h );
    } else {
        http_serve_file( h, CONF("DocumentRoot") );
    }
    g_free( sockname );
}

void initialize_driver (void) {
    return;
}
