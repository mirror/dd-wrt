/*
 * Wireless network adapter utilities
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wl.c,v 1.3 2005/11/11 09:26:19 seg Exp $
 */
#include <string.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <wlutils.h>
#include <shutils.h>
#include <bcmnvram.h>


struct nvram_tuple router_defaults[] = {
  {0, 0, 0}
};



/* DD-WRT addition  (loaned from radauth) */

#ifndef HAVE_MADWIFI
int
getchannels (unsigned int *list)
{
 // int ret, num;
//  num = (sizeof (*list) - 4) / 6;	/* Maximum number of entries in the buffer */
//  memcpy (list, &num, 4);	/* First 4 bytes are the number of ent. */

//  ret = wl_ioctl (name, WLC_GET_VALID_CHANNELS, list, 128);
FILE *in=popen("wl channels","r");
#ifndef HAVE_MSSID
while(fgetc(in)!=':');
#endif
int chan;
int count=0;
while (fscanf(in,"%d",&chan)!=EOF)
    {
    list[count++]=chan;
    }
return count;
}

int
getassoclist (char *name, unsigned char *list)
{
//int ap;
//if ((wl_ioctl(name, WLC_GET_AP, &ap, sizeof(ap)) < 0) || ap) 
//{
  int ret, num;

  num = (sizeof (*list) - 4) / 6;	/* Maximum number of entries in the buffer */
  memcpy (list, &num, 4);	/* First 4 bytes are the number of ent. */

  ret = wl_ioctl (name, WLC_GET_ASSOCLIST, list, 8192);
//}else
//{
//char buf[WLC_IOCTL_MAXLEN];
/*
	wl_bss_info_t *bss_info = (wl_bss_info_t *) buf;
	memset(buf,0,WLC_IOCTL_MAXLEN);
	if (wl_ioctl(name, WLC_GET_BSS_INFO, bss_info, WLC_IOCTL_MAXLEN)<0)return 0;
	struct maclist *l = (struct maclist*)list;
*/
/*	sta_info_t *sta_info = (sta_info_t *) buf;
	memset(buf,0,WLC_IOCTL_MAXLEN);
	if (wl_ioctl(name, WLC_GET_VAR, sta_info, WLC_IOCTL_MAXLEN)<0)return 0;

	struct maclist *l = (struct maclist*)list;
	l->count=1;
	memcpy(&l->ea,&sta_info->ea,6);*/
return ret;
}

int
getwdslist (char *name, unsigned char *list)
{
  int ret, num;

  num = (sizeof (*list) - 4) / 6;	/* Maximum number of entries in the buffer */
  memcpy (list, &num, 4);	/* First 4 bytes are the number of ent. */

  ret = wl_ioctl (name, WLC_GET_WDSLIST, list, 8192);

  return (ret);
}

int getNoise(char *name)
{
	unsigned int noise;
	//rssi = 0;
        //char buf[WLC_IOCTL_MAXLEN];
	if (wl_ioctl(name, WLC_GET_PHY_NOISE, &noise, sizeof(noise)) < 0)

	/*wl_bss_info_t *bss_info = (wl_bss_info_t *) buf;
	memset(buf,0,WLC_IOCTL_MAXLEN);
	
	wl_ioctl(name, WLC_GET_BSS_INFO, bss_info, WLC_IOCTL_MAXLEN);
	if ((wl_ioctl(name, WLC_GET_AP, &ap, sizeof(ap)) < 0) || ap) {
		if (wl_ioctl(name, WLC_GET_PHY_NOISE, &noise, sizeof(noise)) < 0)
			noise = 0;
	} else {
		// somehow the structure doesn't fit here
		rssi = buf[82];
		noise = buf[84];
	}*/
return noise;
}



