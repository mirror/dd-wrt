#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/klog.h>
#include <netdb.h>
#include <unistd.h>
#include <broadcom.h>

/*
 * barry add 20031027 
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_DEV "/proc/net/dev"

#if 1
#else
#define dprintf(fmt, args...)
#endif

static int mysystem(char *cmd)
{
	FILE *fp = popen(cmd, "rb");
	pclose(fp);
	return 0;
}

void ej_wl_packet_get(webs_t wp, int argc, char_t **argv)
{
	char line[256];
	FILE *fp;
	struct dev_info {
		char ifname[10];
		unsigned long rx_bytes;
		unsigned long rx_pks;
		unsigned long rx_errs;
		unsigned long rx_drops;
		unsigned long rx_fifo;
		unsigned long rx_frame;
		unsigned long rx_com;
		unsigned long rx_mcast;
		unsigned long tx_bytes;
		unsigned long tx_pks;
		unsigned long tx_errs;
		unsigned long tx_drops;
		unsigned long tx_fifo;
		unsigned long tx_colls;
		unsigned long tx_carr;
		unsigned long tx_com;
	} info;

	info.rx_pks = info.rx_errs = info.rx_drops = 0;
	info.tx_pks = info.tx_errs = info.tx_drops = info.tx_colls = 0;

	if ((fp = fopen(PROC_DEV, "r")) == NULL) {
		websError(wp, 400, "Can't open %s\n", PROC_DEV);
		return;
	} else {
		/*
		 * Inter-| Receive | Transmit face |bytes packets errs drop fifo
		 * frame compressed multicast|bytes packets errs drop fifo colls
		 * carrier compressed lo: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 eth0:
		 * 674829 5501 0 0 0 0 0 0 1249130 1831 0 0 0 0 0 0 eth1: 0 0 0 0 0 0 
		 * 0 0 0 0 0 0 0 0 0 0 eth2: 0 0 0 0 0 719 0 0 1974 16 295 0 0 0 0 0
		 * br0: 107114 1078 0 0 0 0 0 0 910094 1304 0 0 0 0 0 0
		 * 
		 */
		while (fgets(line, sizeof(line), fp) != NULL) {
			int ifl = 0;

			if (!strchr(line, ':'))
				continue;
			while (line[ifl] != ':')
				ifl++;
			line[ifl] = 0; /* interface */
#ifdef HAVE_MADWIFI
			if (strstr(line, "wifi0"))
#else
			if (strstr(line, nvram_safe_get("wl0_ifname")))
#endif
			{
				sscanf(line + ifl + 1, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
				       &info.rx_bytes, &info.rx_pks, &info.rx_errs, &info.rx_drops, &info.rx_fifo, &info.rx_frame,
				       &info.rx_com, &info.rx_mcast, &info.tx_bytes, &info.tx_pks, &info.tx_errs, &info.tx_drops,
				       &info.tx_fifo, &info.tx_colls, &info.tx_carr, &info.tx_com);
			}
		}
		fclose(fp);
	}

	websWrite(wp, "SWRXgoodPacket=%ld;", info.rx_pks);
	websWrite(wp, "SWRXerrorPacket=%ld;", info.rx_errs + info.rx_drops);

	websWrite(wp, "SWTXgoodPacket=%ld;", info.tx_pks);
	websWrite(wp, "SWTXerrorPacket=%ld;", info.tx_errs + info.tx_drops + info.tx_colls);

	return;
}

