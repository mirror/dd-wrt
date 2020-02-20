/*
To add:
 - Track what SSIDs clients are asking for (up to 20?)
 - Find IP addresses for clients
 - 
*/

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#include <endian.h>
#include <unistd.h>
#include <stdint.h>

#define INFO_UPTIME 0
#define INFO_RSSI 1
#define INFO_NOISE 2
#define INFO_RXRATE 3
#define INFO_TXRATE 4

int getWifiInfo_ath9k(char *ifname, unsigned char *mac, int field);	// only used internal
int getWifiInfo(char *ifname, unsigned char *mac, int field);

#define getNoise(ifname, mac) getWifiInfo(ifname, mac, INFO_NOISE)
#define getRssi(ifname, mac) getWifiInfo(ifname, mac, INFO_RSSI)
#define getTxRate(ifname, mac) getWifiInfo(ifname, mac, INFO_TXRATE)
#define getRxRate(ifname, mac) getWifiInfo(ifname, mac, INFO_RXRATE)
#define getUptime(ifname, mac) getWifiInfo(ifname, mac, INFO_UPTIME)

#define HOST_TIMEOUT 300

#include "wl_access.h"
#include "structs.h"
#include "channelhopper.h"

#ifdef WIVIZ_GPS
#include "wiviz_gps.h"
#endif

#ifndef __cplusplus
#define __cdecl
#endif

#define nonzeromac(x) memcmp(x, "\0\0\0\0\0\0", 6)

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define swap16(x) x
#elif __BYTE_ORDER == __BIG_ENDIAN
#define swap16(x) \
	((uint16_t)( \
			(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) | \
			(((uint16_t)(x) & (uint16_t)0xff00U) >> 8) ))
#else
#error "no endian type"
#endif

char *get_monitor(void);

int openMonitorSocket(char *dev);
void dealWithPacket(wiviz_cfg * cfg, int len, const u_char * packet);
wiviz_host *gotHost(wiviz_cfg * cfg, u_char * mac, host_type type);
void print_host(FILE * outf, wiviz_host * host);
void __cdecl signal_handler(int);
void readWL(wiviz_cfg * cfg);
void reloadConfig();
int stop = 0;

wiviz_cfg *global_cfg;
char *wl_dev;
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	char *dev;
	int oldMonitor, newMonitor;
	u_char packet[4096];
	int pktlen;
	wiviz_cfg cfg;
	int i;
	int defaultHopSeq[] = { 1, 3, 6, 8, 11 };
	int s, one;
	memset(&cfg, 0, sizeof(cfg));
#ifdef HAVE_MADWIFI
	wl_dev = nvram_safe_get("wifi_display");
#elif HAVE_RT2880
	wl_dev = getRADev(nvram_safe_get("wifi_display"));
#else
	char tmp[32];
	sprintf(tmp, "%s_ifname", nvram_safe_get("wifi_display"));
	wl_dev = nvram_safe_get(tmp);
#endif
	if (argc > 1)
		if (!strcmp(argv[1], "terminate")) {
#ifdef HAVE_MADWIFI
			// return to original channel
			if (is_mac80211(wl_dev)) {
				sysprintf("ifconfig %s down", get_monitor());
				sysprintf("iw dev %s del", get_monitor());
			} else {
				sysprintf("iwconfig %s channel %sM", get_monitor(), nvram_nget("%s_channel", nvram_safe_get("wifi_display")));
				sleep(1);
				sysprintf("ifconfig %s down", get_monitor());
				sysprintf("wlanconfig %s destroy", get_monitor());
			}
#elif HAVE_RT2880
			if (nvram_match("wifi_display", "wl0")) {
				nvram_set("wl0_mode", nvram_safe_get("wl0_oldmode"));
				sysprintf("startservice configurewifi");
				if (nvram_match("wl0_mode", "sta") || nvram_match("wl0_mode", "apsta")) {
					sysprintf("startstop wan");
				}
			} else {
				nvram_set("wl1_mode", nvram_safe_get("wl1_oldmode"));
				sysprintf("startservice configurewifi");
				if (nvram_match("wl1_mode", "sta") || nvram_match("wl1_mode", "apsta")) {
					sysprintf("startstop wan");
				}
			}
#else
			oldMonitor = 0;
			wl_ioctl(wl_dev, WLC_SET_MONITOR, &oldMonitor, 4);
#endif
			return 0;
		}

	global_cfg = &cfg;
	signal(SIGUSR1, &signal_handler);
	signal(SIGUSR2, &signal_handler);

	printf("Wi-Viz 2 infogathering daemon by Nathan True\n");
	printf("http://wiviz.natetrue.com\n");

	memset(&cfg, 0, sizeof(wiviz_cfg));
	cfg.numHosts = 0;
	cfg.lastKeepAlive = time(NULL);
	cfg.channelHopping = 0;
	cfg.channelDwellTime = 1000;
	cfg.channelHopSeqLen = 5;
	memcpy(cfg.channelHopSeq, defaultHopSeq, sizeof(defaultHopSeq));

