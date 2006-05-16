
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: olsrd_dyn_gw.c,v 1.18 2005/06/02 13:59:12 br1 Exp $
 */

/*
 * -Threaded ping code added by Jens Nachitgall
 * -HNA4 checking by bjoern riemer
 */

#include <arpa/inet.h>

#include "olsr_types.h"
#include "olsrd_dyn_gw.h"
#include "scheduler.h"
#include "olsr.h"
#include "local_hna_set.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <net/route.h>
#ifdef linux
#include <linux/in_route.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <time.h>
#ifndef WIN32
#include <pthread.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef interface

#define close(x) closesocket(x)

typedef HANDLE pthread_mutex_t;
typedef HANDLE pthread_t;

int pthread_create(HANDLE *Hand, void *Attr, void *(*Func)(void *), void *Arg);
int pthread_kill(HANDLE Hand, int Sig);
int pthread_mutex_init(HANDLE *Hand, void *Attr);
int pthread_mutex_lock(HANDLE *Hand);
int pthread_mutex_unlock(HANDLE *Hand);

struct ThreadPara
{
  void *(*Func)(void *);
  void *Arg;
};

#endif


/* set default interval, in case none is given in the config file */
static int check_interval = 5;

/* list to store the Ping IP addresses given in the config file */
struct ping_list {
  char *ping_address;
  struct ping_list *next;
};

static struct ping_list *
add_to_ping_list(char *, struct ping_list *);

//struct ping_list *the_ping_list = NULL;

struct hna_list {
  union olsr_ip_addr hna_net;
  union olsr_ip_addr hna_netmask;
  struct ping_list *ping_hosts;
  int hna_added;
  int probe_ok;
  struct hna_list *next;
};

static struct hna_list *
	add_to_hna_list(struct hna_list *,
				union olsr_ip_addr *hna_net,
				union olsr_ip_addr *hna_netmask );

struct hna_list *the_hna_list = NULL;

static void *
looped_checks(void *foo);

static int
check_gw(union olsr_ip_addr *, union olsr_ip_addr *,struct ping_list *);

static int
ping_is_possible(struct ping_list *);

   
     

/**
 * read config file parameters
 */  
int
olsrd_plugin_register_param(char *key, char *value)
{
  /* foo_addr is only used for call to inet_aton */ 
  struct in_addr foo_addr;
  int retval = -1;
  int i;
  union olsr_ip_addr temp_net;
  union olsr_ip_addr temp_netmask;
  char s_netaddr[16];
  char s_mask[16];
 
  //printf("%s():%s->%s\n",__func__,key,value);
  
  if (!strcmp(key, "Interval")) {
    if (sscanf(value, "%d", &check_interval) == 1) {
      retval = 1;
    }
  }else if (!strcmp(key, "Ping")) {
    /* if value contains a valid IPaddr, then add it to the list */
    //if (inet_aton(strdup(value), &foo_addr)) {
	if (inet_aton(value, &foo_addr)) {
	    /*if first ping without hna then assume inet gateway*/
		if (the_hna_list==NULL){
		    temp_net.v4 = INET_NET;
		    temp_netmask.v4 = INET_PREFIX;
		    if ((the_hna_list = add_to_hna_list(the_hna_list,&temp_net,&temp_netmask))==NULL)
			    return retval;
		}
		the_hna_list->ping_hosts=add_to_ping_list(value, the_hna_list->ping_hosts);
		retval = 1;
    }
  }else if (!strcmp(key, "HNA")) {
	  //192.168.1.0  255.255.255.0
	  i=sscanf(value,"%15s %15s",s_netaddr,s_mask);
	  //printf("%s():i:%i; net:%s; mask:%s\n",__func__,i,s_netaddr,s_mask);
	  if (inet_aton(s_netaddr, &foo_addr)) {
		  temp_net.v4=foo_addr.s_addr;
		  //printf("GOT: %s(%08x)",inet_ntoa(foo_addr),foo_addr.s_addr);
		  if (inet_aton(s_mask, &foo_addr)) {
			  temp_netmask.v4=foo_addr.s_addr;
			  //printf("/%s(%08x)\n",inet_ntoa(foo_addr),foo_addr.s_addr);
			  //printf("%s():got->%s/%s\n",__func__,olsr_ip_to_string((union olsr_ip_addr *)&));
			  if ((the_hna_list = add_to_hna_list(the_hna_list,&temp_net,&temp_netmask))!=NULL)
				  retval = 1;
		  }
	  }
  }
  return retval;
}


/**
 *Do initialization here
 * 
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 *It is ran _after_ register_olsr_param
 */
