# include <glib.h>
# include <stdio.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <signal.h>
# include <string.h>
# include <time.h>
# include <stdio.h>
# include <fcntl.h>
# include <unistd.h>
# include <syslog.h>
# include "gateway.h"
# include "config.h"

extern GHashTable *peer_tab;

/************ Check peer timeouts **************/

gboolean check_peers( void *dummy ) {
    time_t now = time(NULL);

    if (CONFd("Verbosity") >= 5) g_message("Checking peers for expiration");
    g_hash_table_foreach_remove( peer_tab, (GHRFunc)check_peer_expire, &now );
    return TRUE;
}

/************* Connection handlers ************/
/* Irving - stability patch from Yurgi - don't return TRUE if the 
	   request isn't okay, or it will result in resources not being freed
	   and lots of time_wait sockets hanging around */
/* hsz: correct but his code is cleaner and perfromes the dereferencing as it should */
// This should always return FALSE to remove the handler for this g_io_add_watch connection, 
// and call the (GDestroyNotify) fn: handle_remove().
// handle_remove() now frees the request, closes & unrefs the GIOChannel, as well -TJ
gboolean handle_read( GIOChannel *sock, GIOCondition cond, http_request *h ) {
    guint n = 0;
    int fd = g_io_channel_unix_get_fd(sock);
    int fdh = g_io_channel_unix_get_fd(h->sock);

    if (CONFd("Verbosity") >= 9) g_message( "handle_read: entering...(h_peer_ip: %s h_sock_fd: %d orig_fd: %d)", h->peer_ip, fdh, fd );

    g_assert(sock != NULL);
    g_assert(fd != -1);
    g_assert(h != NULL);
    
    if (!(n = http_request_read(h))) {
	if (CONFd("Verbosity") >= 3) g_message("handle_read: ERROR! 0 length HTTP REQUEST, or ERROR in read.");
	return FALSE;
    }
    if (CONFd("Verbosity") >= 7) g_message( "handle_read: Finished reading request, now PARSING..." );
 
    if (!(http_request_ok(h))) {
	if (CONFd("Verbosity") >= 3) g_message("handle_read: ERROR! HTTP REQUEST malformed; Unable to pass to gateway!");
	return FALSE;
    }
    else {
        // Request-header password checking added for HotSpotZone extensions:
        // For access by remote ip adresses: we redefine peer ip, based on a self defined header in the request
        // NOCAT_REMOTE_ADDR , we only allow this (for security reasons) if there is also a header
        // NOCAT_GATEWAY_PASSWORD with a password as defined in the nocat conf
        // NOTE: sniffing could reveal the password
        if(HEADER("NOCAT_GATEWAY_PASSWORD")) {
            if(!strcmp(CONF("GatewayPassword"),HEADER("NOCAT_GATEWAY_PASSWORD"))) {
                if (HEADER("NOCAT_REMOTE_ADDR")) g_strncpy(h->peer_ip,HEADER("NOCAT_REMOTE_ADDR"),15);
                h->password_checked = TRUE;
            } else {
                g_warning("handle_read: Error: Wrong gateway password");
            }
        }
        
	if (CONFd("Verbosity") >= 7) g_message( "handle_read: HTTP REQUEST OK, passing to handle_request");
        handle_request(h);
    }
	
    if (CONFd("Verbosity") >= 9) g_message( "handle_read: exiting" );
    return FALSE;
}

void handle_remove( http_request *r ) {
    if (CONFd("Verbosity") >= 7) g_message("handle_remove: REMOVING sock/fd: %d and freeing request from peer %s", g_io_channel_unix_get_fd(r->sock), r->peer_ip);
    g_io_channel_unref(r->sock);
    g_io_channel_close(r->sock);
    http_request_free(r);
    if (CONFd("Verbosity") >= 7) g_message("handle_remove: RETURNING to Main Loop.");
}

