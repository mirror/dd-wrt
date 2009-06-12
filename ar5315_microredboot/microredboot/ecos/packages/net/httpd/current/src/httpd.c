/* =================================================================
 *
 *      httpd.c
 *
 *      A simple embedded HTTP server
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####
 * -------------------------------------------
 * This file is part of eCos, the Embedded Configurable Operating
 * System.
 * Copyright (C) 2002 Nick Garnett.
 * Copyright (C) 2003 Andrew Lunn.
 * 
 * eCos is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 or (at your option)
 * any later version.
 * 
 * eCos is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with eCos; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * As a special exception, if other files instantiate templates or
 * use macros or inline functions from this file, or you compile this
 * file and link it with other works to produce a work based on this
 * file, this file does not by itself cause the resulting work to be
 * covered by the GNU General Public License. However the source code
 * for this file must still be made available in accordance with
 * section (3) of the GNU General Public License.
 * 
 * This exception does not invalidate any other reasons why a work
 * based on this file might be covered by the GNU General Public
 * License.
 *
 * -------------------------------------------
 * ####ECOSGPLCOPYRIGHTEND####
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    nickg@calivar.com
 *  Contributors: nickg@calivar.com, Andrew.lunn@ascom.ch
 *  Date:         2002-10-14
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */

#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <pkgconf/httpd.h>

#include <cyg/infra/cyg_trac.h>        /* tracing macros */
#include <cyg/infra/cyg_ass.h>         /* assertion macros */

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <network.h>
#include <arpa/inet.h>

#include <cyg/httpd/httpd.h>

/* ================================================================= */

#if 0
#define HTTPD_DIAG diag_printf
#else
#define HTTPD_DIAG(...)
#endif

/* ================================================================= */
/* Server socket address and file descriptor.
 */

static struct sockaddr_in server_address;

static int server_socket = -1;
#ifdef CYGPKG_NET_INET6
static int server_socket6 = -1;
static struct sockaddr_in6 server_address6;
#endif

/* ================================================================= */
/* Thread stacks, etc.
 */

static cyg_uint8 httpd_stacks[CYGNUM_HTTPD_THREAD_COUNT]
                             [CYGNUM_HAL_STACK_SIZE_MINIMUM+
                              CYGNUM_HTTPD_SERVER_BUFFER_SIZE+
                              CYGNUM_HTTPD_THREAD_STACK_SIZE];

static cyg_handle_t httpd_thread[CYGNUM_HTTPD_THREAD_COUNT];

static cyg_thread httpd_thread_object[CYGNUM_HTTPD_THREAD_COUNT];

/* ================================================================= */
/* Filename lookup table
 */

CYG_HAL_TABLE_BEGIN( cyg_httpd_table, httpd_table );
CYG_HAL_TABLE_END( cyg_httpd_table_end, httpd_table );

__externC cyg_httpd_table_entry cyg_httpd_table[];
__externC cyg_httpd_table_entry cyg_httpd_table_end[];

/* ================================================================= */
/* Page not found message
 */

static char cyg_httpd_not_found[] =
"<head><title>Page Not found</title></head>\n"
"<body><h2>The requested URL was not found on this server.</h2></body>\n";

/* ================================================================= */
/* Simple pattern matcher for filenames
 *
 * This performs a simple pattern match between the given name and the
 * pattern. At present the only matching supported is either exact, or
 * if the pattern ends in * then that matches all remaining
 * characters. At some point we might want to implement a more
 * complete regular expression parser here.
 */

static cyg_bool match( char *name, char *pattern )
{
    while( *name != 0 && *pattern != 0 && *name == *pattern )
        name++, pattern++;

    if( *name == 0 && *pattern == 0 )
        return true;

    if( *pattern == '*' )
        return true;

    return false;
}


/* ================================================================= */
/* Main processing function                                          */
/*                                                                   */
/* Reads the HTTP header, look it up in the table and calls the      */
/* handler.                                                          */

