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
	if (instance==0)
	    return "ra0";
	return "ra1";
}

int get_wl_instances(void)
{
	if (ifexists("ra1"))
	    return 2;
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
	if (get_wl_instance("eth1") == 1)
		return 2;
	if (get_wl_instance("eth2") == 1)
		return 2;
	if (get_wl_instance("eth3") == 1)
		return 2;
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
	else
		return pwr;

	sprintf(cmd, "wl -i %s txpwr1", get_wl_instance_name(c));

	FILE *in = popen(cmd, "rb");
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

	if (isListed("probe_blacklist", name))
		return -1;

#if 0				//defined(linux)
	char buf[DEV_TYPE_LEN];

	if ((ret = wl_get_dev_type(name, buf, DEV_TYPE_LEN)) < 0) {
//              fprintf(stderr,"dev type=%s fail\n",name);
		addList("probe_blacklist", name);
		return ret;
	}
	/*
	 * Check interface 
	 */
//      fprintf(stderr,"dev type=%s\n",buf);
	if (strncmp(buf, "wl", 2)) {
		addList("probe_blacklist", name);
		return -1;
	}
#else
	/*
	 * Check interface 
	 */
	if ((ret = wl_ioctl(name, WLC_GET_MAGIC, &val, sizeof(val)))) {
		//    fprintf(stderr,"magic fail\n");
		addList("probe_blacklist", name);
		return ret;
	}
#endif
	if ((ret = wl_ioctl(name, WLC_GET_VERSION, &val, sizeof(val)))) {
		//    fprintf(stderr,"version fail\n");
		addList("probe_blacklist", name);
		return ret;
	}
	if (val > WLC_IOCTL_VERSION) {
		//    fprintf(stderr,"version fail %d\n",val);
		addList("probe_blacklist", name);
		return -1;
	}
	return ret;
}
