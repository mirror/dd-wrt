#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern int dhcp6c_main(int argc, char **argv);
extern int dhcp6s_main(int argc, char **argv);

int main(int argc, char **argv)
{
	bool restart = false;
	const char *prog = argv[0];

restart:
	if (strstr(argv[0], "dhcp6c"))
		return dhcp6c_main(argc, argv);
	else if (strstr(argv[0], "dhcp6s"))
		return dhcp6s_main(argc, argv);

	if (!restart && argc > 1) {
		argv++;
		argc--;
		restart = true;
		goto restart;
	}

	fprintf(stderr, "Invalid command.\nUsage: %s dhcp6c|dhcp6s [<arguments>]\n", prog);
	return 255;
}
