# include <glib.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <errno.h>
# include <string.h>
# include <stdarg.h>
# include <stdlib.h>
# include <ctype.h>
# include "util.h"
# include "http.h"
# include "mime.h"

GIOChannel *http_bind_socket( const char *ip, int port, int queue ) { 
    struct sockaddr_in addr;
    int fd, r, n = 1;
    
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

    n = fcntl( fd, F_GETFL, 0 );
    if (n == -1)
	g_error("fcntl F_GETFL on %s: %m", ip );

    r = fcntl( fd, F_SETFL, n | O_NDELAY );
    if (n == -1)
	g_error("fcntl F_SETFL O_NDELAY on %s: %m", ip );
    /* 
     * n = fcntl( fd, F_GETFL, 0 );
     * g_warning("fd %d has O_NDELAY %s", fd, (n | O_NDELAY ? "set" : "unset"));
     */

    r = listen( fd, queue );
    if (r == -1)
	g_error("listen failed on %s: %m", ip);	

    return g_io_channel_unix_new( fd );
}

http_request *http_request_new ( GIOChannel *sock ) {
    http_request *h = g_new0(http_request, 1);
    int fd = g_io_channel_unix_get_fd( sock );
    struct sockaddr_in addr;
    int n = sizeof(struct sockaddr_in);
    int r;
    const gchar *r2;

    g_assert( sock != NULL );
    g_assert( h    != NULL );
    g_assert( fd   != -1 );

    h->sock   = sock;
    h->buffer = g_string_new("");

    r = getsockname( fd, (struct sockaddr *)&addr, &n );
    if (r == -1)
	g_error( "getsockname failed: %m" );
    r2 = inet_ntop( AF_INET, &addr.sin_addr, h->sock_ip, INET_ADDRSTRLEN );
    g_assert( r2 != NULL );

    r = getpeername( fd, (struct sockaddr *)&addr, &n );
    if (r == -1)
	g_error( "getpeername failed: %m" );
    r2 = inet_ntop( AF_INET, &addr.sin_addr, h->peer_ip, INET_ADDRSTRLEN );
    g_assert( r2 != NULL );

    g_io_channel_ref( sock );
    return h;
}

void http_request_free ( http_request *h ) {
    g_free( h->uri );
    g_free( h->method );
    g_hash_free( h->header );
    g_hash_free( h->query );
    g_hash_free( h->response );
    g_string_free( h->buffer, 1 );
    g_io_channel_unref( h->sock );
    g_free( h );
}

GHashTable *parse_query_string( gchar *query ) {
    GHashTable *data = g_hash_new();
    gchar **items, *key, *val;
    guint i;

    items = g_strsplit( query, "&", 0 );
    for ( i = 0; items[i] != NULL; i++ ) {
	key = items[i];
	if (key == NULL)
	    break;

	val = strchr( key, '=' );
	if (val != NULL)
	    *(val++) = '\0';
	else
	    val = "1";

	key = url_decode( key );	
	val = url_decode( val );	
	g_hash_set( data, key, val );
	g_free( key );
	g_free( val );
    }
    g_strfreev(items);

    return data;
}

GHashTable *http_parse_header (http_request *h, gchar *req) {
    GHashTable *head = g_hash_new();
    gchar **lines, **items, *key, *val, *p;
    guint i;

    lines = g_strsplit( req, "\r\n", 0 );
    items = g_strsplit( lines[0], " ", 3 );

    h->method = g_strdup( items[0] );
    h->uri    = g_strdup( items[1] );
    // g_message( "method: %s", h->method );
    // g_message( "uri: %s", h->uri );
    g_strfreev( items );

    for (i = 1; lines[i] != NULL && lines[i][0] != '\0'; i++ ) {
	key = lines[i];
	val = strchr(key, ':');
	if (val != NULL) {
	    /* Separate the key from the value */
	    *val = '\0';

	    /* Normalize key -- lowercase every after 1st char */
	    for (p = key + 1; *p != '\0'; p++)
		*p = tolower(*p);

	    /* Strip ": " plus leading and trailing space from val */
	    g_strchomp( val += 2 ); // ": "
	    
	    g_debug("Header in: %s=%s", key, val );
	    g_hash_set( head, key, val );
	}
    }

    g_strfreev( lines );
    h->header = head;
    return head;
}