int StartContinueTx(webs_t wp, char *value)
{
	int ret = 0;
	char buf[80];
	int channel;
	int rate;
	float rates;
	int gmode;
	int txant; // barry add 1117
	char *tx_gmode;
	char *tx_channel;
	char *tx_rate;
	char *tx_ant; // barry add 1117
	FILE *fp;

	dprintf("init, StartContinueTx=[%s]\n", value);

	tx_gmode = websGetVar(wp, "wl_gmode", NULL);
	tx_channel = websGetVar(wp, "wl_channel", NULL);
	tx_rate = websGetVar(wp, "wl_rate", NULL);
	tx_ant = websGetVar(wp, "TXANT", NULL);

	if (tx_channel)
		channel = atoi(tx_channel);
	if (tx_rate) {
		rate = atoi(tx_rate);
		rates = rate / 1000000;
		if (rates == 5)
			rates = 5.5;
	}
	if (tx_gmode)
		gmode = atoi(tx_gmode);
	if (tx_ant)
		txant = atoi(tx_ant);

	dprintf("gmode=[%s](%d), channel=[%s](%d), rate=[%s](%d), rates=(%f), txant=[%s](%d)\n", tx_gmode ? tx_gmode : "NULL",
		tx_gmode ? gmode : -1, tx_channel ? tx_channel : "NULL", tx_channel ? channel : -1, tx_rate ? tx_rate : "NULL",
		tx_rate ? rate : -1, tx_rate ? rates : -1, tx_ant ? tx_ant : "NULL", tx_ant ? txant : -1);

	printf("value=[%d]\n", atoi(value));
	switch (atoi(value)) {
	case 0:
		StopContinueTx(wp, value);
		break;
	case 1:
		/*
		 * Start Continue TX, EVM 
		 */
		// mysystem("wl down");
		// mysystem("wl clk 1");
		// mysystem("wl gmode 0");
		mysystem("nvram set wl_bcn=1");
		mysystem("nvram set wl0_bcn=1");
		if (check_hw_type() == BCM4702_CHIP) /* barry add for 4712 
							 * or 4702 RF test */
			mysystem("wlconf eth2 up"); // For 4702
		else
			mysystem("wlconf eth1 up");

		if (txant == 0) {
			mysystem("wl antdiv 0");
			mysystem("wl txant 3");
			mysystem("wl antdiv");
			mysystem("wl txant");
		} else {
			mysystem("wl antdiv 1");
			mysystem("wl txant 3");
			mysystem("wl antdiv");
			mysystem("wl txant");
		}

		mysystem("wl rateset 11b 54");
		mysystem("wl rate 54");
		mysystem("wl txpwr1 -d -o 16.5"); // 2005-03-04, Set tx power
		// to 16 dbm
		mysystem("wl curpower > /tmp/curpower");

		if ((fp = fopen("/tmp/curpower", "r"))) { // Get real value
			char line[254];
			char string[254];
			char value[254];
			char patt1[] = "Last B phy CCK est. power:";
			char patt2[] = "Last B phy OFDM est. power:";

			bzero(line, sizeof(line));

			while (fgets(line, sizeof(line), fp) != NULL) {
				bzero(string, sizeof(string));
				bzero(value, sizeof(value));

				sscanf(line, "%27c%s", string, value);
				if (!memcmp(string, patt1, strlen(patt1))) {
					nvram_set("wl_cck_result", value);
				} else if (!memcmp(string, patt2, strlen(patt2))) {
					nvram_set("wl_ofdm_result", value);
				}
			}
		}
		fclose(fp);
		mysystem("wl out");
		// mysystem("wl clk");
		// sprintf(buf,"wl fqacurcy %d",channel);
		// mysystem(buf);

		sprintf(buf, "wl evm %d %f &", channel, rates);
		mysystem(buf);

		break;
	case 2:
		dprintf("\nOnly set ANT!\n");
		if (txant == 0) {
			mysystem("wl antdiv 0");
			mysystem("wl txant 3");
		} else {
			mysystem("wl antdiv 1");
			mysystem("wl txant 3");
		}
		break;
	default:
		dprintf("Illegal StartContinueTx parameter : [%s]\n", value);
	}

	dprintf("done\n");
	return ret;
}

int StopContinueTx(webs_t wp, char *value)
{
	int ret = 0;
	char *type;

	dprintf("init, StopContinueTx=[%s]\n", value);

	type = websGetVar(wp, "StopContinueTx", "");
	if (!strcmp(type, "0")) {
		//
	} else if (!strcmp(type, "1")) {
		/*
		 * Stop Continue TX, EVM 
		 */
		mysystem("wl evm 0");
		mysystem("wl up");
		mysystem("nvram set wl_bcn=100");
		mysystem("nvram set wl0_bcn=100");
		if (check_hw_type() == BCM4702_CHIP) /* barry add for 4712 or 4702 
							 * RF test */
			mysystem("wlconf eth2 up"); // For 4702
		else
			mysystem("wlconf eth1 up");
	}

	dprintf("done\n");
	return ret;
}

