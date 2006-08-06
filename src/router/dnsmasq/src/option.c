/* dnsmasq is Copyright (c) 2000 - 2006 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* define this to get facilitynames */
#define SYSLOG_NAMES

#include "dnsmasq.h"

#ifndef HAVE_GETOPT_LONG
struct myoption {
  const char *name;
  int has_arg;
  int *flag;
  int val;
};
#endif

#define OPTSTRING "9531yZDNLERKzowefnbvhdkqr:m:p:c:l:s:i:t:u:g:a:x:S:C:A:T:H:Q:I:B:F:G:O:M:X:V:U:j:P:J:W:Y:2:4:6:7:8:0:"

#ifdef HAVE_GETOPT_LONG
static const struct option opts[] =  
#else
static const struct myoption opts[] = 
#endif
  { 
    {"version", 0, 0, 'v'},
    {"no-hosts", 0, 0, 'h'},
    {"no-poll", 0, 0, 'n'},
    {"help", 0, 0, 'w'},
    {"no-daemon", 0, 0, 'd'},
    {"log-queries", 0, 0, 'q'},
    {"user", 1, 0, 'u'},
    {"group", 1, 0, 'g'},
    {"resolv-file", 1, 0, 'r'},
    {"mx-host", 1, 0, 'm'},
    {"mx-target", 1, 0, 't'},
    {"cache-size", 1, 0, 'c'},
    {"port", 1, 0, 'p'},
    {"dhcp-leasefile", 1, 0, 'l'},
    {"dhcp-lease", 1, 0, 'l' },
    {"dhcp-host", 1, 0, 'G'},
    {"dhcp-range", 1, 0, 'F'},
    {"dhcp-option", 1, 0, 'O'},
    {"dhcp-boot", 1, 0, 'M'},
    {"domain", 1, 0, 's'},
    {"domain-suffix", 1, 0, 's'},
    {"interface", 1, 0, 'i'},
    {"listen-address", 1, 0, 'a'},
    {"bogus-priv", 0, 0, 'b'},
    {"bogus-nxdomain", 1, 0, 'B'},
    {"selfmx", 0, 0, 'e'},
    {"filterwin2k", 0, 0, 'f'},
    {"pid-file", 1, 0, 'x'},
    {"strict-order", 0, 0, 'o'},
    {"server", 1, 0, 'S'},
    {"local", 1, 0, 'S' },
    {"address", 1, 0, 'A' },
    {"conf-file", 1, 0, 'C'},
    {"no-resolv", 0, 0, 'R'},
    {"expand-hosts", 0, 0, 'E'},
    {"localmx", 0, 0, 'L'},
    {"local-ttl", 1, 0, 'T'},
    {"no-negcache", 0, 0, 'N'},
    {"addn-hosts", 1, 0, 'H'},
    {"query-port", 1, 0, 'Q'},
    {"except-interface", 1, 0, 'I'},
    {"no-dhcp-interface", 1, 0, '2'},
    {"domain-needed", 0, 0, 'D'},
    {"dhcp-lease-max", 1, 0, 'X' },
    {"bind-interfaces", 0, 0, 'z'},
    {"read-ethers", 0, 0, 'Z' },
    {"alias", 1, 0, 'V' },
    {"dhcp-vendorclass", 1, 0, 'U'},
    {"dhcp-userclass", 1, 0, 'j'},
    {"dhcp-ignore", 1, 0, 'J'},
    {"edns-packet-max", 1, 0, 'P'},
    {"keep-in-foreground", 0, 0, 'k'},
    {"dhcp-authoritative", 0, 0, 'K'},
    {"srv-host", 1, 0, 'W'},
    {"localise-queries", 0, 0, 'y'},
    {"txt-record", 1, 0, 'Y'},
    {"enable-dbus", 0, 0, '1'},
    {"bootp-dynamic", 0, 0, '3'},
    {"dhcp-mac", 1, 0, '4'},
    {"no-ping", 0, 0, '5'},
    {"dhcp-script", 1, 0, '6'},
    {"conf-dir", 1, 0, '7'},
    {"log-facility", 1, 0 ,'8'},
    {"leasefile-ro", 0, 0, '9'},
    {"dns-forward-max", 1, 0, '0'},
    { NULL, 0, 0, 0 }
  };

struct optflags {
  char c;
  unsigned int flag; 
};

static const struct optflags optmap[] = {
  { 'b', OPT_BOGUSPRIV },
  { 'f', OPT_FILTER },
  { 'q', OPT_LOG },
  { 'e', OPT_SELFMX },
  { 'h', OPT_NO_HOSTS },
  { 'n', OPT_NO_POLL },
  { 'd', OPT_DEBUG },
  { 'k', OPT_NO_FORK },
  { 'K', OPT_AUTHORITATIVE },
  { 'o', OPT_ORDER },
  { 'R', OPT_NO_RESOLV },
  { 'E', OPT_EXPAND },
  { 'L', OPT_LOCALMX },
  { 'N', OPT_NO_NEG },
  { 'D', OPT_NODOTS_LOCAL },
  { 'z', OPT_NOWILD },
  { 'Z', OPT_ETHERS },
  { 'y', OPT_LOCALISE },
  { '1', OPT_DBUS },
  { '3', OPT_BOOTP_DYNAMIC },
  { '5', OPT_NO_PING },
  { '9', OPT_LEASE_RO },
  { 'v', 0},
  { 'w', 0},
  { 0, 0 }
};

