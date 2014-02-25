
/*
 * Broadcom Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: broadcom.c,v 1.9 2005/11/30 11:53:42 seg Exp $
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else				/* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif				/* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <cyutils.h>
#include <support.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <dlfcn.h>

int debug_value = 0;

// static char * rfctime(const time_t *timep);
// static char * reltime(unsigned int seconds);

// #if defined(linux)

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#define sys_stats(url) eval("stats", (url))

// tofu

char *live_translate(char *tran);
#ifdef HAVE_BUFFALO
void do_vsp_page(struct mime_handler *handler, char *url, webs_t stream, char *query);
#endif
/*
 * Deal with side effects before committing 
 */
int sys_commit(void)
{
	if (nvram_match("dhcpnvram", "1")) {
		killall("dnsmasq", SIGUSR2);	// update lease -- tofu
		sleep(1);
	}
	// if (nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto",
	// "pptp") )
	// nvram_set("wan_ifname", "ppp0");
	// else
	// nvram_set("wan_ifname", nvram_get("pppoe_ifname"));
	return nvram_commit();
}

/*
 * Variables are set in order (put dependent variables later). Set
 * nullok to TRUE to ignore zero-length values of the variable itself.
 * For more complicated validation that cannot be done in one pass or
 * depends on additional form components or can throw an error in a
 * unique painful way, write your own validation routine and assign it
 * to a hidden variable (e.g. filter_ip).
 */
/*
 * DD-WRT enhancement by seg This functions parses all
 * /etc/config/xxxxx.nvramconfig files and creates the web var tab. so these
 * vars arent defined anymore staticly 
 */

#include <stdlib.h>
#include <malloc.h>
#include <dirent.h>
#include <stdlib.h>

char *toUP(char *a)
{
	int i;
	int slen = strlen(a);

	for (i = 0; i < slen; i++) {
		if (a[i] > 'a' - 1 && a[i] < 'z' + 1)
			a[i] -= 'a' + 'A';
	}
	return a;
}

int stricmp(char *a, char *b)
{
	if (strlen(a) != strlen(b))
		return -1;
	return strcmp(toUP(a), toUP(b));
}

void StringStart(FILE * in)
{
	while (getc(in) != '"') {
		if (feof(in))
			return;
	}
}

char *getFileString(FILE * in)
{
	char *buf;
	int i, b;

	buf = safe_malloc(1024);
	StringStart(in);
	for (i = 0; i < 1024; i++) {
		b = getc(in);
		if (b == EOF)
			return NULL;
		if (b == '"') {
			buf[i] = 0;
			buf = realloc(buf, strlen(buf) + 1);
			return buf;
		}
		buf[i] = b;
	}
	return buf;
}

void skipFileString(FILE * in)
{
	int i, b;

	StringStart(in);
	for (i = 0; i < 1024; i++) {
		b = getc(in);
		if (b == EOF)
			return;
		if (b == '"') {
			return;
		}
	}
	return;
}

static char *directories[] = {
	"/etc/config",
	"/jffs/etc/config",
	"/mmc/etc/config"
};

struct SIMPLEVAL {
	char *name;
	char *validator;
	int args;
};

struct variable **variables;
void Initnvramtab()
{
	struct dirent *entry;
	DIR *directory;
	FILE *in;
	int varcount = 0, len, i;
	char *tmpstr;
	struct variable *tmp;
	static struct SIMPLEVAL simpleval[] = {
		{"WMEPARAM", "validate_wl_wme_params", 0},
		{"WMETXPARAM", "validate_wl_wme_tx_params", 0},
		{"WANIPADDR", "validate_wan_ipaddr", 0},
		{"MERGEREMOTEIP", "validate_remote_ip", 0},
		{"MERGEIPADDRS", "validate_merge_ipaddrs", 0},
		{"DNS", "validate_dns", 0},
		{"SAVEWDS", "save_wds", 0},
		{"DHCP", "dhcp_check", 0},
		{"STATICS", "validate_statics", 0},
#ifdef HAVE_PORTSETUP
		{"PORTSETUP", "validate_portsetup", 0},
#endif
		{"REBOOT", "validate_reboot", 0},
		{"IPADDR", "validate_ipaddr", 0},
		{"STATICLEASES", "validate_staticleases", 0},
#ifdef HAVE_CHILLILOCAL
		{"USERLIST", "validate_userlist", 0},
#endif
#ifdef HAVE_RADLOCAL
		{"IRADIUSUSERLIST", "validate_iradius", 0},
#endif
		{"IPADDRS", "validate_ipaddrs", 0},
		{"NETMASK", "validate_netmask", 0},
		{"MERGENETMASK", "validate_merge_netmask", 0},
		{"WDS", "validate_wds", 0},
		{"STATICROUTE", "validate_static_route", 0},
		{"MERGEMAC", "validate_merge_mac", 0},
		{"FILTERPOLICY", "validate_filter_policy", 0},
		{"FILTERIPGRP", "validate_filter_ip_grp", 0},
		{"FILTERPORT", "validate_filter_port", 0},
		{"FILTERDPORTGRP", "validate_filter_dport_grp", 0},
		{"BLOCKEDSERVICE", "validate_blocked_service", 0},
		{"FILTERP2P", "validate_catchall", 0},
		{"FILTERMACGRP", "validate_filter_mac_grp", 0},
		{"FILTERWEB", "validate_filter_web", 0},
		{"WLHWADDRS", "validate_wl_hwaddrs", 0},
		{"FORWARDPROTO", "validate_forward_proto", 0},
		{"FORWARDSPEC", "validate_forward_spec", 0},
		{"PORTTRIGGER", "validate_port_trigger", 0},
		{"HWADDR", "validate_hwaddr", 0},
		{"HWADDRS", "validate_hwaddrs", 0},
		{"WLWEPKEY", "validate_wl_wep_key", 0},
#ifdef HAVE_PPPOESERVER
		{"CHAPTABLE", "validate_chaps", 0},
#endif
#ifdef HAVE_MILKFISH
		{"MFSUBSCRIBERS", "validate_subscribers", 0},
		{"MFALIASES", "validate_aliases", 0},
#endif
		{"RANGE", "validate_range", 2},
		{"CHOICE", "validate_choice", -1},
		{"NOACK", "validate_noack", 2},
		{"NAME", "validate_name", 1},
		{"PASSWORD", "validate_password", 1},
		{"PASSWORD2", "validate_password2", 1},
		{"LANIPADDR", "validate_lan_ipaddr", 1},
		{"WPAPSK", "validate_wpa_psk", 1},
		{"WLAUTH", "validate_wl_auth", 2},
		{"WLWEP", "validate_wl_wep", -1},
		{"DYNAMICROUTE", "validate_dynamic_route", -1},
		{"WLGMODE", "validate_wl_gmode", -1},
		{"WLNETMODE", "validate_wl_net_mode", -1},
		{"AUTHMODE", "validate_auth_mode", -1},
		{NULL, NULL},
	};

	variables = NULL;
	char buf[1024];

	// format = VARNAME VARDESC VARVALID VARVALIDARGS FLAGS FLAGS
	// open config directory directory =
	int idx;

	for (idx = 0; idx < 3; idx++) {
		directory = opendir(directories[idx]);
		if (directory == NULL)
			continue;
		// list all files in this directory
		while ((entry = readdir(directory)) != NULL) {
			if (endswith(entry->d_name, ".nvramconfig")) {
				sprintf(buf, "%s/%s", directories[idx], entry->d_name);
				in = fopen(buf, "rb");
				if (in == NULL) {
					return;
				}
				while (1) {
					tmp = (struct variable *)
					    safe_malloc(sizeof(struct variable));
					memset(tmp, 0, sizeof(struct variable));
					tmp->name = getFileString(in);
					if (tmp->name == NULL)
						break;
					skipFileString(in);	// long string
					tmpstr = getFileString(in);
					tmp->argv = NULL;
					if (!stricmp(tmpstr, "NULL")) {
					}
#ifdef HAVE_SPUTNIK_APD
					if (!stricmp(tmpstr, "MJIDTYPE")) {
						tmp->validatename = "validate_choice";
						free(tmpstr);
						tmpstr = getFileString(in);
						len = atoi(tmpstr);
						tmp->argv = (char **)
						    safe_malloc(sizeof(char **)
								* (len + 1));
						for (i = 0; i < len; i++) {
							tmp->argv[i] = getFileString(in);
						}
						tmp->argv[i] = NULL;
						nvram_set("sputnik_rereg", "1");
					}
#endif
					if (tmp->validatename == NULL) {
						int scount = 0;
						while (simpleval[scount].name != NULL) {	//
							if (!stricmp(tmpstr, simpleval[scount].name)) {	//
//                                                              fprintf(stderr,"match %s %s\n",tmpstr,tmp->name);
								tmp->validatename = simpleval[scount].validator;	//
								int arglen = 0;
								if (simpleval[scount].args == -1) {	//
									free(tmpstr);
									tmpstr = getFileString(in);	//
									arglen = atoi(tmpstr);	//
								}
								if (simpleval[scount].args > 0) {	//
									arglen = simpleval[scount].args;	//
								}
								if (arglen) {	//
									tmp->argv = (char **)safe_malloc(sizeof(char **) * (arglen + 1));	//
									for (i = 0; i < arglen; i++) {	//
										tmp->argv[i] = getFileString(in);	//
									}
									tmp->argv[arglen] = NULL;	//

								}
								break;
							}
							scount++;
						}
//                                              if (simpleval[scount].name ==
//                                                  NULL) {
//                                                      fprintf(stderr,
//                                                              "danger %s is missing\n",
//                                                              tmpstr);
//                                              }
					}
					free(tmpstr);
					tmpstr = getFileString(in);
					if (!stricmp(tmpstr, "TRUE")) {
						tmp->nullok = TRUE;
					} else {
						tmp->nullok = FALSE;
					}
					free(tmpstr);
					skipFileString(in);	// todo: remove it
					// tmpstr = getFileString (in);
					// tmp->ezc_flags = atoi (tmpstr);
					// free (tmpstr);
					variables = (struct variable **)
					    realloc(variables, sizeof(struct variable **) * (varcount + 2));
					variables[varcount++] = tmp;
					variables[varcount] = NULL;
				}
				fclose(in);
			}
		}
		closedir(directory);
	}
}

#ifdef HAVE_MACBIND
#include "../../../opt/mac.h"
#endif
// Added by Daniel(2004-07-29) for EZC
int variables_arraysize(void)
{
	int varcount = 0;

	if (variables == NULL)
		return 0;
	while (variables[varcount] != NULL) {
		varcount++;
	}
	// return ARRAYSIZE(variables);
	return varcount;
}

// and now the tricky part (more dirty as dirty)
void do_filtertable(struct mime_handler *handler, char *path, webs_t stream, char *query)
{
	char *temp2 = &path[indexof(path, '-') + 1];
	char ifname[16];

	strcpy(ifname, temp2);

	char *temp3 = websGetVar(stream, "ifname", NULL);
	if (temp3 != NULL) {
		if (strlen(temp3) > 0) {
			strcpy(ifname, temp3);
		}
	}

	ifname[indexof(ifname, '.')] = 0;
	FILE *web = getWebsFile("WL_FilterTable.asp");
	char temp[4096];

	memset(temp, 0, 4096);
	unsigned int len = getWebsFileLen("WL_FilterTable.asp");
	char *webfile = (char *)safe_malloc(len + 1);

	fread(webfile, len, 1, web);
	webfile[len] = 0;
	rep(ifname, '.', 'X');
	sprintf(temp, webfile, ifname, ifname, ifname, ifname, ifname, ifname);
	free(webfile);
	fclose(web);
	do_ej_buffer(temp, stream);
}

#ifdef HAVE_FREERADIUS
#include <radiusdb.h>

static void cert_file_out(struct mime_handler *handler, char *path, webs_t stream, char *query)
{
	char *temp2 = &path[indexof(path, '/') + 1];
	fprintf(stderr, "down %s\n", temp2);
	char link[128];
	sprintf(link, "/jffs/etc/freeradius/certs/clients/%s", temp2);
	do_file_attach(handler, link, stream, NULL, temp2);
}

