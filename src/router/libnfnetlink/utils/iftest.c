/* simple test for index to interface name API */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <net/if.h>

#include <libnfnetlink/libnfnetlink.h>

int main()
{
	int i;
	struct nlif_handle *h;

	h = nlif_open();
	if (h == NULL) {
		perror("nlif_open");
		exit(EXIT_FAILURE);
	}

	nlif_query(h);

	for (i=0; i<64; i++) {
		char name[IFNAMSIZ];
		unsigned int flags;

		if (nlif_index2name(h, i, name) == -1)
			continue;
		if (nlif_get_ifflags(h, i, &flags) == -1)
			continue;
		printf("index (%d) is %s (%s)\n", i, name,
			flags & IFF_RUNNING ? "RUNNING" : "NOT RUNNING");
	}

	nlif_close(h);
}
