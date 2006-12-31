/* brpvcd.c -- daemon for managing RFC2684 bridge-encapsulation VCs

   written by Chuck Musser <chuckie@well.com>
   based on br2684ctl, by Marcel GAL <cell@sch.bme.hu>
*/

#include <stdio.h>             /* printf, read, write, etc.  */
#include <stdlib.h>            /* malloc, exit, etc.         */
#include <stdarg.h>            /* var args support.          */
#include <unistd.h>            /* close, unlink, etc.        */
#include <syslog.h>            /* System logging interface   */
#include <errno.h>             /* system errors              */
#include <sys/un.h>            /* unix domain sockets        */
#include <sys/stat.h>          /* stat, chmod                */
#include <sys/types.h>         /* u_int32_t, etc.            */
#include <sys/ioctl.h>         /* ioctl, etc.                */
// brcm
#include "atm.h"               /* general ATM stuff          */
#include "linux/atmdev.h"      /* ATM device ioctls          */   
#include "atmbr2684.h"   /* ATM bridging structs       */
#include "atmrt2684.h"   /* ATM bridging structs       */

#include "brpvc.h"

// brcm
#define OFFSET 100000

/* Set g_use_bcm_atmapi to 1 to make a br2684 PVC use the Broadcom defined ATM
 * API.  Set g_use_bcm_atmapi to 0 to make a br2684 PVC use the Linux defined
 * ATM API.
 */
#ifdef BCM_ATM_BONDING_ETH
int g_use_bcm_atmapi = 0;
#else
int g_use_bcm_atmapi = 1;
#endif

struct be_group group_head;
struct be_memstat memstat = {0,0,0,0};

void do_error (int priority, const char *fmt, ...)
{
  va_list args;
  char buf[80];

  va_start(args,fmt);
  vsnprintf(buf,sizeof(buf),fmt,args);
  syslog(priority,buf);
  va_end(args);
  
  if(priority == LOG_ERR) {
    unlink(BRPVC_SOCKPATH);
    exit(-1);
  }
}

// brcm
void setIndexName(char * str, const char * hdr, int index) {
    int num1, num2;
    
    num1 = index/OFFSET;
    num2 = index - OFFSET*num1;
    sprintf(str, "%s_%d_%d", hdr, num1, num2);
}

// brcm
int create_br(int nas_idx, int mode, int extif)
//int create_br(int nas_idx)
{
  int sock, result = 0;
  struct atm_newif_br2684 ni;
  // brcm
  struct atm_newif_rt2684 ni_rt;
  int ret = 0;

  if((sock = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5)) < 0)

    do_error(LOG_WARNING, "socket creation failed for nas%d: %s",nas_idx,strerror(errno));

  else {
    
    /* Create the the bridge-encapsulation interface.
     */
    // brcm
    if (!mode) {
	if (extif) {
	    ni.backend_num = (g_use_bcm_atmapi == 1)
		? ATM_BACKEND_BR2684_BCM : ATM_BACKEND_BR2684;
	    ni.media       = extif;
	    ni.mtu         = 1500;
	    // brcm
	    setIndexName(ni.ifname, "nas", nas_idx);
	    sprintf(ni.ifname, "%s", ni.ifname);
	    ret=ioctl (sock, ATM_EXTBACKENDIF, &ni);
	}
	else {
	    ni.backend_num = (g_use_bcm_atmapi == 1)
		? ATM_BACKEND_BR2684_BCM : ATM_BACKEND_BR2684;
	    ni.media       = BR2684_MEDIA_ETHERNET;
	    ni.mtu         = 1500;
	    // brcm
	    setIndexName(ni.ifname, "nas", nas_idx);
	    //sprintf(ni.ifname, "nas_%d", nas_idx);
	    ret=ioctl (sock, ATM_NEWBACKENDIF, &ni);
	}
    } else {
        ni_rt.backend_num = ATM_BACKEND_RT2684;
	// brcm
        setIndexName(ni_rt.ifname, "ipa", nas_idx);
        //sprintf(ni_rt.ifname, "atm_%d", nas_idx);
        ret=ioctl (sock, ATM_NEWBACKENDIF, &ni_rt);
    }
    // brcm
    /*
    ni.backend_num = ATM_BACKEND_BR2684;
    ni.media       = BR2684_MEDIA_ETHERNET;
    ni.mtu         = 1500;
    sprintf(ni.ifname, "nas%d", nas_idx);
    */
    // brcm
    if (ret < 0) {
      
      if(errno == EEXIST) {

	/* It's not fatal to create an interface that already exists.
	   We probably will end up doing it all the time because there's
	   no way to delete interfaces. Not a problem.
	*/
	/*	do_error(LOG_INFO, "Interface %s already exists", ni.ifname);  */
	result = 1;

      } else
	do_error(LOG_WARNING, "Can't create interface : %s", strerror(errno));
    } else {

      do_error(LOG_INFO, "Interface \"%s\" created sucessfully\n",mode? ni_rt.ifname:ni.ifname);
      result = 1;
    }
  }
  close(sock);
  return result;
}

