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
# include "conf.h"

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

http_request *http_request_new ( GIOChannel *GIO_sock ) {
    http_request *h = g_new0(http_request, 1);
    int fd = g_io_channel_unix_get_fd( GIO_sock );
    struct sockaddr_in addr;
    int n = sizeof(struct sockaddr_in);
    int r;
    const gchar *r2;

    g_assert( GIO_sock != NULL );
    g_assert( h    != NULL );
    g_assert( fd   != -1 );
    h->password_checked = FALSE;
    

    h->sock   = GIO_sock;
//    h->sockfd = fd;
    h->buffer = g_string_new("");

    r = getsockname( fd, (struct sockaddr *)&addr, &n );
    if (r == -1)
	{
		g_warning( "getpeername failed: %m" );
		g_error( "getsockname failed: %m" );
		return NULL;
	}
    r2 = inet_ntop( AF_INET, &addr.sin_addr, h->sock_ip, INET_ADDRSTRLEN );
    g_assert( r2 != NULL );

    return h;
}

void http_request_free ( http_request *h ) {
    if (h->uri)
	g_free( h->uri );
    if (h->method)
	g_free( h->method );
    if (h->header)
	g_hash_free( h->header );
    if (h->query)
	g_hash_free( h->query );
    if (h->response)
	g_hash_free( h->response );
    if (h->buffer)
	g_string_free( h->buffer, 1 );
//   if (h->sock) {
//	g_io_channel_shutdown( h->sock );
//	g_io_channel_close( h->sock ); 
//	g_io_channel_unref( h->sock ); 
//   }
    g_free( h );
}

