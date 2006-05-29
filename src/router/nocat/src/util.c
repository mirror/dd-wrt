# include <glib.h>
# include <ctype.h>
# include <string.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <crypt.h>
# include <sys/stat.h>
# include <sys/socket.h>
# include <sys/mman.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include "util.h"
# include "config.h"
# include <syslog.h>
# include <stdarg.h>

/********** Hash stuff **********/

gchar *g_strncpy( gchar *dest, const gchar *src, guint n ) {
    strncpy( dest, src, n );
    dest[ n - 1 ] = '\0';
    return dest;
}

GHashTable *g_hash_new (void) {
    return g_hash_table_new( g_str_hash, g_str_equal );
}

static gboolean g_hash_free_each ( gpointer key, gpointer val, gpointer data ) {
    // g_warning("freeing %s (= %s)", key, val);
    if (key != NULL) g_free(key);
    if (val != NULL) g_free(val);
    return TRUE;
}

guint g_hash_free (GHashTable *h) {
    guint n;
    if (h == NULL) return 0;
    n = g_hash_table_foreach_remove( h, g_hash_free_each, NULL );
    g_hash_table_destroy( h );
    return n;
}

gboolean g_hash_delete(GHashTable * h, const gchar *key) {
    gpointer k, v;
    gchar *kk, *vv;

    g_assert( h   != NULL );
    g_assert( key != NULL );

    // if (g_hash_table_lookup_extended(h, key, (gpointer *)&k, (gpointer *)&v)) {
    if (g_hash_table_lookup_extended(h, key, &k, &v)) {
	g_hash_table_remove(h, key);
        kk = ((gchar *) k);
        vv = ((gchar *) v);
	if (kk != NULL) g_free(kk);
	if (vv != NULL) g_free(vv);
	return TRUE;
    }
    return FALSE;
}

gboolean g_hash_set(GHashTable *h, const gchar *key, gchar *val) {
    gchar *k, *v;
    gboolean over;

    g_assert( h   != NULL );
    g_assert( key != NULL );
    g_assert( val != NULL );

    over = g_hash_delete(h, key);
    k = g_strdup(key);
    v = g_strdup(val);
    g_hash_table_insert(h, k, v);
    return over;
}

static void g_hash_dup_each ( gchar *k, gchar *v, GHashTable *dest ) {
    g_hash_set( dest, k, v );
}

GHashTable *g_hash_merge( GHashTable *dest, GHashTable *src ) {
    g_hash_table_foreach( src, (GHFunc) g_hash_dup_each, dest );
    return dest;
}

GHashTable *g_hash_dup( GHashTable *src ) {
    GHashTable *dest = g_hash_new();
    return g_hash_merge( dest, src );
}

static void g_hash_as_string_each( gchar *k, gchar *v, GString *dest ) {
    g_string_sprintfa( dest, "%s=%s\n", k, v );
}

GString *g_hash_as_string( GHashTable *h ) {
    GString *dest = g_string_new("");
    g_assert( h != NULL );
    g_hash_table_foreach( h, (GHFunc) g_hash_as_string_each, dest );
    return dest;
}

/********** URL encoding **********/

static int fromhex ( char digit ) {
    char d = toupper(digit);
    if (!isxdigit(digit)) return 0;
    if (isdigit(d)) {
	return d - '0';
    } else {
	return (int) ( d - 'A' ) + 10;
    }
}

gchar *url_decode( const gchar *src ) {
    gchar *dest, *dest0;
    int n;

    n = strlen(src) + 1;
    dest = dest0 = g_new0(gchar, n);

    for (; *src != '\0' && n >= 0; ++dest, ++src, --n )
	if ( *src == '%' && n > 2 )  {
	    *dest  = fromhex( *(++src) ) * 0x10;
	    *dest += fromhex( *(++src) );
	    n -= 2;
	} else if ( *src == '+' ) 
	    *dest = ' ';
	else
	    *dest = *src;

    *dest = '\0';
    return dest0; // g_renew( gchar, dest0, ++n );
}

gchar *url_encode( const gchar *src ) {
    char *dest, *dest0;
    int n = strlen(src) + 1;
   
    dest = dest0 = g_new0(gchar, n * 3);

    for (; *src != '\0' && n >= 0; src++, dest++, n--) {
        // g_message( "src: %s dest: %s n: %d", src, dest0, n );
        
        // added some chars in parameters to convert to allow urls in parameters
        if ( strchr(":/?&=%+", *src) ) {
            sprintf( dest, "%%%02X", (int) *src & 0xFF );
            dest += 2;
        }
        else if ( isalnum(*src) || strchr("./-_", *src) )
            *dest = *src;
        else if ( *src == ' ' )
            *dest = '+';
        else {
            sprintf( dest, "%%%02X", (int) *src & 0xFF );
            dest += 2;
        }
    }

    *dest = '\0'; 
    return dest0; // g_renew( gchar, dest0, ++n );
}

