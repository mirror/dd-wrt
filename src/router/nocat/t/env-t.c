# include <stdio.h>

extern char **environ;

int main (int argc, char **argv) {
    char **n;
    for ( n = environ; *n != NULL; n++ )
	puts(*n);
}
