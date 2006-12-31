/* brpvc.c -- client for managing RFC2684 bridge-encapsulation VCs.

   written by Chuck Musser <chuckie@well.com>
*/

#include <stdio.h>             /* printf, read, write, etc.  */
#include <stdlib.h>            /* malloc, exit, etc.         */
#include <unistd.h>            /* getopt, atoi               */
#include <errno.h>             /* system errors              */
#include <sys/un.h>            /* unix domain sockets        */
#include <sys/types.h>         /* u_int32_t, etc.            */
#include <pwd.h>               /* getpwuid, etc.             */
// brcm
#include "atm.h"               /* general ATM stuff          */
#include "atmbr2684.h"         /* ATM bridging structs       */

#include "brpvc.h"

// brcm
#define OFFSET 100000

extern char *optarg;
extern int optind, opterr, optopt;

void clean_exit (char *str)
{
  if(errno)
     perror(str);
  else
    printf(str);
  exit(-1);
}

int unix_connect(void)
{
  int fd;
  struct be_msg msg;
  struct sockaddr_un addr;
  
  if( (fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    clean_exit("Can't create client socket");

  bzero(&addr, sizeof(addr));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, BRPVC_SOCKPATH, sizeof(addr.sun_path) -1);

  if(connect(fd, (struct sockaddr *) &addr, sizeof(addr)))
    clean_exit("Can't connect to server");
  
  bzero(&msg, sizeof(msg));
  msg.msgtype = HELLO;
  
  if( send(fd, &msg, sizeof(msg), 0) < 0 )
    clean_exit("Can't send HELLO message");

  if( recv(fd, &msg, sizeof(msg), 0) <= 0 )
    clean_exit("Can't receive receive OK response from server");
  return fd;
}


void usage(int exitval)
{
  printf("usage:  bctl -a -v <[intf].vpi.vci> -i <nas #> [-c count] [-g group name]\n");
  printf("        bctl -d -v <[intf].vpi.vci> -i <nas #> [-c count] [-g group name]\n");
  printf("        bctl -d -d group name\n");
  printf("        bctl -l [-d group name]\n");
  exit(exitval);
}



#ifdef BUILD_STATIC
int pvc2684ctl_main (int argc, char **argv)
#else
int main (int argc, char **argv)
#endif
{
  int optchar, action = 0, count = 1, starting_nas_idx = -1, 
    t2a_err, srv_fd, nbytes, header = 0;
  char *endptr;
  char name[MAX_GROUPNAME_LEN] = {};
  struct be_msg smsg, rmsg;
  struct be_memstat rstatmsg;
  struct sockaddr_atmpvc vc;
  struct passwd *pw;
  // brcm
  int encap=BR2684_ENCAPS_LLC, proto_filter=0, mode=0, extif=0;
  unsigned short vlan_id=-1;
  
  bzero(&vc, sizeof(vc));

  while(1) {
    optchar = getopt(argc, argv, "Madli:v:c:g:erft:x:");
    //optchar = getopt(argc, argv, "Madli:v:c:g:erft:");
    if (optchar == -1)
       break;

    switch(optchar) {

    case 'M': 
    
    srv_fd = unix_connect();
    smsg.msgtype = MEM_STATS;

    if( send(srv_fd, &smsg, sizeof(smsg), 0) < 0 )
      clean_exit("Can't send MEM_STAT message");

    nbytes = recv(srv_fd, &rstatmsg, sizeof(rstatmsg), 0);
    if(nbytes == 0 )
      clean_exit("connection closed waiting for MEM_STAT response\n");
    else if(nbytes < 0)
      clean_exit("Can't receive MEM_STAT response");
    
    printf("VC mallocs: %d\nVC frees: %d\nGroup mallocs: %d\nGroup frees:%d\n",
	   rstatmsg.vc_mallocs,rstatmsg.vc_frees,
	   rstatmsg.group_mallocs,rstatmsg.group_frees);
    exit(0);
    break;

    case 'a':
    case 'd':
    case 'l':
      action = optchar;
      break;

    case 'i':
// brcm
/*
      starting_nas_idx = strtol(optarg, &endptr, 10);
      if(*endptr || starting_nas_idx < 0) {
	printf("Invalid interface index specified\n");
	exit(1);
      }
*/
      break;
      
    case 'v':
      if((t2a_err = (text2atm(optarg, (struct sockaddr *)&vc, sizeof(vc), T2A_PVC)))) {
	printf ("can't parse \"%s\".\n", optarg);
	exit(1);
      }
// brcm
      {
        int retval, num[3];
        retval = sscanf(optarg, "%d.%d.%d", num, num+1, num+2);
        starting_nas_idx = OFFSET*num[1]+num[2];
      }
      break;
      
    case 'c': 
      count = strtol(optarg, &endptr, 10);
      if(*endptr || count <= 0) {
 	printf("Invalid count specified\n");
 	exit(1);
      }
      break;

// brcm
    case 'e':
      encap=BR2684_ENCAPS_VC;
      break;

// brcm
    case 'f':
      proto_filter = FILTER_PPPOE;
      break;
      
// brcm
    case 'r':
      mode=1;
      break;
// brcm  add vlan id flag
    case 't':
      vlan_id = atoi(optarg);
      break;
// brcm
    case 'x':
      extif= atoi(optarg);
      break;

    case 'g':
      strncpy(name,optarg,MAX_GROUPNAME_LEN);

    case '?':

      break;

    default:
      printf ("getopt returned char %c\n", optchar);
    }
  }

  int hasname = 31;
  for(; hasname>0; hasname--) { 
    if(name[hasname] != '\0') break;
  }

  switch(action) {

  case 'a':
  case 'd':

    if(vc.sap_family == AF_ATMPVC) {

      if(starting_nas_idx < 0) {

	/* If you specify a PVC, you must specify an interface too  */
 	printf("No interface specified. Use the -i option.\n");
	usage(1);
      }

    } else if(action == 'a') {

      /* If this is an ADD, you need to specify the PVC.  */
      printf("No VC specified. Use the -v option.\n");
      usage(1);

    } else if (!hasname) {

      /* If it's not an ADD, it must be a DELETE. You need at least a name.  */
      printf("No VC or VC group name specified. Use the -v or -g options.\n");
      usage(1);

    } else {

      /* If we're here, no VC was specified, it's a DELETE and a group 
	 name was specified. Send a DELETE_GROUP message and exit.
      */

      bzero(&rmsg, sizeof(rmsg));
      srv_fd = unix_connect();
      smsg.msgtype = DELETE_GROUP;
      strncpy(smsg.name,name,MAX_GROUPNAME_LEN);

      if( send(srv_fd, &smsg, sizeof(smsg), 0) < 0 )
	clean_exit("Can't send DELETE_GROUP message");

      nbytes = recv(srv_fd, &rmsg, sizeof(rmsg), 0);
      if(nbytes == 0 )
	clean_exit("connection closed waiting for DELETE_GROUP response\n");
      else if(nbytes < 0)
      clean_exit("Can't receive response to DELETE_GROUP message");

      if(rmsg.msgtype == GROUP_NOT_FOUND)
	printf("group \"%s\" doesn't exist\n", name);

	exit(0);
    }

    smsg.msgtype = (action == 'a' ? ADD : DELETE);
    smsg.nas_idx = starting_nas_idx;
    smsg.pvc     = vc;
    strncpy(smsg.name,name,MAX_GROUPNAME_LEN);
    // brcm
    smsg.encap   = encap;
    smsg.proto_filter   = proto_filter;
    smsg.mode   = mode;
    smsg.vlan_id = vlan_id;
    smsg.extif = extif;

    bzero(&rmsg, sizeof(rmsg));
    srv_fd = unix_connect();
    
    do {

      if( send(srv_fd, &smsg, sizeof(smsg), 0) < 0 )
	clean_exit("Can't send ADD or DELETE message");

      nbytes = recv(srv_fd, &rmsg, sizeof(rmsg), 0);
      if(nbytes == 0 )
	clean_exit("connection closed waiting for ADD or DELETE response\n");
      else if(nbytes < 0)
      clean_exit("Can't receive response to ADD or DELETE message");
	
      switch(rmsg.msgtype) {

      case VC_NOT_FOUND:

	printf("No VC %d/%d on nas%d\n", 
	       smsg.pvc.sap_addr.vpi, 
	       smsg.pvc.sap_addr.vci, 
	       smsg.nas_idx);
	break;
	
      case NOT_OWNER:

	pw = getpwuid(rmsg.uid);
	printf("Can't delete VC %d/%d on nas%d. owner is %s\n", 
	       smsg.pvc.sap_addr.vpi, 
	       smsg.pvc.sap_addr.vci, 
	       smsg.nas_idx,
	       pw->pw_name);
	break;

      case SOCK_FAILED: 

	printf("Interface nas%d already in use\n", smsg.nas_idx);
	break;

      case INTERFACE_FAILED: 

	printf("Interface nas%d couldn't be created\n", smsg.nas_idx);
	break;

      case NOMEM:

	printf("Server can't allocate memory\n");
	break;

      case OK:
	
	break;
	
      default:

	printf("Weird error: %d\n", rmsg.msgtype);
      }
      
      smsg.nas_idx++; 
      smsg.pvc.sap_addr.vci++;

    } while(--count > 0);

    break;

  case 'l':

    srv_fd = unix_connect();
    smsg.msgtype = (strlen(name) ? LIST_GROUP : LIST_ALL );
    strncpy(smsg.name,name,MAX_GROUPNAME_LEN);

    if( send(srv_fd, &smsg, sizeof(smsg), 0) < 0 )
      clean_exit("Can't send LIST_GROUP or LIST_ALL message");


    do {

      nbytes = recv(srv_fd, &rmsg, sizeof(rmsg), 0);
      if(nbytes == 0 )
	clean_exit("connection closed waiting for LIST_GROUP or LIST_ALL response\n");
      else if(nbytes < 0)
      clean_exit("Can't receive response to LIST_GROUP or LIST_ALL message");

      if(rmsg.msgtype == GROUP_NOT_FOUND) {
	if(strlen(name))
	  printf("Group %s not found\n", name);
	else
	  printf("No VCs defined\n");
	break;
      } else if(!header) {
// brcm    
	printf("\nVC          interface  mode             vlan_id\n");
	printf("--          ---------  ------           ----------\n");
//	printf("\nVC          interface  group                            owner\n");
//	printf("--          ---------  ------                           -----\n");
	header = 1;
      }

      if(rmsg.msgtype == OK) {
	printf("%2d/%4d     nas%-6d  %-16s %d (X%02X)\n", 
	       rmsg.pvc.sap_addr.vpi,
	       rmsg.pvc.sap_addr.vci,
	       rmsg.nas_idx,
	       rmsg.mode? "Routing":	"Bridging",
               rmsg.vlan_id, rmsg.vlan_id
		);
      }
    } while(rmsg.msgtype != LIST_END);

    printf("\n");

    break;

  default:
    printf("No command specified. Use -a, -d, or -l\n");
  }
  exit(0);
}
