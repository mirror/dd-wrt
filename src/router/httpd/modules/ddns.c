
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
  {"dyn_good", "DDNS �X?V͊܂"},
  {"dyn_noupdate", "DDNS �X?V͊܂"},
  {"dyn_nohost", "zXg?݂܂�},
  {"dyn_notfqdn", "h?C?��},
  {"dyn_!yours", "?[U?[?��},
  {"dyn_abuse", "zXg�DDNS T?[oɂ�u?bN�܂"},
  {"dyn_nochg", "DDNS �X?V͊܂"},
  {"dyn_badauth", "F?؂Ɏs܂ (?[U?[܂̓pX??[h)"},
  {"dyn_badsys", "VXe p??[^s?ł"},
  {"dyn_badagent", "̃?[U?[ G?[WFg̓u?bN�܂"},
  {"dyn_numhost", "zXg邩?Ȃ܂"},
  {"dyn_dnserr", "DNS G?[?"},
  -{"dyn_911", "\ʃG?[ł?B (1)"},
  -{"dyn_999", "\ʃG?[ł?B (2)"},
  {"dyn_!donator",
   "NGXgꂽ@\͊�?�ɂ̂ݗLł?B�ⵂĂ?B"},
  -{"dyn_strange",
    "s?ł?B?ڑ?�?[o?ǂ?AmF?B"},
  {"dyn_uncode", "DynDns �sȃ^?[ R?[h"},

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
  {"all_closed", "DDNS T?[o͌?݃N??[YĂ܂"},
  -{"all_resolving", "h?C�"},
  -{"all_errresolv", "h?C̉숂Ɏs܂?B"},
  -{"all_connecting", "T?[o�ڑ"},
  -{"all_connectfail", "T?[oւ�ڑɎs܂?B"},
  -{"all_disabled", "DDNS ͖ł"},
  -{"all_noip", "C^?[lbg?ڑ��},
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
  char *enable, *username, *passwd, *hostname, *dyndnstype;
  struct variable ddns_variables[] = {
  {longname: "DDNS enable", argv:ARGV ("0", "1", "2", "3", "4", "5")},
  {longname: "DDNS password", argv:ARGV ("30")},
  }, *which;
  int ret = -1;
  char _username[] = "ddns_username_X";
  char _passwd[] = "ddns_passwd_X";
  char _hostname[] = "ddns_hostname_X";
  char _dyndnstype[] = "ddns_dyndnstype_X";

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
    }
  else if (atoi (enable) == 2)
    {				// afraid
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }
  else if (atoi (enable) == 3)
    {				// zoneedit
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }
  else if (atoi (enable) == 4)
    {				// no-ip
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }
  else if (atoi (enable) == 5)
    {				// custom
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }

  username = websGetVar (wp, _username, NULL);
  passwd = websGetVar (wp, _passwd, NULL);
  hostname = websGetVar (wp, _hostname, NULL);
  dyndnstype = websGetVar (wp, _dyndnstype, NULL);

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
  nvram_set ("ddns_enable", enable);
  nvram_set (_username, username);
  if (strcmp (passwd, TMP_PASSWD))
    nvram_set (_passwd, passwd);
  nvram_set (_hostname, hostname);
  nvram_set (_dyndnstype, dyndnstype);

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
