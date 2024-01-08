#include <stdio.h>
#include <time.h>

int main(int argc, char argv[])
{
	srand(time(NULL));
	fprintf(stdout, "#ifndef WEBS_H_\n");
	fprintf(stdout, "#define WEBS_H_\n");
	fprintf(stdout, "#define websRomPageIndex %c%d\n", 'A' + (rand() % 20), rand() % 65536);
	fprintf(stdout, "#define WEBSOFFSET %d\n", rand() % 65536);
	fprintf(stdout, "#endif\n");
	return 0;
}