/*
struct iw_statistics *wlcompat_get_wireless_stats(struct net_device *dev)
{
	wl_bss_info_t *bss_info = (wl_bss_info_t *) buf;
	get_pktcnt_t pkt;
	unsigned int rssi, noise, ap;
	
	memset(&wstats, 0, sizeof(wstats));
	memset(&pkt, 0, sizeof(pkt));
	memset(buf, 0, sizeof(buf));
	bss_info->version = 0x2000;
	wl_ioctl(dev, WLC_GET_BSS_INFO, bss_info, WLC_IOCTL_MAXLEN);
	wl_ioctl(dev, WLC_GET_PKTCNTS, &pkt, sizeof(pkt));

	rssi = 0;
	if ((wl_ioctl(dev, WLC_GET_AP, &ap, sizeof(ap)) < 0) || ap) {
		if (wl_ioctl(dev, WLC_GET_PHY_NOISE, &noise, sizeof(noise)) < 0)
			noise = 0;
	} else {
		// somehow the structure doesn't fit here
		rssi = buf[82];
		noise = buf[84];
	}
	rssi = (rssi == 0 ? 1 : rssi);
	wstats.qual.updated = 0x10;
	if (rssi <= 1) 
		wstats.qual.updated |= 0x20;
	if (noise <= 1)
		wstats.qual.updated |= 0x40;

	if ((wstats.qual.updated & 0x60) == 0x60)
		return NULL;

	wstats.qual.level = rssi;
	wstats.qual.noise = noise;
	wstats.discard.misc = pkt.rx_bad_pkt;
	wstats.discard.retries = pkt.tx_bad_pkt;

	return &wstats;
}*/

#else
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


#include "wireless.h"
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"


static int
getsocket (void)
{
  static int s = -1;

  if (s < 0)
    {
      s = socket (AF_INET, SOCK_DGRAM, 0);
      if (s < 0)
	err (1, "socket(SOCK_DGRAM)");
    }
  return s;
}


#define IOCTL_ERR(x) [x - SIOCIWFIRSTPRIV] "ioctl[" #x "]"
static int
do80211priv (struct iwreq *iwr, const char *ifname, int op, void *data,
	     size_t len)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))

  memset (iwr, 0, sizeof (struct iwreq));
  strncpy (iwr->ifr_name, ifname, IFNAMSIZ);
  if (len < IFNAMSIZ)
    {
      /*
       * Argument data fits inline; put it there.
       */
      memcpy (iwr->u.name, data, len);
    }
  else
    {
      /*
       * Argument data too big for inline transfer; setup a
       * parameter block instead; the kernel will transfer
       * the data for the driver.
       */
      iwr->u.data.pointer = data;
      iwr->u.data.length = len;
    }

  if (ioctl (getsocket (), op, iwr) < 0)
    {
      static const char *opnames[] = {
	IOCTL_ERR (IEEE80211_IOCTL_SETPARAM),
	IOCTL_ERR (IEEE80211_IOCTL_GETPARAM),
	IOCTL_ERR (IEEE80211_IOCTL_SETMODE),
	IOCTL_ERR (IEEE80211_IOCTL_GETMODE),
	IOCTL_ERR (IEEE80211_IOCTL_SETWMMPARAMS),
	IOCTL_ERR (IEEE80211_IOCTL_GETWMMPARAMS),
	IOCTL_ERR (IEEE80211_IOCTL_SETCHANLIST),
	IOCTL_ERR (IEEE80211_IOCTL_GETCHANLIST),
	IOCTL_ERR (IEEE80211_IOCTL_CHANSWITCH),
	IOCTL_ERR (IEEE80211_IOCTL_GETCHANINFO),
	IOCTL_ERR (IEEE80211_IOCTL_SETOPTIE),
	IOCTL_ERR (IEEE80211_IOCTL_GETOPTIE),
	IOCTL_ERR (IEEE80211_IOCTL_SETMLME),
	IOCTL_ERR (IEEE80211_IOCTL_SETKEY),
	IOCTL_ERR (IEEE80211_IOCTL_DELKEY),
	IOCTL_ERR (IEEE80211_IOCTL_ADDMAC),
	IOCTL_ERR (IEEE80211_IOCTL_DELMAC),
	IOCTL_ERR (IEEE80211_IOCTL_WDSADDMAC),
	IOCTL_ERR (IEEE80211_IOCTL_WDSDELMAC),
      };
      op -= SIOCIWFIRSTPRIV;
      if (0 <= op && op < N (opnames))
	perror (opnames[op]);
      else
	perror ("ioctl[unknown???]");
      return -1;
    }
  return 0;
#undef N
}