// brcm
int assign_vcc_for_linux_intf(struct sockaddr_atmpvc addr, int nas_idx, int encap, int bufsize, int proto_filter, int mode, \
        unsigned short  vlan_id)
//int assign_vcc(struct sockaddr_atmpvc addr, int nas_idx, int encap, int bufsize)
{
  int fd;
  struct atm_qos qos;
  struct atm_backend_br2684 be;
  // brcm
  struct atm_backend_rt2684 be_rt;
  int ret=0;

  if ((fd = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5)) < 0) {
    do_error(LOG_WARNING,"failed to create socket, reason: %s", strerror(errno));
    return -1;
  }
  
  memset(&qos, 0, sizeof(qos));
  qos.aal                     = ATM_AAL5;
  qos.txtp.traffic_class      = ATM_UBR;
  qos.txtp.max_sdu            = 1524;
  qos.txtp.pcr                = ATM_MAX_PCR;
  qos.rxtp = qos.txtp;
    
  if( setsockopt(fd,SOL_SOCKET,SO_SNDBUF, &bufsize ,sizeof(bufsize)) < 0) {
    do_error(LOG_WARNING,"setsockopt SO_SNDBUF: (%d) %s\n",bufsize, strerror(errno));
    return -1;
  }

  if( setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0) {
    do_error(LOG_WARNING,"setsockopt SO_ATMQOS %s", strerror(errno));
    return -1;
  }

  if( connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_atmpvc)) < 0) {
    do_error(LOG_WARNING,"failed to connect on socket: %s", strerror(errno));
    return -1;
  }
  
  /* attach the vcc to device.
   */
  // brcm
  if (!mode) {
  // brcm
  be.backend_num   = ATM_BACKEND_BR2684;
  be.ifspec.method = BR2684_FIND_BYIFNAME;
  // brcm
  setIndexName(be.ifspec.spec.ifname, "nas", nas_idx);
  //sprintf(be.ifspec.spec.ifname, "nas_%d", nas_idx);
  be.fcs_in        = BR2684_FCSIN_NO;
  be.fcs_out       = BR2684_FCSOUT_NO;
  be.fcs_auto      = 0;
  be.encaps        = encap;
  be.has_vpiid     = 0;
  be.send_padding  = 0;
  be.min_size      = 0;
  // brcm
  be.proto_filter  = proto_filter;
  be.vlan_id = vlan_id;

  // brcm
  } else {
    be_rt.backend_num   = ATM_BACKEND_RT2684;
    be_rt.ifspec.method = BR2684_FIND_BYIFNAME;
    // brcm
    setIndexName(be_rt.ifspec.spec.ifname, "ipa", nas_idx);
    //sprintf(be_rt.ifspec.spec.ifname, "atm_%d", nas_idx);
    be_rt.encaps        = encap;
  }

  // brcm
  if (!mode)
      ret = ioctl (fd, ATM_SETBACKEND, &be);
  else
      ret = ioctl (fd, ATM_SETBACKEND, &be_rt);
  if( ret == 0) {
    do_error(LOG_INFO,"Communicating over ATM %d.%d.%d, encapsulation: %s\n", 
	     addr.sap_addr.itf,
	     addr.sap_addr.vpi,
	     addr.sap_addr.vci,
	     encap ? "LLC" : "VC mux");
    return fd;
  } else {
    do_error(LOG_WARNING,"Could not configure interface:%s",strerror(errno));
    return -1;
  }
}

