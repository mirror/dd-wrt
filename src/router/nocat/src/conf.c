# include <glib.h>
# include <ctype.h>
# include <time.h>
# include "conf.h"
# include "util.h"

/*** Global config settings ***/
GHashTable *nocat_conf = NULL;

/********** Conf stuff **********/

// NOTE: parse_conf_line destroys line!!!
GHashTable *parse_conf_line( GHashTable *out, gchar *line ) {
    gchar *key, *val;

    g_assert( out  != NULL );
    g_assert( line != NULL );

    // Format is: \s*(?:#.*|(\w+)\s+(.*)\s*)
    key = g_strchug(line);	    // Strip leading whitespace.
    if (!isalpha( *key )) return out;	    // Skip comments.
    for (val = key; isident(*val); val++); 
					// Find the end of the key.
    *(val++) = '\0';		    // Mark it. 
    g_strstrip(val);		    // The value extends to EOL.
    g_hash_set( out, key, val );
    return out;
}

GHashTable *parse_conf_string( const gchar *in ) {
    gchar **lines;
    guint i;
    GHashTable *out = g_hash_new();

    lines = g_strsplit( in, "\n", 0 );
    for ( i = 1; lines[i] != NULL; i++ )
	parse_conf_line( out, lines[i] );

    g_strfreev( lines );
    return out;
}

GHashTable *set_conf_defaults( GHashTable *conf, struct conf_t *def ) {
    guint i;
    time_t now;
    
    for (i = 0; def[i].param != NULL; i++)
	if (g_hash_table_lookup(conf, def[i].param) == NULL) {
	    if (def[i].value == NULL)
		g_error("Required config param missing: %s", 
			def[i].param);
	    else
		g_hash_set(conf, def[i].param, def[i].value);
	}

    time(&now);
    g_hash_set( conf, "GatewayStartTime", ctime(&now) );

    return conf; 
}


static void scan_network_defaults( GHashTable *conf, int overwrite ) {
    gchar *intdev, *extdev, *localnet, *mac;

    extdev = g_hash_table_lookup(conf, "ExternalDevice");
    if ( overwrite || extdev == NULL ) {
        extdev = detect_network_device(NULL); 
        if (extdev) {
            if (CONFd("Verbosity") >= 5) g_message( "Autodetected ExternalDevice %s", extdev );
            g_hash_table_insert( conf, "ExternalDevice", extdev );
        } else
            g_error( "No ExternalDevice detected!" );
    }
    
    intdev = g_hash_table_lookup(conf, "InternalDevice");
    if ( overwrite || intdev == NULL ) {
        intdev = detect_network_device(extdev); 
        if (intdev) {
            if (CONFd("Verbosity") >= 5) g_message( "Autodetected InternalDevice %s", intdev );
            g_hash_table_insert( conf, "InternalDevice", intdev );
        } else
            g_error( "No InternalDevice detected!" );
    }
    
    if ( overwrite || g_hash_table_lookup(conf, "LocalNetwork") == NULL) {
        localnet = get_network_address(intdev);
        if ( localnet) {
            if (CONFd("Verbosity") >= 5) g_message( "Autodetected LocalNetwork %s", localnet );
            g_hash_table_insert( conf, "LocalNetwork", localnet );
        } else
            g_error( "No LocalNetwork detected!" );
    }
        
    if ( overwrite || g_hash_table_lookup(conf, "NodeID") == NULL) {
        mac = get_mac_address(intdev);
        if (mac) {
            g_hash_table_insert(conf, "NodeID", mac);
            if (CONFd("Verbosity") >= 5) g_message( "My node ID is %s (%s)", mac, intdev);
        } else
            if (CONFd("Verbosity") >= 5) g_warning( "No NodeID discernable from MAC address!" );
    }
}

void reset_network_defaults( GHashTable *conf ) {
    scan_network_defaults( conf, TRUE );
}

void set_network_defaults( GHashTable *conf ) {
    scan_network_defaults( conf, FALSE );
}


GHashTable *read_conf_file( const gchar *path ) {
    gchar *file = load_file(path);

    if (file == NULL) 
	return NULL;
    
    if (nocat_conf != NULL) {
	g_warning("Reloading configuration from %s!", path);
	g_free(nocat_conf);
    }

    nocat_conf = parse_conf_string( file );
    set_conf_defaults( nocat_conf, default_conf );

    // g_message( "Read %d config items from %s", g_hash_table_size(nocat_conf), path ); 
    g_free( file );
    return nocat_conf;
}

gchar *conf_string( GHashTable *conf, const gchar *key ) {
    gchar *val = g_hash_table_lookup( conf, key );
    g_assert( key != NULL );
    g_assert( conf != NULL );
    if (val == NULL)
	g_warning("Missing required configuration directive '%s'", key);
    return val;
}

glong conf_int( GHashTable *conf, const gchar *key ) {
    gchar *val = g_hash_table_lookup( conf, key );
    gchar *err;
    glong vint;
    g_assert( key != NULL );
    if (val == NULL)
	g_warning("Missing required configuration directive '%s'", key);
    vint = strtol( val, &err, 10 );
    if ( err != NULL && *err != '\0' )
	g_warning("Invalid numeric configuration directive '%s': %s", key, val );
    return vint;
}

gdouble conf_float( GHashTable *conf, const gchar *key ) {
    gchar *val = g_hash_table_lookup( conf, key );
    gchar *err;
    gdouble vdbl;
    g_assert( key != NULL );
    if (val == NULL)
	g_warning("Missing required configuration directive '%s'", key);
    vdbl = strtod( val, &err );
    if ( err != NULL && *err != '\0' )
	g_warning("Invalid numeric configuration directive '%s': %s", key, val );
    return vdbl;
}
