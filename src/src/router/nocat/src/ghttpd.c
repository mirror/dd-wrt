# include <glib.h>
# include <stdio.h>
# include <netinet/in.h>
# include <signal.h>
# include <sys/socket.h>
# include "http.h"
# include "util.h"
# include "conf.h"
# include "config.h"

void handle_free (GIOChannel *sock, http_request *h) {
    g_io_channel_unref(sock);
    http_request_free(h);
}

gboolean handle_write( GIOChannel *sock, GIOCondition cond, http_request *h ) {
    http_serve_file( h, CONF("DocumentRoot") );
    g_io_channel_close( sock );
    handle_free(sock, h);
    return FALSE;
}

gboolean handle_read( GIOChannel *sock, GIOCondition cond, http_request *h ) {
    ssize_t n = http_request_read( h );

    if (http_request_ok(h)) {
	g_io_add_watch( sock, G_IO_OUT, (GIOFunc)handle_write, h );
	return FALSE;
    } else if (n == 0) {
	handle_free(sock, h);
	return FALSE;
    } else {
	return TRUE;
    }
}

gboolean handle_accept( GIOChannel *sock, GIOCondition cond, gpointer data ) {
    struct sockaddr_in peer_addr;
    int peer_fd, n = sizeof(peer_addr);
    GIOChannel *peer;
    http_request *req;

    peer_fd = accept( g_io_channel_unix_get_fd(sock),
	    (struct sockaddr *) &peer_addr, &n);
    g_assert( peer_fd != -1 );

    peer = g_io_channel_unix_new( peer_fd );
    req  = http_request_new( peer );
    g_io_add_watch( peer, G_IO_IN, (GIOFunc)handle_read, req );

    return TRUE;
}


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
    GMainLoop *loop;
    GIOChannel *sock;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT,  handle_sigint);

    read_conf_file( NC_CONF_PATH "/nocat.conf" );

    loop = g_main_new(FALSE);

    sock = http_bind_socket( "0.0.0.0", 80, 5 );
    g_io_add_watch( sock, G_IO_IN, (GIOFunc)handle_accept, NULL );
    g_timeout_add( 1000, (GSourceFunc) check_sigint, loop );

    g_message("starting main loop");
    g_main_run( loop );
    g_message("exiting main loop");
    return 0;
}