static int assign_vcc_for_bcm_intf(struct sockaddr_atmpvc addr, int nas_idx,
    int encap, int bufsize, int proto_filter, int mode, unsigned short  vlan_id)
{
    int fd = -1;

    if( (fd = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5)) >= 0 )
    {
        struct atm_backend_br2684 be;
        int vpi = nas_idx / OFFSET;
        int vci = nas_idx - (OFFSET * vpi);

        be.backend_num   = ATM_BACKEND_BR2684_BCM;
        be.ifspec.method = BR2684_FIND_BYIFNAME;
        setIndexName(be.ifspec.spec.ifname, "nas", nas_idx);
        be.fcs_in        = BR2684_FCSIN_NO;
        be.fcs_out       = BR2684_FCSOUT_NO;
        be.fcs_auto      = 0;
        be.encaps        = encap;
        be.has_vpiid     = (vpi << 16) | vci;
        be.send_padding  = 0;
        be.min_size      = 0;
        be.proto_filter  = proto_filter;
        be.vlan_id       = vlan_id;

        if( ioctl (fd, ATM_SETBACKEND, &be) == 0 )
        {
            do_error(LOG_INFO,"Communicating over ATM 0.%d.%d, encapsulation: "
                "LLC\n", vpi, vci);
        }
        else
        {
            do_error(LOG_WARNING,"Could not configure interface:%s\n",
                strerror(errno));
            close(fd);
            fd = -1;
        }
    }
    else
    {
        do_error(LOG_WARNING,"failed to create socket, reason: %s",
            strerror(errno));
    }

    return( fd );
}

int assign_vcc(struct sockaddr_atmpvc addr, int nas_idx, int encap, int bufsize, int proto_filter, int mode, \
        unsigned short  vlan_id)
{
  if (g_use_bcm_atmapi == 1 && !mode)
    return(assign_vcc_for_bcm_intf(addr, nas_idx, encap, bufsize, proto_filter,
        mode, vlan_id));
  else
    return(assign_vcc_for_linux_intf(addr, nas_idx, encap, bufsize, proto_filter,
        mode, vlan_id));
}