int Check_TSSI(webs_t wp, char *value)
{
	int atten_bb;
	int atten_radio;
	int atten_ctl;
	int idelay = 0;

	// int tssi_check ;
	char *wl_atten_bb = NULL;
	char *wl_atten_radio;
	char *wl_atten_ctl;
	char *wl_delay;

	// char *wl_tssi_check ;
	FILE *fp;
	int icck, iofdm;
	char ori[80];
	char buf[80];
	char buf2[80];
	char buf3[80];
	char cck[80], ofdm[80];

	dprintf("init, Check_TSSI=[%s]\n", value);

	wl_atten_radio = websGetVar(wp, "WL_atten_radio", NULL);
	wl_atten_ctl = websGetVar(wp, "WL_atten_ctl", NULL);
	// wl_tssi_check = websGetVar(wp, "WL_tssi_check", NULL);
	wl_delay = websGetVar(wp, "WL_delay", NULL);

	nvram_set("wl_atten_bb", value);
	nvram_set("wl_atten_radio", wl_atten_radio);
	nvram_set("wl_atten_ctl", wl_atten_ctl);
	nvram_set("wl_delay", wl_delay);
	// nvram_set("wl_tssi_check",wl_tssi_check);

	atten_bb = nvram_geti("wl_atten_bb");
	atten_radio = nvram_geti("wl_atten_radio");
	atten_ctl = nvram_geti("wl_atten_ctl");
	idelay = nvram_geti("wl_delay");
	// tssi_check=nvram_geti("wl_tssi_check"));

	dprintf("wl_atten_bb=[%s], wl_atten_radio=[%s], wl_atten_ctl=[%s]\n", wl_atten_bb, wl_atten_radio, wl_atten_ctl);

	bzero(buf, sizeof(buf));
	sprintf(buf, "wl atten %s %s %s", value, wl_atten_radio, wl_atten_ctl);
	mysystem(buf);

	/*
	 * wait for a few seconds 
	 */
	dprintf("Will delay %d seconds\n", idelay);
	if (idelay != 0)
		sleep(idelay);

	mysystem("wl tssi > /tmp/get_tssi");
	bzero(buf, sizeof(buf));
	bzero(cck, sizeof(cck));
	bzero(cck, sizeof(ofdm));
	if ((fp = fopen("/tmp/get_tssi", "r"))) {
		fgets(buf, sizeof(buf), fp);
		strcpy(ori, buf);
		dprintf("\nGot:\n%s\nlen=%d\n", buf, strlen(buf));
	} else
		dprintf("\nFile error!\n");

	bzero(buf2, sizeof(buf2));
	strcpy(buf2, strtok(buf, ","));
	bzero(buf3, sizeof(buf3));
	strcpy(buf3, strtok(buf2, "CCK"));
	strcpy(buf3, strtok(NULL, "CCK"));
	dprintf("CCK:[%s]\n", buf3);
	strcpy(cck, buf3);
	icck = atoi(buf3);

	bzero(buf2, sizeof(buf2));
	strcpy(buf2, strtok(ori, ","));
	strcpy(buf2, strtok(NULL, ","));
	bzero(buf3, sizeof(buf3));
	strcpy(buf3, strtok(buf2, "OFDM"));
	strcpy(buf3, strtok(NULL, "OFDM"));
	dprintf("OFDM:[%s]\n", buf3);
	strcpy(ofdm, buf3);
	iofdm = atoi(buf3);

	dprintf("CCK=[%s](%d),OFDM=[%s](%d)\n", cck, icck, ofdm, iofdm);

	fclose(fp);

	nvram_set("wl_cck", cck);
	nvram_set("wl_ofdm", ofdm);
	nvram_set("wl_tssi_result", cck);
	nvram_commit();

	// if(tssi_check == icck)
	// {
	// printf("\nMatch!\n");
	// nvram_set("wl_tssi_result","1");
	// }
	// else
	// {
	// printf("\nDisMatch!\n");
	// nvram_set("wl_tssi_result","0");
	// }

	dprintf("done\n");

	return 1;
}

