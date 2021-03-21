#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern int ksmbd_main(int argc, char **argv);
extern int smbuseradd_main(int argc, char **argv);
extern int smbshareadd_main(int argc, char **argv);
//extern int airbag_init(void);

int main(int argc, char **argv)
{
	bool restart = false;
	const char *prog = argv[0];
//	airbag_init();
restart:
	if (strstr(argv[0], "ksmbd.mountd"))
		return ksmbd_main(argc, argv);
	else if (strstr(argv[0], "ksmbd.addshare"))
		return smbshareadd_main(argc, argv);
	else if (strstr(argv[0], "ksmbd.adduser"))
		return smbuseradd_main(argc, argv);

	if (!restart && argc > 1) {
		argv++;
		argc--;
		restart = true;
		goto restart;
	}

	fprintf(stderr, "Invalid command.\nUsage: %s ksmbd|ksmbd.adduser|ksmbd.addshare [<arguments>]\n", prog);
	return 255;
}
