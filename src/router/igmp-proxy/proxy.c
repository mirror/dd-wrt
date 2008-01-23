/******************************************************************************
 * Fichier Module : proxy.c - An IGMPv3 Proxy implementation
 ******************************************************************************
 * Fichier    : proxy.c
 * Description: Implementation des divers routines 
 *              Mars 2002
 * Date       : Mars 15, 2000
 * Auteur     : Anis.Ben-Hellel@loria.fr
 * Last Modif : May 13, 2002
 *
 *****************************************************************************/

#include "igmprt.h"

vifi_t numvifs;
char buffer[IP_MSFILTER_SIZE(MAX_ADDRS)];
unsigned long upstream;


/*
 * set the source list and the source filter
 * on upstream interface
 */

void set_source_filter(
                        igmp_router_t* router,
                        igmp_group_t* gp,
                        unsigned long interface_adress,
                        int fmode,
                        int nsources,
                        struct in_addr *sources)

{
       struct ip_msfilter *imsfp;
       int i;

       imsfp = (struct ip_msfilter *) &buffer;

       if (VALID_ADDR(gp->igmpg_addr)) {

         imsfp->imsf_fmode = fmode;
         imsfp->imsf_numsrc = nsources;
#ifdef Linux
	 imsfp->imsf_multiaddr = gp->igmpg_addr.s_addr;
	 imsfp->imsf_interface = interface_adress;
#else
         imsfp->imsf_multiaddr.s_addr =gp->igmpg_addr.s_addr;
         imsfp->imsf_interface.s_addr = interface_adress;
#endif	 
         for (i=0; i<nsources; i++) {
           imsfp->imsf_slist[i] = sources[i].s_addr;
         }
	 /*use setsockopt*/
	 i = sizeof(*imsfp);
	 if (setsockopt(router->igmprt_up_socket, IPPROTO_IP, IP_MSFILTER, imsfp,i) < 0 ){
		perror("setsockopt IP_MSFILTER"); 
	 }
         /*if (ioctl(router->igmprt_up_socket, SIOCSIPMSFILTER, imsfp) != 0){
           perror("ioctl - SIOCSIPMSFILTER");*/
           //printf("ioctl error, group: %s, source: %s\n",inet_ntoa(gp->igmpg_addr.s_addr),inet_ntoa(sources[0].s_addr));
         //}
       }
       return;
}


/*
 * Open and init the multicast routing in the kernel.
 */

void k_init_proxy(int socket)
{
    int v = 1;
    
    if (setsockopt(socket, IPPROTO_IP, MRT_INIT, (char *)&v, sizeof(int)) < 0)
      perror("setsockopt - MRT_INIT");
}



/*
 * Stops the multicast routing in the kernel.
 */

void k_stop_proxy(int socket)
{
    int v = 0;
    
    if (setsockopt(socket, IPPROTO_IP, MRT_DONE, (char *)NULL, 0) < 0)
      perror("setsockopt - MRT_DONE");
}



/* 
 * Add a virtual interface to the kernel 
 * using the pimd API:MRT_ADD_VIF
 * 
 */

int k_proxy_add_vif (int socket,unsigned long vifaddr,vifi_t vifi)
{
	struct vifctl vc;
	int error;
	
	vc.vifc_vifi = vifi;
	vc.vifc_flags = 0;
	vc.vifc_threshold = 0;
	vc.vifc_rate_limit = 0;
	vc.vifc_lcl_addr.s_addr = vifaddr;
	vc.vifc_rmt_addr.s_addr = INADDR_ANY;
	if (error=setsockopt(socket, IPPROTO_IP, MRT_ADD_VIF,(char *)&vc, sizeof(vc)) <0){
	  perror("setsockopt - MRT_ADD_VIF");
	  return FALSE;
	}
	return TRUE;
}


/*
 * Del an MFC entry from the kernel
 * using pimd API:MRT_DEL_MFC
 */

int k_proxy_del_mfc (int socket, u_long source, u_long group)
{
	struct mfcctl mc;
	
	mc.mfcc_origin.s_addr   = source;
	mc.mfcc_mcastgrp.s_addr = group;
	if (setsockopt(socket, IPPROTO_IP, MRT_DEL_MFC, (char *)&mc, sizeof(mc)) <0){
	  perror("setsockopt - MRT_DEL_MFC");
	  printf("k_del_mfc:error on setsockopt\n");
	  return FALSE;
	}
	return TRUE;
}



/*
 * Install and modify a MFC entry in the kernel (S,G,interface address)
 * using pimd API: MRT_AD_MFC
 */

int k_proxy_chg_mfc(int socket,u_long source,u_long group,vifi_t outvif,int fstate)
{
    struct mfcctl mc;
    vifi_t vifi;
    
    mc.mfcc_origin.s_addr = source;
    mc.mfcc_mcastgrp.s_addr = group;
    /* change me to the index of the upstream interface */
    mc.mfcc_parent = 0;
#ifndef Linux    
    mc.mfcc_oif = outvif;
#endif
    mc.mfcc_ttls[outvif] = fstate;
    if (setsockopt(socket, IPPROTO_IP, MRT_ADD_MFC, (char *)&mc, sizeof(mc)) < 0) {
      perror("setsockopt - MRT_ADD_MFC");
      return(FALSE);
    }
    return(TRUE);
}