static void show_certfield(webs_t wp, char *title, char *file)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div>\n", title);
	websWrite(wp, "<script type=\"text/javascript\">\n");
	websWrite(wp, "//<![CDATA[\n");
	websWrite(wp,
		  "document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"download_button\\\" value=\\\"\" + sbutton.download + \"\\\" onclick=\\\"window.location.href='/freeradius-certs/%s';\\\" />\");\n",
		  file);
	websWrite(wp, "//]]>\n");
	websWrite(wp, "</script>\n");
	websWrite(wp, "</div>\n");

}

void do_radiuscert(struct mime_handler *handler, char *path, webs_t stream, char *query)
{
	char *temp2 = &path[indexof(path, '-') + 1];
	char number[16];
	webs_t wp = stream;
	strcpy(number, temp2);
	number[indexof(number, '.')] = 0;
	int radiusindex = atoi(number);

	if (radiusindex == -1)
		return;
	struct radiusdb *db = loadradiusdb();
	if (db == NULL)		// database empty
		return;
	if (radiusindex >= db->usercount)	// index out of bound
	{
		goto out;
	}
	if (db->users[radiusindex].usersize == 0 || db->users[radiusindex].passwordsize == 0 || strlen(db->users[radiusindex].user) == 0 || strlen(db->users[radiusindex].passwd) == 0) {
		//define username fail
		char *argv[] = { "freeradius.clientcert" };
		call_ej("do_pagehead", NULL, wp, 1, argv);	// thats dirty
		websWrite(wp, "</head>\n"
			  "<body>\n"
			  "<div id=\"main\">\n" "<div id=\"contentsInfo\">\n"
			  "<h2>%s</h2>\n"
			  "Error: please specify a value username and password\n"
			  "<div class=\"submitFooter\">\n"
			  "<script type=\"text/javascript\">\n"
			  "//<![CDATA[\n" "submitFooterButton(0,0,0,0,0,1);\n" "//]]>\n" "</script>\n" "</div>\n" "</div>\n" "</div>\n" "</body>\n" "</html>\n", live_translate("freeradius.clientcert"));
		goto out;
	}
	char filename[128];
	char exec[512];
	int generate = 0;
	sprintf(filename, "/jffs/etc/freeradius/certs/clients/%s-cert.pem", db->users[radiusindex].user);
	if (!f_exists(filename))
		generate = 1;
	sprintf(filename, "/jffs/etc/freeradius/certs/clients/%s-cert.p12", db->users[radiusindex].user);
	if (!f_exists(filename))
		generate = 1;
	sprintf(filename, "/jffs/etc/freeradius/certs/clients/%s-key.pem", db->users[radiusindex].user);
	if (!f_exists(filename))
		generate = 1;
	sprintf(filename, "/jffs/etc/freeradius/certs/clients/%s-req.pem", db->users[radiusindex].user);
	if (!f_exists(filename))
		generate = 1;

	if (generate)		//do not regenerate certificates if they are already created
	{

		char expiration_days[64];
		strcpy(expiration_days, nvram_safe_get("radius_expiration"));
		long expiration = 0;	//never
		if (db->users[radiusindex].expiration) {
			time_t tm;
			time(&tm);
			long curtime = ((tm / 60) / 60) / 24;	//in days
			expiration = db->users[radiusindex].expiration - curtime;
			sprintf(expiration_days, "%ld", expiration);
		}
		//erase line from database
		FILE *fp = fopen("/jffs/etc/freeradius/certs/index.txt", "rb");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			int len = ftell(fp);
			rewind(fp);
			char *serial = safe_malloc(len + 1);
			char *output = safe_malloc(len + 1);
			fread(serial, len, 1, fp);
			fclose(fp);
			//look for existing entry
			char common[128];
			sprintf(common, "CN=%s", db->users[radiusindex].user);
			int i;
			int clen = strlen(common);
			int llen = len - clen;
			int line = -1;
			int oc = 0;
			for (i = 0; i < llen; i++) {
				if (serial[i] == 0xa) {
					if (line == -1)
						line++;
					line++;
				}
				if (!strncmp(&serial[i], common, strlen(common))) {
					//found line
					int lines = 0;
					int ic = 0;
					while (1) {
						if (serial[ic] == 0xa) {
							lines++;
						}
						if (line != lines)
							output[oc++] = serial[ic];
						ic++;
						if (ic == len)
							break;
					}
					break;
				}
			}
			if (oc) {
				fp = fopen("/jffs/etc/freeradius/certs/index.txt", "wb");
				if (fp) {
					fwrite(output, oc, 1, fp);
					fclose(fp);
				}
			}
			free(output);
			free(serial);
		}
		sprintf(exec,
			"cd /jffs/etc/freeradius/certs && ./doclientcert \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			expiration_days,
			nvram_safe_get("radius_country"),
			nvram_safe_get("radius_state"),
			nvram_safe_get("radius_locality"),
			nvram_safe_get("radius_organisation"), nvram_safe_get("radius_email"), db->users[radiusindex].user, db->users[radiusindex].passwd, nvram_safe_get("radius_passphrase"));
		system(exec);
	}
	char *argv[] = {
		"freeradius.clientcert"
	};
	call_ej("do_pagehead", NULL, wp, 1, argv);	// thats dirty
	websWrite(wp, "</head>\n" "<body>\n" "<div id=\"main\">\n" "<div id=\"contentsInfo\">\n" "<h2>%s</h2>\n", live_translate("freeradius.clientcert"));
	sprintf(filename, "%s-cert.pem", db->users[radiusindex].user);
	show_certfield(wp, "Certificate PEM", filename);
	sprintf(filename, "%s-cert.p12", db->users[radiusindex].user);
	show_certfield(wp, "Certificate P12 (Windows)", filename);
	sprintf(filename, "%s-req.pem", db->users[radiusindex].user);
	show_certfield(wp, "Certificate Request", filename);
	sprintf(filename, "%s-key.pem", db->users[radiusindex].user);
	show_certfield(wp, "Private Key PEM", filename);
	websWrite(wp, "<div class=\"submitFooter\">\n"
		  "<script type=\"text/javascript\">\n" "//<![CDATA[\n" "submitFooterButton(0,0,0,0,0,1);\n" "//]]>\n" "</script>\n" "</div>\n" "</div>\n" "</div>\n" "</body>\n" "</html>\n");

	//make certificates
      out:;
	freeradiusdb(db);

}

#endif
void do_activetable(struct mime_handler *handler, char *path, webs_t stream, char *query)
{
	char *temp2 = &path[indexof(path, '-') + 1];
	char ifname[16];

	strcpy(ifname, temp2);

	char *temp3 = websGetVar(stream, "ifname", NULL);
	if (temp3 != NULL) {
		if (strlen(temp3) > 0) {
			strcpy(ifname, temp3);
		}
	}

	ifname[indexof(ifname, '.')] = 0;
	FILE *web = getWebsFile("WL_ActiveTable.asp");
	unsigned int len = getWebsFileLen("WL_ActiveTable.asp");
	char *webfile = (char *)safe_malloc(len + 1);

	fread(webfile, len, 1, web);
	webfile[len] = 0;
	fclose(web);

	char temp[32768];

	memset(temp, 0, 32768);
	int ai = 0;
	int i = 0;
	int weblen = strlen(webfile);

	for (i = 0; i < weblen; i++) {
		if (webfile[i] == '%') {
			i++;
			switch (webfile[i]) {
			case '%':
				temp[ai++] = '%';
				break;
			case 's':
				strcpy(&temp[ai], ifname);
				ai += strlen(ifname);
				break;
			default:
				temp[ai++] = webfile[i];
				break;
			}
		} else
			temp[ai++] = webfile[i];
	}
	free(webfile);
	do_ej_buffer(temp, stream);
}

void do_wds(struct mime_handler *handler, char *path, webs_t stream, char *query)
{
	char *temp2 = &path[indexof(path, '-') + 1];
	char ifname[16];

	strcpy(ifname, temp2);
	ifname[indexof(ifname, '.')] = 0;
	FILE *web = getWebsFile("Wireless_WDS.asp");
	unsigned int len = getWebsFileLen("Wireless_WDS.asp");
	char *webfile = (char *)safe_malloc(len + 1);

	fread(webfile, len, 1, web);
	webfile[len] = 0;
	fclose(web);

	char temp[32768];

	memset(temp, 0, 32768);
	int ai = 0;
	int i = 0;
	int weblen = strlen(webfile);

	for (i = 0; i < weblen; i++) {
		if (webfile[i] == '%') {
			i++;
			switch (webfile[i]) {
			case '%':
				temp[ai++] = '%';
				break;
			case 's':
				strcpy(&temp[ai], ifname);
				ai += strlen(ifname);
				break;
			default:
				temp[ai++] = webfile[i];
				break;
			}
		} else
			temp[ai++] = webfile[i];
	}
	free(webfile);
	do_ej_buffer(temp, stream);
}

void do_wireless_adv(struct mime_handler *handler, char *path, webs_t stream, char *query)
{
	char *temp2 = &path[indexof(path, '-') + 1];
	char ifname[16];

	strcpy(ifname, temp2);
	ifname[indexof(ifname, '.')] = 0;
	FILE *web = getWebsFile("Wireless_Advanced.asp");
	unsigned int len = getWebsFileLen("Wireless_Advanced.asp");
	char *webfile = (char *)safe_malloc(len + 1);

	char index[2];
	substring(strlen(ifname) - 1, strlen(ifname), ifname, index);

	fread(webfile, len, 1, web);
	webfile[len] = 0;
	fclose(web);

	char temp[65536];

	memset(temp, 0, 65536);
	int ai = 0;
	int i = 0;
	int weblen = strlen(webfile);

	for (i = 0; i < weblen; i++) {
		if (webfile[i] == '%') {
			i++;
			switch (webfile[i]) {
			case '%':
				temp[ai++] = '%';
				break;
			case 'd':
				strcpy(&temp[ai], index);
				ai++;
				break;
			case 's':
				strcpy(&temp[ai], ifname);
				ai += strlen(ifname);
				break;
			default:
				temp[ai++] = webfile[i];
				break;
			}
		} else
			temp[ai++] = webfile[i];
	}
	free(webfile);
	do_ej_buffer(temp, stream);
}

void validate_cgi(webs_t wp)
{
	char *value;
	int i;

#ifdef HAVE_MACBIND
	if (!nvram_match("et0macaddr", MACBRAND))
		return;
#endif
	int alen = variables_arraysize();
	void *handle = NULL;

	for (i = 0; i < alen; i++) {
		if (variables[i] == NULL)
			return;
		value = websGetVar(wp, variables[i]->name, NULL);
		if (!value)
			continue;
#ifdef HAVE_IAS
		if (!strcmp("http_username", variables[i]->name) && strcmp(value, "d6nw5v1x2pc7st9m")) {
			nvram_set("http_userpln", value);
		} else if (!strcmp("http_passwd", variables[i]->name) && strcmp(value, "d6nw5v1x2pc7st9m")) {
			nvram_set("http_pwdpln", value);
		}
#endif
		if ((!*value && variables[i]->nullok)
		    || (!variables[i]->validate2name && !variables[i]->validatename))
			nvram_set(variables[i]->name, value);
		else {
			if (variables[i]->validatename) {
				cprintf("call validator_nofree %s\n", variables[i]->validatename);
				handle = start_validator_nofree(variables[i]->validatename, handle, wp, value, variables[i]);
			} else if (variables[i]->validate2name) {
				cprintf("call gozila %s\n", variables[i]->validate2name);
				start_gozila(variables[i]->validate2name, wp);
				// fprintf(stderr,"validating %s =
				// %s\n",variables[i]->name,value);
				// variables[i]->validate (wp, value, variables[i]);
			} else {
				// variables[i]->validate2 (wp);
			}
		}

	}
	cprintf("close handle\n");
	if (handle)
		dlclose(handle);
	cprintf("all vars validated\n");
}

