/*
 *	Wireless Tools
 *
 *		Jean II - HPLB 97->99 - HPL 99->04
 *
 * Main code for "iwconfig". This is the generic tool for most
 * manipulations...
 * You need to link this code against "iwlib.c" and "-lm".
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2004 Jean Tourrilhes <jt@hpl.hp.com>
 */

#include "iwlib.h"		/* Header */

/************************* MISC SUBROUTINES **************************/

/*------------------------------------------------------------------*/
/*
 * Print usage string
 */
static void
iw_usage(void)
{
  fprintf(stderr,
	"Usage: iwconfig interface [essid {NN|on|off}]\n"
	"                          [nwid {NN|on|off}]\n"
	"                          [mode {managed|ad-hoc|...}\n"
	"                          [freq N.NNNN[k|M|G]]\n"
	"                          [channel N]\n"
	"                          [ap {N|off|auto}]\n"
	"                          [sens N]\n"
	"                          [nick N]\n"
	"                          [rate {N|auto|fixed}]\n"
	"                          [rts {N|auto|fixed|off}]\n"
	"                          [frag {N|auto|fixed|off}]\n"
	"                          [enc {NNNN-NNNN|off}]\n"
	"                          [power {period N|timeout N}]\n"
	"                          [retry {limit N|lifetime N}]\n"
	"                          [txpower N {mW|dBm}]\n"
	"                          [commit]\n"
	"       Check man pages for more details.\n\n"
  );
}


/************************* DISPLAY ROUTINES **************************/

/*------------------------------------------------------------------*/
/*
 * Get wireless informations & config from the device driver
 * We will call all the classical wireless ioctl on the driver through
 * the socket to know what is supported and to get the settings...
 */
static int
get_info(int			skfd,
	 char *			ifname,
	 struct wireless_info *	info)
{
  struct iwreq		wrq;

  memset((char *) info, 0, sizeof(struct wireless_info));

