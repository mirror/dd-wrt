#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>

static int run_wiviz_main(int argc, char **argv)
{
	if (pidof("wiviz") > 0)
		killall("wiviz", SIGUSR1);
	else {
		char *hopseq = nvram_safe_get("hopseq");
		FILE *fp = fopen("/tmp/wiviz2-cfg", "wb");
		if (nvram_matchi("hopseq", 0))
			fprintf(fp, "channelsel=hop&");
		else if (strstr(hopseq, ","))
			fprintf(fp, "channelsel=hop&");
		else
			fprintf(fp, "channelsel=%s&", hopseq);
		fprintf(fp, "hopdwell=%s&hopseq=%s\n",
			nvram_safe_get("hopdwell"), hopseq);
		fclose(fp);
		if (pidof("wiviz") > 0)
			exit(0);
		eval("wiviz");
	}
	return 0;
}