void do_add(int cli_fd, struct be_msg *rmsg, struct ucred *cli_cred)
{

  //brcm begin: static pointers of prev_vc,group has problems when delete is called in between adds
  //  static struct be_group *group = &group_head;
  //  static struct be_vc *prev_vc = NULL;
  struct be_group *group = &group_head;
  struct be_vc *prev_vc = NULL;
  //brcm end
  static char curr_groupname[MAX_GROUPNAME_LEN] = {};
  static pid_t curr_cli_pid = 0;

  struct be_group *last_group = NULL, *new_group;
  struct be_vc *new_vc;
  struct be_msg smsg;
  int sock;

  /* New client instance means that we reset the current working group.
   */
  if(curr_cli_pid != cli_cred->pid) {
    curr_groupname[0] = '\0';
    group = &group_head;
    curr_cli_pid = cli_cred->pid;
  }

// brcm
  if(create_br(rmsg->nas_idx, rmsg->mode, rmsg->extif) == 0) {
//  if(create_br(rmsg->nas_idx) == 0) {

    smsg.msgtype = INTERFACE_FAILED;
    if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
      do_error(LOG_WARNING,"do_add() couldn't send interface creation failure message.");
    return;
  }


  if (rmsg->extif) {
    if((sock = assign_vcc_filter_for_intf(rmsg->pvc, rmsg->nas_idx, 
                                             rmsg->extif, rmsg->proto_filter)) < 0) {
    smsg.msgtype = SOCK_FAILED;
    if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
      do_error(LOG_WARNING,"do_add() couldn't send socket creation failure message.");
    return;
    }
    else {
      smsg.msgtype = OK;
      if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	do_error(LOG_ERR,"do_add couldn't send OK response: %s",strerror(errno));      	
      return;
    }
  }
    
// brcm
  if( (sock = assign_vcc(rmsg->pvc, rmsg->nas_idx, rmsg->encap, 8192, rmsg->proto_filter, rmsg->mode, \
          rmsg->vlan_id)) < 0) {
//  if( (sock = assign_vcc(rmsg->pvc, rmsg->nas_idx, BR2684_ENCAPS_LLC, 8192)) < 0) {
    
    smsg.msgtype = SOCK_FAILED;
    if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
      do_error(LOG_WARNING,"do_add() couldn't send socket creation failure message.");
    return;
  }


  /* See if this group is different from the last one. If isn't, we simply pick up where
     we left off last time with the group and vc pointers stored in static variables.
     This is more efficient for multiple adds, because we always want go to the end of 
     the current group's VC list.
  */
  if( strncmp(rmsg->name,curr_groupname,MAX_GROUPNAME_LEN) ) {
    
    /* This groupname is different from the last one. Remember it for next time.
     */

    strncpy(curr_groupname,rmsg->name,MAX_GROUPNAME_LEN); 

    /* Does this group exist?  
     */
    for(group = &group_head; group != NULL; group = group->next) {
      last_group = group;
      if(!strncmp(rmsg->name,group->name,MAX_GROUPNAME_LEN)) 
	break;
    }

    if(!group) {

      /* Nope.  Create a new group.
       */
      if(! (new_group = (struct be_group *)malloc(sizeof(struct be_group))) ) {
	do_error(LOG_WARNING,"can't allocate memory for new group %s", rmsg->name);
	
	close(sock);
	smsg.msgtype = NOMEM;
	if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	  do_error(LOG_WARNING,"do_add() couldn't send malloc failure message for new group.");
	return;
      }
  
      memstat.group_mallocs++;

      last_group->next      = new_group;
      new_group->head       = NULL;
      new_group->next       = NULL;
      strncpy(new_group->name,rmsg->name,MAX_GROUPNAME_LEN);
      
      /* set prev_vc to the VC list head.
       */
      group = new_group;
      prev_vc = group->head;
    } else

      /* Ratchet to the end of an existing group VC list.
       */
      for(prev_vc = group->head; prev_vc->next != NULL; prev_vc=prev_vc->next);
  }

  /* Create the new VC
   */
  if(! (new_vc = (struct be_vc *)malloc(sizeof(struct be_vc))) ) {
    do_error(LOG_WARNING,"can't allocate memory for new vc %d/%d on nas%d\n",
	     rmsg->pvc.sap_addr.vpi,
	     rmsg->pvc.sap_addr.vci,
	     rmsg->nas_idx
	     );

    close(sock);
    smsg.msgtype = NOMEM;
    if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
      do_error(LOG_WARNING,"do_add() couldn't send malloc failure message for new VC.");
    return;
  }
  
  memstat.vc_mallocs++;
  
  new_vc->nas_idx  = rmsg->nas_idx;
  memcpy(&new_vc->pvc, &rmsg->pvc, sizeof(struct sockaddr_atmpvc));
  new_vc->sock    = sock;
  new_vc->uid     = cli_cred->uid;
  new_vc->vlan_id = rmsg->vlan_id;
  new_vc->next    = NULL;
  
  /* Add the new VC to the end of the list and remember the
     new end for next time. This "if" block is neccessary because
     we cannot initialize prev_vc with the address of group_head.pvc,
     which is non-constant. My intent is to keep most of the state 
     variables local to this function.
  */
  if(!group->head) {
    group->head = new_vc;
  }
  else {
    for(prev_vc = group->head; prev_vc->next != NULL; prev_vc=prev_vc->next) ;

    if(prev_vc) {
    prev_vc->next = new_vc;
    }
  }
  //brcm this has a problem if the VCC in the list is deleted
  //prev_vc       = new_vc;
  
  smsg.msgtype = OK;
  if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
    do_error(LOG_ERR,"do_add couldn't send OK response: %s",strerror(errno));      	
}

