# include <glib.h>
# include <stdio.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
# include <string.h>
# include <time.h>
# include <unistd.h>
# include <errno.h>
# include "util.h"
# include "conf.h"
# include "config.h"
# include "http.h"
# ifndef BUF_SIZ
# define BUF_SIZ 16384
# endif

#ifndef G_THREADS_ENABLED
#error "error: g_threads not enabled..."
#endif

#ifdef G_THREADS_IMPL_NONE
#error "error: g_threads not implemented..."
#endif

extern char** environ;

// we simulate a client by sending http requests to
// our local splashd just as a webbrowser would do
// we can test 
// redirect mac ip url : calling url
// login mac ip        : login request
// logout mac ip       : logout request
// session             : calling url,login and logout
// session-leave       : calling url,login and leave silently


static void usage( gchar* executable) {
    fprintf( stderr, "Usage: %s ", executable );
    fprintf( stderr, "\n%s <gateway password> redirect <dev> <mac> <ip> <url>                          : simulates client calling url", executable );
    fprintf( stderr, "\n%s <gateway password> login <dev> <mac> <ip> [<name> <password>]               : simulates client login request", executable );
    fprintf( stderr, "\n%s <gateway password> logout <dev> <mac> <ip>                                  : simulates client logout request", executable );
    fprintf( stderr, "\n%s <gateway password> session <dev> <mac> <ip> <url> [<name> <password>]       : simulates client calling url,login and logout", executable );
    fprintf( stderr, "\n%s <gateway password> session-leave <dev> <mac> <ip> <url> [<name> <password>] : simulates client calling url,login and silent leave", executable );
    exit(1);
}




// --ARP related ---------------------------------
// we need to influence the arp table to simulate that a certain client exists
// or has left


static void exec_add_env ( gchar *key, gchar *val, GPtrArray *env ) {
    gchar *p = g_strdup_printf( "%s=%s", key, val );
    g_ptr_array_add( env, p );
}

static int exec_cmd( gchar *cmd, GHashTable *conf ) {
    GHashTable *data;
    GPtrArray *env;
    gchar **arg, **n;
    
    data = g_hash_dup( conf );
    
    cmd = parse_template( cmd, data );
    g_warning("TEST:execute command %s now",cmd);
    arg = g_strsplit( cmd, " ", 0 );

    // prime the environment with our existing environment
    env = g_ptr_array_new();
    for ( n = environ; *n != NULL; n++ )
	g_ptr_array_add( env, *n );

    // Then add everything from the conf file
    g_hash_table_foreach( data, (GHFunc) exec_add_env, env );

    /* We're not cleaning up memory references because 
     * hopefully the exec won't fail... */
    execve( arg[0], (char **)arg, (char **)env->pdata );
    g_error( "TEST:execve %s failed: %m", cmd ); // Shouldn't happen.
    return -1;
}

int exec_cmd_perform( gchar *cmd, GHashTable *conf ) {
    pid_t pid, r = 0;
    guint status = 0, retval = 0;

    pid = fork();
    if (pid == -1)
        g_error( "TEST:Can't fork: %m" );
    
    if (! pid)
        exec_cmd( cmd, conf );
    
    while (r != pid) {
        r = waitpid( pid, &status, 0 );
        if (r == -1 && errno != EINTR) {
            g_warning( "TEST:waitpid failed: %m" );
            break;
        }
    }
    
    if (WIFEXITED(status)) {
        retval = WEXITSTATUS(status);
	    if (retval)
            g_warning( "TEST:%s of %s returned %d", cmd, CONF("IP"), retval );
    } else if (WIFSIGNALED(status)) {
        retval = WTERMSIG(status);
        g_warning( "TEST:%s of %s died from signal %d", cmd, CONF("IP"), retval );
    }
    
    return retval;
}

// perfrom an ip neighbor add 192.168.1.105 lladdr 00:ff:de:63:47:46 dev br0 nud permanent
// as arp is not available
static void add_arp( gchar* mac, gchar* ip, gchar* dev ) {
    g_message("TEST:try ip neighbor add %s lladdr %s dev %s nud permanent",ip,mac,dev);
    g_hash_set( nocat_conf, "IP",    ip );
    g_hash_set( nocat_conf, "MAC",   mac );
    g_hash_set( nocat_conf, "DEV",   dev );
    exec_cmd_perform( "ip neighbor add $IP lladdr $MAC dev $DEV nud permanent", nocat_conf );
}

