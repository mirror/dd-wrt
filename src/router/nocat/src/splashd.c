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

static int exit_signal = 0;
static FILE *pid_file  = NULL;

gboolean check_exit_signal ( GMainLoop *loop ) {
    if (exit_signal) {
	g_message( "Caught exit signal %d!", exit_signal );
	if (pid_file != NULL) {
	    unlink( NC_PID_FILE );
	    fclose( pid_file );
	}
	g_main_quit( loop );
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

    // fclose(lfp);

    // signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP,  signal_handler); /* catch hangup signal */
    signal(SIGTERM, signal_handler); /* catch kill signal */
    signal(SIGINT,  signal_handler);
}

void g_syslog (const gchar *log_domain, GLogLevelFlags log_level, 
	       const gchar *message, gpointer user_data) {

    int priority;

    switch (log_level & G_LOG_LEVEL_MASK) {
	case G_LOG_LEVEL_ERROR:	    priority = LOG_ERR;	    break;
	case G_LOG_LEVEL_CRITICAL:  priority = LOG_CRIT;    break;
	case G_LOG_LEVEL_WARNING:   priority = LOG_WARNING; break;
	case G_LOG_LEVEL_MESSAGE:   priority = LOG_NOTICE;  break;
	case G_LOG_LEVEL_INFO:	    priority = LOG_INFO;    break;
	case G_LOG_LEVEL_DEBUG:	    
	default:		    priority = LOG_DEBUG;   break;
				
    }

    syslog( priority | LOG_DAEMON, message );

    if (log_level & G_LOG_FLAG_FATAL)
	exit_signal = -1;
}

void initialize_log (void) {
    if (strncmp( CONF("LogFacility"), "syslog", 6 ) == 0) {
	openlog( CONF("SyslogIdent"), LOG_CONS | LOG_PID, LOG_DAEMON );	
	g_log_set_handler( NULL, 
	    G_LOG_LEVEL_MASK | G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL,
	    g_syslog, NULL );
    }
}

int main (int argc, char **argv) {
    GMainLoop  *loop;
    GIOChannel *sock;

    /* read nocat.conf */
    read_conf_file( NC_CONF_PATH "/nocat.conf" );

    if (argc < 2 || strncmp(argv[1], "-D", 2) != 0)
	daemonize();

    /* initalize the log */
    initialize_log();
    
    /* set network parameters */
    set_network_defaults( nocat_conf );

    /* initialize the gateway type driver */
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
    g_timeout_add( 1000, (GSourceFunc) check_exit_signal, loop );
    
    /* Go! */
    g_message("starting main loop");
    g_main_run( loop );
    g_message("exiting main loop");
    return 0;
}