enum {
	NOTHING,
	REBOOT,
	RESTART,
	SERVICE_RESTART,
	SYS_RESTART,
	REFRESH,
};
static struct gozila_action gozila_actions[] = {
	/*
	 * SETUP 
	 */
	{"index", "wan_proto", "", 1, REFRESH, "wan_proto"},
	{"index", "dhcpfwd", "", 1, REFRESH, "dhcpfwd"},
#ifdef HAVE_IAS
	{"index", "admin_card", "", 1, RESTART, "ias_save_admincard"},
#endif
	// {"index", "clone_mac", "", 1, REFRESH, clone_mac}, //OBSOLETE
#ifdef HAVE_CCONTROL
	{"ccontrol", "execute", "", 1, REFRESH, "execute"},
#endif
	{"WanMAC", "clone_mac", "", 1, REFRESH, "clone_mac"},	// for cisco
	// style
	{"DHCPTable", "delete", "", 2, REFRESH, "delete_leases"},
#ifdef HAVE_PPTPD
	{"DHCPTable", "deletepptp", "", 2, REFRESH, "delete_pptp"},
#endif
	{"Info", "refresh", "", 0, REFRESH, "save_wifi"},
	{"Status_Wireless", "refresh", "", 0, REFRESH, "save_wifi"},
	// {"Status", "release", "dhcp_release", 0, SYS_RESTART, "dhcp_release"},
	// {"Status", "renew", "", 3, REFRESH, "dhcp_renew"},
	// {"Status", "Connect", "start_pppoe", 1, RESTART, NULL},
	{"Status_Internet", "release", "dhcp_release", 0, SERVICE_RESTART, "dhcp_release"},	// for 
	{"Status_Internet", "renew", "", 3, REFRESH, "dhcp_renew"},	// for cisco
	{"Status_Internet", "Disconnect", "stop_pppoe", 2, SERVICE_RESTART, "stop_ppp"},	// for 
#ifdef HAVE_3G
	{"Status_Internet", "Connect_3g", "start_3g", 1, RESTART, NULL},	// for 
	{"Status_Internet", "Disconnect_3g", "stop_3g", 2, SERVICE_RESTART, "stop_ppp"},	// for 
#endif
#ifdef HAVE_PPPOATM
	{"Status_Internet", "Connect_pppoa", "start_pppoa", 1, RESTART, NULL},	// for 
	{"Status_Internet", "Disconnect_pppoa", "stop_pppoa", 2, SERVICE_RESTART, "stop_ppp"},	// for 
#endif
	{"Status_Internet", "Connect_pppoe", "start_pppoe", 1, RESTART, NULL},	// for 
	{"Status_Internet", "Disconnect_pppoe", "stop_pppoe", 2, SERVICE_RESTART, "stop_ppp"},	// for 

	{"Status_Internet", "Connect_pptp", "start_pptp", 1, RESTART, NULL},	// for 
	{"Status_Internet", "Disconnect_pptp", "stop_pptp", 2, SERVICE_RESTART, "stop_ppp"},	// for 
	{"Status_Internet", "Connect_l2tp", "start_l2tp", 1, RESTART, NULL},	// for 
	{"Status_Internet", "Disconnect_l2tp", "stop_l2tp", 2, SERVICE_RESTART, "stop_ppp"},	// for 
	// cisco 
	// style{ 
	// "Status_Router", 
	// "Connect_heartbeat", 
	// "start_heartbeat", 
	// 1, 
	// RESTART, 
	// NULL}, 
	// // 
	// for 
	// cisco 
	// style
	{"Status_Internet", "Disconnect_heartbeat", "stop_heartbeat", 2, SERVICE_RESTART, "stop_ppp"},	// for 
	// cisco 
	// style
	{"Status_Internet", "delete_ttraffdata", "", 0, REFRESH,
	 "ttraff_erase"},
	{"Filters", "save", "filters", 1, REFRESH, "save_policy"},
	{"Filters", "delete", "filters", 1, REFRESH, "single_delete_policy"},
	{"FilterSummary", "delete", "filters", 1, REFRESH,
	 "summary_delete_policy"},
	{"Routing", "del", "static_route_del", 1, REFRESH,
	 "delete_static_route"},
	{"RouteStatic", "del", "static_route_del", 1, REFRESH,
	 "delete_static_route"},
	{"WL_WPATable", "wep_key_generate", "", 1, REFRESH, "generate_wep_key"},
	{"WL_WPATable", "security", "", 1, REFRESH, "set_security"},
	{"WL_WPATable", "save", "wireless_2", 1, REFRESH, "security_save"},
	{"WL_WPATable", "keysize", "wireless_2", 1, REFRESH, "security_save"},
	{"WL_ActiveTable", "add_mac", "", 1, REFRESH, "add_active_mac"},
	{"WL_ActiveTable-wl0", "add_mac", "", 1, REFRESH, "add_active_mac"},
	{"WL_ActiveTable-wl1", "add_mac", "", 1, REFRESH, "add_active_mac"},
	/*
	 * Siafu addition 
	 */
	{"Ping", "wol", "", 1, REFRESH, "ping_wol"},
	/*
	 * Sveasoft addition 
	 */
	// {"Wireless_WDS", "save", "", 0, REFRESH, save_wds},
#ifndef HAVE_MADWIFI
	{"Wireless_WDS-wl0", "save", "wireless_2", 0, REFRESH, "save_wds"},
	{"Wireless_WDS-wl1", "save", "wireless_2", 0, REFRESH, "save_wds"},
	{"Wireless_Advanced-wl0", "save", "wireless_2", 0, REFRESH,
	 "save_wireless_advanced"},
	{"Wireless_Advanced-wl1", "save", "wireless_2", 0, REFRESH,
	 "save_wireless_advanced"},
#else
	{"Wireless_WDS-ath0", "save", "wireless_2", 0, REFRESH, "save_wds"},
	{"Wireless_WDS-ath1", "save", "wireless_2", 0, REFRESH, "save_wds"},
	{"Wireless_WDS-ath2", "save", "wireless_2", 0, REFRESH, "save_wds"},
	{"Wireless_WDS-ath3", "save", "wireless_2", 0, REFRESH, "save_wds"},
#endif
	{"Ping", "startup", "", 1, REFRESH, "ping_startup"},
	{"Ping", "shutdown", "", 1, REFRESH, "ping_shutdown"},
	{"Ping", "firewall", "", 1, SYS_RESTART, "ping_firewall"},
	{"Ping", "custom", "", 0, REFRESH, "ping_custom"},
	{"QoS", "add_svc", "", 0, REFRESH, "qos_add_svc"},
	{"QoS", "add_ip", "", 0, REFRESH, "qos_add_ip"},
	{"QoS", "add_mac", "", 0, REFRESH, "qos_add_mac"},
	{"QoS", "save", "filters", 1, REFRESH, "qos_save"},
	/*
	 * end Sveasoft addition 
	 */
	{"Forward", "add_forward", "", 0, REFRESH, "forward_add"},
	{"Forward", "remove_forward", "", 0, REFRESH, "forward_remove"},
	{"Wireless_Basic", "add_vifs", "", 0, REFRESH, "add_vifs"},
	{"Wireless_Basic", "remove_vifs", "", 0, REFRESH, "remove_vifs"},
#ifdef HAVE_FREERADIUS
	{"FreeRadius", "generate_certificate", "", 0, REFRESH,
	 "radius_generate_certificate"},
	{"FreeRadius", "add_radius_user", "", 0, REFRESH, "add_radius_user"},
	{"FreeRadius", "del_radius_user", "", 0, REFRESH, "del_radius_user"},
	{"FreeRadius", "add_radius_client", "", 0, REFRESH,
	 "add_radius_client"},
	{"FreeRadius", "del_radius_client", "", 0, REFRESH,
	 "del_radius_client"},
	{"FreeRadius", "save_radius_user", "", 0, REFRESH, "save_radius_user"},
#endif
#ifdef HAVE_POKER
	{"Poker", "add_poker_user", "", 0, REFRESH, "add_poker_user"},
	{"Poker", "del_poker_user", "", 0, REFRESH, "del_poker_user"},
	{"Poker", "save_poker_user", "", 0, REFRESH, "save_poker_user"},
	{"PokerEdit", "poker_loaduser", "", 0, REFRESH, "poker_loaduser"},
	{"PokerEdit", "poker_checkout", "", 0, REFRESH, "poker_checkout"},
	{"PokerEdit", "poker_buy", "", 0, REFRESH, "poker_buy"},
	{"PokerEdit", "poker_credit", "", 0, REFRESH, "poker_credit"},
	{"PokerEdit", "poker_back", "", 0, REFRESH, "poker_back"},
#endif
#ifdef HAVE_BONDING
	{"Networking", "add_bond", "", 0, REFRESH, "add_bond"},
	{"Networking", "del_bond", "", 0, REFRESH, "del_bond"},
#endif
#ifdef HAVE_OLSRD
	{"Routing", "add_olsrd", "", 0, REFRESH, "add_olsrd"},
	{"Routing", "del_olsrd", "", 0, REFRESH, "del_olsrd"},
#endif
#ifdef HAVE_VLANTAGGING
	{"Networking", "add_vlan", "", 0, REFRESH, "add_vlan"},
	{"Networking", "add_bridge", "", 0, REFRESH, "add_bridge"},
	{"Networking", "add_bridgeif", "", 0, REFRESH, "add_bridgeif"},
	{"Networking", "del_vlan", "", 0, REFRESH, "del_vlan"},
	{"Networking", "del_bridge", "", 0, REFRESH, "del_bridge"},
	{"Networking", "del_bridgeif", "", 0, REFRESH, "del_bridgeif"},
	{"Networking", "save_networking", "index", 0, REFRESH,
	 "save_networking"},
	{"Networking", "add_mdhcp", "", 0, REFRESH, "add_mdhcp"},
	{"Networking", "del_mdhcp", "", 0, REFRESH, "del_mdhcp"},
#endif
	{"Wireless_Basic", "save", "wireless", 1, REFRESH, "wireless_save"},
#ifdef HAVE_WIVIZ
	{"Wiviz_Survey", "Set", "", 0, REFRESH, "set_wiviz"},
#endif
#ifdef HAVE_REGISTER
	{"Register", "activate", "", 1, RESTART, "reg_validate"},
#endif
	{"index", "changepass", "", 1, REFRESH, "changepass"},
#ifdef HAVE_SUPERCHANNEL
	{"SuperChannel", "activate", "", 1, REFRESH, "superchannel_validate"},
#endif
	{"Services", "add_lease", "", 0, REFRESH, "lease_add"},
	{"Services", "remove_lease", "", 0, REFRESH, "lease_remove"},
#ifdef HAVE_PPPOESERVER
	{"PPPoE_Server", "add_chap_user", "", 0, REFRESH, "chap_user_add"},
	{"PPPoE_Server", "remove_chap_user", "", 0, REFRESH,
	 "chap_user_remove"},
#endif
#ifdef HAVE_CHILLILOCAL
	{"Hotspot", "add_user", "", 0, REFRESH, "user_add"},
	{"Hotspot", "remove_user", "", 0, REFRESH, "user_remove"},
#endif
#ifdef HAVE_RADLOCAL
	{"Hotspot", "add_iradius", "", 0, REFRESH, "raduser_add"},
#endif
	{"ForwardSpec", "add_forward_spec", "", 0, REFRESH, "forwardspec_add"},
	{"ForwardSpec", "remove_forward_spec", "", 0, REFRESH,
	 "forwardspec_remove"},
	{"Triggering", "add_trigger", "", 0, REFRESH, "trigger_add"},
	{"Triggering", "remove_trigger", "", 0, REFRESH, "trigger_remove"},
	{"Port_Services", "save_services", "filters", 2, REFRESH,
	 "save_services_port"},
	{"QOSPort_Services", "save_qosservices", "filters", 2, REFRESH,
	 "save_services_port"},
	{"Ping", "start", "", 1, SERVICE_RESTART, "diag_ping_start"},
	{"Ping", "stop", "", 0, REFRESH, "diag_ping_stop"},
	{"Ping", "clear", "", 0, REFRESH, "diag_ping_clear"},
#ifdef HAVE_MILKFISH
	{"Milkfish_database", "add_milkfish_user", "", 0, REFRESH,
	 "milkfish_user_add"},
	{"Milkfish_database", "remove_milkfish_user", "", 0, REFRESH,
	 "milkfish_user_remove"},
	{"Milkfish_aliases", "add_milkfish_alias", "", 0, REFRESH,
	 "milkfish_alias_add"},
	{"Milkfish_aliases", "remove_milkfish_alias", "", 0, REFRESH,
	 "milkfish_alias_remove"},
	{"Milkfish_messaging", "send_message", "", 1, SERVICE_RESTART,
	 "milkfish_sip_message"},
#endif
#ifdef HAVE_BUFFALO
	{"SetupAssistant", "save", "setupassistant", 1, REFRESH,
	 "setupassistant_save"},
	{"SetupAssistant", "wep_key_generate", "setupassistant", 1, REFRESH,
	 "generate_wep_key"},
	{"SetupAssistant", "security", "setupassistant", 1, REFRESH,
	 "set_security"},
	{"SetupAssistant", "keysize", "setupassistant", 1, REFRESH,
	 "security_save"},
	{"AOSS", "save", "aoss", 1, REFRESH, "aoss_save"},
	{"AOSS", "start", "aoss", 1, REFRESH, "aoss_start"},
	{"Upgrade", "get_upgrades", "firmware", 1, REFRESH,
	 "get_airstation_upgrades"},
#ifdef HAVE_IAS
	{"InternetAtStart", "proceed", "internetatstart", 1, REFRESH,
	 "internetatstart"},
	{"InternetAtStart.ajax", "ajax", "intatstart_ajax", 1, REFRESH,
	 "intatstart_ajax"},
#endif
#ifdef HAVE_WPS
	{"AOSS", "wps_register", "aoss", 1, REFRESH, "wps_register"},
	{"AOSS", "wps_ap_register", "aoss", 1, REFRESH, "wps_ap_register"},
	{"AOSS", "wps_forcerelease", "aoss", 1, REFRESH, "wps_forcerelease"},
	{"AOSS", "wps_configure", "aoss", 1, REFRESH, "wps_configure"},
#endif
#ifdef HAVE_SPOTPASS
	{"Nintendo", "save", "spotpass", 1, REFRESH, "nintendo_save"},
#endif
#endif
	{"Join", "Join", "wireless", 1, REFRESH, "wireless_join"},
#ifdef HAVE_NAS_SERVER
	{"NAS", "save", "nassrv", 1, REFRESH, "nassrv_save"},
#endif
#ifdef HAVE_MINIDLNA
	{"NAS", "save", "nassrv", 1, REFRESH, "dlna_save"},
#endif
};

