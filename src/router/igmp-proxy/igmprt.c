/******************************************************************************
 * Fichier main :igmprt - An IGMP Proxy implementation
 ******************************************************************************
 * Fichier    : igmprt.c
 * Description: Implementation d'un proxy IGMP en se basant sur
 *              l'internet draft "draft-ietf-idmr-igmp-v3-07.txt", Mars 2001
 *              et "draft-ietf-idmr-igmp-proxy-01.txt", Janvier 2002
 * Date       : May 18, 2000
 * Auteurs    : wilbertdg@hetnet.nl
 *              lahmadi@loria.fr
 *              Anis.Ben-Hellel@loria.fr
 * Last Modif : Juin 10, 2002
 *
 *****************************************************************************/

#include "conf.h"
#include "igmprt.h"
/*version and isquerier variable from the config file*/

int version,querier;
//unsigned long upstream;


void igmp_info_print(igmp_router_t *router){
	
	igmp_interface_t *ifp;
	igmp_group_t *gp;
	igmp_src_t *src;
	igmp_rep_t *rep;
	printf("\nIGMP Table\n");
	printf("-----------------\n");
	printf("\n%-14s %-9s %-14s %-5s %-14s %-14s\n","interface","version","groups","mode","source","Membres");
	for (ifp=router->igmprt_interfaces;ifp;ifp=(igmp_interface_t *)ifp->igmpi_next){
		printf("%-14s 0x%x\n",inet_ntoa(ifp->igmpi_addr),ifp->igmpi_version);
		if (ifp->igmpi_groups != NULL){
			for(gp=ifp->igmpi_groups;gp;gp=(igmp_group_t*)gp->igmpg_next){
				printf("%32s %11s\n",inet_ntoa(gp->igmpg_addr),(gp->igmpg_fmode == IGMP_FMODE_INCLUDE)? "INCLUDE":"EXCLUDE");
				if (gp->igmpg_sources != NULL)
				     for (src=gp->igmpg_sources;src;src=(igmp_src_t *)src->igmps_next)
					printf("%50s\n",inet_ntoa(src->igmps_source));
				if (gp->igmpg_members != NULL)
					for (rep=gp->igmpg_members;rep;rep=(igmp_rep_t *)rep->igmpr_next)
						/*if (gp->igmpg_sources != NULL)
							printf("%17s\n",inet_ntoa(rep->igmpr_addr));
						else*/ 
							printf("%70s\n",inet_ntoa(rep->igmpr_addr));
				else printf("\n");
			}
		}else
			printf("\n");
		
	}


}

/****************************************************************************
 *
 * LDAP communication
 *
 ***************************************************************************/
int validate(){

/* validation du report provenant d'un membre d'un groupe*/

}

/****************************************************************************
 *
 * Routines pour les enregistrements des membres: Ceci n'a pas ete specifie
 * dans le draft, mais pour se rendre compte vite q'un membre viend de quitter 
 * un groupe, ou pour garder trace du membres on peut utiliser ces routines
 *
 ***************************************************************************/
		    
/*
 * igmp_rep_t* igmp_rep_create()
 *
 * Cree un enregistrement d'un membre d'un groupe
 */
igmp_rep_t*
igmp_rep_create(
	struct in_addr srcaddr)
{
	igmp_rep_t* rep;

	if (rep = (igmp_rep_t*) malloc(sizeof(*rep))) {
	  rep->igmpr_addr.s_addr = srcaddr.s_addr;
	  rep->igmpr_next  = NULL;
	}
	return rep;
}

/*
 * void igmp_rep_cleanup()
 *
 * Effacer un enregistrement d'un membre 
 */
void
igmp_rep_cleanup(
	igmp_group_t *gp,
	igmp_rep_t * rep)
{
	igmp_rep_t *re;

	assert(rep != NULL);
	assert (gp != NULL);
	if (rep != gp->igmpg_members){
	  for (re=gp->igmpg_members;(igmp_rep_t *)re->igmpr_next != rep;re=(igmp_rep_t *)re->igmpr_next);		
	  re->igmpr_next = rep->igmpr_next;
	  free(rep);
	}else{
	  /*delete the head*/
	  re=gp->igmpg_members;
	  gp->igmpg_members = (igmp_rep_t *)re->igmpr_next;
	  free(re);
	}
	LOG((LOG_DEBUG, "igmp_rep_cleanup: %s\n", inet_ntoa(rep->igmpr_addr)));
}

/*
 * void igmp_rep_print()
 *
 * Imprimer un enregistrement d'un membre
 */
void
igmp_rep_print(
	igmp_rep_t* rep)
{
  printf("\t\tmembre:%-16s\n", 
	 inet_ntoa(rep->igmpr_addr));
}
/*
 * igmp_rep_t* igmp_group_rep_lookup()
 *
 * Lookup a membre in the sourcetable of a group
 */
igmp_rep_t*
igmp_group_rep_lookup(
	igmp_group_t *gp,
	struct in_addr srcaddr)
{
  igmp_rep_t *rep;
  
  assert(gp != NULL);
  for (rep = (igmp_rep_t *) gp->igmpg_members; rep; rep = (igmp_rep_t *)rep->igmpr_next)
    if (rep->igmpr_addr.s_addr == srcaddr.s_addr)
      return rep;
  return NULL; 
}

/*
 * igmp_rep_t* igmp_group_rep_add()
 *
 * Add a member to the set of sources of a group
 */
