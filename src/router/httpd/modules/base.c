/*
 * DD-WRT base.c (derived originally from broadcom.c WRT54G linksys gpl source. but honestly, there is nothing left of it)
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License
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
#ifdef WEBS
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
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
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <dd_defs.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#define sys_stats(url) eval("stats", (url))

// tofu
int do_file(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);
static void send_headers(webs_t conn_fp, int status, char *title, char *extra_header, char *mime_type, int length,
			 char *attach_file, int nocache);
static int do_file_attach(struct mime_handler *handler, char *path, webs_t stream, char *attachment);
static int do_upgrade_cgi(unsigned char method, struct mime_handler *handler, char *url, webs_t stream);
static int start_validator(char *name, webs_t wp, char *value, struct variable *v);
char *websGetVar(webs_t wp, char *var, char *d);
int websGetVari(webs_t wp, char *var, int d);
static void start_gozila(char *name, webs_t wp);
static void *start_validator_nofree(char *name, void *handle, webs_t wp, char *value, struct variable *v);
static int do_upgrade_post(char *url, webs_t stream, size_t len, char *boundary);
int wfsendfile(int fd, off_t offset, size_t nbytes, webs_t wp);
char *wfgets(char *buf, int len, webs_t fp, int *eof);
size_t wfwrite(void *buf, size_t size, size_t n, webs_t fp);
size_t wfread(void *buf, size_t size, size_t n, webs_t fp);
int wfclose(webs_t fp);
int wfflush(webs_t fp);
int wfputs(char *buf, webs_t fp);
/* Basic authorization userid and passwd limit */

static void send_authenticate(webs_t conn_fp);

char *live_translate(webs_t wp, const char *tran);
#ifdef HAVE_BUFFALO
int do_vsp_page(unsigned char method, struct mime_handler *handler, char *url, webs_t stream);
#endif
/*
 * Deal with side effects before committing
 */