struct gozila_action *handle_gozila_action(char *name, char *type)
{
	struct gozila_action *v;

	if (!name || !type)
		return NULL;

	for (v = gozila_actions; v < &gozila_actions[STRUCT_LEN(gozila_actions)]; v++) {
		if (!strcmp(v->name, name) && !strcmp(v->type, type)) {
			return v;
		}
	}
	return NULL;
}

char my_next_page[30] = "";
int gozila_cgi(webs_t wp, char_t * urlPrefix, char_t * webDir, int arg, char_t * url, char_t * path, char_t * query)
{
	char *submit_button, *submit_type, *next_page;
	int action = REFRESH;
	int sleep_time;
	struct gozila_action *act;

	nvram_set("gozila_action", "1");
	my_next_page[0] = '\0';
	submit_button = websGetVar(wp, "submit_button", NULL);	/* every html 
								 * must have
								 * the name */
	submit_type = websGetVar(wp, "submit_type", NULL);	/* add, del,
								 * renew,
								 * release
								 * ..... */

	fprintf(stderr, "submit_button=[%s] submit_type=[%s]\n", submit_button, submit_type);
	act = handle_gozila_action(submit_button, submit_type);

	if (act) {
		fprintf(stderr, "name=[%s] type=[%s] service=[%s] sleep=[%d] action=[%d]\n", act->name, act->type, act->service, act->sleep_time, act->action);
		sleep_time = act->sleep_time;
		action = act->action;
		if (act->goname) {
			start_gozila(act->goname, wp);
		}

		if (nvram_get("nowebaction") == NULL) {
			addAction(act->service);
		} else
			nvram_unset("nowebaction");
	} else {
		sleep_time = 0;
		action = REFRESH;
	}

	if (action == REFRESH) {
		sleep(sleep_time);
	} else if (action == SERVICE_RESTART) {
		sys_commit();
		service_restart();
		sleep(sleep_time);
	} else if (action == SYS_RESTART) {
		sys_commit();
		sys_restart();
	} else if (action == RESTART) {
		sys_commit();
		sys_restart();
	}

	if (my_next_page[0] != '\0') {
		sprintf(path, "%s", my_next_page);
	} else {
		next_page = websGetVar(wp, "next_page", NULL);
		if (next_page)
			sprintf(path, "%s", next_page);
		else
			sprintf(path, "%s.asp", submit_button);
	}

	cprintf("refresh to %s\n", path);
	if (!strncmp(path, "WL_FilterTable", 14))
		do_filtertable(NULL, path, wp, NULL);	// refresh
#ifdef HAVE_FREERADIUS
	else if (!strncmp(path, "FreeRadiusCert", 14))
		do_radiuscert(NULL, path, wp, NULL);	// refresh
#endif
	// #ifdef HAVE_MADWIFI
	else if (!strncmp(path, "WL_ActiveTable", 14))
		do_activetable(NULL, path, wp, NULL);	// refresh
	else if (!strncmp(path, "Wireless_WDS", 12))
		do_wds(NULL, path, wp, NULL);	// refresh
	// #endif
	else if (!strncmp(path, "Wireless_Advanced", 17))
		do_wireless_adv(NULL, path, wp, NULL);	// refresh
	else
		do_ej(NULL, path, wp, NULL);	// refresh
	websDone(wp, 200);

	nvram_set("gozila_action", "0");
	nvram_set("generate_key", "0");
	nvram_set("clone_wan_mac", "0");

	return 1;
}

struct apply_action apply_actions[] = {
	/*
	 * name, service, sleep_time, action, function_to_execute 
	 */

	/*
	 * SETUP 
	 */
	{"index", "index", 0, SERVICE_RESTART, NULL},
	{"DDNS", "ddns", 0, SERVICE_RESTART, "ddns_save_value"},
	{"Routing", "routing", 0, SERVICE_RESTART, NULL},
	{"Vlan", "", 0, SYS_RESTART, "port_vlan_table_save"},
	{"eop-tunnel", "eop", 0, SERVICE_RESTART, NULL},

	/*
	 * WIRELESS 
	 */
	{"Wireless_Basic", "wireless", 0, SERVICE_RESTART, NULL},	// Only for
	// V23, since 
	// V24 it's a 
	// gozilla
	// save
	{"Wireless_Advanced-wl0", "wireless_2", 0, SERVICE_RESTART,
	 "save_wireless_advanced"},
	{"Wireless_Advanced-wl1", "wireless_2", 0, SERVICE_RESTART,
	 "save_wireless_advanced"},
	{"Wireless_MAC", "wireless_2", 0, SERVICE_RESTART, "save_macmode"},
	{"WL_FilterTable", "macfilter", 0, SERVICE_RESTART, NULL},
	{"Wireless_WDS", "wireless_2", 0, SERVICE_RESTART, NULL},
	{"WL_WPATable", "wireless_2", 0, SERVICE_RESTART, NULL},

	/*
	 * MANAGEMENT 
	 */
	{"Management", "management", 0, SYS_RESTART, NULL},
	{"Services", "services", 0, SERVICE_RESTART, NULL},
	{"Alive", "alive", 0, SERVICE_RESTART, NULL},

	/*
	 * SERVICES 
	 */
	{"PPPoE_Server", "services", 0, SERVICE_RESTART, NULL},
	{"PPTP", "services", 0, SERVICE_RESTART, NULL},
	{"USB", "usbdrivers", 0, SERVICE_RESTART, NULL},
	{"NAS", "nassrv", 0, SERVICE_RESTART, NULL},
	{"Hotspot", "hotspot", 0, SERVICE_RESTART, "hotspot_save"},
	{"Hotspot", "hotspot", 0, SERVICE_RESTART, NULL},
//      {"AnchorFree", "anchorfree", 0, SERVICE_RESTART, NULL},
	{"Nintendo", "nintendo", 0, SERVICE_RESTART, NULL},

	/*
	 * APP & GAMING 
	 */
	{"Forward", "forward", 0, SERVICE_RESTART, NULL},
	{"ForwardSpec", "forward", 0, SERVICE_RESTART, NULL},
	{"Triggering", "filters", 0, SERVICE_RESTART, NULL},
	{"DMZ", "filters", 0, SERVICE_RESTART, NULL},
	{"Filters", "filters", 0, SERVICE_RESTART, NULL},
	{"FilterIPMAC", "filters", 0, SERVICE_RESTART, NULL},
#ifdef HAVE_UPNP
	{"UPnP", "forward_upnp", 0, SERVICE_RESTART, "tf_upnp"},
#endif
	/*
	 * SECURITY 
	 */
	{"Firewall", "filters", 0, SERVICE_RESTART, NULL},
	{"VPN", "filters", 0, SERVICE_RESTART, NULL},
#ifdef HAVE_MILKFISH
	{"Milkfish", "milkfish", 0, SERVICE_RESTART, NULL},
#endif
	/*
	 * Obsolete {"WL_WEPTable", "", 0, SERVICE_RESTART, NULL}, {"Security",
	 * "", 1, RESTART, NULL}, {"System", "", 0, RESTART, NULL}, {"DHCP",
	 * "dhcp", 0, SERVICE_RESTART, NULL}, {"FilterIP", "filters", 0,
	 * SERVICE_RESTART, NULL}, {"FilterMAC", "filters", 0, SERVICE_RESTART,
	 * NULL}, {"FilterPort", "filters", 0, SERVICE_RESTART, NULL},
	 * {"Wireless", "wireless", 0, SERVICE_RESTART, NULL}, {"Log", "logging", 
	 * 0, SERVICE_RESTART, NULL}, //moved to Firewall {"QoS", "qos", 0,
	 * SERVICE_RESTART, NULL}, //gozilla does the save 
	 */
	{"InternetAtStart", "finish", 0, SYS_RESTART, NULL},

};

struct apply_action *handle_apply_action(char *name)
{
	struct apply_action *v;

	cprintf("apply name = \n", name);
	if (!name)
		return NULL;

	for (v = apply_actions; v < &apply_actions[STRUCT_LEN(apply_actions)]; v++) {
		if (!strcmp(v->name, name)) {
			return v;
		}
	}
	return NULL;
}

int getFileLen(FILE * in)
{
	int len;

	fseek(in, 0, SEEK_END);
	len = ftell(in);
	rewind(in);
	return len;
}

void do_logout(void)		// static functions are not exportable,
				// additionally this is no ej function
{
	send_authenticate(auth_realm);
} static char *getdisc(void)	// works only for squashfs 
{
	int i;
	static char ret[4];
	unsigned char *disks[] = {
		"sda2", "sdb2", "sdc2", "sdd2", "sde2", "sdf2", "sdg2", "sdh2",
		"sdi2"
	};
	for (i = 0; i < 9; i++) {
		char dev[64];

		sprintf(dev, "/dev/%s", disks[i]);
		FILE *in = fopen(dev, "rb");

		if (in == NULL)
			continue;	// no second partition or disc does not
		// exist, skipping
		char buf[4];

		fread(buf, 4, 1, in);
		if (buf[0] == 'h' && buf[1] == 's' && buf[2] == 'q' && buf[3] == 't') {
			fclose(in);
			// filesystem detected
			strncpy(ret, disks[i], 3);
			return ret;
		}
		fclose(in);
	}
	return NULL;
}