GHashTable *http_parse_query (http_request *h, gchar *post) {
    gchar *q = NULL;

    g_assert( h != NULL );

    if (h->uri != NULL) {
	// g_message( "Parsing query from %s", h->uri );
	q = strchr( h->uri, '?' );
    }

    if (post != NULL) {
	h->query = parse_query_string( post );
    } else if (q != NULL) {
	h->query = parse_query_string( q + 1 );
    } else {
	h->query = NULL;
    }

    if (q != NULL)
	*q = '\0'; /* remove the query string from the URI */

    return h->query;
}

guint http_request_read (http_request *h) {
    gchar *buf = g_new( gchar, BUFSIZ + 1 );
    GIOError r;
    guint n, t;
    gchar *c_len_hdr;
    guint c_len;
    guint tot_req_size;
    gchar *hdr_end = NULL;

    // g_message("entering http_request_read");
    for (t = 0, n = BUFSIZ; h->buffer->len < MAX_REQUEST_SIZE &&
	    (hdr_end = strstr(h->buffer->str, "\r\n\r\n")) == NULL; t += n ) {
	// g_message("entering read loop");
	r = g_io_channel_read( h->sock, buf, BUFSIZ, &n );
	// g_message("read loop: read %d bytes of %d (%d)", n, BUFSIZ, r);
	if (r != G_IO_ERROR_NONE) {
	    g_warning( "read_http_request failure: %m" );
	    g_free(buf);
	    return 0;
	}
	buf[n] = '\0';
	g_string_append(h->buffer, buf);
    }
    http_parse_header( h, h->buffer->str );
    c_len_hdr = HEADER("Content-length");
    if (c_len_hdr == NULL) {
    	c_len = 0;
    } else {
    	c_len = atoi( c_len_hdr );
    }
    tot_req_size = hdr_end - h->buffer->str + 4 + c_len;
    for (; t < tot_req_size; t += n ) {
	// g_message("entering read loop");
	r = g_io_channel_read( h->sock, buf, BUFSIZ, &n );
	// g_message("read loop: read %d bytes of %d (%d)", n, BUFSIZ, r);
	if (r != G_IO_ERROR_NONE) {
	    g_warning( "read_http_request failure: %m" );
	    g_free(buf);
	    return 0;
	}
	buf[n] = '\0';
	g_string_append(h->buffer, buf);
    }
    g_free(buf);
    return t;
}

gboolean http_request_ok (http_request *h) {
    gchar *header_end = strstr( h->buffer->str, "\r\n\r\n" );
    gchar *c_len_hdr;
    guint c_len;

    if (header_end != NULL) {
	// g_warning( "inside http_request_ok: header_end found" );

	c_len_hdr = HEADER("Content-length");
	if (c_len_hdr == NULL) {
	    GString *z;
	    http_parse_query( h, NULL );
	    if (h->query) {
		z = g_hash_as_string( h->query );
		g_debug( "Query: %s", z->str );
		g_string_free(z, 1);
	    }
	    h->complete++;
	    return TRUE;
	}

	header_end += sizeof("\r\n\r\n") - 1; // *header_end == '\r'
	c_len = atoi( c_len_hdr );
	if (strlen( header_end ) >= c_len) {
	    http_parse_query( h, header_end );
	    h->complete++;
	    return TRUE;
	}
    }
    // g_warning( "inside http_request_ok: header_end not found" );
    return FALSE;
}

