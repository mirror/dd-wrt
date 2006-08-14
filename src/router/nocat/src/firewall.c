# include <glib.h>
# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <string.h>
# include <errno.h>
# include <time.h>

# include "firewall.h"
# include "conf.h"
# include "util.h"

extern char **environ;

typedef struct {
    pid_t pid;
    gchar *cmd;
    peer *p;
} fw_action;

static void fw_exec_add_env ( gchar *key, gchar *val, GPtrArray *env ) {
    gchar *p;
    
    g_assert( key != NULL );
    g_assert( val != NULL );
    p = g_strdup_printf( "%s=%s", key, val );
    g_ptr_array_add( env, p );
}

static void fw_popen_set_env ( gchar *key, gchar *val, void* dummy ) {
    g_assert( key != NULL );
    g_assert( val != NULL );
    setenv(key,val,1);
}

static int fw_exec( fw_action *act, GHashTable *conf ) {
    GHashTable *data;
    GPtrArray *env;
    gchar *cmd, **arg, **n;

    data = g_hash_dup( conf );
    //
    // Than add specifics about this particular client, if any
    if (act->p != NULL) {
	g_hash_set( data, "IP",    act->p->ip );
	g_hash_set( data, "MAC",   act->p->hw );
	g_hash_set( data, "Class", "Public" );
    }

    cmd = conf_string( conf, act->cmd );
    cmd = parse_template( cmd, data );
    if (CONFd("Verbosity") >= 5) g_message("Got command %s from action %s", cmd, act->cmd );
    arg = g_strsplit( cmd, " ", 0 );

    // prime the environment with our existing environment
    env = g_ptr_array_new();
    for ( n = environ; *n != NULL; n++ )
	g_ptr_array_add( env, *n );

    // Then add everything from the conf file
    g_hash_table_foreach( data, (GHFunc) fw_exec_add_env, env );

    // Add a closing NULL so execve knows where to lay off.
    g_ptr_array_add( env, NULL );

    /* We're not cleaning up memory references because 
     * hopefully the exec won't fail... */
    execve( *arg, arg, (char **)env->pdata );
    g_error( "execve %s failed: %m", cmd ); // Shouldn't happen.
    return -1;
}

gboolean fw_wait( fw_action *act ) {
    
    guint status = 0, retval = 0;
    pid_t r = 0;
    
    r = waitpid( act->pid, &status, WNOHANG );
    
    if (! r) {
        return TRUE;
    } else if (r == -1 && errno != EINTR && errno != ECHILD ) {
        g_warning( "waitpid failed: %m" );
        return TRUE;
    } else if (WIFEXITED(status)) {
        retval = WEXITSTATUS(status);
        if (retval)
            g_warning( "%s on peer %s returned %d",
		    act->cmd, act->p->ip, retval );
    } else if (WIFSIGNALED(status)) {
        retval = WTERMSIG(status);
        g_warning( "%s of peer %s died from signal %d", 
                act->cmd, act->p->ip, retval );
    }
    
    g_free( act );
    return FALSE;
}

gboolean fw_cleanup( fw_action *act ) {
    return fw_wait( act );
}

// timeout = 0 , do not wait
int fw_perform_exec( gchar *action, GHashTable *conf, peer *p, gboolean wait ) {
    
    fw_action *act = g_new( fw_action, 1 );
    pid_t pid;
    
    act->cmd = action;
    act->p   = p;
    
    pid = fork();
    if (pid == -1)
        g_error( "Can't fork: %m" );
    
    if (! pid)
        fw_exec( act, conf );
    
    act->pid = pid;
    if ( !wait ) {
        g_idle_add( (GSourceFunc) fw_cleanup, act );
    } else {
        while(fw_wait(act));
    }
    return 0;
}

int fw_perform( gchar *action, GHashTable *conf, peer *p ) {
    return fw_perform_exec( action, conf, p, FALSE );
}

int fw_perform_return( gchar *action, GHashTable *conf, peer *p, gchar *result, int size) {
    GHashTable *data;
    gchar *cmd;
    FILE * fd;
    int ret=0;

    gchar dummy_result[30];
    if ( !result ) {
        result = dummy_result;
        size = sizeof(dummy_result);
    }
    
    data = g_hash_dup( conf );
    //
    // Than add specifics about this particular client, if any
    if (p != NULL) {
	g_hash_set( data, "IP",    p->ip );
	g_hash_set( data, "MAC",   p->hw );
	g_hash_set( data, "Class", "Public" );
    }

    cmd = conf_string( conf, action );
    cmd = parse_template( cmd, data );
    g_warning("Got command %s from action %s", cmd, action );
    
    // add data to the environment
    g_hash_table_foreach( data, (GHFunc) fw_popen_set_env, NULL );

    fd = popen(cmd, "r");
    if(fd == NULL) { 
        g_warning( "popen %s failed: %m", cmd );
        ret=1;
    }
    else if ( fgets(result, size, fd) == NULL ) {
        g_warning( "fgets %s failed: %m", cmd );
        ret=1;
    }
    if ( fd && pclose(fd) != 0 ) {
        g_warning( "pclose %s failed: %m", cmd );
        ret=1;
    }
    g_hash_free(data);
    g_free(cmd);
    return ret;
}

