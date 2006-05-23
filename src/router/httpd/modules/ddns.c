
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>

#include <broadcom.h>

struct ddns_message
{
  char *name;
  char *desc;
};


#if LANGUAGE == JAPANESE
struct ddns_message ddns_messages[] = {
  // Below is DynDNS error code
  {"dyn_good", "DDNS ‚Ì?X?V‚ÍŠ®—¹‚µ‚Ü‚µ‚½"},
  {"dyn_noupdate", "DDNS ‚Ì?X?V‚ÍŠ®—¹‚µ‚Ü‚µ‚½"},
  {"dyn_nohost", "ƒzƒXƒg–¼‚ª‘¶?Ý‚µ‚Ü‚¹‚ñ"},
  {"dyn_notfqdn", "ƒhƒ?ƒCƒ“–¼‚ª?³‚µ‚­‚ ‚è‚Ü‚¹‚ñ"},
  {"dyn_!yours", "ƒ†?[ƒU?[–¼‚ª?³‚µ‚­‚ ‚è‚Ü‚¹‚ñ"},
  {"dyn_abuse", "ƒzƒXƒg–¼‚Í DDNS ƒT?[ƒo‚É‚æ‚èƒuƒ?ƒbƒN‚³‚ê‚Ä‚¢‚Ü‚·"},
  {"dyn_nochg", "DDNS ‚Ì?X?V‚ÍŠ®—¹‚µ‚Ü‚µ‚½"},
  {"dyn_badauth", "”F?Ø‚ÉŽ¸”s‚µ‚Ü‚µ‚½ (ƒ†?[ƒU?[–¼‚Ü‚½‚ÍƒpƒXƒ??[ƒh)"},
  {"dyn_badsys", "ƒVƒXƒeƒ€ ƒpƒ‰ƒ??[ƒ^‚ª•s?³‚Å‚·"},
  {"dyn_badagent", "‚±‚Ìƒ†?[ƒU?[ ƒG?[ƒWƒFƒ“ƒg‚Íƒuƒ?ƒbƒN‚³‚ê‚Ä‚¢‚Ü‚·"},
  {"dyn_numhost", "ƒzƒXƒg‚ª‘½‚·‚¬‚é‚©?­‚È‚·‚¬‚Ü‚·"},
  {"dyn_dnserr", "DNS ƒGƒ‰?[”­?¶"},
  -{"dyn_911", "—\Šú‚¹‚ÊƒGƒ‰?[‚Å‚·?B (1)"},
  -{"dyn_999", "—\Šú‚¹‚ÊƒGƒ‰?[‚Å‚·?B (2)"},
  {"dyn_!donator",
   "ƒŠƒNƒGƒXƒg‚³‚ê‚½‹@”\‚ÍŠñ•t‚µ‚½?ê?‡‚É‚Ì‚Ý—LŒø‚Å‚·?BŠñ•t‚ð‚µ‚Ä‚­‚¾‚³‚¢?B"},
  -{"dyn_strange",
    "‰ž“š‚ª•s?³‚Å‚·?B?Ú‘±?æƒT?[ƒo‚ª?³‚µ‚¢‚©‚Ç‚¤‚©?AŠm”F‚­‚¾‚³‚¢?B"},
  {"dyn_uncode", "DynDns ‚©‚ç‚Ì•s–¾‚ÈƒŠƒ^?[ƒ“ ƒR?[ƒh"},

  // Below is TZO error code
  {"tzo_good", "Operation Complete"},
  {"tzo_noupdate", "Operation Complete"},
  {"tzo_notfqdn", "Invalid Domain Name"},
  {"tzo_notmail", "Invalis Email"},
  {"tzo_notact", "Invalid Action"},
  {"tzo_notkey", "Invalid Key"},
  {"tzo_notip", "Invalid IP address"},
  {"tzo_dupfqdn", "Duplicate Domain Name"},
  {"tzo_fqdncre",
   "Domain Name has already been created for this domain name"},
  {"tzo_expired", "The account has expired"},
  {"tzo_error", "An unexpected server error"},

