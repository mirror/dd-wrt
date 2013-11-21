#ifdef HAVE_STATUS_GPIO

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>

#include <broadcom.h>
#include <cymac.h>

void ej_show_status_gpio_output(webs_t wp, int argc, char_t ** argv)
{
	char *var, *next, *rgpio, *gpio_name;
	char nvgpio[32], gpio_new_name[32];

	char *gpios = nvram_safe_get("gpio_outputs");
	var = (char *)malloc(strlen(gpios) + 1);
	if (var != NULL) {
		if (gpios != NULL) {
			foreach(var, gpios, next) {
				sprintf(nvgpio, "gpio%s", var);
				sprintf(gpio_new_name, "gpio%s_name", var);
				rgpio = nvram_nget("gpio%s", var);
				if (strlen(rgpio) == 0)
					nvram_set(nvgpio, "0");

				rgpio = nvram_nget("gpio%s", var);
				gpio_name = nvram_nget("gpio%s_name", var);
				// enable
				websWrite(wp, "<div class=\"label\">%s (%s)</div>", nvgpio, gpio_name);
				websWrite(wp, "<input type=text maxlength=\"17\" size=\"17\" id=\"%s\" name=\"%s\" value=\"%s\">", gpio_new_name, gpio_new_name, gpio_name);
				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" name=\"%s\" value=\"1\" %s />\n", nvgpio, nvram_match(nvgpio, "1") ? "checked=\"checked\"" : "");
				websWrite(wp, "<script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;");
				//disable 
				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" name=\"%s\" value=\"0\" %s />\n", nvgpio, nvram_match(nvgpio, "0") ? "checked=\"checked\"" : "");
				websWrite(wp, "<script type=\"text/javascript\">Capture(share.disable)</script><br>");
			}
		}
		free(var);
	}
}

void ej_show_status_gpio_input(webs_t wp, int argc, char_t ** argv)
{
	char *var, *next, *rgpio, *gpio_name;
	char nvgpio[32], gpio_new_name[32];

	char *gpios = nvram_safe_get("gpio_inputs");
	var = (char *)malloc(strlen(gpios) + 1);
	if (var != NULL) {
		if (gpios != NULL) {
			foreach(var, gpios, next) {
				sprintf(nvgpio, "gpio%s", var);
				gpio_name = nvram_nget("gpio%s_name", var);
				sprintf(gpio_new_name, "gpio%s_name", var);

				// enable
				websWrite(wp, "<div class=\"label\">%s</div>", nvgpio);
				websWrite(wp, "<input maxlength=\"17\" size=\"17\" id=\"%s\" name=\"%s\" value=\"%s\">", gpio_new_name, gpio_new_name, gpio_name);

				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" name=\"%s\" value=\"1\" disabled=\"true\" %s />\n", nvgpio, !get_gpio(atoi(var)) ? "checked=\"checked\"" : "");
				websWrite(wp, "<script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;");
				//Disable
				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" name=\"%s\" value=\"0\" disabled=\"true\" %s />\n", nvgpio, get_gpio(atoi(var)) ? "checked=\"checked\"" : "");
				websWrite(wp, "<script type=\"text/javascript\">Capture(share.disable)</script><br>");
			}
		}
		free(var);
	}
}

#endif
