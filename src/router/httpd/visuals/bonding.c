void ej_show_bondings(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	char bufferif[512];
	char bondnames[256];
	int count = 0;
	static char word[256];
	char *next, *wordlist;

	memset(buffer, 0, 256);
	memset(bondnames, 0, 256);
	memset(bufferif, 0, 512);
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(networking.bonding)</script></h2>\n");
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<legend><script type=\"text/javascript\">Capture(networking.bonding)</script></legend>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(networking.bondtype)</script></div>\n", count);
	showOptions(wp, "bonding_type", "balance-rr active-backup balance-xor broadcast 802.3ad balance-tlb balance-alb weighted-rr duplex", nvram_default_get("bonding_type", "balance-rr"));
	websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.bondifaces)</script>&nbsp;");
	websWrite(wp, "<input class=\"num\" name=\"bonding_number\"size=\"5\" value=\"%s\" />\n", nvram_default_get("bonding_number", "1"));
	websWrite(wp, "</div>\n");

	getIfList(bufferif, "eth");
	int i;

#ifdef HAVE_XSCALE
	memset(buffer, 0, 256);
	getIfList(buffer, "ixp");
	sprintf(bufferif, "%s %s", bufferif, buffer);
#endif
	memset(buffer, 0, 256);
	getIfList(buffer, "br");
	sprintf(bufferif, "%s %s", bufferif, buffer);

	memset(buffer, 0, 256);
	getIfList(buffer, "vlan");
	sprintf(bufferif, "%s %s", bufferif, buffer);
#ifdef HAVE_MADWIFI
	int c = getdevicecount();

	for (i = 0; i < c; i++) {
		char ath[32];

		sprintf(ath, "ath%d_bridged", i);
		if (nvram_default_match(ath, "0", "1")) {
			sprintf(bufferif, "%s ath%d", bufferif, i);
			char vifs[32];

			sprintf(vifs, "ath%d_vifs", i);
			sprintf(bufferif, "%s %s", bufferif, nvram_safe_get(vifs));
		}
	}
#endif

	for (i = 0; i < atoi(nvram_safe_get("bonding_number")); i++) {
		sprintf(bondnames, "%s bond%d", bondnames, i);
	}
	int totalcount = 0;
	int realcount = atoi(nvram_default_get("bonding_count", "0"));

	wordlist = nvram_safe_get("bondings");
	foreach(word, wordlist, next) {
		char *port = word;
		char *tag = strsep(&port, ">");

		if (!tag || !port)
			break;
		char vlan_name[32];

		// sprintf (vlan_name, "%s.%s", tag, port);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(networking.bonding) + Capture(\" %d \") + Capture(networking.assign)</script></div>\n", count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.bond)</script>&nbsp;");
		sprintf(vlan_name, "bondingifname%d", count);
		showOptions(wp, vlan_name, bondnames, tag);
		sprintf(vlan_name, "bondingattach%d", count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.slave)</script>&nbsp;");
		showIfOptions(wp, vlan_name, bufferif, port);
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bond_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	totalcount = count;
	for (i = count; i < realcount; i++) {
		char vlan_name[32];

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(networking.bonding) + Capture(\" %d \")  + Capture(networking.iface)</script></div>\n", i);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.bond)</script>&nbsp;");
		sprintf(vlan_name, "bondingifname%d", i);
		showOptions(wp, vlan_name, bondnames, "");
		sprintf(vlan_name, "bondingattach%d", i);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.slave)</script>&nbsp;");
		showIfOptions(wp, vlan_name, bufferif, "");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"bond_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("bonding_count", var);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"bond_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</fieldset><br />\n");
}