igmp_rep_t*
igmp_group_rep_add(
	igmp_group_t *gp,
	struct in_addr srcaddr)
{
	igmp_rep_t* rep;

	assert(gp != NULL);
	/* Return the source if it's already present */
	if (rep = igmp_group_rep_lookup(gp, srcaddr))
	  return rep;
	/* Create the source and add to the set */ 
	if (rep = igmp_rep_create(srcaddr)) {
	  rep->igmpr_next = (igmp_rep_t *)gp->igmpg_members;
	  gp->igmpg_members = rep;
	}
	return rep;
}
/******************************************************************************
 *
 * igmp source routines 
 * 
 *****************************************************************************/
/*
 * igm_src_t* igmp_src_create()
 *
 * Cree un enregistrement d'une source
 */
igmp_src_t*
igmp_src_create(
	struct in_addr srcaddr)
{
	igmp_src_t* src;

	if (src = (igmp_src_t*) malloc(sizeof(*src))) {
	  src->igmps_source.s_addr = srcaddr.s_addr;
	  src->igmps_timer = 0;
	  src->igmps_fstate = TRUE;
	  src->igmps_next  = NULL;
	}
	return src;
}

/*
 * void igmp_src_cleanup()
 *
 * Effacer un enregistrement d'une source
 */
void
igmp_src_cleanup(
	igmp_group_t *gp,
	igmp_src_t * src)
{
	igmp_src_t *sr;

	assert(src != NULL);
	assert (gp != NULL);
	if (src != gp->igmpg_sources){
	  for (sr=gp->igmpg_sources;(igmp_src_t *)sr->igmps_next != src;sr=(igmp_src_t *)sr->igmps_next);		
	  sr->igmps_next = src->igmps_next;
	  free(src);
	}else{
	  /*delete the head*/
	  sr=gp->igmpg_sources;
	  gp->igmpg_sources = (igmp_src_t *)sr->igmps_next;
	  free(sr);
	}
	LOG((LOG_DEBUG, "igmp_src_cleanup: %s\n", inet_ntoa(src->igmps_source)));
}

/*
 * void igmp_src_print()
 *
 * Imprimer un enregistrement d'une source
 */
void
igmp_src_print(
	igmp_src_t* src)
{
  printf("\t\tsource:%-16s  %d\n", 
	 inet_ntoa(src->igmps_source),src->igmps_timer);
}
/*
 * igmp_src_t* igmp_group_src_lookup()
 *
 * Lookup a source in the sourcetable of a group
 */
igmp_src_t*
igmp_group_src_lookup(
	igmp_group_t *gp,
	struct in_addr srcaddr)
{
  igmp_src_t *src;
  
 if (gp != NULL){
  for (src = gp->igmpg_sources; src != NULL; src = (igmp_src_t *)src->igmps_next){
    if (src->igmps_source.s_addr == srcaddr.s_addr)
      return src;
  }
  }
  return NULL; 
}

/*
 * igmp_src_t* igmp_group_src_add()
 *
 * Add a source to the set of sources of a group
 */
igmp_src_t*
igmp_group_src_add(
	igmp_group_t *gp,
	struct in_addr srcaddr)
{
	igmp_src_t* src;

	assert(gp != NULL);
	/* Return the source if it's already present */
	if (src = igmp_group_src_lookup(gp, srcaddr))
	  return src;
	/* Create the source and add to the set */ 
	if (src = igmp_src_create(srcaddr)) {
	  src->igmps_next = (igmp_src_t *)gp->igmpg_sources;
	  gp->igmpg_sources = src;
	}
	return src;
}



/******************************************************************************
 *
 * igmp group routines 
 *
 *****************************************************************************/

/*
 * igmp_group_t* igmp_group_create()
 *
 * Create a group record
 */
igmp_group_t*
igmp_group_create(
	struct in_addr groupaddr)
{
	igmp_group_t* gp;

	if (gp = (igmp_group_t*) malloc(sizeof(*gp))) {
		gp->igmpg_addr.s_addr = groupaddr.s_addr;
		gp->igmpg_fmode = IGMP_FMODE_INCLUDE;
		gp->igmpg_version = IGMP_VERSION_3;/*default version is V3*/
		gp->igmpg_timer = 0;
		gp->igmpg_sources = NULL;
		gp->igmpg_members = NULL;
		gp->igmpg_next = NULL;
		return gp;
	}else
		return NULL;
	
}

/*
 * void igmp_group_cleanup()
 *
 * Cleanup a group record
 */
void
igmp_group_cleanup(
	igmp_interface_t *ifp,
	igmp_group_t* gp)
{
	igmp_group_t *g;
	assert(gp != NULL);
	assert(ifp != NULL);
	if (ifp->igmpi_groups != gp){
	  g=ifp->igmpi_groups;
	  while((igmp_group_t *)g->igmpg_next != gp)		
	  g=(igmp_group_t *)g->igmpg_next;
	  g->igmpg_next=gp->igmpg_next;		
	  free(gp);
	}else{/*delete the head*/
	  g=ifp->igmpi_groups;
	  ifp->igmpi_groups = (igmp_group_t *)g->igmpg_next;
	  free(g);
	} 
	LOG((LOG_DEBUG, "igmp_group_cleanup: %s\n", inet_ntoa(gp->igmpg_addr)));
}

/*
 * void igmp_group_print()
 *
 * Print a group record
 */