gboolean handle_accept( GIOChannel *GIO_sock, GIOCondition cond, void *dummy ) {
    GIOChannel *GIO_conn;
    http_request *req;
    struct sockaddr_in sa;
    int fd, n;

    n = sizeof(sa);
    fd = accept( g_io_channel_unix_get_fd(GIO_sock), (struct sockaddr *) &sa, &n );
    g_assert( fd != -1 );
    if (CONFd("Verbosity") >= 7) g_message("handle_accept: ACCEPTED connection on sock_fd: %d, created conn_fd: %d", g_io_channel_unix_get_fd(GIO_sock), fd);

    GIO_conn = g_io_channel_unix_new( fd );
//  g_io_channel_set_close_on_unref( GIO_conn, 1 );

    req  = http_request_new( GIO_conn );
    if ( req != NULL ) { 
//        req.sock = GIO_conn;
//        req.sockfd = g_io_channel_unix_get_fd(GIO_conn);
        inet_ntop(AF_INET, &(sa.sin_addr), (char *)&(req->peer_ip), INET_ADDRSTRLEN);
        /*  g_strncpy((gchar *)&(req->peer_ip), 
			(gchar *)&(sa.sin_addr), sizeof(sa.sin_addr));
	*/
	if (CONFd("Verbosity") >= 6) g_message("handle_accept: (peer %s, fd: %d), Request QUEUED for read...", req->peer_ip, fd);
        g_io_add_watch_full( GIO_conn, 0,  G_IO_IN, (GIOFunc) handle_read, req, (GDestroyNotify) handle_remove );
    }
    return TRUE;
}


/************* main ************/

/* for SIGUSR1 = reinitialze fw */
static gboolean re_permit_peer( gpointer key, gpointer val, gpointer data ) {
    peer *p	 = (peer *) val;    
    if ( PEER_ACCEPT == p->status ) {
        if (CONFd("Verbosity") >= 3) g_message("re-permit peer %s", p->ip );
        peer_re_permit( nocat_conf, p );
    }
    return TRUE;
}

static void re_init_fw() {
    /* network settings may have changed */
    if (CONFd("Verbosity") >= 3) g_message("re-set network parameters");
    reset_network_defaults( nocat_conf );
    
    /* set the new rules */
    if (CONFd("Verbosity") >= 3) g_message("re-initialize the firewall");
    fw_re_init( nocat_conf );
    // now we have to make rules all currently accepted users
    if (CONFd("Verbosity") >= 3) g_message("re-permit the accepted peers");
    g_hash_table_foreach( peer_tab, (GHFunc) re_permit_peer, NULL );
}        

static int exit_signal = 0;
static FILE *pid_file  = NULL;
static guint log_hndl = 0;

gboolean check_exit_signal ( GMainLoop *loop ) {
    if (exit_signal) {
        g_message( "Caught exit signal %d!", exit_signal );
        if ( SIGUSR1 == exit_signal ) {
            re_init_fw();
            // reset signal flag
            exit_signal = 0;
        } else if ( SIGUSR2 == exit_signal ) {
            if (CONF("LeaseFile")) peer_file_sync(CONF("LeaseFile"));
            // reset signal flag
            exit_signal = 0;
        } else {
            if (pid_file != NULL) {
                unlink( NC_PID_FILE );
                fclose( pid_file );
            }
            g_main_quit( loop );
        }
    }
    return TRUE;
}


void signal_handler( int sig ) {
    switch(sig) {
	case SIGINT:
	    /*log_message(LOG_FILE,"interrupt signal caught");*/
	    exit_signal = sig;
	    break;
	case SIGTERM:
	    /*log_message(LOG_FILE,"terminate signal caught");*/
	    exit_signal = sig;
	    break;
	case SIGHUP:
	    /*log_message(LOG_FILE,"hangup signal caught");*/
	    break;
	case SIGUSR1:
	    /* we will reinitialize the firewall when this signal arrives */
	    exit_signal = sig;
	    break;
    }
}

void daemonize(void) {
    int f, r;

    if (getppid() == 1) return; /* already a daemon */

    r = fork();
    if (r<0) 
	exit(1); /* fork error */
    if (r>0) 
	exit(0); /* parent exits */

    /* child (daemon) continues */
    setsid(); /* obtain a new process group */

    for (f = getdtablesize(); f >= 0; --f)
	close(f); /* close all descriptors */

    f = open("/dev/null",O_RDWR); dup(f); dup(f); /* handle standard I/O */
    umask(027); /* set newly created file permissions */

    chdir( NC_STATE_DIR ); /* change running directory */

    pid_file = fopen( NC_PID_FILE, "w" );
    if (pid_file == NULL)
	exit(1); /* can not open */
    if (lockf( fileno(pid_file), F_TLOCK, 0 ) < 0) 
	exit(0); /* can not lock */

    /* write PID to lockfile */
    fprintf(pid_file, "%d\n", getpid());

    /* fssek() in order to sync the pid file */
    fseek(pid_file, 0L, SEEK_CUR);

    // fclose(lfp);

    // signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP,  signal_handler); /* catch hangup signal */
    signal(SIGTERM, signal_handler); /* catch kill signal */
    signal(SIGINT,  signal_handler);
    signal(SIGUSR1, signal_handler);
}

