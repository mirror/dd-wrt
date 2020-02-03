#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern int usmbd_main(int argc, char **argv);
extern int smbuseradd_main(int argc, char **argv);
extern int smbshareadd_main(int argc, char **argv);
//extern int airbag_init(void);

int main(int argc, char **argv)
{
	bool restart = false;
	const char *prog = argv[0];
//	airbag_init();
restart:
	if (strstr(argv[0], "usmbd"))
		return usmbd_main(argc, argv);
	else if (strstr(argv[0], "smbshareadd"))
		return smbshareadd_main(argc, argv);
	else if (strstr(argv[0], "smbuseradd"))
		return smbuseradd_main(argc, argv);

	if (!restart && argc > 1) {
		argv++;
		argc--;
		restart = true;
		goto restart;
	}

	fprintf(stderr, "Invalid command.\nUsage: %s usmbd|smbuseradd|smbshareadd [<arguments>]\n", prog);
	return 255;
}
