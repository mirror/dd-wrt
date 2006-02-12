// $Id: cfg.h,v 1.10 2004/06/16 10:48:02 ensc Exp $    --*- c++ -*--

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

#ifndef H_DHCP_FORWARDER_SRC_CONFIG_H
#define H_DHCP_FORWARDER_SRC_CONFIG_H

#include "splint.h"

#include <net/if.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/param.h>
#include <sys/resource.h>

#include "util.h"
#include "compat.h"

struct InterfaceInfo {
    char	name[IFNAMSIZ];	//< name of the interface
    char	aid[IFNAMSIZ];	//< agent id
    bool	has_clients;	//< tells if DHCP clients are on this interface
    bool	has_servers;	//< tells if DHCP servers are on this interface
    bool	allow_bcast;	//< honor bcast-flag and send to bcast address?

    bool	need_mac;	//< tells if MAC will be required

    unsigned int	if_idx;		//< interface index
    in_addr_t		if_real_ip;	//< ip got by recvmsg()
    in_addr_t		if_ip;		//< ip to be set in 'giaddr' field
    size_t		if_mtu;		//< MTU
    uint8_t		if_mac[16];	//< MAC
    size_t		if_maclen;	//< length of MAC

    int			sender_fd;	//< the sender-filedescriptor
};

struct InterfaceInfoList {
    /*@only@*//*@null@*/
    struct InterfaceInfo		*dta;	//< array of InterfaceInfo
    size_t				len;	//< length of InterfaceInfoList
};

  /*@-export@*/
typedef enum { svUNICAST, svBCAST }	ServerInfoType;	//< type of server
  /*@=export@*/

struct ServerInfo {
    ServerInfoType			type;
      /*@observer@*/
    struct InterfaceInfo *		iface;
    
    union {
	struct {
	    struct in_addr		ip;
	    int				fd;
	}				unicast;
    }					info;
};

struct ServerInfoList {
    /*@only@*//*@null@*/
    struct ServerInfo			*dta;
    size_t				len;
};


struct FdInfo {
    int					fd;
    /*@observer@*/
    struct InterfaceInfo const		*iface;
};

struct FdInfoList {
    /*@only@*//*@null@*/
    struct FdInfo			*dta;
    size_t				len;

    int					raw_fd;
};


struct UlimitInfo {
    int					code;
    struct rlimit			rlim;
};

struct UlimitInfoList {
      /*@only@*//*@null@*/
    struct UlimitInfo			*dta;
    size_t				len;
};

struct ConfigInfo {
    uid_t				uid;
    gid_t				gid;
    char				chroot_path[PATH_MAX];
    struct UlimitInfoList		ulimits;
    
    char				logfile_name[PATH_MAX];
    int					loglevel;

    char				pidfile_name[PATH_MAX];

    struct InterfaceInfoList		interfaces;
    struct ServerInfoList		servers;

      // Commandline options
    /*@observer@*/
    char const *			conffile_name;
    bool				do_fork;
    bool				do_bindall;
};

  /*@-superuser@*/
extern int	initializeSystem(int argc, /*@in@*/char *argv[],
				 /*@out@*/struct InterfaceInfoList *	ifs,
				 /*@out@*/struct ServerInfoList *	servers,
				 /*@out@*/struct FdInfoList *		fds)
  /*@warn superuser "Only super-user processes may call initializeSystem()"@*/
  /*@globals errno, fileSystem, internalState@*/
  /*@modifies *ifs, *servers, *fds, fileSystem, errno, internalState@*/
  /*@requires (maxRead(argv)+1)==argc
           /\ maxSet(ifs)==0
	   /\ maxSet(servers)==0
	   /\ maxRead(fds)==0@*/  ;
  /*@=superuser@*/
    
#endif	// H_DHCP_FORWARDER_SRC_CONFIG_H

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