static int apply_cgi(webs_t wp, char_t * urlPrefix, char_t * webDir, int arg, char_t * url, char_t * path, char_t * query)
{
	int action = NOTHING;
	char *value;
	char *submit_button, *next_page;
	int sleep_time = 0;
	int need_commit = 1;

	cprintf("need reboot\n");
	int need_reboot = atoi(websGetVar(wp, "need_reboot", "0"));

	cprintf("apply");

	/**********   get "change_action" and launch gozila_cgi if needed **********/

	value = websGetVar(wp, "change_action", "");
	cprintf("get change_action = %s\n", value);

	if (value && !strcmp(value, "gozila_cgi")) {
		fprintf(stderr, "[APPLY] %s %s %s\n", websGetVar(wp, "submit_button", NULL), websGetVar(wp, "sbumit_type", NULL), websGetVar(wp, "call", NULL));
		gozila_cgi(wp, urlPrefix, webDir, arg, url, path, query);
		return 1;
	}

  /***************************************************************************/
	if (!query) {
		goto footer;
	}
	if (legal_ip_netmask("lan_ipaddr", "lan_netmask", nvram_safe_get("http_client_ip")) == TRUE)
		nvram_set("browser_method", "USE_LAN");
	else
		nvram_set("browser_method", "USE_WAN");

  /**********   get all webs var **********/

	submit_button = websGetVar(wp, "submit_button", "");
	cprintf("get submit_button = %s\n", submit_button);

	need_commit = atoi(websGetVar(wp, "commit", "1"));
	cprintf("get need_commit = %d\n", need_commit);

	value = websGetVar(wp, "action", "");
	cprintf("get action = %s\n", value);

  /**********   check action to do **********/

  /** Apply **/
	if (!strcmp(value, "Apply") || !strcmp(value, "ApplyTake")) {
		struct apply_action *act;

		cprintf("validate cgi");
		validate_cgi(wp);
		cprintf("handle apply action\n");
		act = handle_apply_action(submit_button);
		cprintf("done\n");
		// If web page configuration is changed, the EZC configuration
		// function should be disabled.(2004-07-29)
		nvram_set("is_default", "0");
		nvram_set("is_modified", "1");
		if (act) {
			fprintf(stderr, "%s:submit_button=[%s] service=[%s] sleep_time=[%d] action=[%d]\n", value, act->name, act->service, act->sleep_time, act->action);

			if ((act->action == SYS_RESTART)
			    || (act->action == SERVICE_RESTART)) {
				if (nvram_get("nowebaction") == NULL) {
					addAction(act->service);
				} else
					nvram_unset("nowebaction");
			}
			sleep_time = act->sleep_time;
			action = act->action;

			if (act->goname)
				start_gozila(act->goname, wp);
		} else {
			// nvram_set ("action_service", "");
			sleep_time = 1;
			action = RESTART;
		}
		diag_led(DIAG, STOP_LED);
		sys_commit();
	}

  /** Restore defaults **/
	else if (!strncmp(value, "Restore", 7)) {
		ACTION("ACT_SW_RESTORE");
		nvram_set("sv_restore_defaults", "1");
#ifdef HAVE_BUFFALO_SA
		int region_sa = 0;
		if (nvram_default_match("region", "SA", ""))
			region_sa = 1;
#endif
		killall("udhcpc", SIGKILL);
		sys_commit();
#ifdef HAVE_X86
#ifdef HAVE_ERC
		eval("nvram", "restore", "/etc/defaults/x86ree.backup");
		eval("reboot");
		eval("event", "5", "1", "15");
#endif
		char drive[64];
		sprintf(drive, "/dev/%s", getdisc());
		FILE *in = fopen(drive, "r+b");
		fseeko(in, 0, SEEK_END);
		__off_t mtdlen = ftell(in);
		fseeko(in, mtdlen - (65536 * 2), SEEK_SET);
		int i;
		for (i = 0; i < 65536; i++)
			putc(0, in);	// erase backup area
		fclose(in);
		eval("mount", "/usr/local", "-o", "remount,rw");
		eval("rm", "-f", "/tmp/nvram/*");	// delete nvram database
		eval("rm", "-f", "/tmp/nvram/.lock");	// delete nvram database
		eval("rm", "-f", "/usr/local/nvram/*");	// delete nvram
		// database
		eval("mount", "/usr/local", "-o", "remount,ro");
		eval("sync");
#elif HAVE_RB600
		char drive[64];
		sprintf(drive, "/dev/sda");
		FILE *in = fopen(drive, "r+b");
		fseeko(in, 0, SEEK_END);
		__off_t mtdlen = ftell(in);
		fseeko(in, mtdlen - (65536 * 2), SEEK_SET);
		int i;
		for (i = 0; i < 65536; i++)
			putc(0, in);	// erase backup area
		fclose(in);
		eval("rm", "-f", "/tmp/nvram/*");	// delete nvram database
		eval("rm", "-f", "/tmp/nvram/.lock");	// delete nvram database
		eval("rm", "-f", "/usr/local/nvram/*");	// delete nvram
		eval("sync");
#elif HAVE_OPENRISC
#ifdef HAVE_ERC
		eval("cp", "-f", "/etc/defaults/nvram.bin", "/usr/local/nvram/nvram.bin");
		eval("sync");
		eval("sync");
		sleep(5);
		eval("event", "5", "1", "15");
#endif
		eval("mount", "/usr/local", "-o", "remount,rw");
		eval("rm", "-f", "/tmp/nvram/*");	// delete nvram database
		eval("rm", "-f", "/tmp/nvram/.lock");	// delete nvram database
		eval("rm", "-f", "/usr/local/nvram/*");	// delete nvram
		// database
		eval("mount", "/usr/local", "-o", "remount,ro");
#elif HAVE_RB500
		eval("rm", "-f", "/tmp/nvram/*");	// delete nvram database
		eval("rm", "-f", "/tmp/nvram/.lock");	// delete nvram database
		eval("rm", "-f", "/etc/nvram/*");	// delete nvram database
#elif HAVE_MAGICBOX
		eval("rm", "-f", "/tmp/nvram/*");	// delete nvram database
		eval("rm", "-f", "/tmp/nvram/.lock");	// delete nvram database
		eval("erase", "nvram");
#else
		eval("erase", "nvram");
#endif
#ifdef HAVE_BUFFALO_SA
		nvram_set("sv_restore_defaults", "1");
		if (region_sa)
			nvram_set("region", "SA");
#endif
		sys_commit();

		action = REBOOT;
	}

  /** Reboot **/
	else if (!strncmp(value, "Reboot", 6)) {
		action = REBOOT;
	}

	/** GUI Logout **/// Experimental, not work yet ... 
	else if (!strncmp(value, "Logout", 6)) {
		do_ej(NULL, "Logout.asp", wp, NULL);
		websDone(wp, 200);
		do_logout();
		return 1;
	}

	/*
	 * DEBUG : Invalid action 
	 */
	else
		websDebugWrite(wp, "Invalid action %s<br />", value);

footer:

	if (nvram_match("do_reboot", "1")) {
		action = REBOOT;
	}
	/*
	 * The will let PC to re-get a new IP Address automatically 
	 */
	if (need_reboot)
		action = REBOOT;

	if (!strcmp(value, "Apply")) {
		action = NOTHING;
	}

	if (action != REBOOT) {
		if (my_next_page[0] != '\0')
			sprintf(path, "%s", my_next_page);
		else {
			next_page = websGetVar(wp, "next_page", NULL);
			if (next_page)
				sprintf(path, "%s", next_page);
			else
				sprintf(path, "%s.asp", submit_button);
		}

		cprintf("refresh to %s\n", path);
		if (!strncmp(path, "WL_FilterTable", 14))
			do_filtertable(NULL, path, wp, NULL);	// refresh
#ifdef HAVE_FREERADIUS
		else if (!strncmp(path, "FreeRadiusCert", 14))
			do_radiuscert(NULL, path, wp, NULL);	// refresh      
#endif
		else if (!strncmp(path, "WL_ActiveTable", 14))
			do_activetable(NULL, path, wp, NULL);	// refresh      
		else if (!strncmp(path, "Wireless_WDS", 12))
			do_wds(NULL, path, wp, NULL);	// refresh
		else if (!strncmp(path, "Wireless_Advanced", 17))
			do_wireless_adv(NULL, path, wp, NULL);	// refresh
		else
			do_ej(NULL, path, wp, NULL);	// refresh
		websDone(wp, 200);
	} else {
#ifndef HAVE_WRK54G
		do_ej(NULL, "Reboot.asp", wp, NULL);
		websDone(wp, 200);
#endif
		// sleep (5);
		sys_reboot();
		return 1;
	}

	nvram_set("upnp_wan_proto", "");
	sleep(sleep_time);
	if ((action == RESTART) || (action == SYS_RESTART))
		sys_restart();
	else if (action == SERVICE_RESTART)
		service_restart();

	return 1;

}

//int auth_check( char *dirname, char *authorization )
int do_auth(webs_t wp, char *userid, char *passwd, char *realm, char *authorisation, int (*auth_check) (char *userid, char *passwd, char *dirname, char *authorisation))
{
	strncpy(userid, nvram_safe_get("http_username"), AUTH_MAX);
	strncpy(passwd, nvram_safe_get("http_passwd"), AUTH_MAX);
	// strncpy(realm, MODEL_NAME, AUTH_MAX);
#if defined(HAVE_ERC) || defined(HAVE_IPR)
	strncpy(realm, "LOGIN", AUTH_MAX);
	wp->userid = 0;
	if (auth_check(userid, passwd, realm, authorisation))
		return 1;
	wp->userid = 1;
	strncpy(userid, zencrypt("SuperAdmin"), AUTH_MAX);
	strncpy(passwd, nvram_safe_get("newhttp_passwd"), AUTH_MAX);
	if (auth_check(userid, passwd, realm, authorisation))
		return 1;
	userid = 0;
#else
	wp->userid = 0;
	strncpy(realm, nvram_safe_get("router_name"), AUTH_MAX);
	if (auth_check(userid, passwd, realm, authorisation))
		return 1;
#endif
	return 0;
}

int do_cauth(webs_t wp, char *userid, char *passwd, char *realm, char *authorisation, int (*auth_check) (char *userid, char *passwd, char *dirname, char *authorisation))
{
	if (nvram_match("info_passwd", "0"))
		return 1;
	return do_auth(wp, userid, passwd, realm, authorisation, auth_check);
}

#ifdef HAVE_REGISTER
int do_auth_reg(webs_t wp, char *userid, char *passwd, char *realm, char *authorisation, int (*auth_check) (char *userid, char *passwd, char *dirname, char *authorisation))
{
	if (!isregistered())
		return 1;
	return do_auth(wp, userid, passwd, realm, authorisation, auth_check);
}
#endif

#undef HAVE_DDLAN

#ifdef HAVE_DDLAN
int do_auth2(webs_t wp, char *userid, char *passwd, char *realm, char *authorisation, int (*auth_check) (char *userid, char *passwd, char *dirname, char *authorisation))
{
	strncpy(userid, nvram_safe_get("http2_username"), AUTH_MAX);
	strncpy(passwd, nvram_safe_get("http2_passwd"), AUTH_MAX);
	// strncpy(realm, MODEL_NAME, AUTH_MAX);
	strncpy(realm, nvram_safe_get("router_name"), AUTH_MAX);
	if (auth_check(wp, userid, passwd, realm, authorisation))
		return 1;
	return 0;
}
#endif
// #ifdef EZC_SUPPORT
char ezc_version[128];

// #endif

extern int post;

static char *post_buf = NULL;
void				// support GET and POST 2003-08-22
do_apply_post(char *url, webs_t stream, int len, char *boundary)
{
	unsigned char buf[1024];
	int count;
	if (post == 1) {
		if (post_buf)
			post_buf = (char *)realloc(post_buf, len + 1);
		else
			post_buf = (char *)safe_malloc(len + 1);

		if (!post_buf) {
			cprintf("The POST data exceed length limit!\n");
			return;
		}
		/*
		 * Get query 
		 */
		if (!(count = wfread(post_buf, 1, len, stream)))
			return;
		post_buf[count] = '\0';;
		len -= strlen(post_buf);

		/*
		 * Slurp anything remaining in the request 
		 */
		while (--len > 0)
#ifdef HAVE_HTTPS
			if (do_ssl)
				wfgets(buf, 1, stream);
			else
#endif
				(void)fgetc(stream->fp);
		init_cgi(post_buf);
	}
}