// brcm
int delete_br(int num)
//int delete_br(char *nstr)
{
//  int num, err;
  int err;
  int sock;

  sock = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5);
  if (sock<0) {
    syslog(LOG_ERR, "socket creation failed: %s",strerror(errno));
  } else {
    /* create the device with ioctl: */
    //num=atoi(nstr);
    if( num>=0 && num<1234567890){
      struct atm_newif_br2684 ni;
      ni.backend_num = ATM_BACKEND_BR2684;
      ni.media = BR2684_MEDIA_ETHERNET;
      ni.mtu = 1500;
      setIndexName(ni.ifname, "nas", num);
      //sprintf(ni.ifname, "nas_%d", num);
      err=ioctl (sock, BR2684_DEL, &ni);
    } else {
      syslog(LOG_ERR,"err: strange interface number %d", num );
    }
  }
  return 0;
}


int vc_match(struct be_vc *vc, struct be_msg *msg)
{
  if(vc == NULL) return 0;

  if( vc->pvc.sap_addr.vpi == msg->pvc.sap_addr.vpi &&
      vc->pvc.sap_addr.vci == msg->pvc.sap_addr.vci &&
      vc->nas_idx          == msg->nas_idx
      ) return 1;

  return 0;
}


void do_delete(int cli_fd, struct be_msg *rmsg, struct ucred *cli_cred)
{
  static struct be_vc placeholder, *bookmark = NULL;
  static struct be_group *group = NULL, *prev_group = &group_head;
  static pid_t curr_cli_pid;

  struct be_vc *prev = NULL, *match = NULL, *new_next, *doomed;
  struct be_msg smsg;
  int search_done = 0;

  /* New client instance means that we reset the current bookmark.
   */
  if(curr_cli_pid != cli_cred->pid) {
    bookmark = NULL;
    curr_cli_pid = cli_cred->pid;
  }

  if(bookmark == NULL || (vc_match(bookmark->next,rmsg) == 0)) {

    /* No current bookmark, or the VC after the bookmark doesn't match
       the VC we want to delete. Try to find the VC somewhere.
    */

    for(group = &group_head; group != NULL ; group = group->next) {
      for(match = group->head; match != NULL; match = match->next) {
	if((search_done = vc_match(match,rmsg))) break;
	prev = match;
      }
      if(search_done) break;
      prev_group = group;
    }

    if(!match) {
      
    /* VC not found, invalidate the bookmark, send fail message and return.
     */

      bookmark     = NULL;
      smsg.msgtype = VC_NOT_FOUND;
      if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	do_error(LOG_ERR,"do_delete couldn't send VC_NOT_FOUND response: %s",strerror(errno));
      
      return;
    }
 
    /* Wrong credentials, also no dice.
     */
    if(match->uid != cli_cred->uid) {
      
      smsg.msgtype = NOT_OWNER;
      smsg.uid     = bookmark->next->uid;
      
      if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	do_error(LOG_ERR,"do_delete couldn't send NOT_OWNER response: %s", strerror(errno));
      return;
    }

    if(match == group->head) {

      /* VC to be deleted is at the beginning of the group list. Set the bookmark
	 to our static placeholder, update its next member, and move the group head
	 to the next VC in the list.
      */

      if(match->next == NULL) {

	/* Special case: if only one VC remains in the list, free the group if
	   we allocated it.
	*/
	
	if(group == &group_head) {
	  group->head = NULL;
	} else {
	  prev_group->next = group->next;
	  free(group);
	  memstat.group_frees++;
	}
      }

      bookmark       = &placeholder;
      group->head    = match->next;
      bookmark->next = match->next;
      
    } else if(match->next == NULL) {
      
      /* VC to be deleted is at the end of the list. Invalidate the bookmark
	 so we don't bother picking up where we left off next time. Also, update
	 the new last item in the list (prev) so that its next member points to NULL.
      */
      
      bookmark   = NULL;
      prev->next = NULL;
      
    } else {

      /* VC to be deleted is somewhere in the middle of the list. Set the bookmark
	 to the previous VC, and set that VC's next member to the VC following the
	 one we'll be deleting.
       */

      bookmark       = prev;
      bookmark->next = match->next;
    }
    
    /* In all cases, close the socket related to the VC we want to delete,
       free the memory, increment our vc_frees counter and send the OK message
       to the client.
    */

    // brcm
    delete_br(match->nas_idx);

    close(match->sock);
    free(match);
    memstat.vc_frees++;

    smsg.msgtype = OK;
    if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
      do_error(LOG_ERR,"no bookmark: do_delete couldn't send OK response: %s",strerror(errno));
      
   
  } else { /* The bookmark is usable because its next member matches
	      the VC we want to delete.
	   */

    if(bookmark->next->uid != cli_cred->uid) {

    /* Wrong credentials, no dice.
     */
      
      smsg.msgtype = NOT_OWNER;
      smsg.uid     = bookmark->next->uid;

      if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	do_error(LOG_ERR,"using bookmark: do_delete couldn't send NOT_OWNER response: %s",strerror(errno));

      return;
    }
    
    /* Remember the VC after the one we're going to delete.
     */
    new_next = bookmark->next->next;

    if(bookmark->next == group->head) {

      /* If we're at the beginning of the list, there are two special cases.
       */

      if(group->head->next == NULL) {

	/* If only one VC remains in the list, free the group structure
	   (if we allocated it) and invalidate the bookmark.
	 */

	doomed = group->head;

	/* Don't free the global group list head!
	 */
	if(group == &group_head) {
	  group->head = NULL;
	} else {
	  prev_group->next = group->next;
	  free(group);
	  memstat.group_frees++;
	}

	bookmark = NULL;

      } else {

	/* If there is more than one VC in the list, we update the group head,
	   AND update the bookmark.
	*/

	doomed         = bookmark->next;
	group->head    = new_next;
	bookmark->next = new_next;
      }
      
    } else if(new_next == NULL) {

      /* If the VC to be deleted is the last one, we set the bookmark VC's next pointer
	 to NULL, and then invalidate the bookmark.
       */
      
      doomed         = bookmark->next;
      bookmark->next = NULL;
      bookmark       = NULL;
      
    } else {
     
      /* If the VC to be deleted is somewhere in the middle, then all we do is update
	 the bookmark and do the usual VC delete actions.
      */
      
      doomed         = bookmark->next; 
      bookmark->next = new_next;
      
    }

    // brcm
    delete_br(doomed->nas_idx);

    close(doomed->sock);
    free(doomed);
    memstat.vc_frees++;

    smsg.msgtype = OK;
    if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
      do_error(LOG_ERR,"do_delete couldn't send OK response: %s",strerror(errno));
  }
}


