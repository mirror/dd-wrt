#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern int mactelnetd_main(int argc, char **argv);
extern int mactelnet_main(int argc, char **argv);
extern int macping_main(int argc, char **argv);
extern int mndp_main(int argc, char **argv);
//extern int airbag_init(void);

int main(int argc, char **argv)
{
	bool restart = false;
	const char *prog = argv[0];
//	airbag_init();
restart:
	if (strstr(argv[0], "btrfstune"))
		return btrfstune_main(argc, argv);
	else if (strstr(argv[0], "btrfs"))
		return btrfs_main(argc, argv);
	else if (strstr(argv[0], "mkfs.btrfs"))
		return mkfs_main(argc, argv);

	if (!restart && argc > 1) {
		argv++;
		argc--;
		restart = true;
		goto restart;
	}

	fprintf(stderr, "Invalid command.\nUsage: %s btrfstune|btrfs|mkfs.btrfs [<arguments>]\n", prog);
	return 255;
}