  /* Get basic information */
  if(iw_get_basic_config(skfd, ifname, &(info->b)) < 0)
    {
      /* If no wireless name : no wireless extensions */
      /* But let's check if the interface exists at all */
      struct ifreq ifr;

      strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
      if(ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	return(-ENODEV);
      else
	return(-ENOTSUP);
    }

  /* Get ranges */
  if(iw_get_range_info(skfd, ifname, &(info->range)) >= 0)
    info->has_range = 1;

  /* Get sensitivity */
  if(iw_get_ext(skfd, ifname, SIOCGIWSENS, &wrq) >= 0)
    {
      info->has_sens = 1;
      memcpy(&(info->sens), &(wrq.u.sens), sizeof(iwparam));
    }

  /* Get AP address */
  if(iw_get_ext(skfd, ifname, SIOCGIWAP, &wrq) >= 0)
    {
      info->has_ap_addr = 1;
      memcpy(&(info->ap_addr), &(wrq.u.ap_addr), sizeof (sockaddr));
    }

  /* Get NickName */
  wrq.u.essid.pointer = (caddr_t) info->nickname;
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  wrq.u.essid.flags = 0;
  if(iw_get_ext(skfd, ifname, SIOCGIWNICKN, &wrq) >= 0)
    if(wrq.u.data.length > 1)
      info->has_nickname = 1;

  /* Get bit rate */
  if(iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
    {
      info->has_bitrate = 1;
      memcpy(&(info->bitrate), &(wrq.u.bitrate), sizeof(iwparam));
    }

  /* Get RTS threshold */
  if(iw_get_ext(skfd, ifname, SIOCGIWRTS, &wrq) >= 0)
    {
      info->has_rts = 1;
      memcpy(&(info->rts), &(wrq.u.rts), sizeof(iwparam));
    }

  /* Get fragmentation threshold */
  if(iw_get_ext(skfd, ifname, SIOCGIWFRAG, &wrq) >= 0)
    {
      info->has_frag = 1;
      memcpy(&(info->frag), &(wrq.u.frag), sizeof(iwparam));
    }

  /* Get Power Management settings */
  wrq.u.power.flags = 0;
  if(iw_get_ext(skfd, ifname, SIOCGIWPOWER, &wrq) >= 0)
    {
      info->has_power = 1;
      memcpy(&(info->power), &(wrq.u.power), sizeof(iwparam));
    }

  if((info->has_range) && (info->range.we_version_compiled > 9))
    {
      /* Get Transmit Power */
      if(iw_get_ext(skfd, ifname, SIOCGIWTXPOW, &wrq) >= 0)
	{
	  info->has_txpower = 1;
	  memcpy(&(info->txpower), &(wrq.u.txpower), sizeof(iwparam));
	}
    }

  if((info->has_range) && (info->range.we_version_compiled > 10))
    {
      /* Get retry limit/lifetime */
      if(iw_get_ext(skfd, ifname, SIOCGIWRETRY, &wrq) >= 0)
	{
	  info->has_retry = 1;
	  memcpy(&(info->retry), &(wrq.u.retry), sizeof(iwparam));
	}
    }

  /* Get stats */
  if(iw_get_stats(skfd, ifname, &(info->stats),
		  &info->range, info->has_range) >= 0)
    {
      info->has_stats = 1;
    }

#ifdef DISPLAY_WPA
  /* Note : currently disabled to not bloat iwconfig output. Also,
   * if does not make total sense to display parameters that we
   * don't allow (yet) to configure.
   * For now, use iwlist instead... Jean II */

  /* Get WPA/802.1x/802.11i security parameters */
  if((info->has_range) && (info->range.we_version_compiled > 17))
    {
      wrq.u.param.flags = IW_AUTH_KEY_MGMT;
      if(iw_get_ext(skfd, ifname, SIOCGIWAUTH, &wrq) >= 0)
	{
	  info->has_auth_key_mgmt = 1;
	  info->auth_key_mgmt = wrq.u.param.value;
	}

      wrq.u.param.flags = IW_AUTH_CIPHER_PAIRWISE;
      if(iw_get_ext(skfd, ifname, SIOCGIWAUTH, &wrq) >= 0)
	{
	  info->has_auth_cipher_pairwise = 1;
	  info->auth_cipher_pairwise = wrq.u.param.value;
	}

      wrq.u.param.flags = IW_AUTH_CIPHER_GROUP;
      if(iw_get_ext(skfd, ifname, SIOCGIWAUTH, &wrq) >= 0)
	{
	  info->has_auth_cipher_group = 1;
	  info->auth_cipher_group = wrq.u.param.value;
	}
    }
#endif

  return(0);
}

/*------------------------------------------------------------------*/
/*
 * Print on the screen in a neat fashion all the info we have collected
 * on a device.
 */
static void
display_info(struct wireless_info *	info,
	     char *			ifname)
{
  char		buffer[128];	/* Temporary buffer */

  /* One token is more of less 5 characters, 14 tokens per line */
  int	tokens = 3;	/* For name */

  /* Display device name and wireless name (name of the protocol used) */
  printf("%-8.16s  %s  ", ifname, info->b.name);

  /* Display ESSID (extended network), if any */
  if(info->b.has_essid)
    {
      if(info->b.essid_on)
	{
	  /* Does it have an ESSID index ? */
	  if((info->b.essid_on & IW_ENCODE_INDEX) > 1)
	    printf("ESSID:\"%s\" [%d]  ", info->b.essid,
		   (info->b.essid_on & IW_ENCODE_INDEX));
	  else
	    printf("ESSID:\"%s\"  ", info->b.essid);
	}
      else
	printf("ESSID:off/any  ");
    }

  /* Display NickName (station name), if any */
  if(info->has_nickname)
    printf("Nickname:\"%s\"", info->nickname);

  /* Formatting */
  if(info->b.has_essid || info->has_nickname)
    {
      printf("\n          ");
      tokens = 0;
    }

  /* Display Network ID */
  if(info->b.has_nwid)
    {
      /* Note : should display proper number of digits according to info
       * in range structure */
      if(info->b.nwid.disabled)
	printf("NWID:off/any  ");
      else
	printf("NWID:%X  ", info->b.nwid.value);
      tokens +=2;
    }

  /* Display the current mode of operation */
  if(info->b.has_mode)
    {
      printf("Mode:%s  ", iw_operation_mode[info->b.mode]);
      tokens +=3;
    }

  /* Display frequency / channel */
  if(info->b.has_freq)
    {
      double		freq = info->b.freq;	/* Frequency/channel */
      int		channel = -1;		/* Converted to channel */
      /* Some drivers insist of returning channel instead of frequency.
       * This fixes them up. Note that, driver should still return
       * frequency, because other tools depend on it. */
      if(info->has_range && (freq < KILO))
	channel = iw_channel_to_freq((int) freq, &freq, &info->range);
      /* Display */
      iw_print_freq(buffer, sizeof(buffer), freq, -1, info->b.freq_flags);
      printf("%s  ", buffer);
      tokens +=4;
    }

  /* Display the address of the current Access Point */
  if(info->has_ap_addr)
    {
      /* A bit of clever formatting */
      if(tokens > 8)
	{
	  printf("\n          ");
	  tokens = 0;
	}
      tokens +=6;

      /* Oups ! No Access Point in Ad-Hoc mode */
      if((info->b.has_mode) && (info->b.mode == IW_MODE_ADHOC))
	printf("Cell:");
      else
	printf("Access Point:");
      printf(" %s   ", iw_sawap_ntop(&info->ap_addr, buffer));
    }

  /* Display the currently used/set bit-rate */
  if(info->has_bitrate)
    {
      /* A bit of clever formatting */
      if(tokens > 11)
	{
	  printf("\n          ");
	  tokens = 0;
	}
      tokens +=3;

      /* Display it */
      iw_print_bitrate(buffer, sizeof(buffer), info->bitrate.value);
      printf("Bit Rate%c%s   ", (info->bitrate.fixed ? '=' : ':'), buffer);
    }

  /* Display the Transmit Power */
  if(info->has_txpower)
    {
      /* A bit of clever formatting */
      if(tokens > 11)
	{
	  printf("\n          ");
	  tokens = 0;
	}
      tokens +=3;

      /* Display it */
      iw_print_txpower(buffer, sizeof(buffer), &info->txpower);
      printf("Tx-Power%c%s   ", (info->txpower.fixed ? '=' : ':'), buffer);
    }

  /* Display sensitivity */
  if(info->has_sens)
    {
      /* A bit of clever formatting */
      if(tokens > 10)
	{
	  printf("\n          ");
	  tokens = 0;
	}
      tokens +=4;

      /* Fixed ? */
      if(info->sens.fixed)
	printf("Sensitivity=");
      else
	printf("Sensitivity:");

      if(info->has_range)
	/* Display in dBm ? */
	if(info->sens.value < 0)
	  printf("%d dBm  ", info->sens.value);
	else
	  printf("%d/%d  ", info->sens.value, info->range.sensitivity);
      else
	printf("%d  ", info->sens.value);
    }

  printf("\n          ");
  tokens = 0;

  /* Display retry limit/lifetime information */
  if(info->has_retry)
    { 
      printf("Retry");
      /* Disabled ? */
      if(info->retry.disabled)
	printf(":off");
      else
	{
	  /* Let's check the value and its type */
	  if(info->retry.flags & IW_RETRY_TYPE)
	    {
	      iw_print_retry_value(buffer, sizeof(buffer),
				   info->retry.value, info->retry.flags);
	      printf("%s", buffer);
	    }

	  /* Let's check if nothing (simply on) */
	  if(info->retry.flags == IW_RETRY_ON)
	    printf(":on");
 	}
      printf("   ");
      tokens += 5;	/* Between 3 and 5, depend on flags */
    }

  /* Display the RTS threshold */
  if(info->has_rts)
    {
      /* Disabled ? */
      if(info->rts.disabled)
	printf("RTS thr:off   ");
      else
	{
	  /* Fixed ? */
	  if(info->rts.fixed)
	    printf("RTS thr=");
	  else
	    printf("RTS thr:");

	  printf("%d B   ", info->rts.value);
	}
      tokens += 3;
    }

  /* Display the fragmentation threshold */
  if(info->has_frag)
    {
      /* A bit of clever formatting */
      if(tokens > 10)
	{
	  printf("\n          ");
	  tokens = 0;
	}
      tokens +=4;

      /* Disabled ? */
      if(info->frag.disabled)
	printf("Fragment thr:off");
      else
	{
	  /* Fixed ? */
	  if(info->frag.fixed)
	    printf("Fragment thr=");
	  else
	    printf("Fragment thr:");

	  printf("%d B   ", info->frag.value);
	}
    }

  /* Formating */
  if(tokens > 0)
    printf("\n          ");

  /* Display encryption information */
  /* Note : we display only the "current" key, use iwlist to list all keys */
  if(info->b.has_key)
    {
      printf("Encryption key:");
      if((info->b.key_flags & IW_ENCODE_DISABLED) || (info->b.key_size == 0))
	printf("off");
      else
	{
	  /* Display the key */
	  iw_print_key(buffer, sizeof(buffer),
		       info->b.key, info->b.key_size, info->b.key_flags);
	  printf("%s", buffer);

	  /* Other info... */
	  if((info->b.key_flags & IW_ENCODE_INDEX) > 1)
	    printf(" [%d]", info->b.key_flags & IW_ENCODE_INDEX);
	  if(info->b.key_flags & IW_ENCODE_RESTRICTED)
	    printf("   Security mode:restricted");
	  if(info->b.key_flags & IW_ENCODE_OPEN)
	    printf("   Security mode:open");
 	}
      printf("\n          ");
    }

#ifdef DISPLAY_WPA
  /* Display WPA/802.1x/802.11i security parameters */
  if(info->has_auth_key_mgmt || info->has_auth_cipher_pairwise ||
     info->has_auth_cipher_group)
    {
      printf("Auth params:");
      if(info->has_auth_key_mgmt)
	printf(" key_mgmt:0x%X ", info->auth_key_mgmt);
      if(info->has_auth_cipher_pairwise)
	printf(" cipher_pairwise:0x%X ", info->auth_cipher_pairwise);
      if(info->has_auth_cipher_group)
	printf(" cipher_group:0x%X ", info->auth_cipher_group);
      printf("\n          ");
    }
#endif

  /* Display Power Management information */
  /* Note : we display only one parameter, period or timeout. If a device
   * (such as HiperLan) has both, the user need to use iwlist... */
  if(info->has_power)	/* I hope the device has power ;-) */
    { 
      printf("Power Management");
      /* Disabled ? */
      if(info->power.disabled)
	printf(":off");
      else
	{
	  /* Let's check the value and its type */
	  if(info->power.flags & IW_POWER_TYPE)
	    {
	      iw_print_pm_value(buffer, sizeof(buffer),
				info->power.value, info->power.flags);
	      printf("%s  ", buffer);
	    }

	  /* Let's check the mode */
	  iw_print_pm_mode(buffer, sizeof(buffer), info->power.flags);
	  printf("%s", buffer);

	  /* Let's check if nothing (simply on) */
	  if(info->power.flags == IW_POWER_ON)
	    printf(":on");
 	}
      printf("\n          ");
    }

  /* Display statistics */
  if(info->has_stats)
    {
      iw_print_stats(buffer, sizeof(buffer),
		     &info->stats.qual, &info->range, info->has_range);
      printf("Link %s\n", buffer);

      if(info->range.we_version_compiled > 11)
	printf("          Rx invalid nwid:%d  Rx invalid crypt:%d  Rx invalid frag:%d\n          Tx excessive retries:%d  Invalid misc:%d   Missed beacon:%d\n",
	       info->stats.discard.nwid,
	       info->stats.discard.code,
	       info->stats.discard.fragment,
	       info->stats.discard.retries,
	       info->stats.discard.misc,
	       info->stats.miss.beacon);
      else
	printf("          Rx invalid nwid:%d  invalid crypt:%d  invalid misc:%d\n",
	       info->stats.discard.nwid,
	       info->stats.discard.code,
	       info->stats.discard.misc);
    }

  printf("\n");
}

/*------------------------------------------------------------------*/
/*
 * Print on the screen in a neat fashion all the info we have collected
 * on a device.
 */
static int
print_info(int		skfd,
	   char *	ifname,
	   char *	args[],
	   int		count)
{
  struct wireless_info	info;
  int			rc;

  /* Avoid "Unused parameter" warning */
  args = args; count = count;

  rc = get_info(skfd, ifname, &info);
  switch(rc)
    {
    case 0:	/* Success */
      /* Display it ! */
      display_info(&info, ifname);
      break;

    case -ENOTSUP:
      fprintf(stderr, "%-8.16s  no wireless extensions.\n\n",
	      ifname);
      break;

    default:
      fprintf(stderr, "%-8.16s  %s\n\n", ifname, strerror(-rc));
    }
  return(rc);
}

/************************* SETTING ROUTINES **************************/

/*------------------------------------------------------------------*/
/*
 * Macro to handle errors when setting WE
 * Print a nice error message and exit...
 * We define them as macro so that "return" do the right thing.
 * The "do {...} while(0)" is a standard trick
 */
#define ERR_SET_EXT(rname, request) \
	fprintf(stderr, "Error for wireless request \"%s\" (%X) :\n", \
		rname, request)

#define ABORT_ARG_NUM(rname, request) \
	do { \
		ERR_SET_EXT(rname, request); \
		fprintf(stderr, "    too few arguments.\n"); \
		return(-1); \
	} while(0)

#define ABORT_ARG_TYPE(rname, request, arg) \
	do { \
		ERR_SET_EXT(rname, request); \
		fprintf(stderr, "    invalid argument \"%s\".\n", arg); \
		return(-2); \
	} while(0)

#define ABORT_ARG_SIZE(rname, request, max) \
	do { \
		ERR_SET_EXT(rname, request); \
		fprintf(stderr, "    argument too big (max %d)\n", max); \
		return(-3); \
	} while(0)

/*------------------------------------------------------------------*/
/*
 * Wrapper to push some Wireless Parameter in the driver
 * Use standard wrapper and add pretty error message if fail...
 */
#define IW_SET_EXT_ERR(skfd, ifname, request, wrq, rname) \
	do { \
	if(iw_set_ext(skfd, ifname, request, wrq) < 0) { \
		ERR_SET_EXT(rname, request); \
		fprintf(stderr, "    SET failed on device %-1.16s ; %s.\n", \
			ifname, strerror(errno)); \
		return(-5); \
	} } while(0)

/*------------------------------------------------------------------*/
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 * Use standard wrapper and add pretty error message if fail...
 */
#define IW_GET_EXT_ERR(skfd, ifname, request, wrq, rname) \
	do { \
	if(iw_get_ext(skfd, ifname, request, wrq) < 0) { \
		ERR_SET_EXT(rname, request); \
		fprintf(stderr, "    GET failed on device %-1.16s ; %s.\n", \
			ifname, strerror(errno)); \
		return(-6); \
	} } while(0)

/*------------------------------------------------------------------*/
/*
 * Set the wireless options requested on command line
 * This function is too long and probably should be split,
 * because it look like the perfect definition of spaghetti code,
 * but I'm way to lazy
 */
static int
set_info(int		skfd,		/* The socket */
	 char *		args[],		/* Command line args */
	 int		count,		/* Args count */
	 char *		ifname)		/* Dev name */
{
  struct iwreq		wrq;
  int			i;

  /* if nothing after the device name - will never happen */
  if(count < 1)
    {
      fprintf(stderr, "Error : too few arguments.\n");
      return(-1);
    }

  /* The other args on the line specify options to be set... */
  for(i = 0; i < count; i++)
    {
      /* ---------- Commit changes to driver ---------- */
      if(!strncmp(args[i], "commit", 6))
	{
	  /* No args */
	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWCOMMIT, &wrq,
			 "Commit changes");
	  continue;
	}

      /* ---------- Set network ID ---------- */
      if((!strcasecmp(args[i], "nwid")) ||
	 (!strcasecmp(args[i], "domain")))
	{
	  i++;
	  if(i >= count)
	    ABORT_ARG_NUM("Set NWID", SIOCSIWNWID);
	  if((!strcasecmp(args[i], "off")) ||
	     (!strcasecmp(args[i], "any")))
	    wrq.u.nwid.disabled = 1;
	  else
	    if(!strcasecmp(args[i], "on"))
	      {
		/* Get old nwid */
		IW_GET_EXT_ERR(skfd, ifname, SIOCGIWNWID, &wrq,
			       "Set NWID");
		wrq.u.nwid.disabled = 0;
	      }
	    else
	      if(sscanf(args[i], "%lX", (unsigned long *) &(wrq.u.nwid.value))
		 != 1)
		ABORT_ARG_TYPE("Set NWID", SIOCSIWNWID, args[i]);
	      else
		wrq.u.nwid.disabled = 0;
	  wrq.u.nwid.fixed = 1;

	  /* Set new nwid */
	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWNWID, &wrq,
			 "Set NWID");
	  continue;
	}

      /* ---------- Set frequency / channel ---------- */
      if((!strncmp(args[i], "freq", 4)) ||
	 (!strcmp(args[i], "channel")))
	{
	  double		freq;

	  if(++i >= count)
	    ABORT_ARG_NUM("Set Frequency", SIOCSIWFREQ);
	  if(!strcasecmp(args[i], "auto"))
	    {
	      wrq.u.freq.m = -1;
	      wrq.u.freq.e = 0;
	      wrq.u.freq.flags = 0;
	    }
	  else
	    {
	      if(!strcasecmp(args[i], "fixed"))
		{
		  /* Get old bitrate */
		  IW_GET_EXT_ERR(skfd, ifname, SIOCGIWFREQ, &wrq,
				 "Set Bit Rate");
		  wrq.u.freq.flags = IW_FREQ_FIXED;
		}
	      else			/* Should be a numeric value */
		{
		  if(sscanf(args[i], "%lg", &(freq)) != 1)
		    ABORT_ARG_TYPE("Set Frequency", SIOCSIWFREQ, args[i]);
		  if(index(args[i], 'G')) freq *= GIGA;
		  if(index(args[i], 'M')) freq *= MEGA;
		  if(index(args[i], 'k')) freq *= KILO;

		  iw_float2freq(freq, &(wrq.u.freq));

		  wrq.u.freq.flags = IW_FREQ_FIXED;

		  /* Check for an additional argument */
		  if(((i+1) < count) &&
		     (!strcasecmp(args[i+1], "auto")))
		    {
		      wrq.u.freq.flags = 0;
		      ++i;
		    }
		  if(((i+1) < count) &&
		     (!strcasecmp(args[i+1], "fixed")))
		    {
		      wrq.u.freq.flags = IW_FREQ_FIXED;
		      ++i;
		    }
		}
	    }

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWFREQ, &wrq,
			 "Set Frequency");
	  continue;
	}