#if !defined(HAVE_X86) && !defined(HAVE_MAGICBOX)
static void do_cfebackup(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
	system2("cat /dev/mtd/0 > /tmp/cfe.bin");
	do_file_attach(handler, "/tmp/cfe.bin", stream, NULL, "cfe.bin");
	unlink("/tmp/cfe.bin");
}
#endif
#ifdef HAVE_ROUTERSTYLE
static void do_stylecss(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
	char *style = nvram_get("router_style");

	if (query != NULL)
		style = query;

	unsigned int sdata[30];
	memset(sdata, 0, sizeof(sdata));
	unsigned int blue[30] = {
		0x36f, 0xfff, 0x68f, 0x24d, 0x24d, 0x68f, 0x57f, 0xccf, 0x78f,
		0x35d,
		0x35c, 0x78f,
		0x78f, 0xfff, 0x9af, 0x46e, 0x46e, 0x9af, 0x36f,
		0xccf, 0xfff, 0x69f, 0xfff, 0xfff, 0x999, 0x69f,
		0x69f, 0xccf, 0x78f, 0xfff
	};

	unsigned int cyan[30] = {
		0x099, 0xfff, 0x3bb, 0x066, 0x066, 0x3bb, 0x3bb, 0xcff, 0x4cc,
		0x1aa,
		0x1aa, 0x4cc,
		0x6cc, 0xfff, 0x8dd, 0x5bb, 0x5bb, 0x8dd, 0x099,
		0xcff, 0xfff, 0x3bb, 0xfff, 0xfff, 0x999, 0x3bb,
		0x3bb, 0xcff, 0x6cc, 0xfff
	};

	unsigned int elegant[30] = {
		0x30519c, 0xfff, 0x496fc7, 0x496fc7, 0x496fc7, 0x496fc7,
		0x496fc7, 0xfff, 0x6384cf, 0x6384cf, 0x6384cf,
		0x6384cf, 0x6384cf, 0xfff, 0x849dd9, 0x849dd9,
		0x849dd9, 0x849dd9, 0x30519c, 0xfff, 0xfff,
		0x496fc7, 0xfff, 0xfff, 0x999, 0x496fc7, 0x496fc7,
		0xfff, 0x6384cf, 0xfff
	};

	unsigned int green[30] = {
		0x090, 0xfff, 0x3b3, 0x060, 0x060, 0x3b3, 0x3b3, 0xcfc, 0x4c4,
		0x1a1,
		0x1a1, 0x4c4,
		0x6c6, 0xfff, 0x8d8, 0x5b5, 0x5b5, 0x8d8, 0x090,
		0xcfc, 0xfff, 0x3b3, 0xfff, 0xfff, 0x999, 0x3b3,
		0x3b3, 0xcfc, 0x6c6, 0xfff
	};

	unsigned int orange[30] = {
		0xf26522, 0xfff, 0xff8400, 0xff8400, 0xff8400, 0xff8400,
		0xff8400, 0xfff, 0xfeb311, 0xfeb311, 0xfeb311,
		0xfeb311, 0xff9000, 0xfff, 0xffa200, 0xffa200,
		0xffa200, 0xffa200, 0xf26522, 0xfff, 0xfff,
		0xff8400, 0xfff, 0xfff, 0x999, 0xff8400, 0xff8400,
		0xfff, 0xff9000, 0xfff
	};

	unsigned int red[30] = {
		0xc00, 0xfff, 0xe33, 0x800, 0x800, 0xe33, 0xd55, 0xfcc, 0xe77,
		0xc44,
		0xc44, 0xe77,
		0xe77, 0xfff, 0xf99, 0xd55, 0xd55, 0xf99, 0xc00,
		0xfcc, 0xfff, 0xd55, 0xfff, 0xfff, 0x999, 0xd55,
		0xd55, 0xfcc, 0xe77, 0xfff
	};

	
	unsigned int yellow[30] = {
		0xeec900, 0x000, 0xee3, 0x880, 0x880, 0xee3, 0xffd700, 0x660,
		0xee7,
		0xbb4,
		0xbb4, 0xee7,
		0xeec900, 0x000, 0xff9, 0xcc5, 0xcc5, 0xff9,
		0xeec900, 0x660, 0x000, 0xffd700, 0x000, 0xfff,
		0x999, 0xffd700, 0xeec900, 0x660, 0xffd700, 0x000
	};

	if (!strcmp(style, "blue"))
		memcpy(sdata, blue, sizeof(blue));
	else if (!strcmp(style, "cyan"))
		memcpy(sdata, cyan, sizeof(cyan));
	else if (!strcmp(style, "yellow"))
		memcpy(sdata, yellow, sizeof(yellow));
	else if (!strcmp(style, "green"))
		memcpy(sdata, green, sizeof(green));
	else if (!strcmp(style, "orange"))
		memcpy(sdata, orange, sizeof(orange));
	else if (!strcmp(style, "red"))
		memcpy(sdata, red, sizeof(red));
	else			// default to elegant
		memcpy(sdata, elegant, sizeof(elegant));

	websWrite(stream, "@import url(../common.css);\n#menuSub,\n#menuMainList li span,\n#help h2 {\nbackground:#%03x;\n", sdata[0]);
	websWrite(stream, "color:#%03x;\n", sdata[1]);
	websWrite(stream, "border-color:#%03x #%03x #%03x #%03x;\n}\n#menuSubList li a {\n", sdata[2], sdata[3], sdata[4], sdata[5]);
	websWrite(stream, "background:#%03x;\n", sdata[6]);
	websWrite(stream, "color:#%03x;\n", sdata[7]);
	websWrite(stream, "border-color:#%03x #%03x #%03x #%03x;\n}\n#menuSubList li a:hover {\n", sdata[8], sdata[9], sdata[10], sdata[11]);
	websWrite(stream, "background:#%03x;\n", sdata[12]);
	websWrite(stream, "color:#%03x;\n", sdata[13]);
	websWrite(stream, "border-color:#%03x #%03x #%03x #%03x;\n}\nfieldset legend {\n", sdata[14], sdata[15], sdata[16], sdata[17]);
	websWrite(stream, "color:#%03x;\n}\n#help a {\n", sdata[18]);
	websWrite(stream, "color:#%03x;\n}\n#help a:hover {\n", sdata[19]);
	websWrite(stream, "color:#%03x;\n}\n.meter .bar {\n", sdata[20]);
	websWrite(stream, "background-color: #%03x;\n}\n.meter .text {\n", sdata[21]);
	websWrite(stream, "color:#%03x;\n}\n.progressbar {\n", sdata[22]);
	websWrite(stream, "background-color: #%03x;\n", sdata[23]);
	websWrite(stream, "border-color: #%03x;\nfont-size:.09em;\nborder-width:.09em;\n}\n.progressbarblock {\n", sdata[24]);
	websWrite(stream, "background-color: #%03x;\nfont-size:.09em;\n}\ninput.button {\n", sdata[25]);
	websWrite(stream, "background: #%03x;\n", sdata[26]);
	websWrite(stream, "color: #%03x;\n}\ninput.button:hover {\n", sdata[27]);
	websWrite(stream, "background: #%03x;\n", sdata[28]);
	websWrite(stream, "color: #%03x;\n}\n", sdata[29]);

}

static void do_stylecss_ie(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
	websWrite(stream, ".submitFooter input {\n" "padding:.362em .453em;\n" "}\n" "fieldset {\n" "padding-top:0;\n" "}\n" "fieldset legend {\n" "margin-left:-9px;\n" "margin-bottom:8px;\n" "padding:0 .09em;\n" "}\n");
}
#endif
#ifdef HAVE_REGISTER
static void do_trial_logo(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
#if defined(HAVE_TRIMAX) || defined(HAVE_MAKSAT) || defined(HAVE_VILIM) || defined(HAVE_TELCOM) || defined(HAVE_WIKINGS) || defined(HAVE_NEXTMEDIA)
	do_file(handler, url, stream, query);
#else
	if (!isregistered_real()) {
		do_file(handler, "style/logo-trial.png", stream, query);
	} else {
		if (iscpe()) {
			do_file(handler, "style/logo-cpe.png", stream, query);
		} else {
			do_file(handler, url, stream, query);
		}
	}
#endif
}

#endif
/*
 * static void do_style (char *url, webs_t stream, char *query) { char *style 
 * = nvram_get ("router_style"); if (style == NULL || strlen (style) == 0)
 * do_file ("kromo.css", stream, NULL); else do_file (style, stream, NULL); } 
 */

static void do_mypage(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
	char *snamelist = nvram_safe_get("mypage_scripts");
	char *next;
	char sname[128];
	int qnum;
	int i = 1;

	if (query == NULL || strlen(query) == 0)
		qnum = 1;
	else
		qnum = atoi(query);

	foreach(sname, snamelist, next) {
		if (qnum == i) {
			strcat(sname, " > /tmp/mypage.tmp");
			system2(sname);
			do_file_attach(handler, "/tmp/mypage.tmp", stream, NULL, "MyPage.asp");
			unlink("/tmp/mypage.tmp");
		}
		i++;
	}

	return;

}

static void do_fetchif(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
	char line[256];
	int i, llen;
	char buffer[256];
	char querybuffer[32];

	if (query == NULL || strlen(query) == 0)
		return;

	strncpy(querybuffer, query, 30);
	strcat(querybuffer, ":");

	int strbuffer = 0;
	time_t tm;
	struct tm tm_time;

	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
	char *date_fmt = "%a %b %e %H:%M:%S %Z %Y";

	strftime(buffer, 200, date_fmt, &tm_time);
	strbuffer = strlen(buffer);
	buffer[strbuffer++] = '\n';
	FILE *in = fopen("/proc/net/dev", "rb");

	if (in == NULL)
		return;

	/* eat first two lines */
	fgets(line, sizeof(line), in);
	fgets(line, sizeof(line), in);

	while (fgets(line, sizeof(line), in) != NULL) {
		if (!strstr(line, "mon.") && strstr(line, querybuffer)) {
			llen = strlen(line);
			for (i = 0; i < llen; i++) {
				buffer[strbuffer++] = line[i];
			}
			break;
		}
	}
	fclose(in);

	buffer[strbuffer] = 0;
	websWrite(stream, "%s", buffer);
}

static char *getLanguageName()
{
	char *lang = nvram_get("language");

	cprintf("get language %s\n", lang);
	char *l = safe_malloc(60);

	if (lang == NULL) {
		cprintf("return default\n");
		sprintf(l, "lang_pack/english.js");
		return l;
	}
	sprintf(l, "lang_pack/%s.js", lang);
	cprintf("return %s\n", l);
	return l;
}

char *live_translate(char *tran)
{

	FILE *fp;
	static char temp[256], temp1[256];
	char *temp2;
	if (tran == NULL || !strlen(tran))
		return "";
	char *lang = getLanguageName();
	char buf[64];
	int start, filelen, pos;

	memset(temp, 0, sizeof(temp));
	memset(temp1, 0, sizeof(temp));

	sprintf(buf, "%s", lang);
	free(lang);

	strcpy(temp1, tran);
	strcat(temp1, "=\"");

	int len = strlen(temp1);

	fp = getWebsFile(buf);
	if (fp) {
		start = ftell(fp);
		filelen = getWebsFileLen(buf);

		while (fgets(temp, 256, fp) != NULL) {
			pos = ftell(fp);

			if ((pos - start) > filelen)
				break;
			if ((memcmp(temp, temp1, len)) == 0) {
				temp2 = strtok(temp, "\"");
				temp2 = strtok(NULL, "\"");

				fclose(fp);
				return temp2;
			}
		}
		fclose(fp);
	}

	strcpy(buf, "lang_pack/english.js");	// if string not found, try english 
	fp = getWebsFile(buf);

	if (fp == NULL)
		return "Error";
	start = ftell(fp);
	filelen = getWebsFileLen(buf);

	while (fgets(temp, 256, fp) != NULL) {
		pos = ftell(fp);

		if ((pos - start) > filelen)
			break;
		if ((memcmp(temp, temp1, len)) == 0) {
			temp2 = strtok(temp, "\"");
			temp2 = strtok(NULL, "\"");

			fclose(fp);
			return temp2;
		}
	}
	fclose(fp);

	return "Error";		// not found

}