void do_delete_group(int cli_fd, char *name, struct ucred *cli_cred)
{
  struct be_group *group, *prev_group = NULL;
  struct be_vc *curr,*doomed;
  struct be_msg smsg;

  smsg.msgtype = GROUP_NOT_FOUND;

  for(group = &group_head; group != NULL; group = group->next) {
    if(!strncmp(name,group->name,MAX_GROUPNAME_LEN))
      break;
    prev_group = group;
  }

  if(group) {
    curr = group->head; 
    while(curr != NULL) {
      doomed = curr;
      curr = curr->next;

      // brcm
      delete_br(doomed->nas_idx);

      close(doomed->sock);
      free(doomed);
      memstat.vc_frees++;
    }

    prev_group->next = group->next;
    free(group);
    memstat.group_frees++;
    smsg.msgtype = OK;
  }

  if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
    do_error(LOG_ERR,"do_delete_group() couldn't send response: %s",strerror(errno));
}


void do_list_group(int cli_fd, struct be_msg *rmsg)
{
  struct be_group *group;
  struct be_vc *curr;
  struct be_msg smsg;

  smsg.msgtype = GROUP_NOT_FOUND;

  for(group = &group_head; group != NULL; group = group->next)
    if(!strncmp(rmsg->name,group->name,MAX_GROUPNAME_LEN))
      break;

  if(group) {
    for(curr = group->head; curr != NULL; curr = curr->next) {

      smsg.msgtype =  OK;
      smsg.nas_idx = curr->nas_idx;
      smsg.uid     = curr->uid;
      memcpy(&smsg.pvc, &curr->pvc, sizeof(struct sockaddr_atmpvc));
      memcpy(&smsg.name, &group->name, MAX_GROUPNAME_LEN);
      if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	do_error(LOG_ERR,"do_list_group() couldn't send OK message: %s",strerror(errno));
    }

    smsg.msgtype = LIST_END;
  }

  if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
    do_error(LOG_ERR,"do_list_group() couldn't send LIST_END message: %s",strerror(errno));
}


