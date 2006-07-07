// $Id: cfg.c,v 1.19 2004/06/22 10:46:56 ensc Exp $    --*- c++ -*--

// Copyright (C) 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "splint.h"
#include "cfg.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <sys/param.h>
#include <net/if_arp.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "output.h"
#include "parser.h"
#include "wrappers.h"
#include "inet.h"


/*@noreturn@*/ static void
exitFatal(char const msg[], register size_t len) __attribute__ ((noreturn))
  /*:requires maxRead(msg)+1 >= len@*/
  /*@*/ ;

  /*@noreturn@*/
static void scEXITFATAL(/*@in@*//*@sef@*/char const *msg) /*@*/; 
#define scEXITFATAL(msg)	exitFatal(msg, sizeof(msg)-1)


inline static void
exitFatal(char const msg[], register size_t len)
{
  (void)write(2, msg, len);
  (void)write(2, "\n", 1);

  exit(2);
}


inline static in_addr_t
sockaddrToInet4(/*@in@*//*@sef@*/struct sockaddr const *addr)
    /*@*/
{

  if (/*@-type@*/addr->sa_family!=AF_INET/*@=type@*/)
    scEXITFATAL("Interface has not IPv4 address");

  return (reinterpret_cast(struct sockaddr_in const *)(addr))->sin_addr.s_addr;
}

inline static void
initClientFD(struct FdInfo *fd,
	     /*@in@*//*@observer@*/struct InterfaceInfo const *iface)
    /*@globals internalState, fileSystem@*/
    /*@modifies internalState, fileSystem, *fd@*/
{
  struct sockaddr_in	s;
  int const		ON = 1;

  assert(fd!=0 && iface!=0);
  
  fd->iface = iface;
  fd->fd    = Esocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  Esetsockopt(fd->fd, SOL_IP,     IP_PKTINFO,      &ON, sizeof ON);
  Esetsockopt(fd->fd, SOL_SOCKET, SO_BROADCAST,    &ON, sizeof ON);
  Esetsockopt(fd->fd, SOL_SOCKET, SO_BINDTODEVICE, iface->name, strlen(iface->name)+1);

  memset(&s, 0, sizeof(s));
  
    /*@-type@*/
  s.sin_family      = AF_INET; /*@=type@*/
  s.sin_port        = htons(DHCP_PORT_SERVER);
  s.sin_addr.s_addr = htonl(INADDR_ANY);

  Ebind(fd->fd, &s);
}

inline static void
initRawFD(/*@out@*/int *fd)
    /*@globals internalState@*/
    /*@modifies internalState, *fd@*/
    /*@requires maxSet(fd)==0@*/
{
  assert(fd!=0);

  *fd = Esocket(AF_PACKET, SOCK_RAW, 0xFFFF);
}

inline static int
initSenderFD(struct InterfaceInfo const *iface)
    /*@globals internalState, fileSystem@*/
    /*@modifies internalState, fileSystem@*/
{
  struct sockaddr_in	s;
  int			fd;
  int const		ON = 1;

  fd = Esocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // used for sending only...
  Esetsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &ON, sizeof ON);
  if (iface)
    Esetsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
		iface->name, strlen(iface->name)+1);

  memset(&s, 0, sizeof(s));
  
    /*@-type@*/
  s.sin_family      = AF_INET; /*@=type@*/
  s.sin_port        = htons(DHCP_PORT_CLIENT);
  s.sin_addr.s_addr = htonl(INADDR_ANY);

  Ebind(fd, &s);
  return fd;
}

inline static void
sockaddrToHwAddr(/*@in@*/struct sockaddr const	*addr,
		 /*@out@*/uint8_t		mac[],
		 /*@out@*/size_t		*len)
    /*@modifies *mac, *len@*/
    /*@requires maxSet(len)==1 /\ maxSet(mac)>=ETH_ALEN @*/
    /*@ensures  maxRead(mac)==(*len)@*/
{
  assert(addr!=0 && len!=0);

  switch (addr->sa_family) {
    case ARPHRD_EETHER	:
    case ARPHRD_IEEE802	:
    case ARPHRD_ETHER	:  *len = ETH_ALEN; break;
    default		:  scEXITFATAL("Unsupported hardware-type"); 
  }

  assert(*len <= ETH_ALEN);
  memcpy(mac, addr->sa_data, *len);
}

