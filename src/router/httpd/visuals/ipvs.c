void ej_show_ipvs(webs_t wp, int argc, char_t ** argv)
{
	int count = 0;
	static char word[256];
	char *next, *wordlist;
	char *stp = word;
	char *ipvsname, *sourceip, *sourceport, *scheduler;
	char ipvs_name[32];

	char *schedulers[] = { "wrr", "lc", "wlc", "fo", "evf", "lblc", "lblcr", "dh", "sh", "sed", "qn", NULL };

	int realcount = atoi(nvram_default_get("ipvs_count", "0"));

	websWrite(wp, "<h2>%s</h2>\n", live_translate("networking.ipvs"));
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>%s</legend>\n", live_translate("networking.create_ipvs"));

	websWrite(wp, "<table cellspacing=\"5\" summary=\"ipvs\" id=\"ipvs_table\" class=\"table center\"><tr>\n");
	websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_name"));
	websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_sourceip"));
	websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_sourceport"));
	websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_scheduler"));
	websWrite(wp, "<th>&nbsp;</th></tr>\n");

	wordlist = nvram_safe_get("ipvs");

	foreach(word, wordlist, next) {

		sourceip = word;
		ipvsname = strsep(&sourceip, ">");
		sourceport = sourceip;
		sourceip = strsep(&sourceport, ">");
		scheduler = sourceport;

		if (!ipvsname || !sourceport || !sourceip || !scheduler)
			break;

		sprintf(ipvs_name, "ipvsname%d", count);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\"size=\"3\" value=\"%s\" /></td>\n", ipvs_name, ipvsname);
		sprintf(ipvs_name, "ipvsip%d", count);
		websWrite(wp, "<td>");
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"12\" value=\"%s\" /></td>\n", ipvs_name, sourceip);
		websWrite(wp, "</td>");
		sprintf(ipvs_name, "ipvsport%d", count);
		websWrite(wp, "<td>");
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" /></td>\n", ipvs_name, sourceport);
		websWrite(wp, "</td>");
		websWrite(wp, "<td>");
		sprintf(ipvs_name, "ipvsscheduler%d", count);

		websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		int scount = 0;
		while (schedulers[scount] != NULL) {
			char translate[32];
			char *sched = schedulers[scount++];
			sprintf(translate, "networking.%s", sched);
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", sched, !strcmp(sched, scheduler) ? "selected=\\\"selected\\\"" : "", live_translate(translate));
		}
		websWrite(wp, "//]]>\n</script>\n</select>\n");
		websWrite(wp, "</td>");

		websWrite(wp,
			  "<td><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"ipvs_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
			  count);
		count++;
	}
	int i;
	int totalcount = count;

	for (i = count; i < realcount; i++) {

		sprintf(ipvs_name, "ipvsname%d", count);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\"size=\"3\"/></td>\n", ipvs_name);
		sprintf(ipvs_name, "ipvsip%d", count);
		websWrite(wp, "<td>");
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"12\" value=\"\" /></td>\n", ipvs_name);
		websWrite(wp, "</td>");
		sprintf(ipvs_name, "ipvsport%d", count);
		websWrite(wp, "<td>");
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"5\" value=\"\" /></td>\n", ipvs_name);
		websWrite(wp, "</td>");
		websWrite(wp, "<td>");
		sprintf(ipvs_name, "ipvsscheduler%d", count);

		websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		int scount = 0;
		while (schedulers[scount] != NULL) {
			char translate[32];
			char *sched = schedulers[scount];
			sprintf(translate, "networking.%s", sched);
			websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", sched, !strcmp(sched, "wrr") ? "selected=\\\"selected\\\"" : "", live_translate(translate));
		}
		websWrite(wp, "//]]>\n</script>\n</select>\n");
		websWrite(wp, "</td>");

		websWrite(wp, "<td>");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"ipvs_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
			  i);
		totalcount++;
	}
	websWrite(wp, "</table>");

	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"ipvs_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</fieldset>\n");

	char var[32];
	sprintf(var, "%d", totalcount);
	nvram_set("ipvs_count", var);
}