static const struct {
  char * const flag;
  char * const desc;
  char * const arg;
} usage[] = {
  { "-a, --listen-address=ipaddr",  gettext_noop("Specify local address(es) to listen on."), NULL },
  { "-A, --address=/domain/ipaddr", gettext_noop("Return ipaddr for all hosts in specified domains."), NULL },
  { "-b, --bogus-priv", gettext_noop("Fake reverse lookups for RFC1918 private address ranges."), NULL },
  { "-B, --bogus-nxdomain=ipaddr", gettext_noop("Treat ipaddr as NXDOMAIN (defeats Verisign wildcard)."), NULL }, 
  { "-c, --cache-size=cachesize", gettext_noop("Specify the size of the cache in entries (defaults to %s)."), "$" },
  { "-C, --conf-file=path", gettext_noop("Specify configuration file (defaults to %s)."), CONFFILE },
  { "-d, --no-daemon", gettext_noop("Do NOT fork into the background: run in debug mode."), NULL },
  { "-D, --domain-needed", gettext_noop("Do NOT forward queries with no domain part."), NULL }, 
  { "-e, --selfmx", gettext_noop("Return self-pointing MX records for local hosts."), NULL },
  { "-E, --expand-hosts", gettext_noop("Expand simple names in /etc/hosts with domain-suffix."), NULL },
  { "-f, --filterwin2k", gettext_noop("Don't forward spurious DNS requests from Windows hosts."), NULL },
  { "-F, --dhcp-range=ipaddr,ipaddr,time", gettext_noop("Enable DHCP in the range given with lease duration."), NULL },
  { "-g, --group=groupname", gettext_noop("Change to this group after startup (defaults to %s)."), CHGRP },
  { "-G, --dhcp-host=<hostspec>", gettext_noop("Set address or hostname for a specified machine."), NULL }, 
  { "-h, --no-hosts", gettext_noop("Do NOT load %s file."), HOSTSFILE },
  { "-H, --addn-hosts=path", gettext_noop("Specify a hosts file to be read in addition to %s."), HOSTSFILE },
  { "-i, --interface=interface", gettext_noop("Specify interface(s) to listen on."), NULL },
  { "-I, --except-interface=int", gettext_noop("Specify interface(s) NOT to listen on.") , NULL },
  { "-j, --dhcp-userclass=<id>,<class>", gettext_noop("Map DHCP user class to option set."), NULL },
  { "-J, --dhcp-ignore=<id>", gettext_noop("Don't do DHCP for hosts in option set."), NULL },
  { "-k, --keep-in-foreground", gettext_noop("Do NOT fork into the background, do NOT run in debug mode."), NULL },
  { "-K, --dhcp-authoritative", gettext_noop("Assume we are the only DHCP server on the local network."), NULL },
  { "-l, --dhcp-leasefile=path", gettext_noop("Specify where to store DHCP leases (defaults to %s)."), LEASEFILE },
  { "-L, --localmx", gettext_noop("Return MX records for local hosts."), NULL },
  { "-m, --mx-host=host_name,target,pref", gettext_noop("Specify an MX record."), NULL },
  { "-M, --dhcp-boot=<bootp opts>", gettext_noop("Specify BOOTP options to DHCP server."), NULL },
  { "-n, --no-poll", gettext_noop("Do NOT poll %s file, reload only on SIGHUP."), RESOLVFILE }, 
  { "-N, --no-negcache", gettext_noop("Do NOT cache failed search results."), NULL },
  { "-o, --strict-order", gettext_noop("Use nameservers strictly in the order given in %s."), RESOLVFILE },
  { "-O, --dhcp-option=<optspec>", gettext_noop("Set extra options to be set to DHCP clients."), NULL },
  { "-p, --port=number", gettext_noop("Specify port to listen for DNS requests on (defaults to 53)."), NULL },
  { "-P, --edns-packet-max=<size>", gettext_noop("Maximum supported UDP packet size for EDNS.0 (defaults to %s)."), "*" },
  { "-q, --log-queries", gettext_noop("Log queries."), NULL },
  { "-Q, --query-port=number", gettext_noop("Force the originating port for upstream queries."), NULL },
  { "-R, --no-resolv", gettext_noop("Do NOT read resolv.conf."), NULL },
  { "-r, --resolv-file=path", gettext_noop("Specify path to resolv.conf (defaults to %s)."), RESOLVFILE }, 
  { "-S, --server=/domain/ipaddr", gettext_noop("Specify address(es) of upstream servers with optional domains."), NULL },
  { "    --local=/domain/", gettext_noop("Never forward queries to specified domains."), NULL },
  { "-s, --domain=domain", gettext_noop("Specify the domain to be assigned in DHCP leases."), NULL },
  { "-t, --mx-target=host_name", gettext_noop("Specify default target in an MX record."), NULL },
  { "-T, --local-ttl=time", gettext_noop("Specify time-to-live in seconds for replies from /etc/hosts."), NULL },
  { "-u, --user=username", gettext_noop("Change to this user after startup. (defaults to %s)."), CHUSER }, 
  { "-U, --dhcp-vendorclass=<id>,<class>", gettext_noop("Map DHCP vendor class to option set."), NULL },
  { "-v, --version", gettext_noop("Display dnsmasq version and copyright information."), NULL },
  { "-V, --alias=addr,addr,mask", gettext_noop("Translate IPv4 addresses from upstream servers."), NULL },
  { "-W, --srv-host=name,target,...", gettext_noop("Specify a SRV record."), NULL },
  { "-w, --help", gettext_noop("Display this message."), NULL },
  { "-x, --pid-file=path", gettext_noop("Specify path of PID file. (defaults to %s)."), RUNFILE },
  { "-X, --dhcp-lease-max=number", gettext_noop("Specify maximum number of DHCP leases (defaults to %s)."), "&" },
  { "-y, --localise-queries", gettext_noop("Answer DNS queries based on the interface a query was sent to."), NULL },
  { "-Y  --txt-record=name,txt....", gettext_noop("Specify TXT DNS record."), NULL },
  { "-z, --bind-interfaces", gettext_noop("Bind only to interfaces in use."), NULL },
  { "-Z, --read-ethers", gettext_noop("Read DHCP static host information from %s."), ETHERSFILE },
  { "-1, --enable-dbus", gettext_noop("Enable the DBus interface for setting upstream servers, etc."), NULL },
  { "-2, --no-dhcp-interface=interface", gettext_noop("Do not provide DHCP on this interface, only provide DNS."), NULL },
  { "-3, --bootp-dynamic", gettext_noop("Enable dynamic address allocation for bootp."), NULL },
  { "-4, --dhcp-mac=<id>,<mac address>", gettext_noop("Map MAC address (with wildcards) to option set."), NULL },
  { "-5, --no-ping", gettext_noop("Disable ICMP echo address checking in the DHCP server."), NULL },
  { "-6, --dhcp-script=path", gettext_noop("Script to run on DHCP lease creation and destruction."), NULL },
  { "-7, --conf-dir=path", gettext_noop("Read configuration from all the files in this directory."), NULL },
  { "-8, --log-facility=facilty", gettext_noop("Log to this syslog facility. (defaults to DAEMON)"), NULL },
  { "-9, --leasefile-ro", gettext_noop("Read leases at startup, but never write the lease file."), NULL },
  { "-0, --dns-forward-max=<queries>", gettext_noop("Maximum number of concurrent DNS queries. (defaults to %s)"), "!" }, 
  { NULL, NULL, NULL }
}; 

/* We hide metacharaters in quoted strings by mapping them into the ASCII control
   character space. Note that the \0, \t \a \b \r  and \n characters are carefully placed in the
   following sequence so that they map to themselves: it is therefore possible to call
   unhide_metas repeatedly on string without breaking things.
   The transformation gets undone by opt_canonicalise, atoi_check and safe_string_alloc, and a 
   couple of other places. */

static const char meta[] = "\000123456\a\b\t\n78\r90abcdefABCDEF:,.";

static void one_file(struct daemon *daemon, char *file, int nest);

static char hide_meta(char c)
{
  unsigned int i;

  for (i = 0; i < (sizeof(meta) - 1); i++)
    if (c == meta[i])
      return (char)i;
  
  return c;
}

static char unhide_meta(char cr)
{ 
  unsigned int c = cr;
  
  if (c < (sizeof(meta) - 1))
    cr = meta[c];
  
  return cr;
}

static void unhide_metas(char *cp)
{
  if (cp)
    for(; *cp; cp++)
      *cp = unhide_meta(*cp);
}

static char *safe_string_alloc(char *cp)
{
  char *ret = NULL;
  
  if (cp && strlen(cp) != 0)
    {
      ret = safe_malloc(strlen(cp)+1);
      strcpy(ret, cp); 
      
      /* restore hidden metachars */
      unhide_metas(ret);
    }
    
  return ret;
}

static char *safe_strchr(char *s, int c)
{
  if (!s) 
    return NULL;

  return strchr(s, c);
}

static int canonicalise_opt(char *s)
{
  if (!s)
    return 0;

  unhide_metas(s);
  return canonicalise(s);
}

static int atoi_check(char *a, int *res)
{
  char *p;

  if (!a)
    return 0;

  unhide_metas(a);
  
  for (p = a; *p; p++)
     if (*p < '0' || *p > '9')
       return 0;

  *res = atoi(a);
  return 1;
}

static void add_txt(struct daemon *daemon, char *name, char *txt)
{
  size_t len = strlen(txt);
  struct txt_record *r = safe_malloc(sizeof(struct txt_record));
  
  r->name = safe_string_alloc(name);
  r->next = daemon->txt;
  daemon->txt = r;
  r->class = C_CHAOS;
  r->txt = safe_malloc(len+1);
  r->len = len+1;
  *(r->txt) = len;
  memcpy((r->txt)+1, txt, len);
}

static void do_usage(void)
{
  char buff[100];
  int i;

  printf(_("Usage: dnsmasq [options]\n\n"));
#ifndef HAVE_GETOPT_LONG
  printf(_("Use short options only on the command line.\n"));
#endif
  printf(_("Valid options are :\n"));
  
  for (i = 0; usage[i].flag; i++)
    {
      if (usage[i].arg)
	{
	  if (strcmp(usage[i].arg, "$") == 0)
	    sprintf(buff, "%d", CACHESIZ);
	  else if (strcmp(usage[i].arg, "*") == 0)
	    sprintf(buff, "%d", EDNS_PKTSZ);
	  else if (strcmp(usage[i].arg, "&") == 0)
	    sprintf(buff, "%d", MAXLEASES);
	  else if (strcmp(usage[i].arg, "!") == 0)
	    sprintf(buff, "%d", FTABSIZ);
	  else 
	    strcpy(buff, usage[i].arg);
	}
      printf("%-36.36s", usage[i].flag);
      printf(_(usage[i].desc), buff);
      printf("\n");
    }
}

