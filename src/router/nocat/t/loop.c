# include <glib.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <stdio.h>
# include "http.h"
# include "util.h"

int bind_socket( const char *ip, int port, int queue ) { 
    struct sockaddr_in addr;
    int fd, r, n;
    
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    r = inet_aton( ip, &addr.sin_addr );
    if (r == 0)
	g_error("inet_aton failed on %s: %m", ip);	

    fd = socket( PF_INET, SOCK_STREAM, 0 );
    if (r == -1)
	g_error("socket failed: %m");	
    
    r = bind( fd, (struct sockaddr *)&addr, sizeof(addr) );
    if (r == -1)
	g_error("bind failed on %s: %m", ip);	

    r = setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n) );
    if (r == -1)
	g_error("setsockopt failed on %s: %m", ip);	

    r = listen( fd, queue );
    if (r == -1)
	g_error("listen failed on %s: %m", ip);	

    return fd;
}

gboolean handle_read( GIOChannel *sock, GIOCondition cond, http_request *h ) {
    static gchar hdr_out[] = "Content-type: text/plain\r\n\r\n";
    GString *msg;
    guint n;

    while (http_request_read( h, sock ));

    if (http_request_ok(h)) {
	g_io_channel_write( sock, hdr_out, sizeof(hdr_out), &n );

	msg = g_hash_as_string( h->header );
	g_io_channel_write( sock, msg->str, msg->len, &n );
	g_io_channel_close( sock );

	g_string_free(msg, 1);
	http_request_free(h);
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
    req  = http_request_new();
    g_io_add_watch( peer, G_IO_IN, (GIOFunc)handle_read, req );

    return TRUE;
}

int main (int argc, char **argv) {
    GMainLoop *loop;
    GIOChannel *sock;
    int sock_fd;

    loop = g_main_new(FALSE);
    sock_fd = bind_socket( "0.0.0.0", 5280, 5 );
    sock = g_io_channel_unix_new( sock_fd );
    g_io_add_watch( sock, G_IO_IN, (GIOFunc)handle_accept, NULL );
    g_warning("starting main loop");
    g_main_run( loop );
}