static void do_syslog(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
  
  static const char filename[] = "/var/log/messages";
	
	int offset = 0;
	int count = 0;
	if (sscanf(query, "%d", &offset) != 1)
		return;
	
	websWrite(stream,
		  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
		  "<html>\n" "<head>\n" "<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n", live_translate("lang_charset.set"));
	websWrite(stream, "<style type=\"text/css\">\n body { font-size: small; } \n</style>\n</head>\n");
	websWrite(stream, "<body>\n<fieldset><legend>System Log</legend>");
	
	if( nvram_match("syslogd_enable", "1") ){
		FILE *fp = fopen ( filename, "r" );
		if ( fp != NULL )
		{
			char line [1024];
			websWrite(stream,"<div style=\"height:740px; overflow-y:auto;\"><table>");
			while ( fgets ( line, sizeof line, fp ) != NULL )
			{
				count++;
				if( offset < count && ((offset + 50) > count) ) // show 100 lines
				// a few sample colors
				if( strstr(line, "authpriv.info") ){	
					websWrite(stream,"<tr bgcolor=\"#FFFF00\"><td>%s</td></tr>", line);
				}else if(strstr(line, "authpriv.notice") ){
					websWrite(stream,"<tr bgcolor=\"#7CFC00\"><td>%s</td></tr>", line);
				}else if(strstr(line, "mounting unchecked fs") || strstr(line, "httpd login failure")){
					websWrite(stream,"<tr bgcolor=\"#FF0000\"><td>%s</td></tr>", line);
				}else{
					websWrite(stream,"<tr><td>%s</td></tr>", line);
				}
			
			}
			websWrite(stream,"</table></div>");
		
			fclose(fp);
		}
	}else{
		websWrite(stream,"<table><tr align=\"center\"><td>No messages available! Syslogd is not enabled!</td></tr></table>");
	}
	websWrite(stream,"</fieldset><p></body></html>");
	return;
}

static void do_ttgraph(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
#define COL_WIDTH 16		/* single column width */

	char *next;
	char var[80];

	unsigned int days;
	unsigned int month;
	unsigned int year;
	int wd;
	int i = 0;
	char months[12][12] = {
		"share.jan", "share.feb", "share.mar", "share.apr", "share.may",
		"share.jun",
		"share.jul", "share.aug", "share.sep", "share.oct",
		"share.nov", "share.dec"
	};
	unsigned long rcvd[31] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	unsigned long sent[31] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	unsigned long max = 5, smax = 5, f = 1;
	unsigned long totin = 0;
	unsigned long totout = 0;

	if (sscanf(query, "%u-%u", &month, &year) != 2)
		return;
	if (month < 1 || month > 12)
		return;

	days = daysformonth(month, year);
	wd = weekday(month, 1, year);	// first day in month (mon=0, tue=1,
	// ..., sun=6)

	char tq[32];

	sprintf(tq, "traff-%02u-%u", month, year);
	char *tdata = nvram_safe_get(tq);

	if (tdata != NULL && strlen(tdata)) {
		foreach(var, tdata, next) {
			if (i == days)
				break;	//skip monthly total
			int ret = sscanf(var, "%lu:%lu", &rcvd[i], &sent[i]);
			if (ret != 2)
				break;
			totin += rcvd[i];
			totout += sent[i];
			if (rcvd[i] > max)
				max = rcvd[i];
			if (sent[i] > max)
				max = sent[i];
			i++;
		}
	}

	while (max > smax) {
		if (max > (f * 5))
			smax = f * 10;
		if (max > (f * 10))
			smax = f * 25;
		if (max > (f * 25))
			smax = f * 50;
		f = f * 10;
	}

	char incom[32];

	snprintf(incom, 32, "%s", live_translate("status_inet.traffin"));
	char outcom[32];

	snprintf(outcom, 32, "%s", live_translate("status_inet.traffout"));
	char monthname[32];

	snprintf(monthname, 32, "%s", live_translate(months[month - 1]));

	websWrite(stream,
		  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
		  "<html>\n" "<head>\n" "<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n", live_translate("lang_charset.set"));

	websWrite(stream, "<title>dd-wrt traffic graph</title>\n"
		  "<script type=\"text/javascript\">\n"
		  "//<![CDATA[\n"
		  "function Show(label) {\n"
		  "document.getElementById(\"label\").innerHTML = label;\n" "}\n" "//]]>\n" "</script>\n" "<style type=\"text/css\">\n\n" "#t-graph {position: relative; width: %upx; height: 300px;\n", days * COL_WIDTH);
	websWrite(stream, "  margin: 1.1em 0 3.5em; padding: 0;\n"
		  "  border: 1px solid gray; list-style: none;\n"
		  "  font: 9px Tahoma, Arial, sans-serif;}\n" "#t-graph ul {margin: 0; padding: 0; list-style: none;}\n" "#t-graph li {position: absolute; bottom: 0; width: %dpx; z-index: 2;\n", COL_WIDTH);
	websWrite(stream, "  margin: 0; padding: 0;\n"
		  "  text-align: center; list-style: none;}\n"
		  "#t-graph li.day {height: 298px; padding-top: 2px; border-right: 1px dotted #C4C4C4; color: #AAA;}\n"
		  "#t-graph li.day_sun {height: 298px; padding-top: 2px; border-right: 1px dotted #C4C4C4; color: #E00;}\n"
		  "#t-graph li.bar {width: 4px; border: 1px solid; border-bottom: none; color: #000;}\n"
		  "#t-graph li.bar p {margin: 5px 0 0; padding: 0;}\n" "#t-graph li.rcvd {left: 3px; background: #228B22;}\n" "#t-graph li.sent {left: 8px; background: #CD0000;}\n");

	for (i = 0; i < days - 1; i++) {
		websWrite(stream, "#t-graph #d%d {left: %dpx;}\n", i + 1, i * COL_WIDTH);
	}
	websWrite(stream, "#t-graph #d%u {left: %upx; border-right: none;}\n", days, (days - 1) * COL_WIDTH);

	websWrite(stream, "#t-graph #ticks {width: %upx; height: 300px; z-index: 1;}\n", days * COL_WIDTH);
	websWrite(stream, "#t-graph #ticks .tick {position: relative; border-bottom: 1px solid #BBB; width: %upx;}\n", days * COL_WIDTH);
	websWrite(stream,
		  "#t-graph #ticks .tick p {position: absolute; left: 100%%; top: -0.67em; margin: 0 0 0 0.5em;}\n"
		  "#t-graph #label {width: 500px; bottom: -20px;  z-index: 1; font: 12px Tahoma, Arial, sans-serif; font-weight: bold;}\n" "</style>\n" "</head>\n\n" "<body>\n" "<ul id=\"t-graph\">\n");

	for (i = 0; i < days; i++) {
		websWrite(stream, "<li class=\"day%s\" id=\"d%d\" ", (wd % 7) == 6 ? "_sun" : "", i + 1);
		wd++;
		websWrite(stream, "onmouseover=\"Show(\'%s %d, %d (%s: %lu MB / %s: %lu MB)\')\" ", monthname, i + 1, year, incom, rcvd[i], outcom, sent[i]);
		websWrite(stream, "onmouseout=\"Show(\'%s %d (%s: %lu MB / %s: %lu MB)\')\"", monthname, year, incom, totin, outcom, totout);
		websWrite(stream, ">%d\n", i + 1);
		websWrite(stream, "<ul>\n");
		websWrite(stream, "<li class=\"rcvd bar\" style=\"height: %lupx;\"><p></p></li>\n", rcvd[i] * 300 / smax);
		websWrite(stream, "<li class=\"sent bar\" style=\"height: %lupx;\"><p></p></li>\n", sent[i] * 300 / smax);
		websWrite(stream, "</ul>\n");
		websWrite(stream, "</li>\n");
	}

	websWrite(stream, "<li id=\"ticks\">\n");
	for (i = 5; i; i--)	// scale
	{
		websWrite(stream, "<div class=\"tick\" style=\"height: 59px;\"><p>%d%sMB</p></div>\n", smax * i / 5, (smax > 10000) ? " " : "&nbsp;");
	}
	websWrite(stream, "</li>\n\n");

	websWrite(stream, "<li id=\"label\">\n");
	websWrite(stream, "%s %d (%s: %lu MB / %s: %lu MB)\n", monthname, year, incom, totin, outcom, totout);
	websWrite(stream, "</li>\n" "</ul>\n\n" "</body>\n" "</html>\n");

}

static void ttraff_backup(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
	system2("echo TRAFF-DATA > /tmp/traffdata.bak");
	system2("nvram show | grep traff- >> /tmp/traffdata.bak");
	do_file_attach(handler, "/tmp/traffdata.bak", stream, NULL, "traffdata.bak");
	unlink("/tmp/traffdata.bak");
} static void do_apply_cgi(struct mime_handler *handler, char *url, webs_t stream, char *q)
{
	char *path, *query;

	if (post == 1) {
		query = post_buf;
		path = url;
	} else {
		query = url;
		path = strsep(&query, "?") ? : url;
		init_cgi(query);
	}

	if (!query)
		return;

	apply_cgi(stream, NULL, NULL, 0, url, path, query);
	init_cgi(NULL);
}

#ifdef HAVE_MADWIFI
extern int getdevicecount(void);
#endif

#ifdef HAVE_LANGUAGE
static void do_language(struct mime_handler *handler, char *path, webs_t stream, char *query)	// jimmy, 
									// https, 
									// 8/4/2003
{
	char *langname = getLanguageName();
	char *prefix, *lang;

	prefix = safe_malloc(strlen(path) - strlen("lang_pack/language.js") + 1);
	memset(prefix, 0, strlen(path) - strlen("lang_pack/language.js") + 1);
	strncpy(prefix, path, strlen(path) - strlen("lang_pack/language.js"));

	lang = safe_malloc(strlen(prefix) + strlen(langname) + 1);
	sprintf(lang, "%s%s", prefix, langname);
	do_file(handler, lang, stream, NULL);

	free(prefix);
	free(lang);
	free(langname);
	return;
}
#endif
extern int issuperchannel(void);

static char no_cache[] = "Cache-Control: no-cache\r\n" "Pragma: no-cache\r\n" "Expires: 0";