static int _sys_commit(int noasync)
{
	if (nvram_matchi("dhcpnvram", 1)) {
		killall("dnsmasq", SIGUSR2); // update lease -- tofu
	}
	if (noasync)
		return _nvram_commit();
	else
		return nvram_async_commit();
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
 * vars arent defined anymore statically
 */

#include <stdlib.h>
#include <malloc.h>
#include <dirent.h>
#include <stdlib.h>

static void StringStart(FILE *in)
{
	while (getc(in) != '"') {
		if (feof(in))
			return;
	}
}

static char *getFileString(FILE *in)
{
	char *buf;
	int i, b;

	buf = safe_malloc(1024);
	if (!buf)
		return NULL;
	StringStart(in);
	for (i = 0; i < 1024; i++) {
		b = getc(in);
		if (b == EOF) {
			debug_free(buf);
			return NULL;
		}
		if (b == '"') {
			buf[i] = 0;
			buf = realloc(buf, strlen(buf) + 1);
			return buf;
		}
		buf[i] = b;
	}
	return buf;
}

static void skipFileString(FILE *in)
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

static char *directories[] = { "/etc/config", "/jffs/etc/config", "/mmc/etc/config" };

struct SIMPLEVAL {
	char *name;
	char *validator;
	int args;
};

static void checkError(FILE *in, char *name, int line)
{
	int showonce = 0;
	while (!feof(in)) {
		int c = getc(in);
		if (c == EOF || c == 0xa)
			break;
		if (c == 0x20 || c == '\t')
			continue;
		if (!showonce) {
			showonce = 1;
			dd_syslog(LOG_ERR,
				  "Error in FILE %s on line %d, this may lead to strange effects like non working save actions\n",
				  name, line + 1);
		}
	}
}

static struct variable **variables;
void Initnvramtab()
{
	struct dirent *entry;
	DIR *directory;
	FILE *in;
	int varcount = 0, len, i;
	char *tmpstr;
	int line;
	struct variable *tmp;
	static struct SIMPLEVAL simpleval[] = {
		{ "WMEPARAM", "validate_wl_wme_params", 0 },
		{ "WMETXPARAM", "validate_wl_wme_tx_params", 0 },
		{ "WANIPADDR", "validate_wan_ipaddr", 0 },
		{ "MERGEREMOTEIP", "validate_remote_ip", 0 },
		{ "MERGEIPADDRS", "validate_merge_ipaddrs", 0 },
		{ "MERGEDHCPSTART", "validate_merge_dhcpstart", 0 },
		{ "DNS", "validate_dns", 0 },
		{ "SAVEWDS", "save_wds", 0 },
		{ "DHCP", "validate_dhcp_check", 0 },
		{ "STATICS", "validate_statics", 0 },
#ifdef HAVE_PORTSETUP
		{ "PORTSETUP", "validate_portsetup", 0 },
#endif
#ifdef HAVE_MDNS
		{ "AVAHI", "validate_avahi", 0 },
#endif
#ifdef HAVE_OPENVPN
		{ "OPENVPNUSERPASS", "validate_openvpnuserpass", 0 },
#endif
		{ "REBOOT", "validate_reboot", 0 },
		{ "IPADDR", "validate_ipaddr", 0 },
		{ "STATICLEASES", "validate_staticleases", 0 },
#ifdef HAVE_CHILLI
#ifdef HAVE_CHILLILOCAL
		{ "USERLIST", "validate_userlist", 0 },
#endif
#endif
#ifdef HAVE_RADLOCAL
		{ "IRADIUSUSERLIST", "validate_iradius", 0 },
#endif
		{ "IPADDRS", "validate_ipaddrs", 0 },
		{ "NETMASK", "validate_netmask", 0 },
		{ "MERGENETMASK", "validate_merge_netmask", 0 },
		{ "WDS", "validate_wds", 0 },
		{ "STATICROUTE", "validate_static_route", 0 },
#ifndef HAVE_MICRO
		{ "PBRRULE", "validate_pbr_rule", 0 },
#endif
		{ "MERGEMAC", "validate_merge_mac", 0 },
		{ "FILTERPOLICY", "validate_filter_policy", 0 },
		{ "FILTERIPGRP", "validate_filter_ip_grp", 0 },
		{ "FILTERPORT", "validate_filter_port", 0 },
		{ "FILTERDPORTGRP", "validate_filter_dport_grp", 0 },
		{ "BLOCKEDSERVICE", "validate_blocked_service", 0 },
		{ "FILTERP2P", "validate_catchall", 0 },
		{ "FILTERMACGRP", "validate_filter_mac_grp", 0 },
		{ "FILTERWEB", "validate_filter_web", 0 },
		{ "WLHWADDRS", "validate_wl_hwaddrs", 0 },
		{ "FORWARDPROTO", "validate_forward_proto", 0 },
		{ "FORWARDSPEC", "validate_forward_spec", 0 },
		{ "FORWARDIP", "validate_forward_ip", 0 },
		{ "PORTTRIGGER", "validate_port_trigger", 0 },
		{ "HWADDR", "validate_hwaddr", 0 },
		{ "HWADDRS", "validate_hwaddrs", 0 },
		{ "WLWEPKEY", "validate_wl_wep_key", 0 },
#ifdef HAVE_PPPOESERVER
		{ "CHAPTABLE", "validate_chaps", 0 },
#endif
#ifdef HAVE_MILKFISH
		{ "MFSUBSCRIBERS", "validate_subscribers", 0 },
		{ "MFALIASES", "validate_aliases", 0 },
#endif
		{ "RANGE", "validate_range", 2 },
		{ "CHOICE", "validate_choice", -1 },
		{ "NOACK", "validate_noack", 2 },
		{ "NAME", "validate_name", 1 },
		{ "PASSWORD", "validate_password", 1 },
		{ "PASSWORD2", "validate_password2", 1 },
		{ "LANIPADDR", "validate_lan_ipaddr", 1 },
		{ "WPAPSK", "validate_wpa_psk", 1 },
		{ "WLAUTH", "validate_wl_auth", 2 },
		{ "WLWEP", "validate_wl_wep", -1 },
		{ "DYNAMICROUTE", "validate_dynamic_route", -1 },
		{ "WLGMODE", "validate_wl_gmode", -1 },
		{ "WLNETMODE", "validate_wl_net_mode", -1 },
		{ "AUTHMODE", "validate_auth_mode", -1 },
		{ "SECURITYMODE", "validate_auth_mode", -1 },
		{ NULL, NULL },
	};

	variables = NULL;
	char *buf;

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
				asprintf(&buf, "%s/%s", directories[idx], entry->d_name);
				in = fopen(buf, "rb");
				line = 0;
				if (in == NULL) {
					debug_free(buf);
					return;
				}
				while (1) {
					tmp = (struct variable *)calloc(sizeof(struct variable), 1);
					if (!tmp)
						break;
					tmp->name = getFileString(in);
					if (tmp->name == NULL)
						break;
					skipFileString(in); // long string
					tmpstr = getFileString(in);
					if (!tmpstr)
						break;
					tmp->argv = NULL;
					if (!strcasecmp(tmpstr, "NULL")) {
					}
#ifdef HAVE_SPUTNIK_APD
					if (!strcasecmp(tmpstr, "MJIDTYPE")) {
						tmp->validatename = "validate_choice";
						debug_free(tmpstr);
						tmpstr = getFileString(in);
						len = atoi(tmpstr);
						tmp->argv = (char **)safe_malloc(sizeof(char **) * (len + 1));
						for (i = 0; i < len; i++) {
							tmp->argv[i] = getFileString(in);
						}
						tmp->argv[i] = NULL;
						nvram_seti("sputnik_rereg", 1);
					}
#endif
					if (tmp->validatename == NULL) {
						int scount = 0;
						while (simpleval[scount].name != NULL) { //
							if (!strcasecmp(tmpstr,
									simpleval[scount].name)) { //
								//                                                              fprintf(stderr,"match %s %s\n",tmpstr,tmp->name);
								tmp->validatename = simpleval[scount].validator; //
								int arglen = 0;
								if (simpleval[scount].args == -1) { //
									debug_free(tmpstr);
									tmpstr = getFileString(in); //
									arglen = atoi(tmpstr); //
								}
								if (simpleval[scount].args > 0) { //
									arglen = simpleval[scount].args; //
								}
								if (arglen) { //
									tmp->argv = (char **)safe_malloc(sizeof(char **) *
													 (arglen + 1)); //
									for (i = 0; i < arglen; i++) { //
										tmp->argv[i] = getFileString(in); //
									}
									tmp->argv[arglen] = NULL; //
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
					debug_free(tmpstr);
					tmpstr = getFileString(in);
					if (!strcasecmp(tmpstr, "TRUE")) {
						tmp->nullok = TRUE;
					} else {
						tmp->nullok = FALSE;
					}
					debug_free(tmpstr);
					skipFileString(in); // todo: remove it
					checkError(in, buf, line++);
					// tmpstr = getFileString (in);
					// tmp->ezc_flags = atoi (tmpstr);
					// free (tmpstr);
					variables =
						(struct variable **)realloc(variables, sizeof(struct variable **) * (varcount + 2));
					variables[varcount++] = tmp;
					variables[varcount] = NULL;
				}
				debug_free(buf);
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
static int variables_arraysize(void)
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

static int calclength(char *webfile, char *ifname)
{
	int weblen = strlen(webfile);
	int len = 0;
	int i;
	int iflen = strlen(ifname);
	for (i = 0; i < weblen - 1; i++) {
		if (webfile[i] == '%') {
			if (webfile[i + 1] == '%') {
				i += 2;
				len += 2;
				continue;
			}
			if (webfile[i + 1] == 'd') {
				len--;
			}
			if (webfile[i + 1] == 's') {
				len += iflen;
				len -= 2; // substract size for %s
			}
		}
		len++;
	}
	return len + 1;
}

static void filteralphanum(char *str)
{
	int len = strlen(str);
	int i;
	for (i = 0; i < len; i++) {
		if (!isalnum(str[i]) && str[i] != '.')
			str[i] = 0;
	}
}

static char *_tran_string(char *buf, size_t len, char *str)
{
	snprintf(buf, len - 1, "<script type=\"text/javascript\">Capture(%s)</script>", str);
	return buf;
}

static char *readweb(webs_t wp, char *filename)
{
	size_t len;
	FILE *web = _getWebsFile(wp, filename, &len);
	if (!web) {
		return NULL;
	}
	char *webfile = (char *)safe_malloc(len + 1);
	if (!webfile) {
		fclose(web);
		return NULL;
	}
	fread(webfile, len, 1, web);
	fclose(web);
	webfile[len] = 0;
	return webfile;
}

static char *insert(webs_t wp, char *ifname, char *index, char *filename)
{
	char *webfile = readweb(wp, filename);
	if (!webfile)
		return NULL;
	int weblen = strlen(webfile);
	int i;
	int ai = 0;
	int length = calclength(webfile, ifname);
	char *temp = calloc(length + 4, 1);
	for (i = 0; i < weblen; i++) {
		if (webfile[i] == '%') {
			i++;
			switch (webfile[i]) {
			case '%':
				temp[ai++] = '%';
				break;
			case 'd':
				if (index && strlen(index)) {
					strcpy(&temp[ai], index);
					ai++;
				}
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
	debug_free(webfile);
	return temp;
}

/* bigfile.bin download method used for benchmarking. use http://x.x.x.x/bigfile.bin?size=FILESIZE to request any filesize you want */
static int do_bigfile(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char fs[128];
	char *parameter = "s=";
	char *parameter2 = "u=";
	if (!nvram_matchi("speedchecker_enable", 1))
		return -1;
	bzero(fs, sizeof(fs));
	char *idx = strchr(path, '?');
	if (idx) {
		strlcpy(fs, idx, sizeof(fs) - 1);
		fs[strlen(fs)] = 0;
	} else {
		return -1;
	}
	char *size = strstr(fs, parameter);
	if (!size)
		return -1; // size = NULL if parameter hasnt been found
	long long filesize = 0;
	char *s_fs = size + strlen(parameter); //skip s=
	char *uuid;
	idx = strchr(s_fs, '&');
	if (idx) {
		idx[0] = 0;
		uuid = idx + 1;
		if (strlen(uuid) <= strlen(parameter2))
			return -1;
		if (strncmp(uuid, "u=", 2))
			return -1;
		//syslog(LOG_INFO, "UUID: %s", uuid);
		uuid += strlen(parameter2);
		idx = strchr(uuid, '&');
		if (idx) {
			idx[0] = 0;
		}
		//syslog(LOG_INFO, "UUIX: %s", uuid);
		if (strlen(uuid) == 36) {
			if (!nvram_match("speedchecker_uuid2", uuid)) {
				return -1;
			}
		} else
			return -1;
	} else {
		return -1;
	}
	int b;
	for (b = 0; b < strlen(s_fs); b++)
		if (s_fs[b] < '0' || s_fs[b] > '9')
			return -1;
	filesize = atoll(s_fs);
	if (!filesize || filesize < 0) //if argument is not numeric or invalid, just return with no action
		return -1;
	long i;
	char *extra;
	char *options = "Access-Control-Allow-Origin: *\r\n" //
			"Access-Control-Allow-Headers: Origin,X-RequestedWith,Content-Type,Range,Authorization\r\n" //
			"Access-Control-Allow-Methods: GET,OPTIONS\r\nAccept-Ranges: *"; //

	if (handler->extra_header)
		asprintf(&extra, "%s\r\n%s", options, handler->extra_header);
	else
		asprintf(&extra, "%s", options);
	if (method == METHOD_OPTIONS) {
		send_headers(
			stream, 200, "OK", extra, handler->mime_type, 0, NULL,
			1); // special case if call was for OPTIONS and not GET, so we return the requested header with zero body size
		goto ret;
	} else {
		send_headers(stream, 200, "OK", extra, handler->mime_type, filesize, "bigfile.bin", 1);
	}
	// send body in 64 KiB chunks based on random values
	FILE *fp;
	long long i64;
	long long sz = filesize / 65536;
	if (!f_exists("/tmp/bigfilemem.bin")) {
		char *test = malloc(65536);
		srand(time(NULL));
		for (i = 0; i < 65536; i++)
			test[i] = rand() % 255;
		fp = fopen("/tmp/bigfilemem.bin", "wb");
		fwrite(test, 1, 65536, fp);
		fclose(fp);
		debug_free(test);
	}
	wfflush(stream);
	fp = fopen("/tmp/bigfilemem.bin", "rb");
	for (i64 = 0; i64 < sz; i64++) {
		wfsendfile(fileno(fp), 0, 65536, stream);
	}
	wfsendfile(fileno(fp), 0, filesize % 65536, stream);
	fclose(fp);

ret:;
	debug_free(extra);
	return 0;
}

static int do_redirect(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char redirect_path[128];
	snprintf(redirect_path, sizeof(redirect_path), "Location: %s", path);
	send_headers(stream, 302, "Found", redirect_path, "", -1, NULL, 1);
	return 0;
}

static int do_filtertable(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char ifname[32];
	char *temp2;
	stream->path = path;
	bzero(ifname, sizeof(ifname));
	char *idx = strchr(path, '-');
	if (idx) {
		temp2 = idx + 1;
		strlcpy(ifname, temp2, sizeof(ifname));
	}
	// and now the tricky part (more dirty as dirty)
	char *temp3 = websGetVar(stream, "ifname", NULL);
	if (temp3) {
		if (*(temp3)) {
			strlcpy(ifname, temp3, sizeof(ifname));
		}
	}
	filteralphanum(ifname);
	idx = strrchr(ifname, '.');
	if (idx)
		*idx = 0;
	if (!*(ifname))
		return -1;
	rep(ifname, '.', 'X');

	char *temp = insert(stream, ifname, "0", "WL_FilterTable.asp");
	if (!temp)
		return -1;
	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
	do_ej_buffer(temp, stream);
	debug_free(temp);
	return 0;
}

#ifdef HAVE_FREERADIUS
#include <radiusdb.h>

static int cert_file_out(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char *idx = strrchr(path, '/');
	if (!idx)
		return -1;

	char *temp2 = idx + 1;
	char link[128];
	if (!strcmp(temp2, "ca.pem"))
		sprintf(link, "/jffs/etc/freeradius/certs/ca.pem");
	else
		sprintf(link, "/jffs/etc/freeradius/certs/clients/%s", temp2);
	return do_file_attach(handler, link, stream, temp2);
}

static void show_certfield(webs_t wp, char *title, char *file)
{
	websWrite(
		wp,
		"<div class=\"setting\">\n<div class=\"label\">%s</div>\n"
		"<script type=\"text/javascript\">\n"
		"//<![CDATA[\n"
		"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"download_button\\\" style=\\\"float: right\\\" "
		"value=\\\"\" + sbutton.download + \"\\\" onclick=\\\"window.location.href='/freeradius-certs/%s';\\\" /><br />\");\n//]]>\n</script>\n</div>\n",
		title, file);
}

static int do_radiuscert(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char buf[128];
	stream->path = path;
	char *idx = strchr(path, '-');
	if (!idx)
		return -1;
	char *temp2 = idx + 1;
	char number[32];
	webs_t wp = stream;
	strlcpy(number, temp2, sizeof(number) - 1);
	idx = strrchr(number, '.');
	if (!idx)
		return -1;
	*idx = 0;
	int radiusindex = atoi(number);

	if (radiusindex == -1)
		return -1;
	struct radiusdb *db = loadradiusdb();
	if (db == NULL) // database empty
		return -1;
	if (radiusindex >= db->usercount) // index out of bound
	{
		goto out;
	}
	if (db->users[radiusindex].usersize == 0 || db->users[radiusindex].passwordsize == 0 ||
	    *(db->users[radiusindex].user) == 0 || *(db->users[radiusindex].passwd) == 0) {
		//define username fail
		char *argv[] = { "freeradius.clientcert" };
		call_ej("do_pagehead", NULL, wp, 1, argv); // thats dirty
		websWrite(wp,
			  "</head>\n"
			  "<body>\n"
			  "<div id=\"main\">\n"
			  "<div id=\"contentsInfo\" style=\"width: 360px\">\n"
			  "<h2>%s</h2>\n"
			  "Error: please specify a username and password.\n"
			  "<br /><br />\n"
			  "<div id=\"footer\" class=\"submitFooter\">\n"
			  "<script type=\"text/javascript\">\n"
			  "//<![CDATA[\n"
			  "submitFooterButton(0,0,0,0,0,1);\n"
			  "//]]>\n"
			  "</script>\n"
			  "</div>\n"
			  "</div>\n"
			  "</div>\n"
			  "</body>\n"
			  "\n",
			  _tran_string(buf, sizeof(buf), "freeradius.clientcert"));
		websWrite(stream, "</html>");
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

	if (generate) //do not regenerate certificates if they are already created
	{
		char expiration_days[64];
		strlcpy(expiration_days, nvram_safe_get("radius_expiration"), sizeof(expiration_days));
		long expiration = 0; //never
		if (db->users[radiusindex].expiration) {
			time_t tm;
			time(&tm);
			long curtime = ((tm / 60) / 60) / 24; //in days
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
			if (!serial || !output) {
				fclose(fp);
				return -1;
			}
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
			debug_free(output);
			debug_free(serial);
		}
		eval("/jffs/etc/freeradius/certs/doclientcert", expiration_days, nvram_safe_get("radius_country"),
		     nvram_safe_get("radius_state"), nvram_safe_get("radius_locality"), nvram_safe_get("radius_organisation"),
		     nvram_safe_get("radius_email"), db->users[radiusindex].user, db->users[radiusindex].passwd,
		     nvram_safe_get("radius_passphrase"));
	}
	char *argv[] = { "freeradius.clientcert" };
	call_ej("do_pagehead", NULL, wp, 1, argv); // thats dirty
	websWrite(wp,
		  "</head>\n"
		  "<body>\n"
		  "<div id=\"main\">\n"
		  "<div id=\"contentsInfo\" style=\"width: 360px\">\n"
		  "<h2>%s</h2>\n",
		  _tran_string(buf, sizeof(buf), "freeradius.clientcert"));
	sprintf(filename, "ca.pem");
	show_certfield(wp, "CA Certificate", filename);
	sprintf(filename, "%s-cert.pem", db->users[radiusindex].user);
	show_certfield(wp, "Certificate PEM", filename);
	sprintf(filename, "%s-cert.p12", db->users[radiusindex].user);
	show_certfield(wp, "Certificate P12 (Windows)", filename);
	sprintf(filename, "%s-req.pem", db->users[radiusindex].user);
	show_certfield(wp, "Certificate Request", filename);
	sprintf(filename, "%s-key.pem", db->users[radiusindex].user);
	show_certfield(wp, "Private Key PEM", filename);
	websWrite(wp, "<div id=\"footer\" class=\"submitFooter\">\n"
		      "<script type=\"text/javascript\">\n"
		      "//<![CDATA[\n"
		      "submitFooterButton(0,0,0,0,0,1);\n"
		      "//]]>\n"
		      "</script>\n"
		      "</div>\n"
		      "</div>\n"
		      "</div>\n"
		      "</body>\n");
	websWrite(wp, "</html>");

	//make certificates
out:;
	freeradiusdb(db);
	return 0;
}

#endif

#ifdef HAVE_ATH9K
static int do_spectral_scan(unsigned char method, struct mime_handler *handler, char *p, webs_t stream)
{
#define json_cache "/tmp/spectral_scan.json"
#define json_cache_timeout 2
	char *ifname = nvram_safe_get("wifi_display");
	int phy = mac80211_get_phyidx_by_vifname(ifname);
	char *path;

	if (is_ath11k(ifname))
		asprintf(&path, "/sys/kernel/debug/ieee80211/phy%d/ath11k", phy);
	else if (is_ath10k(ifname))
		asprintf(&path, "/sys/kernel/debug/ieee80211/phy%d/ath10k", phy);
	else
		asprintf(&path, "/sys/kernel/debug/ieee80211/phy%d/ath9k", phy);

	char dest[64];
	sprintf(dest, "%s/spectral_count", path);
	writestr(dest, nvram_default_get("spectral_count", "1"));
	sprintf(dest, "%s/spectral_scan0", path);
	FILE *fp = fopen(dest, "rb");
	while (!feof(fp))
		getc(fp);
	fclose(fp);
	if (is_ath10k(ifname) && has_wave2(ifname)) {
		sprintf(dest, "%s/spectral_bins", path);
		writestr(dest, "64");
		sprintf(dest, "%s/spectral_scan_ctl", path);
		writestr(dest, "manual");
		writestr(dest, "trigger");
	} else if (is_ath10k(ifname)) {
		sprintf(dest, "%s/spectral_bins", path);
		writestr(dest, "64");
		sprintf(dest, "%s/spectral_scan_ctl", path);
		writestr(dest, "manual");
		writestr(dest, "trigger");
	} else if (is_ath11k(ifname)) {
		sprintf(dest, "%s/spectral_bins", path);
		writestr(dest, "256");
		writestr(dest, "512");
		writestr(dest, "1024");
		sprintf(dest, "%s/spectral_scan_ctl", path);
		writestr(dest, "background");
		writestr(dest, "trigger");
	} else {
		sprintf(dest, "%s/spectral_scan_ctl", path);
		writestr(dest, "chanscan");
	}
	eval("iw", ifname, "scan");
	char *exec;
	asprintf(&exec, "fft_eval \"%s/spectral_scan0\"", path);

	fp = popen(exec, "rb");
	//      FILE *fp = fopen(json_cache, "rb");
	debug_free(exec);
	if (!fp) {
		debug_free(path);
		return -1;
	}
	char *buffer = malloc(65536 + 1);
	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
	websWrite(stream, "{ \"epoch\": %d, \"samples\":\n", time(NULL));
	int result = 0;
	while (!feof(fp)) {
		result = fread(buffer, 1, 65536, fp);
		buffer[result] = 0;
		websWrite(stream, "%s", buffer);
	}
	pclose(fp);
	sprintf(dest, "%s/spectral_scan_ctl", path);
	writestr(dest, "disable");

	debug_free(buffer);
	debug_free(path);

	websWrite(stream, "}");
	return 0;
}
#endif

static void getiffromurl(char *ifname, int len, char *ori_path)
{
	char *path = strdup(ori_path);
	bzero(ifname, len);
	char *idx = strchr(path, '-');
	if (idx) {
		char *temp2 = idx + 1;
		char *idx = strrchr(temp2, '.');
		if (!idx)
			return;
		*idx = 0;
		strlcpy(ifname, temp2, len - 1);
	}
	debug_free(path);
}

static int sanitize_ifname(char *ifname)
{
	if (!*(ifname) || strlen(ifname) < 2 || strlen(ifname) > 15)
		return -1;

	return 0;
}

static int do_activetable(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char ifname[32];
	stream->path = path;
	getiffromurl(ifname, sizeof(ifname), path);
	char *temp3 = websGetVar(stream, "ifname", NULL);
	if (temp3 != NULL) {
		if (*temp3) {
			strlcpy(ifname, temp3, sizeof(ifname));
		}
	}
	if (sanitize_ifname(ifname))
		return -1;
	filteralphanum(ifname);
	char *temp = insert(stream, ifname, "0", "WL_ActiveTable.asp");
	if (!temp)
		return -1;
	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
	do_ej_buffer(temp, stream);
	debug_free(temp);
	return 0;
}

static int do_sitesurvey(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char ifname[32];
	stream->path = path;
	getiffromurl(ifname, sizeof(ifname), path);
	filteralphanum(ifname);
	if (sanitize_ifname(ifname)) {
		bzero(ifname, sizeof(ifname));
	}
	char *temp = insert(stream, ifname, "0", "Site_Survey.asp");
	if (!temp)
		return -1;
	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
	do_ej_buffer(temp, stream);
	debug_free(temp);
	return 0;
}

static int do_wds(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char ifname[32];
	stream->path = path;
	getiffromurl(ifname, sizeof(ifname), path);
	if (!*(ifname))
		return -1;
	filteralphanum(ifname);

	if (sanitize_ifname(ifname))
		return -1;
	char *temp = insert(stream, ifname, "0", "Wireless_WDS.asp");
	if (!temp)
		return -1;
	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
	do_ej_buffer(temp, stream);
	debug_free(temp);
	return 0;
}

static int do_wireless_adv(unsigned char method, struct mime_handler *handler, char *path, webs_t stream)
{
	char ifname[32];
	stream->path = path;
	getiffromurl(ifname, sizeof(ifname), path);
	if (!*(ifname))
		return -1;
	filteralphanum(ifname);
	char index[32];
	int strl = strlen(ifname);
	if (strl > 2)
		substring(strl - 1, strl, ifname, index, sizeof(index));
	else
		return -1;
	if (!strlen(index) || strlen(ifname) < 2)
		return -1;
	char *temp = insert(stream, ifname, index, "Wireless_Advanced.asp");
	if (!temp)
		return -1;
	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
	do_ej_buffer(temp, stream);
	debug_free(temp);
	return 0;
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
		if ((!*value && variables[i]->nullok) || (!variables[i]->validate2name && !variables[i]->validatename))
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
	REFRESH_DELAY,
};
static struct gozila_action gozila_actions[] = {
	/*
	 * SETUP
	 */
	{ "index", "wan_proto", "", REFRESH, "wan_proto" },
	{ "index", "dhcpfwd", "", REFRESH, "dhcpfwd" },
//egc
#ifdef HAVE_OPENVPN
	//{ "PPTP", "import_vpntunnel", "", REFRESH, "import_vpntunnel" },
	//{ "PPTP", "import_vpntunnel", "", NOTHING, "import_vpntunnel" },
	{ "PPTP", "import_vpntunnel", "", REFRESH_DELAY, "import_vpntunnel" },
	{ "PPTP", "add_userpass", "", REFRESH, "userpass_add" },
	{ "PPTP", "del_userpass", "", REFRESH, "userpass_del" },

#endif
#ifdef HAVE_IAS
	{ "index", "admin_card", "", RESTART, "ias_save_admincard" },
#endif
// {"index", "clone_mac", "", REFRESH, clone_mac}, //OBSOLETE
#ifdef HAVE_CCONTROL
	{ "ccontrol", "execute", "", REFRESH, "execute" },
#endif
	{ "WanMAC", "clone_mac", "", REFRESH, "clone_mac" }, // for cisco
	// style
	{ "Status_Lan", "delete", "", REFRESH_DELAY, "delete_leases" },
	{ "Status_Lan", "static", "", REFRESH_DELAY, "static_leases" },
#ifdef HAVE_PPTPD
	{ "Status_Lan", "deletepptp", "", REFRESH, "delete_pptp" },
#endif
	{ "Info", "refresh", "", REFRESH, "save_wifi" },
	{ "Status_Wireless", "refresh", "", REFRESH, "save_wifi" },
	// {"Status", "release", "dhcp_release", SYS_RESTART, "dhcp_release"},
	// {"Status", "renew", "", REFRESH, "dhcp_renew"},
	// {"Status", "Connect", "start_pppoe", RESTART, NULL},
	{ "Status_Internet", "release", "dhcp_release", SERVICE_RESTART, "dhcp_release" }, // for
	{ "Status_Internet", "renew", "", REFRESH, "dhcp_renew" }, // for cisco
	{ "Status_Internet", "Disconnect", "stop_pppoe", SERVICE_RESTART, "stop_ppp" }, // for
#ifdef HAVE_3G
	{ "Status_Internet", "Connect_3g", "start_3g", RESTART, NULL }, // for
	{ "Status_Internet", "Disconnect_3g", "stop_3g", SERVICE_RESTART, "stop_ppp" }, // for
#endif
#ifdef HAVE_PPPOATM
	{ "Status_Internet", "Connect_pppoa", "start_pppoa", RESTART, NULL }, // for
	{ "Status_Internet", "Disconnect_pppoa", "stop_pppoa", SERVICE_RESTART, "stop_ppp" }, // for
#endif
	{ "Status_Internet", "Connect_pppoe", "start_pppoe", RESTART, NULL }, // for
	{ "Status_Internet", "Disconnect_pppoe", "stop_pppoe", SERVICE_RESTART, "stop_ppp" }, // for
#ifdef HAVE_SPEEDTEST_CLI
	{ "Status_Internet", "speedtest", "speedtest", SERVICE_RESTART, NULL }, // for
#endif

	{ "Status_Internet", "Connect_pptp", "start_pptp", RESTART, NULL }, // for
	{ "Status_Internet", "Disconnect_pptp", "stop_pptp", SERVICE_RESTART, "stop_ppp" }, // for
	{ "Status_Internet", "Connect_l2tp", "start_l2tp", RESTART, NULL }, // for
	{ "Status_Internet", "Disconnect_l2tp", "stop_l2tp", SERVICE_RESTART, "stop_ppp" }, // for
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
	{ "Status_Internet", "Disconnect_heartbeat", "stop_heartbeat", SERVICE_RESTART, "stop_ppp" }, // for
	// cisco
	// style
	{ "Status_Internet", "delete_ttraffdata", "", REFRESH, "ttraff_erase" },
	{ "Filters", "save", "filters", REFRESH, "save_policy" },
	{ "Filters", "delete", "filters", REFRESH, "single_delete_policy" },
	{ "FilterSummary", "delete", "filters", REFRESH, "summary_delete_policy" },
	{ "Routing", "del", "static_route_del", REFRESH, "delete_static_route" },
	{ "RouteStatic", "del", "static_route_del", REFRESH, "delete_static_route" },
#ifndef HAVE_MICRO
	{ "Routing", "del_rule", "pbr_rule_del", REFRESH, "delete_pbr_rule" },
	{ "RouteStatic", "del_rule", "pbr_rule_del", REFRESH, "delete_pbr_rule" },
#endif
	{ "WL_WPATable", "wep_key_generate", "", REFRESH, "generate_wep_key" },
	{ "WL_WPATable", "security", "", REFRESH, "set_security" },
	{ "WL_WPATable", "save", "wireless_2", REFRESH, "security_save" },
	{ "WL_WPATable", "keysize", "wireless_2", REFRESH, "security_save" },
#ifdef HAVE_80211R
	{ "Roaming", "save", "wireless_2", REFRESH, "roaming_save" },
#endif
	{ "WL_ActiveTable", "add_mac", "", REFRESH, "add_active_mac" },
	/*
	 * Siafu addition
	 */
	{ "Wol", "wol", "", REFRESH, "ping_wol" },
/*
	 * Sveasoft addition
	 */
// {"Wireless_WDS", "save", "", REFRESH, save_wds},
#ifndef HAVE_MADWIFI
	{ "Wireless_WDS-wl0", "save", "wireless_2", REFRESH, "save_wds" },
	{ "Wireless_WDS-wl1", "save", "wireless_2", REFRESH, "save_wds" },
	{ "Wireless_WDS-wl2", "save", "wireless_2", REFRESH, "save_wds" },
	{ "Wireless_Advanced-wl0", "save", "wireless_2", REFRESH, "save_wireless_advanced" },
	{ "Wireless_Advanced-wl1", "save", "wireless_2", REFRESH, "save_wireless_advanced" },
	{ "Wireless_Advanced-wl2", "save", "wireless_2", REFRESH, "save_wireless_advanced" },
#else
	{ "Wireless_WDS-wlan0", "save", "wireless_2", REFRESH, "save_wds" },
	{ "Wireless_WDS-wlan1", "save", "wireless_2", REFRESH, "save_wds" },
	{ "Wireless_WDS-wlan2", "save", "wireless_2", REFRESH, "save_wds" },
	{ "Wireless_WDS-wlan3", "save", "wireless_2", REFRESH, "save_wds" },
#endif
	{ "Diagnostics", "startup", "", REFRESH, "save_startup" },
	{ "Diagnostics", "shutdown", "", REFRESH, "save_shutdown" },
	{ "Diagnostics", "firewall", "", SYS_RESTART, "save_firewall" },
	{ "Diagnostics", "custom", "", REFRESH, "save_custom" },
	{ "Diagnostics", "usb", "", REFRESH, "save_usb" },
	{ "QoS", "add_svc", "", REFRESH, "qos_add_svc" },
	{ "QoS", "add_ip", "", REFRESH, "qos_add_ip" },
	{ "QoS", "add_mac", "", REFRESH, "qos_add_mac" },
	{ "QoS", "add_dev", "", REFRESH, "qos_add_dev" },
	{ "QoS", "save", "filters", REFRESH, "qos_save" },
	{ "QoS", "del_qossvcs", "", REFRESH, "qossvcs_del" },
	{ "QoS", "del_qosmacs", "", REFRESH, "qosmacs_del" },
	{ "QoS", "del_qosips", "", REFRESH, "qosips_del" },
	{ "QoS", "del_qosdevs", "", REFRESH, "qosdevs_del" },

	/*
	 * end Sveasoft addition
	 */
	{ "Forward", "add_forward", "", REFRESH, "forward_add" },
	{ "Forward", "del_forward", "", REFRESH, "forward_del" },
	{ "Filters", "add_filter", "", REFRESH, "filter_add" },
	{ "Filters", "remove_filter", "", REFRESH, "filter_remove" },
	{ "Filters", "sel_filter", "", REFRESH, "sel_filter" },
	{ "Wireless_Basic", "add_vifs", "", REFRESH, "add_vifs" },
	{ "Wireless_Basic", "remove_vifs", "", REFRESH, "remove_vifs" },
	{ "Wireless_Basic", "copy_if", "", REFRESH, "copy_if" },
	{ "Wireless_Basic", "paste_if", "", REFRESH, "paste_if" },
#ifdef HAVE_FREERADIUS
	{ "FreeRadius", "generate_certificate", "", REFRESH, "radius_generate_certificate" },
	{ "FreeRadius", "add_radius_user", "", REFRESH, "add_radius_user" },
	{ "FreeRadius", "del_radius_user", "", REFRESH, "del_radius_user" },
	{ "FreeRadius", "add_radius_client", "", REFRESH, "add_radius_client" },
	{ "FreeRadius", "del_radius_client", "", REFRESH, "del_radius_client" },
	{ "FreeRadius", "save_radius_user", "", REFRESH, "save_radius_user" },
#endif
#ifdef HAVE_POKER
	{ "Poker", "add_poker_user", "", REFRESH, "add_poker_user" },
	{ "Poker", "del_poker_user", "", REFRESH, "del_poker_user" },
	{ "Poker", "save_poker_user", "", REFRESH, "save_poker_user" },
	{ "PokerEdit", "poker_loaduser", "", REFRESH, "poker_loaduser" },
	{ "PokerEdit", "poker_checkout", "", REFRESH, "poker_checkout" },
	{ "PokerEdit", "poker_buy", "", REFRESH, "poker_buy" },
	{ "PokerEdit", "poker_credit", "", REFRESH, "poker_credit" },
	{ "PokerEdit", "poker_back", "", REFRESH, "poker_back" },
#endif
	{ "Vlan", "portvlan_add", "", REFRESH, "portvlan_add" },
	{ "Vlan", "portvlan_remove", "", REFRESH, "portvlan_remove" },
#ifdef HAVE_BONDING
	{ "Networking", "add_bond", "", REFRESH, "add_bond" },
	{ "Networking", "del_bond", "", REFRESH, "del_bond" },
#endif
#ifdef HAVE_OLSRD
	{ "Routing", "add_olsrd", "", REFRESH, "add_olsrd" },
	{ "Routing", "del_olsrd", "", REFRESH, "del_olsrd" },
#endif
#ifdef HAVE_VLANTAGGING
	{ "Networking", "add_vlan", "", REFRESH, "add_vlan" },
	{ "Networking", "add_bridge", "", REFRESH, "add_bridge" },
	{ "Networking", "add_bridgeif", "", REFRESH, "add_bridgeif" },
	{ "Networking", "del_vlan", "", REFRESH, "del_vlan" },
	{ "Networking", "del_bridge", "", REFRESH, "del_bridge" },
	{ "Networking", "del_bridgeif", "", REFRESH, "del_bridgeif" },
	{ "Networking", "save_networking", "index", REFRESH, "save_networking" },
	{ "Networking", "add_mdhcp", "", REFRESH, "add_mdhcp" },
	{ "Networking", "del_mdhcp", "", REFRESH, "del_mdhcp" },
#endif
#ifdef HAVE_IPVS
	{ "Networking", "add_ipvs", "", REFRESH, "add_ipvs" },
	{ "Networking", "del_ipvs", "", REFRESH, "del_ipvs" },
	{ "Networking", "add_ipvstarget", "", REFRESH, "add_ipvstarget" },
	{ "Networking", "del_ipvstarget", "", REFRESH, "del_ipvstarget" },
#endif
	{ "Wireless_Basic", "save", "wireless", REFRESH, "wireless_save" },
#ifdef HAVE_WIVIZ
	{ "Wiviz_Survey", "Set", "", REFRESH, "set_wiviz" },
#endif
#ifdef HAVE_REGISTER
	{ "Register", "activate", "", RESTART, "reg_validate" },
#endif
	{ "index", "changepass", "", REFRESH, "changepass" },
#ifdef HAVE_SUPERCHANNEL
	{ "SuperChannel", "activate", "", REFRESH, "superchannel_validate" },
#endif
	{ "Services", "add_lease", "", REFRESH, "lease_add" },
	{ "Services", "del_lease", "", REFRESH, "lease_del" },
#ifdef HAVE_SSHD
	{ "Services", "ssh_downloadkey", "", REFRESH, "ssh_downloadkey" },
#endif
#ifdef HAVE_PPPOESERVER
	{ "PPPoE_Server", "add_chap_user", "", REFRESH, "chap_user_add" },
	{ "PPPoE_Server", "remove_chap_user", "", REFRESH, "chap_user_remove" },
#endif
#ifdef HAVE_CHILLI
#ifdef HAVE_CHILLILOCAL
	{ "Hotspot", "add_user", "", REFRESH, "user_add" },
	{ "Hotspot", "remove_user", "", REFRESH, "user_remove" },
#endif
#endif
#ifdef HAVE_RADLOCAL
	{ "Hotspot", "add_iradius", "", REFRESH, "raduser_add" },
#endif
#ifdef HAVE_EOP_TUNNEL
#ifdef HAVE_WIREGUARD
	{ "eop-tunnel", "gen_wg_key", "", REFRESH, "gen_wg_key" },
	{ "eop-tunnel", "gen_wg_psk", "", REFRESH, "gen_wg_psk" },
	{ "eop-tunnel", "gen_wg_client", "", REFRESH, "gen_wg_client" },
	{ "eop-tunnel", "del_wg_client", "", REFRESH, "del_wg_client" },
	{ "eop-tunnel", "add_peer", "", REFRESH, "add_peer" },
	{ "eop-tunnel", "del_peer", "", REFRESH, "del_peer" },
	{ "eop-tunnel", "import_tunnel", "", REFRESH, "import_tunnel" },
#endif
	{ "eop-tunnel", "add_tunnel", "", REFRESH, "add_tunnel" },
	{ "eop-tunnel", "del_tunnel", "", REFRESH, "del_tunnel" },
	{ "eop-tunnel", "save", "eop", REFRESH, "tunnel_save" },
#endif
	{ "ForwardSpec", "add_forward_spec", "", REFRESH, "forwardspec_add" },
	{ "ForwardSpec", "del_forward_spec", "", REFRESH, "forwardspec_del" },
	{ "ForwardIP", "add_forward_ip", "", REFRESH, "forwardip_add" },
	{ "ForwardIP", "del_forward_ip", "", REFRESH, "forwardip_del" },
	{ "Triggering", "add_trigger", "", REFRESH, "trigger_add" },
	{ "Triggering", "del_trigger", "", REFRESH, "trigger_del" },
	{ "Port_Services", "save_services", "filters", REFRESH, "save_services_port" },
	{ "QOSPort_Services", "save_qosservices", "filters", REFRESH, "save_services_port" },
	{ "Diagnostics", "start", "", SERVICE_RESTART, "diag_ping_start" },
	{ "Diagnostics", "stop", "", REFRESH, "diag_ping_stop" },
	{ "Diagnostics", "clear", "", REFRESH, "diag_ping_clear" },
#ifdef HAVE_MILKFISH
	{ "Milkfish_database", "add_milkfish_user", "", REFRESH, "milkfish_user_add" },
	{ "Milkfish_database", "remove_milkfish_user", "", REFRESH, "milkfish_user_remove" },
	{ "Milkfish_aliases", "add_milkfish_alias", "", REFRESH, "milkfish_alias_add" },
	{ "Milkfish_aliases", "remove_milkfish_alias", "", REFRESH, "milkfish_alias_remove" },
	{ "Milkfish_messaging", "send_message", "", SERVICE_RESTART, "milkfish_sip_message" },
#endif
#ifdef HAVE_STATUS_GPIO
	{ "Gpio", "gpios_save", "", REFRESH, "gpios_save" },
#endif
#ifdef HAVE_BUFFALO
	{ "SetupAssistant", "save", "setupassistant", REFRESH, "setupassistant_save" },
	{ "SetupAssistant", "wep_key_generate", "setupassistant", REFRESH, "generate_wep_key" },
	{ "SetupAssistant", "security", "setupassistant", REFRESH, "set_security" },
	{ "SetupAssistant", "keysize", "setupassistant", REFRESH, "security_save" },
	{ "Upgrade", "get_upgrades", "firmware", REFRESH, "get_airstation_upgrades" },
#ifdef HAVE_IAS
	{ "InternetAtStart", "proceed", "internetatstart", REFRESH, "internetatstart" },
	{ "InternetAtStart.ajax", "ajax", "intatstart_ajax", REFRESH, "intatstart_ajax" },
#endif
#ifdef HAVE_SPOTPASS
	{ "Nintendo", "save", "spotpass", REFRESH, "nintendo_save" },
#endif
#endif
	{ "Join", "Join", "wireless", REFRESH, "wireless_join" },
#ifdef HAVE_NAS_SERVER
	{ "NAS", "save", "nassrv", REFRESH, "nassrv_save" },
#endif
#ifdef HAVE_MINIDLNA
	{ "NAS", "save", "nassrv", REFRESH, "dlna_save" },
#endif
#ifdef HAVE_RAID
	{ "NAS", "add_raid", "nassrv", REFRESH, "add_raid" },
	{ "NAS", "del_raid", "nassrv", REFRESH, "del_raid" },
	{ "NAS", "add_raid_member", "nassrv", REFRESH, "add_raid_member" },
	{ "NAS", "del_raid_member", "nassrv", REFRESH, "del_raid_member" },
	{ "NAS", "format_raid", "nassrv", REFRESH, "format_raid" },
	{ "NAS", "format_drive", "nassrv", REFRESH, "format_drive" },
	{ "NAS", "raid_save", "nassrv", REFRESH, "raid_save" },
#ifdef HAVE_ZFS
	{ "NAS", "zfs_scrub", "nassrv", REFRESH, "zfs_scrub" },
#endif
#endif
#if defined(HAVE_WPS) || defined(HAVE_AOSS)
	{ "AOSS", "save", "aoss", REFRESH, "aoss_save" },
	{ "AOSS", "start", "aoss", REFRESH, "aoss_start" },
#ifdef HAVE_WPS
	{ "AOSS", "wps_register", "aoss", REFRESH, "wps_register" },
	{ "AOSS", "wps_ap_register", "aoss", REFRESH, "wps_ap_register" },
	{ "AOSS", "wps_forcerelease", "aoss", REFRESH, "wps_forcerelease" },
	{ "AOSS", "wps_configure", "aoss", REFRESH, "wps_configure" },
#endif
#endif
#ifdef HAVE_SYSCTL_EDIT
	{ "Sysctl", "save", "sysctl", REFRESH, "sysctl_save" },
#endif
};

static struct gozila_action *handle_gozila_action(char *name, char *type)
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

static int gozila_cgi(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, char_t *url, char_t *path,
		      struct mime_handler *handler)
{
	char *submit_button, *submit_type, *next_page;
	int action = REFRESH;
	struct gozila_action *act;

	wp->gozila_action = 1;
	submit_button = websGetVar(wp, "submit_button", NULL); /* every html
								 * must have
								 * the name */
	submit_type = websGetVar(wp, "submit_type", NULL); /* add, del,
								 * renew,
								 * release
								 * ..... */
	dd_logdebug("httpd", "submit_button=[%s] submit_type=[%s]\n", submit_button, submit_type);
	act = handle_gozila_action(submit_button, submit_type);

	if (act) {
		dd_logdebug("httpd", "name=[%s] type=[%s] service=[%s] action=[%d]\n", act->name, act->type, act->service,
			    act->action);
		action = act->action;
		if (act->goname) {
			start_gozila(act->goname, wp);
		}

		if (!nvram_exists("nowebaction")) {
			addAction(act->service);
		} else
			nvram_unset("nowebaction");
	} else {
		action = REFRESH;
	}

	if (action == SERVICE_RESTART) {
		_sys_commit(0);
		service_restart();
	} else if (action == SYS_RESTART) {
		_sys_commit(0);
		sys_restart();
	} else if (action == RESTART) {
		_sys_commit(0);
		sys_restart();
	} else if (action == REFRESH_DELAY) {
		struct timespec tim, tim2;
		tim.tv_sec = 1;
		tim.tv_nsec = 0;
		nanosleep(&tim, &tim2);
	}

	next_page = websGetVar(wp, "next_page", NULL);
	char newpath[128];
	if (next_page && *next_page != '/' && *next_page != '.') {
		sprintf(newpath, "%s", next_page);
	} else
		sprintf(newpath, "%s.asp", submit_button);
	if (!strncmp(newpath, "WL_FilterTable", 14))
		do_filtertable(METHOD_GET, handler, newpath, wp); // refresh
#ifdef HAVE_FREERADIUS
	else if (!strncmp(newpath, "FreeRadiusCert", 14))
		do_radiuscert(METHOD_GET, handler, newpath, wp); // refresh
#endif
	// #ifdef HAVE_MADWIFI
	else if (!strncmp(newpath, "WL_ActiveTable", 14))
		do_activetable(METHOD_GET, handler, newpath, wp); // refresh
	else if (!strncmp(newpath, "Wireless_WDS", 12))
		do_wds(METHOD_GET, handler, newpath, wp); // refresh
	// #endif
	else if (!strncmp(newpath, "Wireless_Advanced", 17))
		do_wireless_adv(METHOD_GET, handler, newpath, wp); // refresh
	else
		do_ej(METHOD_GET, handler, newpath, wp); // refresh

#ifdef HAVE_ANTAIRA
	// @markus. this is wrong here, it works as a hack but structural it should use handlers
	if (!strcmp(submit_type, "browser_date")) {
		int d = websGetVari(wp, "browser_ts", 0);
		if (d != 0) {
			char cmd[32];
			snprintf(&cmd, 32, "date -s '@%d'", d);
			system(cmd);
			system("hwclock -w -u");
		}
	}
#endif

	websDone(wp, 200);

	wp->gozila_action = 0;
	wp->p->generate_key = 0;
	wp->p->clone_wan_mac = 0;

	return 1;
}

static struct apply_action apply_actions[] = {
	/*
	 * name, service, action, function_to_execute
	 */

	/*
	 * SETUP
	 */
	{ "index", "index", SERVICE_RESTART, NULL },
	{ "DDNS", "ddns", SERVICE_RESTART, "ddns_save_value" },
	{ "Routing", "routing", SERVICE_RESTART, NULL },
	{ "Vlan", "", SYS_RESTART, "port_vlan_table_save" },
	{ "eop-tunnel", "eop", SERVICE_RESTART, NULL },

	/*
	 * WIRELESS
	 */
	{ "Wireless_Basic", "wireless", SERVICE_RESTART, NULL }, // Only for
	// V23, since
	// V24 it's a
	// gozilla
	// save
	{ "Wireless_Advanced-wl0", "wireless_2", SERVICE_RESTART, "save_wireless_advanced" },
	{ "Wireless_Advanced-wl1", "wireless_2", SERVICE_RESTART, "save_wireless_advanced" },
	{ "Wireless_Advanced-wl2", "wireless_2", SERVICE_RESTART, "save_wireless_advanced" },
	{ "Wireless_MAC", "wireless_2", SERVICE_RESTART, "save_macmode" },
	{ "WL_FilterTable", "macfilter", SERVICE_RESTART, NULL },
	{ "Wireless_WDS", "wireless_2", SERVICE_RESTART, NULL },
	{ "WL_WPATable", "wireless_2", SERVICE_RESTART, NULL },

	/*
	 * MANAGEMENT
	 */
	{ "Management", "management", SYS_RESTART, NULL },
	{ "Services", "services", SERVICE_RESTART, NULL },
	{ "Alive", "alive", SERVICE_RESTART, NULL },
#ifdef HAVE_SYSCTL_EDIT
	{ "Sysctl", "sysctl", SERVICE_RESTART, "sysctl_save" },
#endif

	/*
	 * SERVICES
	 */
	{ "PPPoE_Server", "services", SERVICE_RESTART, NULL },
	{ "PPTP", "pptp", SERVICE_RESTART, NULL },
	{ "USB", "usbdrivers", SERVICE_RESTART, NULL },
	{ "NAS", "nassrv", SERVICE_RESTART, NULL },
	{ "Hotspot", "hotspot", SERVICE_RESTART, "hotspot_save" },
	{ "Hotspot", "hotspot", SERVICE_RESTART, NULL },
	//      {"AnchorFree", "anchorfree", SERVICE_RESTART, NULL},
	{ "Nintendo", "nintendo", SERVICE_RESTART, NULL },

	/*
	 * APP & GAMING
	 */
	{ "Forward", "forward", SERVICE_RESTART, NULL },
	{ "ForwardSpec", "forward", SERVICE_RESTART, NULL },
	{ "Triggering", "filters", SERVICE_RESTART, NULL },
	{ "DMZ", "filters", SERVICE_RESTART, NULL },
	{ "Filters", "filters", SERVICE_RESTART, NULL },
	{ "FilterIPMAC", "filters", SERVICE_RESTART, NULL },
#ifdef HAVE_UPNP
	{ "UPnP", "forward_upnp", SERVICE_RESTART, "tf_upnp" },
#endif
	/*
	 * SECURITY
	 */
	{ "Firewall", "filters", SERVICE_RESTART, NULL },
	{ "VPN", "filters", SERVICE_RESTART, NULL },
#ifdef HAVE_MILKFISH
	{ "Milkfish", "milkfish", SERVICE_RESTART, NULL },
#endif
#ifdef HAVE_IPV6
	{ "IPV6", "ipv6", SERVICE_RESTART, NULL },
#endif
	/*
	 * Obsolete {"WL_WEPTable", "", SERVICE_RESTART, NULL}, {"Security",
	 * "", RESTART, NULL}, {"System", "", RESTART, NULL}, {"DHCP",
	 * "dhcp", SERVICE_RESTART, NULL}, {"FilterIP", "filters",
	 * SERVICE_RESTART, NULL}, {"FilterMAC", "filters", SERVICE_RESTART,
	 * NULL}, {"FilterPort", "filters", SERVICE_RESTART, NULL},
	 * {"Wireless", "wireless", SERVICE_RESTART, NULL}, {"Log", "logging",
	 * 0, SERVICE_RESTART, NULL}, //moved to Firewall {"QoS", "qos",
	 * SERVICE_RESTART, NULL}, //gozilla does the save
	 */
	{ "InternetAtStart", "finish", SYS_RESTART, NULL },
#ifdef HAVE_SPEEDCHECKER
	{ "Speedchecker", "speedchecker", SERVICE_RESTART, NULL },
#endif

};

static struct apply_action *handle_apply_action(char *name)
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

static void do_logout(webs_t conn_fp) // static functions are not exportable,
	// additionally this is no ej function
{
	send_authenticate(conn_fp);
}

static void apply_cgi(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, char_t *url, char_t *path, char *query,
		      struct mime_handler *handler)
{
	int action = NOTHING;
	char *value;
	char *submit_button, *next_page;

	cprintf("need reboot\n");
	int need_reboot = websGetVari(wp, "need_reboot", 0);
	if (*nvram_safe_get("ctf_disable")) {
		char *sfe = websGetVar(wp, "sfe", NULL);
		char *fa = websGetVar(wp, "ctf_fa_mode", NULL);
		char *wan_proto = websGetVar(wp, "wan_proto", NULL);
		char *wshaper_enable = websGetVar(wp, "wshaper_enable", NULL);
		if (wan_proto && !nvram_match("wan_proto", wan_proto)) {
			if (strcmp(wan_proto, "static") && strcmp(wan_proto, "dhcp") && (fa && strcmp(fa, "0"))) {
				need_reboot = 1;
			}
		}
		if (wshaper_enable && !nvram_match("wshaper_enable", wshaper_enable)) {
			if (!strcmp(wshaper_enable, "1") && !nvram_match("ctf_fa_mode", "0"))
				need_reboot = 1;
		}
		if (fa && nvram_geti("ctf_fa_mode") != atoi(fa))
			need_reboot = 1;
		if (need_reboot)
			nvram_seti("do_reboot", 1);
	}
	if (*nvram_safe_get("vlans")) {
		char *vlans = websGetVar(wp, "vlans", NULL);
		if (vlans && nvram_match("vlans", "1")) {
			if (!strcmp(vlans, "0"))
				need_reboot = 1;
		}
	}
	cprintf("apply");

	/**********   get "change_action" and launch gozila_cgi if needed **********/

	value = websGetVar(wp, "change_action", "");
	cprintf("get change_action = %s\n", value);

	if (value && !strcmp(value, "gozila_cgi")) {
		dd_logdebug("httpd", "[GOZILLA_APPLY] %s %s %s\n", websGetVar(wp, "submit_button", NULL),
			    websGetVar(wp, "submit_type", NULL), websGetVar(wp, "call", "no call defined"));
		gozila_cgi(wp, urlPrefix, webDir, arg, url, path, handler);
		return;
	}

	/***************************************************************************/
	if (!query) {
		goto footer;
	}
	if (legal_ip_netmask("lan_ipaddr", "lan_netmask", wp->http_client_ip) == TRUE)
		wp->browser_method = USE_LAN;
	else
		wp->browser_method = USE_WAN;

	/**********   get all webs var **********/

	submit_button = websGetVar(wp, "submit_button", "");
	cprintf("get submit_button = %s\n", submit_button);

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
		nvram_seti("is_default", 0);
		nvram_seti("is_modified", 1);
		if (act) {
			dd_logdebug("httpd", "%s:submit_button=[%s] service=[%s] action=[%d]\n", value, act->name, act->service,
				    act->action);

			if ((act->action == SYS_RESTART) || (act->action == SERVICE_RESTART)) {
				if (!nvram_exists("nowebaction")) {
					addAction(act->service);
				} else
					nvram_unset("nowebaction");
			}
			action = act->action;

			if (act->goname)
				start_gozila(act->goname, wp);
		} else {
			// nvram_set ("action_service", "");
			action = RESTART;
		}
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
		diag_led(DIAG, STOP_LED);
#endif
		if (!strcmp(value, "ApplyTake")) {
			_sys_commit(0);
		}
	}

	/** Restore defaults **/
	else if (!strncmp(value, "Restore", 7)) {
		ACTION("ACT_SW_RESTORE");
		nvram_seti("sv_restore_defaults", 1);
#ifdef HAVE_BUFFALO_SA
		int region_sa = 0;
		if (nvram_default_match("region", "SA", ""))
			region_sa = 1;
#endif
		killall("udhcpc", SIGKILL);
#ifdef HAVE_TMK
#ifdef HAVE_CAMBRIA
		eval("/sbin/kmtdefaults.sh", "noreboot");
		do_ej(METHOD_GET, NULL, "Reboot.asp", wp);
		websDone(wp, 200);
		eval("reboot");
		eval("event", "5", "1", "15");
#endif
#endif
#ifdef HAVE_X86
#ifdef HAVE_ERC
		eval("nvram", "restore", "/etc/defaults/x86ree.backup");
		eval("reboot");
		eval("event", "5", "1", "15");
#endif
		char drive[64];
		char *d = getdisc();
		sprintf(drive, "/dev/%s", d);
		free(d);
		FILE *in = fopen(drive, "r+b");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftello(in);
		fseeko(in, mtdlen - (65536 * 2), SEEK_SET);
		int i;
		for (i = 0; i < 65536; i++)
			putc(0, in); // erase backup area
		fclose(in);
		eval("mount", "/usr/local", "-o", "remount,rw");
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram database
		unlink("/tmp/nvram/.lock"); // delete nvram database
		eval("rm", "-f", "/usr/local/nvram/*"); // delete nvram
		// database
		eval("mount", "/usr/local", "-o", "remount,ro");
		eval("sync");
#elif HAVE_RB600
		char drive[64];
		sprintf(drive, "/dev/sda");
		FILE *in = fopen(drive, "r+b");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftello(in);
		fseeko(in, mtdlen - (65536 * 2), SEEK_SET);
		int i;
		for (i = 0; i < 65536; i++)
			putc(0, in); // erase backup area
		fclose(in);
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram database
		unlink("/tmp/nvram/.lock"); // delete nvram database
		eval("rm", "-f", "/usr/local/nvram/*"); // delete nvram
		eval("sync");
#elif HAVE_OPENRISC
#ifdef HAVE_ERC
		eval("cp", "-f", "/etc/defaults/nvram.bin", "/usr/local/nvram/nvram.bin");
		eval("sync");
		eval("sync");
		struct timespec tim, tim2;
		tim.tv_sec = 5;
		tim.tv_nsec = 0;
		nanosleep(&tim, &tim2);
		eval("event", "5", "1", "15");
#endif
		eval("mount", "/usr/local", "-o", "remount,rw");
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram database
		unlink("/tmp/nvram/.lock"); // delete nvram database
		eval("rm", "-f", "/usr/local/nvram/*"); // delete nvram
		// database
		eval("mount", "/usr/local", "-o", "remount,ro");
#elif HAVE_RB500
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram database
		unlink("/tmp/nvram/.lock"); // delete nvram database
		eval("rm", "-f", "/etc/nvram/*"); // delete nvram database
#elif HAVE_MAGICBOX
		eval("rm", "-f", "/tmp/nvram/*"); // delete nvram database
		unlink("/tmp/nvram/.lock"); // delete nvram database
		nvram_clear();
		nvram_commit();
#endif
#ifdef HAVE_BUFFALO_SA
		nvram_seti("sv_restore_defaults", 1);
		if (region_sa)
			nvram_set("region", "SA");
#endif
		_sys_commit(1);

		action = REBOOT;
	}

	/** Reboot **/
	else if (!strncmp(value, "Reboot", 6)) {
		action = REBOOT;
	}

	/** GUI Logout **/
	// Experimental, does not work yet ...
	else if (!strncmp(value, "Logout", 6)) {
		do_ej(METHOD_GET, NULL, "Logout.asp", wp);
		websDone(wp, 200);
		do_logout(wp);
		return;
	}

	/*
	 * DEBUG : Invalid action
	 */
	else
		websDebugWrite(wp, "Invalid action %s<br />", value);

footer:

	if (nvram_matchi("do_reboot", 1)) {
		action = REBOOT;
	}
	/*
	 * This will let PC to re-get a new IP Address automatically
	 */
	if (need_reboot)
		action = REBOOT;

	if (!strcmp(value, "Apply")) {
		action = NOTHING;
	}

	if (action != REBOOT) {
		next_page = websGetVar(wp, "next_page", NULL);
		char newpath[128];
		if (next_page)
			sprintf(newpath, "%s", next_page);
		else {
			sprintf(newpath, "%s.asp", submit_button);
		}
		do_redirect(METHOD_GET, handler, newpath, wp);
		websDone(wp, 200);
	} else {
#ifndef HAVE_WRK54G
		do_redirect(METHOD_GET, handler, "Reboot.asp", wp);
		websDone(wp, 200);
#endif
		sys_reboot();
		return;
	}

	nvram_set("upnp_wan_proto", "");
	if ((action == RESTART) || (action == SYS_RESTART))
		sys_restart();
	else if (action == SERVICE_RESTART)
		service_restart();

	return;
}

//int auth_check( char *dirname, char *authorization )
static int do_auth(webs_t wp, int (*auth_check)(webs_t conn_fp))
{
	strlcpy(wp->auth_userid, nvram_safe_get("http_username"), AUTH_MAX - 1);
	strlcpy(wp->auth_passwd, nvram_safe_get("http_passwd"), AUTH_MAX - 1);
	// strncpy(realm, MODEL_NAME, AUTH_MAX);
#if defined(HAVE_ERC) || defined(HAVE_IPR)
	strncpy(wp->auth_realm, "LOGIN", AUTH_MAX);
	wp->userid = 0;
	if (auth_check(wp))
		return 1;
	wp->userid = 1;
	char passout[MD5_OUT_BUFSIZE];
	strlcpy(wp->auth_userid, zencrypt("SuperAdmin", passout), AUTH_MAX - 1);
	strlcpy(wp->auth_passwd, nvram_safe_get("newhttp_passwd"), AUTH_MAX - 1);
	if (auth_check(wp))
		return 1;
	wp->userid = 0;
#else
	wp->userid = 0;
	strlcpy(wp->auth_realm, nvram_safe_get("router_name"), AUTH_MAX - 1);
	if (auth_check(wp))
		return 1;
#endif
	return 0;
}

static int do_cauth(webs_t wp, int (*auth_check)(webs_t conn_fp))
{
	if (nvram_matchi("info_passwd", 0))
		return 1;
	return do_auth(wp, auth_check);
}

static int do_auth_changepass(webs_t wp, int (*auth_check)(webs_t conn_fp))
{
	if ((nvram_match("http_username", DEFAULT_USER) && nvram_match("http_passwd", DEFAULT_PASS)))
		return 1;
	return do_auth(wp, auth_check);
}

#ifdef HAVE_REGISTER
static int do_auth_reg(webs_t wp, int (*auth_check)(webs_t conn_fp))
{
	if (!wp->isregistered)
		return 1;
	return do_auth(wp, auth_check);
}
#endif

#undef HAVE_DDLAN

#ifdef HAVE_DDLAN
static int do_auth2(webs_t wp, int (*auth_check)(webs_t conn_fp))
{
	strlcpy(wp->auth_userid, nvram_safe_get("http2_username"), AUTH_MAX - 1);
	strlcpy(wp->auth_passwd, nvram_safe_get("http2_passwd"), AUTH_MAX - 1);
	// strncpy(realm, MODEL_NAME, AUTH_MAX);
	strlcpy(wp->auth_realm, nvram_safe_get("router_name"), AUTH_MAX - 1);
	if (auth_check(wp))
		return 1;
	return 0;
}
#endif
// #ifdef EZC_SUPPORT
char ezc_version[128];

// #endif

int // support GET and POST 2003-08-22
do_apply_post(char *url, webs_t stream, size_t len, char *boundary)
{
	int count;
	if (stream->post == 1) {
		if (!(len + 1)) {
			dd_logerror("httpd", "The POST data content length is bullshit\n");
			return -1;
		}
		stream->post_buf = (char *)malloc(len + 1);

		if (!stream->post_buf) {
			dd_logerror("httpd", "The POST data exceed length limit! (allocation of %ld bytes failed)\n", len + 1);
			return -1;
		}
		/*
		 * Get query
		 */
		if (!(count = wfread(stream->post_buf, 1, len, stream)))
			return -1;
		stream->post_buf[count] = '\0';
		;
		len -= strlen(stream->post_buf);
		/*
		 * Slurp anything remaining in the request
		 */
		if (len) {
			char *buf = malloc(len);
			if (!buf) {
				dd_logerror("httpd",
					    "The POST data exceed length limit! (remaining request of length %ld failed)\n", len);
				return -1;
			}
			wfgets(buf, len, stream, NULL);
			debug_free(buf);
		}
		init_cgi(stream, stream->post_buf);
	}
	return 0;
}

#if !defined(HAVE_X86) && !defined(HAVE_MAGICBOX)
static int do_cfebackup(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	FILE *fp = fopen("/dev/mtd/0", "rb");
	if (fp) {
		FILE *out = fopen("/tmp/cfe.bin", "wb");
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);
		rewind(fp);
		int i;
		for (i = 0; i < len; i++)
			putc(getc(fp), out);
		fclose(out);
		fclose(fp);
		int ret = do_file_attach(handler, "/tmp/cfe.bin", stream, "cfe.bin");
		unlink("/tmp/cfe.bin");
		return ret;
	}
	return 0;
}
#endif

#ifdef HAVE_PRIVOXY
static int do_wpad(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	FILE *fp;

	fp = fopen("/tmp/wpad.dat", "wb");

	if (fp != NULL) {
		fprintf(fp,
			"function FindProxyForURL(url, host) {\n" //
			"var proxy = \"PROXY %s:8118; DIRECT\";\n" //
			"var direct = \"DIRECT\";\n" //
			"if(isPlainHostName(host)) return direct;\n" //
			"if (\n"
			"url.substring(0, 4) == \"ftp:\" ||\n" //
			"url.substring(0, 6) == \"rsync:\"\n"
			")\n" //
			"return direct;\n"
			"return proxy;\n"
			"}",
			nvram_safe_get("lan_ipaddr"));
		fclose(fp);
	}

	return do_file_attach(handler, "/tmp/wpad.dat", stream, "wpad.dat");
}
#endif

#ifdef HAVE_ROUTERSTYLE
static int do_stylecss(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	char *style = nvram_safe_get("router_style");

	unsigned int sdata[33];
	bzero(sdata, sizeof(sdata));
	unsigned int blue[33] = { 0x36f, 0xfff, 0x68f, 0x24d, 0x24d, 0x68f, 0x57f, 0xccf, 0x78f, 0x35d, 0x35c,
				  0x78f, 0x78f, 0xfff, 0x9af, 0x46e, 0x46e, 0x9af, 0x36f, 0xccf, 0xfff, 0x69f,
				  0xfff, 0xfff, 0x999, 0x69f, 0x69f, 0xccf, 0x78f, 0xfff, 0xfff, 0x36f, 0xccc };

	unsigned int cyan[33] = { 0x099, 0xfff, 0x3bb, 0x066, 0x066, 0x3bb, 0x3bb, 0xcff, 0x4cc, 0x1aa, 0x1aa,
				  0x4cc, 0x6cc, 0xfff, 0x8dd, 0x5bb, 0x5bb, 0x8dd, 0x099, 0xcff, 0xfff, 0x3bb,
				  0xfff, 0xfff, 0x999, 0x3bb, 0x3bb, 0xcff, 0x6cc, 0xfff, 0xfff, 0x099, 0xcff };

	unsigned int elegant[33] = { 0x30519c, 0xfff,	 0x496fc7, 0x496fc7, 0x496fc7, 0x496fc7, 0x496fc7, 0xfff,    0x6384cf,
				     0x6384cf, 0x6384cf, 0x6384cf, 0x6384cf, 0xfff,    0x849dd9, 0x849dd9, 0x849dd9, 0x849dd9,
				     0x30519c, 0xfff,	 0xfff,	   0x496fc7, 0xfff,    0xfff,	 0x999,	   0x496fc7, 0x496fc7,
				     0xfff,    0x6384cf, 0xfff,	   0xfff,    0x30519c, 0xccc };

	unsigned int green[33] = { 0x090, 0xfff, 0x3b3, 0x060, 0x060, 0x3b3, 0x3b3, 0xcfc, 0x4c4, 0x1a1, 0x1a1,
				   0x4c4, 0x6c6, 0xfff, 0x8d8, 0x5b5, 0x5b5, 0x8d8, 0x090, 0xcfc, 0xfff, 0x3b3,
				   0xfff, 0xfff, 0x999, 0x3b3, 0x3b3, 0xcfc, 0x6c6, 0xfff, 0xfff, 0x090, 0xcfc };

	unsigned int orange[33] = { 0xf26522, 0xfff,	0xff8400, 0xff8400, 0xff8400, 0xff8400, 0xff8400, 0xfff,    0xfeb311,
				    0xfeb311, 0xfeb311, 0xfeb311, 0xff9000, 0xfff,    0xffa200, 0xffa200, 0xffa200, 0xffa200,
				    0xf26522, 0xfff,	0xfff,	  0xff8400, 0xfff,    0xfff,	0x999,	  0xff8400, 0xff8400,
				    0xfff,    0xff9000, 0xfff,	  0xfff,    0xf26522, 0xccc };

	unsigned int red[33] = { 0xc00, 0xfff, 0xe33, 0x800, 0x800, 0xe33, 0xd55, 0xfcc, 0xe77, 0xc44, 0xc44,
				 0xe77, 0xe77, 0xfff, 0xf99, 0xd55, 0xd55, 0xf99, 0xc00, 0xfcc, 0xfff, 0xd55,
				 0xfff, 0xfff, 0x999, 0xd55, 0xd55, 0xfcc, 0xe77, 0xfff, 0xfff, 0xc00, 0xfcc };

	unsigned int yellow[33] = { 0xeec900, 0x000,	0xee3, 0x880,	 0x880,	   0xee3,   0xffd700, 0x660,	0xee7,
				    0xbb4,    0xbb4,	0xee7, 0xeec900, 0x000,	   0xff9,   0xcc5,    0xcc5,	0xff9,
				    0xeec900, 0x660,	0x000, 0xffd700, 0x000,	   0xfff,   0x999,    0xffd700, 0xeec900,
				    0x660,    0xffd700, 0x000, 0x333,	 0xeec900, 0xffd700 };

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
	else // default to elegant
		memcpy(sdata, elegant, sizeof(elegant));

	websWrite(stream,
		  "@import url(../common.css);\n#menuSub,\n#menuMainList li span,\n#help h2 {\nbackground:#%03x;\n" //
		  "color:#%03x;\nborder-color:#%03x #%03x #%03x #%03x;\n}\n#menuSubList li a {\n" //
		  "background:#%03x;\ncolor:#%03x;\nborder-color:#%03x #%03x #%03x #%03x;\n}\n" //
		  "#menuSubList li a:hover {\nbackground:#%03x;\ncolor:#%03x;\nborder-color:#%03x #%03x #%03x #%03x;\n}" //
		  "\nfieldset legend {\ncolor:#%03x;\n}\n#help a {\ncolor:#%03x;\n}\n" //
		  "#help a:hover {\ncolor:#%03x;\n}\n.meter .bar {\nbackground-color: #%03x;\n}\n" //
		  ".meter .text {\ncolor:#%03x;\n}\n.progressbar {\nbackground-color: #%03x;\n" //
		  "border-color: #%03x;\nfont-size:.09em;\nborder-width:.09em;\n}\n" //
		  ".progressbarblock {\nbackground-color: #%03x;\nfont-size:.09em;\n}" //
		  "\ninput.button {\nbackground: #%03x;\ncolor: #%03x;\n}\n" //
		  "input.button:hover {\nbackground: #%03x;\ncolor: #%03x;\n" //
		  "}\nh2, h3 {\ncolor: #%03x;\nbackground-color: #%03x;\n}" //
		  "\ntable tr th {\ncolor: #%03x;\n}\n",
		  sdata[0], sdata[1], sdata[2], sdata[3], sdata[4], sdata[5], //
		  sdata[6], sdata[7], sdata[8], sdata[9], sdata[10], sdata[11],
		  sdata[12], //
		  sdata[13], sdata[14], sdata[15], sdata[16], sdata[17],
		  sdata[18], //
		  sdata[19], sdata[20], sdata[21], sdata[22], //
		  sdata[23], sdata[24], sdata[25], sdata[26], sdata[27],
		  sdata[28], //
		  sdata[29], sdata[30], sdata[31], sdata[32]);
	return 0;
}

static int do_stylecss_ie(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	websWrite(stream,
		  ".submitFooter input {\npadding:.362em .453em;\n}\n" //
		  "fieldset {\npadding-top:0;\n}\nfieldset legend {\n" //
		  "margin-left:-9px;\nmargin-bottom:8px;\npadding:0 .09em;\n}\n");
	return 0;
}
#endif
#ifdef HAVE_REGISTER
static int do_trial_logo(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
#if defined(HAVE_TRIMAX) || defined(HAVE_MAKSAT) || defined(HAVE_VILIM) || defined(HAVE_TELCOM) || defined(HAVE_WIKINGS) || \
	defined(HAVE_NEXTMEDIA)
	return do_file(method, handler, url, stream);
#else
	if (!stream->isregistered_real) {
		return do_file(method, handler, "style/logo-trial.png", stream);
	} else {
		if (iscpe()) {
			return do_file(method, handler, "style/logo-cpe.png", stream);
		} else {
			return do_file(method, handler, url, stream);
		}
	}
#endif
}

#endif

static int do_logout_asp(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	int ret = do_ej(method, handler, "Logout.asp", stream);
	websDone(stream, 200);
	return ret;
}

/*
 * static void do_style (char *url, webs_t stream, char *query) { char *style
 * = nvram_get ("router_style"); if (style == NULL || strlen (style) == 0)
 * do_file ("kromo.css", stream, NULL); else do_file (style, stream, NULL); }
 */

static int do_mypage(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	char *snamelist = nvram_safe_get("mypage_scripts");
	char *next;
	char sname[128];
	char buf[1024];
	int qnum;
	int i = 1;
	char *query = strchr(url, '?');
	if (query == NULL || *(query) == 0)
		qnum = 1;
	else
		qnum = atoi(query + 1);

	if (qnum < 0)
		qnum = 1;
	foreach(sname, snamelist, next)
	{
		if (qnum == i) {
			FILE *fp;
			dd_logdebug("httpd", "exec %s\n", sname);
			if ((fp = popen(sname, "rb")) != NULL) {
				while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
					wfwrite(buf, strlen(buf), 1, stream);
				}
				pclose(fp);
			} else {
				return -1;
			}
		}
		i++;
	}

	return 0;
}

static int do_fetchif(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	char line[512];
	int i, llen;
	char *buffer;
	char querybuffer[64];
	if (!url)
		return -1; // unlikely
	char *query = strchr(url, '?');

	if (query == NULL || *(query) == 0)
		return -1;
	query++;

	snprintf(querybuffer, sizeof(querybuffer) - 1, "%s:", query);

	int strbuffer = 0;
	time_t tm;
	struct tm tm_time;

	time(&tm);
	localtime_r(&tm, &tm_time);
	char *date_fmt = "%a %b %e %H:%M:%S %Z %Y";
	int baselen = 64;
	buffer = malloc(baselen);
	if (!buffer)
		return -1;
	strftime(buffer, 200, date_fmt, &tm_time);
	strbuffer = strlen(buffer);
	buffer[strbuffer++] = '\n';
	FILE *in = fopen("/proc/net/dev", "rb");

	if (in == NULL)
		return -1;

	/* eat first two lines */
	if (feof(in))
		return -1;
	fgets(line, sizeof(line) - 1, in);
	if (feof(in))
		return -1;
	fgets(line, sizeof(line) - 1, in);
	if (feof(in))
		return -1;
	memset(line, 0, sizeof(line));
	while (!feof(in) && (fgets(line, sizeof(line) - 1, in) != NULL)) {
		if (!strstr(line, "mon.") && strstr(line, querybuffer)) {
			llen = strlen(line);
			if (llen) {
				baselen += llen;
				buffer = realloc(buffer, baselen + 1);
				if (buffer != NULL) {
					memcpy(&buffer[strbuffer], line, llen);
					strbuffer += llen;
				}
			}
			break;
		}
		memset(line, 0, sizeof(line));
	}
	fclose(in);

	buffer[strbuffer] = 0;
	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
	websWrite(stream, "%s", buffer);
	debug_free(buffer);
	return 0;
}

static char *getLanguageName()
{
	char *lang = nvram_safe_get("language");
	char *l;
	cprintf("get language %s\n", lang);

	if (!*lang) {
		cprintf("return default\n");
		return strdup("lang_pack/english.js");
	}
	asprintf(&l, "lang_pack/%s.js", lang);
	return l;
}

static char *scanfile(webs_t wp, char *buf, const char *tran)
{
	char *temp = malloc(256);
	int limit = 256;
	char *temp2;
	char *temp1;
	size_t filelen;
	FILE *fp = _getWebsFile(wp, buf, &filelen);
	if (fp) {
		temp1 = malloc(strlen(tran) + 3);
		strcpy(temp1, tran);
		strcat(temp1, "=\"");
		int len = strlen(temp1);
		int i;
		int count = 0;
		int ign = 0;
		int val = 0;
		int prev = 0;
		for (i = 0; i < filelen; i++) {
again:;
			if (count < limit) {
				prev = val;
				val = getc(fp);
				if (val == EOF) {
					debug_free(temp);
					debug_free(temp1);
					fclose(fp);
					return NULL;
				}
				if (!count && (val == ' ' || val == '\r' || val == '\t' || val == '\n'))
					continue;
			} else {
				int a, v = 0;
				for (a = 0; a < filelen - i; a++) {
					prev = v;
					v = getc(fp);
					if (v == EOF) {
						debug_free(temp);
						debug_free(temp1);
						return NULL;
					}
					if (v == '"' && prev != '\\') {
						if (!ign)
							ign = 1;
						else
							ign = 0;

					} else if (!ign && v == ';') {
						i += a;
						count = 0;
						goto again;
					}
				}
				debug_free(temp);
				debug_free(temp1);
				fclose(fp);
				return NULL;
			}
			if (count == 255) {
				limit = 512;
				temp = realloc(temp, 512);
			}
			if (count == 511) {
				limit = 1024;
				temp = realloc(temp, 1024);
			}
			temp[count++] = val;
			switch (val) {
			case '\r':
			case '\t':
			case '\n':
				continue;
			case '"':
				if (prev != '\\') {
					if (!ign)
						ign = 1;
					else
						ign = 0;
				}
				break;
			case ';':
				if (!ign) {
					temp[count] = 0;
					count = 0;
					if ((memcmp(temp, temp1, len)) == 0) {
						temp2 = strtok(temp, "\"");
						temp2 = strdup(strtok(NULL, "\""));
						debug_free(temp);
						debug_free(temp1);
						fclose(fp);
						return temp2;
					}
				}
			}
		}
		debug_free(temp1);
		fclose(fp);
	}
	debug_free(temp);
	return NULL;
}

struct cacheentry {
	char *request;
	char *translation;
	time_t time;
};
static int cachecount = 0;
static struct cacheentry *translationcache = NULL;
static char *private_live_translate(webs_t wp, const char *tran)
{
	if (tran == NULL || !*(tran))
		return "";
	char *lang = getLanguageName();
	char buf[64];
	sprintf(buf, "%s", lang);
	debug_free(lang);

	char *result = scanfile(wp, buf, tran);
	if (result)
		return result;

	strcpy(buf, "lang_pack/english.js"); // if string not found, try english
	result = scanfile(wp, buf, tran);
	if (result)
		return result;
	return NULL;
}

static void clear_translationcache(void)
{
	int i;
	if (translationcache && cachecount > 0) {
		for (i = 0; i < cachecount; i++) {
			if (translationcache[i].request != NULL) {
				debug_free(translationcache[i].request);
				debug_free(translationcache[i].translation);
				translationcache[i].request = NULL;
				translationcache[i].translation = NULL;
			}
		}
		debug_free(translationcache);
		translationcache = NULL;
		cachecount = 0;
	}
}

char *live_translate(webs_t wp,
		     const char *tran) // todo: add locking to be thread safe
{
	static char *cur_language = NULL;
	if (!tran || *(tran) == 0) {
		dd_logdebug("httpd", "translation string is empty\n");
		return "Error";
	}
	if (!cur_language) {
		cur_language = nvram_safe_get("language");
	} else {
		if (!nvram_match("language", cur_language)) {
			clear_translationcache();
			cur_language = nvram_safe_get("language");
		}
	}

	time_t cur = time(NULL);
	if (translationcache) {
		int i;
		char *translation = NULL;
		for (i = 0; i < cachecount; i++) {
			if (!translation && translationcache[i].request && !strcmp(translationcache[i].request, tran)) {
				translation = translationcache[i].translation;
				translationcache[i].time = cur;
			}
			if (translationcache[i].request != NULL &&
			    cur > translationcache[i].time + 120) { // free translation if not used for 2 minutes
				debug_free(translationcache[i].translation);
				debug_free(translationcache[i].request);
				translation = NULL;
				translationcache[i].request = NULL;
				translationcache[i].translation = NULL;
			}
		}
		if (translation) {
			return translation;
		}
	}
	char *ret = private_live_translate(wp, tran);
	struct cacheentry *entry = NULL;
	/* fill hole if there is any */
	int i;
	for (i = 0; i < cachecount; i++) {
		if (translationcache[i].request == NULL) {
			entry = &translationcache[i];
			break;
		}
	}
	if (!entry) {
		/* no hole has been found, alloc a new one */
		translationcache = (struct cacheentry *)realloc(translationcache, sizeof(struct cacheentry) * (cachecount + 1));
		entry = &translationcache[cachecount++];
	}
	entry->request = strdup(tran);
	entry->time = cur;
	if (ret)
		entry->translation = ret;
	else {
		dd_logdebug("httpd", "no translation found for %s\n", tran);
		entry->translation = strdup("Error");
	}
	return entry->translation;
}

void do_ddwrt_inspired_themes(webs_t wp);

#ifdef HAVE_STATUS_SYSLOG
static int do_syslog(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	const char filename[] = "/var/log/messages";
	const char filename_jffs[] = "/jffs/log/messages";
	char *style_dark = nvram_safe_get("router_style_dark");
	char buf[128];
	char *charset = live_translate(stream, "lang_charset.set");
	int offset = 0;
	int count = 0;
	char *query = strchr(url, '?');
	if (!query || sscanf(query + 1, "%d", &offset) != 1)
		return -1;

	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);

	websWrite(stream,
		  "<!DOCTYPE html>\n" //
		  "<html>\n"
		  "<head>\n"
		  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\" />\n" //
		  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n" //
		  "<script type=\"text/javascript\" src=\"common.js\"></script>\n" //
		  "<script type=\"text/javascript\" src=\"lang_pack/english.js\"></script>\n",
		  charset);
#ifdef HAVE_LANGUAGE
	if (!nvram_match("language", "english"))
		websWrite(stream, "<script type=\"text/javascript\" src=\"lang_pack/language.js\"></script>\n");
#endif
	char *style = nvram_safe_get("router_style");
	if (!style)
		style = "elegant";
	websWrite(stream, "<link type=\"text/css\" rel=\"stylesheet\" href=\"style/syslogd/syslogd.css\" />\n");
	if (!strcmp(style, "blue") || !strcmp(style, "cyan") || !strcmp(style, "elegant") || !strcmp(style, "carlson") ||
	    !strcmp(style, "green") || !strcmp(style, "orange") || !strcmp(style, "red") || !strcmp(style, "yellow")) {
		websWrite(stream, "<link type=\"text/css\" rel=\"stylesheet\" href=\"style/%s/colorscheme.css\" />\n", style);
		if (style_dark != NULL && !strcmp(style_dark, "1")) {
			websWrite(stream,
				  "<link type=\"text/css\" rel=\"stylesheet\" href=\"style/syslogd/syslogd_dark.css\" />\n");
		}
	}
	websWrite(stream, //
		  "</head>\n<body class=\"syslog_bd\">\n" //
		  "<fieldset class=\"syslog_bg\">" //
		  "<legend class=\"syslog_legend\">" //
		  "%s" //
		  "</legend>",
		  _tran_string(buf, sizeof(buf), "share.sysloglegend"));

	do_ddwrt_inspired_themes(stream);
	if (nvram_matchi("syslogd_enable", 1)) {
		FILE *fp = NULL;
		if (nvram_matchi("syslogd_jffs2", 1))
			fp = fopen(filename_jffs, "r");
		if (!fp) //fallback
			fp = fopen(filename, "r");
		if (fp != NULL) {
			char line[1024];
			websWrite(stream, "<div style=\"height: 770px; overflow-y: auto; overflow-x: hidden;\"><table><tbody>");
			while (fgets(line, sizeof(line), fp) != NULL) {
				count++;
				if (offset <= count && ((offset + 50) > count)) { // show 100 lines
					// a few sample colors
					if (strstr(line, ".warn")) {
						websWrite(
							stream,
							"<tr class=\"syslog_bg_yellow\"><td class=\"syslog_text_dark\">%s</td></tr>",
							line);
					} else if (strstr(line, "authpriv.notice")) {
						websWrite(
							stream,
							"<tr class=\"syslog_bg_green\"><td class=\"syslog_text_dark\">%s</td></tr>",
							line);
					} else if (strstr(line, "mounting unchecked fs") || strstr(line, "httpd login failure") ||
						   strstr(line, "auth-failure") || strstr(line, ".err")) {
						websWrite(stream,
							  "<tr class=\"syslog_bg_red\"><td class=\"syslog_text_dark\">%s</td></tr>",
							  line);
					} else {
						websWrite(stream, "<tr><td>%s</td></tr>", line);
					}
				}
			}
			websWrite(stream, "</tbody></table></div>");

			fclose(fp);
		}
	} else {
		websWrite(
			stream,
			"<table style=\"margin-bottom: 10px;\"><tr class=\"center\"><td style=\"padding-bottom: 4px;\">%s</td></tr></table>",
			_tran_string(buf, sizeof(buf), "share.syslogdisabled"));
	}
	websWrite(stream, "</fieldset></body>");
	websWrite(stream, "</html>");
	return 0;
}
#endif

static int do_ttgraph(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	char *charset = live_translate(stream, "lang_charset.set");

#define COL_WIDTH 16 /* single column width */

	char *next;
	char var[80];

	unsigned int days;
	unsigned int month;
	unsigned int year;
	int wd;
	int i = 0;
	char months[12][12] = { "share.jan", "share.feb", "share.mar", "share.apr", "share.may", "share.jun",
				"share.jul", "share.aug", "share.sep", "share.oct", "share.nov", "share.dec" };
	unsigned long rcvd[31] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned long sent[31] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned long max = 5, smax = 5, f = 1;
	unsigned long totin = 0;
	unsigned long totout = 0;
	char *query = strchr(url, '?');
	if (!query || sscanf(query + 1, "%u-%u", &month, &year) != 2)
		return -1;
	if (month < 1 || month > 12)
		return -1;

	days = daysformonth(month, year);
	wd = weekday(month, 1, year); // first day in month (mon=0, tue=1,
	// ..., sun=6)

	char tq[32];

	sprintf(tq, "traff-%02u-%u", month, year);
	char *tdata = nvram_safe_get(tq);

	if (tdata != NULL && *(tdata)) {
		foreach(var, tdata, next)
		{
			if (i == days)
				break; //skip monthly total
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

	char *incom = live_translate(stream, "status_inet.traffin");
	char *outcom = live_translate(stream, "status_inet.traffout");
	char *monthname = live_translate(stream, months[month - 1]);
	if (handler && !handler->send_headers)
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);

	websWrite(stream,
		  "<!DOCTYPE html>\n" //
		  "<html>\n"
		  "<head>\n"
		  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\" />\n" //
		  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n" //
		  "<title>DD-WRT Traffic Graph</title>\n" //
		  "<script type=\"text/javascript\">\n" //
		  "//<![CDATA[\n" //
		  "function Show(label) {\n" //
		  "document.getElementById(\"label\").innerHTML = label;\n" //
		  "}\n"
		  "//]]>\n"
		  "</script>\n"
		  "<style type=\"text/css\">\n\n" //
		  "#t-graph {position: relative; width: %upx; height: 300px;\n" //
		  "  margin: 1.1em 0 3.5em; padding: 0;\n" //
		  "  border: 1px solid gray; list-style: none;\n" //
		  "  font: 9px Tahoma, Arial, sans-serif; color: #666;}\n"
		  "#t-graph ul {margin: 0; list-style: none;}\n" //
		  "#t-graph li {position: absolute; bottom: 0; width: %dpx; z-index: 2;\n" //
		  "  margin: 0; padding: 0;\n" //
		  "  text-align: center; list-style: none;}\n" //
		  "#t-graph li.day {height: 298px; padding-top: 2px; border-right: 1px dotted #c4c4c4; color: #aaa;}\n" //
		  "#t-graph li.day_sun {height: 298px; padding-top: 2px; border-right: 1px dotted #c4c4c4; color: #e00;}\n" //
		  "#t-graph li.bar {width: 4px; border: 1px solid; border-bottom: none; color: #000;}\n" //
		  "#t-graph li.bar p {margin: 5px 0 0; padding: 0;}\n" //
		  "#t-graph li.rcvd {left: 3px; background: #228b22;}\n" //
		  "#t-graph li.sent {left: 8px; background: #cd0000;}\n",
		  charset, days * COL_WIDTH, COL_WIDTH);

	for (i = 0; i < days - 1; i++) {
		websWrite(stream, "#t-graph #d%d {left: %dpx;}\n", i + 1, i * COL_WIDTH);
	}
	websWrite(stream, "#t-graph #d%u {left: %upx; border-right: none;}\n", days, (days - 1) * COL_WIDTH);

	websWrite(
		stream,
		"#t-graph #ticks {width: %upx; height: 300px; z-index: 1;}\n" //
		"#t-graph #ticks .tick {position: relative; border-bottom: 1px solid #bbb; width: %upx;}\n" //
		"#t-graph #ticks .tick p {position: absolute; left: 100%%; top: -0.67em; margin: 0 0 0 0.5em;}\n" //
		"#t-graph #label {width: 500px; bottom: -20px;  z-index: 1; font: 12px Tahoma, Arial, sans-serif; font-weight: bold;}\n" //
		"</style>\n"
		"</head>\n\n",
		days * COL_WIDTH, days * COL_WIDTH);
	do_ddwrt_inspired_themes(stream);
	// add body class to target background in internal inspired themes
	websWrite(stream, "<body class=\"t-graph-bg\">\n"
			  "<ul id=\"t-graph\">\n");
	for (i = 0; i < days; i++) {
		websWrite(stream, "<li class=\"day%s\" id=\"d%d\" ", (wd % 7) == 6 ? "_sun" : "", i + 1);
		wd++;
		websWrite(stream,
			  "onmouseover=\"Show(\'%s %d, %d (%s: %lu MB / %s: %lu MB)\')\" " //
			  "onmouseout=\"Show(\'%s %d (%s: %lu MB / %s: %lu MB)\')\">%d\n<ul>\n"
			  "<li class=\"rcvd bar\" style=\"height: %lupx;\"><p></p></li>\n"
			  "<li class=\"sent bar\" style=\"height: %lupx;\"><p></p></li>\n</ul>\n</li>\n",
			  monthname, i + 1, year, incom, rcvd[i], outcom, sent[i], monthname, year, incom, totin, outcom, totout,
			  i + 1, rcvd[i] * 300 / smax, sent[i] * 300 / smax);
	}

	websWrite(stream, "<li id=\"ticks\">\n");
	for (i = 5; i; i--) // scale
	{
		websWrite(stream, "<div class=\"tick\" style=\"height: 59px;\"><p>%d%sMB</p></div>\n", smax * i / 5,
			  (smax > 10000) ? " " : "&nbsp;");
	}
	websWrite(stream,
		  "</li>\n\n<li id=\"label\">\n%s %d (%s: %lu MB / %s: %lu MB)\n</li>\n"
		  "</ul>\n\n"
		  "</body>"
		  "\n\n",
		  monthname, year, incom, totin, outcom, totout);
	websWrite(stream, "</html>");
	return 0;
}

static int ttraff_backup(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	FILE *out = fopen("/tmp/traffdata.bak", "wb");
	if (!out)
		return -1;
	fprintf(out, "TRAFF-DATA\n");
	FILE *fp = popen("nvram show | grep traff-", "rb");
	if (!fp) {
		fclose(out);
		return -1;
	}
	while (!feof(fp))
		putc(getc(fp), out);
	pclose(fp);
	fclose(out);
	int ret = do_file_attach(handler, "/tmp/traffdata.bak", stream, "traffdata.bak");
	unlink("/tmp/traffdata.bak");
	return ret;
}

static int do_apply_cgi(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	char *path, *query;
	if (stream->post == 1) {
		query = stream->post_buf;
		path = url;
	} else {
		query = url;
		path = strsep(&query, "?") ?: url;
#if 0
		init_cgi(stream, query);
#endif
	}

	if (!query)
		return -1;

	apply_cgi(stream, NULL, NULL, 0, url, path, query, handler);
	deinit_cgi(stream);
	return 0;
}

static int do_wifiselect_cgi(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	char *path, *query;
	char newpath[128];
	if (stream->post == 1) {
		query = stream->post_buf;
		path = url;
	} else {
		query = url;
		path = strsep(&query, "?") ?: url;
	}

	if (!query)
		return -1;
	char *select = websGetVar(stream, "wifi_display", NULL);
	if (select)
		nvram_set("wifi_display", select);
	char *next_page = websGetVar(stream, "next_page", NULL);
	char *submit_button = websGetVar(stream, "submit_button", "");
	if (next_page)
		sprintf(newpath, "%s", next_page);
	else {
		sprintf(newpath, "%s.asp", submit_button);
	}
	do_redirect(METHOD_GET, handler, newpath, stream);
	websDone(stream, 200);
	deinit_cgi(stream);
	return 0;
}

#ifdef HAVE_MADWIFI
extern int getdevicecount(void);
#endif

#ifdef HAVE_LANGUAGE
static int do_language(unsigned char method, struct mime_handler *handler, char *path, webs_t stream) // jimmy,
// https,
// 8/4/2003
{
	char *langname = getLanguageName();
	char *prefix, *lang;

	prefix = calloc(1, strlen(path) - (sizeof("lang_pack/language.js") - 1) + 1);
	strlcpy(prefix, path, strlen(path) - ((sizeof("lang_pack/language.js") - 1)));
	asprintf(&lang, "%s%s", prefix, langname);
	int ret = do_file(method, handler, lang, stream);

	debug_free(lang);
	debug_free(prefix);
	debug_free(langname);
	return ret;
}
#endif

#define NO_HEADER 0
#define SEND_HEADER 1
#define IGNORE_OPTIONS 0
#define HANDLE_OPTIONS 1
static char no_cache[] = "Cache-Control: no-cache\r\n"
			 "Pragma: no-cache\r\n"
			 "Expires: 0";
static char do_cache[] = "Cache-Control: private, max-age=600\r\n";

static struct mime_handler mime_handlers[] = {
#ifdef HAVE_POKER
	{ "PokerEdit.asp", "text/html", no_cache, NULL, do_ej, NULL, NO_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_DDLAN
	{ "Upgrade*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Management*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Services*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Hotspot*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Wireless*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "WL_*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "WPA*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Log*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Alive*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Diagnostics*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Wol*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "Factory_Defaults*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "config*", "text/html", no_cache, NULL, do_ej, do_auth2, NO_HEADER, IGNORE_OPTIONS },
#endif
	{ "Logout.asp", "text/html", no_cache, NULL, do_ej, NULL, NO_HEADER, IGNORE_OPTIONS },

	{ "changepass.asp", "text/html", no_cache, NULL, do_ej, do_auth_changepass, NO_HEADER, IGNORE_OPTIONS },
#ifdef HAVE_REGISTER
	{ "register.asp", "text/html", no_cache, NULL, do_ej, do_auth_reg, NO_HEADER, IGNORE_OPTIONS },
#endif
	{ "WL_FilterTable*", "text/html", no_cache, NULL, do_filtertable, do_auth, NO_HEADER, IGNORE_OPTIONS },
#ifdef HAVE_FREERADIUS
	{ "FreeRadiusCert*", "text/html", no_cache, NULL, do_radiuscert, do_auth, SEND_HEADER, IGNORE_OPTIONS },
	{ "freeradius-certs/*", "application/octet-stream", no_cache, NULL, cert_file_out, do_auth, NO_HEADER, IGNORE_OPTIONS },
#endif
	{ "Wireless_WDS*", "text/html", no_cache, NULL, do_wds, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "WL_ActiveTable*", "text/html", no_cache, NULL, do_activetable, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "Site_Survey*", "text/html", no_cache, NULL, do_sitesurvey, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "Wireless_Advanced*", "text/html", no_cache, NULL, do_wireless_adv, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "MyPage.asp*", "text/html", no_cache, NULL, do_mypage, do_auth, SEND_HEADER, IGNORE_OPTIONS },
	{ "dologout.asp*", "text/html", no_cache, NULL, do_logout_asp, do_auth, SEND_HEADER, IGNORE_OPTIONS },
	{ "**.asp", "text/html", no_cache, NULL, do_ej, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "**.JPG", "image/jpeg", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "common.js", "text/javascript", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
#ifdef HAVE_LANGUAGE
	{ "lang_pack/language.js", "text/javascript", NULL, NULL, do_language, NULL, NO_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_BUFFALO
	{ "intatstart/lang_pack/language.js", "text/javascript", NULL, NULL, do_language, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "intatstart/js/intatstart.js", "text/javascript", NULL, NULL, do_ej, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "intatstart/js/mdetect.js", "text/javascript", NULL, NULL, do_ej, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "vsp.html", "text/plain", no_cache, NULL, do_vsp_page, NULL, SEND_HEADER, IGNORE_OPTIONS },
#endif
	{ "SysInfo.htm*", "text/plain", no_cache, NULL, do_ej, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "Info.htm*", "text/html", no_cache, NULL, do_ej, do_cauth, NO_HEADER, IGNORE_OPTIONS },
	{ "Info.live.htm", "text/html", no_cache, NULL, do_ej, do_cauth, NO_HEADER, IGNORE_OPTIONS },
	{ "**.htm", "text/html", no_cache, NULL, do_ej, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.html", "text/html", no_cache, NULL, do_ej, NULL, NO_HEADER, IGNORE_OPTIONS },
#ifdef HAVE_ROUTERSTYLE
	{ "style/common_style_ie.css", "text/css", do_cache, NULL, do_stylecss_ie, NULL, SEND_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_REGISTER
	{ "style/logo.png", "image/png", NULL, NULL, do_trial_logo, NULL, NO_HEADER, IGNORE_OPTIONS },
#endif
	{ "graph_if.svg", "image/svg+xml", NULL, NULL, do_file, do_auth, NO_HEADER, IGNORE_OPTIONS },
#ifdef HAVE_PRIVOXY
	{ "wpad.dat", "application/x-ns-proxy-autoconfig", no_cache, NULL, do_wpad, NULL, NO_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_ATH9K
	{ "spectral_scan.json", "application/json", no_cache, NULL, do_spectral_scan, do_auth, NO_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_DDLAN
	{ "applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, NULL, NO_HEADER, IGNORE_OPTIONS },
#else
	{ "applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, do_auth, NO_HEADER, IGNORE_OPTIONS },
#endif
	{ "fetchif.cgi*", "text/html", no_cache, NULL, do_fetchif, do_auth, NO_HEADER, IGNORE_OPTIONS },
#ifdef HAVE_DDLAN
	{ "apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi, NULL, NO_HEADER, IGNORE_OPTIONS },
#else
	{ "apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi, do_auth, NO_HEADER, IGNORE_OPTIONS },
#endif
	{ "wifiselect.cgi*", "text/html", no_cache, do_apply_post, do_wifiselect_cgi, do_cauth, NO_HEADER, IGNORE_OPTIONS },
#ifdef HAVE_BUFFALO
	{ "olupgrade.cgi*", "text/html", no_cache, do_olupgrade_post, do_upgrade_cgi, do_auth, SEND_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_DDLAN
	{ "restore.cgi**", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi, NULL, SEND_HEADER, IGNORE_OPTIONS },
#else
	{ "restore.cgi**", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi, do_auth, SEND_HEADER, IGNORE_OPTIONS },
#endif
	{ "test.bin**", "application/octet-stream", no_cache, NULL, do_file, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "bigfile.bin*", "application/octet-stream", no_cache, NULL, do_bigfile, NULL, NO_HEADER, HANDLE_OPTIONS },

#ifdef HAVE_DDLAN
	{ "nvrambak.bin*", "application/octet-stream", no_cache, NULL, nv_file_out, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "nvrambak**.bin*", "application/octet-stream", no_cache, NULL, nv_file_out, do_auth2, NO_HEADER, IGNORE_OPTIONS },
	{ "nvram.cgi*", "text/html", no_cache, nv_file_in, sr_config_cgi, NULL, SEND_HEADER, IGNORE_OPTIONS },
#else
	{ "nvrambak.bin*", "application/octet-stream", no_cache, NULL, nv_file_out, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "nvrambak**.bin*", "application/octet-stream", no_cache, NULL, nv_file_out, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "nvram.cgi*", "text/html", no_cache, nv_file_in, sr_config_cgi, do_auth, SEND_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_SSHD
	{ "id_ed25519.ssh*", "application/octet-stream", no_cache, NULL, download_ssh_key, do_auth, NO_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_WIREGUARD
	{ "wireguard_config_oet**.conf*", "application/octet-stream", no_cache, NULL, download_wireguard_config, do_auth, NO_HEADER,
	  IGNORE_OPTIONS },
#endif
#ifdef HAVE_OPENVPN
	{ "ovpncl_config.ovpn*", "application/octet-stream", no_cache, NULL, download_ovpncl_config, do_auth, NO_HEADER,
	  IGNORE_OPTIONS },
#endif
#if !defined(HAVE_X86) && !defined(HAVE_MAGICBOX)
	{ "backup/cfe.bin", "application/octet-stream", no_cache, NULL, do_cfebackup, do_auth, NO_HEADER, IGNORE_OPTIONS },
#endif
#ifdef HAVE_STATUS_SYSLOG
	{ "syslog.cgi*", "text/html", no_cache, NULL, do_syslog, do_auth, NO_HEADER, IGNORE_OPTIONS },
#endif
	{ "**.svg", "image/svg+xml", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.avif", "image/avif", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.bmp", "image/bmp", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.gif", "image/gif", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.png", "image/png", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.jpg", "image/jpeg", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.jpeg", "image/jpeg", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.ico", "image/x-icon", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.js", "text/javascript", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.css", "text/css", NULL, NULL, do_ej, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.txt", "text/plain", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.bin", "application/octet-stream", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.pdf", "application/pdf", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.doc", "application/msword", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document", NULL, NULL, do_file, NULL,
	  NO_HEADER, IGNORE_OPTIONS },
	{ "**.ppt", "application/vnd.ms-powerpoint", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation", NULL, NULL, do_file, NULL,
	  NO_HEADER, IGNORE_OPTIONS },
	{ "**.xls", "application/vnd.ms-excel", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", NULL, NULL, do_file, NULL, NO_HEADER,
	  IGNORE_OPTIONS },
	{ "**.xml", "application/xml", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.7z", "application/x-7z-compressed", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.gz", "application/gzip", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.rar", "application/vnd.rar", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.tar", "application/x-tar", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.zip", "application/zip", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.xhtml", "application/xhtml+xml", no_cache, NULL, do_ej, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "**.avi", "video/x-msvideo", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.mp4", "video/mp4", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.mpeg", "video/mpeg", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.wmv", "video/x-ms-wmv", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.aac", "audio/aac", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.webm", "audio/webm", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.weba", "audio/webm", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.mp3", "audio/mpeg3", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.wav", "audio/wav", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.wma", "audio/x-ms-wma", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.swf", "application/x-shockwave-flash", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },
	{ "**.flv", "video/x-flv", NULL, NULL, do_file, NULL, NO_HEADER, IGNORE_OPTIONS },

	{ "ttgraph.cgi*", "text/html", no_cache, NULL, do_ttgraph, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "traffdata.bak*", "text/html", no_cache, NULL, ttraff_backup, do_auth, NO_HEADER, IGNORE_OPTIONS },
	{ "tadmin.cgi*", "text/html", no_cache, td_file_in, td_config_cgi, do_auth, SEND_HEADER, IGNORE_OPTIONS },
	{ "*", "application/octet-stream", no_cache, NULL, do_file, do_auth, NO_HEADER, IGNORE_OPTIONS },
	// for ddm
	{ NULL, NULL, NULL, NULL, NULL, NULL, NO_HEADER, IGNORE_OPTIONS }
};

#ifdef HAVE_BUFFALO
int do_vsp_page(unsigned char method, struct mime_handler *handler, char *url, webs_t stream)
{
	/*
#ifdef HAVE_MADWIFI
	char *ifname = "wlan0";
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
	return 0;
}
#endif

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
		{ ' ', "&nbsp;" },
		{ ':', "&semi;" },
		{ '<', "&lt;" },
		{ '>', "&gt;" },
	};

	struct pattern *v;

	*new_name = 0;

	switch (type) {
	case SET:
		for (i = 0; *(old_name + i); i++) {
			match = 0;
			for (v = patterns; v < &patterns[STRUCT_LEN(patterns)]; v++) {
				if (*(old_name + i) == v->ch) {
					size_t slen = strlen(new_name);

					if (slen + strlen(v->string) + 1 > size) { // avoid overflow
						cprintf("%s(): overflow\n", __FUNCTION__);
						new_name[size - 1] = '\0';
						return 1;
					}
					strcat(new_name, v->string);
					match = 1;
					break;
				}
			}
			if (!match) {
				// strlen() depends on NULL termination
				size_t slen = strlen(new_name);
				if (slen > size) {
					cprintf("%s(): overflow\n",
						__FUNCTION__); // avoid
					// overflow
					new_name[size - 1] = '\0';
					return 1;
				}
				*(new_name + slen) = *(old_name + i); // Copy character over
				*(new_name + slen + 1) = '\0'; // add a NULL terminator so strlen() works
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
