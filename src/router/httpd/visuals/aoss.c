#ifdef HAVE_AOSS

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>
#include <cymac.h>

int aoss_status(void)
{
	if (pidof("aoss") > 0) {
		return 1;
	}
	return 0;
}

void start_aoss(void)
{
	if (!aoss_status()) {
		system("startservice aoss");
	}
}

void ej_isChecked(webs_t wp, int argc, char_t ** argv)
{
	fprintf(stderr, "[%s] %s %s\n", argv[0], argv[1], nvram_selget(wp, argv[0]));
	if (!strcmp(nvram_selget(wp, argv[0]), argv[1])) {
		websWrite(wp, " checked");
	}
	return;
}

void ej_ifnvram_match(webs_t wp, int argc, char_t ** argv)
{
	if (!strcmp(nvram_selget(wp, argv[0]), argv[1])) {
		websWrite(wp, "%s", argv[2]);
	}
	return;
}

void ej_ifnvram_nmatch(webs_t wp, int argc, char_t ** argv)
{
	if (strcmp(nvram_selget(wp, argv[0]), argv[1])) {
		websWrite(wp, "%s", argv[2]);
	}
	return;
}

void ej_ifaoss_possible(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_WZRHPAG300NH
	if (!strcmp(argv[0], "yes")) {
		if (!strcmp(nvram_selget(wp, "ath0_mode"), "ap")
		    || !strcmp(nvram_selget(wp, "ath0_mode"), "wdsap")
		    || !strcmp(nvram_selget(wp, "ath1_mode"), "ap")
		    || !strcmp(nvram_selget(wp, "ath1_mode"), "wdsap")) {
			websWrite(wp, "%s", argv[1]);
		}
	} else if (!strcmp(argv[0], "no")) {
		if (strcmp(nvram_selget(wp, "ath0_mode"), "ap")
		    && strcmp(nvram_selget(wp, "ath0_mode"), "wdsap")
		    && strcmp(nvram_selget(wp, "ath1_mode"), "ap")
		    && strcmp(nvram_selget(wp, "ath1_mode"), "wdsap")) {
			websWrite(wp, "%s", argv[1]);
		}
	}
#else
	if (!strcmp(argv[0], "yes")) {
		if (!strcmp(nvram_selget(wp, "ath0_mode"), "ap")
		    || !strcmp(nvram_selget(wp, "ath0_mode"), "wdsap")) {
			websWrite(wp, "%s", argv[1]);
		}
	} else if (!strcmp(argv[0], "no")) {
		if (strcmp(nvram_selget(wp, "ath0_mode"), "ap")
		    && strcmp(nvram_selget(wp, "ath0_mode"), "wdsap")) {
			websWrite(wp, "%s", argv[1]);
		}
	}
#endif
	return;
}
#endif