inline static void
fillInterfaceInfo(struct InterfaceInfoList *ifs)
    /*@globals fileSystem, internalState@*/
    /*@modifies ifs->dta, fileSystem, internalState@*/
    /*@requires maxRead(ifs->dta)==ifs->len /\ maxRead(ifs)==0@*/
{
  int			fd = Esocket(AF_INET, SOCK_DGRAM, 0);
  size_t		i;

  assert(ifs->len==0 || ifs->dta!=0);

  for (i=0; i<ifs->len; ++i) {
    struct ifreq		iface;
    struct InterfaceInfo	*ifinfo;

    assert(ifs->dta!=0);
    ifinfo = &ifs->dta[i];

    memcpy(iface.ifr_name, ifinfo->name, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFINDEX,  &iface)==-1) goto err;

    if (iface.ifr_ifindex<0)                  goto err;
    ifinfo->if_idx = (unsigned int)(iface.ifr_ifindex);

    if (ioctl(fd, SIOCGIFADDR,   &iface)==-1) goto err;
    ifinfo->if_real_ip = sockaddrToInet4(&iface.ifr_addr);

    if (ioctl(fd, SIOCGIFMTU,    &iface)==-1) goto err;
    ifinfo->if_mtu = static_cast(size_t)(iface.ifr_mtu);

    if (!ifinfo->need_mac) ifinfo->if_maclen = 0;
    else {
      if (ioctl(fd, SIOCGIFHWADDR, &iface)==-1) goto err;
      sockaddrToHwAddr(&iface.ifr_hwaddr,
		       ifinfo->if_mac, &ifinfo->if_maclen);
    }

    if (ifinfo->if_ip==INADDR_NONE)
      ifinfo->if_ip = ifinfo->if_real_ip;
  }
  
  Eclose(fd);

  return;
  err:
  perror("ioctl()");
  scEXITFATAL("Can not get interface information");
}

static char const /*@null@*//*@observer@*/ *
getSenderIfaceName(struct InterfaceInfoList const * const ifs,
		   bool                    		  do_it)
{
  struct InterfaceInfo const *	res = 0;
  struct InterfaceInfo const *	ptr = (ifs->len==0 || ifs->dta==0) ? 0 : ifs->dta + ifs->len;
  
  if (!do_it) return 0;

  while (ptr>ifs->dta) {
    assert(ptr!=0);
    --ptr;
    if (ptr->has_servers) {
      if (res) return 0;	// there are more than one sender...
      else     res = ptr;
    }
  }

  if (res) return res->name;
  else     return 0;
}

inline static void
initFDs(/*@out@*/struct FdInfoList 		*fds,
	/*@in@*/struct ConfigInfo const	* const	cfg)
    /*@globals internalState, fileSystem@*/
    /*@modifies internalState, fileSystem, *fds, cfg->servers@*/
    /*@requires maxRead(fds)>=0 /\ maxSet(fds)>=0 /\ maxSet(fds->sender_fd)>=0@*/
{
  size_t					i, idx;
  struct InterfaceInfoList const * const	ifs = &cfg->interfaces;
  struct ServerInfo *				servers;
  int						bind_all_fd = -1;

  assert(cfg->servers.dta==0 || cfg->servers.len!=0);
  
  for (servers = cfg->servers.dta;
       /*@-nullptrarith@*/servers < cfg->servers.dta + cfg->servers.len/*@=nullptrarith@*/;
       ++servers) {
    assert(servers!=0);
    switch (servers->type) {
      case svUNICAST	:
	if (bind_all_fd!=-1 && servers->iface!=0)
	  scEXITFATAL("There are mixed 'server ip ...' declarations; please use either only such ones with or such ones without an interface");
	else if (bind_all_fd!=-1)
	  servers->info.unicast.fd = bind_all_fd;
	else if (servers->iface!=0 && servers->iface->sender_fd!=-1)
	  servers->info.unicast.fd = servers->iface->sender_fd;
	else {
	  servers->info.unicast.fd = initSenderFD(servers->iface);

	  if (servers->iface==0)
	    bind_all_fd               = servers->info.unicast.fd;
	  else
	    servers->iface->sender_fd = servers->info.unicast.fd;
	}

	break;
      default		:
	break;
    }
  }

  initRawFD(&fds->raw_fd);
  
  fds->len = ifs->len;
  fds->dta = static_cast(struct FdInfo*)(Emalloc(fds->len *
						 (sizeof(*fds->dta))));

  assert(fds->dta!=0 || fds->len==0);
  assert(ifs->dta!=0 || ifs->len==0);
  
  for (idx=0, i=0; i<ifs->len; ++i, ++idx) {
    assert(ifs->dta!=0);
    assert(fds->dta!=0);
    
    initClientFD(&fds->dta[idx], &ifs->dta[i]);
  }
}