int fw_init ( GHashTable *conf ) {
    return fw_perform_exec( "ResetCmd", conf, NULL, TRUE );
}

int fw_re_init ( GHashTable *conf ) {
    // await completion
    //return fw_perform_return( "ResetCmd", conf, NULL, NULL, 0);
    return fw_perform_exec( "ResetCmd", conf, NULL, TRUE );
}

/******* peer.c routines **********/
/*
void peer_extend_timeout( GHashTable *conf, peer *p ) {
    p->expire = time(NULL) + conf_int( conf, "LoginTimeout" );
}
*/

peer *peer_new ( GHashTable *conf, const gchar *ip ) {
    peer *p;
    gchar *hw;
    g_assert( ip != NULL );
    if (CONFd("Verbosity") >= 5) g_message("peer_new: Creating new peer object for %s.", ip);
    
    // Find & Set MAC address.
    if ( (hw = find_peer_arp( ip )) == NULL ){
        if (CONFd("Verbosity") >= 5)
            g_message( "peer_new: No MAC Address found for peer %s, not allocated!", ip );
        return NULL;
    }
    p = g_new0( peer, 1 );
    g_strncpy(p->hw, hw, strlen(hw)+1 );

    // Set IP address.
    g_strncpy( p->ip, ip, strlen(ip)+1 );
    
    // Set connection time.
    p->connected = time( NULL );
    p->token[0] = '\0';
    //peer_extend_timeout(conf, p);
    p->idle_check =0;
    p->name = NULL;
    p->session_id = NULL;
    p->acct_input_octets=0;
    p->acct_output_octets=0;
    p->acct_session_start=0;
    p->missing_count=0;
    p->auth_check =0;
    p->password = NULL;
	p->expire =0;
    return p;
}

void peer_free ( peer *p ) {
    g_assert( p != NULL );
    if (p->request != NULL)
        g_free( p->request );
    if ( p->name != NULL )
        g_free( p->name );
    if ( p->password != NULL )
        g_free( p->password );
    if ( p->session_id != NULL )
        g_free( p->session_id );
    g_free(p);
}

int peer_permit ( GHashTable *conf, peer *p ) {
    unsigned int login_timeout;
    g_assert( p != NULL );
    if (p->status != PEER_ACCEPT) {
	if (fw_perform_exec( "PermitCmd", conf, p, TRUE ) == 0) {
	    p->status = PEER_ACCEPT;
	} else {
	    return -1;
	}
    }
    //peer_extend_timeout(conf, p);
    if (login_timeout = conf_int( conf, "LoginTimeout" )) p->expire = time(NULL) + login_timeout;
    if (conf_int(conf, "IdleTimeout")) p->idle_check = time(NULL) + conf_int( conf, "IdleTimeout" );
    return 0;
}

int peer_deny ( GHashTable *conf, peer *p ) {
    g_assert( p != NULL );
    if (p->status != PEER_DENY) {
	if (fw_perform_exec( "DenyCmd", conf, p, TRUE ) == 0) {
	    p->status = PEER_DENY;
	} else {
	    return -1;
	}
    }
    return 0;
}





void peer_set_name( peer *p, const char* name ){
    p->name = strdup(name); 
}
void peer_set_password( peer *p, const char* password ){
    p->password = strdup(password);
}
void peer_set_auth_check( peer *p, const time_t auth_check ){
    p->auth_check = auth_check;
}
void peer_set_session_id( peer *p, const char* session_id ){
    p->session_id = strdup(session_id); 
}

# ifdef HAVE_LIBCRYPT

gchar *get_peer_token ( peer *p ) {
    char *n;
    int carry = 1;
    int len = sizeof(p->token) - 1;

    g_assert( p != NULL );
    if (! *(p->token))
	strrand(p->token, len);	

    for (n = p->token + len - 1; carry && n >= p->token; n--)
	switch (*n) {
	    case '9': *n = '0'; carry = 1; break;
	    case 'Z': *n = 'A'; carry = 1; break;
	    case 'z': *n = 'a'; carry = 1; break;
	    default : (*n)++; carry = 0;
	}
        
    n = md5_crypt( p->token, p->token + len - 8 );
    strncpy( p->token, n, len );
    p->token[len - 1] = '\0';

    g_free(n);
    return p->token;
}

# endif /* HAVE_LIBCRYPT */