#if defined(HAVE_MADWIFI)
	if (is_mac80211(nvram_safe_get("wifi_display"))) {
		sysprintf("iw phy phy%d interface add %s type monitor", get_ath9k_phy_ifname(nvram_safe_get("wifi_display")), get_monitor());
		sysprintf("ifconfig %s up", get_monitor());
	} else {
		sysprintf("wlanconfig %s create wlandev %s wlanmode monitor", get_monitor(), getWifi(nvram_safe_get("wifi_display")));
		sysprintf("ifconfig %s up", get_monitor());
	}
	cfg.readFromWl = 1;
#elif HAVE_RT2880
	if (nvram_match("wifi_display", "wl0")) {
		nvram_set("wl0_oldmode", nvram_safe_get("wl0_mode"));
		nvram_set("wl0_mode", "sta");
		if (!nvram_match("wl0_oldmode", "sta"))
			sysprintf("startservice configurewifi");
		sysprintf("iwconfig ra0 mode monitor");
	} else {
		nvram_set("wl1_oldmode", nvram_safe_get("wl1_mode"));
		nvram_set("wl1_mode", "sta");
		if (!nvram_match("wl1_oldmode", "sta"))
			sysprintf("startservice configurewifi");
		sysprintf("iwconfig ba0 mode monitor");

	}
	cfg.readFromWl = 1;
#else
	wl_ioctl(wl_dev, WLC_GET_MAGIC, &i, 4);
	if (i != WLC_IOCTL_MAGIC) {
		printf("Wireless magic not correct, not querying wl for info %X!=%X\n", i, WLC_IOCTL_MAGIC);
		cfg.readFromWl = 0;
	} else {
		cfg.readFromWl = 1;
		wl_ioctl(wl_dev, WLC_GET_MONITOR, &oldMonitor, 4);
		newMonitor = 1;
		wl_ioctl(wl_dev, WLC_SET_MONITOR, &newMonitor, 4);
	}
#endif
	reloadConfig();

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_ATH9K)
	s = openMonitorSocket(get_monitor());	// for testing we use ath0
#else

	if (nvram_match("wifi_display", "wl1"))
		s = openMonitorSocket("prism1");
	else
		s = openMonitorSocket("prism0");
#endif
	if (s == -1)
		return -1;
	one = 1;
	ioctl(s, FIONBIO, (char *)&one);

	if (cfg.readFromWl) {
		readWL(&cfg);
	}
#ifdef WIVIZ_GPS
	gps_init(&cfg);
#endif

	while (!stop) {
#ifdef WIVIZ_GPS
		gps_tick();
#else
		if (time(NULL) - cfg.lastKeepAlive > 30)
			stop = 1;
#endif
		pktlen = recv(s, packet, 4096, 0);
		if (pktlen <= 0)
			continue;
		dealWithPacket(&cfg, pktlen, packet);
	}

	signal_handler(SIGUSR1);

	if (cfg.channelHopperPID)
		kill(cfg.channelHopperPID, SIGKILL);

#ifndef WIVIZ_GPS
	for (i = 0; i < MAX_HOSTS; i++) {
		print_host(stderr, cfg.hosts + i);
		if (cfg.hosts[i].occupied)
			printf("\n");
		if (cfg.hosts[i].apInfo)
			free(cfg.hosts[i].apInfo);
		if (cfg.hosts[i].staInfo)
			free(cfg.hosts[i].staInfo);
	}
#endif
	close(s);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
int openMonitorSocket(char *dev)
{
	//Open the socket
	struct ifreq ifr;
	struct sockaddr_ll addr;
	int s;
	sysprintf("ifconfig %s up", dev);
	s = socket(PF_PACKET, SOCK_RAW, 0);
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, dev);
	if (ioctl(s, SIOCGIFINDEX, &ifr) != 0) {
		printf("ioctl IFINDEX failed!!!\n");
		return -1;
	}
	close(s);

	s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;
	addr.sll_protocol = 0;
	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("bind failed!!! (%s)\n", dev);
		return -1;
	}

	return s;
}