void do_list_all(int cli_fd, struct be_msg *rmsg)
{
  struct be_group *group = NULL;
  struct be_vc *curr;
  struct be_msg smsg;

  for(group = &group_head; group != NULL; group = group->next) {

    for(curr = group->head; curr != NULL; curr = curr->next) {

      smsg.msgtype = OK; 
      smsg.nas_idx = curr->nas_idx;
      smsg.uid     = curr->uid;
      memcpy(&smsg.pvc, &curr->pvc, sizeof(struct sockaddr_atmpvc));
      memcpy(&smsg.name, group->name, MAX_GROUPNAME_LEN);
      // brcm begin
      smsg.mode = curr->mode;
      smsg.vlan_id = curr->vlan_id;
      // brcm end
      if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	do_error(LOG_ERR,"do_list_all() couldn't send VC info: %s",strerror(errno));
    }

  }

  if(smsg.msgtype == OK) 
    smsg.msgtype = LIST_END;
  else
    smsg.msgtype = GROUP_NOT_FOUND;

  if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
    do_error(LOG_ERR,"do_list_all() couldn't send LIST_END message: %s",strerror(errno));
}

#ifdef BUILD_STATIC
int pvc2684d_main (int argc, char **argv)
#else
int main (int argc, char **argv)
#endif
{
  int test_fd, listen_fd, cli_fd;
  socklen_t  cliaddr_len;
  struct sockaddr_un test_addr, listen_addr, cli_addr;
  const int on = 1;
  int nbytes;
  struct ucred cli_cred;
  int ucred_len = sizeof(cli_cred);
  struct be_msg smsg, rmsg;
  struct stat listen_stat;

  openlog("pvc2684d",LOG_PERROR, LOG_DAEMON);

  bzero(group_head.name,MAX_GROUPNAME_LEN);
  group_head.head = NULL;
  group_head.next = NULL;

  bzero(&test_addr, sizeof(test_addr));
  test_addr.sun_family = AF_LOCAL;
  strncpy(test_addr.sun_path, BRPVC_SOCKPATH, sizeof(test_addr.sun_path) -1);
  memcpy(&listen_addr, &test_addr, sizeof(test_addr));

  if( (test_fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) 
    do_error(LOG_ERR,"Couldn't create initial socket: %s",strerror(errno));

  /* Check for already running daemon  */

  if(connect(test_fd, (struct sockaddr *) &test_addr, sizeof(test_addr))) {
    if(errno == ECONNREFUSED)
      unlink(BRPVC_SOCKPATH);
  }
  close(test_fd);

  if( (listen_fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    do_error(LOG_ERR,"Couldn't create server socket: %s",strerror(errno));

  if( bind(listen_fd, (struct sockaddr *) &listen_addr, SUN_LEN(&listen_addr)) ) {
    do_error(LOG_WARNING,"Another b2684d is running");
    exit(-1);
  }
  
  
  if(stat(BRPVC_SOCKPATH, &listen_stat))
    do_error(LOG_ERR,"Can't fstat listen socket: %s",strerror(errno));

  if(chmod(BRPVC_SOCKPATH, listen_stat.st_mode | S_IWOTH))
    do_error(LOG_ERR,"Can't fchmod listen socket: %s",strerror(errno));

  if( listen(listen_fd, 5) )
    do_error(LOG_ERR,"listen() on server socket failed: %s",strerror(errno));
  
  while(1) {
    cliaddr_len = sizeof(cli_addr);
    if((cli_fd = accept(listen_fd, (struct sockaddr *) &cli_addr, &cliaddr_len)) < 0) {
      if(errno == EINTR)
	continue;
      else
	do_error(LOG_ERR,"accept() on server socket failed: %s",strerror(errno));
    }
    if( setsockopt(cli_fd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0 )
      do_error(LOG_ERR,"setsockopt() on client socket failed: %s",strerror(errno));
    
    while((nbytes = recv(cli_fd, &rmsg, sizeof(rmsg), 0)) > 0) {
      switch (rmsg.msgtype) {
	
      case HELLO:
	
	if ( getsockopt(cli_fd, SOL_SOCKET, SO_PEERCRED, &cli_cred, &ucred_len) < 0 )
	  do_error(LOG_ERR,"getsockopt() for credentials failed: %s",strerror(errno));
	
	smsg.msgtype = OK;
	if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	  do_error(LOG_ERR,"Couldn't send OK message to new client: %s",strerror(errno)); 

	break;
      
      case ADD:

	do_add(cli_fd,&rmsg,&cli_cred);
	break;
      
      case DELETE:

	do_delete(cli_fd,&rmsg,&cli_cred);
	break;
      
      case DELETE_GROUP:

	do_delete_group(cli_fd,rmsg.name,&cli_cred);
	break;
      
      case LIST_GROUP:
	 
	do_list_group(cli_fd,&rmsg);
	break;
      
      case LIST_ALL:

	do_list_all(cli_fd,&rmsg);
	break;

      case MEM_STATS:
	
	if( send(cli_fd, &memstat, sizeof(memstat), 0) < 0 )
	  do_error(LOG_ERR,"Couldn't send MEM_STAT message: %s",strerror(errno)); 
	break;
      
      default:
	smsg.msgtype = UNKNOWN_CMD;
	if( send(cli_fd, &smsg, sizeof(smsg), 0) < 0 )
	  do_error(LOG_ERR,"Couldn't send UNKNOWN_COMMAND message: %s",strerror(errno)); 
      }
    }
    close(cli_fd);
  }
}

static int assign_vcc_filter_for_intf(struct sockaddr_atmpvc addr, int nas_idx,
                                            int extif, int proto_filter)
{
    int fd = -1;

    if( (fd = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5)) >= 0 )
    {
        struct atm_backend_br2684 be;
        int vpi = nas_idx / OFFSET;
        int vci = nas_idx - (OFFSET * vpi);

        //bzero(&be, sizeof(struct atm_backend_br2684));
        be.backend_num   = ATM_BACKEND_BR2684_BCM;
        be.ifspec.method = BR2684_FIND_BYIFNAME;
        setIndexName(be.ifspec.spec.ifname, "nas", nas_idx);
        //sprintf(be.ifspec.spec.ifname, "%s_%d_%d_%d","nas",vpi,vci,extif);
        be.has_vpiid = extif;
        be.proto_filter  = proto_filter;

        if( ioctl (fd, ATM_SETEXTFILT, &be) == 0 )
        {
            do_error(LOG_INFO,"Communicating over ATM 0.%d.%d, encapsulation: "
                "LLC\n", vpi, vci);
        }
        else
        {
            do_error(LOG_WARNING,"Could not configure interface:%s\n",
                strerror(errno));
            close(fd);
            fd = -1;
        }
    }
    else
    {
        do_error(LOG_WARNING,"failed to create socket, reason: %s",
            strerror(errno));
    }

    return( fd );
}
