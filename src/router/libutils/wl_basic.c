#include <string.h>
#include <unistd.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <wlutils.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>
//#include <math.h>

#if defined(HAVE_RT2880) || defined(HAVE_RT61)
char *get_wl_instance_name(int instance)
{
	if (instance == 0)
		return "ra0";
	return "ba0";
}

int get_wl_instances(void)
{
	FILE *fp = fopen("/sys/bus/pci/devices/0000:01:00.0/device", "rb");
	if (fp) {
		fclose(fp);
		return 2;
	}
	return 1;
}

int get_wl_instance(char *name)
{
	return 1;
}

#elif HAVE_MADWIFI

char *get_wl_instance_name(int instance)
{
	return "ath0";
}

int get_wl_instances(void)
{
	return 1;
}

int get_wl_instance(char *name)
{
	return 1;
}

#else
char *get_wl_instance_name(int instance)
{
#ifdef HAVE_MVEBU
	fprintf(stderr, "get_wl_instance_name \n");
	/*if (instance == 1)
	   return "wlan0";
	   if (instance == 2)
	   return "wlan1"; */

#endif
#ifdef HAVE_QTN
	if (instance == 1)
		return "qtn";
#endif
	if (get_wl_instance("eth1") == instance)
		return "eth1";
	if (get_wl_instance("eth2") == instance)
		return "eth2";
	if (get_wl_instance("eth0") == instance)
		return "eth0";
	if (get_wl_instance("eth3") == instance)
		return "eth3";
	fprintf(stderr, "get_wl_instance doesnt return the right value %d\n", instance);
	return nvram_safe_get("wl0_ifname");	// dirty for debugging
}

int get_wl_instances(void)
{
#ifdef HAVE_MVEBU
	return 2;
#endif
#ifdef HAVE_QTN
	return 2;
#else
	if (get_wl_instance("eth3") == 2)
		return 3;
	if (get_wl_instance("eth1") == 1)
		return 2;
	if (get_wl_instance("eth2") == 1)
		return 2;
	if (get_wl_instance("eth3") == 1)
		return 2;
#endif
	return 1;
}

int get_wl_instance(char *name)
{
	int unit;
	int ret;
	if (!ifexists(name))
		return -1;
	if (wl_probe(name))
		return -1;
	ret = wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit));
//      fprintf(stderr,"wl_instance = %d\n",unit);
	if (ret == 0)
		return unit;
	return ret;
}

int bcm_gettxpower(char *wlname)
{
	int pwr = 0;
	int realpwr;
	int c;
	char cmd[32];
	char wl[16];
	sprintf(wl, "%s_txpwr", wlname);
	pwr = atoi(nvram_safe_get(wl));
	if (!strcmp(wlname, "wl0"))
		c = 0;
	else if (!strcmp(wlname, "wl1"))
		c = 1;
	else if (!strcmp(wlname, "wl2"))
		c = 2;
	else
		return pwr;
#ifdef HAVE_QTN
	if (c == 1)
		return atoi(nvram_safe_get("wl1_txpwr"));
#endif
	sprintf(cmd, "wl -i %s txpwr1", get_wl_instance_name(c));
	FILE *in = popen(cmd, "r");
	if (in == NULL)
		return pwr;
	// TxPower is 74 qdbm,  18.50 dbm, 71 mW  Override is Off               
	if (fscanf(in, "%*s %*s %*s %*s %*s %*s %d", &realpwr) == 1)
		pwr = realpwr;
	pclose(in);
	return pwr;
}

#endif

    /*
     * return 1st wireless interface 
     */
char *get_wdev(void)
{
#ifdef HAVE_MADWIFI
	if (nvram_match("wifi_bonding", "1"))
		return "bond0";
	else {
		return "ath0";
	}
#elif defined(HAVE_RT2880) || defined(HAVE_RT61)
	return "ra0";
#else
	return get_wl_instance_name(0);
#endif
}

int wl_probe(char *name)
{
	int ret, val;
	char buf[DEV_TYPE_LEN];
	if (isListed("probe_blacklist", name))
		return -1;
	if ((ret = wl_get_dev_type(name, buf, DEV_TYPE_LEN)) < 0)
		return ret;
	/* Check interface */
	if (strncmp(buf, "wl", 2)) {
		addList("probe_blacklist", name);
		return -1;
	}
#if 0
	/*
	 * Check interface 
	 */
	if ((ret = wl_ioctl(name, WLC_GET_MAGIC, &val, sizeof(val)))) {
		//fprintf(stderr,"WLC_GET_MAGIC fail: %s\n", name);
#ifdef HAVE_DHDAP
		if (dhd_probe(name)) {
#endif
			addList("probe_blacklist", name);
			return ret;
#ifdef HAVE_DHDAP
		}
#endif
	}
#endif

	if ((ret = wl_ioctl(name, WLC_GET_VERSION, &val, sizeof(val)))) {
		fprintf(stderr, "WLC_GET_VERSION fail: %s\n", name);
		addList("probe_blacklist", name);
		return ret;
	}

	if (val > WLC_IOCTL_VERSION) {
		fprintf(stderr, "WLC_IOCTL_VERSION fail name: %s val: %d ictlv: %d \n", name, val, WLC_IOCTL_VERSION);
		addList("probe_blacklist", name);
		return -1;
	}
	return ret;
}

