#include <stdio.h>
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/un.h>
#include <poll.h>
#include <assert.h>
#include <linux/if.h>
#include <linux/types.h>
#include <linux/wireless.h>
#include <syslog.h>

struct survey_table
{
	char channel[4];
	char ssid[33];
	char bssid[20];
	char security[23];
	char *crypto;
};

struct survey_table st[64];
int survey_count = 0;

#define RTPRIV_IOCTL_SET (SIOCIWFIRSTPRIV + 0x02)
void iwpriv(const char *name, const char *key, const char *val)
{
	int socket_id;
	struct iwreq wrq;
	char data[64];
	snprintf(data, 64, "%s=%s", key, val);
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(wrq.ifr_ifrn.ifrn_name, name);
	wrq.u.data.length = strlen(data);
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ioctl(socket_id, RTPRIV_IOCTL_SET, &wrq);
	close(socket_id);
}

static void next_field(char **line, char *output, int n) {
	char *l = *line;
	int i;

	memcpy(output, *line, n);
	*line = &l[n];

	for (i = n - 1; i > 0; i--) {
		if (output[i] != ' ')
			break;
		output[i] = '\0';
	}
}

#define RTPRIV_IOCTL_GSITESURVEY (SIOCIWFIRSTPRIV + 0x0D)
void wifi_site_survey(const char *ifname, char* essid, int print)
{
	char *s = malloc(IW_SCAN_MAX_DATA);
	int ret;
	int socket_id;
	struct iwreq wrq;
	char *line, *start;
	iwpriv(ifname, "SiteSurvey", (essid ? essid : ""));
	sleep(5);
	memset(s, 0x00, IW_SCAN_MAX_DATA);
	strcpy(wrq.ifr_name, ifname);
	wrq.u.data.length = IW_SCAN_MAX_DATA;
	wrq.u.data.pointer = s;
	wrq.u.data.flags = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	ret = ioctl(socket_id, RTPRIV_IOCTL_GSITESURVEY, &wrq);
	close(socket_id);
	if(ret != 0)
		goto out;
	if(wrq.u.data.length < 1)
		goto out;
	/* ioctl result starts with a newline, for some reason */
	start = s;
	while (*start == '\n')
		start++;
	line = strtok((char *)start, "\n");
	line = strtok(NULL, "\n");
	survey_count = 0;
	while(line && (survey_count < 64)) {
		next_field(&line, st[survey_count].channel, sizeof(st->channel));
		next_field(&line, st[survey_count].ssid, sizeof(st->ssid));
		next_field(&line, st[survey_count].bssid, sizeof(st->bssid));
		next_field(&line, st[survey_count].security, sizeof(st->security));
		line = strtok(NULL, "\n");
		st[survey_count].crypto = strstr(st[survey_count].security, "/");
		if (st[survey_count].crypto) {
			*st[survey_count].crypto = '\0';
			st[survey_count].crypto++;
			printf("%s|%s|%s|%s\n",
				st[survey_count].channel, st[survey_count].ssid, st[survey_count].bssid, st[survey_count].security);
		} else {
			st[survey_count].crypto = "";
		}
		survey_count++;
	}
	if (survey_count == 0 && !print)
		syslog(LOG_INFO, "No results");
out:
	free(s);
}

struct survey_table* wifi_find_ap(const char *name)
{
	int i;
	for(i = 0; i < survey_count; i++)
	{
		if(!strcmp(name, (char*)st[i].ssid))
			return &st[i];
	}
	return 0;
}


#define lengthof(x) (sizeof(x) / sizeof(x[0]))

/* This function is heavily similar to the wifi_repeater_start in
 * net/wifi_core.c from microd (but changed to call ifdown/ifup instead
 * of fiddling with interface configuration manually. */
