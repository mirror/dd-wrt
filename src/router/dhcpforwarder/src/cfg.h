// Copyright (C) 2002, 2003, 2004, 2008, 2014
//               Enrico Scholz <enrico.scholz@ensc.de>
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


struct DHCPSubOption {
    uint8_t			code;  //< suboption-id
    size_t			len;   //< suboption length

    /** payload Points to a member of 'val' or to the 'aid' field of
     *	interface. During parsing (when memory can be realloc()ed later), it can
     *	be NULL which will be fixed later to point to 'val' */
    void const			*data;

    /** scratch buffer; 'data' will point to this */
    union {
	    in_addr_t		ip;
	    char		*str;
	    uint8_t		test[4];
    }				val;
};

struct DHCPSuboptionsList {
    struct DHCPSubOption	*dta; //< array of DHCPSuboptionsList
    size_t			len;
};

struct InterfaceInfo {
    char	name[IFNAMSIZ];	//< name of the interface
    char	aid[IFNAMSIZ];	//< agent id
    bool	has_clients;	//< tells if DHCP clients are on this interface
    bool	has_servers;	//< tells if DHCP servers are on this interface
    uint16_t	port_server;	/** port on which for client requests will be
				    listened.  Value will be stored in network
				    byte order. Interesting both for has_clients
				    and for has_servers case. */
    uint16_t	port_client;	/** port from which requests to servers will be
				    sended out.  Value will be stored in network
				    byte order. Interesting for has_servers case
				    only. */
    bool	allow_bcast;	//< honor bcast-flag and send to bcast address?

    bool	need_mac;	//< tells if MAC will be required

    unsigned int	if_idx;		//< interface index
    in_addr_t		if_real_ip;	//< ip got by recvmsg()
    in_addr_t		if_ip;		//< ip to be set in 'giaddr' field
    size_t		if_mtu;		//< MTU
    uint8_t		if_mac[16];	//< MAC
    size_t		if_maclen;	//< length of MAC

    int			sender_fd;	//< the sender-filedescriptor

    struct DHCPSuboptionsList	suboptions; //< dhcp-relay suboptions
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
    enum { dmFORK, dmFG, dmSTOP }	daemon_mode;
    bool				do_bindall;

    unsigned long			compat_hacks;
};

extern unsigned long	g_compat_hacks;
enum {
  COMPAT_HACK_CLIENT_ADDRESSING	=  0
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