#ifdef HAVE_DHDAP
#include <sys/ioctl.h>
#include <net/if.h>
#include <dhdioctl.h>
/*
 * Probe the specified interface.
 * @param	name	interface name
 * @return	0       if using dhd driver
 *          <0      otherwise
 */
int dhd_probe(char *name)
{
	int ret, val;
	val = 0;
	/* Check interface */
	ret = dhd_ioctl(name, DHD_GET_MAGIC, &val, sizeof(val));
	if (val == WLC_IOCTL_MAGIC) {
		ret = 1;	/* is_dhd = !dhd_probe(), so ret 1 for WL */
	} else if (val == DHD_IOCTL_MAGIC) {
		ret = 0;
	} else {
		ret = 1;	/* default: WL mode */
	}
	return ret;
}

int dhd_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	uint namelen;
	uint iolen;
	namelen = strlen(iovar) + 1;	/* length of iovar name plus null */
	iolen = namelen + paramlen;
	/* check for overflow */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);
	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8 *) bufptr + namelen, param, paramlen);
	return dhd_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}

static int dhd_bssiovar_mkbuf(char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen, unsigned int *plen)
{
	char *prefix = "bsscfg:";
	int8 *p;
	uint prefixlen;
	uint namelen;
	uint iolen;
	prefixlen = strlen(prefix);	/* length of bsscfg prefix */
	namelen = strlen(iovar) + 1;	/* length of iovar name + null */
	iolen = prefixlen + namelen + sizeof(int) + paramlen;
	if (buflen < 0 || iolen > (uint) buflen) {
		*plen = 0;
		return BCME_BUFTOOSHORT;
	}

	p = (int8 *) bufptr;
	memcpy(p, prefix, prefixlen);
	p += prefixlen;
	memcpy(p, iovar, namelen);
	p += namelen;
	memcpy(p, &bssidx, sizeof(int32));
	p += sizeof(int32);
	if (paramlen)
		memcpy(p, param, paramlen);
	*plen = iolen;
	return 0;
}

int dhd_iovar_set(char *ifname, char *iovar, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];
	return dhd_iovar_setbuf(ifname, iovar, param, paramlen, smbuf, sizeof(smbuf));
}

/*
 * set named driver variable to int value
 * calling example: dhd_iovar_setint(ifname, "arate", rate)
*/
int dhd_iovar_setint(char *ifname, char *iovar, int val)
{
	return dhd_iovar_set(ifname, iovar, &val, sizeof(val));
}

int dhd_ioctl(char *name, int cmd, void *buf, int len)
{
	struct ifreq ifr;
	dhd_ioctl_t ioc;
	int ret = 0;
	int s;
	char buffer[WLC_IOCTL_SMLEN];
	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	/* do it */
	if (cmd == WLC_SET_VAR) {
		cmd = DHD_SET_VAR;
	} else if (cmd == WLC_GET_VAR) {
		cmd = DHD_GET_VAR;
	}

	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	ioc.set = FALSE;
	ioc.driver = DHD_IOCTL_MAGIC;
	ioc.used = 0;
	ioc.needed = 0;
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
	ifr.ifr_data = (caddr_t) & ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0)
		if (cmd != WLC_GET_MAGIC && cmd != WLC_GET_BSSID) {
			if ((cmd == WLC_GET_VAR) || (cmd == WLC_SET_VAR)) {
				snprintf(buffer, sizeof(buffer), "%s: WLC_%s_VAR(%s)", name, cmd == WLC_GET_VAR ? "GET" : "SET", (char *)buf);
			} else {
				snprintf(buffer, sizeof(buffer), "%s: cmd=%d", name, cmd);
			}
			perror(buffer);
		}
	/* cleanup */
	close(s);
	return ret;
}

/*
 * set named & bss indexed driver variable to buffer value
 */
int dhd_bssiovar_setbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	int iolen;
	err = dhd_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &iolen);
	if (err)
		return err;
	return dhd_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}

/*
 * set named & bss indexed driver variable to buffer value
 */
int dhd_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];
	return dhd_bssiovar_setbuf(ifname, iovar, bssidx, param, paramlen, smbuf, sizeof(smbuf));
}

/*
 * set named & bss indexed driver variable to int value
 */
int dhd_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val)
{
	return dhd_bssiovar_set(ifname, iovar, bssidx, &val, sizeof(int));
}

#endif				/* __CONFIG_DHDAP__ */
