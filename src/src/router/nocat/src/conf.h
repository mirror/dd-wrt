# include <glib.h>
# include <stdlib.h> // strtod, strtof
# include "config.h"

# define CONF(x)    conf_string( nocat_conf, (x) )
# define CONFd(x)   conf_int( nocat_conf, (x) )
# define CONFf(x)   conf_float( nocat_conf, (x) )

struct conf_t {
    gchar *param;
    gchar *value;
};

extern struct conf_t default_conf[];
extern GHashTable *nocat_conf;

/*** Function prototypes start here ***/
GHashTable *parse_conf_line( GHashTable *out, gchar *line );
GHashTable *parse_conf_string( const gchar *in );
GHashTable *set_conf_defaults( GHashTable *conf, struct conf_t *def );
void set_network_defaults( GHashTable *conf );
GHashTable *read_conf_file( const gchar *path );
gchar *conf_string( GHashTable *conf, const gchar *key );
glong conf_int( GHashTable *conf, const gchar *key );
gdouble conf_float( GHashTable *conf, const gchar *key );
