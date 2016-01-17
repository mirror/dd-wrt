void ej_show_mdhcp(webs_t wp, int argc, char_t ** argv)
{
	char buffer[256];
	int count = 0;
	char word[256];
	char *next, *wordlist;

	websWrite(wp, "<h2>%s</h2>\n<fieldset>\n", live_translate("networking.h5"));
	websWrite(wp, "<legend>%s</legend>\n", live_translate("networking.legend5"));

	memset(buffer, 0, 256);
	getIfList(buffer, NULL);
	int totalcount = 0;
	int realcount = atoi(nvram_default_get("mdhcpd_count", "0"));

	wordlist = nvram_safe_get("mdhcpd");
	foreach(word, wordlist, next) {
		char *interface = word;
		char *dhcpon = interface;

		interface = strsep(&dhcpon, ">");
		char *start = dhcpon;

		dhcpon = strsep(&start, ">");
		char *max = start;

		start = strsep(&max, ">");
		char *leasetime = max;

		max = strsep(&leasetime, ">");
		if (max == NULL) {
			max = leasetime;
			leasetime = "3660";
		}
		if (!interface || !start || !dhcpon || !max || !leasetime)
			break;
		char vlan_name[32];

		// interface
		char *ipaddr = nvram_nget("%s_ipaddr", interface);
		char *netmask = nvram_nget("%s_netmask", interface);

		if (strlen(ipaddr) > 0 && strlen(netmask) > 0) {
			websWrite(wp, "<script type=\"text/javascript\">Capture(networking.iface);</script> %s: IP %s/%s\n", getNetworkLabel(interface), ipaddr, netmask);
		}
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "DHCP %d &nbsp;\n", count);
		sprintf(vlan_name, "mdhcpifname%d", count);
		showIfOptions(wp, vlan_name, buffer, interface);
		// on off
		sprintf(vlan_name, "mdhcpon%d", count);
		showOptions(wp, vlan_name, "On Off", dhcpon);
		// start
		sprintf(vlan_name, "mdhcpstart%d", count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(share.start);</script>&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, start);
		// max
		sprintf(vlan_name, "mdhcpmax%d", count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.max);</script>&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, max);
		sprintf(vlan_name, "mdhcpleasetime%d", count);
		websWrite(wp, "&nbsp;<script type=\"text/javascript\">Capture(networking.leasetime);</script>&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n", vlan_name, leasetime);
		// 
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"mdhcp_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  count);
		websWrite(wp, "</div>\n");
		count++;
	}
	totalcount = count;
	int i;

	for (i = count; i < realcount; i++) {
		char vlan_name[32];

		// sprintf (mdhcp_name, "%s.%s", tag, port);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "DHCP %d\n", totalcount);
		// interface
		sprintf(vlan_name, "mdhcpifname%d", totalcount);
		showIfOptions(wp, vlan_name, buffer, "");
		// on off
		sprintf(vlan_name, "mdhcpon%d", totalcount);
		showOptions(wp, vlan_name, "On Off", "");
		// start
		sprintf(vlan_name, "mdhcpstart%d", totalcount);
		websWrite(wp, "&nbsp;Start&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, "100");
		// max
		sprintf(vlan_name, "mdhcpmax%d", totalcount);
		websWrite(wp, "&nbsp;Max&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"3\" value=\"%s\" />\n", vlan_name, "50");
		sprintf(vlan_name, "mdhcpleasetime%d", totalcount);
		websWrite(wp, "&nbsp;Leasetime&nbsp;");
		websWrite(wp, "<input class=\"num\" name=\"%s\" size=\"5\" value=\"%s\" />\n", vlan_name, "3600");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"mdhcp_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</div>\n");
		totalcount++;
	}
	char var[32];

	sprintf(var, "%d", totalcount);
	nvram_set("mdhcpd_count", var);
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"mdhcp_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</fieldset><br />\n");

}
