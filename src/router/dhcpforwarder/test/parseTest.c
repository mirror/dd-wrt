// Copyright (C) 2004, 2008
//               Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <pwd.h>

#include <parser.h>
#include <cfg.h>
#include <output.h>

#define ensc_DHCP_FORWARDER_ULIMIT_H_I_KNOW_WHAT_I_DO
#include "ulimit_codes.h"

inline static void
showUInt(char const varname[], unsigned int value)
{
  write(1, varname, strlen(varname));
  write(1, "=", 1);
  writeUInt(1, value);
  write(1, "\n", 1);
}

inline static void
showString(char const varname[], char const value[])
{
  write(1, varname, strlen(varname));
  write(1, "='", 2);
  write(1, value, strlen(value));
  write(1, "'\n", 2);
}

inline static void
showRlimit(struct rlimit const *val)
{
  if (val->rlim_cur==RLIM_INFINITY) write(1, "INF", 3);
  else                              writeUInt(1, val->rlim_cur);

  write(1, ", ", 2);

  if (val->rlim_max==RLIM_INFINITY) write(1, "INF", 3);
  else                              writeUInt(1, val->rlim_max);
}

int main(int argc, char const *argv[])
{
  struct ConfigInfo		cfg;
  size_t			i;
  struct passwd *		pw;

  if (argc!=2) exit(1);

  memset(&cfg, 0, sizeof cfg);
  parse(argv[1], &cfg);

  // HACK: workaround different uids/gids for 'bin'
  if ((pw=getpwnam("bin")) &&
      pw->pw_uid!=1 && pw->pw_uid==cfg.uid)
    cfg.uid = 1;

  if (pw!=0 &&
      pw->pw_gid!=1 && pw->pw_gid==cfg.gid)
    cfg.gid = 1;

  showUInt("compathacks", cfg.compat_hacks);
  showUInt("uid", cfg.uid);
  showUInt("gid", cfg.gid);
  showString("chroot_path", cfg.chroot_path);
  showString("logfile_name", cfg.logfile_name);
  showString("pidfile_name", cfg.pidfile_name);
  showUInt("loglevel", cfg.loglevel);

  write(1, "ulimits={len=", 9);
  writeUInt(1, cfg.ulimits.len);
  write(1, ", data={", 8);
  for (i=0; i<cfg.ulimits.len; ++i) {
    size_t		j;
    char const		*name = 0;

    write(1, "\n  ", 3);
    for (j=0; name==0 && j<sizeof(ULIMIT_CODES)/sizeof(ULIMIT_CODES[0]); ++j) {
      if (ULIMIT_CODES[j].code==cfg.ulimits.dta[i].code)
	name=ULIMIT_CODES[j].name;
    }

    if (name==0) {
      write(1, "#", 1);
      writeUInt(1, cfg.ulimits.dta[i].code);
    }
    else {
      size_t	len = strlen(name);
      write(1, name,  len);
      if (len<10) write(1, "           ", 10-len);
    }

    write(1, "-> (", 4);
    showRlimit(&cfg.ulimits.dta[i].rlim);
    write(1, ")", 1);
  }
  write(1, "}}\n", 3);

  write(1, "interfaces={len=", 16);
  writeUInt(1, cfg.interfaces.len);
  write(1, ", data={", 8);
  for (i=0; i<cfg.interfaces.len; ++i) {
    struct InterfaceInfo	*iface = &cfg.interfaces.dta[i];
    char			*aux;
    struct in_addr		in;

    write(1, "\n  '", 4);
    write(1, iface->name, strlen(iface->name));
    write(1, "', '", 4);
    write(1, iface->aid, strlen(iface->aid));
    write(1, "', ", 3);
    writeUInt(1, iface->has_clients);
    write(1, ", ", 2);
    writeUInt(1, iface->has_servers);
    write(1, ", ", 2);
    writeUInt(1, iface->allow_bcast);
    write(1, ", ", 2);
    writeUInt(1, ntohs(iface->port_client));
    write(1, ", ", 2);
    writeUInt(1, ntohs(iface->port_server));
    write(1, ", ", 2);

    in.s_addr = iface->if_ip;
    aux = inet_ntoa(in);
    if (aux==0)  write(1, "<null>", 6);
    else         write(1, aux, strlen(aux));
  }
  write(1, "}}\n", 3);

  write(1, "servers={len=", 13);
  writeUInt(1, cfg.servers.len);
  write(1, ", data={", 8);
  for (i=0; i<cfg.servers.len; ++i) {
    struct ServerInfo	*svr = &cfg.servers.dta[i];
    char			*aux;

    write(1, "\n  ", 3);
    switch (svr->type) {
      case svUNICAST	:
	write(1, "UNICAST, ", 9);
	aux = inet_ntoa(svr->info.unicast.ip);
	if (aux==0)  write(1, "<null>", 6);
	else         write(1, aux, strlen(aux));

	if (svr->iface) {
	  write(1, ", &", 3);
	  write(1, svr->iface->name, strlen(svr->iface->name));
	}
	break;
      case svBCAST	:
	write(1, "BROADCAST, ", 11);
	write(1, "&", 1);
	write(1, svr->iface->name, strlen(svr->iface->name));
	break;
      default		:  write(1, "???", 3); abort();
    }
  }
  write(1, "}}\n", 3);

  return 0;
}
