#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <airbag.h>


int main(int argc, char *argv[])
{
airbag_init();
fprintf(stderr, "test\n");
char *dest = malloc(-1);
strcpy(dest, "test");
fprintf(stderr, "%s", dest);


}