// arp -s 192.168.05
static void remove_arp( gchar* ip, gchar* dev ) {
    g_message("TEST:try ip neighbor del %s dev %s",ip,dev);
    g_hash_set( nocat_conf, "IP",    ip );
    g_hash_delete( nocat_conf, "MAC" );
    g_hash_set( nocat_conf, "DEV",    dev );
    exec_cmd_perform( "ip neighbor del $IP dev $DEV", nocat_conf );
}

// --HTTP related ---------------------------------
// we simulate an http client calling the gateway
// by sending http requests


gboolean handle_response( GIOChannel *sock ) {
    gchar *buf = g_new( gchar, BUF_SIZ + 1 );
    GIOError r;
    guint n, t;
    
    g_message( "TEST:handle response" );
    for (t = 0, n = 0; n > 0; t += n ) {
        r = g_io_channel_read( sock, buf, BUF_SIZ, &n );
	    if (r != G_IO_ERROR_NONE) {
            g_warning( "TEST read http_result failure: %m" );
            g_free(buf);
            return 0;
        }
        buf[n] = '\0';
        // print out result
	    g_message(buf);
    }
    g_free(buf);
    g_message("TEST:response %d bytes",t);
    return TRUE;
}



// GET /index.html HTTP/1.1\n\n
static GIOError send_http_request( GIOChannel *sock, gchar* password, gchar* ip, gchar* hostname, gchar* query ) {
    gchar* send_query = NULL;
    GString *hdr = g_string_new("");
    GIOError r;
    int n;
    send_query = url_encode( send_query );
    g_message("TEST: try request hostname:\"%s\" query:\"%s\"",hostname,send_query);
    g_string_sprintfa( hdr, "GET %s HTTP/1.1\r\n", send_query );
    if ( hostname ) {
        g_string_sprintfa( hdr, "Host: %s\r\n", hostname );
    }
    if ( password ) {
        g_string_sprintfa( hdr, "NOCAT_GATEWAY_PASSWORD: %s\r\n", password );
    }
    if ( ip ) {
        g_string_sprintfa( hdr, "NOCAT_REMOTE_ADDR: %s\r\n", ip );
    }
    g_string_append( hdr, "Connection: close\r\n");
    g_string_append( hdr, "\r\n" );
    g_message("send request: %s", hdr->str);
    r = g_io_channel_write( sock, hdr->str, hdr->len, &n );
    g_free( send_query );
    g_string_free( hdr, 1 );
    return r;
}


GIOChannel *connect_socket( const char *ip, int port, int queue ) { 
    struct sockaddr_in addr;
    int fd, r, n = 1;
    GIOChannel *sock;
    
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    r = inet_aton( ip, &addr.sin_addr );
    if (r == 0)
	g_error("inet_aton failed on %s: %m", ip);	

    // PF_NET
    fd = socket( AF_INET, SOCK_STREAM, 0 );
    if (r == -1) g_error("socket failed: %m");	
    
    r = connect( fd, (struct sockaddr *)&addr, sizeof(addr) );
    if (r == -1) g_error("connect failed on %s: %m", ip);	
    
    r = setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n) );
    if (r == -1) g_error("setsockopt failed on %s: %m", ip);	
    
    sock = g_io_channel_unix_new( fd );
    g_message("created sock for %s", ip);
    return sock;
}

static void perform_http_request( gchar* password, gchar* ip, gchar* url ) {
    GIOChannel *sock;
    gchar *parse_url,*slash1,*slash2,*colon,*hostname,*server,*port,*query,*err = NULL;
    int portnr = -1;
    g_message("TEST:try http request:%s",url);
    // parse url in server, port and query
    parse_url = url;
    slash2 = NULL;
    slash1 = strchr( parse_url, '/' );
    if ( slash1 != NULL ) slash2 = strchr( slash1+1, '/' );
    if ( slash1 != NULL && slash2 != NULL && slash1[0]==slash2[0] ) {
        // //www.xyz.com or //www.xyz.com/ or //www.xyz.com/abc
        //ignore the stuff before the first // and the // themselves
        parse_url = slash2+1;
        slash2 = NULL;
        slash1 = strchr( parse_url, '/' );
        if ( slash1 != NULL ) slash2 = strchr( slash1+1, '/' );
    }
    if ( slash1 == NULL && slash2 == NULL ) {
        // www.xyz.com
        hostname = g_strdup(url);
        query = g_strdup("/");
    } else if (  slash1 != NULL ) {
        // www.xyz.com/ or www.xyz.com/abc/ or /index.html
        if ( slash1 == parse_url ) {
            // /index.html use the gateway...
            hostname = g_strdup_printf( "%s:%s", CONF("GatewayAddr"), CONF("GatewayPort") );
            query = g_strdup(parse_url);
        } else {
            // for www.xyz.com:45/index.html 
            // hostname="www.xyz.com:45" and "query=/index.html"
            *slash1='\0'; hostname = g_strdup(parse_url);*slash1='/';
            query = g_strdup(slash1);
        }
    } else {
        g_warning("could not perfrom http request, mal formed url=%s", url );
        return;
    }
    // split server an port
    colon = strchr( hostname, ':' );
    if ( !colon ) {
        portnr = 80;
        port   = g_strdup("80");
        server = g_strdup(hostname);
    } else {
        *colon='\0'; server = g_strdup(colon);*colon=':';
        port = g_strdup(colon+1);
        portnr = strtol( port, &err, 10 );
        if ( err != NULL && *err != '\0' ) { g_message("TEST:wrong port nr format"); return; }
    }
    g_message("TEST:hostname=%s,server=%s,port=%s,query=%s",hostname,server,port,query);
    g_message("TEST:initialize the socket ");
    sock = connect_socket( server, portnr, CONFd("ListenQueue") );
    g_message("TEST:add_watch to handle_response");
    send_http_request( sock, password, ip, hostname, query );
    handle_response( sock );
    g_io_channel_close( sock );
    g_free(hostname);
    g_free(query);
    g_free(server);
    g_free(port);
    return;
}