void http_add_header ( http_request *h, const gchar *key, gchar *val ) {
    if ( h->response == NULL )
	h->response = g_hash_new();
    g_hash_set( h->response, key, val );
}

void http_printf_header ( http_request *h, gchar *key, gchar *fmt, ... ) {
    gchar *val;
    va_list data;
    va_start( data, fmt );
    val = g_strdup_vprintf( fmt, data );
    http_add_header( h, key, val );
    va_end( data );
    g_free( val );
}

static void http_compose_header ( gchar *key, gchar *val, GString *buf ) {
    g_string_sprintfa( buf, "%s: %s\r\n", key, val );
}

GIOError http_send_header ( http_request *h, int status, const gchar *msg ) {
    GString *hdr = g_string_new("");
    GIOError r;
    int n;

    g_string_sprintfa( hdr, "HTTP/1.1 %d %s\r\n", status, msg );
    g_hash_table_foreach( h->response, (GHFunc) http_compose_header, hdr );
    g_string_append( hdr, "\r\n" );
    g_debug("Header out: %s", hdr->str);
    r = g_io_channel_write( h->sock, hdr->str, hdr->len, &n );
    g_string_free( hdr, 1 );
    return r;
}

void http_send_redirect( http_request *h, gchar *dest ) {
    http_add_header ( h, "Location", dest );
    http_send_header( h, 302, "Moved" );
}

gchar *http_fix_path (const gchar *uri, const gchar *docroot) {
    GString *path = g_string_new(docroot);
    gchar *dotdot;

    // Remove leading slashes.
    while (*uri != '\0' && *uri == '/') uri++;

    // Instantiate the string.
    g_string_sprintfa(path, "/%s", uri);

    // Find ..'s and remove them.
    while ((dotdot = strstr(path->str, "..")) != NULL)
	g_string_erase(path, dotdot - path->str, 2 );

    uri = path->str;
    g_string_free(path, 0); // don't free the char data, we're returning it
    return (gchar *)uri;
}

gchar *http_mime_type (const gchar *path) {
    guint i;
    gchar *ext;

    ext = strrchr( path, '.' );
    if ( ext++ != NULL )
	for (i = 0; mime_types[i].ext != NULL; i++) {
	    // g_warning( "http_mime_type: %s vs %s", ext, mime_types[i].ext );
	    if (strcmp(ext, mime_types[i].ext) == 0)
		return mime_types[i].type;
	}

    return "text/plain";
} 

int http_open_file (const gchar *path, int *status) {
    int fd;

    fd = open( path, O_RDONLY );
    if (fd == -1) {
	if (errno == ENOENT) {
	    g_warning("File not found: %s", path);
	    *status = 404;
	} else if (errno == EACCES) {
	    g_warning("Access not permitted: %s", path);
	    *status = 400;
	} else {
	    g_warning("Error accessing %s: %m", path);
	    *status = 500;
	}
	return -1;
    }
    *status = 200;
    return fd;
}

int http_serve_file ( http_request *h, const gchar *docroot ) {
    gchar *path;
    guint fd, status;

    path = http_fix_path( h->uri, docroot );
    fd   = http_open_file( path, &status );

    http_add_header(  h, "Content-Type", http_mime_type( path ) );
    http_send_header( h, status, fd == -1 ? "Not OK" : "OK" );

    if ( fd != -1 )
	http_sendfile( h, fd );

    close(fd);
    g_free(path);
    return ( fd != -1 );
}

int http_serve_template ( http_request *h, gchar *file, GHashTable *data ) {
    gchar *form;
    guint r, n;

    form = parse_template( file, data );
    n = strlen(form);

    http_add_header( h, "Content-Type", "text/html" );
    http_send_header( h, 200, "OK" );

    r = g_io_channel_write( h->sock, form, n, &n );

    g_free( form );

    if ( r != G_IO_ERROR_NONE ) {
	g_warning( "Serving template to %s failed: %m", h->peer_ip );
	return 0;
    }

    return 1;
}
