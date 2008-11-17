
#define WL(a)	"wl_"a
#define ATH0(a)	"ath0_"a
#define WL0(a)	"wl0_"a

#define PPP(a)		"ppp_"a
#define PPPOE(a)	"pppoe_"a

struct nvram_convert
{
  char *name;			// for WEB
  char *wl0_name;		// for driver
};

struct nvram_convert nvram_converts[] = {
  // Bellow change from 3.11.48.7
#ifdef HAVE_MADWIFI
  {WL ("ssid"), ATH0 ("ssid")},
#else
  {WL ("ssid"), WL0 ("ssid")},
#endif
  {WL ("radio"), WL0 ("radio")},
  {WL ("mode"), WL0 ("mode")},
  {WL ("wds"), WL0 ("wds")},
  {WL ("auth"), WL0 ("auth")},
  {WL ("key"), WL0 ("key")},
  {WL ("key1"), WL0 ("key1")},
  {WL ("key2"), WL0 ("key2")},
  {WL ("key3"), WL0 ("key3")},
  {WL ("key4"), WL0 ("key4")},
  {WL ("maclist"), WL0 ("maclist")},
  {WL ("channel"), WL0 ("channel")},
  {WL ("rateset"), WL0 ("rateset")},
  {WL ("rts"), WL0 ("rts")},
  {WL ("bcn"), WL0 ("bcn")},
  {WL ("gmode"), WL0 ("gmode")},
  {WL ("unit"), WL0 ("unit")},
  {WL ("ifname"), WL0 ("ifname")},
  {WL ("phytype"), WL0 ("phytype")},
  {WL ("country"), WL0 ("country")},
  {WL ("closed"), WL0 ("closed")},
  {WL ("lazywds"), WL0 ("lazywds")},
//  {WL ("wep"), WL0 ("wep")},
  {WL ("macmode"), WL0 ("macmode")},
  {WL ("rate"), WL0 ("rate")},
  {WL ("frag"), WL0 ("frag")},
  {WL ("dtim"), WL0 ("dtim")},
  {WL ("plcphdr"), WL0 ("plcphdr")},
  {WL ("gmode_protection"), WL0 ("gmode_protection")},
  // Bellow change from 3.21.9.0
  {WL ("auth_mode"), WL0 ("auth_mode")},
  {WL ("radauth"), WL0 ("radauth")},
  {WL ("radmactype"), WL0 ("radmactype")},
  {WL ("radius_ipaddr"), WL0 ("radius_ipaddr")},
  {WL ("radius_port"), WL0 ("radius_port")},
  {WL ("radius_key"), WL0 ("radius_key")},
  {WL ("wpa_psk"), WL0 ("wpa_psk")},
  {WL ("wpa_gtk_rekey"), WL0 ("wpa_gtk_rekey")},
  {WL ("frameburst"), WL0 ("frameburst")},
  // Below were missing wl variables - wolf 20040924
  {WL ("crypto"), WL0 ("crypto")},
  {WL ("antdiv"), WL0 ("antdiv")},
  {WL ("afterburner"), WL0 ("afterburner")},
  {WL ("hwaddr"), WL0 ("hwaddr")},
  {WL ("ap_isolate"), WL0 ("ap_isolate")},
  {WL ("distance"), WL0 ("distance")},	//ack timing
  // Below change from 3.63.13.1
  {WL ("akm"), WL0 ("akm")},
  {WL ("preauth"), WL0 ("preauth")},
  {WL ("wme"), WL0 ("wme")},
  {WL ("wme_sta_bk"), WL0 ("wme_sta_bk")},
  {WL ("wme_sta_be"), WL0 ("wme_sta_be")},
  {WL ("wme_sta_vi"), WL0 ("wme_sta_vi")},
  {WL ("wme_sta_vo"), WL0 ("wme_sta_vo")},
  {WL ("wme_ap_bk"), WL0 ("wme_ap_bk")},
  {WL ("wme_ap_be"), WL0 ("wme_ap_be")},
  {WL ("wme_ap_vi"), WL0 ("wme_ap_vi")},
  {WL ("wme_ap_vo"), WL0 ("wme_ap_vo")},
  {WL ("wme_no_ack"), WL0 ("wme_no_ack")},
  {WL ("wme_apsd"), WL0 ("wme_apsd")},
  {WL ("shortslot"), WL0 ("shortslot")},
  {WL ("sta_retry_time"), WL0 ("sta_retry_time")},
  {WL ("reg_mode"), WL0 ("reg_mode")},
  {WL ("dfs_preism"), WL0 ("dfs_preism")},
  {WL ("dfs_postism"), WL0 ("dfs_postism")},
  {WL ("mrate"), WL0 ("mrate")},
  {WL ("maxassoc"), WL0 ("maxassoc")},
  {WL ("ure"), WL0 ("ure")},
  {WL ("wds_timeout"), WL0 ("wds_timeout")},
  {WL ("nbw"), WL0 ("nbw")},
  {WL ("nctrlsb"), WL0 ("nctrlsb")},
  {WL ("nband"), WL0 ("nband")},
  {WL ("nmcsidx"), WL0 ("nmcsidx")},
  {WL ("nstf"), WL0 ("nstf")},
  {WL ("nmode_protection"), WL0 ("nmode_protection")},
  {WL ("nreqd"), WL0 ("nreqd")},
  {WL ("vlan_prio_mode"), WL0 ("vlan_prio_mode")},
  {WL ("amsdu"), WL0 ("amsdu")},
  {WL ("ampdu"), WL0 ("ampdu")},
  {WL ("nmode"), WL0 ("nmode")},
  {WL ("radauth"), WL0 ("radauth")},
  {WL ("btc_mode"), WL0 ("btc_mode")},
  {WL ("phytypes"), WL0 ("phytypes")},

  // for PPPoE
  {PPP ("username"), PPPOE ("username")},
  {PPP ("passwd"), PPPOE ("passwd")},
  {PPP ("idletime"), PPPOE ("idletime")},
  {PPP ("keepalive"), PPPOE ("keepalive")},
  {PPP ("demand"), PPPOE ("demand")},
  {PPP ("service"), PPPOE ("service")},
  {PPP ("ac"), PPPOE ("ac")},
  {PPP ("static"), PPPOE ("static")},
  {PPP ("static_ip"), PPPOE ("static_ip")},
  {PPP ("username_1"), PPPOE ("username_1")},
  {PPP ("passwd_1"), PPPOE ("passwd_1")},
  {PPP ("idletime_1"), PPPOE ("idletime_1")},
  {PPP ("keepalive_1"), PPPOE ("keepalive_1")},
  {PPP ("demand_1"), PPPOE ("demand_1")},
  {PPP ("service_1"), PPPOE ("service_1")},
  {PPP ("ac_1"), PPPOE ("ac_1")},

  {0, 0},
};
