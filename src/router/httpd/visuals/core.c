#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>

#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>

void show_caption(webs_t wp, const char *class, const char *caption, const char *ext)
{
	show_caption_pp(wp, class, caption, NULL, ext);
}

void show_caption_simple(webs_t wp, const char *caption)
{
	show_caption_pp(wp, NULL, caption, NULL, NULL);
}

void show_caption_legend(webs_t wp, const char *caption)
{
	show_caption_pp(wp, NULL, caption, "<legend>", "</legend>");
}

void show_ip(webs_t wp, char *prefix, char *var, int nm, int invalid_allowed, char *type)
{
	char name[64];
	if (prefix)
		snprintf(name, 64, "%s_%s", prefix, var);
	else
		snprintf(name, 64, "%s", var);
	char *ipv = nvram_default_get(name, "0.0.0.0");
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,%d,%d,%s)\" name=\"%s_0\" value=\"%d\" />.",
		nm		? 0 :
		invalid_allowed ? 0 :
				  1,
		nm ? 255 : 254, type, name, get_single_ip(ipv, 0));
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_1\" value=\"%d\" />.",
		type, name, get_single_ip(ipv, 1));
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_2\" value=\"%d\" />.",
		type, name, get_single_ip(ipv, 2));
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_3\" value=\"%d\" />\n",
		type, name, get_single_ip(ipv, 3));
}

void show_ip_cidr(webs_t wp, char *prefix, char *var, int nm, char *type, char *nmname, char *nmtype)
{
	char name[64];
	if (prefix)
		snprintf(name, 64, "%s_%s", prefix, var);
	else
		snprintf(name, 64, "%s", var);
	char *ipv = nvram_default_get(name, "0.0.0.0");
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,%d,%d,%s)\" name=\"%s_0\" value=\"%d\" />.",
		nm ? 0 : 1, nm ? 255 : 254, type, name, get_single_ip(ipv, 0));
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_1\" value=\"%d\" />.",
		type, name, get_single_ip(ipv, 1));
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_2\" value=\"%d\" />.",
		type, name, get_single_ip(ipv, 2));
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,%s)\" name=\"%s_3\" value=\"%d\" /> / ",
		type, name, get_single_ip(ipv, 3));
	websWrite(
		wp,
		"<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,32,%s)\" name=\"%s\" value=\"%d\" />\n",
		nmtype, nmname, getmask(nvram_default_get(nmname, "0.0.0.0")));
}

void show_ipnetmask(webs_t wp, char *var)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", "share.ip", NULL);
	char temp[32];
	sprintf(temp, "%s_netmask", var);
	show_ip_cidr(wp, var, "ipaddr", 0, "share.ip", temp, "share.subnet");
	websWrite(wp, "</div>\n");
}

#if 0
void showOption(webs_t wp, char *propname, char *nvname)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", propname, NULL);
	websWrite(wp, "<select name=\"%s\">\n", nvname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.disabled + \"</option>\");\n", nvram_default_matchi(nvname, 0, 0) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + share.enabled + \"</option>\");\n", nvram_default_matchi(nvname, 1, 0) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");

}
#endif

void showInputNum(webs_t wp, char *propname, char *nvname, int size, int maxsize, int def)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", propname, NULL);
	websWrite(wp, "<input class=\"num\" name=\"%s\" maxlength=\"%d\" size=\"%d\" value=\"%d\" />\n", nvname, maxsize, size,
		  nvram_default_geti(nvname, def));
	websWrite(wp, "</div>\n");
}

void showInput(webs_t wp, char *propname, char *nvname, int size, int maxsize, char *def)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", propname, NULL);
	websWrite(wp, "<input class=\"num\" name=\"%s\" maxlength=\"%d\" size=\"%d\" value=\"%s\" />\n", nvname, maxsize, size,
		  nvram_default_get(nvname, def));
	websWrite(wp, "</div>\n");
}
void showRadioNoDef(webs_t wp, char *propname, char *nvname, int val)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", propname, NULL);
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		nvname, val ? "checked=\"checked\"" : "");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		nvname, !val ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
}

#ifdef HAVE_MADWIFI
void showAutoOption(webs_t wp, char *propname, char *nvname, int nodisable)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", propname, NULL);
	websWrite(wp, "<select name=\"%s\">\n", nvname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"-1\\\" %s >\" + share.auto + \"</option>\");\n",
		  nvram_default_matchi(nvname, 0, -1) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + share.enabled + \"</option>\");\n",
		  nvram_default_matchi(nvname, 1, -1) ? "selected=\\\"selected\\\"" : "");
	if (!nodisable)
		websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >\" + share.disabled + \"</option>\");\n",
			  nvram_default_matchi(nvname, 0, -1) ? "selected=\\\"selected\\\"" : "");
	websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");
}
#endif