int
get80211priv (const char *ifname, int op, void *data, size_t len)
{
  struct iwreq iwr;

  if (do80211priv (&iwr, ifname, op, data, len) < 0)
    return -1;
  if (len < IFNAMSIZ)
    memcpy (data, iwr.u.name, len);
  return iwr.u.data.length;
}


struct wifi_channels
{
  int channel;
  int freq;
};

static u_int
ieee80211_mhz2ieee (u_int freq)
{
  if (freq == 2484)
    return 14;
  if (freq < 2484)
    return (freq - 2407) / 5;
  if (freq < 5000)
    return 15 + ((freq - 2512) / 20);
  return (freq - 5000) / 5;
}

static struct wifi_channels *
list_channelsext (const char *ifname, int allchans)
{
  struct ieee80211req_chaninfo chans;
  struct ieee80211req_chaninfo achans;
  const struct ieee80211_channel *c;
  int i, half;
  fprintf (stderr, "list channels for %s\n", ifname);
  if (get80211priv
      (ifname, IEEE80211_IOCTL_GETCHANINFO, &chans, sizeof (chans)) < 0)
    {
    fprintf(stderr,"unable to get channel information\n");
      return NULL;
    }
  if (!allchans)
    {
      uint8_t active[32];

      if (get80211priv
	  (ifname, IEEE80211_IOCTL_GETCHANLIST, &active, sizeof (active)) < 0)
	{
      fprintf(stderr, "unable to get active channel list\n");
	  return NULL;
	}
      memset (&achans, 0, sizeof (achans));
      for (i = 0; i < chans.ic_nchans; i++)
	{
	  c = &chans.ic_chans[i];
	  if (isset (active, ieee80211_mhz2ieee (c->ic_freq)) || allchans)
	    achans.ic_chans[achans.ic_nchans++] = *c;
	}
    }
  else
    achans = chans;


  fprintf(stderr,"channel number %d\n", achans.ic_nchans);
  struct wifi_channels *list =
    (struct wifi_channels *) malloc (sizeof (struct wifi_channels) *
				     (achans.ic_nchans + 1));

  char wl_mode[16];
  char wl_turbo[16];
  sprintf (wl_mode, "%s_net_mode", ifname);
  sprintf (wl_turbo, "%s_turbo", ifname);
  int l = 0;
  for (i = 0; i < achans.ic_nchans; i++)
    {

      //filter out A channels if mode isnt A-Only or mixed
      if (IEEE80211_IS_CHAN_A (&achans.ic_chans[i]))
	{
	  if (nvram_invmatch (wl_mode, "a-only")
	      && nvram_invmatch (wl_mode, "mixed"))
	    continue;
	}
      //filter out B/G channels if mode isnt g-only, b-only or mixed
      if (IEEE80211_IS_CHAN_G (&achans.ic_chans[i]))
	{
	  if (nvram_invmatch (wl_mode, "g-only")
	      && nvram_invmatch (wl_mode, "mixed")
	      && nvram_invmatch (wl_mode, "b-only"))
	    continue;
	}
      //filter out channels which are not supporting turbo mode if turbo is enabled
      if (!IEEE80211_IS_CHAN_STURBO (&achans.ic_chans[i]))
	{
	  if (nvram_match (wl_turbo, "1"))
	    continue;
	}
      //filter out turbo channels if turbo mode is disabled
      if (IEEE80211_IS_CHAN_STURBO (&achans.ic_chans[i])
	  || IEEE80211_IS_CHAN_DTURBO (&achans.ic_chans[i]))
	{
	  if (nvram_match (wl_turbo, "0"))
	    continue;
	}

      list[l].channel = achans.ic_chans[i].ic_ieee;
      list[l].freq = achans.ic_chans[i].ic_freq;
      l++;
    }

  list[l].freq = -1;
  return list;
}




