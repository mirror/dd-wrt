#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <broadcom.h>
#include <cymac.h>

void ej_show_index_setting(webs_t wp, int argc, char_t ** argv)
{
	char *type;

	type = GOZILA_GET(wp, "wan_proto");
	if (type == NULL)
		type = nvram_safe_get("wan_proto");
	char ejname[32];
	snprintf(ejname,31,"index_%s.asp",type);
	do_ej(NULL, ejname, wp, NULL);
}

char *nvram_selget(webs_t wp, char *name)
{
	if (nvram_match("gozila_action", "1")) {
	char *buf = GOZILA_GET(wp, name);
	
		if (buf) {
			return buf;
		}
	}
	return nvram_safe_get(name);
}

void show_3g_settings(webs_t wp, int iface) {
	char name[64];
	char number[2];
	int i, j, multisim;

	sprintf( name, "3g_ms_%i", iface);
	multisim = atoi( nvram_selget( wp, name ) );
	if( multisim == 0 ) {
		multisim = 1;
	}

	websWrite( wp, "      <div class=\"setting\"><strong>IMEI: %s</strong><div>\n", nvram_get("3g_imei_1") );
	websWrite( wp, "      <div class=\"setting\">\n" );
	websWrite( wp, "           <div class=\"label\">Multisim</div>\n" );
	websWrite( wp, "                <input type=\"checkbox\" name=\"3g_ms_%i_select\">\n", iface );
	websWrite( wp, "                <input type=\"text\" name=\"3g_ms_%i\" value=\"%i\" size=\"2\">\n", iface, multisim );
	websWrite( wp, "      </div>\n" );

	for( i = 1; i < multisim + 1; i++ ) {
		websWrite( wp, "    <div class=\"setting\"><i>SIM-Card %i</i></div>\n", i ); 
		
		if( multisim > 1 ) {
			sprintf( name, "3g_prio_%i_%i", iface, i);
			websWrite( wp, "      <div class=\"setting\">\n");
			websWrite( wp, "         <div class=\"label\">Dial Priority</div>\n" );
			websWrite( wp, "         <select name=\"3g_prio_%i_%i\">\n", iface, i );

			websWrite( wp, "           <option value=\"0\" %s>disabled</option>\n", nvram_match( name,
											"0") ? "selected=\\\"selected\\\"" :
													"");
			for( j = 1; j <= multisim; j++ ) {
				sprintf( number, "%i", j );
				websWrite( wp, "           <option value=\"%i\" %s>%i</option>\n", j, nvram_match( name,
												number ) ? "selected=\\\"selected\\\"" :
												"", j );
			}
		}
		websWrite( wp, "         </select>\n" );
		websWrite( wp, "      </div>\n" );
		websWrite( wp, "   </div>\n" );
	
		sprintf(name, "3g_uname_%i_%i", iface, i);
		websWrite( wp, "    <div class=\"setting\">\n" );
		websWrite( wp, "      <div class=\"label\"><script type=\"text/javascript\">Capture(share.usrname);</script></div>\n" );
		websWrite( wp, "      <input name=\"3g_uname_%i_%i\" size=\"40\" maxlength=\"63\" onblur=\"valid_name(this,share.usrname);\" value=\"%s\" />", iface, i, nvram_selget(wp, name) );
		websWrite( wp, "    </div>" );

		sprintf(name, "3g_pw_%i_%i", iface, i);
		websWrite( wp, "    <div class=\"setting\">\n" );
		websWrite( wp, "      <div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd);</script></div>" );
		websWrite( wp, "      <input id=\"3g_pw_%i_%i\" name=\"3g_pw_%i_%i\" size=\"40\" maxlength=\"63\" onblur=\"valid_name(this,share.passwd)\" type=\"password\" value=\"%s\" />&nbsp;&nbsp;&nbsp", iface, i, iface, i, nvram_selget( wp, name ) );
		websWrite( wp, "<input type=\"checkbox\" name=\"_3g_pw_%i_%i_unmask\" value=\"0\" onclick=\"setElementMask('3g_pw_%i_%i', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask);</script></input>", iface, i, iface, i );
		websWrite( wp, "    </div>\n" );

		sprintf(name, "3g_ds_%i_%i", iface, i);
		websWrite( wp, "    <div class=\"setting\">\n" );
		websWrite( wp, "      <div class=\"label\"><script type=\"text/javascript\">Capture(share.dial);</script></div>\n" );
		websWrite( wp, "	    <select name=\"3g_ds_%i_%i\">\n", iface, i );
		websWrite( wp, "          <option value=\"0\" %s >*99***1# (3G/UMTS)</option>\n", nvram_match(name,
					                      "0") ? "selected=\\\"selected\\\"" :
										          "");
		websWrite( wp, "          <option value=\"1\" %s >*99# (3G/UMTS)</option>\n", nvram_match(name,
					                      "1") ? "selected=\\\"selected\\\"" :
										          "");
		websWrite( wp, "          <option value=\"2\" %s >#777 (CDMA/EVDO)</option>\n", nvram_match(name,
					                      "2") ? "selected=\\\"selected\\\"" :
										          "");
		websWrite( wp, "  	    </select>\n" );
		websWrite( wp, "      </div>\n" );

		sprintf(name, "3g_apn_%i_%i", iface, i);
		websWrite( wp, "      <div class=\"setting\">\n" );
		websWrite( wp, "        <div class=\"label\"><script type=\"text/javascript\">Capture(share.apn);</script></div>\n" );
		websWrite( wp, "        <input name=\"3g_apn_%i_%i\" size=\"40\" maxlength=\"63\" onblur=\"valid_name(this,share.apn)\" value=\"%s\" />\n", iface, i, nvram_selget( wp, name ) );
		websWrite( wp, "      </div>\n" );

		sprintf(name, "3g_pin_%i_%i", iface, i);
		websWrite( wp, "      <div class=\"setting\">\n" );
		websWrite( wp, "        <div class=\"label\"><script type=\"text/javascript\">Capture(share.pin);</script></div>\n" );
		websWrite( wp, "        <input id=\"3g_pin_%i_%i\" name=\"3g_pin_%i_%i\" size=\"4\" maxlength=\"4\" onblur=\"valid_name(this,share.pin)\" type=\"password\" value=\"%s\" />&nbsp;&nbsp;&nbsp;\n", iface, i, iface, i, nvram_selget( wp, name ) ); 
		websWrite( wp, "        <input type=\"checkbox\" name=\"_3g_pin_%i_%i_unmask\" value=\"0\" onclick=\"setElementMask('3g_pin_%i_%i', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask);</script></input>\n", iface, i, iface, i);
		websWrite( wp, "      </div>\n" );
	}
}