////////////////////////////////////////////////////////////////////////////////
void writeJavascript()
{
	int i;
	FILE *outf;
	wiviz_host *h;

	outf = fopen("/tmp/wiviz2-dump", "w");
	if (!outf) {
		printf("Failure to open output file\n");
		return;
	}

	global_cfg->lastKeepAlive = time(NULL);

	if (global_cfg->readFromWl)
		readWL(global_cfg);

	fprintf(outf, "top.hosts = new Array();\nvar hnum = 0;\nvar h;\n");
	for (i = 0; i < MAX_HOSTS; i++) {
		h = global_cfg->hosts + i;
		if (h->occupied == 0)
			continue;
		if (time(NULL) - h->lastSeen > HOST_TIMEOUT) {
			h->occupied = 0;
		}
		fprintf(outf, "h = new Object();\n");
		print_host(outf, h);
		fprintf(outf, "top.hosts[hnum] = h; hnum++;\n");
	}
	fprintf(outf, "\nvar wiviz_cfg = new Object();\n wiviz_cfg.channel = ");
	if (global_cfg->channelHopping) {
		fprintf(outf, "'hopping'");
	} else {
		fprintf(outf, "%i", global_cfg->curChannel);
	}
	fprintf(outf, ";\ntop.wiviz_callback(top.hosts, wiviz_cfg);\n");
	fprintf(outf, "function wiviz_callback(one, two) {\n");
	fprintf(outf, "alert('This asp is intended to run inside Wi-Viz.  You will now be redirected there.');\n");
	fprintf(outf, "location.replace('Wiviz_Survey.asp');\n");
	fprintf(outf, "}");
	fclose(outf);
}

static const char *ntoa(const uint8_t mac[6])
{
	static char a[18];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return (i < 17 ? NULL : a);
}