int
olsrd_plugin_init()
{
  pthread_t ping_thread;
  
  //gw_net.v4 = INET_NET;
  //gw_netmask.v4 = INET_PREFIX;

  //gw_already_added = 0;
  //has_available_gw = 0;

  
  /* Remove all local Inet HNA entries */
  /*while(remove_local_hna4_entry(&gw_net, &gw_netmask))
  {
    olsr_printf(1, "HNA Internet gateway deleted\n");
  }*/

  pthread_create(&ping_thread, NULL, looped_checks, NULL);
  
  /* Register the GW check */
  olsr_register_scheduler_event(&olsr_event_doing_hna, NULL, 3, 4, NULL);

  return 1;
}


/**
 * Scheduled event to update the hna table,
 * called from olsrd main thread to keep the hna table thread-safe
 */
void
olsr_event_doing_hna(void *foo)
{
	struct hna_list *li;
	/*
  if (has_available_gw == 1 && gw_already_added == 0) {
    olsr_printf(1, "Adding OLSR local HNA entry for Internet\n");
    add_local_hna4_entry(&gw_net, &gw_netmask);
    gw_already_added = 1;
  } else if ((has_available_gw == 0) && (gw_already_added == 1)) {
    // Remove all local Inet HNA entries /
    while(remove_local_hna4_entry(&gw_net, &gw_netmask)) {
      olsr_printf(1, "Removing OLSR local HNA entry for Internet\n");
    }
    gw_already_added = 0;
  }
	*/
	for(li=the_hna_list; li; li=li->next){
		if((li->probe_ok==1)&&(li->hna_added==0)){
			olsr_printf(1, "Adding OLSR local HNA entry\n");
			add_local_hna4_entry(&li->hna_net, &li->hna_netmask);
			li->hna_added=1;
		}else if((li->probe_ok==0)&&(li->hna_added==1)){
			while(remove_local_hna4_entry(&li->hna_net, &li->hna_netmask)) {
				olsr_printf(1, "Removing OLSR local HNA entry\n");
			}
			li->hna_added=0;
		}
	}
}



/**
 * the threaded function which happens within an endless loop,
 * reiterated every "Interval" sec (as given in the config or 
 * the default value)
 */
static void *
looped_checks(void *foo)
{
	/*
	struct hna_list {
  union olsr_ip_addr hna_net;
  union hna_netmask hna_netmask;
  struct ping_list *ping_hosts;
  int hna_added;
  int probe_ok;
  struct hna_list *next;
};
	*/
	struct hna_list *li;
	
  for(;;) {
    struct timespec remainder_spec;
    /* the time to wait in "Interval" sec (see connfig), default=5sec */
    struct timespec sleeptime_spec  = {(time_t) check_interval, 0L };

    li=the_hna_list;
    while(li){
	    /* check for gw in table entry and if Ping IPs are given also do pings */
	    li->probe_ok = check_gw(&li->hna_net,&li->hna_netmask,li->ping_hosts);
	    //has_available_gw = check_gw(&gw_net, &gw_netmask);
	    
	    li=li->next;
    }

    while(nanosleep(&sleeptime_spec, &remainder_spec) < 0)
      sleeptime_spec = remainder_spec;
  }
}


static int
check_gw(union olsr_ip_addr *net, union olsr_ip_addr *mask, struct ping_list *the_ping_list)
{
    char buff[1024], iface[16];
    olsr_u32_t gate_addr, dest_addr, netmask;
    unsigned int iflags;
    int num, metric, refcnt, use;
    int retval = 0;

    FILE *fp = fopen(PROCENTRY_ROUTE, "r");

    if (!fp) 
      {
        perror(PROCENTRY_ROUTE);
        olsr_printf(1, "INET (IPv4) not configured in this system.\n");
	return -1;
      }
    
    rewind(fp);

    /*
    olsr_printf(1, "Genmask         Destination     Gateway         "
                "Flags Metric Ref    Use Iface\n");
    */
    while (fgets(buff, 1023, fp))
	{	
	num = sscanf(buff, "%15s %128X %128X %X %d %d %d %128X \n",
		     iface, &dest_addr, &gate_addr,
		     &iflags, &refcnt, &use, &metric, &netmask);

	if (num < 8)
	  {
	    continue;
	  }

	/*
	olsr_printf(1, "%-15s ", olsr_ip_to_string((union olsr_ip_addr *)&netmask));

	olsr_printf(1, "%-15s ", olsr_ip_to_string((union olsr_ip_addr *)&dest_addr));

	olsr_printf(1, "%-15s %-6d %-2d %7d %s\n",
		    olsr_ip_to_string((union olsr_ip_addr *)&gate_addr),
		    metric, refcnt, use, iface);
	*/

		if( (iflags & RTF_UP) &&
		   (metric == 0) &&
		   (netmask == mask->v4) && 
		   (dest_addr == net->v4))
		{
			if ( ((mask->v4==INET_PREFIX)&&(net->v4==INET_NET))&&(!(iflags & RTF_GATEWAY)))
				return retval;
			/* don't ping, if there was no "Ping" IP addr in the config file */
			if (the_ping_list != NULL) {  
				/*validate the found inet gw by pinging*/ 
				if (ping_is_possible(the_ping_list)) {
					olsr_printf(1, "HNA[%08x/%08x](ping is possible) VIA %s detected in routing table.\n", dest_addr,netmask,iface);
					retval=1;      
				}
			} else {
				olsr_printf(1, "HNA[%08x/%08x] VIA %s detected in routing table.\n", dest_addr,netmask,iface);
				retval=1;      
			}
		}

	}//while

	fclose(fp);  
  
	if(retval == 0){
		olsr_printf(1, "HNA[%08x/%08x] is invalid\n", net->v4,mask->v4);
	}  
    return retval;
}