static void cyg_httpd_process( int client_socket, struct sockaddr *client_address )
{
    int calen = sizeof(*client_address);
    int nlc = 0;
    char request[CYGNUM_HTTPD_SERVER_BUFFER_SIZE];
    FILE *client;
    cyg_httpd_table_entry *entry = cyg_httpd_table;
    char *filename;
    char *formdata = NULL;
    char *p;
    cyg_bool success = false;
    char name[64];
    char port[10];

    getnameinfo(client_address, calen, name, sizeof(name), 
                port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);
    HTTPD_DIAG("Connection from %s[%s]\n",name,port);
  
    /* Convert the file descriptor to a C library FILE object so
     * we can use fprintf() and friends on it.
     */
    client = fdopen( client_socket, "r+");
  
    /* We are really only interested in the first line.
     */
    fgets( request, sizeof(request), client );
  
    HTTPD_DIAG("Request >%s<\n", request );
  
    /* Absorb the rest of the header. We nibble it away a
     * character at a time like this to avoid having to define
     * another buffer to read lines into. If we ever need to take
     * more interest in the header fields, we will need to be a
     * lot more sophisticated than this.
     */
    do{
        int c = getc( client );
        HTTPD_DIAG("%c",c);
        if( c == '\n' )
            nlc++;
        else if( c != '\r' )
            nlc = 0;
    } while(nlc < 2);
  
    /* Extract the filename and any form data being returned.
     * We know that the "GET " request takes 4 bytes.
     * TODO: handle POST type requests as well as GET's.
     */
  
    filename = p = request+4;
  
    /* Now scan the filename until we hit a space or a '?'. If we
     * end on a '?' then the rest is a form request. Put NULs at
     * the end of each string.
     */
    while( *p != ' ' && *p != '?' )
        p++;
    if( *p == '?' )
        formdata = p+1;
    *p = 0;
  
    if( formdata != NULL )
    {
        while( *p != ' ' )
            p++;
        *p = 0;
    }
  
    HTTPD_DIAG("Request filename >%s< formdata >%s<\n",filename,formdata?formdata:"-NULL-");
  
    HTTPD_DIAG("table: %08x...%08x\n",cyg_httpd_table, cyg_httpd_table_end);
  
    /* Now scan the table for a matching entry. If we find one
     * call the handler routine. If that returns true then we
     * terminate the scan, otherwise we keep looking.
     */
    while( entry != cyg_httpd_table_end )
    {
        HTTPD_DIAG("try %08x: %s\n", entry, entry->pattern);
      
        if( match( filename, entry->pattern ) )
        {
            HTTPD_DIAG("calling %08x: %s\n", entry, entry->pattern);
            if( (success = entry->handler( client, filename, formdata, entry->arg )) )
                break;
        }
      
        entry++;
    }
  
    /* If we failed to find a match in the table, send a "not
     * found" response.
     * TODO: add an optional fallback to go look for files in
     * some filesystem, somewhere.
     */
    if( !success )
    {
        HTTPD_DIAG("Not found %s\n",filename);
        cyg_httpd_send_html( client, NULL, NULL, cyg_httpd_not_found );
    }

    fclose(client);
}

/* ================================================================= */
/* Main HTTP server
 *
 * This just loops, collects client connections, and calls the main
 * process function on the connects*/

static void cyg_httpd_server( cyg_addrword_t arg )
{
    do
    {
        int client_socket;
        struct sockaddr client_address;
        int calen = sizeof(client_address);
        fd_set readfds;
        int n;

        /* Wait for a connection.
         */
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
#ifdef CYGPKG_NET_INET6
        FD_SET(server_socket6, &readfds);
        n = (server_socket > server_socket6 ? server_socket : server_socket6) + 1;
#else
        n = server_socket + 1;
#endif
        select(n,&readfds,NULL,NULL,NULL);
        if (FD_ISSET(server_socket, &readfds)) {
          client_socket = accept( server_socket, &client_address, &calen );
          cyg_httpd_process(client_socket, &client_address);
        }
#ifdef CYGPKG_NET_INET6
        if (FD_ISSET(server_socket6, &readfds)) {
          client_socket = accept( server_socket6, &client_address, &calen );
          cyg_httpd_process(client_socket, &client_address);
        }
#endif            
    } while(1);
}

