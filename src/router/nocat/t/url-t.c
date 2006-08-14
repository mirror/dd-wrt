# include <glib.h>
# include <stdio.h>
# include "util.c"

int main (int argc, char **argv) {
    gchar *foo = url_encode( argv[1] );
    printf( "string: %s\nencoded: %s\ndecoded: %s\n",
	    argv[1], foo, url_decode(foo) );
}