static int
ping_is_possible(struct ping_list *the_ping_list) 
{
  struct ping_list *list;
  for (list = the_ping_list; list != NULL; list = list->next) {
    char ping_command[50] = "ping -c 1 -q ";
    strcat(ping_command, list->ping_address);
    olsr_printf(1, "\nDo ping on %s ...\n", list->ping_address);
    if (system(ping_command) == 0) {
      olsr_printf(1, "...OK\n\n");
      return 1;      
    } else {
      olsr_printf(1, "...FAILED\n\n");
    }
  }
  return 0;
}

/* add the valid IPs to the head of the list */
static struct ping_list *
add_to_ping_list(char *ping_address, struct ping_list *the_ping_list)
{
  struct ping_list *new = (struct ping_list *) malloc(sizeof(struct ping_list));
  if(new == NULL)
  {
    fprintf(stderr, "DYN GW: Out of memory!\n");
    exit(0);
  }
  new->ping_address = strdup(ping_address);
  new->next = the_ping_list;
  return new;
}    



static struct hna_list *
add_to_hna_list(struct hna_list * list_root, union olsr_ip_addr *hna_net, union olsr_ip_addr *hna_netmask )
{
  struct hna_list *new = (struct hna_list *) malloc(sizeof(struct hna_list));
  if(new == NULL)
  {
    fprintf(stderr, "DYN GW: Out of memory!\n");
    exit(0);
  }
  //memcpy(&new->hna_net,hna_net,sizeof(union hna_net));
  //memcpy(&new->hna_netmask,hna_netmask,sizeof(union hna_netmask));
  new->hna_net.v4=hna_net->v4;
  new->hna_netmask.v4=hna_netmask->v4;
  new->hna_added=0;
  new->probe_ok=0;
  new->ping_hosts=NULL;
  new->next=list_root;  

  return new;
}



#ifdef WIN32

/*
 * Windows ptread compat stuff
 */
static unsigned long __stdcall ThreadWrapper(void *Para)
{
  struct ThreadPara *Cast;
  void *(*Func)(void *);
  void *Arg;

  Cast = (struct ThreadPara *)Para;

  Func = Cast->Func;
  Arg = Cast->Arg;
  
  HeapFree(GetProcessHeap(), 0, Para);

  Func(Arg);

  return 0;
}

int pthread_create(HANDLE *Hand, void *Attr, void *(*Func)(void *), void *Arg)
{
  struct ThreadPara *Para;
  unsigned long ThreadId;

  Para = HeapAlloc(GetProcessHeap(), 0, sizeof (struct ThreadPara));

  if (Para == NULL)
    return -1;

  Para->Func = Func;
  Para->Arg = Arg;

  *Hand = CreateThread(NULL, 0, ThreadWrapper, Para, 0, &ThreadId);

  if (*Hand == NULL)
    return -1;

  return 0;
}

int pthread_kill(HANDLE Hand, int Sig)
{
  if (!TerminateThread(Hand, 0))
    return -1;

  return 0;
}

int pthread_mutex_init(HANDLE *Hand, void *Attr)
{
  *Hand = CreateMutex(NULL, FALSE, NULL);

  if (*Hand == NULL)
    return -1;

  return 0;
}

int pthread_mutex_lock(HANDLE *Hand)
{
  if (WaitForSingleObject(*Hand, INFINITE) == WAIT_FAILED)
    return -1;

  return 0;
}

int pthread_mutex_unlock(HANDLE *Hand)
{
  if (!ReleaseMutex(*Hand))
    return -1;

  return 0;
}

#endif
