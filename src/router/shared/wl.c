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


/* DD-WRT addition  (loaned from radauth) */
int
getassoclist (char *name, unsigned char *list)
{
  int ret, num;

  num = (sizeof (*list) - 4) / 6;	/* Maximum number of entries in the buffer */
  memcpy (list, &num, 4);	/* First 4 bytes are the number of ent. */

  ret = wl_ioctl (name, WLC_GET_ASSOCLIST, list, 8192);

  return (ret);
}


/* Sveasoft addition - return wirteless interface */
char *
get_wdev (void)
{
  if (wl_probe ("eth2"))	// identify wireless interface
    return "eth1";
  else
    return "eth2";
}

/* end Sveasoft addition */

int
wl_probe (char *name)
{
  int ret, val;

  /* Check interface */
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

#ifndef HAVE_MSSID
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
#else
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

#endif