static char *one_opt(struct daemon *daemon, int option, char *arg, char *problem, int nest)
{      
  int i;
  char *comma;

  if(option == '?')
    return problem;

  for (i=0; optmap[i].c; i++)
    if (option == optmap[i].c)
      {
	daemon->options |= optmap[i].flag;
	return arg ? _("extraneous parameter") : NULL;
      }
  
  if (!arg)
    return _("missing parameter");
  
  switch (option)
    { 
    case 'C':
      {
	char *file = safe_string_alloc(arg);
	if (file)
	  one_file(daemon, file, nest);
	break;
      }

    case '7':	      
      {
	DIR *dir_stream;
	struct dirent *ent;
	char *directory, *path;
	
	if (!(directory = safe_string_alloc(arg)))
	  break;
	
	if (!(dir_stream = opendir(directory)))
	  die(_("cannot access directory %s: %s"), directory);
		
	while ((ent = readdir(dir_stream)))
	  {
	    size_t len;
	    struct stat buf;

	    if ((len = strlen(ent->d_name)) == 0)
	      continue;
	     /* ignore emacs backups and dotfiles */
	    if (ent->d_name[len - 1] == '~' ||
		(ent->d_name[0] == '#' && ent->d_name[len - 1] == '#') ||
		ent->d_name[0] == '.')
	      continue;
	    path = safe_malloc(strlen(directory) + len + 2);
	    strcpy(path, directory);
	    strcat(path, "/");
	    strcat(path, ent->d_name);
	    if (stat(path, &buf) == -1)
	      die(_("cannot access %s: %s"), path);
	    /* only reg files allowed. */
	    if (!S_ISREG(buf.st_mode))
	      continue;
	    
	    /* dir is one level, so files must be readable */
	    one_file(daemon, path, nest + 1);
	    free(path);
	  }
     
	closedir(dir_stream);
	break;
      }

    case '8':
      for (i = 0; facilitynames[i].c_name; i++)
	if (hostname_isequal((char *)facilitynames[i].c_name, arg))
	  break;
      
      if (facilitynames[i].c_name)
	daemon->log_fac = facilitynames[i].c_val;
      else
	{
	  option = '?';
	  problem = "bad log facility";
	}
      break;
      
    case 'x': 
      daemon->runfile = safe_string_alloc(arg);
      break;
      
    case 'r':
      {
	char *name = safe_string_alloc(arg);
	struct resolvc *new, *list = daemon->resolv_files;
	
	if (list && list->is_default)
	  {
	    /* replace default resolv file - possibly with nothing */
	    if (name)
	      {
		list->is_default = 0;
		list->name = name;
	      }
	    else
	      list = NULL;
	  }
	else if (name)
	  {
	    new = safe_malloc(sizeof(struct resolvc));
	    new->next = list;
	    new->name = name;
	    new->is_default = 0;
	    new->mtime = 0;
	    new->logged = 0;
	    list = new;
	  }
	daemon->resolv_files = list;
	break;
      }
      
    case 'm':
      {
	int pref = 1;
	struct mx_srv_record *new;
	
	if ((comma = safe_strchr(arg, ',')))
	  {
	    char *prefstr;
	    *(comma++) = 0;
	    if ((prefstr=strchr(comma, ',')))
	      {
		*(prefstr++) = 0;
		if (!atoi_check(prefstr, &pref))
		  {
		    option = '?';
		    problem = _("bad MX preference");
		    break;
		  }
	      }
	  }
	
	if (!canonicalise_opt(arg) || (comma && !canonicalise_opt(comma)))
	  {
	    option = '?';
	    problem = _("bad MX name");
	    break;
	  }
	
	new = safe_malloc(sizeof(struct mx_srv_record));
	new->next = daemon->mxnames;
	daemon->mxnames = new;
	new->issrv = 0;
	new->name = safe_string_alloc(arg);
	new->target = safe_string_alloc(comma); /* may be NULL */
	new->weight = pref;
	break;
      }
      
    case 't':
      if (!canonicalise_opt(arg))
	{
	  option = '?';
	  problem = _("bad MX target");
	}
      else
	daemon->mxtarget = safe_string_alloc(arg);
      break;
      
    case 'l':
      daemon->lease_file = safe_string_alloc(arg);
      break;
      
    case '6':
#ifdef NO_FORK
      problem = _("cannot run scripts under uClinux");
      option = '?';
#else
      daemon->lease_change_command = safe_string_alloc(arg);
#endif
      break;
      
    case 'H':
      {
	struct hostsfile *new = safe_malloc(sizeof(struct hostsfile));
	static int hosts_index = 1;
	new->fname = safe_string_alloc(arg);
	new->index = hosts_index++;
	new->next = daemon->addn_hosts;
	daemon->addn_hosts = new;
	break;
      }
      
    case 's':
      if (strcmp (arg, "#") == 0)
	daemon->options |= OPT_RESOLV_DOMAIN;
      else if (!canonicalise_opt(arg))
	option = '?';
      else
	daemon->domain_suffix = safe_string_alloc(arg);
      break;
      
    case 'u':
      daemon->username = safe_string_alloc(arg);
      break;
      
    case 'g':
      daemon->groupname = safe_string_alloc(arg);
      break;
      
    case 'i':
      do {
	struct iname *new = safe_malloc(sizeof(struct iname));
	if ((comma = safe_strchr(arg, ',')))
	  *comma++ = 0;
	new->next = daemon->if_names;
	daemon->if_names = new;
	/* new->name may be NULL if someone does
	   "interface=" to disable all interfaces except loop. */
	new->name = safe_string_alloc(arg);
	new->isloop = new->used = 0;
	arg = comma;
      } while (arg);
      break;
      
    case 'I':
    case '2':
      do {
	struct iname *new = safe_malloc(sizeof(struct iname));
	if ((comma = safe_strchr(arg, ',')))
	  *comma++ = 0;
	new->name = safe_string_alloc(arg);
	if (option == 'I')
	  {
	    new->next = daemon->if_except;
	    daemon->if_except = new;
	  }
	else
	  {
	    new->next = daemon->dhcp_except;
	    daemon->dhcp_except = new;
	  }
	arg = comma;
      } while (arg);
      break;
      
    case 'B':
      {
	struct in_addr addr;
	unhide_metas(arg);
	if (arg && (addr.s_addr = inet_addr(arg)) != (in_addr_t)-1)
	  {
	    struct bogus_addr *baddr = safe_malloc(sizeof(struct bogus_addr));
	    baddr->next = daemon->bogus_addr;
	    daemon->bogus_addr = baddr;
	    baddr->addr = addr;
	  }
	else
	  option = '?'; /* error */
	break;	
      }
      
    case 'a':
      do {
	struct iname *new = safe_malloc(sizeof(struct iname));
	if ((comma = safe_strchr(arg, ',')))
	  *comma++ = 0;
	unhide_metas(arg);
	new->next = daemon->if_addrs;
	if (arg && (new->addr.in.sin_addr.s_addr = inet_addr(arg)) != (in_addr_t)-1)
	  {
	    new->addr.sa.sa_family = AF_INET;
#ifdef HAVE_SOCKADDR_SA_LEN
	    new->addr.in.sin_len = sizeof(new->addr.in);
#endif
	  }
#ifdef HAVE_IPV6
	else if (arg && inet_pton(AF_INET6, arg, &new->addr.in6.sin6_addr) > 0)
	  {
	    new->addr.sa.sa_family = AF_INET6;
	    new->addr.in6.sin6_flowinfo = 0;
	    new->addr.in6.sin6_scope_id = 0;
#ifdef HAVE_SOCKADDR_SA_LEN
	    new->addr.in6.sin6_len = sizeof(new->addr.in6);
#endif
	  }
#endif
	else
	  {
	    option = '?'; /* error */
	    free(new);
	    break;
	  }
	
	daemon->if_addrs = new;
	arg = comma;
      } while (arg);
      break;
      
    case 'S':
    case 'A':
      {
	struct server *serv, *newlist = NULL;
	
	unhide_metas(arg);
	
	if (arg && *arg == '/')
	  {
	    char *end;
	    arg++;
	    while ((end = strchr(arg, '/')))
	      {
		char *domain = NULL;
		*end = 0;
		/* # matches everything and becomes a zero length domain string */
		if (strcmp(arg, "#") == 0)
		  domain = "";
		else if (!canonicalise_opt(arg) && strlen(arg) != 0)
		  option = '?';
		else
		  domain = safe_string_alloc(arg); /* NULL if strlen is zero */
		serv = safe_malloc(sizeof(struct server));
		serv->next = newlist;
		newlist = serv;
		serv->sfd = NULL;
		serv->domain = domain;
		serv->flags = domain ? SERV_HAS_DOMAIN : SERV_FOR_NODOTS;
		memset(&serv->addr, 0, sizeof(serv->addr));
		memset(&serv->source_addr, 0, sizeof(serv->source_addr));
		arg = end+1;
	      }
	    if (!newlist)
	      {
		option = '?';
		break;
	      }
	    
	  }
	else
	  {
	    newlist = safe_malloc(sizeof(struct server));
	    newlist->next = NULL;
	    newlist->flags = 0;
	    newlist->sfd = NULL;
	    newlist->domain = NULL;
	  }
	
	if (option == 'A')
	  {
	    newlist->flags |= SERV_LITERAL_ADDRESS;
	    if (!(newlist->flags & SERV_TYPE))
	      option = '?';
	  }
	
	if (!arg || !*arg)
	  {
	    newlist->flags |= SERV_NO_ADDR; /* no server */
	    if (newlist->flags & SERV_LITERAL_ADDRESS)
	      option = '?';
	  }
	else
	  {
	    int source_port = 0, serv_port = NAMESERVER_PORT;
	    char *portno, *source;
	    
	    if ((source = strchr(arg, '@'))) /* is there a source. */
	      {
		*source = 0; 
		if ((portno = strchr(source+1, '#')))
		  { 
		    *portno = 0;
		    if (!atoi_check(portno+1, &source_port))
		      {
			option = '?';
			problem = _("bad port");
		      }
		  }
	      }
	    
	    if ((portno = strchr(arg, '#'))) /* is there a port no. */
	      {
		*portno = 0;
		if (!atoi_check(portno+1, &serv_port))
		  {
		    option = '?';
		    problem = _("bad port");
		  }
	      }
	    
	    if ((newlist->addr.in.sin_addr.s_addr = inet_addr(arg)) != (in_addr_t) -1)
	      {
		newlist->addr.in.sin_port = htons(serv_port);	
		newlist->source_addr.in.sin_port = htons(source_port); 
		newlist->addr.sa.sa_family = newlist->source_addr.sa.sa_family = AF_INET;
#ifdef HAVE_SOCKADDR_SA_LEN
		newlist->source_addr.in.sin_len = newlist->addr.in.sin_len = sizeof(struct sockaddr_in);
#endif
		if (source)
		  {
		    if ((newlist->source_addr.in.sin_addr.s_addr = inet_addr(source+1)) != (in_addr_t) -1)
		      newlist->flags |= SERV_HAS_SOURCE;
		    else
		      option = '?'; /* error */
		  }
		else
		  newlist->source_addr.in.sin_addr.s_addr = INADDR_ANY;
		      }
#ifdef HAVE_IPV6
	    else if (inet_pton(AF_INET6, arg, &newlist->addr.in6.sin6_addr) > 0)
	      {
		newlist->addr.in6.sin6_port = htons(serv_port);
		newlist->source_addr.in6.sin6_port = htons(source_port);
		newlist->addr.sa.sa_family = newlist->source_addr.sa.sa_family = AF_INET6;
#ifdef HAVE_SOCKADDR_SA_LEN
		newlist->addr.in6.sin6_len = newlist->source_addr.in6.sin6_len = sizeof(newlist->addr.in6);
#endif
		if (source)
		  {
		    if (inet_pton(AF_INET6, source+1, &newlist->source_addr.in6.sin6_addr) > 0)
		      newlist->flags |= SERV_HAS_SOURCE;
		    else
		      option = '?'; /* error */
		  }
		else
		  newlist->source_addr.in6.sin6_addr = in6addr_any; 
	      }
#endif
	    else
	      option = '?'; /* error */
	    
	  }
	
	if (option == '?')
	  while (newlist)
	    { 
	      serv = newlist;
	      newlist = newlist->next;
	      free(serv);
	    }
	else
	  {
	    serv = newlist;
	    while (serv->next)
	      {
		serv->next->flags = serv->flags;
		serv->next->addr = serv->addr;
		serv->next->source_addr = serv->source_addr;
		serv = serv->next;
	      }
	    serv->next = daemon->servers;
	    daemon->servers = newlist;
	  }
	break;
      }
      
    case 'c':
      {
	int size;
	
	if (!atoi_check(arg, &size))
	  option = '?';
	else
	  {
	    /* zero is OK, and means no caching. */
	    
	    if (size < 0)
	      size = 0;
	    else if (size > 10000)
	      size = 10000;
	    
	    daemon->cachesize = size;
	  }
	break;
      }
      
    case 'p':
      if (!atoi_check(arg, &daemon->port))
	option = '?';
      break;
    
    case '0':
      if (!atoi_check(arg, &daemon->ftabsize))
	option = '?';
      break;  
    
    case 'P':
      {
	int i;
	if (!atoi_check(arg, &i))
	  option = '?';
	daemon->edns_pktsz = (unsigned short)i;	
	break;
      }
      
    case 'Q':
      if (!atoi_check(arg, &daemon->query_port))
	option = '?';
      break;
      
    case 'T':
      {
	int ttl;
	if (!atoi_check(arg, &ttl))
	  option = '?';
	else
	  daemon->local_ttl = (unsigned long)ttl;
	break;
      }
      
    case 'X':
      if (!atoi_check(arg, &daemon->dhcp_max))
	option = '?';
      break;
      
    case 'F':
      {
	int k, leasepos = 2;
	char *cp, *a[5] = { NULL, NULL, NULL, NULL, NULL };
	struct dhcp_context *new = safe_malloc(sizeof(struct dhcp_context));
	
	new->next = daemon->dhcp;
	new->lease_time = DEFLEASE;
	new->addr_epoch = 0;
	new->netmask.s_addr = 0;
	new->broadcast.s_addr = 0;
	new->router.s_addr = 0;
	new->netid.net = NULL;
	new->filter = NULL;
	new->flags = 0;
	
	problem = _("bad dhcp-range");
	
	if (!arg)
	  {
	    option = '?';
	    break;
	  }
	
	while(1)
	  {
	    for (cp = arg; *cp; cp++)
	      if (!(*cp == ' ' || *cp == '.' ||  (*cp >='0' && *cp <= '9')))
		break;
	    
	    if (*cp != ',' && (comma = strchr(arg, ',')))
	      {
		*comma = 0;
		if (strstr(arg, "net:") == arg)
		  {
		    struct dhcp_netid *tt = safe_malloc(sizeof (struct dhcp_netid));
		    tt->net = safe_string_alloc(arg+4);
		    tt->next = new->filter;
		    new->filter = tt;
		  }
		else
		  {
		    if (new->netid.net)
		      {
			option = '?';
			problem = _("only one netid tag allowed");
		      }
		    else
		      new->netid.net = safe_string_alloc(arg);
		  }
		arg = comma + 1;
	      }
	    else
	      {
		a[0] = arg;
		break;
	      }
	  }
	
	for (k = 1; k < 5; k++)
	  {
	    if (!(a[k] = strchr(a[k-1], ',')))
	      break;
	    *(a[k]++) = 0;
	  }
	
	if (option == '?' || (k < 2) || ((new->start.s_addr = inet_addr(a[0])) == (in_addr_t)-1))
	  option = '?';
	else if (strcmp(a[1], "static") == 0)
	  {
	    new->end = new->start;
	    new->flags |= CONTEXT_STATIC;
	  }
	else if ((new->end.s_addr = inet_addr(a[1])) == (in_addr_t)-1)
	  option = '?';
	
	if (ntohl(new->start.s_addr) > ntohl(new->end.s_addr))
	  {
	    struct in_addr tmp = new->start;
	    new->start = new->end;
	    new->end = tmp;
	  }
	
	if (option != '?' && k >= 3 && strchr(a[2], '.') &&  
	    ((new->netmask.s_addr = inet_addr(a[2])) != (in_addr_t)-1))
	  {
	    new->flags |= CONTEXT_NETMASK;
	    leasepos = 3;
	    if (!is_same_net(new->start, new->end, new->netmask))
	      {
		problem = _("inconsistent DHCP range");
		option = '?';
	      }
	  }
	
	if (option == '?')
	  {
	    free(new);
	    break;
	  }
	else
	  daemon->dhcp = new;
	
	if (k >= 4 && strchr(a[3], '.') &&  
	    ((new->broadcast.s_addr = inet_addr(a[3])) != (in_addr_t)-1))
	  {
	    new->flags |= CONTEXT_BRDCAST;
	    leasepos = 4;
	  }
	
	if (k >= leasepos+1)
	  {
	    if (strcmp(a[leasepos], "infinite") == 0)
	      new->lease_time = 0xffffffff;
	    else
	      {
		int fac = 1;
		if (strlen(a[leasepos]) > 0)
		  {
		    switch (a[leasepos][strlen(a[leasepos]) - 1])
		      {
		      case 'd':
		      case 'D':
			fac *= 24;
			/* fall though */
		      case 'h':
		      case 'H':
			fac *= 60;
			/* fall through */
		      case 'm':
		      case 'M':
			fac *= 60;
			/* fall through */
		      case 's':
		      case 'S':
				a[leasepos][strlen(a[leasepos]) - 1] = 0;
		      }
		    
		    new->lease_time = atoi(a[leasepos]) * fac;
		    /* Leases of a minute or less confuse
		       some clients, notably Apple's */
		    if (new->lease_time < 120)
		      new->lease_time = 120;
		  }
	      }
	  }
	break;
      }
      
    case 'G':
      {
	int j, k;
	char *a[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
	struct dhcp_config *new = safe_malloc(sizeof(struct dhcp_config));
	struct in_addr in;
	
	new->next = daemon->dhcp_conf;
	new->flags = 0;		  
	
	
	if ((a[0] = arg))
	  for (k = 1; k < 6; k++)
	    {
	      if (!(a[k] = strchr(a[k-1], ',')))
		break;
	      *(a[k]++) = 0;
	    }
	else
	  k = 0;
	
	for (j = 0; j < k; j++)
	  if (strchr(a[j], ':')) /* ethernet address, netid or binary CLID */
	    {
	      char *arg = a[j];
	      
	      if ((arg[0] == 'i' || arg[0] == 'I') &&
		  (arg[1] == 'd' || arg[1] == 'D') &&
		  arg[2] == ':')
		{
		  if (arg[3] == '*')
		    new->flags |= CONFIG_NOCLID;
		  else
		    {
		      int len;
		      arg += 3; /* dump id: */
		      if (strchr(arg, ':'))
			len = parse_hex(arg, (unsigned char *)arg, -1, NULL, NULL);
		      else
			len = (int) strlen(arg);
		      
		      new->flags |= CONFIG_CLID;
		      new->clid_len = len;
		      new->clid = safe_malloc(len);
		      memcpy(new->clid, arg, len);
		    }
		}
	      else if (strstr(arg, "net:") == arg)
		{
		  new->flags |= CONFIG_NETID;
		  new->netid.net = safe_string_alloc(arg+4);
		}
	      else 
		{
		  new->hwaddr_len = parse_hex(a[j],  new->hwaddr, DHCP_CHADDR_MAX, &new->wildcard_mask, &new->hwaddr_type);
		  new->flags |= CONFIG_HWADDR;
		}
	    }
	  else if (strchr(a[j], '.') && (in.s_addr = inet_addr(a[j])) != (in_addr_t)-1)
	    {
	      new->addr = in;
	      new->flags |= CONFIG_ADDR;
	    }
	  else
	    {
	      char *cp, *lastp = NULL, last = 0;
	      int fac = 1;
	      
	      if (strlen(a[j]) > 1)
		{
		  lastp = a[j] + strlen(a[j]) - 1;
		  last = *lastp;
		  switch (last)
		    {
		    case 'd':
		    case 'D':
		      fac *= 24;
		      /* fall through */
		    case 'h':
		    case 'H':
		      fac *= 60;
		      /* fall through */
		    case 'm':
		    case 'M':
		      fac *= 60;
		      /* fall through */
		    case 's':
		    case 'S':
		      *lastp = 0;
		    }
		}
	      
	      for (cp = a[j]; *cp; cp++)
		if (!isdigit(*cp) && *cp != ' ')
		  break;
	      
	      if (*cp)
		{
		  if (lastp)
		    *lastp = last;
		  if (strcmp(a[j], "infinite") == 0)
		    {
		      new->lease_time = 0xffffffff;
		      new->flags |= CONFIG_TIME;
		    }
		  else if (strcmp(a[j], "ignore") == 0)
		    new->flags |= CONFIG_DISABLE;
		  else
		    {
		      new->hostname = safe_string_alloc(a[j]);
		      new->flags |= CONFIG_NAME;
		    }
		}
	      else
		{
		  new->lease_time = atoi(a[j]) * fac; 
		  /* Leases of a minute or less confuse
		     some clients, notably Apple's */
		  if (new->lease_time < 120)
		    new->lease_time = 120;
		  new->flags |= CONFIG_TIME;
		}
	    }
	
	if (option == '?')
	  {
	    problem = _("bad dhcp-host");
	    if (new->flags & CONFIG_NAME)
	      free(new->hostname);
	    if (new->flags & CONFIG_CLID)
	      free(new->clid);
	    if (new->flags & CONFIG_NETID)
	      free(new->netid.net);
	    free(new);
	  }
	else
	  daemon->dhcp_conf = new;
	
	break;
      }
      
    case 'O':
      {
	struct dhcp_opt *new = safe_malloc(sizeof(struct dhcp_opt));
	char lenchar = 0, *cp;
	int addrs, digs, is_addr, is_hex, is_dec;
	
	new->len = 0;
	new->flags = 0;
	new->netid = NULL;
	new->val = NULL;
	new->vendor_class = NULL;
	
	if ((comma = safe_strchr(arg, ',')))
	  {
	    struct dhcp_netid *np = NULL;
	    *comma++ = 0;
	    
	    do {
	      for (cp = arg; *cp; cp++)
		if (!(*cp == ' ' || (*cp >='0' && *cp <= '9')))
		  break;
	      if (!*cp)
		break;
	      
	      if (strstr(arg, "vendor:") == arg)
		new->vendor_class = (unsigned char *)safe_string_alloc(arg+7);
	      else
		{
		  new->netid = safe_malloc(sizeof (struct dhcp_netid));
		  /* allow optional "net:" for consistency */
		  if (strstr(arg, "net:") == arg)
		    new->netid->net = safe_string_alloc(arg+4);
		  else
		    new->netid->net = safe_string_alloc(arg);
		  new->netid->next = np;
		  np = new->netid;
		}
	      arg = comma;
	      if ((comma = safe_strchr(arg, ',')))
		*comma++ = 0;
	    } while (arg);
	  }
	
	if (!arg || (new->opt = atoi(arg)) == 0)
	  {
	    option = '?';
	    problem = _("bad dhcp-option");
	  }
	else if (comma && new->opt == 119 && !new->vendor_class)
	  {
	    /* dns search, RFC 3397 */
	    unsigned char *q, *r, *tail;
	    unsigned char *p = NULL;
	    size_t newlen, len = 0;
		    
	    arg = comma;
	    if ((comma = safe_strchr(arg, ',')))
	      *(comma++) = 0;
	    
	    while (arg && *arg)
	      {
		if (!canonicalise_opt(arg))
		  {
		    option = '?';
		    problem = _("bad domain in dhcp-option");
		    break;
		  }
		
		if (!(p = realloc(p, len + strlen(arg) + 2)))
		  die(_("could not get memory"), NULL);
		q = p + len;
		
		/* add string on the end in RFC1035 format */
		while (*arg) 
		  {
		    unsigned char *cp = q++;
		    int j;
		    for (j = 0; *arg && (*arg != '.'); arg++, j++)
		      *q++ = *arg;
		    *cp = j;
		    if (*arg)
		      arg++;
		  }
		*q++ = 0;
		
		/* Now tail-compress using earlier names. */
		newlen = q - p;
		for (tail = p + len; *tail; tail += (*tail) + 1)
		  for (r = p; r - p < (int)len; r += (*r) + 1)
		    if (strcmp((char *)r, (char *)tail) == 0)
		      {
			PUTSHORT((r - p) | 0xc000, tail); 
			newlen = tail - p;
			goto end;
		      }
	      end:
		len = newlen;
		
		arg = comma;
		if ((comma = safe_strchr(arg, ',')))
		  *(comma++) = 0;
	      }
	    
	    new->len = (int) len;
	    new->val = p;
	  }
	else if (comma)
	  {
	    /* not option 119 */
	    /* characterise the value */
	    is_addr = is_hex = is_dec = 1;
	    addrs = digs = 1;
	    for (cp = comma; *cp; cp++)
	      if (*cp == ',')
		{
		  addrs++;
		  is_dec = is_hex = 0;
		}
	      else if (*cp == ':')
		{
		  digs++;
		  is_dec = is_addr = 0;
		}
	      else if (*cp == '.' || *cp == '/')
		is_dec = is_hex = 0;
	      else if (!((*cp >='0' && *cp <= '9') || *cp == '-'))
		{
		  is_addr = 0;
		  if (cp[1] == 0 && is_dec &&
		      (*cp == 'b' || *cp == 's' || *cp == 'i'))
		    {
		      lenchar = *cp;
		      *cp = 0;
		    }
		  else
		    is_dec = 0;
		  if (!((*cp >='A' && *cp <= 'F') ||
			(*cp >='a' && *cp <= 'f')))
		    is_hex = 0;
		}
	    
	    if (is_hex && digs > 1)
	      {
		new->len = digs;
		new->val = safe_malloc(new->len);
		parse_hex(comma, new->val, digs, NULL, NULL);
	      }
	    else if (is_dec)
	      {
		int i, val = atoi(comma);
		/* assume numeric arg is 1 byte except for
		   options where it is known otherwise.
		   For vendor class option, we have to hack. */
		new->len = 1;
		if (lenchar == 'b')
		  new->len = 1;
		else if (lenchar == 's')
		  new->len = 2;
		else if (lenchar == 'i')
		  new->len = 4;
		else if (new->vendor_class)
		  {
		    if (val & 0xffff0000)
		      new->len = 4;
		    else if (val & 0xff00)
		      new->len = 2;
		  } 
		else
		  switch (new->opt)
		    {
		    case 13: case 22: case 25: case 26: 
		      new->len = 2;
		      break;
		    case 2: case 24: case 35: case 38: 
		      new->len = 4;
		      break;
		    }
		new->val = safe_malloc(new->len);
		for (i=0; i<new->len; i++)
		  new->val[i] = val>>((new->len - i - 1)*8);
	      }
	    else if (is_addr)	
	      {
		struct in_addr in;
		unsigned char *op;
		char *slash;
		/* max length of address/subnet descriptor is five bytes */
		new->val = op = safe_malloc(5 * addrs);
		if (!new->vendor_class)
		  new->flags |= DHOPT_ADDR;
		while (addrs--) 
		  {
		    cp = comma;
		    if ((comma = strchr(cp, ',')))
		      *comma++ = 0;
		    if ((slash = strchr(cp, '/')))
		      *slash++ = 0;
		    in.s_addr = inet_addr(cp);
		    if (!slash)
		      {
			memcpy(op, &in, INADDRSZ);
			op += INADDRSZ;
		      }
		    else
		      {
			unsigned char *p = (unsigned char *)&in;
			int netsize = atoi(slash);
			*op++ = netsize;
			if (netsize > 0)
			  *op++ = *p++;
			if (netsize > 8)
			  *op++ = *p++;
			if (netsize > 16)
			  *op++ = *p++;
			if (netsize > 24)
			  *op++ = *p++;
			new->flags &= ~DHOPT_ADDR; /* cannot re-write descriptor format */
		      } 
		  }
		new->len = op - new->val;
	      }
	    else
	      {
		/* text arg */
		new->len = strlen(comma);
		/* keep terminating zero on string */
		new->val = (unsigned char *)safe_string_alloc(comma);
		new->flags |= DHOPT_STRING;
	      }
	  }
	
	if (new->len > 255)
	  {
	    option = '?';
	    problem = _("dhcp-option too long");
	  }
	
	if (option == '?')
	  {
	    if (new->netid)
	      free(new->netid);
	    if (new->val)
	      free(new->val);
	    if (new->vendor_class)
	      free(new->vendor_class);
	    free(new);
	  }
	else if (new->vendor_class)
	  {
	    new->next = daemon->vendor_opts;
	    daemon->vendor_opts = new;
	  }
	else
	  {
	    new->next = daemon->dhcp_opts;
	    daemon->dhcp_opts = new;
	  }
		break;
      }
      
    case 'M':
      {
	struct dhcp_netid *id = NULL;
	while (arg && strstr(arg, "net:") == arg)
	  {
	    struct dhcp_netid *newid = safe_malloc(sizeof(struct dhcp_netid));
	    newid->next = id;
	    id = newid;
	    if ((comma = strchr(arg, ',')))
	      *comma++ = 0;
	    newid->net = safe_string_alloc(arg+4);
	    arg = comma;
	  };
	
	if (!arg)
	  option = '?';
	else 
	  {
	    char *dhcp_file, *dhcp_sname = NULL;
	    struct in_addr dhcp_next_server;
	    if ((comma = strchr(arg, ',')))
	      *comma++ = 0;
	    dhcp_file = safe_string_alloc(arg);
	    dhcp_next_server.s_addr = 0;
	    if (comma)
	      {
		arg = comma;
		if ((comma = strchr(arg, ',')))
		  *comma++ = 0;
		dhcp_sname = safe_string_alloc(arg);
		if (comma)
		  {
		    unhide_metas(comma);
		    if ((dhcp_next_server.s_addr = inet_addr(comma)) == (in_addr_t)-1)
		      option = '?';
		  }
	      }
	    if (option != '?')
	      {
		struct dhcp_boot *new = safe_malloc(sizeof(struct dhcp_boot));
		new->file = dhcp_file;
		new->sname = dhcp_sname;
		new->next_server = dhcp_next_server;
		new->netid = id;
		new->next = daemon->boot_config;
		daemon->boot_config = new;
	      }
	  }
	
	if (option == '?')
	  {
	    struct dhcp_netid *tmp;
	    for (; id; id = tmp)
	      {
		tmp = id->next;
		free(id);
	      }
	  }
	break;
      }
      
    case '4':
      {
	if (!(comma = safe_strchr(arg, ',')))
	  option = '?';
	else
	  {
	    struct dhcp_mac *new = safe_malloc(sizeof(struct dhcp_mac));
	    *comma = 0;
	    new->netid.net = safe_string_alloc(arg);
	    unhide_metas(comma+1);
	    new->hwaddr_len = parse_hex(comma+1, new->hwaddr, DHCP_CHADDR_MAX, &new->mask, &new->hwaddr_type);
	    new->next = daemon->dhcp_macs;
	    daemon->dhcp_macs = new;
	  }
      }
      break;
      
    case 'U':
    case 'j':
      {
	if (!(comma = safe_strchr(arg, ',')))
	  option = '?';
	else
	  {
	    struct dhcp_vendor *new = safe_malloc(sizeof(struct dhcp_vendor));
	    *comma = 0;
	    new->netid.net = safe_string_alloc(arg);
	    unhide_metas(comma+1);
	    new->len = strlen(comma+1);
	    new->data = safe_malloc(new->len);
	    memcpy(new->data, comma+1, new->len);
	    new->is_vendor = (option == 'U');
	    new->next = daemon->dhcp_vendors;
	    daemon->dhcp_vendors = new;
	  }
	break;
      }
      
    case 'J':
      {
	struct dhcp_netid_list *new = safe_malloc(sizeof(struct dhcp_netid_list));
	struct dhcp_netid *list = NULL;
	new->next = daemon->dhcp_ignore;
	daemon->dhcp_ignore = new;
	do {
	  struct dhcp_netid *member = safe_malloc(sizeof(struct dhcp_netid));
	  if ((comma = safe_strchr(arg, ',')))
	    *comma++ = 0;
	  member->next = list;
	  list = member;
	  member->net = safe_string_alloc(arg);
	  arg = comma;
	} while (arg);
	
	new->list = list;
	break;
      }
      
    case 'V':
      {
	char *a[3] = { NULL, NULL, NULL };
	int k;
	struct in_addr in, out, mask;
	struct doctor *new;
	
	mask.s_addr = 0xffffffff;
	
	if ((a[0] = arg))
	  for (k = 1; k < 3; k++)
	    {
	      if (!(a[k] = strchr(a[k-1], ',')))
		break;
	      *(a[k]++) = 0;
	      unhide_metas(a[k]);
	    }
	else
	  k = 0;
	
	if ((k < 2) || 
	    ((in.s_addr = inet_addr(a[0])) == (in_addr_t)-1) ||
	    ((out.s_addr = inet_addr(a[1])) == (in_addr_t)-1))
	  {
	    option = '?';
	    break;
	  }
	
	if (k == 3)
	  mask.s_addr = inet_addr(a[2]);
	
	new = safe_malloc(sizeof(struct doctor));
	new->in = in;
	new->out = out;
	new->mask = mask;
	new->next = daemon->doctors;
	daemon->doctors = new;
	
	break;
      }
      
    case 'Y':
      {
	struct txt_record *new;
	unsigned char *p, *q;
	
	if ((comma = safe_strchr(arg, ',')))
	  *(comma) = 0;
	
	if (!canonicalise_opt(arg))
	  {
	    option = '?';
	    problem = _("bad TXT record");
	    break;
	  }
	
	if ((q = (unsigned char *)comma))
	  while (1)
	    {
	      size_t len;
	      if ((p = (unsigned char *)strchr((char*)q+1, ',')))
		{
		  if ((len = p - q - 1) > 255)
		    { 
		      option = '?';
		      break;
		    }
		  *q = len;
		  for (q = q+1; q < p; q++)
		    *q = unhide_meta(*q);
		}
	      else
		{
		  if ((len = strlen((char *)q+1)) > 255)
		    option = '?';
		  *q = len;
		  for (q = q+1; *q; q++)
		    *q = unhide_meta(*q);
		  break;
		}
	    }
	
	if (option == '?')
	  {
	    problem = _("TXT record string too long");
	    break;
	  }
	
	new = safe_malloc(sizeof(struct txt_record));
	new->next = daemon->txt;
	daemon->txt = new;
	new->class = C_IN;
	if (comma)
	  {
	    new->len = q - ((unsigned char *)comma);
	    new->txt = safe_malloc(new->len);
	    memcpy(new->txt, comma, new->len);
	  }
	else
	  {
	    static char empty[] = "";
	    new->len = 1;
	    new->txt = empty;
	  }
	
	if (comma)
	  *comma = 0;
	new->name = safe_string_alloc(arg);
	break;
      }
      
    case 'W':
      {
	int port = 1, priority = 0, weight = 0;
	char *name, *target = NULL;
	struct mx_srv_record *new;
	
	if ((comma = safe_strchr(arg, ',')))
	  *(comma++) = 0;
	
	if (!canonicalise_opt(arg))
	  {
	    option = '?';
	    problem = _("bad SRV record");
	    break;
	  }
	name = safe_string_alloc(arg);
	
	if (comma)
	  {
	    arg = comma;
	    if ((comma = strchr(arg, ',')))
	      *(comma++) = 0;
	    if (!canonicalise_opt(arg))
	      {
		option = '?';
		problem = _("bad SRV target");
		break;
	      }
	    target = safe_string_alloc(arg);
	    if (comma)
	      {
		arg = comma;
		if ((comma = strchr(arg, ',')))
		  *(comma++) = 0;
		if (!atoi_check(arg, &port))
		  {
		    option = '?';
		    problem = _("invalid port number");
		    break;
		  }
		if (comma)
		  {
		    arg = comma;
		    if ((comma = strchr(arg, ',')))
		      *(comma++) = 0;
		    if (!atoi_check(arg, &priority))
		      {
			option = '?';
			problem = _("invalid priority");
			break;
		      }
		    if (comma)
		      {
			arg = comma;
			if ((comma = strchr(arg, ',')))
			  *(comma++) = 0;
			if (!atoi_check(arg, &weight))
			  {
			    option = '?';
			    problem = _("invalid weight");
			    break;
			  }
		      }
		  }
	      }
	  }
	
	new = safe_malloc(sizeof(struct mx_srv_record));
	new->next = daemon->mxnames;
	daemon->mxnames = new;
	new->issrv = 1;
	new->name = name;
	new->target = target;
	new->srvport = port;
	new->priority = priority;
	new->weight = weight;
	break;
      }
    }

  return option == '?' ? problem : NULL;
}

static void one_file(struct daemon *daemon, char *file, int nest)	
{
  int i, option, lineno = 0; 
  FILE *f;
  char *p, *arg, *buff = daemon->namebuff;

  if (nest > 20)
    die(_("files nested too deep in %s"), file);
  
  if (!(f = fopen(file, "r")))
    {   
      if (errno == ENOENT && nest == 0)
	return; /* No conffile, all done. */
      else
	die(_("cannot read %s: %s"), file);
    } 
  
  while (fgets(buff, MAXDNAME, f))
    {
      int white;
      unsigned int lastquote;
      
      lineno++;
            
      /* Implement quotes, inside quotes we allow \\ \" \n and \t 
	 metacharacters get hidden also strip comments */
      
      for (white = 1, lastquote = 0, p = buff; *p; p++)
	{
	  if (*p == '"')
	    {
	      memmove(p, p+1, strlen(p+1)+1);
	      for(; *p && *p != '"'; p++)
		{
		  if (*p == '\\' && strchr("\"tnabr\\", p[1]))
		    {
		      if (p[1] == 't')
			p[1] = '\t';
		      else if (p[1] == 'n')
			p[1] = '\n';
		      else if (p[1] == 'a')
			p[1] = '\a';
		      else if (p[1] == 'b')
			p[1] = '\b';
		      else if (p[1] == 'r')
			p[1] = '\r';
		      memmove(p, p+1, strlen(p+1)+1);
		    }
		  *p = hide_meta(*p);
		}
	      if (*p == '"') 
		{
		  memmove(p, p+1, strlen(p+1)+1);
		  lastquote = p - buff;
		}
	      else
		complain(_("missing \""), lineno, file);
	    }
	  
	  if (white && *p == '#')
	    { 
	      *p = 0;
	      break;
	    }
	  white = isspace(unhide_meta(*p)); 
	}

      /* fgets gets end of line char too. */
      while (strlen(buff) > lastquote && isspace(unhide_meta(buff[strlen(buff)-1])))
	buff[strlen(buff)-1] = 0;

      if (*buff == 0)
	continue; 

      if ((p=strchr(buff, '=')))
	{
	  /* allow spaces around "=" */
	  for (arg = p+1; isspace(*arg); arg++);
	  for (; p >= buff && (isspace(*p) || *p == '='); p--)
	    *p = 0;
	}
      else
	arg = NULL;
      
      for (option = 0, i = 0; opts[i].name; i++) 
	if (strcmp(opts[i].name, buff) == 0)
	  option = opts[i].val;

      if (option)
	{
	  char *errmess;
	  if ((errmess = one_opt(daemon, option, arg, _("error"), nest + 1)))
	    complain(errmess, lineno, file);
	}
      else
	complain(_("bad option"), lineno, file);
    }

  fclose(f);
}

struct daemon *read_opts(int argc, char **argv, char *compile_opts)
{
  struct daemon *daemon = safe_malloc(sizeof(struct daemon));
  char *buff = safe_malloc(MAXDNAME);
  int option, nest = 0;
  char *errmess, *arg, *conffile = CONFFILE;
      
  opterr = 0;
  
  memset(daemon, 0, sizeof(struct daemon));
  daemon->namebuff = buff;

  /* Set defaults - everything else is zero or NULL */
  daemon->cachesize = CACHESIZ;
  daemon->ftabsize = FTABSIZ;
  daemon->port = NAMESERVER_PORT;
  daemon->default_resolv.is_default = 1;
  daemon->default_resolv.name = RESOLVFILE;
  daemon->resolv_files = &daemon->default_resolv;
  daemon->username = CHUSER;
  daemon->groupname = CHGRP;
  daemon->runfile =  RUNFILE;
  daemon->dhcp_max = MAXLEASES;
  daemon->edns_pktsz = EDNS_PKTSZ;
  daemon->log_fac = -1;
  add_txt(daemon, "version.bind", "dnsmasq-" VERSION );
  add_txt(daemon, "authors.bind", "Simon Kelley");
  add_txt(daemon, "copyright.bind", COPYRIGHT);

  while (1) 
    {
#ifdef HAVE_GETOPT_LONG
      option = getopt_long(argc, argv, OPTSTRING, opts, NULL);
#else
      option = getopt(argc, argv, OPTSTRING);
#endif
      
      if (option == -1)
	break;
      
      /* Copy optarg so that argv doesn't get changed */
      if (optarg)
	{
	  strncpy(buff, optarg, MAXDNAME);
	  buff[MAXDNAME-1] = 0;
	  arg = buff;
	}
      else
	arg = NULL;
      
      /* command-line only stuff */
      if (option == 'w')
	{
	  do_usage();
	  exit(0);
	}
      else if (option == 'v')
	{
	  printf(_("Dnsmasq version %s  %s\n"), VERSION, COPYRIGHT);
	  printf(_("Compile time options %s\n\n"), compile_opts); 
	  printf(_("This software comes with ABSOLUTELY NO WARRANTY.\n"));
	  printf(_("Dnsmasq is free software, and you are welcome to redistribute it\n"));
	  printf(_("under the terms of the GNU General Public License, version 2.\n"));
          exit(0);
        }
      else if (option == 'C')
	{
	  conffile = safe_string_alloc(arg);
	  nest++;
	}
      else
	{
#ifdef HAVE_GETOPT_LONG
	  errmess = one_opt(daemon, option, arg, _("try --help"), 0);
#else 
	  errmess = one_opt(daemon, option, arg, _("try -w"), 0); 
#endif  
	  if (errmess)
	    die(_("bad command line options: %s"), errmess);
	}
    }

  if (conffile)
    one_file(daemon, conffile, nest);

  /* Do old default, if nothing set for this. */
  if (daemon->log_fac == -1)
    {
      daemon->log_fac = LOG_DAEMON;
#ifdef LOG_LOCAL0
      if (daemon->options & OPT_DEBUG)
	daemon->log_fac = LOG_LOCAL0;
#endif
    }

  /* port might no be known when the address is parsed - fill in here */
  if (daemon->servers)
    {
      struct server *tmp;
      for (tmp = daemon->servers; tmp; tmp = tmp->next)
	if (!(tmp->flags & SERV_HAS_SOURCE))
	  {
	    if (tmp->source_addr.sa.sa_family == AF_INET)
	      tmp->source_addr.in.sin_port = htons(daemon->query_port);
#ifdef HAVE_IPV6
	    else if (tmp->source_addr.sa.sa_family == AF_INET6)
	      tmp->source_addr.in6.sin6_port = htons(daemon->query_port);
#endif  
	  }
    }
  
  if (daemon->if_addrs)
    {  
      struct iname *tmp;
      for(tmp = daemon->if_addrs; tmp; tmp = tmp->next)
	if (tmp->addr.sa.sa_family == AF_INET)
	  tmp->addr.in.sin_port = htons(daemon->port);
#ifdef HAVE_IPV6
	else if (tmp->addr.sa.sa_family == AF_INET6)
	  tmp->addr.in6.sin6_port = htons(daemon->port);
#endif /* IPv6 */
    }
		      
  /* only one of these need be specified: the other defaults to the host-name */
  if ((daemon->options & OPT_LOCALMX) || daemon->mxnames || daemon->mxtarget)
    {
      struct mx_srv_record *mx;
      
      if (gethostname(buff, MAXDNAME) == -1)
	die(_("cannot get host-name: %s"), NULL);
      
      for (mx = daemon->mxnames; mx; mx = mx->next)
	if (!mx->issrv && hostname_isequal(mx->name, buff))
	  break;
      
      if ((daemon->mxtarget || (daemon->options & OPT_LOCALMX)) && !mx)
	{
	  mx = safe_malloc(sizeof(struct mx_srv_record));
	  mx->next = daemon->mxnames;
	  mx->issrv = 0;
	  mx->target = NULL;
	  mx->name = safe_string_alloc(buff);
	  daemon->mxnames = mx;
	}
      
      if (!daemon->mxtarget)
	daemon->mxtarget = safe_string_alloc(buff);

      for (mx = daemon->mxnames; mx; mx = mx->next)
	if (!mx->issrv && !mx->target)
	  mx->target = daemon->mxtarget;
    }

  if (!(daemon->options & OPT_NO_RESOLV) &&
      daemon->resolv_files && 
      daemon->resolv_files->next && 
      (daemon->options & OPT_NO_POLL))
    die(_("only one resolv.conf file allowed in no-poll mode."), NULL);
  
  if (daemon->options & OPT_RESOLV_DOMAIN)
    {
      char *line;
      FILE *f;

      if ((daemon->options & OPT_NO_RESOLV) ||
	  !daemon->resolv_files || 
	  (daemon->resolv_files)->next)
	die(_("must have exactly one resolv.conf to read domain from."), NULL);
      
      if (!(f = fopen((daemon->resolv_files)->name, "r")))
	die(_("failed to read %s: %s"), (daemon->resolv_files)->name);
      
      while ((line = fgets(buff, MAXDNAME, f)))
	{
	  char *token = strtok(line, " \t\n\r");
	  
	  if (!token || strcmp(token, "search") != 0)
	    continue;
	  
	  if ((token = strtok(NULL, " \t\n\r")) &&  
	      canonicalise_opt(token) &&
	      (daemon->domain_suffix = safe_string_alloc(token)))
	    break;
	}

      fclose(f);

      if (!daemon->domain_suffix)
	die(_("no search directive found in %s"), (daemon->resolv_files)->name);
    }

  if (daemon->domain_suffix)
    {
       /* add domain for any srv record without one. */
      struct mx_srv_record *srv;
      
      for (srv = daemon->mxnames; srv; srv = srv->next)
	if (srv->issrv &&
	    strchr(srv->name, '.') && 
	    strchr(srv->name, '.') == strrchr(srv->name, '.'))
	  {
	    strcpy(buff, srv->name);
	    strcat(buff, ".");
	    strcat(buff, daemon->domain_suffix);
	    free(srv->name);
	    srv->name = safe_string_alloc(buff);
	  }
    }
     
  return daemon;
}  
     
      