void wifi_repeater_start(const char *ifname, const char *staname, const char *channel, const char *ssid, const char *key, const char *enc, const char *crypto)
{
	char buf[100];
	int enctype = 0;

	iwpriv(ifname, "Channel", channel);
	iwpriv(staname, "ApCliEnable", "0");
	if((strstr(enc, "WPA2PSK") || strstr(enc, "WPAPSKWPA2PSK")) && key)
	{
		enctype = 1;
		iwpriv(staname, "ApCliAuthMode", "WPA2PSK");
	} else if(strstr(enc, "WPAPSK") && key)
	{
		enctype = 1;
		iwpriv(staname, "ApCliAuthMode", "WPAPSK");
	} else if(strstr(enc, "WEP") && key)
	{
		iwpriv(staname, "ApCliAuthMode", "AUTOWEP");
		iwpriv(staname, "ApCliEncrypType", "WEP");
		iwpriv(staname, "ApCliDefaultKeyID", "1");
		iwpriv(staname, "ApCliKey1", key);
		iwpriv(staname, "ApCliSsid", ssid);
	} else if(!key || key[0] == '\0') {
		iwpriv(staname, "ApCliAuthMode", "NONE");
		iwpriv(staname, "ApCliSsid", ssid);
	} else {
		return;
	}
	if(enctype)
	{
		if(strstr(crypto, "AES") || strstr(crypto, "TKIPAES"))
			iwpriv(staname, "ApCliEncrypType", "AES");
		else
			iwpriv(staname, "ApCliEncrypType", "TKIP");
		iwpriv(staname, "ApCliSsid", ssid);
		iwpriv(staname, "ApCliWPAPSK", key);
	}
	iwpriv(staname, "ApCliEnable", "1");
	snprintf(buf, lengthof(buf) - 1, "ifconfig '%s' up", staname);
	system(buf);
}

int check_assoc(char *ifname)
{
	int socket_id, i;
	struct iwreq wrq;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(wrq.ifr_ifrn.ifrn_name, ifname);
	ioctl(socket_id, SIOCGIWAP, &wrq);
	close(socket_id);
	for(i = 0; i < 6; i++)
		if(wrq.u.ap_addr.sa_data[i])
			return 1;
	return 0;
}

void assoc_loop(char *ifname, char *staname, char *essid, char *pass)
{
	static int try_count = 0;
	static int assoc_count = 0;
	while (1) {
		if (!check_assoc(staname)) {
			struct survey_table *c;
			syslog(LOG_INFO, "%s is not associated\n", staname);
			syslog(LOG_INFO, "Scanning for networks...\n");
			wifi_site_survey(ifname, essid, 0);
			c = wifi_find_ap(essid);
			try_count++;
			assoc_count = 0;
			if (c) {
				syslog(LOG_INFO, "Found network, trying to associate (essid: %s, bssid: %s, channel: %s, enc: %s, crypto: %s)\n",
					essid, c->ssid, c->channel, c->security, c->crypto);
				wifi_repeater_start(ifname, staname, c->channel, essid, pass, c->security, c->crypto);
			} else {
				syslog(LOG_INFO, "No signal found to connect to\n");
				try_count = 0;
			}
		} else {
			if (assoc_count == 0)
				syslog(LOG_INFO, "%s is associated\n", staname);
			assoc_count++;
			if (assoc_count > 1)
				try_count = 0;
		}
		sleep(15);
	}
}

int main(int argc, char **argv)
{
	FILE *fp;
	char path[256];

	/* Disable output buffering on stdout (otherwise redirecting
	 * stdout to a file doesn't seem to work). */
	if (argc != 5)
		return -1;
	daemon(0, 0);
	snprintf(path, sizeof(path), "/tmp/apcli-%s.pid", argv[2]);
	fp = fopen(path, "w+");
	if (fp) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	setbuf(stdout, NULL);
	openlog("ap_client", 0, 0);
	assoc_loop(argv[1], argv[2], argv[3], argv[4]);
	return 0;
}