      /* ---------- Set sensitivity ---------- */
      if(!strncmp(args[i], "sens", 4))
	{
	  if(++i >= count)
	    ABORT_ARG_NUM("Set Sensitivity", SIOCSIWSENS);
	  if(sscanf(args[i], "%i", &(wrq.u.sens.value)) != 1)
	    ABORT_ARG_TYPE("Set Sensitivity", SIOCSIWSENS, args[i]);

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWSENS, &wrq,
			 "Set Sensitivity");
	  continue;
	}

      /* ---------- Set encryption stuff ---------- */
      if((!strncmp(args[i], "enc", 3)) ||
	 (!strcmp(args[i], "key")))
	{
	  unsigned char	key[IW_ENCODING_TOKEN_MAX];

	  if(++i >= count)
	    ABORT_ARG_NUM("Set Encode", SIOCSIWENCODE);

	  if(!strcasecmp(args[i], "on"))
	    {
	      /* Get old encryption information */
	      wrq.u.data.pointer = (caddr_t) key;
	      wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
	      wrq.u.data.flags = 0;
	      IW_GET_EXT_ERR(skfd, ifname, SIOCGIWENCODE, &wrq,
			     "Set Encode");
	      wrq.u.data.flags &= ~IW_ENCODE_DISABLED;	/* Enable */
	    }
	  else
	    {
	      int	gotone = 0;
	      int	oldone;
	      int	keylen;
	      int	temp;

	      wrq.u.data.pointer = (caddr_t) NULL;
	      wrq.u.data.flags = 0;
	      wrq.u.data.length = 0;

	      /* Allow arguments in any order (it's safe) */
	      do
		{
		  oldone = gotone;

		  /* -- Check for the key -- */
		  if(i < count)
		    {
		      keylen = iw_in_key_full(skfd, ifname,
					      args[i], key, &wrq.u.data.flags);
		      if(keylen > 0)
			{
			  wrq.u.data.length = keylen;
			  wrq.u.data.pointer = (caddr_t) key;
			  ++i;
			  gotone++;
			}
		    }

		  /* -- Check for token index -- */
		  if((i < count) &&
		     (sscanf(args[i], "[%i]", &temp) == 1) &&
		     (temp > 0) && (temp < IW_ENCODE_INDEX))
		    {
		      wrq.u.encoding.flags |= temp;
		      ++i;
		      gotone++;
		    }

		  /* -- Check the various flags -- */
		  if((i < count) && (!strcasecmp(args[i], "off")))
		    {
		      wrq.u.data.flags |= IW_ENCODE_DISABLED;
		      ++i;
		      gotone++;
		    }
		  if((i < count) && (!strcasecmp(args[i], "open")))
		    {
		      wrq.u.data.flags |= IW_ENCODE_OPEN;
		      ++i;
		      gotone++;
		    }
		  if((i < count) && (!strncasecmp(args[i], "restricted", 5)))
		    {
		      wrq.u.data.flags |= IW_ENCODE_RESTRICTED;
		      ++i;
		      gotone++;
		    }
		  if((i < count) && (!strncasecmp(args[i], "temporary", 4)))
		    {
		      wrq.u.data.flags |= IW_ENCODE_TEMP;
		      ++i;
		      gotone++;
		    }
		}
	      while(gotone != oldone);

	      /* Pointer is absent in new API */
	      if(wrq.u.data.pointer == NULL)
		wrq.u.data.flags |= IW_ENCODE_NOKEY;

	      /* Check if we have any invalid argument */
	      if(!gotone)
		ABORT_ARG_TYPE("Set Encode", SIOCSIWENCODE, args[i]);
	      /* Get back to last processed argument */
	      --i;
	    }

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWENCODE, &wrq,
			 "Set Encode");
	  continue;
  	}

      /* ---------- Set ESSID ---------- */
      if(!strcasecmp(args[i], "essid"))
	{
	  char		essid[IW_ESSID_MAX_SIZE + 1];
	  int		we_kernel_version;

	  i++;
	  if(i >= count)
	    ABORT_ARG_NUM("Set ESSID", SIOCSIWESSID);
	  if((!strcasecmp(args[i], "off")) ||
	     (!strcasecmp(args[i], "any")))
	    {
	      wrq.u.essid.flags = 0;
	      essid[0] = '\0';
	    }
	  else
	    if(!strcasecmp(args[i], "on"))
	      {
		/* Get old essid */
		memset(essid, '\0', sizeof(essid));
		wrq.u.essid.pointer = (caddr_t) essid;
		wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
		wrq.u.essid.flags = 0;
		IW_GET_EXT_ERR(skfd, ifname, SIOCGIWESSID, &wrq,
			       "Set ESSID");
		wrq.u.essid.flags = 1;
	      }
	    else
	      {
		/* '-' or '--' allow to escape the ESSID string, allowing
		 * to set it to the string "any" or "off".
		 * This is a big ugly, but it will do for now */
		if((!strcmp(args[i], "-")) || (!strcmp(args[i], "--")))
		  {
		    i++;
		    if(i >= count)
		      ABORT_ARG_NUM("Set ESSID", SIOCSIWESSID);
		  }

		/* Check the size of what the user passed us to avoid
		 * buffer overflows */
		if(strlen(args[i]) > IW_ESSID_MAX_SIZE)
		  ABORT_ARG_SIZE("Set ESSID", SIOCSIWESSID, IW_ESSID_MAX_SIZE);
		else
		  {
		    int		temp;

		    wrq.u.essid.flags = 1;
		    strcpy(essid, args[i]);	/* Size checked, all clear */

		    /* Check for ESSID index */
		    if(((i+1) < count) &&
		       (sscanf(args[i+1], "[%i]", &temp) == 1) &&
		       (temp > 0) && (temp < IW_ENCODE_INDEX))
		      {
			wrq.u.essid.flags = temp;
			++i;
		      }
		  }
	      }

	  /* Get version from kernel, device may not have range... */
	  we_kernel_version = iw_get_kernel_we_version();

	  /* Finally set the ESSID value */
	  wrq.u.essid.pointer = (caddr_t) essid;
	  wrq.u.essid.length = strlen(essid) + 1;
	  if(we_kernel_version > 20)
	    wrq.u.essid.length--;
	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWESSID, &wrq,
			 "Set ESSID");
	  continue;
	}

      /* ---------- Set AP address ---------- */
      if(!strcasecmp(args[i], "ap"))
	{
	  if(++i >= count)
	    ABORT_ARG_NUM("Set AP Address", SIOCSIWAP);

	  if((!strcasecmp(args[i], "auto")) ||
	     (!strcasecmp(args[i], "any")))
	    {
	      /* Send a broadcast address */
	      iw_broad_ether(&(wrq.u.ap_addr));
	    }
	  else
	    {
	      if(!strcasecmp(args[i], "off"))
		{
		  /* Send a NULL address */
		  iw_null_ether(&(wrq.u.ap_addr));
		}
	      else
		{
		  /* Get the address and check if the interface supports it */
		  if(iw_in_addr(skfd, ifname, args[i++], &(wrq.u.ap_addr)) < 0)
		    ABORT_ARG_TYPE("Set AP Address", SIOCSIWAP, args[i-1]);
		}
	    }

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWAP, &wrq,
			 "Set AP Address");
	  continue;
	}

      /* ---------- Set NickName ---------- */
      if(!strncmp(args[i], "nick", 4))
	{
	  int		we_kernel_version;

	  i++;
	  if(i >= count)
	    ABORT_ARG_NUM("Set Nickname", SIOCSIWNICKN);
	  if(strlen(args[i]) > IW_ESSID_MAX_SIZE)
	    ABORT_ARG_SIZE("Set Nickname", SIOCSIWNICKN, IW_ESSID_MAX_SIZE);

	  we_kernel_version = iw_get_kernel_we_version();

	  wrq.u.essid.pointer = (caddr_t) args[i];
	  wrq.u.essid.length = strlen(args[i]) + 1;
	  if(we_kernel_version > 20)
	    wrq.u.essid.length--;
	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWNICKN, &wrq,
			 "Set Nickname");
	  continue;
	}

      /* ---------- Set Bit-Rate ---------- */
      if((!strncmp(args[i], "bit", 3)) ||
	 (!strcmp(args[i], "rate")))
	{
	  if(++i >= count)
	    ABORT_ARG_NUM("Set Bit Rate", SIOCSIWRATE);
	  if(!strcasecmp(args[i], "auto"))
	    {
	      wrq.u.bitrate.value = -1;
	      wrq.u.bitrate.fixed = 0;
	    }
	  else
	    {
	      if(!strcasecmp(args[i], "fixed"))
		{
		  /* Get old bitrate */
		  IW_GET_EXT_ERR(skfd, ifname, SIOCGIWRATE, &wrq,
				 "Set Bit Rate");
		  wrq.u.bitrate.fixed = 1;
		}
	      else			/* Should be a numeric value */
		{
		  double		brate;

		  if(sscanf(args[i], "%lg", &(brate)) != 1)
		    ABORT_ARG_TYPE("Set Bit Rate", SIOCSIWRATE, args[i]);
		  if(index(args[i], 'G')) brate *= GIGA;
		  if(index(args[i], 'M')) brate *= MEGA;
		  if(index(args[i], 'k')) brate *= KILO;
		  wrq.u.bitrate.value = (long) brate;
		  wrq.u.bitrate.fixed = 1;

		  /* Check for an additional argument */
		  if(((i+1) < count) &&
		     (!strcasecmp(args[i+1], "auto")))
		    {
		      wrq.u.bitrate.fixed = 0;
		      ++i;
		    }
		  if(((i+1) < count) &&
		     (!strcasecmp(args[i+1], "fixed")))
		    {
		      wrq.u.bitrate.fixed = 1;
		      ++i;
		    }
		}
	    }

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWRATE, &wrq,
			 "Set Bit Rate");
	  continue;
	}

      /* ---------- Set RTS threshold ---------- */
      if(!strncasecmp(args[i], "rts", 3))
	{
	  i++;
	  if(i >= count)
	    ABORT_ARG_NUM("Set RTS Threshold", SIOCSIWRTS);
	  wrq.u.rts.value = -1;
	  wrq.u.rts.fixed = 1;
	  wrq.u.rts.disabled = 0;
	  if(!strcasecmp(args[i], "off"))
	    wrq.u.rts.disabled = 1;	/* i.e. max size */
	  else
	    if(!strcasecmp(args[i], "auto"))
	      wrq.u.rts.fixed = 0;
	    else
	      {
		if(!strcasecmp(args[i], "fixed"))
		  {
		    /* Get old RTS threshold */
		    IW_GET_EXT_ERR(skfd, ifname, SIOCGIWRTS, &wrq,
				   "Set RTS Threshold");
		    wrq.u.rts.fixed = 1;
		  }
		else			/* Should be a numeric value */
		  if(sscanf(args[i], "%li", (unsigned long *) &(wrq.u.rts.value))
		     != 1)
		    ABORT_ARG_TYPE("Set RTS Threshold", SIOCSIWRTS, args[i]);
	    }

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWRTS, &wrq,
			 "Set RTS Threshold");
	  continue;
	}

      /* ---------- Set fragmentation threshold ---------- */
      if(!strncmp(args[i], "frag", 4))
	{
	  i++;
	  if(i >= count)
	    ABORT_ARG_NUM("Set Fragmentation Threshold", SIOCSIWFRAG);
	  wrq.u.frag.value = -1;
	  wrq.u.frag.fixed = 1;
	  wrq.u.frag.disabled = 0;
	  if(!strcasecmp(args[i], "off"))
	    wrq.u.frag.disabled = 1;	/* i.e. max size */
	  else
	    if(!strcasecmp(args[i], "auto"))
	      wrq.u.frag.fixed = 0;
	    else
	      {
		if(!strcasecmp(args[i], "fixed"))
		  {
		    /* Get old fragmentation threshold */
		    IW_GET_EXT_ERR(skfd, ifname, SIOCGIWFRAG, &wrq,
				   "Set Fragmentation Threshold");
		    wrq.u.frag.fixed = 1;
		  }
		else			/* Should be a numeric value */
		  if(sscanf(args[i], "%li",
			    (unsigned long *) &(wrq.u.frag.value))
		     != 1)
		    ABORT_ARG_TYPE("Set Fragmentation Threshold", SIOCSIWFRAG,
				   args[i]);
	    }

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWFRAG, &wrq,
			 "Set Fragmentation Threshold");
	  continue;
	}

      /* ---------- Set operation mode ---------- */
      if(!strcmp(args[i], "mode"))
	{
	  int	k;

	  i++;
	  if(i >= count)
	    ABORT_ARG_NUM("Set Mode", SIOCSIWMODE);

	  if(sscanf(args[i], "%i", &k) != 1)
	    {
	      k = 0;
	      while((k < IW_NUM_OPER_MODE) &&
		    strncasecmp(args[i], iw_operation_mode[k], 3))
		k++;
	    }
	  if((k >= IW_NUM_OPER_MODE) || (k < 0))
	    ABORT_ARG_TYPE("Set Mode", SIOCSIWMODE, args[i]);

	  wrq.u.mode = k;
	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWMODE, &wrq,
			 "Set Mode");
	  continue;
	}

      /* ---------- Set Power Management ---------- */
      if(!strncmp(args[i], "power", 3))
	{
	  if(++i >= count)
	    ABORT_ARG_NUM("Set Power Management", SIOCSIWPOWER);

	  if(!strcasecmp(args[i], "off"))
	    wrq.u.power.disabled = 1;	/* i.e. max size */
	  else
	    if(!strcasecmp(args[i], "on"))
	      {
		/* Get old Power info */
		wrq.u.power.flags = 0;
		IW_GET_EXT_ERR(skfd, ifname, SIOCGIWPOWER, &wrq,
			       "Set Power Management");
		wrq.u.power.disabled = 0;
	      }
	    else
	      {
		double		temp;
		int		gotone = 0;
		/* Default - nope */
		wrq.u.power.flags = IW_POWER_ON;
		wrq.u.power.disabled = 0;

		/* Check value modifier */
		if(!strcasecmp(args[i], "min"))
		  {
		    wrq.u.power.flags |= IW_POWER_MIN;
		    if(++i >= count)
		      ABORT_ARG_NUM("Set Power Management", SIOCSIWPOWER);
		  }
		else
		  if(!strcasecmp(args[i], "max"))
		    {
		      wrq.u.power.flags |= IW_POWER_MAX;
		      if(++i >= count)
			ABORT_ARG_NUM("Set Power Management", SIOCSIWPOWER);
		    }

		/* Check value type */
		if(!strcasecmp(args[i], "period"))
		  {
		    wrq.u.power.flags |= IW_POWER_PERIOD;
		    if(++i >= count)
		      ABORT_ARG_NUM("Set Power Management", SIOCSIWPOWER);
		  }
		else
		  if(!strcasecmp(args[i], "timeout"))
		    {
		      wrq.u.power.flags |= IW_POWER_TIMEOUT;
		      if(++i >= count)
			ABORT_ARG_NUM("Set Power Management", SIOCSIWPOWER);
		    }

		/* Is there any value to grab ? */
		if(sscanf(args[i], "%lg", &(temp)) == 1)
		  {
		    temp *= MEGA;	/* default = s */
		    if(index(args[i], 'u')) temp /= MEGA;
		    if(index(args[i], 'm')) temp /= KILO;
		    wrq.u.power.value = (long) temp;
		    if((wrq.u.power.flags & IW_POWER_TYPE) == 0)
		      wrq.u.power.flags |= IW_POWER_PERIOD;
		    ++i;
		    gotone = 1;
		  }

		/* Now, check the mode */
		if(i < count)
		  {
		    if(!strcasecmp(args[i], "all"))
		      wrq.u.power.flags |= IW_POWER_ALL_R;
		    if(!strncasecmp(args[i], "unicast", 4))
		      wrq.u.power.flags |= IW_POWER_UNICAST_R;
		    if(!strncasecmp(args[i], "multicast", 5))
		      wrq.u.power.flags |= IW_POWER_MULTICAST_R;
		    if(!strncasecmp(args[i], "force", 5))
		      wrq.u.power.flags |= IW_POWER_FORCE_S;
		    if(!strcasecmp(args[i], "repeat"))
		      wrq.u.power.flags |= IW_POWER_REPEATER;
		    if(wrq.u.power.flags & IW_POWER_MODE)
		      {
			++i;
			gotone = 1;
		      }
		  }
		if(!gotone)
		  ABORT_ARG_TYPE("Set Power Management", SIOCSIWPOWER,
				 args[i]);
		--i;
	      }

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWPOWER, &wrq,
		       "Set Power Management");
	  continue;
  	}

      /* ---------- Set Transmit-Power ---------- */
      if(!strncmp(args[i], "txpower", 3))
	{
	  struct iw_range	range;

	  if(++i >= count)
	    ABORT_ARG_NUM("Set Tx Power", SIOCSIWTXPOW);

	  /* Extract range info */
	  if(iw_get_range_info(skfd, ifname, &range) < 0)
	    memset(&range, 0, sizeof(range));

	  /* Prepare the request */
	  wrq.u.txpower.value = -1;
	  wrq.u.txpower.fixed = 1;
	  wrq.u.txpower.disabled = 0;
	  wrq.u.txpower.flags = IW_TXPOW_DBM;
	  if(!strcasecmp(args[i], "off"))
	    wrq.u.txpower.disabled = 1;	/* i.e. turn radio off */
	  else
	    if(!strcasecmp(args[i], "auto"))
	      wrq.u.txpower.fixed = 0;	/* i.e. use power control */
	    else
	      {
		if(!strcasecmp(args[i], "on"))
		  {
		    /* Get old tx-power */
		    IW_GET_EXT_ERR(skfd, ifname, SIOCGIWTXPOW, &wrq,
				   "Set Tx Power");
		    wrq.u.txpower.disabled = 0;
		  }
		else
		  {
		    if(!strcasecmp(args[i], "fixed"))
		      {
			/* Get old tx-power */
			IW_GET_EXT_ERR(skfd, ifname, SIOCGIWTXPOW, &wrq,
				       "Set Tx Power");
			wrq.u.txpower.fixed = 1;
			wrq.u.txpower.disabled = 0;
		      }
		    else			/* Should be a numeric value */
		      {
			int		power;
			int		ismwatt = 0;

			/* Get the value */
			if(sscanf(args[i], "%i", &(power)) != 1)
			  ABORT_ARG_TYPE("Set Tx Power", SIOCSIWTXPOW,
					 args[i]);

			/* Check if milliWatt
			 * We authorise a single 'm' as a shorthand for 'mW',
			 * on the other hand a 'd' probably means 'dBm'... */
			ismwatt = ((index(args[i], 'm') != NULL)
				   && (index(args[i], 'd') == NULL));

			/* We could check 'W' alone... Another time... */

			/* Convert */
			if(range.txpower_capa & IW_TXPOW_RELATIVE)
			  {
			    /* Can't convert */
			    if(ismwatt)
			      ABORT_ARG_TYPE("Set Tx Power",
					     SIOCSIWTXPOW,
					     args[i]);
			  }
			else
			  if(range.txpower_capa & IW_TXPOW_MWATT)
			    {
			      if(!ismwatt)
				power = iw_dbm2mwatt(power);
			      wrq.u.txpower.flags = IW_TXPOW_MWATT;
			    }
			  else
			    {
			      if(ismwatt)
				power = iw_mwatt2dbm(power);
			      wrq.u.txpower.flags = IW_TXPOW_DBM;
			    }
			wrq.u.txpower.value = power;

			/* Check for an additional argument */
			if(((i+1) < count) &&
			   (!strcasecmp(args[i+1], "auto")))
			  {
			    wrq.u.txpower.fixed = 0;
			    ++i;
			  }
			if(((i+1) < count) &&
			   (!strcasecmp(args[i+1], "fixed")))
			  {
			    wrq.u.txpower.fixed = 1;
			    ++i;
			  }
		      }
		  }
	      }

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWTXPOW, &wrq,
			 "Set Tx Power");
	  continue;
	}

      /* ---------- Set Retry limit ---------- */
      if(!strncmp(args[i], "retry", 3))
	{
	  double		temp;
	  int		gotone = 0;

	  if(++i >= count)
	    ABORT_ARG_NUM("Set Retry Limit", SIOCSIWRETRY);

	  /* Default - nope */
	  wrq.u.retry.flags = IW_RETRY_LIMIT;
	  wrq.u.retry.disabled = 0;

	  /* Check value modifier */
	  if(!strcasecmp(args[i], "min"))
	    {
	      wrq.u.retry.flags |= IW_RETRY_MIN;
	      if(++i >= count)
		ABORT_ARG_NUM("Set Retry Limit", SIOCSIWRETRY);
	    }
	  else
	    if(!strcasecmp(args[i], "max"))
	      {
		wrq.u.retry.flags |= IW_RETRY_MAX;
		if(++i >= count)
		  ABORT_ARG_NUM("Set Retry Limit", SIOCSIWRETRY);
	      }

	  /* Check value type */
	  if(!strcasecmp(args[i], "limit"))
	    {
	      wrq.u.retry.flags |= IW_RETRY_LIMIT;
	      if(++i >= count)
		ABORT_ARG_NUM("Set Retry Limit", SIOCSIWRETRY);
	    }
	  else
	    if(!strncasecmp(args[i], "lifetime", 4))
	      {
		wrq.u.retry.flags &= ~IW_RETRY_LIMIT;
		wrq.u.retry.flags |= IW_RETRY_LIFETIME;
		if(++i >= count)
		  ABORT_ARG_NUM("Set Retry Limit", SIOCSIWRETRY);
	      }

	  /* Is there any value to grab ? */
	  if(sscanf(args[i], "%lg", &(temp)) == 1)
	    {
	      /* Limit is absolute, on the other hand lifetime is seconds */
	      if(!(wrq.u.retry.flags & IW_RETRY_LIMIT))
		{
		  /* Normalise lifetime */
		  temp *= MEGA;	/* default = s */
		  if(index(args[i], 'u')) temp /= MEGA;
		  if(index(args[i], 'm')) temp /= KILO;
		}
	      wrq.u.retry.value = (long) temp;
	      ++i;
	      gotone = 1;
	    }

	  if(!gotone)
	    ABORT_ARG_TYPE("Set Retry Limit", SIOCSIWRETRY, args[i]);
	  --i;

	  IW_SET_EXT_ERR(skfd, ifname, SIOCSIWRETRY, &wrq,
			 "Set Retry Limit");
	  continue;
	}

      /* ---------- Other ---------- */
      /* Here we have an unrecognised arg... */
      fprintf(stderr, "Error : unrecognised wireless request \"%s\"\n",
	      args[i]);
      return(-1);
    }		/* for(index ... */
  return(0);
}

/******************************* MAIN ********************************/

/*------------------------------------------------------------------*/
/*
 * The main !
 */
int
main(int	argc,
     char **	argv)
{
  int skfd;		/* generic raw socket desc.	*/
  int goterr = 0;

  /* Create a channel to the NET kernel. */
  if((skfd = iw_sockets_open()) < 0)
    {
      perror("socket");
      exit(-1);
    }

  /* No argument : show the list of all device + info */
  if(argc == 1)
    iw_enum_devices(skfd, &print_info, NULL, 0);
  else
    /* Special case for help... */
    if((!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help")))
      iw_usage();
    else
      /* Special case for version... */
      if(!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))
	goterr = iw_print_version_info("iwconfig");
      else
	{
	  /* '--' escape device name */
	  if((argc > 2) && !strcmp(argv[1], "--"))
	    {
	      argv++;
	      argc--;
	    }

	  /* The device name must be the first argument */
	  if(argc == 2)
	    print_info(skfd, argv[1], NULL, 0);
	  else
	    /* The other args on the line specify options to be set... */
	    goterr = set_info(skfd, argv + 2, argc - 2, argv[1]);
	}

  /* Close the socket. */
  iw_sockets_close(skfd);

  return(goterr);
}