static void redirect( gchar* password, gchar* ip, gchar* url ) {
    g_message("TEST:try redirect");
    perform_http_request(password,ip,url);
}

static void login( gchar* password, gchar* ip, gchar* user_name, gchar* user_password ) {
    gchar* url = NULL;
    g_message("TEST:try login");
    url = g_strdup_printf( "/login?login=x&name=%s&password=%s", user_name, user_password );
    perform_http_request(password,ip,url);
    g_free(url);
}

static void logout( gchar* password, gchar* ip ) {
    g_message("TEST:try logout");
    perform_http_request(password,ip,"/logout?logout=x");
}

int main (int argc, char **argv) {
    gchar* password,*conf_password, *command, *ip, *mac, *dev, *url, *user_name, *user_password;
    password = NULL;
    command = NULL;
    ip = NULL;
    mac = NULL;
    dev = NULL;
    url = NULL;
    user_name = "guest";
    user_password="guest";
    
    
    read_conf_file( NC_CONF_PATH "/nocat.conf" );
    conf_password = g_strdup(CONF("GatewayPassword"));
    
    if (argc > 2 ) {
        password = argv[1];
        if ( conf_password != NULL && strcmp(password,conf_password) != 0 ) {
            g_error("Error: Wrong password");
            exit(1);
        }
        command = argv[2];
        if ( strcmp( "redirect", command ) == 0 ) {
            if ( argc < 7 ) usage(argv[0]);
            dev = argv[3]; mac = argv[4]; ip = argv[5]; url = argv[6];
            add_arp(mac,ip,dev);
            redirect(password,ip,url);
        } else if ( strcmp( "login", command ) == 0 ) {
            if ( argc < 6 ) usage(argv[0]);
            dev = argv[3]; mac = argv[4]; ip = argv[5];
            if ( argc > 6 ) { user_name = argv[6];
                if ( argc > 7 ) { user_password = argv[7]; }
            }
            add_arp(mac,ip,dev);
            login(password,ip,user_name,user_password);
        } else if ( strcmp( "logout", command ) == 0 ) {
            if ( argc < 6 ) usage(argv[0]);
            dev = argv[3]; mac = argv[4]; ip = argv[5];
            add_arp(mac,ip,dev);
            logout(password,ip);
            remove_arp(ip,dev);
        } else if ( strcmp( "session", command ) == 0 ) {
            if ( argc < 7 ) usage(argv[0]);
            dev = argv[3]; mac = argv[4]; ip = argv[5]; url = argv[6];
            if ( argc > 7 ) { user_name = argv[7];
                if ( argc > 8 ) { user_password = argv[8]; }
            }
            add_arp(mac,ip,dev);
            redirect(password,ip,url);
            login(password,ip,user_name,user_password);
            logout(password,ip);
            remove_arp(ip,dev);
        } else if ( strcmp( "session-leave", command ) == 0 ) {
            if ( argc < 7 ) usage(argv[0]);
            dev = argv[3]; mac = argv[4]; ip = argv[5]; url = argv[6];
            if ( argc > 7 ) { user_name = argv[7];
                if ( argc > 8 ) { user_password = argv[8]; }
            }
            add_arp(mac,ip,dev);
            redirect(password,ip,url);
            login(password,ip,user_name,user_password);
            remove_arp(ip,dev);
        } else usage(argv[0]);
    } else usage(argv[0]);
    
    return 0;
}