/* ================================================================= */
/* Initialization thread
 *
 * Optionally delay for a time before getting the network
 * running. Then create and bind the server socket and put it into
 * listen mode. Spawn any further server threads, then enter server
 * mode.
 */

static void cyg_httpd_init(cyg_addrword_t arg)
{
    int i;
    int err = 0;

    /* Delay for a configurable length of time to give the application
     * a chance to get going, or even complete, without interference
     * from the HTTPD.
     */
    if( CYGNUM_HTTPD_SERVER_DELAY > 0 )
    {
        cyg_thread_delay( CYGNUM_HTTPD_SERVER_DELAY );
    }
    
    server_address.sin_family = AF_INET;
    server_address.sin_len = sizeof(server_address);
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(CYGNUM_HTTPD_SERVER_PORT);
#ifdef CYGPKG_NET_INET6
    server_address6.sin6_family = AF_INET6;
    server_address6.sin6_len = sizeof(server_address6);
    server_address6.sin6_addr = in6addr_any;
    server_address6.sin6_port = htons(CYGNUM_HTTPD_SERVER_PORT);
#endif 
    /* Get the network going. This is benign if the application has
     * already done this.
     */
    init_all_network_interfaces();

    /* Create and bind the server socket.
     */
    server_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    CYG_ASSERT( server_socket > 0, "Socket create failed");

    err = bind( server_socket, (struct sockaddr *)&server_address,
                sizeof(server_address) );
    CYG_ASSERT( err == 0, "bind() returned error");

    err = listen( server_socket, SOMAXCONN );
    CYG_ASSERT( err == 0, "listen() returned error" );
#ifdef CYGPKG_NET_INET6
    server_socket6 = socket( AF_INET6, SOCK_STREAM, IPPROTO_TCP );
    CYG_ASSERT( server_socket6 > 0, "Socket AF_INET6 create failed");

    err = bind( server_socket6, (struct sockaddr *)&server_address6,
                sizeof(server_address6) );
    CYG_ASSERT( err == 0, "bind(AF_INET6) returned error");

    err = listen( server_socket6, SOMAXCONN );
    CYG_ASSERT( err == 0, "listen(AF_INET6) returned error" );
#endif
    /* If we are configured to have more than one server thread,
     * create them now.
     */
    for( i = 1; i < CYGNUM_HTTPD_THREAD_COUNT; i++ )
    {
        cyg_thread_create( CYGNUM_HTTPD_THREAD_PRIORITY,
                           cyg_httpd_server,
                           0,
                           "HTTPD",
                           &httpd_stacks[i][0],
                           sizeof(httpd_stacks[i]),
                           &httpd_thread[i],
                           &httpd_thread_object[i]
            );
    
        cyg_thread_resume( httpd_thread[i] );
    }

    /* Now go be a server ourself.
     */
    cyg_httpd_server(arg);
}

/* ================================================================= */
/* System initializer
 *
 * This is called from the static constructor in init.cxx. It spawns
 * the main server thread and makes it ready to run. It can also be
 * called explicitly by the application if the auto start option is
 * disabled.
 */

__externC void cyg_httpd_startup(void)
{
    cyg_thread_create( CYGNUM_HTTPD_THREAD_PRIORITY,
                       cyg_httpd_init,
                       0,
                       "HTTPD",
                       &httpd_stacks[0][0],
                       sizeof(httpd_stacks[0]),
                       &httpd_thread[0],
                       &httpd_thread_object[0]
        );
    
    cyg_thread_resume( httpd_thread[0] );
}