  // Below is for all
  {"all_closed", "DDNS ƒT?[ƒo‚ÍŒ»?ÝƒNƒ??[ƒY‚µ‚Ä‚¢‚Ü‚·"},
  -{"all_resolving", "ƒhƒ?ƒCƒ“–¼‚ð‰ðŒˆ’†"},
  -{"all_errresolv", "ƒhƒ?ƒCƒ“–¼‚Ì‰ðŒˆ‚ÉŽ¸”s‚µ‚Ü‚µ‚½?B"},
  -{"all_connecting", "ƒT?[ƒo‚Ö?Ú‘±’†"},
  -{"all_connectfail", "ƒT?[ƒo‚Ö‚Ì?Ú‘±‚ÉŽ¸”s‚µ‚Ü‚µ‚½?B"},
  -{"all_disabled", "DDNS ‚Í–³Œø‚Å‚·"},
  -{"all_noip", "ƒCƒ“ƒ^?[ƒlƒbƒg?Ú‘±‚ª‚ ‚è‚Ü‚¹‚ñ"},
};
#else
struct ddns_message ddns_messages[] = {
  // Below is DynDNS error code
  {"dyn_strange", "ddnsm.dyn_strange"},
  {"dyn_good", "ddnsm.dyn_good"},
  {"dyn_noupdate", "ddnsm.dyn_noupdate"},
  {"dyn_nohost", "ddnsm.dyn_nohost"},
  {"dyn_notfqdn", "ddnsm.dyn_notfqdn"},
  {"dyn_!yours", "ddnsm.dyn_yours"},
  {"dyn_abuse", "ddnsm.dyn_abuse"},
  {"dyn_nochg", "ddnsm.dyn_nochg"},
  {"dyn_badauth", "ddnsm.dyn_badauth"},
  {"dyn_badsys", "ddnsm.dyn_badsys"},
  {"dyn_badagent", "ddnsm.dyn_badagent"},
  {"dyn_numhost", "ddnsm.dyn_numhost"},
  {"dyn_dnserr", "ddnsm.dyn_dnserr"},
  {"dyn_911", "ddnsm.dyn_911"},
  {"dyn_999", "ddnsm.dyn_999"},
  {"dyn_!donator", "ddnsm.dyn_donator"},
  {"dyn_uncode", "ddnsm.dyn_uncode"},

  // Below is TZO error code
  {"tzo_good", "ddnsm.tzo_good"},
  {"tzo_noupdate", "ddnsm.tzo_noupdate"},
  {"tzo_notfqdn", "ddnsm.tzo_notfqdn"},
  {"tzo_notmail", "ddnsm.tzo_notmail"},
  {"tzo_notact", "ddnsm.tzo_notact"},
  {"tzo_notkey", "ddnsm.tzo_notkey"},
  {"tzo_notip", "ddnsm.tzo_notip"},
  {"tzo_dupfqdn", "ddnsm.tzo_dupfqdn"},
  {"tzo_fqdncre", "ddnsm.tzo_fqdncre"},
  {"tzo_expired", "ddnsm.tzo_expired"},
  {"tzo_error", "ddnsm.tzo_error"},

  // Below is ZON error code

  {"zone_701", "ddnsm.zone_701"},
  {"zone_702", "ddnsm.zone_702"},
  {"zone_703", "ddnsm.zone_703"},
  {"zone_704", "ddnsm.zone_704"},
  {"zone_705", "ddnsm.zone_705"},
  {"zone_707", "ddnsm.zone_707"},
  {"zone_201", "ddnsm.zone_201"},
  {"zone_badauth", "ddnsm.zone_badauth"},
  {"zone_good", "ddnsm.zone_good"},
  {"zone_strange", "ddnsm.zone_strange"},

  // Below is for all
  {"all_closed", "ddnsm.all_closed"},
  {"all_resolving", "ddnsm.all_resolving"},
  {"all_errresolv", "ddnsm.all_errresolv"},
  {"all_connecting", "ddnsm.all_connecting"},
  {"all_connectfail", "ddnsm.all_connectfail"},
  {"all_disabled", "ddnsm.all_disabled"},
  {"all_noip", "ddnsm.all_noip"},
};
#endif

char *
convert (char *msg)
{
  static char buf[200];
  struct ddns_message *m;

  for (m = ddns_messages; m < &ddns_messages[STRUCT_LEN (ddns_messages)]; m++)
    {
      if (!strcmp (m->name, msg))
	{
	  snprintf (buf, sizeof (buf), "%s", m->desc);
	  return buf;
	}
    }

  snprintf (buf, sizeof (buf), "%s", msg);
  return buf;
}