////////////////////////////////////////////////////////////////////////////////
void reloadConfig()
{
	FILE *cnf;
	wiviz_cfg *cfg = global_cfg;
	char filebuffer[512];
	char *fbptr, *p, *v, *vv;
	int fblen, val;
	int hopCfgChanged = 0;
	int newHopSeq[12];
	int newHopSeqLen = 0;

	printf("Loading config file\n");

	cnf = fopen("/tmp/wiviz2-cfg", "r");
	if (!cnf) {
		printf("Wiviz: No config file (/tmp/wiviz2-cfg) present, using defaults\n");
		return;
	}

	fblen = fread(filebuffer, 1, 512, cnf);
	fclose(cnf);
	if (fblen >= 512) {
		printf("Error reading config file\n");
		return;
	}
	filebuffer[fblen] = 0;
	printf("Read %i bytes from config file\n", fblen);

	fbptr = filebuffer;

	while (fbptr < filebuffer + fblen && *fbptr != 0) {
		p = fbptr;
		//Find end of parameter
		for (; *fbptr != '=' && *fbptr != 0; fbptr++) ;
		*fbptr = 0;
		v = ++fbptr;
		//Find end of value
		for (; *fbptr != '&' && *fbptr != 0; fbptr++) ;
		*(fbptr++) = 0;
		printf("Config: %s=%s\n", p, v);
		//Apply configuration
		if (!strcmp(p, "channelsel")) {
			//Channel selector
			cfg->channelHopping = 0;
			if (!strcmp(v, "hop")) {
				//Set channel hopping
				cfg->channelHopping = 1;
				hopCfgChanged = 1;
			} else if (!strcmp(v, "nochange")) {
				//Don't change anything, read channel from wireless card
				readWL(cfg);
			} else {
				val = atoi(v);
				if (val < 0 || val > 254) {
					printf("Channel setting in config file invalid (%i)\n", cfg->curChannel);
				} else {
					cfg->curChannel = val;
					if (cfg->readFromWl) {
#ifdef HAVE_MADWIFI
						set_channel(wl_dev, cfg->curChannel);
//          sysprintf("iwconfig %s channel %d\n",wl_dev,cfg->curChannel);
#elif HAVE_RT2880
						if (nvram_match("wifi_display", "wl0"))
							sysprintf("iwpriv ra0 set Channel=%d", cfg->curChannel);
						else
							sysprintf("iwpriv ba0 set Channel=%d", cfg->curChannel);
#else
						if (wl_ioctl(wl_dev, WLC_SET_CHANNEL, &cfg->curChannel, 4) < 0) {
							printf("Channel set to %i failed\n", cfg->curChannel);
						}
#endif
					} else {
						printf("Can't set channel, no Broadcom wireless device present\n");
					}
				}
			}
		}
		if (!strcmp(p, "hopdwell")) {
			val = atoi(v);
			if (val < 100)
				val = 100;
			if (val > 30000)
				val = 30000;
			if (cfg->channelDwellTime != val)
				hopCfgChanged = 1;
			cfg->channelDwellTime = val;
		}
		if (!strcmp(p, "hopseq")) {
			cfg->channelHopSeqLen = 0;
			while (v < fbptr) {
				for (vv = v; *vv != ',' && *vv != 0; vv++) ;
				if (*vv == 0) {
					cfg->channelHopSeq[cfg->channelHopSeqLen++] = atoi(v);
					break;
				}
				*vv = 0;
				cfg->channelHopSeq[cfg->channelHopSeqLen++] = atoi(v);
				v = vv + 1;
			}
		}
		/*
		   if (!strcmp(p, "")) {
		   }
		 */
	}
	//Apply channel hopper settings
	if (cfg->channelHopping == 0 && cfg->channelHopperPID) {
		kill(cfg->channelHopperPID, SIGKILL);
		cfg->channelHopperPID = 0;
	}
	if (cfg->channelHopping == 1 && hopCfgChanged) {
		if (cfg->channelHopperPID)
			kill(cfg->channelHopperPID, SIGKILL);
		if ((cfg->channelHopperPID = fork()) == 0) {
			channelHopper(cfg);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void __cdecl signal_handler(int signum)
{
	if (signum == SIGUSR1)
		writeJavascript();
	if (signum == SIGUSR2)
		reloadConfig();
	if (signum == SIGTERM)
		stop = 1;
}

static unsigned char i_src[6];	// = "\0\0\0\0\0\0";
static unsigned char i_dst[6];	// = "\0\0\0\0\0\0";
static unsigned char i_bss[6];	// = "\0\0\0\0\0\0";

////////////////////////////////////////////////////////////////////////////////
void dealWithPacket(wiviz_cfg * cfg, int pktlen, const u_char * packet)
{
	ieee802_11_hdr *hWifi;
	wiviz_host *host;
	wiviz_host *emergebss;
	host_type type = typeUnknown;
	int wfType;
	int rssi = 0;
	int to_ds, from_ds;
	ieee_802_11_tag *e;
	ieee_802_11_mgt_frame *m;
	unsigned char *src;	// = "\0\0\0\0\0\0";
	unsigned char *dst;	// = "\0\0\0\0\0\0";
	unsigned char *bss;	// = "\0\0\0\0\0\0";
	char *ssid = "";
	int channel = 0;
	int adhocbeacon = 0;
	u_char ssidlen = 0;
	ap_enc_type encType = aetUnknown;
	if (!packet)
		return;
	src = i_src;
	dst = i_dst;
	bss = i_bss;

	if (is_mac80211(nvram_safe_get("wifi_display"))) {
		if (packet[0] > 0) {
			printf("Wrong radiotap header version.\n");
			return;
		} else
			printf("ver %d\n", packet[0]);
		int number = packet[2] | (unsigned int)((unsigned int)packet[3] << 8);
		printf("num %d\n", number);
		if (number <= 0 || number >= pktlen) {
			printf("something wrong %d\n", number);
			return;
		}

		int noise = packet[number - 3];
		rssi = -(100 - (packet[number - 4] - noise));
		printf("rssi %d\n", rssi);
		hWifi = (ieee802_11_hdr *) (packet + (number));
	} else {
		prism_hdr *hPrism;
		prism_did *i;
		if (pktlen < sizeof(prism_hdr) + (sizeof(ieee802_11_hdr)))
			return;
		hPrism = (prism_hdr *) packet;
		if (pktlen < hPrism->msg_length + (sizeof(ieee802_11_hdr)))
			return;	// bogus packet
		hWifi = (ieee802_11_hdr *) (packet + (hPrism->msg_length));
		i = (prism_did *) ((char *)hPrism + sizeof(prism_hdr));
		//Parse the prism DIDs
#ifdef HAVE_MADWIFI

		int received = 0;
		while ((int)i < (int)hWifi) {
			if (i->did == pdn_rssi) {
				received = 1;
				rssi = i->data;
			}
			if (i->did == pdn_signal) {
				rssi = (int)i->data + rssi;
			}
			if (i->did == 0)	//skip bogus empty value from atheros sequence counter
			{
				i = (prism_did *) (((unsigned char *)&i->data) + 4);
			} else {
				i = (prism_did *) (((unsigned char *)&i->data) + i->length);
			}
		}
		if (!received)	// bogus, no prism data
			return;
		if (!rssi)	// no rssi? can't be a packet
			return;
#else
		while ((int)i < (int)hWifi) {
			if (i->did == pdn_rssi)
				rssi = *(int *)(i + 1);
			i = (prism_did *) ((int)(i + 1) + i->length);
		}
#endif
	}

	memset(bss, 0, 6);
	memset(src, 0, 6);
	memset(dst, 0, 6);
	int fctype = (hWifi->frame_control & 0xC);
	int fc = (hWifi->frame_control & 0xf0);
	type = typeUnknown;
	printf("type %X, fc %X\n", fctype, fc);
	if (!fctype)		// only accept management frames (type 0)
	{
		switch (fc) {
			//case mgt_assocRequest: //fc = 0 can be a broken frame too, no check possible here
		case mgt_reassocRequest:
		case mgt_probeRequest:
			type = typeSta;
			src = hWifi->addr2;
			dst = hWifi->addr1;
			break;
		case mgt_assocResponse:
		case mgt_reassocResponse:
		case mgt_probeResponse:
		case mgt_beacon:
			src = hWifi->addr2;
			dst = hWifi->addr1;
			bss = hWifi->addr3;
			type = typeAP;
			break;
		}
#ifdef DEBUG
		fprintf(stderr, "fc: %X", hWifi->frame_control);
		fprintf(stderr, " src:");
		fprintf(stderr, "%s", ntoa(src));
		fprintf(stderr, " dst:");
		fprintf(stderr, "%s", ntoa(dst));
		fprintf(stderr, " bss:");
		fprintf(stderr, "%s\n", ntoa(src));
#endif
	}
	to_ds = hWifi->flags & IEEE80211_TO_DS;
	from_ds = hWifi->flags & IEEE80211_FROM_DS;
	unsigned char subtype = hWifi->frame_control & 0xF0;
	if (fctype == 0x8 && subtype == 0) {
		//Data frame
		src = hWifi->addr2;
		dst = hWifi->addr1;
		if (!from_ds)
			type = typeSta;
		else if (from_ds && to_ds)
			type = typeWDS;
		else
			type = typeAP;
		if (!to_ds && !from_ds)
			bss = hWifi->addr3;
		if (to_ds && !from_ds)
			bss = hWifi->addr1;
		if (!to_ds && from_ds)
			bss = hWifi->addr2;
		if (to_ds && from_ds)
			bss = hWifi->addr1;	// wds frame
#ifdef DEBUG
		fprintf(stderr, "addr1:");
		fprintf(stderr, "%s", ntoa(hWifi->addr1));
		fprintf(stderr, " addr2:");
		fprintf(stderr, "%s", ntoa(hWifi->addr2));
		fprintf(stderr, " addr3:");
		fprintf(stderr, "%s", ntoa(hWifi->addr3));
		fprintf(stderr, " bss:");
		fprintf(stderr, "%s\n", ntoa(bss));
#endif
	}
	if (type == typeUnknown)
		return;

	//Parse the 802.11 tags
	if (!fctype && (fc == mgt_probeResponse || fc == mgt_beacon || fc == mgt_probeRequest)) {
		m = (ieee_802_11_mgt_frame *) (hWifi + 1);
		if (swap16(m->caps) & MGT_CAPS_IBSS) {
			type = typeSta;
			adhocbeacon = 1;
		}
		if (swap16(m->caps) & MGT_CAPS_WEP)
			encType = aetEncWEP;
		else
			encType = aetUnencrypted;
		e = (ieee_802_11_tag *) ((int)m + sizeof(ieee_802_11_mgt_frame));
		while ((u_int) e < (u_int) packet + pktlen) {
			if (e->tag == tagSSID) {
				ssidlen = e->length;
				ssid = (char *)(e + 1);
			}
			if (e->tag == tagChannel) {
				channel = *(char *)(e + 1);
			}
			if (e->tag == tagRSN) {
				if (encType != aetEncWPAmix) {
					if (encType == aetEncWPA)
						encType = aetEncWPAmix;
					else
						encType = aetEncWPA2;
				}
			}
			if (e->tag == tagVendorSpecific) {
				if (e->length >= 4 && memcmp(e + 1, "\x00\x50\xf2\x01", 4) == 0) {
					//WPA encryption
					if (encType != aetEncWPAmix) {
						if (encType == aetEncWPA2)
							encType = aetEncWPAmix;
						else
							encType = aetEncWPA;
					}
				}
				if (e->length >= 4 && memcmp(e + 1, "\x00\x0f\xac\x01", 4) == 0) {
					//WPA2 encryption
					if (encType != aetEncWPAmix) {
						if (encType == aetEncWPA)
							encType = aetEncWPAmix;
						else
							encType = aetEncWPA2;
					}
				}
			}
			e = (ieee_802_11_tag *) ((int)(e + 1) + e->length);
		}
	}
	//Look up the host in the hash table
	host = gotHost(cfg, src, type);

	//Add any info we received
	if (host->RSSI) {
		host->RSSI = host->RSSI * 9 / 10 + (-rssi * 10);
	} else {
		host->RSSI = -rssi * 100;
	}
	if (type == typeSta) {
		if (nonzeromac(bss)) {
			memcpy(host->staInfo->connectedBSSID, bss, 6);
			host->staInfo->state = ssAssociated;
			emergebss = gotHost(cfg, bss, typeAP);
			if (emergebss->RSSI == 0)
				emergebss->RSSI = 10000;
			memcpy(emergebss->apInfo->bssid, bss, 6);
			if (adhocbeacon) {
				emergebss->type = typeAdhocHub;
				if (ssidlen > 0 && ssidlen <= 32) {
					memcpy(emergebss->apInfo->ssid, ssid, ssidlen);
					emergebss->apInfo->ssidlen = ssidlen;
				}
				if (channel)
					emergebss->apInfo->channel = channel;
				emergebss->apInfo->flags = hWifi->flags;
				emergebss->RSSI = host->RSSI;
				if (encType != aetUnknown)
					emergebss->apInfo->encryption = encType;
			}
		}
		if (!fctype && fc == mgt_probeRequest && host->staInfo->state == ssUnknown)
			host->staInfo->state = ssUnassociated;
		if (!fctype && fc == mgt_probeRequest && ssidlen > 0 && ssidlen <= 32) {
			memcpy(host->staInfo->lastssid, ssid, ssidlen);
			host->staInfo->lastssid[ssidlen] = 0;
			host->staInfo->lastssidlen = ssidlen;
		}
	}
	if (type == typeWDS) {
		if (nonzeromac(bss)) {
			memcpy(host->apInfo->bssid, bss, 6);
		}
	}
	if (type == typeAP) {
		if (nonzeromac(bss)) {
			memcpy(host->apInfo->bssid, bss, 6);
		}
		if (ssidlen > 0 && ssidlen <= 32) {
			memcpy(host->apInfo->ssid, ssid, ssidlen);
			host->apInfo->ssid[ssidlen] = 0;
			host->apInfo->ssidlen = ssidlen;
		}
		if (channel)
			host->apInfo->channel = channel;
		host->apInfo->flags = hWifi->flags;
		if (encType != aetUnknown)
			host->apInfo->encryption = encType;
	}
}

////////////////////////////////////////////////////////////////////////////////
void print_mac(u_char * mac, char *extra)
{
	fprint_mac(stdout, mac, extra);
}

////////////////////////////////////////////////////////////////////////////////
void fprint_mac(FILE * outf, u_char * mac, char *extra)
{
	fprintf(outf, "%02X:%02X:%02X:%02X:%02X:%02X%s", mac[0] & 0xFF, mac[1] & 0xFF, mac[2] & 0xFF, mac[3] & 0xFF, mac[4] & 0xFF, mac[5] & 0xFF, extra);
}

////////////////////////////////////////////////////////////////////////////////
#define MAX_PROBES MAX_HOSTS/2
wiviz_host *gotHost(wiviz_cfg * cfg, u_char * mac, host_type type)
{
	int i = (mac[5] + (mac[4] << 8)) % MAX_HOSTS;
	int c = 0;
	wiviz_host *h = cfg->hosts + i;
	while (h->occupied && memcmp(h->mac, mac, 6)) {
		i++;
		h++;
		c++;
		if (i >= MAX_HOSTS) {
			i = 0;
			h = cfg->hosts;
		}
		if (c > MAX_PROBES)
			break;
	}
#ifdef DEBUG
	if (!h->occupied) {
		fprintf(stderr, "New host %s\n", ntoa(mac));
	}
#endif
	h->occupied = 1;
	h->lastSeen = time(NULL);
	h->type = type;
	memcpy(h->mac, mac, 6);
	if (h->type == typeAP && !h->apInfo) {
		h->apInfo = (ap_info *) malloc(sizeof(ap_info));
		memset(h->apInfo, 0, sizeof(ap_info));
	}
	if (h->type == typeWDS && !h->apInfo) {
		h->apInfo = (ap_info *) malloc(sizeof(ap_info));
		memset(h->apInfo, 0, sizeof(ap_info));
	}
	if (h->type == typeSta && !h->staInfo) {
		h->staInfo = (sta_info *) malloc(sizeof(sta_info));
		memset(h->staInfo, 0, sizeof(sta_info));
	}
	return h;
}

////////////////////////////////////////////////////////////////////////////////
void print_host(FILE * outf, wiviz_host * host)
{
	int i;

	if (!host->occupied)
		return;
	fprintf(outf, "h.mac = '");
	fprint_mac(outf, host->mac, "';\n");
	if (host->RSSI < 0)
		fprintf(outf, "h.rssi = %i;\nh.type = '", host->RSSI / 100);
	else
		fprintf(outf, "h.rssi = -%i;\nh.type = '", host->RSSI / 100);
	switch (host->type) {
	case typeAP:
		fprintf(outf, "ap");
		break;
	case typeWDS:
		fprintf(outf, "wds");
		break;
	case typeSta:
		fprintf(outf, "sta");
		break;
	case typeAdhocHub:
		fprintf(outf, "adhoc");
		break;
	}
	fprintf(outf, "';\nh.self = ");
	fprintf(outf, host->isSelf ? "true;\n" : "false;\n");
	if (host->type == typeSta) {
		switch (host->staInfo->state) {
		case ssAssociated:
			fprintf(outf, "h.sta_state='assoc';\nh.sta_bssid='");
			fprint_mac(outf, host->staInfo->connectedBSSID, "';\n");
			break;
		case ssUnassociated:
			fprintf(outf, "h.sta_state='unassoc';\n");
		}
		fprintf(outf, "h.sta_lastssid = '");
		for (i = 0; i < host->staInfo->lastssidlen; i++) {
			fprintf(outf, "&#%04i;", *((char *)host->staInfo->lastssid + i) & 0xFF);
		}
		fprintf(outf, "';\n");
	}
	if (host->type == typeAP || host->type == typeAdhocHub) {
		fprintf(outf, "h.channel = %i;\nh.ssid = '", host->apInfo->channel & 0xFF);
		for (i = 0; i < host->apInfo->ssidlen; i++) {
			fprintf(outf, "&#%04i;", *((char *)host->apInfo->ssid + i) & 0xFF);
		}
		fprintf(outf, "';\nh.encrypted = ");
		switch (host->apInfo->encryption) {
		case aetUnknown:
			fprintf(outf, "'unknown';\n");
			break;
		case aetUnencrypted:
			fprintf(outf, "'no';\n");
			break;
		case aetEncUnknown:
			fprintf(outf, "'yes';\nh.enctype = 'unknown';\n");
			break;
		case aetEncWEP:
			fprintf(outf, "'yes';\nh.enctype = 'wep';\n");
			break;
		case aetEncWPA:
			fprintf(outf, "'yes';\nh.enctype = 'wpa';\n");
			break;
		case aetEncWPA2:
			fprintf(outf, "'yes';\nh.enctype = 'wpa2';\n");
			break;
		case aetEncWPAmix:
			fprintf(outf, "'yes';\nh.enctype = 'wpa wpa2';\n");
			break;
		}
	}
	if (host->type == typeWDS) {
		fprintf(outf, "h.channel = %i;\nh.ssid = '", host->apInfo->channel & 0xFF);
		fprintf(outf, "';\nh.encrypted = ");
		switch (host->apInfo->encryption) {
		case aetUnknown:
			fprintf(outf, "'unknown';\n");
			break;
		case aetUnencrypted:
			fprintf(outf, "'no';\n");
			break;
		case aetEncUnknown:
			fprintf(outf, "'yes';\nh.enctype = 'unknown';\n");
			break;
		case aetEncWEP:
			fprintf(outf, "'yes';\nh.enctype = 'wep';\n");
			break;
		case aetEncWPA:
			fprintf(outf, "'yes';\nh.enctype = 'wpa';\n");
			break;
		case aetEncWPA2:
			fprintf(outf, "'yes';\nh.enctype = 'wpa2';\n");
			break;
		case aetEncWPAmix:
			fprintf(outf, "'yes';\nh.enctype = 'wpa wpa2';\n");
			break;
		}
	}
	fprintf(outf, "h.age = %i;\n", time(0) - host->lastSeen);
}

////////////////////////////////////////////////////////////////////////////////
#define MAX_STA_COUNT 64
void readWL(wiviz_cfg * cfg)
{
	int ap, i;
	wiviz_host *host, *sta;
	uchar mac[6];
	wlc_ssid_t ssid;
	channel_info_t channel;
	maclist_t *macs;
	sta_rssi_t starssi;
	char buf[32];

	get_mac(wl_dev, mac);
	printf("AP mac: ");
#ifdef NEED_PRINTF
	print_mac(mac, "\n");
#endif
	if (!nonzeromac(mac))
		return;
#ifdef HAVE_MADWIFI
	if (nvram_nmatch("ap", "%s_mode", wl_dev))
		ap = 1;
	if (nvram_nmatch("wdsap", "%s_mode", wl_dev))
		ap = 1;
#else
	if (nvram_match("wifi_display", "wl0")) {
		if (nvram_match("ap", "wl0_oldmode"))
			ap = 1;
	} else {
		if (nvram_match("ap", "wl1_oldmode"))
			ap = 1;
	}
#endif
//      wl_ioctl(wl_dev, WLC_GET_AP, &ap, 4);
	if (ap) {
		host = gotHost(cfg, mac, typeAP);
		host->isSelf = 1;
#if defined(HAVE_MADWIFI)
		strcpy(host->apInfo->ssid, nvram_nget("%s_ssid", wl_dev));
		host->apInfo->ssidlen = strlen(host->apInfo->ssid);
		ether_atoe(nvram_nget("%s_hwaddr", wl_dev), buf);
		memcpy(host->apInfo->bssid, buf, 6);
#elif defined(HAVE_RT2880)
		if (nvram_match("wifi_display", "wl0")) {
			strcpy(host->apInfo->ssid, nvram_safe_get("wl0_ssid"));
			ether_atoe(nvram_safe_get("wl0_hwaddr"), buf);
		} else {
			strcpy(host->apInfo->ssid, nvram_safe_get("wl1_ssid"));
			ether_atoe(nvram_safe_get("wl1_hwaddr"), buf);
		}
		host->apInfo->ssidlen = strlen(host->apInfo->ssid);
		memcpy(host->apInfo->bssid, buf, 6);
#else
		wl_ioctl(wl_dev, WLC_GET_BSSID, host->apInfo->bssid, 6);
		wl_ioctl(wl_dev, WLC_GET_SSID, &ssid, sizeof(wlc_ssid_t));
		memcpy(host->apInfo->ssid, ssid.SSID, 32);
		host->apInfo->ssidlen = ssid.SSID_len;
#endif
		host->RSSI = 0;
#ifdef HAVE_MADWIFI
		host->apInfo->channel = wifi_getchannel(wl_dev);
#elif HAVE_RT2880
		if (nvram_match("wifi_display", "wl0"))
			host->apInfo->channel = atoi(nvram_safe_get("wl0_channel"));
		else
			host->apInfo->channel = atoi(nvram_safe_get("wl1_channel"));
#else
		wl_ioctl(wl_dev, WLC_GET_CHANNEL, &channel, sizeof(channel_info_t));
		host->apInfo->channel = channel.hw_channel;
#endif

		macs = (maclist_t *) malloc(4 + MAX_STA_COUNT * sizeof(ether_addr_t));
		macs->count = MAX_STA_COUNT;
		int code = getassoclist(wl_dev, macs);
		printf("code :%d\n", code);
		if (code > 0) {
			for (i = 0; i < macs->count; i++) {
				sta = gotHost(cfg, (char *)&macs->ea[i], typeSta);
#ifdef HAVE_MADWIFI
				sta->RSSI = -getRssi(wl_dev, macs->ea) * 100;
#elif HAVE_RT2880
				sta->RSSI = -getRssi(wl_dev, macs->ea) * 100;	// needs to be solved                            
#else
				memcpy(starssi.mac, &macs->ea[i], 6);
				starssi.RSSI = 3000;
				starssi.zero_ex_forty_one = 0x41;
				if (wl_ioctl(wl_dev, WLC_GET_RSSI, &starssi, 12) < 0)
					printf("rssifail\n");
				sta->RSSI = -starssi.RSSI * 100;
#endif
				sta->staInfo->state = ssAssociated;
				memcpy(sta->staInfo->connectedBSSID, host->apInfo->bssid, 6);
			}
		}
	} else {
		host = gotHost(cfg, mac, typeSta);
		host->isSelf = 1;
		host->RSSI = 0;
#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880)
		if (getassoclist(wl_dev, macs) > -1) {
			if (macs->count > 0) {
				host->staInfo->state = ssUnassociated;
			} else {
				host->staInfo->state = ssAssociated;
			}
		} else {
			host->staInfo->state = ssUnassociated;

		}

#else
		if (wl_ioctl(wl_dev, WLC_GET_BSSID, &host->staInfo->connectedBSSID, 6) < 0) {
			host->staInfo->state = ssUnassociated;
		} else {
			host->staInfo->state = ssAssociated;
		}
#endif
	}
#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880)
	cfg->curChannel = wifi_getchannel(wl_dev);

#else
	if (wl_ioctl(wl_dev, WLC_GET_CHANNEL, &channel, sizeof(channel_info_t)) >= 0) {
		cfg->curChannel = channel.hw_channel;
		printf("Current channel is %i\n", cfg->curChannel);
	}
#endif
}