inline static void
getConfig(/*@in@*/char const				*filename,
	  /*@partial@*//*@dependent@*/struct ConfigInfo	*cfg)
    /*@globals internalState, fileSystem@*/
    /*@modifies *cfg, internalState, fileSystem@*/
    /*@requires maxRead(cfg)>=0
             /\ PATH_MAX >= 1
	     /\ maxRead(cfg->conffile_name)>=1
             /\ (maxSet(cfg->chroot_path)+1)  == PATH_MAX
	     /\ (maxSet(cfg->logfile_name)+1) == PATH_MAX
	     /\ (maxSet(cfg->pidfile_name)+1) == PATH_MAX@*/
    /*@ensures  maxRead(cfg->chroot_path)>=0
             /\ maxRead(cfg->logfile_name)>=0
	     /\ maxRead(cfg->pidfile_name)>=0
	     /\ maxRead(cfg->servers.dta)>=0
	     /\ maxRead(cfg->interfaces.dta)>=0@*/
{
  cfg->interfaces.dta = 0;
  cfg->interfaces.len = 0;

  cfg->servers.dta    = 0;
  cfg->servers.len    = 0;

  cfg->ulimits.dta    = 0;
  cfg->ulimits.len    = 0;

  cfg->uid            = 99;
  cfg->gid            = 99;

  cfg->chroot_path[0]  = '\0';
  cfg->logfile_name[0] = '\0';
  cfg->loglevel        = 0;

  cfg->pidfile_name[0] = '\0';
  
  parse(filename, cfg);
    /*@-boundsread@*/
  fillInterfaceInfo(&cfg->interfaces);
    /*@=boundsread@*/
}

  /*@maynotreturn@*/
inline static void
showVersion() /*@*/
{
  (void)write(1, PACKAGE_STRING, strlen(PACKAGE_STRING));
  (void)write(1, "\n", 1);
}

  /*@maynotreturn@*/
inline static void
showHelp(/*@in@*//*@nullterminated@*/char const *cmd) /*@*/
{
/*  char const	msg[] = (" [-v] [-h] [-c <filename>] [-n] [-d]\n\n"
			 "  -v              show version\n"
			 "  -h              show help\n"
			 "  -c <filename>   read configuration from <filename>\n"
			 "  -n              do not fork separate process\n"
			 "  -d              debug-mode; same as '-n'\n\n"
			 "Report bugs to Enrico Scholz <"
			 PACKAGE_BUGREPORT
			 ">\n");

  showVersion();
  (void)write(1, "\nusage: ", 8	);
  (void)write(1, cmd, strlen(cmd));
  (void)write(1, msg, strlen(msg));*/
}

inline static void
limitResources(/*@in@*/struct UlimitInfoList const *limits)
    /*@globals internalState@*/
    /*@modifies internalState@*/
    /*@requires (maxRead(limits->dta)+1) == limits->len@*/
{
  size_t			i;

  assert(limits->len==0 || limits->dta!=0);
  
  for (i=0; i<limits->len; ++i) {
    assert(limits->dta!=0);
    Esetrlimit(limits->dta[i].code, &limits->dta[i].rlim);
  }
}

