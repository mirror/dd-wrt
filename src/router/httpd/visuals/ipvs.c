/*
 * ipvs.c
 *
 * Copyright (C) 2005 - 2016 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
void ej_show_ipvsassignments(webs_t wp, int argc, char_t ** argv)
{
	int count = 0;
	char word[256];
	char *next, *wordlist;

	char tword[256];
	char *tnext, *twordlist;
	char *ipvsname, *targetip, *targetport, *targetweight, *targetnat;
	char ipvs_name[32];

	if (strlen(nvram_safe_get("ipvs"))) {
		int realcount = atoi(nvram_default_get("ipvstarget_count", "0"));
		websWrite(wp, "<fieldset>\n");
		websWrite(wp, "<legend>%s</legend>\n", live_translate("networking.ipvs_targets"));

		websWrite(wp, "<table cellspacing=\"5\" summary=\"ipvstargets\" id=\"ipvstarget_table\" class=\"table center\"><tr>\n");
		websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_name"));
		websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_targetip"));
		websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_targetport"));
		websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_weight"));
		websWrite(wp, "<th>%s</th>\n", live_translate("wl_basic.masquerade"));
		websWrite(wp, "<th>&nbsp;</th></tr>\n");

		wordlist = nvram_safe_get("ipvstarget");

		foreach(word, wordlist, next) {
			targetip = word;
			ipvsname = strsep(&targetip, ">");
			targetport = targetip;
			targetip = strsep(&targetport, ">");
			targetweight = targetport;
			targetport = strsep(&targetweight, ">");
			targetnat = targetweight;
			targetweight = strsep(&targetnat, ">");

			if (!targetweight)
				targetweight = "5";
			if (!targetnat)
				targetweight = "1";
			if (!ipvsname || !targetport || !targetip || !targetweight || !targetnat)
				break;

			sprintf(ipvs_name, "target_ipvsname%d", count);
			websWrite(wp, "<tr><td>");
			websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			twordlist = nvram_safe_get("ipvs");
			char *matchname = "";
			foreach(tword, twordlist, tnext) {
				char *tempword = tword;
				matchname = strsep(&tempword, ">");
				websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", matchname, !strcmp(matchname, ipvsname) ? "selected=\\\"selected\\\"" : "", matchname);
			}
			websWrite(wp, "//]]>\n</script>\n</select>\n");
			websWrite(wp, "</td>");

			sprintf(ipvs_name, "target_ipvsip%d", count);
			websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"12\" value=\"%s\" /></td>\n", ipvs_name, targetip);
			sprintf(ipvs_name, "target_ipvsport%d", count);
			websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" /></td>\n", ipvs_name, targetport);
			sprintf(ipvs_name, "target_ipvsweight%d", count);
			websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"5\" value=\"%s\" /></td>\n", ipvs_name, targetweight);
			sprintf(ipvs_name, "target_ipvsmasquerade%d", count);
			websWrite(wp, "<td><input type=\"checkbox\" name=\"%s\" value=\"1\" %s/></td>\n", ipvs_name, !strcmp(targetnat, "1") ? "checked=\"checked\"" : "");
			websWrite(wp,
				  "<td><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"ipvstarget_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
				  count);
			count++;
		}
		int i;
		int totalcount = count;

		for (i = count; i < realcount; i++) {

			sprintf(ipvs_name, "target_ipvsname%d", i);
			websWrite(wp, "<tr><td>");
			websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			twordlist = nvram_safe_get("ipvs");
			char *matchname = "";
			foreach(tword, twordlist, tnext) {
				char *tempword = tword;
				matchname = strsep(&tempword, ">");
				websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n", matchname, !strcmp(matchname, "") ? "selected=\\\"selected\\\"" : "", matchname);
			}
			websWrite(wp, "//]]>\n</script>\n</select>\n");
			websWrite(wp, "</td>");

			sprintf(ipvs_name, "target_ipvsip%d", i);
			websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\" size=\"12\" /></td>\n", ipvs_name);
			sprintf(ipvs_name, "target_ipvsport%d", i);
			websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" /></td>\n", ipvs_name);
			sprintf(ipvs_name, "target_ipvsweight%d", i);
			websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" value=\"5\" /></td>\n", ipvs_name);
			sprintf(ipvs_name, "target_ipvsmasquerade%d", i);
			websWrite(wp, "<td><input type=\"checkbox\" name=\"%s\" value=\"1\" checked=\"checked\"/></td>\n", ipvs_name);

			websWrite(wp, "<td>");
			websWrite(wp,
				  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"ipvstarget_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
				  i);
			totalcount++;
		}
		websWrite(wp, "</table>");

		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"ipvstarget_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
		websWrite(wp, "</fieldset>\n");

		char var[32];
		sprintf(var, "%d", totalcount);
		nvram_set("ipvstarget_count", var);
	}
}

void ej_show_ipvs(webs_t wp, int argc, char_t ** argv)
{
	int count = 0;
	char word[256];
	char *next, *wordlist;
	char *ipvsname, *sourceip, *sourceport, *scheduler, *sourceproto;
	char ipvs_name[32];

	char *schedulers[] = { "wrr", "lc", "wlc", "fo", "ovf", "lblc", "lblcr", "dh", "sh", "sed", "nq", NULL };

	int realcount = atoi(nvram_default_get("ipvs_count", "0"));

	websWrite(wp, "<h2>%s</h2>\n", live_translate("networking.ipvs"));
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>%s</legend>\n", live_translate("networking.ipvs_config"));
	showOptionsLabel(wp, "networking.ipvs_role", "ipvsrole", "Master Backup", nvram_default_match("ipvs_role", "master", "master") ? "Master" : "Backup");
	websWrite(wp, "</fieldset>\n");
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend>%s</legend>\n", live_translate("networking.create_ipvs"));

	websWrite(wp, "<table cellspacing=\"5\" summary=\"ipvs\" id=\"ipvs_table\" class=\"table center\"><tr>\n");
	websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_name"));
	websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_sourceip"));
	websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_sourceport"));
	websWrite(wp, "<th>%s</th>\n", live_translate("share.proto"));
	websWrite(wp, "<th>%s</th>\n", live_translate("networking.ipvs_scheduler"));
	websWrite(wp, "<th>&nbsp;</th></tr>\n");

	wordlist = nvram_safe_get("ipvs");

	foreach(word, wordlist, next) {
		sourceip = word;
		ipvsname = strsep(&sourceip, ">");
		sourceport = sourceip;
		sourceip = strsep(&sourceport, ">");
		scheduler = sourceport;
		sourceport = strsep(&scheduler, ">");
		sourceproto = scheduler;
		scheduler = strsep(&sourceproto, ">");
		if (!sourceproto)
			sourceproto = "tcp";
		if (!ipvsname || !sourceport || !sourceip || !scheduler || !sourceproto)
			break;

		sprintf(ipvs_name, "ipvsname%d", count);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\"size=\"3\" value=\"%s\" /></td>\n", ipvs_name, ipvsname);
		sprintf(ipvs_name, "ipvsip%d", count);
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\" size=\"12\" value=\"%s\" /></td>\n", ipvs_name, sourceip);
		sprintf(ipvs_name, "ipvsport%d", count);
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" /></td>\n", ipvs_name, sourceport);
		websWrite(wp, "<td>");
		sprintf(ipvs_name, "ipvsproto%d", count);
		showOptions(wp, ipvs_name, "tcp udp sip", sourceproto);
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

		sprintf(ipvs_name, "ipvsname%d", i);
		websWrite(wp, "<tr><td><input class=\"num\" name=\"%s\"size=\"3\"/></td>\n", ipvs_name);
		sprintf(ipvs_name, "ipvsip%d", i);
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"12\" value=\"\" /></td>\n", ipvs_name);
		sprintf(ipvs_name, "ipvsport%d", i);
		websWrite(wp, "<td align=\"center\"><input class=\"num\" name=\"%s\"size=\"5\" value=\"\" /></td>\n", ipvs_name);
		websWrite(wp, "<td>");
		sprintf(ipvs_name, "ipvsproto%d", count);
		showOptions(wp, ipvs_name, "tcp udp sip", "tcp");
		websWrite(wp, "</td>");
		websWrite(wp, "<td>");
		sprintf(ipvs_name, "ipvsscheduler%d", i);

		websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		int scount = 0;
		while (schedulers[scount] != NULL) {
			char translate[32];
			char *sched = schedulers[scount++];
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
	websWrite(wp, "</fieldset><br />\n");

	char var[32];
	sprintf(var, "%d", totalcount);
	nvram_set("ipvs_count", var);
	ej_show_ipvsassignments(wp, argc, argv);
}