void ej_show_3g_settings(webs_t wp, int argc, char_t ** argv) {
	int i, ifaces;

	ifaces = atoi(nvram_safe_get("3g_imei_cnt"));
	if( ifaces == 0) {
		websWrite(wp, "<p>No 3G Interfaces available</p>");
		return;
	}
	for( i = 1; i <= ifaces; i++) {
		show_3g_settings( wp, i );
	}
}

void ej_get_wl_max_channel(webs_t wp, int argc, char_t ** argv)
{

	websWrite(wp, "%s", WL_MAX_CHANNEL);
}

void ej_get_wl_domain(webs_t wp, int argc, char_t ** argv)
{

#if COUNTRY == EUROPE
	websWrite(wp, "ETSI");
#elif COUNTRY == JAPAN
	websWrite(wp, "JP");
#else
	websWrite(wp, "US");
#endif
}

void ej_get_clone_mac(webs_t wp, int argc, char_t ** argv)
{
	char *c;
	int mac, which;
	int dofree = 0;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d", &which);
#else
	if (ejArgs(argc, argv, "%d", &which) < 1) {
		websError(wp, 400, "Insufficient args\n");
	}
#endif

	if (nvram_match("clone_wan_mac", "1"))
		c = nvram_safe_get("http_client_mac");
	else {
		if (nvram_match("def_hwaddr", "00:00:00:00:00:00")) {
			if (nvram_match("port_swap", "1"))
				c = strdup(nvram_safe_get("et1macaddr"));
			else
				c = strdup(nvram_safe_get("et0macaddr"));
			if (c) {
				MAC_ADD(c);
				dofree = 1;
			}
		} else
			c = nvram_safe_get("def_hwaddr");
	}

	if (c) {
		mac = get_single_mac(c, which);
		websWrite(wp, "%02X", mac);
		if (dofree)
			free(c);
	} else
		websWrite(wp, "00");
}

void macclone_onload(webs_t wp, char *arg)
{

	if (nvram_match("clone_wan_mac", "1"))
		websWrite(wp, arg);

	return;
}
