#ifdef HAVE_WPS

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>
#include <cymac.h>


void ej_get_wpsstatus(webs_t wp, int argc, char_t ** argv)
{
	if (nvram_match("wps_status","0"))
	    websWrite(wp,"Unconfigured");
	else
	if (nvram_match("wps_status","1"))
	    websWrite(wp,"Configured");
	else
	if (nvram_match("wps_status","2"))
	    websWrite(wp,"Waiting for Enrollee");
	else
	if (nvram_match("wps_status","3"))
	    websWrite(wp,"Waiting for Registrar");
	else
	    websWrite(wp,"Configured");
	return;
}
#endif