/* ================================================================= */
/*  HTTP protocol handling
 *
 * cyg_http_start() generates an HTTP header with the given content
 * type and, if non-zero, length.
 * cyg_http_finish() just adds a couple of newlines for luck and
 * flushes the stream.
 */

__externC void cyg_http_start( FILE *client, char *content_type,
                               int content_length )
{
    fputs( "HTTP/1.1 200 OK\n"
           "Server: " CYGDAT_HTTPD_SERVER_ID "\n",
           client );

    if( content_type != NULL )
        fprintf( client,"Content-type: %s\n", content_type );

    if( content_length != 0 )
        fprintf( client, "Content-length: %d\n", content_length );

    fputs( "Connection: close\n"
           "\n",
           client );    
}

__externC void cyg_http_finish( FILE *client )
{
    fputs( "\n\n", client );
    fflush( client );
}
    

/* ================================================================= */
/* HTML tag generation
 *
 * These functions generate standard HTML begin and end tags. By using
 * these rather than direct printf()s we help to reduce the number of
 * distinct strings present in the executable.
 */

__externC void cyg_html_tag_begin( FILE *client, char *tag, char *attr )
{
    char *pad = "";

    if( attr == NULL )
        attr = pad;
    else if( attr[0] != 0 )
        pad = " ";

    fprintf(client, "<%s%s%s>\n",tag,pad,attr);
}

__externC void cyg_html_tag_end( FILE *client, char *tag )
{
    fprintf( client, "<%s%s%s>\n","/",tag,"");
}

/* ================================================================= */
/* Parse form request data
 *
 * Given a form response string, we parse it into an argv/environment
 * style array of "name=value" strings. We also convert any '+'
 * separators back into spaces.
 *
 * TODO: also translate any %xx escape sequences back into real
 * characters.
 */

__externC void cyg_formdata_parse( char *data, char *list[], int size )
{
    char *p = data;
    int i = 0;

    list[i] = p;
    
    while( p && *p != 0 && i < size-1 )
    {
        if( *p == '&' )
        {
            *p++ = 0;
            list[++i] = p;
            continue;
        }
        if( *p == '+' )
            *p = ' ';
        p++;
    }

    list[++i] = 0;
}

/* ----------------------------------------------------------------- */
/* Search for a form response value
 *
 * Search a form response list generated by cyg_formdata_parse() for
 * the named element. If it is found a pointer to the value part is
 * returned. If it is not found a NULL pointer is returned.
 */

__externC char *cyg_formlist_find( char *list[], char *name )
{
    while( *list != 0 )
    {
        char *p = *list;
        char *q = name;

        while( *p == *q )
            p++, q++;

        if( *q == 0 && *p == '=' )
            return p+1;
        
        list++;
    }

    return 0;
}

/* ================================================================= */
/* Predefined page handlers
 */

/* ----------------------------------------------------------------- */
/* Send an HTML page from a single string
 *
 * This just sends the string passed as the argument with an HTTP
 * header that describes it as HTML. This is useful for sending
 * straightforward static web content.
 */

__externC cyg_bool cyg_httpd_send_html( FILE *client, char *filename,
                                        char *request, void *arg )
{
    html_begin( client );
    
    fwrite( arg, 1, strlen((char *)arg), client );

    html_end( client );

    return true;
}

/* ----------------------------------------------------------------- */
/* Send arbitrary data
 *
 * This takes a pointer to a cyg_httpd_data structure as the argument
 * and sends the data therein after a header that uses the content
 * type and size from the structure. This is useful for non-HTML data
 * such a images.
 */

__externC cyg_bool cyg_httpd_send_data( FILE *client, char *filename,
                                        char *request, void *arg )
{
    cyg_httpd_data *data = (cyg_httpd_data *)arg;

    cyg_http_start( client, data->content_type, data->content_length );
    
    fwrite( data->data, 1, data->content_length, client );

    return true;
}

/* ----------------------------------------------------------------- */
/* end of httpd.c                                                    */
