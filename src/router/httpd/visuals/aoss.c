#ifdef HAVE_AOSS

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>
#include <cymac.h>

int aoss_status(void) {
	if(pidof("aoss") > 0) {
		return 1;
	}
	return 0;
}

void start_aoss(void) {
	if(!aoss_status()) {
		system("startservice aoss");
	}
}

void ej_isChecked(webs_t wp, int argc, char_t ** argv) {
	fprintf(stderr, "[%s] %s %s\n", argv[0], argv[1], nvram_selget(wp, argv[0]));
	if(!strcmp(nvram_selget(wp, argv[0]), argv[1])) {
		websWrite(wp, " checked");
	}
	return;
}	
#endif