struct mime_handler mime_handlers[] = {
	// { "ezconfig.asp", "text/html", ezc_version, do_apply_ezconfig_post,
	// do_ezconfig_asp, do_auth },
#ifdef HAVE_SKYTRON
	{"setupindex*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
#endif
#ifdef HAVE_POKER
	{"PokerEdit.asp", "text/html", no_cache, NULL, do_ej, NULL, 1},
#endif
#ifdef HAVE_DDLAN
	{"Upgrade*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Management*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Services*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Hotspot*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Wireless*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"WL_*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"WPA*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Log*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Alive*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Diagnostics*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Wol*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Factory_Defaults*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"config*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
#endif

	{"changepass.asp", "text/html", no_cache, NULL, do_ej, NULL, 1},
#ifdef HAVE_REGISTER
	{"register.asp", "text/html", no_cache, NULL, do_ej, do_auth_reg, 1},
#endif
	{"WL_FilterTable*", "text/html", no_cache, NULL, do_filtertable,
	 do_auth, 1},
#ifdef HAVE_FREERADIUS
	{"FreeRadiusCert*", "text/html", no_cache, NULL, do_radiuscert, do_auth,
	 1},
	{"freeradius-certs/*", "application/octet-stream", no_cache, NULL,
	 cert_file_out, do_auth, 0},
#endif
	// #endif
	// #ifdef HAVE_MADWIFI
	{"Wireless_WDS*", "text/html", no_cache, NULL, do_wds, do_auth, 1},
	{"WL_ActiveTable*", "text/html", no_cache, NULL, do_activetable,
	 do_auth, 1},
	{"Wireless_Advanced*", "text/html", no_cache, NULL, do_wireless_adv,
	 do_auth, 1},
	// #endif
	{"MyPage.asp*", "text/html", no_cache, NULL, do_mypage, do_auth, 1},
	{"**.asp", "text/html", no_cache, NULL, do_ej, do_auth, 1},
	{"**.JPG", "image/jpeg", no_cache, NULL, do_file, NULL, 0},
	// {"style.css", "text/css", NULL, NULL, do_style, NULL},
	{"common.js", "text/javascript", NULL, NULL, do_file, NULL, 0},
#ifdef HAVE_LANGUAGE
	{"lang_pack/language.js", "text/javascript", NULL, NULL, do_language,
	 NULL, 0},
#endif
#ifdef HAVE_BUFFALO
	{"intatstart/lang_pack/language.js", "text/javascript", NULL, NULL, do_language,
	 NULL, 0},
	{"vsp.html", "text/plain", no_cache, NULL, do_vsp_page, NULL, 1},
#endif
	{"SysInfo.htm*", "text/plain", no_cache, NULL, do_ej, do_auth, 1},
#ifdef HAVE_SKYTRON
	{"Info.htm*", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"Info.live.htm", "text/html", no_cache, NULL, do_ej, do_auth, 1},
	{"**.htm", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
	{"**.html", "text/html", no_cache, NULL, do_ej, do_auth2, 1},
#else
	{"Info.htm*", "text/html", no_cache, NULL, do_ej, do_cauth, 1},
	{"Info.live.htm", "text/html", no_cache, NULL, do_ej, do_cauth, 1},
	{"**.htm", "text/html", no_cache, NULL, do_ej, NULL, 1},
	{"**.html", "text/html", no_cache, NULL, do_ej, NULL, 1},

#endif
#ifdef HAVE_ROUTERSTYLE
	{"style/blue/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
	{"style/cyan/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
	{"style/elegant/style.css", "text/css", NULL, NULL, do_stylecss, NULL,
	 1},
	{"style/green/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
	{"style/orange/style.css", "text/css", NULL, NULL, do_stylecss, NULL,
	 1},
	{"style/red/style.css", "text/css", NULL, NULL, do_stylecss, NULL, 1},
	{"style/yellow/style.css", "text/css", NULL, NULL, do_stylecss, NULL,
	 1},
	{"style/blue/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
	 NULL,
	 1},
	{"style/cyan/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
	 NULL,
	 1},
	{"style/elegant/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
	 NULL, 1},
	{"style/green/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
	 NULL,
	 1},
	{"style/orange/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
	 NULL, 1},
	{"style/red/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie, NULL,
	 1},
	{"style/yellow/style_ie.css", "text/css", NULL, NULL, do_stylecss_ie,
	 NULL, 1},
#endif
#ifdef HAVE_REGISTER
	{"style/logo.png", "image/png", NULL, NULL, do_trial_logo, NULL, 0},
#endif
	{"**.css", "text/css", NULL, NULL, do_file, NULL, 0},
	{"**.svg", "image/svg+xml", NULL, NULL, do_file, NULL, 0},
	{"**.gif", "image/gif", NULL, NULL, do_file, NULL, 0},
	{"**.png", "image/png", NULL, NULL, do_file, NULL, 0},
	{"**.jpg", "image/jpeg", NULL, NULL, do_file, NULL, 0},
	{"**.ico", "image/x-icon", NULL, NULL, do_file, NULL, 0},
	{"**.js", "text/javascript", NULL, NULL, do_file, NULL, 0},
	{"**.swf", "application/x-shockwave-flash", NULL, NULL, do_file, NULL,
	 0},
	{"**.pdf", "application/pdf", NULL, NULL, do_file, NULL, 0},
	{"**.mp4", "video/mp4", NULL, NULL, do_file, NULL, 0},
	{"**.mp3", "audio/mpeg3", NULL, NULL, do_file, NULL, 0},
	{"**.mpg", "video/mpeg", NULL, NULL, do_file, NULL, 0},
	{"**.avi", "video/x-msvideo", NULL, NULL, do_file, NULL, 0},
	{"**.wma", "audio/x-ms-wma", NULL, NULL, do_file, NULL, 0},
	{"**.wmv", "video/x-ms-wmv", NULL, NULL, do_file, NULL, 0},
	{"**.flv", "video/x-flv", NULL, NULL, do_file, NULL, 0},
#ifdef HAVE_SKYTRON
	{"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
	 do_auth2, 1},
#elif HAVE_DDLAN
	{"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
	 NULL, 1},
#else
	{"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
	 do_auth, 1},
#endif
	{"fetchif.cgi*", "text/html", no_cache, NULL, do_fetchif, do_auth, 1},
#ifdef HAVE_DDLAN
	{"apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, NULL,
	 1},
	{"upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
	 NULL, 1},
#else
	{"apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
	 do_auth, 1},
	{"upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
	 do_auth, 1},
#endif
#ifdef HAVE_BUFFALO
	{"olupgrade.cgi*", "text/html", no_cache, do_olupgrade_post,
	 do_upgrade_cgi,
	 do_auth, 1},
#endif
	// {"Gozila.cgi*", "text/html", no_cache, NULL, do_setup_wizard,
	// do_auth}, // for setup wizard
	/*
	 * { "**.cfg", "application/octet-stream", no_cache, NULL, do_backup,
	 * do_auth }, 
	 */
#ifdef HAVE_DDLAN
	{"restore.cgi**", "text/html", no_cache, do_upgrade_post,
	 do_upgrade_cgi,
	 NULL, 1},
#else
	{"restore.cgi**", "text/html", no_cache, do_upgrade_post,
	 do_upgrade_cgi,
	 do_auth, 1},
#endif
	{"test.bin**", "application/octet-stream", no_cache, NULL, do_file,
	 do_auth, 0},

#ifdef HAVE_DDLAN
	{"nvrambak.bin*", "application/octet-stream", no_cache, NULL,
	 nv_file_out, do_auth2, 0},
	{"nvrambak**.bin*", "application/octet-stream", no_cache, NULL,
	 nv_file_out,
	 do_auth2, 0},
	{"nvram.cgi*", "text/html", no_cache, nv_file_in, sr_config_cgi, NULL,
	 1},
#else
	{"nvrambak.bin*", "application/octet-stream", no_cache, NULL,
	 nv_file_out, do_auth, 0},
	{"nvrambak**.bin*", "application/octet-stream", no_cache, NULL,
	 nv_file_out,
	 do_auth, 0},
	{"nvram.cgi*", "text/html", no_cache, nv_file_in, sr_config_cgi,
	 do_auth,
	 1},
#endif
#if !defined(HAVE_X86) && !defined(HAVE_MAGICBOX)
	{"backup/cfe.bin", "application/octet-stream", no_cache, NULL,
	 do_cfebackup,
	 do_auth, 0},
#endif
	{"syslog.cgi*", "text/html", no_cache, NULL, do_syslog, do_auth, 1}, 
	{"ttgraph.cgi*", "text/html", no_cache, NULL, do_ttgraph, do_auth, 1},
	{"traffdata.bak*", "text/html", no_cache, NULL, ttraff_backup,
	 do_auth, 0},
	{"tadmin.cgi*", "text/html", no_cache, td_file_in, td_config_cgi,
	 do_auth, 1},
	{"*", "application/octet-stream", no_cache, NULL, do_file, do_auth, 1},
	// for ddm
	{NULL, NULL, NULL, NULL, NULL, NULL, 0}
};

/*
 * Format: type = SET : " " => "&nbsp;" , ":" => "&semi;" type = GET :
 * "&nbsp;" => " " , "&semi;" => ":" Example: name1 = test 123:abc
 * filter_name("name1", new_name, SET); new_name="test&nbsp;123&semi;abc"
 * name2 = test&nbsp;123&semi;abc filter_name("name2", new_name, GET);
 * new_name="test 123:abc" 
 */
int httpd_filter_name(char *old_name, char *new_name, size_t size, int type)
{
	int i, j, match;

	cprintf("httpd_filter_name\n");

	struct pattern {
		char ch;
		char *string;
	};

	struct pattern patterns[] = {
		{' ', "&nbsp;"},
		{':', "&semi;"},
		{'<', "&lt;"},
		{'>', "&gt;"},
	};

	struct pattern *v;

	strcpy(new_name, "");

	switch (type) {
	case SET:
		for (i = 0; *(old_name + i); i++) {
			match = 0;
			for (v = patterns; v < &patterns[STRUCT_LEN(patterns)]; v++) {
				if (*(old_name + i) == v->ch) {
					if (strlen(new_name) + strlen(v->string) > size) {	// avoid overflow
						cprintf("%s(): overflow\n", __FUNCTION__);
						new_name[strlen(new_name)] = '\0';
						return 1;
					}
					sprintf(new_name + strlen(new_name), "%s", v->string);
					match = 1;
					break;
				}
			}
			if (!match) {
				if (strlen(new_name) + 1 > size) {
					cprintf("%s(): overflow\n", __FUNCTION__);	// avoid 
					// overflow
					new_name[strlen(new_name)] = '\0';
					return 1;
				}
				sprintf(new_name + strlen(new_name), "%c", *(old_name + i));
			}
		}

		break;
	case GET:
		for (i = 0, j = 0; *(old_name + j); j++) {
			match = 0;
			for (v = patterns; v < &patterns[STRUCT_LEN(patterns)]; v++) {
				if (!memcmp(old_name + j, v->string, strlen(v->string))) {
					*(new_name + i) = v->ch;
					j = j + strlen(v->string) - 1;
					match = 1;
					break;
				}
			}
			if (!match)
				*(new_name + i) = *(old_name + j);

			i++;
		}
		*(new_name + i) = '\0';
		break;
	default:
		cprintf("%s():Invalid type!\n", __FUNCTION__);
		break;
	}
	// cprintf("%s():new_name=[%s]\n", __FUNCTION__, new_name);

	return 1;
}

#ifdef HAVE_BUFFALO
void do_vsp_page(struct mime_handler *handler, char *url, webs_t stream, char *query)
{
/*
#ifdef HAVE_MADWIFI
	char *ifname = "ath0";
#else
	char *ifname = "wl0";
#endif

	char *authmode = nvram_nget("%s_security_mode", ifname);
	char *encrypt = nvram_nget("%s_crypto", ifname);
	char *wpakey = nvram_nget("%s_wpa_psk ", ifname);

	if (!strcmp(encrypt, "tkip")) {
		encrypt = "TKIP";
	} else if (!strcmp(authmode, "aes")) {
		encrypt = "AES";
	} else if (!strcmp(authmode, "tkip+aes")) {
		encrypt = "TKIP-AES-MIX";
	} else {
		encrypt = "";
	}

	if (!strcmp(authmode, "disabled")) {
		authmode = "NONE";
		encrypt = "";
		wpakey = "";
	} else if (!strcmp(authmode, "wep")) {
		authmode = "WEP";
		encrypt = "";
		wpakey = "";
	} else if (!strcmp(authmode, "radius")
		   || !strcmp(authmode, "wpa")
		   || !strcmp(authmode, "wpa2")
		   || !strcmp(authmode, "wpa wpa2")) {
		authmode = "RADIUS";
		encrypt = "";
		wpakey = "";
	} else if (!strcmp(authmode, "8021X")) {
		authmode = "802.1X";
		encrypt = "";
		wpakey = "";
	} else if (!strcmp(authmode, "psk")) {
		authmode = "WPA";
	} else if (!strcmp(authmode, "psk2")) {
		authmode = "WPA2";
	} else if (!strcmp(authmode, "psk psk2")) {
		authmode = "WPA-WPA2-MIX";
	} else {
		authmode = "UNKNOWN";
		encrypt = "";
		wpakey = "";
	}
*/
	websWrite(stream, "<html>\n");
	websWrite(stream, "<head>\n");
	websWrite(stream, "<title>VSP</title>\n");
	websWrite(stream, "</head>\n");
	websWrite(stream, "<body>\n");
	websWrite(stream, "<pre>\n");

	websWrite(stream, "DEVICE_VSP_VERSION=0.1\n");
	websWrite(stream, "DEVICE_VENDOR=BUFFALO INC.\n");
	websWrite(stream, "DEVICE_MODEL=%s DDWRT\n", nvram_safe_get("DD_BOARD"));
	websWrite(stream, "DEVICE_FIRMWARE_VERSION=1.00\n");
	char *reg = getUEnv("region");
	if (!reg)
		reg = "US";
	websWrite(stream, "DEVICE_REGION=%s\n", reg);
	websWrite(stream, "WIRELESS_DEVICE_NUMBER=1\n");
//      websWrite(stream, "WIRELESS_1_PRESET_AUTHMODE=%s\n", authmode);
//      websWrite(stream, "WIRELESS_1_PRESET_ENCRYPT=%s\n", encrypt);
//      websWrite(stream, "WIRELESS_1_PRESET_ENCRYPT_KEY=%s\n", wpakey);
	websWrite(stream, "DEVICE_URL_GET=/vsp.html\n");
	websWrite(stream, "DEVICE_URL_SET=/vsp.html\n");

	websWrite(stream, "</pre>\n");
	websWrite(stream, "</body>\n");
	websWrite(stream, "</html>\n");
}
#endif
