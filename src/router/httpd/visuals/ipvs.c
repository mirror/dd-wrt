/*
 * ipvs.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
EJ_VISIBLE void ej_show_ipvsassignments(webs_t wp, int argc, char_t **argv)
{
	int count = 0;
	char word[256];
	char *next, *wordlist;

	char tword[256];
	char *tnext, *twordlist;
	char *ipvsname, *targetip, *targetport, *targetweight, *targetnat;
	char ipvs_name[32];
	char buf[128];

	if (*(nvram_safe_get("ipvs"))) {
		int realcount = nvram_default_geti("ipvstarget_count", 0);
		websWrite(wp, "<fieldset>\n");
		show_caption_pp(wp, NULL, "networking.ipvs_targets", "<legend>",
				"</legend>\n");

		websWrite(
			wp,
			"<table cellspacing=\"5\" summary=\"ipvstargets\" id=\"ipvstarget_table\" class=\"table\"><tr>\n");
		show_caption_pp(wp, NULL, "networking.ipvs_name", "<th>",
				"</th>\n");
		show_caption_pp(wp, NULL, "networking.ipvs_targetip", "<th>",
				"</th>\n");
		show_caption_pp(wp, NULL, "networking.ipvs_targetport", "<th>",
				"</th>\n");
		show_caption_pp(wp, NULL, "networking.ipvs_weight", "<th>",
				"</th>\n");
		show_caption_pp(wp, NULL, "wl_basic.masquerade", "<th>",
				"</th>\n");
		show_caption_pp(wp, NULL, "share.actiontbl",
				"<th class=\"center\" width=\"10%%\">",
				"</th>\n");

		wordlist = nvram_safe_get("ipvstarget");

		foreach(word, wordlist, next)
		{
			GETENTRYBYIDX(ipvsname, word, 0);
			GETENTRYBYIDX(targetip, word, 1);
			GETENTRYBYIDX(targetport, word, 2);
			GETENTRYBYIDX(targetweight, word, 3);
			GETENTRYBYIDX(targetnat, word, 4);

			if (!targetweight)
				targetweight = "5";
			if (!targetnat)
				targetweight = "1";
			if (!ipvsname || !targetport || !targetip ||
			    !targetweight || !targetnat)
				break;

			sprintf(ipvs_name, "target_ipvsname%d", count);
			websWrite(wp, "<tr><td>");
			websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			twordlist = nvram_safe_get("ipvs");
			char *matchname = "";
			foreach(tword, twordlist, tnext)
			{
				GETENTRYBYIDX(matchname, tword, 0);
				websWrite(
					wp,
					"document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n",
					matchname ? matchname : "",
					(matchname &&
					 !strcmp(matchname, ipvsname)) ?
						"selected=\\\"selected\\\"" :
						"",
					matchname);
			}
			websWrite(wp, "//]]>\n</script>\n</select>\n");
			websWrite(wp, "</td>");

			sprintf(ipvs_name, "target_ipvsip%d", count);
			websWrite(
				wp,
				"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"12\" value=\"%s\" /></td>\n",
				ipvs_name, targetip);
			sprintf(ipvs_name, "target_ipvsport%d", count);
			websWrite(
				wp,
				"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" /></td>\n",
				ipvs_name, targetport);
			sprintf(ipvs_name, "target_ipvsweight%d", count);
			websWrite(
				wp,
				"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" /></td>\n",
				ipvs_name, targetweight);
			sprintf(ipvs_name, "target_ipvsmasquerade%d", count);
			websWrite(
				wp,
				"<td><input type=\"checkbox\" name=\"%s\" value=\"1\" %s/></td>\n",
				ipvs_name,
				!strcmp(targetnat, "1") ?
					"checked=\"checked\"" :
					"");
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"ipvstarget_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
				count);
			count++;
		}
		int i;
		int totalcount = count;

		for (i = count; i < realcount; i++) {
			sprintf(ipvs_name, "target_ipvsname%d", i);
			websWrite(wp, "<tr><td>");
			websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			twordlist = nvram_safe_get("ipvs");
			char *matchname = "";
			foreach(tword, twordlist, tnext)
			{
				GETENTRYBYIDX(matchname, tword, 0);
				websWrite(
					wp,
					"document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n",
					matchname ? matchname : "",
					(matchname && !strcmp(matchname, "")) ?
						"selected=\\\"selected\\\"" :
						"",
					matchname);
			}
			websWrite(wp, "//]]>\n</script>\n</select>\n");
			websWrite(wp, "</td>");

			sprintf(ipvs_name, "target_ipvsip%d", i);
			websWrite(
				wp,
				"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"12\" /></td>\n",
				ipvs_name);
			sprintf(ipvs_name, "target_ipvsport%d", i);
			websWrite(
				wp,
				"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" /></td>\n",
				ipvs_name);
			sprintf(ipvs_name, "target_ipvsweight%d", i);
			websWrite(
				wp,
				"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" value=\"5\" /></td>\n",
				ipvs_name);
			sprintf(ipvs_name, "target_ipvsmasquerade%d", i);
			websWrite(
				wp,
				"<td><input type=\"checkbox\" name=\"%s\" value=\"1\" checked=\"checked\"/></td>\n",
				ipvs_name);

			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"ipvstarget_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
				i);
			totalcount++;
		}
		websWrite(wp, "<tr>");
		websWrite(wp, "<td colspan=\"5\"></td>\n");
		websWrite(wp, "<td class=\"center\">\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"ipvstarget_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
		websWrite(wp, "</td>\n");
		websWrite(wp, "</tr>");
		websWrite(wp, "</table>");
		websWrite(wp, "</fieldset><br/>\n");

		char var[32];
		sprintf(var, "%d", totalcount);
		nvram_set("ipvstarget_count", var);
	}
}

EJ_VISIBLE void ej_show_ipvs(webs_t wp, int argc, char_t **argv)
{
	int count = 0;
	char word[256];
	char *next, *wordlist;
	char *ipvsname, *sourceip, *sourceport, *scheduler, *sourceproto;
	char ipvs_name[32];
	char buf[128];
	char *schedulers[] = { "dh",  "fo",  "lblc", "lblcr", "lc",  "mh", "nq",
			       "ovf", "sed", "sh",   "wlc",   "wrr", NULL };

	int realcount = nvram_default_geti("ipvs_count", 0);

	show_caption_pp(wp, NULL, "networking.ipvs", "<h2>", "</h2>\n");
	websWrite(wp, "<fieldset>\n");
	show_caption_pp(wp, NULL, "networking.ipvs_config", "<legend>",
			"</legend>\n");
	showOptionsLabel(wp, "networking.ipvs_role", "ipvsrole",
			 "Master Backup",
			 nvram_default_match("ipvs_role", "master", "master") ?
				 "Master" :
				 "Backup");
	websWrite(wp, "</fieldset><br/>\n");
	websWrite(wp, "<fieldset>\n");
	show_caption_pp(wp, NULL, "networking.create_ipvs", "<legend>",
			"</legend>\n");

	websWrite(
		wp,
		"<table cellspacing=\"5\" summary=\"ipvs\" id=\"ipvs_table\" class=\"table\"><thead><tr>\n");
	show_caption_pp(wp, NULL, "networking.ipvs_name", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.ipvs_sourceip", "<th>",
			"</th>\n");
	show_caption_pp(wp, NULL, "networking.ipvs_sourceport", "<th>",
			"</th>\n");
	show_caption_pp(wp, NULL, "share.proto", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "networking.ipvs_scheduler", "<th>",
			"</th>\n");
	show_caption_pp(wp, NULL, "share.actiontbl",
			"<th class=\"center\" width=\"10%%\">",
			"</th></thead><tbody>\n");

	wordlist = nvram_safe_get("ipvs");

	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(ipvsname, word, 0);
		GETENTRYBYIDX(sourceip, word, 1);
		GETENTRYBYIDX(sourceport, word, 2);
		GETENTRYBYIDX(scheduler, word, 3);
		GETENTRYBYIDX(sourceproto, word, 4);
		if (!sourceproto)
			sourceproto = "tcp";
		if (!ipvsname || !sourceport || !sourceip || !scheduler ||
		    !sourceproto)
			break;

		sprintf(ipvs_name, "ipvsname%d", count);
		websWrite(
			wp,
			"<tr><td><input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" /></td>\n",
			ipvs_name, ipvsname);
		sprintf(ipvs_name, "ipvsip%d", count);
		websWrite(
			wp,
			"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"12\" value=\"%s\" /></td>\n",
			ipvs_name, sourceip);
		sprintf(ipvs_name, "ipvsport%d", count);
		websWrite(
			wp,
			"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" /></td>\n",
			ipvs_name, sourceport);
		websWrite(wp, "<td>");
		sprintf(ipvs_name, "ipvsproto%d", count);
		showOptions(wp, ipvs_name, "tcp udp sip", sourceproto);
		websWrite(wp, "</td>");

		websWrite(wp, "<td>");

		sprintf(ipvs_name, "ipvsscheduler%d", count);

		websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n");
		int scount = 0;
		while (schedulers[scount] != NULL) {
			char translate[32];
			char *sched = schedulers[scount++];
			sprintf(translate, "networking.%s", sched);
			websWrite(
				wp,
				"document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n",
				sched,
				!strcmp(sched, scheduler) ?
					"selected=\\\"selected\\\"" :
					"",
				tran_string(buf, sizeof(buf), translate));
		}
		websWrite(wp, "//]]>\n</script>\n</select>\n");
		websWrite(wp, "</td>");

		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"ipvs_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
			count);
		count++;
	}
	int i;
	int totalcount = count;

	for (i = count; i < realcount; i++) {
		sprintf(ipvs_name, "ipvsname%d", i);
		websWrite(
			wp,
			"<tr><td><input class=\"num\" name=\"%s\" size=\"3\"/></td>\n",
			ipvs_name);
		sprintf(ipvs_name, "ipvsip%d", i);
		websWrite(
			wp,
			"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"12\" value=\"\" /></td>\n",
			ipvs_name);
		sprintf(ipvs_name, "ipvsport%d", i);
		websWrite(
			wp,
			"<td class=\"center\"><input class=\"num\" name=\"%s\" size=\"5\" value=\"\" /></td>\n",
			ipvs_name);
		websWrite(wp, "<td>");
		sprintf(ipvs_name, "ipvsproto%d", count);
		showOptions(wp, ipvs_name, "tcp udp sip", "tcp");
		websWrite(wp, "</td>");
		websWrite(wp, "<td>");
		sprintf(ipvs_name, "ipvsscheduler%d", i);

		websWrite(wp, "<select name=\"%s\">\n", ipvs_name);
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n");
		int scount = 0;
		while (schedulers[scount] != NULL) {
			char translate[32];
			char *sched = schedulers[scount++];
			sprintf(translate, "networking.%s", sched);
			websWrite(
				wp,
				"document.write(\"<option value=\\\"%s\\\" %s >%s</option>\");\n",
				sched,
				!strcmp(sched, "wrr") ?
					"selected=\\\"selected\\\"" :
					"",
				tran_string(buf, sizeof(buf), translate));
		}
		websWrite(wp, "//]]>\n</script>\n</select>\n");
		websWrite(wp, "</td>");

		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"ipvs_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td></tr>\n",
			i);
		totalcount++;
	}

	websWrite(wp, "<tr>");
	websWrite(wp, "<td colspan=\"5\"></td>\n");
	websWrite(wp, "<td class=\"center\">\n");
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"ipvs_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</td>\n");
	websWrite(wp, "</tr>");
	websWrite(wp, "</tbody></table>\n");
	websWrite(wp, "</fieldset><br/>\n");

	char var[32];
	sprintf(var, "%d", totalcount);
	nvram_set("ipvs_count", var);
	ej_show_ipvsassignments(wp, argc, argv);
}