/*
 * create entry in the membership database
 */

membership_db*
create_membership(struct in_addr group,int fmode,int numsources,struct in_addr sources[MAX_ADDRS])
{
        membership_db* member;
        int i;
        if (member = (membership_db*) malloc(sizeof(*member))) {
                member->membership.group = group;
                member->membership.fmode = fmode;
                member->membership.numsources = numsources;
                for(i=0;i<numsources;i++)
                  member->membership.sources[i].s_addr = sources[i].s_addr;
                member->next = NULL;
                return member;
        }else
                return NULL;
}


/*
 * lookup for a group entry in the membership database
 */

membership_db*
find_membership(membership_db *membership,struct in_addr group)
{
  membership_db* memb;

        for(memb=membership;memb;memb=memb->next)
          if (memb->membership.group.s_addr == group.s_addr)
            return memb;
        return NULL;
}

/*
 * deleate group entry from membership database
 */

membership_db*
deleate_membership(igmp_router_t* igmprt,struct in_addr group)
{
       membership_db *member;
       membership_db *memb;

        member = find_membership(igmprt->igmprt_membership_db,group);
        assert(igmprt != NULL);
        assert(member != NULL);
        if (igmprt->igmprt_membership_db->membership.group.s_addr != group.s_addr){
          memb = igmprt->igmprt_membership_db;
          while(memb->next->membership.group.s_addr != group.s_addr)
            memb = memb->next;
          memb->next = member->next;
          free(member);
        }
        else { /*deleate the head*/
          memb = igmprt->igmprt_membership_db;
          igmprt->igmprt_membership_db = memb->next;
          free(memb);
        }
        LOG((LOG_DEBUG, "membership database, group_cleanup: %s\n", inet_ntoa(member->membership.group.s_addr)));
}



/*
 * find a source in a in a source list
 */

int find_source(struct in_addr sr,int nsources,struct in_addr *sources)
{
       int i;

       for(i=0;i<nsources;i++)
         if (sources[i].s_addr == sr.s_addr)
           return TRUE;
       return FALSE;
}
/*
 * add multicast group to the membership database
 */

membership_db*
update_multi(igmp_router_t *igmprt,struct in_addr group,int fmode,int nsources,struct in_addr sources[MAX_ADDRS])
{

       int i,k;
       struct ip_msfilter *imsfp;
       membership_db* member;
       struct in_addr sr[MAX_ADDRS];

       /*find corresponding group*/
       if (member = find_membership(igmprt->igmprt_membership_db,group)) {
           /*update group status using merging rules*/
           member->membership.fmode = (int)member->membership.fmode && (int)fmode;
           if (member->membership.fmode == IGMP_FMODE_INCLUDE) {
                if (fmode == IGMP_FMODE_INCLUDE) {
                     for(i=0;i<nsources;i++)
                       if (find_source(sources[i],member->membership.numsources,member->membership.sources) == FALSE){
                         member->membership.numsources = member->membership.numsources + 1;
                         member->membership.sources[member->membership.numsources].s_addr = sources[i].s_addr;
                       }
                }
                else{
                     k = 0;
                     for(i=0;i<nsources;i++)
                         if (find_source(sources[i],member->membership.numsources,member->membership.sources) == FALSE){
                             sr[k].s_addr = sources[i].s_addr;
                             k = k+1;
                         }
                     member->membership.numsources = k;
                     for(i=0;i<k;i++)
                         member->membership.sources[i].s_addr = sr[i].s_addr;
                }
           }
           else {
                if (fmode == IGMP_FMODE_INCLUDE) {
                     k = 0;
                     for(i=0;i<member->membership.numsources;i++)
                         if (find_source(member->membership.sources[i],nsources,sources) == FALSE){
                             sr[k].s_addr = member->membership.sources[i].s_addr;
                             k = k+1;
                         }
                     member->membership.numsources = k;
                     for(i=0;i<k;i++)
                       member->membership.sources[i].s_addr = sr[i].s_addr;
                }
                else{
                     k = 0;
                     for(i=0;i<member->membership.numsources;i++)
                         if (find_source(member->membership.sources[i],nsources,sources) == TRUE){
                             sr[k].s_addr = member->membership.sources[i].s_addr;
                             k = k+1;
                         }
                     member->membership.numsources = k;
                     for(i=0;i<k;i++)
                       member->membership.sources[i].s_addr = sr[i].s_addr;
                }
           }
           //printf("update membership database group: %s, source: %s\n",inet_ntoa(group.s_addr),inet_ntoa(member->membership.sources[0]
       }
       else {
            /*create new entry in the membership database*/
            member = create_membership(group,fmode,nsources,sources);
            member->next = igmprt->igmprt_membership_db;
            igmprt->igmprt_membership_db = member;
       }
       return member;
}