static void build_url_each( gchar *k, gchar *v, GString *dest ) {
    gchar *val = url_encode( v );
    g_string_sprintfa( dest, "%s=%s&", k, val );
    g_free( val );
}

GString *build_url( const gchar *uri, GHashTable *query ) {
    GString *dest = g_string_new( uri );
    g_assert( query != NULL );

    g_string_append( dest, "?" );
    g_hash_table_foreach( query, (GHFunc) build_url_each, dest );
    g_string_erase( dest, dest->len - 1, 1 );
    
    return dest;
}


/********** I/O stuff **********/

gchar *load_file( const char *path ) {
    gchar *file;
    struct stat s;
    void *data;
    int fd, r;

    g_assert( path != NULL );

    fd = open( path, O_RDONLY );
    if ( fd == -1 ) {
	g_error( "Can't open %s: %m", path );
	return NULL;
    }

    r = fstat( fd, &s );
    g_assert( r == 0 );

    data = mmap( NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    g_assert( data != MAP_FAILED );

    file = g_strndup( data, s.st_size );
    g_assert( file != NULL );

    r = munmap( data, s.st_size );
    g_assert( r == 0 );

    r = close(fd);
    g_assert( r == 0 );

    return file;
}


gchar *parse_template( gchar *src, GHashTable *data ) {
    GString *dest = g_string_sized_new(strlen(src));
    guint n;
    gchar *var, *val;

    for (; *src != '\0'; src++) {
	// Find the text chunk up to the next $, 
	// and append it to the buffer.
	n = strcspn( src, "$" );
	if (n) {
	    g_string_sprintfa( dest, "%.*s", n, src );
	    src += n;
	    if ( *src == '\0' )
		break;
	}

	// If the immediately following char is alphabetical...
	if (isalpha(*( src + 1 ))) {
	    // Find the identifier following the $
	    for (n = 2; isident(src[n]); n++);

	    // Having found it, copy the variable name out
	    // and get the corresponding value.
	    var = g_strndup( src + 1, --n );
	    val = g_hash_table_lookup( data, var );
	    if (val)
		g_string_append(dest, val);
	    g_free(var);

	    src += n;
	} else {
	    // Otherwise save the $
	    g_string_append(dest, "$");
	}
    }
    
    val = g_renew( gchar, dest->str, strlen(dest->str) + 1 );
    g_string_free( dest, 0 );
    return val;
}
/******  logging *******/


/* log handler for sending messages to syslog */
void 
log_handler (const gchar *log_domain,
             GLogLevelFlags log_level,
             const gchar *message,
             gpointer user_data)
{
	int syslog_priority;
    
	// prevent debug
        // if (log_level & G_LOG_LEVEL_DEBUG)
	//	 return;
    
	/* syslog uses reversed meaning of LEVEL_ERROR and LEVEL_CRITICAL */
	if (log_level & G_LOG_LEVEL_ERROR)
		syslog_priority = LOG_CRIT;
	else if (log_level & G_LOG_LEVEL_CRITICAL)
		syslog_priority = LOG_ERR;
	else if (log_level & G_LOG_LEVEL_WARNING)
		syslog_priority = LOG_WARNING;
	else if (log_level & G_LOG_LEVEL_MESSAGE)
		syslog_priority = LOG_NOTICE;
	else if (log_level & G_LOG_LEVEL_INFO)
		syslog_priority = LOG_INFO;
	else if (log_level & G_LOG_LEVEL_DEBUG)
		syslog_priority = LOG_DEBUG;
	else
		syslog_priority = LOG_NOTICE;
        
	syslog (syslog_priority, "%s:%s", log_domain, message);
    
	if (log_level & G_LOG_FLAG_FATAL) {
		fprintf (stderr, "%s:%s", log_domain, message);
		_exit (1);
	}
}

/**** crypt-type functions *********/

# ifdef HAVE_LIBCRYPT

static char salt_chars[] = 
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ./";

gchar *strrand (gchar *dest, int n) {
    int i;
    for (i = 0; i < n; i++)
	dest[i] = salt_chars[ rand() % sizeof(salt_chars) ];
    dest[n] = '\0';
    return dest; 
}

gchar *md5_crypt( const gchar *src, gchar *salt ) {
    gchar salt2[12], *hash;

    if (salt == NULL) {
	strcpy( salt2, "$1$" );
	strrand( salt2 + 3, 8 );
    } else {
	strncpy( salt2, salt, 11 );
    }
    salt2[11] = '\0';
    
    hash = g_strdup( crypt(src, salt2) );
    return hash;
}

# endif /* HAVE_LIBCRYPT */