void g_syslog (const gchar *log_domain, GLogLevelFlags log_level, 
	       const gchar *message, gpointer user_data) {

    int priority;
    if (message==NULL || log_domain==NULL9
	return;
    switch (log_level & G_LOG_LEVEL_MASK) {
	case G_LOG_LEVEL_ERROR:	    priority = LOG_ERR;	    break;
	case G_LOG_LEVEL_CRITICAL:  priority = LOG_CRIT;    break;
	case G_LOG_LEVEL_WARNING:   priority = LOG_WARNING; break;
	case G_LOG_LEVEL_MESSAGE:   priority = LOG_NOTICE;  break;
	case G_LOG_LEVEL_INFO:	    priority = LOG_INFO;    break;
	case G_LOG_LEVEL_DEBUG:	    
	default:		    priority = LOG_DEBUG;   break;
				
    }
    int msize = strlen(tempmessage);
    gchar *tempmessage = (gchar*)malloc(msize+1);
    int i,c=0;
    /* temp hack, filter % (%% too unfortunatly)*/
    for (i=0;i<msize;i++) {
	    if (message[i]!='%') {
		tempmessage[c++]=message[i];
	    }
    }
    tempmessage[c++]=0;
    syslog( priority | LOG_DAEMON, tempmessage );
    free(tempmessage);
    if (log_level & G_LOG_FLAG_FATAL)
	exit_signal = -1;
}

void initialize_log (void) {
    if (strncmp( CONF("LogFacility"), "syslog", 6 ) == 0) {
	openlog( CONF("SyslogIdent"), LOG_CONS | LOG_PID, LOG_DAEMON );	
	log_hndl = g_log_set_handler( NULL, 
	    G_LOG_LEVEL_MASK | G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL,
	    g_syslog, NULL );
    }
}

int main (int argc, char **argv) {
    GMainLoop  *loop;
    GIOChannel *sock;
    guint t;

    //g_message("starting splashd");
    
    //g_message("read the configuration " NC_CONF_PATH "/nocat.conf");
    read_conf_file( NC_CONF_PATH "/nocat.conf" );
    
    /* initalize the log */
    initialize_log();
    
    g_message("splashd start");
    
    if (argc < 2 || strncmp(argv[1], "-D", 2) != 0)
	daemonize();

    if (CONFd("Verbosity") >= 3) g_message("set network parameters");
    set_network_defaults( nocat_conf );
    
    if (CONFd("Verbosity") >= 3) g_message("initialize driver");
    initialize_driver();
    
    if (CONFd("Verbosity") >= 3) g_message("initialize the listen socket");
    sock = http_bind_socket( 
	    CONF("GatewayAddr"), CONFd("GatewayPort"), CONFd("ListenQueue") );
    
    //moved to here so its only done if a socket can be created.
    if (CONFd("Verbosity") >= 3) g_message("initialize the firewall");
    fw_init( nocat_conf );
    
    if (CONFd("Verbosity") >= 3) g_message("initialize the peer table");
    peer_tab = g_hash_new();
    if (CONF("LeaseFile")) peer_file_reinit(CONF("LeaseFile"));
    
    if (CONFd("Verbosity") >= 3) g_message("initialize SIGUSR1(%d) signal handler",SIGUSR1);
    signal(SIGUSR1, signal_handler);
    
    if (CONFd("Verbosity") >= 3) g_message("initialize the main loop and handlers");
    loop = g_main_new(FALSE);
    g_io_add_watch( sock, G_IO_IN,  (GIOFunc) handle_accept, NULL );

/*    if(t = CONFd("PeerCheckTimeout")*1000) {
        if (CONFd("Verbosity") >= 3) g_message("Checking peers every %s seconds.", CONFd("PeerCheckTimeout"));
        g_timeout_add( t, (GSourceFunc) check_peers, NULL );
    } 
    else */ 
        g_timeout_add( 30000, (GSourceFunc) check_peers, NULL );

/*    if(t = CONFd("ExitCheckTimeout")) g_timeout_add( t, (GSourceFunc) check_exit_signal, loop );
    else */
	g_timeout_add( 1000, (GSourceFunc) check_exit_signal, loop );
 
    
    /* Go! */
    if (CONFd("Verbosity") >= 3) g_message("starting main loop");
    g_main_run( loop );
    if (CONFd("Verbosity") >= 3) g_message("exiting main loop");
    
    g_message("splashd exit");
    
    g_log_remove_handler( NULL, log_hndl );
    closelog();
    
    return 0;
}