int Get_TSSI(char *value)
{
	char cck[80], ofdm[80];

	dprintf("init, Get_TSSI=[%s]\n", value);

	bzero(cck, sizeof(cck));
	bzero(ofdm, sizeof(ofdm));

	strcpy(cck, nvram_safe_get("wl_cck"));
	strcpy(ofdm, nvram_safe_get("wl_ofdm"));

	dprintf("\nCCK=[%s],OFDM=[%s]\n", cck, ofdm);

	dprintf("done\n");

	return 1;
}

int Enable_TSSI(char *value)
{
	int ret;

	dprintf("\ninit, value=[%s]\n", value);

	ret = mysystem("wl txpwr1 -d -o 16.5"); // 2005-03-08

	dprintf("done\n");
	/*
	 * int enabled; int atten_bb; int atten_radio; int atten_ctl; char
	 * buf[80];
	 * 
	 * enabled=atoi(value); if(enabled) {
	 * atten_bb=nvram_geti("wl_atten_bb"));
	 * atten_radio=nvram_geti("wl_atten_radio"));
	 * atten_ctl=nvram_geti("wl_atten_ctl"));
	 * 
	 * bzero(buf,sizeof(buf)); sprintf(buf,"wl atten %d %d
	 * %d",atten_bb,atten_radio,atten_ctl); mysystem(buf); } 
	 */
	return ret;
}

int Change_Ant(char *value)
{
	dprintf("init, Change_Ant=[%s]\n", value);

	switch (atoi(value)) {
	case 0:
		mysystem("wl antdiv 0");
		mysystem("wl txant 3");
		mysystem("wl antdiv");
		mysystem("wl txant");

		break;
	case 1:
		mysystem("wl antdiv 1");
		mysystem("wl txant 3");
		mysystem("wl antdiv");
		mysystem("wl txant");
		break;
	default:
		dprintf("Illegal ChangeANT parameter : [%s]\n", value);
	}

	dprintf("done\n");

	return 0;
}

int StartContinueTx_4702(webs_t wp, char *value)
{
	int ret = 0;
	char buf[80];
	int channel;
	int rate;
	float rates;
	int gmode;
	char *type;
	char *tx_gmode;
	char *tx_channel;
	char *tx_rate;

	tx_gmode = websGetVar(wp, "wl_gmode", NULL);
	tx_channel = websGetVar(wp, "wl_channel", NULL);
	tx_rate = websGetVar(wp, "wl_rate", NULL);

	printf("\ngmode=%s,channel=%s,rate=%s\n", tx_gmode, tx_channel, tx_rate);
	channel = atoi(tx_channel);
	rate = atoi(tx_rate);
	gmode = atoi(tx_gmode);

	type = websGetVar(wp, "StartContinueTx", "");
	printf("\ngmode=%d,channel=%d,rate=%d\n", gmode, channel, rate);

	if (!strcmp(type, "0")) {
		//
	} else if (!strcmp(type, "1")) {
		/*
		 * Start Continue TX, EVM 
		 */
		// mysystem("wl down");
		// mysystem("wl clk 1");
		// mysystem("wl gmode 0");
		mysystem("nvram set wl_bcn=1");
		mysystem("nvram set wl0_bcn=1");
		mysystem("wlconf eth2 up");

		mysystem("wl txant 0");
		mysystem("wl out");
		mysystem("wl clk");
		// sprintf(buf,"wl fqacurcy %d",channel);
		// mysystem(buf);

		// channel=nvram_geti("wl_channel"));
		// rate=nvram_geti("wl_rate"));
		rates = rate / 1000000;
		printf("\nrate=%d, rates=%f\n", rate, rates);
		if (rates == 5)
			rates = 5.5;
		sprintf(buf, "wl evm %d %f &", channel, rates);
		mysystem(buf);
		printf("\nStartContinueTx, exec:%s\n", buf);
	}
	return ret;
}

int StopContinueTx_4702(webs_t wp, char *value)
{
	int ret = 0;
	char *type;

	type = websGetVar(wp, "StopContinueTx", "");
	if (!strcmp(type, "0")) {
		//
	} else if (!strcmp(type, "1")) {
		/*
		 * Stop Continue TX, EVM 
		 */
		mysystem("wl evm 0");
		mysystem("wl up");
		mysystem("nvram set wl_bcn=100");
		mysystem("nvram set wl0_bcn=100");
		mysystem("wlconf eth2 up");
		printf("\nStopContinueTx\n");
	}

	return ret;
}
