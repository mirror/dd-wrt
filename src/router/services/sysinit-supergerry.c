int
start_sysinit (void)
{
  char buf[PATH_MAX];
  struct utsname name;
  struct stat tmp_stat;
  time_t tm = 0;
#ifdef HAVE_RB500
  unlink ("/etc/nvram/.lock");
#elif HAVE_XSCALE
  unlink ("/etc/nvram/.lock");
#endif
  cprintf ("sysinit() proc\n");
  /* /proc */
  mount ("proc", "/proc", "proc", MS_MGC_VAL, NULL);
#ifdef HAVE_XSCALE
  system ("/etc/convert");
  mount ("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
#endif
  cprintf ("sysinit() tmp\n");

  /* /tmp */
  mount ("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
#ifdef HAVE_RB500
  // fix for linux kernel 2.6
  mount ("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
#elif HAVE_XSCALE
  mount ("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
  eval ("mount", "/etc/www.fs", "/www", "-t", "squashfs", "-o", "loop");
  eval ("mount", "/etc/modules.fs", "/lib/modules", "-t", "squashfs", "-o",
	"loop");
  eval ("mount", "/etc/usr.fs", "/usr", "-t", "squashfs", "-o", "loop");
#endif
  eval ("mkdir", "/tmp/www");

#ifdef HAVE_RB500
  //load ext2 
  // eval("insmod","jbd");
  eval ("insmod", "ext2");
#ifndef KERNEL_24
  if (mount ("/dev/cf/card0/part3", "/usr/local", "ext2", MS_MGC_VAL, NULL))
#else
  if (mount
      ("/dev/discs/disc0/part3", "/usr/local", "ext2", MS_MGC_VAL, NULL))
#endif
    {
      //not created yet, create ext2 partition
      eval ("/sbin/mke2fs", "-F", "-b", "1024", "/dev/cf/card0/part3");
      //mount ext2 
      mount ("/dev/cf/card0/part3", "/usr/local", "ext2", MS_MGC_VAL, NULL);
      eval ("/bin/tar", "-xvvjf", "/etc/local.tar.bz2", "-C", "/");
      mkdir ("/usr/local/nvram", 0777);
//    eval("ln","-s","/etc/nvram","/usr/local/nvram");
    }

  unlink ("/tmp/nvram/.lock");
  eval ("mkdir", "/tmp/nvram");
  eval ("cp", "/etc/nvram/nvram.db", "/tmp/nvram");
  eval ("cp", "/etc/nvram/offsets.db", "/tmp/nvram");
#elif HAVE_XSCALE
  eval ("mount", "-o", "remount,rw", "/");
//if (eval("mount","-t","jffs2","/dev/mtdblock/3","/etc/nvram"))
//    eval("mtd","erase","DDWRT");
//eval("mount","-t","jffs2","/dev/mtdblock/3","/etc/nvram");    
  mkdir ("/usr/local/nvram", 0777);
  unlink ("/tmp/nvram/.lock");
  eval ("mkdir", "/tmp/nvram");
  eval ("cp", "/etc/nvram/nvram.db", "/tmp/nvram");
  eval ("cp", "/etc/nvram/offsets.db", "/tmp/nvram");
#endif
  cprintf ("sysinit() var\n");

  /* /var */
  mkdir ("/tmp/var", 0777);
  mkdir ("/var/lock", 0777);
  mkdir ("/var/log", 0777);
  mkdir ("/var/run", 0777);
  mkdir ("/var/tmp", 0777);
  cprintf ("sysinit() setup console\n");

  /* Setup console */

  cprintf ("sysinit() klogctl\n");
  klogctl (8, NULL, atoi (nvram_safe_get ("console_loglevel")));
  cprintf ("sysinit() get router\n");

  int brand = getRouterBrand ();

  switch (brand)
    {
    case ROUTER_BUFFALO_WZRRSG54:
      check_brcm_cpu_type ();
      setup_4712 ();
      break;

    case ROUTER_MOTOROLA:
      nvram_set ("cpu_type", "BCM4712");
      nvram_set ("wl0gpio0", "2");	//Fix for wireless led, Eko.10.may.06
      setup_4712 ();
      break;

    case ROUTER_SIEMENS:
    case ROUTER_BELKIN_F5D7230:
      setup_4712 ();
      break;

    case ROUTER_RT210W:
      setup_4712 ();
      nvram_set ("wan_ifname", "eth1");	// fix for Belkin f5d7230 v1000 WAN problem.
      nvram_set ("wan_ifnames", "eth1");
      if (nvram_get ("et0macaddr") == NULL || nvram_get ("et0macaddr") == "")
	{
	  nvram_set ("et0macaddr", "00:16:E3:00:00:10");	//fix for missing cfe default = dead LAN ports.
	}
      eval ("gpio", "disable", "5");	// power led on
      break;

    case ROUTER_BRCM4702_GENERIC:
      setup_4712 ();
      nvram_set ("wan_ifname", "eth1");	// fix for Belkin f5d7230 v1000 WAN problem.
      nvram_set ("wan_ifnames", "eth1");

      if (nvram_get ("et0macaddr") == NULL || nvram_get ("et0macaddr") == "")
	{
	  nvram_set ("et0macaddr", "00:0C:6E:00:00:10");	//fix for missing cfe default = dead LAN ports.
	}

    case ROUTER_WZRG300N:
    case ROUTER_WRT300N:
    case ROUTER_WRTSL54GS:
      nvram_set ("wan_ifname", "eth1");
      nvram_set ("wan_ifnames", "eth1");
      nvram_set ("pppoe_wan_ifname", "eth1");
      break;

    case ROUTER_ASUS_WL500G_PRE:
      nvram_set ("sdram_init", "0x0009");
      nvram_set ("lan_ifnames", "vlan0 eth2");
      nvram_set ("wl0_ifname", "eth2");
      strcpy (wlifname, "eth2");
      nvram_set ("wan_ifname", "vlan1");	// fix for Asus WL500gPremium WAN problem.
      nvram_set ("wan_ifnames", "vlan1");
      strcpy (wanifname, "vlan1");
      nvram_set ("vlan1ports", "0 5");
      eval ("gpio", "disable", "1");	//Asus WL-500gP power led on
      eval ("gpio", "disable", "0");	//reset the reset button to 0
      break;

    case ROUTER_MICROSOFT_MN700:
      eval ("gpio", "enable", "6");	//MN700 power led on
      break;

#ifndef HAVE_MSSID
    case ROUTER_BUFFALO_WBR54G:
      nvram_set ("wl0gpio0", "130");	//Fix for wireless led polarity (v23 only)
      break;
#endif

    case ROUTER_BUFFALO_WBR2G54S:
      eval ("gpio", "disable", "1");	//WBR2G54 diag led off
      break;

    case ROUTER_BUFFALO_WLA2G54C:
      nvram_set ("lan_ifnames", "eth0 eth1");	// fix for WLA2G54C interfaces
      nvram_set ("wl0_ifname", "eth1");
      strcpy (wlifname, "eth1");
      nvram_set ("wan_ifname", "eth2");	// map WAN port to nonexistant interface
      nvram_set ("wan_ifnames", "eth2");
      eval ("gpio", "enable", "3");	//WLA2-G54C, WLA3-TX1-G54 diag led off
      eval ("gpio", "enable", "4");
      break;
    }

  if (nvram_match ("boardnum", "1024") && nvram_match ("boardtype", "0x0446"))
    {
      nvram_set ("lan_ifnames", "eth0 eth1");	// fix for WAP54Gv2 interfaces
      nvram_set ("wl0_ifname", "eth1");
      strcpy (wlifname, "eth1");
      nvram_set ("wan_ifname", "eth2");	// map WAN port to nonexistant interface
      nvram_set ("wan_ifnames", "eth2");
    }

  /* Modules */
  uname (&name);

  enableAfterBurner ();
#ifdef HAVE_RB500
#define MODULES
#elif HAVE_XSCALE
#define MODULES
#endif

#ifndef MODULES
  snprintf (buf, sizeof (buf), "/lib/modules/%s", name.release);
  if (stat ("/proc/modules", &tmp_stat) == 0 && stat (buf, &tmp_stat) == 0)
    {
      char module[80], *modules, *next;
      //modules="wl switch-core";

      if (check_vlan_support () && check_hw_type () != BCM5325E_CHIP)
	{
	  switch (brand)
	    {
	    case ROUTER_LINKSYS_WRT55AG:
	    case ROUTER_MOTOROLA_V1:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl switch-core switch-adm";

	      break;
	    case ROUTER_WRT54G1X:
	    case ROUTER_WRT54G:
	    case ROUTER_SIEMENS:
	    case ROUTER_RT210W:
	    case ROUTER_BRCM4702_GENERIC:
	    case ROUTER_BELKIN_F5D7230:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl";
	      eval ("insmod", "switch-core");
	      if (eval ("insmod", "switch-robo"))
		eval ("insmod", "switch-adm");

	      break;
//          case ROUTER_BUFFALO_WBR54G:
	    case ROUTER_MOTOROLA:
	    case ROUTER_BUFFALO_WBR2G54S:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl switch-core switch-robo switch-adm";
	      break;
	    default:
//          case ROUTER_MOTOROLA:
//          case ROUTER_BELKIN:
//          case ROUTER_BUFFALO_WBR2G54S:
//          case ROUTER_ASUS:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl switch-core switch-robo";
	      break;
	    }
	}
      else
	{
	  switch (brand)
	    {
	    case ROUTER_LINKSYS_WRT55AG:
	    case ROUTER_MOTOROLA_V1:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl switch-core switch-adm";

	      break;
	    case ROUTER_ASUS:
	    case ROUTER_BELKIN_F5D7230:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl";
	      eval ("insmod", "switch-core");
	      if (eval ("insmod", "switch-robo"))
		eval ("insmod", "switch-adm");
	      break;
	    case ROUTER_BUFFALO_WZRRSG54:
	      modules =
		nvram_invmatch ("ct_modules",
				"") ? nvram_safe_get ("ct_modules") :
		"diag wl";
	      break;
	    default:
	      if (check_vlan_support ())
		modules =
		  nvram_invmatch ("ct_modules",
				  "") ? nvram_safe_get ("ct_modules") :
		  "diag wl switch-core switch-robo";
	      else
		modules =
		  nvram_invmatch ("ct_modules",
				  "") ? nvram_safe_get ("ct_modules") :
		  "diag wl";
	      break;
	    }
	}
      cprintf ("insmod %s\n", modules);

      foreach (module, modules, next)
      {
#ifdef HAVE_MACBIND
	if (nvram_match ("et0macaddr", MACBRAND))
	  eval ("insmod", module);
#else
/*insmod("diag");
insmod("wl");
if (check_vlan_support())
    {
    insmod("switch-core");
    if (insmod("switch-robo"))
	insmod("switch-adm");
    }
*/
	cprintf ("loading %s\n", module);
	eval ("insmod", module);
	cprintf ("done\n");
#endif
      }
    }
#else
  eval ("insmod", "md5");
  eval ("insmod", "aes");
  eval ("insmod", "blowfish");
  eval ("insmod", "deflate");
  eval ("insmod", "des");
  eval ("insmod", "michael_mic");
  eval ("insmod", "cast5");
  eval ("insmod", "crypto_null");

  system ("/etc/kendin");
  eval ("insmod", "ixp400th");
  eval ("insmod", "ixp400");
  system ("cat /usr/lib/firmware/IxNpeMicrocode.dat > /dev/IxNpe");
  eval ("insmod", "ixp400_eth");
  eval ("insmod", "ocf");
  eval ("insmod", "cryptodev");
  eval ("insmod", "ixp4xx", "init_crypto=0");
  eval ("ifconfig", "ixp0", "0.0.0.0", "up");
  eval ("vconfig", "add", "ixp0", "1");
  eval ("vconfig", "add", "ixp0", "2");

  eval ("insmod", "ath_hal");
  eval ("insmod", "wlan");
  eval ("insmod", "ath_rate_sample");
  eval ("insmod", "ath_pci", "rfkill=0", "autocreate=none");


  eval ("insmod", "wlan_acl");
  eval ("insmod", "wlan_ccmp");
  eval ("insmod", "wlan_tkip");
  eval ("insmod", "wlan_wep");
  eval ("insmod", "wlan_xauth");
  eval ("insmod", "wlan_scan_ap");
  eval ("insmod", "wlan_scan_sta");

  eval ("ifconfig", "wifi0", "up");
  eval ("ifconfig", "wifi1", "up");
  eval ("ifconfig", "wifi2", "up");
  eval ("ifconfig", "wifi3", "up");
  eval ("ifconfig", "wifi4", "up");
  eval ("ifconfig", "wifi5", "up");


//  eval ("insmod", "mii");
//  eval ("insmod", "korina");
//  eval ("insmod", "via-rhine");
  eval ("insmod", "ipv6");
//  load_drivers(); //load madwifi drivers
#endif
  /* Set a sane date */
  stime (&tm);
  if (brand == ROUTER_SIEMENS)
    {
      eval ("insmod", "led.o");	// Jerry Lee
      powerled_ctrl (0);
      led_ctrl (0);		// turn LED2 off
    }

  return 0;
  cprintf ("done\n");
}