void
igmp_group_print(
	igmp_group_t* gp)
{
  igmp_src_t *src;
  igmp_rep_t *rep;
  printf("  %-16s %s %d\n", 
	 inet_ntoa(gp->igmpg_addr),
	 (gp->igmpg_fmode == IGMP_FMODE_EXCLUDE) ? "exclude" : "include",
	 gp->igmpg_timer);
  if (gp->igmpg_sources != NULL)
    for (src=gp->igmpg_sources;src;src=(igmp_src_t *)src->igmps_next)
      printf("source : %s timer : %d\n",inet_ntoa(src->igmps_source.s_addr),src->igmps_timer);
   if (gp->igmpg_members != NULL)
    for (rep=gp->igmpg_members;rep;rep=(igmp_rep_t *)rep->igmpr_next)
      printf("member : %s \n",inet_ntoa(rep->igmpr_addr.s_addr));
}

/******************************************************************************
 *
 * igmp interface routines 
 *
 *****************************************************************************/

/*
 * igmp_interface_t* igmp_interface_create()
 *
 * Create and initialize interface record
 */
igmp_interface_t*
igmp_interface_create(
	struct in_addr ifaddr,
	char *ifname,
	vifi_t index)
{
  igmp_interface_t* ifp;
  struct ip_mreq mreq;
  char ra[4];
  int i, prom;
  short flags;
  
	if (((ifp = (igmp_interface_t*) malloc(sizeof(*ifp)))) == NULL)
		return NULL;

	/* Allocate a buffer to receive igmp messages */ 
	ifp->igmpi_bufsize = get_interface_mtu(ifname);
	if (ifp->igmpi_bufsize == -1)
		ifp->igmpi_bufsize = MAX_MSGBUFSIZE;
	/* XXX Should make buffer slightly bigger for ip header */
	if ((ifp->igmpi_buf = (char*) malloc(ifp->igmpi_bufsize)) == NULL) {
		free(ifp);
		return NULL;
	}	

	/* Create a raw igmp socket */
	ifp->igmpi_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
	if (ifp->igmpi_socket == -1) {
		printf("can't create socket \n");
 		free(ifp->igmpi_buf);
		free(ifp);
		return NULL;
	}
	/* Initialize all fields */
	ifp->igmpi_addr.s_addr = ifaddr.s_addr;
	strncpy(ifp->igmpi_name, ifname, IFNAMSIZ);
	ifp->igmpi_groups = NULL;
	ifp->sch_group_query = NULL;
	if (ifp->igmpi_addr.s_addr == upstream){
		ifp->igmpi_type = UPSTREAM;
	}else{
	   ifp->igmpi_type = DOWNSTREAM;
	}
	ifp->igmpi_isquerier = TRUE;
  	ifp->igmpi_version = version; /*IGMP_VERSION_3;*/
	ifp->igmpi_qi = IGMP_DEF_QI;
	ifp->igmpi_qri = IGMP_DEF_QRI;
	ifp->igmpi_rv = IGMP_DEF_RV;
	ifp->igmpi_gmi = ifp->igmpi_rv * ifp->igmpi_qi + ifp->igmpi_qri;
	ifp->igmpi_ti_qi = 0;
	ifp->igmpi_next = NULL;

	/* Set router alert */
	ra[0] = 148;
	ra[1] = 4;
	ra[2] = 0;
	ra[3] = 0;
	setsockopt(ifp->igmpi_socket, IPPROTO_IP, IP_OPTIONS, ra, 4);


	/* Set reuseaddr, ttl, loopback and set outgoing interface */
	i = 1;
	setsockopt(ifp->igmpi_socket, SOL_SOCKET, SO_REUSEADDR, 
		(void*)&i, sizeof(i));
	i = 1;
	setsockopt(ifp->igmpi_socket, IPPROTO_IP, IP_MULTICAST_TTL, 
		(void*)&i, sizeof(i));
	i = 1;
	setsockopt(ifp->igmpi_socket, IPPROTO_IP, IP_MULTICAST_LOOP, 
		(void*)&i, sizeof(i));
	setsockopt(ifp->igmpi_socket, IPPROTO_IP, IP_MULTICAST_IF, 
		(void*)&ifaddr, sizeof(ifaddr));

	//setsockopt(ifp->igmpi_socket, IPPROTO_IP, IP_RECVIF, &i, sizeof(i));
	/* Add membership to ALL_ROUTERS and ALL_ROUTERS_V3 on this interface */
	mreq.imr_multiaddr.s_addr = inet_addr(IGMP_ALL_ROUTERS);
	mreq.imr_interface.s_addr = ifaddr.s_addr;/*htonl(0);*/
	setsockopt(ifp->igmpi_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
		(void*)&mreq, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(IGMP_ALL_ROUTERS_V3);
	mreq.imr_interface.s_addr = ifaddr.s_addr;/*htonl(0);*/
	setsockopt(ifp->igmpi_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
		(void*)&mreq, sizeof(mreq));

	/* Tell the kernel this interface belongs to a multicast router */
	mrouter_onoff(ifp->igmpi_socket,1);
	ifp->igmpi_index = index;
	
	/* Set the interface flags to receive all multicast packets */
	ifp->igmpi_save_flags = get_interface_flags(ifname);
	if (ifp->igmpi_save_flags != -1) {
		set_interface_flags(ifname, ifp->igmpi_save_flags | IFF_ALLMULTI | IFF_PROMISC);
	}

	return ifp;
}

/*
 * void igmp_interface_cleanup()
 *
 * Cleanup an interface 
 */
void
igmp_interface_cleanup(igmp_interface_t* ifp)
{
	igmp_group_t* gp,*gp2;

	assert(ifp != NULL);
	LOG((LOG_DEBUG, "igmp_interface_cleanup: %s\n", inet_ntoa(ifp->igmpi_addr)));

	/* Cleanup all groups */
	gp=ifp->igmpi_groups;
	ifp->igmpi_groups = NULL;
	while(gp != NULL){
		gp2=gp;
		gp=gp->igmpg_next;
		free(gp2);
	}
	/* Tell the kernel the multicast router is no more */
	mrouter_onoff(ifp->igmpi_socket, 0);
	close(ifp->igmpi_socket);

 	free(ifp->igmpi_buf);

	/* If we managed to get/set the interface flags, revert */
	if (ifp->igmpi_save_flags != -1)
		set_interface_flags(ifp->igmpi_name, ifp->igmpi_save_flags);
	free(ifp);
}

/*
 * igmp_group_t* igmp_interface_group_add()
 *
 * Add a group to the set of groups of an interface
 */
igmp_group_t*
igmp_interface_group_add(
	igmp_router_t* router,
	igmp_interface_t *ifp,
	struct in_addr groupaddr)
{
	igmp_group_t* gp;
	struct ip_mreq mreq;
	igmp_interface_t* upstream_interface;
	struct in_addr up;

	assert(ifp != NULL);
	/* Return the group if it's already present */
	if (gp = igmp_interface_group_lookup(ifp, groupaddr))
		return gp;
	/* Create the group and add to the set */ 
	if (gp = igmp_group_create(groupaddr)) {
		gp->igmpg_next = ifp->igmpi_groups;
		ifp->igmpi_groups = gp;

		/*add group to the set of groups of upstream interface*/
		mreq.imr_multiaddr.s_addr=groupaddr.s_addr;
		mreq.imr_interface.s_addr=upstream;
		if ((VALID_ADDR(mreq.imr_multiaddr)) && (ifp->igmpi_addr.s_addr != upstream)) {
		  up.s_addr = upstream;
		  upstream_interface = igmprt_interface_lookup(router,up);
		  if (igmp_interface_group_lookup(upstream_interface,mreq.imr_multiaddr) == NULL) {
		    if (setsockopt(router->igmprt_up_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &mreq, sizeof(mreq)) < 0) { 
		      perror("setsockopt - IP_ADD_MEMBERSHIP");
		      exit(1);
		    }
		  }
		} 
	}
	return gp;
}

/*
 * igmp_group_t* igmp_interface_group_lookup()
 *
 * Lookup a group in the grouptable of an interface
 */
igmp_group_t*
igmp_interface_group_lookup(
	igmp_interface_t *ifp,
	struct in_addr groupaddr)
{
	igmp_group_t* gp;

	assert(ifp != NULL);
	for (gp = ifp->igmpi_groups; gp; gp = gp->igmpg_next)
		if (gp->igmpg_addr.s_addr == groupaddr.s_addr)
			return gp;
	return NULL; 
}

/*
 * void igmp_interface_membership_report_v12()
 *
 * Process a v1/v2 membership report
 */
void
igmp_interface_membership_report_v12(
	igmp_router_t* router,	
	igmp_interface_t* ifp,
	struct in_addr src,
	igmpr_t* report,
	int len)
{
	igmp_group_t* gp;
	igmp_rep_t *rep;

	/* Ignore a report for a non-multicast address */ 
	if (! IN_MULTICAST(ntohl(report->igmpr_group.s_addr)))
		return;
	/* Find the group, and if not present, add it */
	if ((gp = igmp_interface_group_add(router,ifp, report->igmpr_group)) == NULL)
		return;
    	/* find the member and add it if not present*/
	rep=igmp_group_rep_add(gp,src);

	/* Consider this to be a v3 is_ex{} report */
	igmp_group_handle_isex(router,ifp, gp, 0, NULL);
}
/*
 * void igmp_interface_membership_report_v3()
 *
 * Process a v3 membership report :only a group record in the report
 * len :will be used in the Baal implementation
 */
void
igmp_interface_membership_report_v3(
				    igmp_router_t* router,
				    igmp_interface_t* ifp,
				    struct in_addr src,/* addresse membre */
				    igmp_report_t* report,
				    int len)
{
	igmp_group_t* gp;
        igmp_rep_t *rep;
	u_short numsrc;
	u_short numgrps;
	u_char type;
	int i;

	/* test l'interface: si report prevenant de l'upstream,
           et @addresse membre != mon addresse 	alors forward_upstream  = 1
         */
        if ((ifp->igmpi_addr.s_addr == upstream) && (src.s_addr != upstream) && (VALID_ADDR(report->igmpr_group[0].igmpg_group)))
           forward_upstream = 1;
	numgrps = ntohs(report->igmpr_numgrps);
	for (i=0; i < numgrps;i++){
		/* Ignore a report for a non-multicast address */ 
		if (! IN_MULTICAST(ntohl(report->igmpr_group[i].igmpg_group.s_addr)))
			return;
		//printf("group :%s\n",inet_ntoa(report->igmpr_group[i].igmpg_group));
		/* Find the group, and if not present, add it */
		if ((gp = igmp_interface_group_add(router,ifp, report->igmpr_group[i].igmpg_group)) == NULL)
			return;
	        /* find the source of the report and add it if not present*/
		/*if (src.s_addr != ifp->igmpi_addr.s_addr) */
		rep=igmp_group_rep_add(gp,src);
		/* Find the group record type */
		type   = (u_char)report->igmpr_group[i].igmpg_type;
		numsrc = ntohs(report->igmpr_group[i].igmpg_numsrc);
		switch(type) {
		case 1 : /* an IS_IN report */
		  igmp_group_handle_isin(router,ifp,gp,numsrc,(struct in_addr *) &report->igmpr_group[i].igmpg_sources);
		  break;
		case 2 : /* an IS_EX report */
		  igmp_group_handle_isex(router,ifp,gp,numsrc,(struct in_addr *) &report->igmpr_group[i].igmpg_sources);
		  break;
		case 3: /* an TO_IN report */
		  igmp_group_handle_toin(router,ifp,gp,numsrc,(struct in_addr *)&report->igmpr_group[i].igmpg_sources);
		  break;
		case 4: /* an TO_EX report */
		  igmp_group_handle_toex(router,ifp,gp,numsrc,(struct in_addr *)&report->igmpr_group[i].igmpg_sources);
		  break;
		case 5: /* an ALLOW report */
		  igmp_group_handle_allow(router,ifp,gp,numsrc,(struct in_addr *)&report->igmpr_group[i].igmpg_sources);
		  break;
		case 6: /* an BLOCK report */
		  igmp_group_handle_block(router,ifp,gp,numsrc,(struct in_addr *)&report->igmpr_group[i].igmpg_sources);
		  break;
		default:
		  LOG((LOG_INFO, "igmp_interface_membership_report_v3: group record type undefined"));
		  break;
		}
	}

}


/*
 * void igmp_interface_print()
 *
 * Print status of an interface 
 */
void
igmp_interface_print(
	igmp_interface_t* ifp)
{
	igmp_group_t* gp;

	printf(" interface %s, %s ver=0x%x\n",
		inet_ntoa(ifp->igmpi_addr),(ifp->igmpi_type == UPSTREAM)?"UPSTREAM":"DOWNSTREAM", ifp->igmpi_version);
	for (gp = ifp->igmpi_groups; gp; gp = gp->igmpg_next)
		igmp_group_print(gp);
}

/******************************************************************************
 *
 * igmp router routines
 *
 *****************************************************************************/

/*
 * int igmprt_init()
 *
 * Initialize igmp router
 */
int
igmprt_init(
	igmp_router_t* igmprt)
{
	igmprt->igmprt_interfaces = NULL;
	igmprt->igmprt_thr_timer = igmprt->igmprt_thr_input = NULL;
	igmprt->igmprt_flag_timer = 0;
	igmprt->igmprt_flag_input = 0;
	/*create datagram socket to tell igmpv3 host about reports to send on upstream interface*/
	igmprt->igmprt_up_socket = socket( AF_INET, SOCK_DGRAM, 0 );
        if( igmprt->igmprt_up_socket < 0) {
	  perror("can't open upstream socket");
	  exit (1);
	}
	/*create raw socket to update mfc and vif table*/
	igmprt->igmprt_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
	if (igmprt->igmprt_socket < 0) 
	  perror("can't open igmp socket");
        forward_upstream = 0;
	return 1;
}

/*
 * void igmprt_destroy()
 *
 * Cleanup the router 
 */
void
igmprt_destroy(igmp_router_t* igmprt)
{
	igmp_interface_t *ifp, *ifp2;
	igmp_group_t *gp;
	igmp_src_t *sp;

	for (ifp = igmprt->igmprt_interfaces; ifp;) {
		ifp2 = ifp;
		ifp = ifp->igmpi_next;
		igmp_interface_cleanup(ifp2);
	}
	LOG((LOG_DETAIL, "destroy igmp router ...\n"));
	igmprt_stop(igmprt);
	
}

/*
 * interface_t* igmprt_interface_lookup()
 *
 * Lookup a group, identified by the interface address
 */
igmp_interface_t*
igmprt_interface_lookup(
	igmp_router_t* igmprt, 
	struct in_addr ifaddr)
{
	igmp_interface_t *ifp;

	for (ifp = igmprt->igmprt_interfaces; ifp; ifp = ifp->igmpi_next)
		if (ifp->igmpi_addr.s_addr == ifaddr.s_addr)
			return ifp;
	return NULL;
}

/*
 * interface_t* igmprt_interface_lookup_index()
 *
 * Lookup a group, identified by the interface index
 */
igmp_interface_t*
igmprt_interface_lookup_index(
	igmp_router_t* igmprt, 
	int ifp_index)
{
	igmp_interface_t *ifp;

	for (ifp = igmprt->igmprt_interfaces; ifp; ifp = ifp->igmpi_next)
		if (ifp->igmpi_index == ifp_index)
			return ifp;
	return NULL;
}

/*
 * igmp_entry_t* igmprt_group_lookup() 
 *
 * Lookup a group, identified by the interface and group address
 */
igmp_group_t*
igmprt_group_lookup(
	igmp_router_t* igmprt, 
	struct in_addr ifaddr, 
	struct in_addr groupaddr)
{
	igmp_interface_t *ifp; 

	if (ifp = igmprt_interface_lookup(igmprt, ifaddr))
		return igmp_interface_group_lookup(ifp, groupaddr);
	return NULL;
}

/*
 * struct igmp_interface_t* igmprt_interface_add()
 *
 * Add an interface to the interfacetable
 */
igmp_interface_t*
igmprt_interface_add(
	igmp_router_t* igmprt, 
	struct in_addr ifaddr,
	char *ifname,
	vifi_t index)
{
	igmp_interface_t *ifp;

	/* Return the interface if it's already in the set */
	if (ifp = igmprt_interface_lookup(igmprt, ifaddr))
		return ifp;
	/* Create the interface and add to the set */
	if (ifp = igmp_interface_create(ifaddr, ifname,index)) {
		ifp->igmpi_next = igmprt->igmprt_interfaces;
		igmprt->igmprt_interfaces = ifp;
	}
	return ifp;
}

/*
 * igmp_group_t* igmprt_group_add()
 *
 * Add a group to the grouptable of some interface
 */
igmp_group_t*
igmprt_group_add(
	igmp_router_t* igmprt, 
	struct in_addr ifaddr, 
	struct in_addr groupaddr)
{
	igmp_interface_t *ifp; 
	igmp_group_t *gp; 

	if (ifp = igmprt_interface_lookup(igmprt, ifaddr))
		return NULL;
	return igmp_interface_group_add(igmprt,ifp, groupaddr);
}

/*
 * void igmprt_timergroup(igmp_router_t* igmprt)
 *
 * Decrement timers and handle whatever has to be done when one expires
 */
void
igmprt_timer(igmp_router_t* igmprt)
{
  igmp_interface_t* ifp;
  igmp_group_t* gp;
  struct in_addr zero;
  
  zero.s_addr = 0;
  
  /* Handle every interface */
  for (ifp = igmprt->igmprt_interfaces; ifp; ifp = ifp->igmpi_next) {
    /* If we're the querier for this network, handle all querier 
     * duties */
    if (ifp->igmpi_isquerier == TRUE) {
      /* Deal with the general query */
      if (--ifp->igmpi_ti_qi <= 0) {
	ifp->igmpi_ti_qi = ifp->igmpi_qi;
	igmprt_membership_query(igmprt, ifp, &zero, NULL, 0, 0);
      }
    }else{
     /* If not the querier, deal with other-querier-present timer*/
     igmprt_timer_querier(ifp);	
    }
    /*handle group timer*/
    /*LOG((LOG_DEBUG, "handle timer group\n"));*/
    igmprt_timer_group(igmprt,ifp);
    /*handle source timer*/
    /*LOG((LOG_DEBUG, "handle timer source\n"));*/
    igmprt_timer_source(igmprt,ifp);
    /*handle scheduled query*/
    send_sh_query(igmprt,ifp);
   	
  }
}

/*
 * void* igmprt_timer_thread(void* arg)
 *
 * The thread that deals with the timer 
 */
void*
igmprt_timer_thread(void* arg)
{
  igmp_router_t* igmprt = (igmp_router_t*) arg;
  
  while (igmprt->igmprt_flag_timer) {
    igmprt_timer(igmprt);
    /* Should be changed to take care of drifting */
    usleep(1000000);
  }
  pthread_exit(NULL);
  return NULL;
}

/*
 * void igmprt_input()
 *
 * Process an incoming igmp message
 */
void
igmprt_input(igmp_router_t* igmprt, igmp_interface_t* ifp)
{

	/*struct msghdr msg;
	struct iovec iov;
	struct cmsghdr *cmsg;
	char *ctrl = (char *)malloc(MAXCTRLSIZE);*/
	
	int if_index;
	struct sockaddr_in sin;
	struct ip *iph;
	unsigned char *ptype;
	int n, len, igmplen;
	igmp_report_t *report;
	igmpv3q_t *query;
        struct in_addr src;
	int srsp;
	igmp_interface_t *ifpi;

	/* Read the igmp message */
	/*iov.iov_base = (char *)ifp->igmpi_buf;
	iov.iov_len = ifp->igmpi_bufsize;
	msg.msg_iov = &iov;
	msg.msg_iovlen= 1;
	msg.msg_control = ctrl;
	msg.msg_controllen = MAXCTRLSIZE;*/

	/*n=recvmsg(ifp->igmpi_socket,&msg,0);*/

	//for(cmsg=CMSG_FIRSTHDR(&msg); cmsg != NULL;cmsg =CMSG_NXTHDR(&msg,cmsg)) {
		/*if (cmsg->cmsg_type == IP_RECVIF){
			if_index = CMSG_IFINDEX(cmsg);*//* returns 1 or 2*/
			//if_index --; /* for us interfaces are 0 or 1*/

		//}
	//}
	
	len = sizeof(sin);
	n = recvfrom(ifp->igmpi_socket, ifp->igmpi_buf, ifp->igmpi_bufsize, 0,
		(struct sockaddr*)&sin, &len);

	if (n <= sizeof(*iph))
		return;

	/* Set pointer to start of report */
	iph = (struct ip*) ifp->igmpi_buf;
	if ((igmplen = n - (iph->ip_hl << 2)) < IGMP_MINLEN)
		return;
        src=iph->ip_src;
 	ptype = ifp->igmpi_buf + (iph->ip_hl << 2);
	/*lookup the network interface from which the packet arrived*/
	if_index =  ifp->igmpi_index;
	ifpi = igmprt_interface_lookup_index(igmprt,if_index);
	/* Handle the message */
	switch (*ptype) {
	case IGMP_MEMBERSHIP_QUERY:
		query = (igmpv3q_t *)(ifp->igmpi_buf + (iph->ip_hl << 2));
		srsp=IGMP_SRSP(query);
		if (query->igmpq_code == 0){
			/*version 1 query*/
			LOG((LOG_DEBUG, "igmpv1 query \n"));
			receive_membership_query(ifpi,query->igmpq_group,NULL,src.s_addr,0,srsp);
		}else if (igmplen == 8){
			/*version 2 query*/
			LOG((LOG_DEBUG, "igmpv2 query \n"));
			receive_membership_query(ifpi,query->igmpq_group,NULL,src.s_addr,0,srsp);
		}else if (igmplen >= 12){
			/*version 3 query*/
			receive_membership_query(ifpi,query->igmpq_group,query->igmpq_sources,src.s_addr,query->igmpq_numsrc,srsp);
		}
		break;
	case IGMP_V1_MEMBERSHIP_REPORT:
	case IGMP_V2_MEMBERSHIP_REPORT:
		igmp_interface_membership_report_v12(igmprt,ifpi,src, (igmpr_t*) ptype, igmplen);
		break;
	case IGMP_V3_MEMBERSHIP_REPORT:
	  //printf("report source %s",inet_ntoa(src));
	  report = (igmp_report_t *)(ifpi->igmpi_buf + (iph->ip_hl << 2));
	  igmp_interface_membership_report_v3(igmprt,ifpi,src,report,sizeof(report));
	        break;
	default:
		break;
	}
}

/*
 * void* igmprt_input_thread(void* arg)
 *
 * Wait on all interfaces for packets to arrive
 */
void*
igmprt_input_thread(void* arg)
{
	igmp_router_t* igmprt = (igmp_router_t*) arg;
	igmp_interface_t* ifp;
	fd_set allset, rset;
	int n, maxfd;

	/* Add the sockets from all interfaces to the set */
	FD_ZERO(&allset);
	maxfd = 0;
	for (ifp = igmprt->igmprt_interfaces; ifp; ifp = ifp->igmpi_next) {
		FD_SET(ifp->igmpi_socket, &allset);
		if (maxfd < ifp->igmpi_socket)
			maxfd = ifp->igmpi_socket;
	}
	if (maxfd == 0) {
		LOG((LOG_INFO, "error: no interfaces available to wait for input\n"));
		return NULL;
	}
	/* Wait for data on all sockets */
	while (1) {
		FD_COPY(&allset, &rset);
		n = select(maxfd+1, &rset, NULL, NULL, NULL);
		if (n == 0)
			sleep(1);
		for (ifp = igmprt->igmprt_interfaces; ifp; ifp = ifp->igmpi_next) {
			if (FD_ISSET(ifp->igmpi_socket, &rset))
				igmprt_input(igmprt, ifp);
			if (--n == 0)
				break;
		} 
	}
	return NULL;
}

/*
 * void igmprt_start(igmp_router_t* igmprt)
 *
 * Start the threads of this router 
 */
void
igmprt_start(igmp_router_t* igmprt)
{
	int err;

	/* Return if already running */
	if (igmprt->igmprt_running)
		return;

	/* Create and start the timer handling (thread) */
	igmprt->igmprt_flag_timer = 1;
	igmprt->igmprt_thr_timer = NULL;
	if ((err = pthread_create(&igmprt->igmprt_thr_timer, NULL, 
		igmprt_timer_thread, (void*) igmprt)) != 0)
		printf("couldn't start timer thread\n");

	/* Create and start input handling (thread) */
	igmprt->igmprt_flag_input = 1;
	igmprt->igmprt_thr_input = NULL;
	if ((err = pthread_create(&igmprt->igmprt_thr_input, NULL, 
		igmprt_input_thread, (void*) igmprt)) != 0)
		printf("couldn't start input thread\n");

	igmprt->igmprt_running = 1;
}

/*
 * void igmprt_stop(igmp_router_t* igmprt)
 *
 * Stop the threads of this router 
 */
void
igmprt_stop(igmp_router_t* igmprt)
{
	igmp_interface_t* ifp;
	int i=0;

	/* Return if not running */ 
	if (! igmprt->igmprt_running)
		return;

	k_stop_proxy(igmprt->igmprt_socket);

	/* Signal threads to stop */
	igmprt->igmprt_flag_timer = 0;
	igmprt->igmprt_flag_input = 0;

	/* Wait for the threads to finish */
	/* XXX Should use attach */ 
	igmprt->igmprt_running = 0;
	/* Make sure select of  input-thread wakes up */	
	/*if ((ifp = igmprt->igmprt_interfaces))
		write(ifp->igmpi_socket, &i, sizeof(i));*/
	sleep(3);
}

/*
 * void igmprt_print()
 *
 * Print the status of the igmpv3 proxy/router
 */
void
igmprt_print(igmp_router_t* igmprt)
{
	igmp_interface_t* ifp;

	assert(igmprt != NULL);
	printf("igmp router:\n");
	for (ifp = igmprt->igmprt_interfaces; ifp; ifp = ifp->igmpi_next)
		igmp_interface_print(ifp);
}

/******************************************************************************
 *
 * The application
 *
 *****************************************************************************/

#define BUF_CMD	100

igmp_router_t router;
int go_on = 1;
char* pidfile = DEFAULT_PID_FILE_NAME;

void
write_pid()
{
	FILE *fp = fopen(pidfile, "w");
	if (fp) {
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
	}
}

void
done(int sig)
{
	igmprt_destroy(&router);
	unlink(pidfile);
	exit(0);
}

void parse_option(void)
{
  
  FILE *f;
  char linebuf[100];
  char *w, *s;
  int option;
  
  if ((f=fopen(DEFAULT_CONF_FILE_PATH,"r")) == NULL)
    printf("can't open config file DEFAULT_CONF_FILE_PATH\n");
  
  while(fgets(linebuf,sizeof(linebuf),f) != NULL){
    s=linebuf;
    w=(char *)next_word(&s);
    option=wordToOption(w);
    switch(option){
    case EMPTY:
      continue;
      break;
    case IGMPVERSION:
      printf("version :");
      if (EQUAL((w=(char *)next_word(&s))," ")){
	version= DEFAULT_VERSION;
	printf("version set to default :%d\n",DEFAULT_VERSION);
      }
      else if(sscanf(w,"%d",&version) == 1){
	printf("%d\n",version);
      }
      break;
    case IS_QUERIER:
      printf("isquerier : ");
      if (EQUAL((w=(char *)next_word(&s))," ")){
	querier=DEFAULT_ISQUERIER;
	printf("is_querier set to default :%d\n",DEFAULT_ISQUERIER);
      }
      else if(sscanf(w,"%d",&querier) == 1)
	printf("%d\n",querier);
	break;
     case UPSTREAM:
        w=(char *)next_word(&s);
        upstream = inet_addr(w);
	break;
    default:
      printf("unknown option\n");

    }
  }
}

int
main(int argc, char *argv[])
{
	struct in_addr addr;
	struct sockaddr_in *psin;
	interface_list_t *ifl, *ifp;
	char cmd[BUF_CMD];
	char *i,*group,*str1;
	igmp_group_t *gp;
	struct in_addr gr,interface;
	vifi_t vifi;

	/* log_level = LOG_INFO; */
	log_level = LOG_DEBUG;
	
	/* Initialize */
	signal(SIGUSR1, done);
	signal(SIGKILL, done);
	signal(SIGABRT, done);
	signal(SIGTERM, done);
	signal(SIGHUP, done);
	write_pid();
	/*parse option on config file*/
	parse_option();
	/* Create and initialize the router */
	igmprt_init(&router);
	k_init_proxy(((igmp_router_t *) &router)->igmprt_socket);
	numvifs = 0;
	/* Add all the multicast enabled ipv4 interfaces */
	ifl = get_interface_list(AF_INET, IFF_MULTICAST, IFF_LOOPBACK);
	for (vifi=0,ifp=ifl;ifp;ifp=ifp->ifl_next,vifi++) {
		psin = (struct sockaddr_in*) &ifp->ifl_addr;
		igmprt_interface_add(&router, psin->sin_addr, ifp->ifl_name,vifi);
		k_proxy_add_vif(((igmp_router_t *) &router)->igmprt_socket,psin->sin_addr.s_addr,vifi);
		numvifs++;
	}
	free_interface_list(ifl);
	/* Print the status of the router */
	igmprt_print(&router);
	/* Start the router */
	igmprt_start(&router);
       	while (go_on) {
		/* Read and process commands */
		printf("> ");
		fflush(stdout);
		if (fgets(cmd, BUF_CMD, stdin) != NULL)
			go_on=process_cmd(cmd);
	}

	/* Done */
	done(0);
	return 0;
}
void print_usage()
{
	printf("g adress multicast:show a group state\n");
	printf("s adress unicast  :show a source state\n");
	printf("a		  :show the IGMP table\n");
	printf("q		  :quit\n");
}
/* Process a command */
int process_cmd (cmd)
	char *cmd;
{
	char *line;
	char group[20],i[20],source[20];
	igmp_group_t *gp;
	igmp_src_t *src;
	struct in_addr interface,gr,srcaddr;
	igmp_interface_t *ifp;
	/* skip white spaces */
	line = cmd + 1;
	while (isblank(*line)) line ++;
	switch (*cmd) {
		case '?':
			print_usage();
			return 1;
		case 'q':
			return 0;
		case 'g':
		   /* print a group details*/
		   if (sscanf(line,"%s",group) != 1){
			printf("-1\n");
			return 1;
		   }
		  /* interface.s_addr=inet_addr(i);*/
		   gr.s_addr=inet_addr(group);
		   for (ifp=(&router)->igmprt_interfaces;ifp;ifp=ifp->igmpi_next){
	 	   	gp=igmprt_group_lookup(&router,ifp->igmpi_addr,gr);
			if (gp != NULL){ 
				printf("interface: %s\n",inet_ntoa(ifp->igmpi_addr));	
	           		igmp_group_print(gp);
			} else
			   printf("unknown group\n");
		   }
	           return 1;
		case 's':
		    /* print a source of a group details*/
		    if (sscanf(line,"%s",source) !=1){	
			printf("-1\n");
			return 1;
		    }
		    gr.s_addr=inet_addr(group);
		    srcaddr.s_addr=inet_addr(source);	
		    for (ifp=(&router)->igmprt_interfaces;ifp;ifp=ifp->igmpi_next)
			for (gp=ifp->igmpi_groups;gp;gp=gp->igmpg_next)				
				if ((src=igmp_group_src_lookup(gp,srcaddr)) != NULL){
					printf("interface : %s\n",inet_ntoa(ifp->igmpi_addr));
					printf("\tgroup : %s\n",inet_ntoa(gp->igmpg_addr));
					igmp_src_print(src);
				}
		    return 1;
	
	       case 'a' :
		 /*print all details of the router*/
		 /*igmprt_print(&router);   with this more details*/
		 igmp_info_print(&router);  /* but this more beautiful*/
		return 1;
	       default	:
		  printf("-1\n");
		  return 1;
               }
  return 1;
}		