struct wifi_channels *
list_channels (char *devnr)
{
  return list_channelsext (devnr, 1);
/*
  char csign[64];
  char channel[64];
  char ppp[64];
  char freq[64];
  char dum1[64];
  char dum2[64];
  char dum3[64];
  char dum4[64];

  char cmd[64];
  sprintf (cmd, "iwlist %s chan>/tmp/.channels", devnr);
  system (cmd);
  FILE *in = fopen ("/tmp/.channels", "rb");
  if (in == NULL)
    return NULL;
  fscanf (in, "%s %s %s %s %s %s %s %s", csign, channel, ppp, freq, dum1,
	  dum2, dum3, dum4);
  int ch = atoi (channel);
  int i;
  struct wifi_channels *list =
    (struct wifi_channels *) malloc (sizeof (struct wifi_channels) * (ch+1) );
  for (i = 0; i < ch; i++)
    {
      fscanf (in, "%s %s %s %s %s", csign, channel, ppp, freq, dum1);
      if (!strcmp (csign, "Current"))
	break;
      list[i].channel = atoi (channel);
      list[i].freq = strdup (freq);
      channelcount++;
    }
  fclose (in);
  return list;
*/
}


int
getdevicecount (void)
{
/*  eval ("ifconfig", "wifi0", "up");
  eval ("ifconfig", "wifi1", "up");
  eval ("ifconfig", "wifi2", "up");
  eval ("ifconfig", "wifi3", "up");
  eval ("ifconfig", "wifi4", "up");
  eval ("ifconfig", "wifi5", "up");
*/
  system ("/sbin/ifconfig|grep wifi|wc -l>/tmp/.counts");
  FILE *in = fopen ("/tmp/.counts", "rb");
  if (in == NULL)
    return 0;
  int count;
  fscanf (in, "%d", &count);
  fclose (in);
//  unlink ("/tmp/.counts");
  if (count < 7 && count > 0)
    return count;
  return 0;
}