void showOptions_trans(webs_t wp, char *propname, char *names, char **trans, char *select)
{
	char *next;
	char var[80];
	int cnt = 0;

	websWrite(wp, "<select name=\"%s\">\n", propname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	foreach(var, names, next)
	{
		if (trans)
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + %s + \"</option>\");\n", var,
				  select && !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", trans[cnt++]);
		else
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", var,
				  select && !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", var);
	}
	websWrite(wp, "//]]>\n</script>\n</select>\n");
}

void showOptions(webs_t wp, char *propname, char *names, char *select)
{
	showOptions_trans(wp, propname, names, NULL, select);
}

void showOptions_ext_trans(webs_t wp, char *propname, char *names, char **trans, char *select, int disabled)
{
	char *next;
	char var[80];
	int cnt = 0;

	websWrite(wp, "<select name=\"%s\"%s>\n", propname, disabled ? " disabled=\"true\"" : "");
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	foreach(var, names, next)
	{
		if (trans)
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + %s + \"</option>\");\n", var,
				  select && !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", trans[cnt++]);
		else
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", var,
				  select && !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", var);
	}
	websWrite(wp, "//]]>\n</script>\n</select>\n");
}

void showOptionsNames(webs_t wp, char *label, char *propname, char *valuenames, char **names, char *select)
{
	char *next;
	char var[80];
	int idx = 0;

	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", label, NULL);
	websWrite(wp, "<select name=\"%s\">\n", propname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	foreach(var, valuenames, next)
	{
		if (names[idx])
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", var,
				  !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", names[idx++]);
	}
	websWrite(wp, "//]]>\n</script>\n</select></div>\n");
}

void showIfOptions_ext(webs_t wp, char *propname, char *names, char *select, int disabled)
{
	char *next;
	char var[80];

	websWrite(wp, "<select name=\"%s\"%s>\n", propname, disabled ? " disabled=\"true\"" : "");
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	foreach(var, names, next)
	{
		websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", var,
			  !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", getNetworkLabel(wp, var));
	}
	websWrite(wp, "//]]>\n</script>\n</select>\n");
}

void showIfOptions(webs_t wp, char *propname, char *names, char *select)
{
	showIfOptions_ext(wp, propname, names, select, 0);
}

void showOptionsChoose(webs_t wp, char *propname, char *names, char **trans, char *select)
{
	char *next;
	char var[80];
	int cnt = 0;

	websWrite(wp, "<select name=\"%s\">\n", propname);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(wp, "document.write(\"<option value=\\\"null\\\" >\" + share.choice + \"</option>\");\n");
	foreach(var, names, next)
	{
		if (trans) {
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + %s + \"</option>\");\n", var,
				  !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", trans[cnt++]);
		} else {
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", var,
				  !strcmp(var, select) ? "selected=\\\"selected\\\"" : "", var);
		}
	}
	websWrite(wp, "//]]>\n</script>\n</select>\n");
}

void showOptionsLabel(webs_t wp, char *labelname, char *propname, char *names, char *select)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", labelname, NULL);
	showOptions(wp, propname, names, select);
	websWrite(wp, "</div>\n");
}

void show_inputlabel(webs_t wp, char *labelname, char *propertyname, int propertysize, char *inputclassname, int inputmaxlength)
{
	websWrite(wp, "<div class=\"setting\">\n");
	show_caption(wp, "label", labelname, NULL);
	websWrite(wp, "<input class=\"%s\" size=\"%d\" maxlength=\"%d\" name=\"%s\" value=\"%s\" />\n", inputclassname,
		  propertysize, inputmaxlength, propertyname, nvram_safe_get(propertyname));
	websWrite(wp, "</div>\n");
}

void show_custominputlabel(webs_t wp, char *labelname, char *propertyname, char *property, int propertysize)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div>", labelname);
	websWrite(wp, "<input size=\"%d\" name=\"%s\" value=\"%s\" />\n", propertysize, propertyname, property);
	websWrite(wp, "</div>\n");
}

void showRadio(webs_t wp, char *propname, char *nvname)
{
	showRadioDefaultOff(wp, propname, nvname);
}

void showRadioInv(webs_t wp, char *propname, char *nvname)
{
	showRadioDefaultOn(wp, propname, nvname);
}

void showRadioPrefix(webs_t wp, char *propname, char *nv, char *prefix)
{
	char nvname[32];
	sprintf(nvname, "%s_%s", prefix, nv);
	showRadioNoDef(wp, propname, nvname, nvram_default_geti(nvname, 0));
}

void showInputNumPrefix(webs_t wp, char *propname, char *nv, char *prefix, int size, int maxsize, int def)
{
	char nvname[64];
	sprintf(nvname, "%s_%s", prefix, nv);
	showInputNum(wp, propname, nvname, size, maxsize, def);
}
void showInputPrefix(webs_t wp, char *propname, char *nv, char *prefix, int size, int maxsize, char *def)
{
	char nvname[64];
	sprintf(nvname, "%s_%s", prefix, nv);
	showInput(wp, propname, nvname, size, maxsize, def);
}

void showRadioInvPrefix(webs_t wp, char *propname, char *nv, char *prefix)
{
	char nvname[64];
	sprintf(nvname, "%s_%s", prefix, nv);
	showRadioNoDef(wp, propname, nvname, nvram_default_geti(nvname, 1));
}
