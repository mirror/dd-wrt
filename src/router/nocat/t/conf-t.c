# include <glib.h>
# include <stdio.h>
# include "util.h"
# include "conf.h"

int main (int argc, char **argv) {
    GHashTable *z = read_conf_file( argv[1] );
    puts( g_hash_as_string(z)->str );
}