GHashTable *parse_query_string( gchar *query ) {
    GHashTable *data = g_hash_new();
    gchar **items, *key, *val;
    guint i;

    if (!query)
	return data;

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
	/* Irving - fix from Yurgi - check to see if the key is already in the
	   hash table.  This deals with keys that are set twice by web sites */
	if(g_hash_table_lookup_extended( data, key, NULL, NULL ) == FALSE ) {
		g_hash_set( data, key, val );
		if(CONFd("Verbosity") >= 9) g_message("parse_query_string: Setting key %s=%s", key, val);
	}
	else
		if(CONFd("Verbosity") >= 9) g_message("parse_query_string: Duplicate key: %s", key);

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

    h->method = NULL;
    h->uri    = NULL;
    h->header = head;
    
    if (req == NULL)
	return head;
    
    lines = g_strsplit( req, "\r\n", 0 );

    if (lines == NULL || lines[0] == NULL)
	return head;

    items = g_strsplit( lines[0], " ", 3 );

    h->method = g_strdup( items[0] );
    h->uri    = g_strdup( items[1] );
    // if (CONFd("Verbosity") >= 8) g_message( "http_parse_header: method_len: %d, uri_len: %d", strlen(h->method), strlen(h->uri));
    if (CONFd("Verbosity") >= 8) g_message( "http_parse_header: method: %s", h->method );
    if (CONFd("Verbosity") >= 8) g_message( "http_parse_header: uri: %s", h->uri );
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

	    if ( strcmp(key, "Referer" )== 0) {
	        if (CONFd("Verbosity") >= 9) g_message("http_parse_header: Referer=%s, IGNORED", val );
	    } else {
	        if (CONFd("Verbosity") >= 9) g_message("http_parse_header: %s=%s", key, val );
	        g_hash_set( head, key, val );
	    }
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
	if(CONFd("Verbosity") >= 8) g_message( "http_parse_query: URI: %s", h->uri );
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

guint http_get_hdr_len( const gchar *str ) {
    gchar *hdr_end = NULL;
    // guint hdr_len = 0;
    
    if ((hdr_end = strstr( str, "\r\n\r\n"))) return hdr_end - str + 4;
    return 0;
}

guint http_request_read (http_request *h) {
    gchar *buf = g_new( gchar, BUFSIZ + 1 );
    gchar *dbuf = g_new( gchar, RAW_LINE_SIZE+1);
/*
    GIOStatus r;
    GError *err = NULL;
*/
    GIOError r;
    guint i, s, times, n = 0, t = 0;
    gchar *c_len_hdr = NULL;
    guint hdr_len = 0, c_len = 0, tot_req_size = 0;
    struct timeval tv;
    fd_set fdset;

    FD_ZERO(&fdset);
    FD_SET(g_io_channel_unix_get_fd(h->sock), &fdset);
    tv.tv_sec = 0;
    buf[0] = '\0';
    buf[BUFSIZ] = '\0';
    dbuf[0] = '\0';
    dbuf[RAW_LINE_SIZE] =  '\0';

    //    for (t = 0, n = BUF_SIZ; n == BUF_SIZ &&
    //      h->buffer->len < MAX_REQUEST_SIZE; t += n ) {
    // BPsmythe: The above (original) loop will never execute
    // more than once unless the size of the buffer read in (n)
    // is equal to the constant BUF_SIZE.  What is desired is
    // to keep looping until there is nothing left to read.
    // The for was changed to look for the end of the headers.
    // FIXME: We should use the newer g_io_channel_read_char
    //
    // TJaqua: Added buffer overflow checking, content read loop from 0.93pre2, and fixed up the timeouts, logging and error exits

    if (CONFd("Verbosity") >= 6) g_message("http_request_read: READING request from peer %s (on %s, fd: %d)", h->peer_ip, h->sock_ip, g_io_channel_unix_get_fd(h->sock));
    if (CONFd("Verbosity") >= 9) g_message("http_request_read: entering HEADER read loop (BUFSIZE=%u)", BUFSIZ);
    for (times=MAX_REQ_TIMEOUTS; !hdr_len && (times > 0); t += n ) {
	n=0;
/*
	r = g_io_channel_read_chars( h->sock, buf, BUFSIZ, &n, &err );
	if (r == G_IO_STATUS_ERROR || err != NULL) {
	    g_message( "http_request_read: Socket IO ERROR: %s, exiting!", err->message );
	    g_error_free(err);
*/
	r = g_io_channel_read( h->sock, buf, BUFSIZ, &n);
	if (r != G_IO_ERROR_NONE) {
	    g_warning( "http_request_read: Socket IO ERROR: %m, exiting!" );
	    g_free(buf);
	    return 0;
	}
	else if (!n) times--;

	if (CONFd("Verbosity") >= 9) g_message("http_request_read: HEADER loop read %u bytes", n);
	buf[n] = '\0';
	if (n && CONFd("Verbosity") >= 10) { 
	    dbuf[0] = '\0';
	    for (i=0; i < n; i += RAW_LINE_SIZE) { 
		g_strncpy(dbuf, &(buf[i]), RAW_LINE_SIZE); 
		g_message("RAW_HDR_BF: %s", dbuf); 
	    }
	}
	if (h->buffer->len == MAX_REQUEST_SIZE)
	    g_warning("http_request_read: header buffer full (%u bytes, %u bytes discarded!)", MAX_REQUEST_SIZE, n);
	else if (n <= (MAX_REQUEST_SIZE - h->buffer->len)) {
	    g_string_append(h->buffer, buf);
	}
	else {
	    g_warning("http_request_read: header buffer full (%u bytes, %u bytes discarded!)", MAX_REQUEST_SIZE, n - (MAX_REQUEST_SIZE - h->buffer->len));
	    buf[MAX_REQUEST_SIZE - h->buffer->len] = '\0';
	    g_string_append(h->buffer, buf);
	}

        // BPsmythe: Check for the end of the headers.
	if (((hdr_len = http_get_hdr_len( h->buffer->str )) != 0) && (CONFd("Verbosity") >= 7))
	        if (CONFd("Verbosity") >= 6) g_message("http_request_read: header END found, length: %u", hdr_len );
	else {
            tv.tv_usec = REQ_TIMEOUT;
	    s = select (g_io_channel_unix_get_fd(h->sock)+1, &fdset, NULL, NULL, &tv);
	    if(s<0) {
	        g_warning("http_request_read: ERROR in select, exiting!");
		g_free(buf);
		return 0;
	    }
	    if(!s) {
	        if (CONFd("Verbosity") >= 7)  g_message("http_request_read: HEADER select timeout, %d more tries", times);
	        if (times) times--;
	    }
	}
    }

    if (!times) {
	if(CONFd("Verbosity") >= 6) {
	    g_message("http_request_read: ERROR: Too many timeouts waiting for HEADER end!");
	    if (!t) g_message("http_request_read: Empty HTTP-request header.");
	    else {
	        g_message("http_request_read: Invalid HTTP-request header, %u bytes read.", t+n);
	        if (CONFd("Verbosity") >= 10) { 
	            dbuf[0] = '\0';
	            for (i=0; i < h->buffer->len; i += RAW_LINE_SIZE) { 
		        g_strncpy(dbuf, &(h->buffer->str[i]), RAW_LINE_SIZE); 
	        	g_message("RAW_HEADER: %s", dbuf); 
	            }
	        }
	    }
	    g_message("http_request_read: exiting.");
	}
        g_free(buf);
        return 0;
    }
    
    // if(CONFd("Verbosity") >= 10) g_message("RAW_HEADER: %s", h->buffer->str);
    if (CONFd("Verbosity") >= 10) { 
        dbuf[0] = '\0';
        for (i=0; i < h->buffer->len; i += RAW_LINE_SIZE) { 
            g_strncpy(dbuf, &(h->buffer->str[i]), RAW_LINE_SIZE); 
      	    g_message("RAW_HEADER: %s", dbuf); 
        }
    }

    // Read the content length from the header
    http_parse_header( h, h->buffer->str );
    c_len_hdr = HEADER("Content-length");
    if (c_len_hdr == NULL) {
    	c_len = 0;
    } else {
    	c_len = atoi( c_len_hdr );
    }

    if(c_len) {
        if (CONFd("Verbosity") >= 9) g_message("http_request_read: entering CONTENT read loop to read %u bytes.", c_len);
        tot_req_size = hdr_len + c_len;
        for (times=MAX_REQ_TIMEOUTS; (t < tot_req_size) && (times > 0); t += n ) {
	    if (CONFd("Verbosity") >= 9) g_message("http_request_read: %u bytes of %u total.", t, tot_req_size );
	    n=0;
/*
	    r = g_io_channel_read_chars( h->sock, buf, BUFSIZ, &n, &err );
	    if (r == G_IO_STATUS_ERROR || err != NULL) {
	        g_message( "http_request_read: Socket-IO ERROR: %s, exiting!", err->message );
	        g_error_free(err);
*/
	    r = g_io_channel_read( h->sock, buf, BUFSIZ, &n);
	    if (r != G_IO_ERROR_NONE) {
	        g_warning( "http_request_read: Socket-IO ERROR: %m, exiting!" );
	        g_free(buf);
	        return 0;
	    }
	    if (CONFd("Verbosity") >= 9) g_message("http_request_read: CONTENT loop read %d bytes", n );

	    buf[n] = '\0';
	    if (n && CONFd("Verbosity") >= 10) { 
	        dbuf[0] = '\0';
	        for (i=0; i < n; i += RAW_LINE_SIZE) { 
		    g_strncpy(dbuf, &(buf[i]), RAW_LINE_SIZE); 
		    g_message("RAW_CON_BUF: %s", dbuf); 
	        }
	    }
	    if (h->buffer->len == tot_req_size)
	        g_warning("http_request_read: content length exceeded (%u bytes discarded!)", n);
	    else if (n <= (tot_req_size - h->buffer->len)) {
	        g_string_append(h->buffer, buf);
	    }
	    else {
	        g_warning("http_request_read: content length exceeded (%u bytes, %u bytes discarded!)", tot_req_size, n - (tot_req_size - h->buffer->len));
	        buf[tot_req_size - h->buffer->len] = '\0';
	        g_string_append(h->buffer, buf);
	    }

            // TJaqua: Check for the end of the content.
	    if (((t+n) >= tot_req_size) && (CONFd("Verbosity") >= 6))  
	        g_message("http_request_read: CONTENT end reached, length: %u", tot_req_size);
	    else if (!n) {
                tv.tv_usec = REQ_TIMEOUT;
	        s = select (g_io_channel_unix_get_fd(h->sock)+1, &fdset, NULL, NULL, &tv);
	        if(s<0) {
	            g_warning("http_request_read: ERROR in select, exiting!");
		    g_free(buf);
		    return 0;
	        }
	        if(!s) {
	            if (CONFd("Verbosity") >= 7)  g_message("http_request_read: content unfinished, %d more tries", times);
	            times--;
	        }
	    }
        }
        if (!times) {
	    if(CONFd("Verbosity") >= 6) {
	        g_message("http_request_read: ERROR: Too many timeouts waiting for CONTENT end.");
	        if (CONFd("Verbosity") >= 10) { 
	            dbuf[0] = '\0';
	            for (i=hdr_len; i < h->buffer->len; i += RAW_LINE_SIZE) { 
		        g_strncpy(dbuf, &(h->buffer->str[i]), RAW_LINE_SIZE); 
		        g_message("RAW_CONTENT: %s", dbuf); 
	            }
	        }
	        g_message("http_request_read: exiting.");
	    }
            g_free(buf);
            return 0;
        }
        if (t<tot_req_size) 
	    g_message("http_request_read: CONTENT unfinished - should not happen!");
        else if (t > tot_req_size)
	    g_message("http_request_read: CONTENT overflow - should not happen!");

        // if (CONFd("Verbosity") >=10) g_message("RAW_CONTENT: %s", &(h->buffer->str[hdr_len]));
        if (CONFd("Verbosity") >= 10) { 
            dbuf[0] = '\0';
            for (i=hdr_len; i < h->buffer->len; i += RAW_LINE_SIZE) { 
                g_strncpy(dbuf, &(h->buffer->str[i]), RAW_LINE_SIZE); 
                g_message("RAW_CONTENT: %s", dbuf); 
            }
        }
    }

    // if (CONFd("Verbosity") >=10) g_message("RAW_HDR: %s", h->buffer->str);
    if (CONFd("Verbosity") >= 6) g_message("http_request_read: FINISHED read (%u bytes total), exiting.", t);
    g_free(buf);
    return t;
}

gboolean http_request_ok (http_request *h) {
    guint hdr_len = http_get_hdr_len( h->buffer->str );
    gchar *c_len_hdr;
    guint c_len;

    if (hdr_len != 0) {
	if(CONFd("Verbosity") >= 8) g_warning( "http_request_ok: hdr_len %d", hdr_len );

	c_len_hdr = HEADER("Content-length");
	if (c_len_hdr == NULL) {
	    if(CONFd("Verbosity") >= 8) g_warning( "http_request_ok: No Content-length header, parsing query from URI." );
	    GString *z;
	    http_parse_query( h, NULL );
	    if ((h->query) && (CONFd("Verbosity") >= 8)) {
		z = g_hash_as_string( h->query );
		g_debug( "http_request_ok: Query: %s", z->str );
		g_string_free(z, 1);
	    }
	    h->complete++;
	    if(CONFd("Verbosity") >= 8) g_warning( "http_request_ok: Query parsing finished, exiting." );
	    return TRUE;
	}

	c_len = atoi( c_len_hdr );
	if (hdr_len >= c_len) {
	    if(CONFd("Verbosity") >= 8) g_warning( "http_request_ok: Parsing query from HTTP-Content." );
	    http_parse_query( h, &(h->buffer->str[hdr_len]) );
	    h->complete++;
	    if(CONFd("Verbosity") >= 8) g_warning( "http_request_ok: Query parsing finished, exiting." );
	    return TRUE;
	}
    }
    if(CONFd("Verbosity") >= 6) g_warning( "http_request_ok: Request not HTTP: <CR><LF><CR><LF> (header_end) NOT found" );
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