inline static void
freeLimitList(struct UlimitInfoList *limits)
    /*@modifies limits->dta, limits->len@*/
    /*@requires only   limits->dta@*/
    /*@ensures  isnull limits->dta@*/
{
  /*@-nullpass@*/free(limits->dta);/*@=nullpass@*/
  limits->dta=0;
  limits->len=0;
}

inline static pid_t
initializeDaemon(/*@in@*/struct ConfigInfo const *cfg)
    /*@globals  fileSystem, internalState@*/
    /*@modifies fileSystem, internalState@*/
{
  assert(cfg!=0);
  
  if (cfg->do_fork) (void)Esetsid();

  Eclose(1);
      
  if (cfg->chroot_path[0]!='\0') {
    Echdir (cfg->chroot_path);
      /*@-superuser@*/
    Echroot(cfg->chroot_path);
      /*@=superuser@*/
  }
  
  Esetgroups(1, &cfg->gid);
  Esetgid(cfg->gid);
  Esetuid(cfg->uid);

  limitResources(&cfg->ulimits);

  if (cfg->do_fork) return 0;
  else              return getpid();
}

inline static void
parseCommandline(int argc, char *argv[],
		 /*@out@*/struct ConfigInfo *	cfg)
    /*@modifies cfg@*/
{
  assert(cfg!=0);

  while (true) {
    int c = getopt(argc, argv, "vhdnc:-");
    if (c==-1) break;

    switch (c) {
      case 'v'	:  showVersion();     exit(0);
      case 'h'	:  showHelp(argv[0]); exit(0);
      case 'd'	:
      case 'n'	:  cfg->do_fork       = false;  break;
      case 'c'	:  cfg->conffile_name = optarg; break;
      case '-'	:
	if (strcmp(argv[optind], "--version")==0) { showVersion();     exit(0); }
	if (strcmp(argv[optind], "--help")==0)    { showHelp(argv[0]); exit(0); }
	  /*@fallthrough@*/
      default	:
	scEXITFATAL("Use '-h' to get help about possible options");
    }
  }
  
  if (argv[optind]!=0) scEXITFATAL("No extra-args allowed; use '-h' to get help.");
}

  /*@-superuser@*/
int
initializeSystem(int argc, char *argv[],
		 struct InterfaceInfoList *	ifs,
		 struct ServerInfoList *	servers,
		 struct FdInfoList *		fds)
{
  struct ConfigInfo		cfg;
  pid_t				pid, pidfile_pid;
  int				pidfile_fd;

  cfg.conffile_name = CFG_FILENAME;
  cfg.do_fork       = true;
  cfg.do_bindall    = false;

  parseCommandline(argc, argv, &cfg);

  getConfig(cfg.conffile_name, &cfg);
  initFDs(fds, &cfg);

  pidfile_fd = Eopen(cfg.pidfile_name, O_WRONLY|O_CREAT, 0444);
  openMsgfile(cfg.logfile_name);
  
  *ifs     = cfg.interfaces;
  *servers = cfg.servers;

  Eclose(0);

  if (cfg.do_fork) pid = fork();
  else             pid = 0;

  pidfile_pid = 0;

  switch (pid) {
    case 0	:
	/*@-usereleased@*/
      pidfile_pid = initializeDaemon(&cfg);
	/*@=usereleased@*/
      break;
      
    case -1	:  perror("fork()");  break;
    default	:  pidfile_pid = pid; break;
  }

  if (pidfile_pid!=0) {
    writeUInt(pidfile_fd, pidfile_pid);
    (void)write(pidfile_fd, "\n", 1);
  }
	  
  freeLimitList(&cfg.ulimits);

    /* It is too late to handle an error here. So just ignore it... */
  (void)close(pidfile_fd);
  return pid;
}
  /*@=superuser@*/

  // Local Variables:
  // compile-command: "make -k -C .."
  // fill-column: 80
  // End:
