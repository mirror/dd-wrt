/* znth.c - ZN122x timestamp adjustments history */

/* Written 1996-1998 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <atm.h>
#include <linux/atm_zatm.h>


int main(int argc,char **argv)
{
    struct atmif_sioc sioc;
    struct zatm_t_hist history[ZATM_TIMER_HISTORY_SIZE];
    char *end;
    int s,i;

    if (argc != 2 || ((sioc.number = strtoul(argv[1],&end,0)), *end)) {
	fprintf(stderr,"usage: %s itf\n",*argv);
	return 1;
    }
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
        perror("socket");
        return 1;
    }
    sioc.arg = history;
    sioc.length = sizeof(history);
    if (ioctl(s,ZATM_GETTHIST,&sioc) < 0) {
	perror("ioctl ZATM_GETTHIST");
	return 1;
    }
    for (i = 0; i < ZATM_TIMER_HISTORY_SIZE; i++) {
	struct timeval diff;

	if (!history[i].real.tv_sec) continue;
	printf("%2ld:%02ld:%02ld.%06ld: ",
	  ((long) history[i].real.tv_sec/3600) % 24,
	  ((long) history[i].real.tv_sec/60) % 60,
	  (long) history[i].real.tv_sec % 60,
	  (long) history[i].real.tv_usec);
	history[i].expected.tv_sec += history[i].expected.tv_usec/1000000;
	history[i].expected.tv_usec %= 1000000;
	diff.tv_sec = history[i].expected.tv_sec-history[i].real.tv_sec;
	diff.tv_usec = history[i].expected.tv_usec-history[i].real.tv_usec;
	if (diff.tv_sec < -2000 || diff.tv_sec > 2000)
	    printf("%11ld SECONDS\n",(long) diff.tv_sec);
	else {
	    diff.tv_usec += diff.tv_sec*1000000;
	    printf("%11ld usec\n",(long) diff.tv_usec);
	}
    }
    return 0;
}
