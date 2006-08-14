# include <stdio.h>
# include "util.h"
# include "conf.h"

int main (int argc, char **argv) {
    GHashTable *z = read_conf_file( "nocat.conf" );
    gchar *out = parse_template( argv[1], z );
    puts( out );
}