int getassoclist(char *ifname, unsigned char *list)
{
  unsigned char buf[24 * 1024];
  unsigned char *cp;
  int len;
  struct iwreq iwr;
  int s;
  s = socket (AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    {
      fprintf (stderr, "socket(SOCK_DRAGM)\n");
      return -1;
    }
  (void) memset (&iwr, 0, sizeof (iwr));
  (void) strncpy (iwr.ifr_name, ifname, sizeof (iwr.ifr_name));
  iwr.u.data.pointer = (void *) buf;
  iwr.u.data.length = sizeof (buf);
  if (ioctl (s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0)
    {
      close(s);
      return -1;
    }
  len = iwr.u.data.length;
  if (len < sizeof (struct ieee80211req_sta_info))
    return -1;
  int cnt = 0;
  cp = buf;
  unsigned char *l = (unsigned char*)list;
  uint *count = (uint*)list;
  l+=4;
  do
    {
      struct ieee80211req_sta_info *si;
      si = (struct ieee80211req_sta_info *) cp;            
      memcpy(l,&si->isi_macaddr[0],6);
//      printf("%X%X%X%X%X%X\n",l[0],l[1],l[2],l[3],l[4],l[5]);
      l+=6;
      count[0]++;      
      cp += si->isi_len, len -= si->isi_len;
    }
  while (len >= sizeof (struct ieee80211req_sta_info));
close(s);
return count[0];
}



#endif

/* Sveasoft addition - return wirteless interface */
char *
get_wdev (void)
{
#ifdef HAVE_MADWIFI
  return "ath0";
#else
  if (wl_probe ("eth2"))
    return "eth1";
  else
    return "eth2";
#endif
}

/* end Sveasoft addition */

int
wl_probe(char *name)
{
	int ret, val;

#if defined(linux)
	char buf[DEV_TYPE_LEN];
	if ((ret = wl_get_dev_type(name, buf, DEV_TYPE_LEN)) < 0)
		return ret;
	/* Check interface */
	if (strncmp(buf, "wl", 2))
		return -1;
#else
	/* Check interface */
	if ((ret = wl_ioctl(name, WLC_GET_MAGIC, &val, sizeof(val))))
		return ret;
#endif
	if ((ret = wl_ioctl(name, WLC_GET_VERSION, &val, sizeof(val))))
		return ret;
	if (val > WLC_IOCTL_VERSION)
		return -1;

	return ret;
}

/*int
wl_probe (char *name)
{
  int ret, val;

  if ((ret = wl_ioctl (name, WLC_GET_MAGIC, &val, sizeof (val))))
    return ret;
  if (val != WLC_IOCTL_MAGIC)
    return -1;
  if ((ret = wl_ioctl (name, WLC_GET_VERSION, &val, sizeof (val))))
    return ret;
  if (val > WLC_IOCTL_VERSION)
    return -1;

  return ret;
}
*/
//#ifndef HAVE_MSSID
int
wl_set_val (char *name, char *var, void *val, int len)
{
  char buf[128];
  int buf_len;

  /* check for overflow */
  if ((buf_len = strlen (var)) + 1 + len > sizeof (buf))
    return -1;

  strcpy (buf, var);
  buf_len += 1;

  /* append int value onto the end of the name string */
  memcpy (&buf[buf_len], val, len);
  buf_len += len;

  return wl_ioctl (name, WLC_SET_VAR, buf, buf_len);
}

int
wl_get_val (char *name, char *var, void *val, int len)
{
  char buf[128];
  int ret;

  /* check for overflow */
  if (strlen (var) + 1 > sizeof (buf) || len > sizeof (buf))
    return -1;

  strcpy (buf, var);
  if ((ret = wl_ioctl (name, WLC_GET_VAR, buf, sizeof (buf))))
    return ret;

  memcpy (val, buf, len);
  return 0;
}

int
wl_set_int (char *name, char *var, int val)
{
  return wl_set_val (name, var, &val, sizeof (val));
}

int
wl_get_int (char *name, char *var, int *val)
{
  return wl_get_val (name, var, val, sizeof (*val));
}

//#else
int
wl_iovar_getbuf (char *ifname, char *iovar, void *param, int paramlen,
		 void *bufptr, int buflen)
{
  int err;
  uint namelen;
  uint iolen;

  namelen = strlen (iovar) + 1;	/* length of iovar name plus null */
  iolen = namelen + paramlen;

  /* check for overflow */
  if (iolen > buflen)
    return (BCME_BUFTOOSHORT);

  memcpy (bufptr, iovar, namelen);	/* copy iovar name including null */
  memcpy ((int8 *) bufptr + namelen, param, paramlen);

  err = wl_ioctl (ifname, WLC_GET_VAR, bufptr, buflen);

  return (err);
}

int
wl_iovar_setbuf (char *ifname, char *iovar, void *param, int paramlen,
		 void *bufptr, int buflen)
{
  uint namelen;
  uint iolen;

  namelen = strlen (iovar) + 1;	/* length of iovar name plus null */
  iolen = namelen + paramlen;

  /* check for overflow */
  if (iolen > buflen)
    return (BCME_BUFTOOSHORT);

  memcpy (bufptr, iovar, namelen);	/* copy iovar name including null */
  memcpy ((int8 *) bufptr + namelen, param, paramlen);

  return wl_ioctl (ifname, WLC_SET_VAR, bufptr, iolen);
}

int
wl_iovar_set (char *ifname, char *iovar, void *param, int paramlen)
{
  char smbuf[WLC_IOCTL_SMLEN];

  return wl_iovar_setbuf (ifname, iovar, param, paramlen, smbuf,
			  sizeof (smbuf));
}

int
wl_iovar_get (char *ifname, char *iovar, void *bufptr, int buflen)
{
  char smbuf[WLC_IOCTL_SMLEN];
  int ret;

  /* use the return buffer if it is bigger than what we have on the stack */
  if (buflen > sizeof (smbuf))
    {
      ret = wl_iovar_getbuf (ifname, iovar, NULL, 0, bufptr, buflen);
    }
  else
    {
      ret = wl_iovar_getbuf (ifname, iovar, NULL, 0, smbuf, sizeof (smbuf));
      if (ret == 0)
	memcpy (bufptr, smbuf, buflen);
    }

  return ret;
}

/* 
 * set named driver variable to int value
 * calling example: wl_iovar_setint(ifname, "arate", rate) 
*/
int
wl_iovar_setint (char *ifname, char *iovar, int val)
{
  return wl_iovar_set (ifname, iovar, &val, sizeof (val));
}

/* 
 * get named driver variable to int value and return error indication 
 * calling example: wl_iovar_getint(ifname, "arate", &rate) 
 */
int
wl_iovar_getint (char *ifname, char *iovar, int *val)
{
  return wl_iovar_get (ifname, iovar, val, sizeof (int));
}

/* 
 * format a bsscfg indexed iovar buffer
 */
static int
wl_bssiovar_mkbuf (char *iovar, int bssidx, void *param, int paramlen,
		   void *bufptr, int buflen, int *plen)
{
  char *prefix = "bsscfg:";
  int8 *p;
  uint prefixlen;
  uint namelen;
  uint iolen;

  prefixlen = strlen (prefix);	/* length of bsscfg prefix */
  namelen = strlen (iovar) + 1;	/* length of iovar name + null */
  iolen = prefixlen + namelen + sizeof (int) + paramlen;

  /* check for overflow */
  if (buflen < 0 || iolen > (uint) buflen)
    {
      *plen = 0;
      return BCME_BUFTOOSHORT;
    }

  p = (int8 *) bufptr;

  /* copy prefix, no null */
  memcpy (p, prefix, prefixlen);
  p += prefixlen;

  /* copy iovar name including null */
  memcpy (p, iovar, namelen);
  p += namelen;

  /* bss config index as first param */
  memcpy (p, &bssidx, sizeof (int32));
  p += sizeof (int32);

  /* parameter buffer follows */
  if (paramlen)
    memcpy (p, param, paramlen);

  *plen = iolen;
  return 0;
}

/* 
 * set named & bss indexed driver variable to buffer value
 */
int
wl_bssiovar_setbuf (char *ifname, char *iovar, int bssidx, void *param,
		    int paramlen, void *bufptr, int buflen)
{
  int err;
  uint iolen;

  err =
    wl_bssiovar_mkbuf (iovar, bssidx, param, paramlen, bufptr, buflen,
		       &iolen);
  if (err)
    return err;

  return wl_ioctl (ifname, WLC_SET_VAR, bufptr, iolen);
}

/* 
 * get named & bss indexed driver variable buffer value
 */
int
wl_bssiovar_getbuf (char *ifname, char *iovar, int bssidx, void *param,
		    int paramlen, void *bufptr, int buflen)
{
  int err;
  uint iolen;

  err =
    wl_bssiovar_mkbuf (iovar, bssidx, param, paramlen, bufptr, buflen,
		       &iolen);
  if (err)
    return err;

  return wl_ioctl (ifname, WLC_GET_VAR, bufptr, buflen);
}

/* 
 * set named & bss indexed driver variable to buffer value
 */
int
wl_bssiovar_set (char *ifname, char *iovar, int bssidx, void *param,
		 int paramlen)
{
  char smbuf[WLC_IOCTL_SMLEN];

  return wl_bssiovar_setbuf (ifname, iovar, bssidx, param, paramlen, smbuf,
			     sizeof (smbuf));
}

/* 
 * get named & bss indexed driver variable buffer value
 */
int
wl_bssiovar_get (char *ifname, char *iovar, int bssidx, void *outbuf, int len)
{
  char smbuf[WLC_IOCTL_SMLEN];
  int err;

  /* use the return buffer if it is bigger than what we have on the stack */
  if (len > (int) sizeof (smbuf))
    {
      err = wl_bssiovar_getbuf (ifname, iovar, bssidx, NULL, 0, outbuf, len);
    }
  else
    {
      memset (smbuf, 0, sizeof (smbuf));
      err =
	wl_bssiovar_getbuf (ifname, iovar, bssidx, NULL, 0, smbuf,
			    sizeof (smbuf));
      if (err == 0)
	memcpy (outbuf, smbuf, len);
    }

  return err;
}

/* 
 * set named & bss indexed driver variable to int value
 */
int
wl_bssiovar_setint (char *ifname, char *iovar, int bssidx, int val)
{
  return wl_bssiovar_set (ifname, iovar, bssidx, &val, sizeof (int));
}


/*
void 
wl_printlasterror(char *name)
{
	char err_buf[WLC_IOCTL_SMLEN];
	strcpy(err_buf, "bcmerrstr");
	
	fprintf(stderr, "Error: ");
	if ( wl_ioctl(name, WLC_GET_VAR, err_buf, sizeof (err_buf)) != 0)
		fprintf(stderr, "Error getting the Errorstring from driver\n");
	else
		fprintf(stderr, err_buf);
}
*/

//#endif