void
ej_show_ddns_status (int eid, webs_t wp, int argc, char_t ** argv)
{
  char string[80] = "";
  char *enable = websGetVar (wp, "ddns_enable", NULL);

  if (!enable)
    enable = nvram_safe_get ("ddns_enable");	// for first time

  if (strcmp (nvram_safe_get ("ddns_enable"), enable))	// change service
    {
      websWrite (wp, " ");
    }

  if (nvram_match ("ddns_enable", "0"))	// only for no hidden page
    {
      websWrite (wp, "%s", convert ("all_disabled"));
      return;
    }
  if (!check_wan_link (0))
    {
      websWrite (wp, "%s", convert ("all_noip"));
      return;
    }
  if (file_to_buf ("/tmp/ddns_msg", string, sizeof (string)))
    {
      if (!strcmp (string, ""))
	{
	  if (nvram_match ("ddns_status", "1"))
	    {
	      if (nvram_match ("ddns_enable", "1"))
		websWrite (wp, "%s", convert ("dyn_good"));	// dyndns
	      if (nvram_match ("ddns_enable", "2"))
		websWrite (wp, "%s", convert ("tzo_good"));	// tzo
	      if (nvram_match ("ddns_enable", "3"))
		websWrite (wp, "%s", convert ("zone_good"));	// zoneedit
	    }
	  else
	    websWrite (wp, "%s", convert ("all_closed"));
	}
      else
	websWrite (wp, "%s", convert (string));
    }

  return;
}

int
ddns_save_value (webs_t wp)
{
  char *enable, *username, *passwd, *hostname, *dyndnstype, *wildcard;
  struct variable ddns_variables[] = {
  {longname: "DDNS enable", argv:ARGV ("0", "1", "2", "3")},
  {longname: "DDNS password", argv:ARGV ("30")},
  }, *which;
  int ret = -1;
  char _username[] = "ddns_username_X";
  char _passwd[] = "ddns_passwd_X";
  char _hostname[] = "ddns_hostname_X";
  char _dyndnstype[] = "ddns_dyndnstype_X";
  char _wildcard[] = "ddns_wildcard_X";

  which = &ddns_variables[0];

  enable = websGetVar (wp, "ddns_enable", NULL);
  if (!enable && !valid_choice (wp, enable, &which[0]))
    {
      error_value = 1;
      return 1;
    }

  if (atoi (enable) == 0)
    {				// Disable
      nvram_set ("ddns_enable_buf", nvram_safe_get ("ddns_enable"));
      nvram_set ("ddns_enable", enable);
      return 1;
    }
  else if (atoi (enable) == 1)
    {				// dyndns
      snprintf (_username, sizeof (_username), "%s", "ddns_username");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname");
      snprintf (_dyndnstype, sizeof (_dyndnstype), "%s", "ddns_dyndnstype");
      snprintf (_wildcard, sizeof (_wildcard), "%s", "ddns_wildcard");
    }
  else if (atoi (enable) == 2)
    {				// tzo
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }
  else if (atoi (enable) == 3)
    {				// Zoneedit
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }

  username = websGetVar (wp, _username, NULL);
  passwd = websGetVar (wp, _passwd, NULL);
  hostname = websGetVar (wp, _hostname, NULL);
  dyndnstype = websGetVar (wp, _dyndnstype, NULL);
  wildcard = websGetVar (wp, _wildcard, NULL);

  if (!username || !passwd || !hostname)
    {
      error_value = 1;
      return 1;
    }

  nvram_set ("ddns_enable_buf", nvram_safe_get ("ddns_enable"));
  nvram_set ("ddns_username_buf", nvram_safe_get (_username));
  nvram_set ("ddns_passwd_buf", nvram_safe_get (_passwd));
  nvram_set ("ddns_hostname_buf", nvram_safe_get (_hostname));
  nvram_set ("ddns_dyndnstype_buf", nvram_safe_get (_dyndnstype));
  nvram_set ("ddns_wildcard_buf", nvram_safe_get (_wildcard));
  nvram_set ("ddns_enable", enable);
  nvram_set (_username, username);
  if (strcmp (passwd, TMP_PASSWD))
    nvram_set (_passwd, passwd);
  nvram_set (_hostname, hostname);
  nvram_set (_dyndnstype, dyndnstype);
  nvram_set (_wildcard, wildcard);

  return ret;
}

int
ddns_update_value (webs_t wp)
{
  return 1;
}

void
ej_show_ddns_ip (int eid, webs_t wp, int argc, char_t ** argv)
{

  if (check_wan_link (0))
    {
      if (nvram_match ("wan_proto", "pptp"))
	websWrite (wp, "%s", nvram_safe_get ("pptp_get_ip"));
      else if (nvram_match ("wan_proto", "l2tp"))
	websWrite (wp, "%s", nvram_safe_get ("l2tp_get_ip"));
      else if (nvram_match ("pptpd_connected", "1"))
	websWrite (wp, "%s", nvram_safe_get ("pptpd_client_info_localip"));
      else
	websWrite (wp, "%s", nvram_safe_get ("wan_ipaddr"));
    }
  else
    websWrite (wp, "0.0.0.0");

  return;
}
