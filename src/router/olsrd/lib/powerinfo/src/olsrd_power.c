
/*
 * Copyright (c) 2004, Andreas Tønnesen(andreto-at-olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the UniK olsr daemon nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* $Id: olsrd_power.c,v 1.15 2005/12/30 02:24:00 tlopatic Exp $ */

/*
 * Dynamic linked library example for UniK OLSRd
 */

#include "olsrd_power.h"
#include "olsrd_plugin.h"

#include "olsr.h"
#include "mantissa.h"
#include "parser.h"
#include "scheduler.h"
#include "link_set.h"
#include "socket_parser.h"
#include "interfaces.h"
#include "duplicate_set.h"
#include "apm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef OS
#undef OS
#endif

#ifdef WIN32
#define close(x) closesocket(x)
#define OS "Windows"
#endif
#ifdef linux
#define OS "GNU/Linux"
#endif
#ifdef __FreeBSD__
#define OS "FreeBSD"
#endif

#ifndef OS
#define OS "Undefined"
#endif

/* The database - (using hashing) */
static struct pwrentry list[HASHSIZE];


static int has_apm;

/* set buffer to size of IPv6 message */
static char buffer[sizeof(struct olsrmsg6)];

int ipc_socket;
int ipc_open;
int ipc_connection;
int ipc_connected;

int
ipc_send(char *, int);


/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */
int
olsrd_plugin_init()
{
  int i;
  struct olsr_apm_info apm_info;

  if(olsr_cnf->ip_version != AF_INET)
    {
      fprintf(stderr, "This plugin only supports IPv4!\n");
      return 0;
    }
  /* Initial IPC value */
  ipc_open = 0;

  /* Init list */
  for(i = 0; i < HASHSIZE; i++)
    {
      list[i].next = &list[i];
      list[i].prev = &list[i];
    }

  if(apm_read(&apm_info) < 0)
    {
      has_apm = 0;
      olsr_printf(1, "No APM info avalible! This node will not generate powermessages!\n\n");
    }
  else
    {
      olsr_printf(1, "Node has APM info!\n");
      has_apm = 1;
    }

  /* Register functions with olsrd */
  olsr_parser_add_function(&olsr_parser, PARSER_TYPE, 1);

  olsr_register_timeout_function(&olsr_timeout);

  olsr_register_scheduler_event(&olsr_event, NULL, EMISSION_INTERVAL, 0, NULL);

  return 1;
}

int
plugin_ipc_init()
{
  struct sockaddr_in sin;
  olsr_u32_t yes = 1;

  /* Init ipc socket */
  if ((ipc_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
      perror("IPC socket");
      return 0;
    }
  else
    {
      if (setsockopt(ipc_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) 
      {
	perror("SO_REUSEADDR failed");
	return 0;
      }

#if defined __FreeBSD__ && defined SO_NOSIGPIPE
      if (setsockopt(ipc_socket, SOL_SOCKET, SO_NOSIGPIPE, (char *)&yes, sizeof(yes)) < 0) 
      {
	perror("SO_REUSEADDR failed");
	return 0;
      }
#endif

      /* Bind the socket */
      
      /* complete the socket structure */
      memset(&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = INADDR_ANY;
      sin.sin_port = htons(IPC_PORT);
      
      /* bind the socket to the port number */
      if (bind(ipc_socket, (struct sockaddr *) &sin, sizeof(sin)) == -1) 
	{
	  perror("IPC bind");
	  return 0;
	}
      
      /* show that we are willing to listen */
      if (listen(ipc_socket, 1) == -1) 
	{
	  perror("IPC listen");
	  return 0;
	}

      /* Register with olsrd */
      add_olsr_socket(ipc_socket, &ipc_action);

    }

  ipc_open = 1;
  return 1;
}

void
ipc_action(int fd)
{
  struct sockaddr_in pin;
  socklen_t addrlen;
  char *addr;  

  addrlen = sizeof(struct sockaddr_in);

  if ((ipc_connection = accept(ipc_socket, (struct sockaddr *)  &pin, &addrlen)) == -1)
    {
      perror("IPC accept");
      exit(1);
    }
  else
    {
      addr = inet_ntoa(pin.sin_addr);
      if(ntohl(pin.sin_addr.s_addr) != INADDR_LOOPBACK)
	{
	  olsr_printf(1, "Front end-connection from foregin host(%s) not allowed!\n", addr);
	  close(ipc_connection);
	  return;
	}
      else
	{
	  ipc_connected = 1;
	  olsr_printf(1, "POWER: Connection from %s\n",addr);
	}
    }

}

/*
 * destructor - called at unload
 */
void
olsr_plugin_exit()
{
  if(ipc_open)
    close(ipc_socket);
}


/**
 *A timeoutfunction called every time
 *the scheduler is polled
 */
void
olsr_timeout()
{
  //printf("PLUGIN Timeout!\n");
  struct pwrentry *tmp_list;
  struct pwrentry *entry_to_delete;
  int index;


  for(index=0;index<HASHSIZE;index++)
    {
      tmp_list = list[index].next;
      /*Traverse MID list*/
      while(tmp_list != &list[index])
	{
	  /*Check if the entry is timed out*/
	  if(olsr_timed_out(&tmp_list->timer))
	    {
	      entry_to_delete = tmp_list;
	      tmp_list = tmp_list->next;
	      olsr_printf(1, "POWER info for %s timed out.. deleting it\n", 
			  olsr_ip_to_string(&entry_to_delete->originator));
	      /* Dequeue */
	      entry_to_delete->prev->next = entry_to_delete->next;
	      entry_to_delete->next->prev = entry_to_delete->prev;

	      /* Delete */
	      free(entry_to_delete);
	    }
	  else
	      tmp_list = tmp_list->next;
	}
    }

  return;
}


/**
 *Scheduled event
 */
void
olsr_event(void *foo)
{
  union p_olsr_message *message = (union p_olsr_message*)buffer;
  struct interface *ifn;

  /* If we can't produce power info we do nothing */ 
  if(!has_apm)
    return;

  olsr_printf(3, "PLUG-IN: Generating package - ");

  /* looping trough interfaces */
  for (ifn = ifnet; ifn ; ifn = ifn->int_next) 
    {
      olsr_printf(3, "[%s]  ", ifn->int_name);
      /* Fill message */
      if(olsr_cnf->ip_version == AF_INET)
	{
	  /* IPv4 */
	  message->v4.olsr_msgtype = MESSAGE_TYPE;
	  message->v4.olsr_vtime = double_to_me(7.5);
	  message->v4.olsr_msgsize = htons(sizeof(struct olsrmsg));
	  memcpy(&message->v4.originator, &main_addr, ipsize);
	  message->v4.ttl = MAX_TTL;
	  message->v4.hopcnt = 0;
	  message->v4.seqno = htons(get_msg_seqno());
	  
	  get_powerstatus(&message->v4.msg);

	  if(net_outbuffer_push(ifn, (olsr_u8_t *)message, sizeof(struct olsrmsg)) != sizeof(struct olsrmsg))
	    {

	      /* Send data and try again */
	      net_output(ifn);
	      if(net_outbuffer_push(ifn, (olsr_u8_t *)message, sizeof(struct olsrmsg)) != sizeof(struct olsrmsg))
		olsr_printf(1, "Powerplugin: could not write to buffer for interface: %s\n", ifn->int_name);
	    }

	}
      else
	{
	  /* IPv6 */
	  message->v6.olsr_msgtype = MESSAGE_TYPE;
	  message->v6.olsr_vtime = double_to_me(7.5);
	  message->v6.olsr_msgsize = htons(sizeof(struct olsrmsg));
	  memcpy(&message->v6.originator, &main_addr, ipsize);
	  message->v6.ttl = MAX_TTL;
	  message->v6.hopcnt = 0;
	  message->v6.seqno = htons(get_msg_seqno());
	  
	  get_powerstatus(&message->v6.msg);

	  if(net_outbuffer_push(ifn, (olsr_u8_t *)message, sizeof(struct olsrmsg6)) != sizeof(struct olsrmsg6))
	    {
	      /* Send data and try again */
	      net_output(ifn);
	      if(net_outbuffer_push(ifn, (olsr_u8_t *)message, sizeof(struct olsrmsg6)) != sizeof(struct olsrmsg6))
		olsr_printf(1, "Powerplugin: could not write to buffer for interface: %s\n", ifn->int_name);
	    }

	}

    }
  olsr_printf(2, "\n");

  /* Try to set up IPC socket if not already up */
  if(!ipc_open)
    plugin_ipc_init();

  print_power_table();

  return;
}



void
olsr_parser(union olsr_message *m, struct interface *in_if, union olsr_ip_addr *in_addr)
{
  union p_olsr_message* pm;
  struct  powermsg *message;
  union olsr_ip_addr originator;
  double vtime;
  
  pm = (union p_olsr_message*)m;

  /* Fetch the originator of the messsage */
  memcpy(&originator, &m->v4.originator, ipsize);

  /* Fetch the message based on IP version */
  if(olsr_cnf->ip_version == AF_INET)
    {
      message = &pm->v4.msg;
      vtime = ME_TO_DOUBLE(m->v4.olsr_vtime);
    }
  else
    {
      message = &pm->v6.msg;
      vtime = ME_TO_DOUBLE(m->v6.olsr_vtime);
    }

  /* Check if message originated from this node */
  if(memcmp(&originator, &main_addr, ipsize) == 0)
    /* If so - back off */
    return;

  /* Check that the neighbor this message was received
     from is symmetric */
  if(check_neighbor_link(in_addr) != SYM_LINK)
    {
      /* If not symmetric - back off */
      olsr_printf(3, "Received POWER from NON SYM neighbor %s\n", olsr_ip_to_string(in_addr));
      return;
    }

  /* Check if this message has been processed before
   * Remeber that this also registeres the message as
   * processed if nessecary
   */
  if(!olsr_check_dup_table_proc(&originator,
                     ntohs(m->v4.seqno))) /* REMEMBER NTOHS!! */
    {
      /* If so - do not process */
      goto forward;
    }

  /* Process */

  olsr_printf(3, "POWER PLUG-IN: Processing PWR from %s seqno: %d\n",
	      olsr_ip_to_string(&originator),
	      ntohs(m->v4.seqno));

  /* Call a function that updates the database entry */
  update_power_entry(&originator, message, vtime);


 forward:
  /* Forward the message if nessecary
   * default_fwd does all the work for us!
   */
  olsr_forward_message(m,
              &originator,
              ntohs(m->v4.seqno), /* IMPORTANT!!! */
              in_if,
              in_addr);

}



/**
 *Update or register a new power entry
 */
int
update_power_entry(union olsr_ip_addr *originator, struct powermsg *message, double vtime)
{
  int hash;
  struct pwrentry *entry;

  hash = olsr_hashing(originator);

  /* Check for the entry */
  for(entry = list[hash].next;
      entry != &list[hash];
      entry = entry->next)
    {
      if(memcmp(originator, &entry->originator, ipsize) == 0)
	{
	  entry->source_type = message->source_type;
	  entry->percentage = message->percentage;
	  entry->time_left = message->time_left;

	  olsr_get_timestamp(vtime * 1000, &entry->timer);

	  return 0;
	}
    }

  olsr_printf(1, "New power entry %s: ", olsr_ip_to_string(originator));

  if(message->source_type == OLSR_BATTERY_POWERED)
    olsr_printf(1, "BATTERY P: %d%% T: %d mins\n",
	   message->percentage,
	   message->time_left);
  else
    olsr_printf(1, "AC\n");

  entry = olsr_malloc(sizeof(struct pwrentry), "POWERPLUGIN: new power entry");
     
  /* Fill struct */

  memcpy(&entry->originator, originator, ipsize);

  entry->source_type = message->source_type;
  entry->percentage = message->percentage;
  entry->time_left = message->time_left;
  
  olsr_get_timestamp(vtime * 1000, &entry->timer);

  /* Queue */
  entry->next = list[hash].next->prev;
  entry->prev = &list[hash];
  list[hash].next->prev = entry;
  list[hash].next = entry;

  return 1;
}


/**
 *Print all registered power entries
 */

void
print_power_table()
{
  int hash;
  struct pwrentry *entry;
  char buf[200];

  if(!ipc_connection)
    return;

  ipc_send("--POWERTABLE--\n", 15);

  for(hash = 0; hash < HASHSIZE; hash++)
    /* Check for the entry */
    for(entry = list[hash].next;
	entry != &list[hash];
	entry = entry->next)
      {
	sprintf(buf, "[%s]: ", olsr_ip_to_string(&entry->originator));
	ipc_send(buf, strlen(buf));

	if(entry->source_type == OLSR_BATTERY_POWERED)
	  {
	    sprintf(buf,
		    "BATTERY P: %d%% T: %d mins\n",
		    entry->percentage,
		    entry->time_left);
	    ipc_send(buf, strlen(buf));
	  }
	else
	  ipc_send("AC\n", 3);
      }

  ipc_send("--------------\n", 15);

}



int
ipc_send(char *data, int size)
{
  if(!ipc_connected)
    return 0;

#if defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__ || defined __MacOSX__
  if (send(ipc_connection, data, size, 0) < 0) 
#else
  if (send(ipc_connection, data, size, MSG_NOSIGNAL) < 0) 
#endif
    {
      //perror("send - IPC");
      olsr_printf(1, "(OUTPUT)IPC connection lost!\n");
      close(ipc_connection);
      //use_ipc = 0;
      ipc_connected = 0;
      return -1;
    }

  return 1;
}

/**
 *Fill a powermsg struct with power data
 */
int
get_powerstatus(struct powermsg *msg)
{
  struct olsr_apm_info apm_info;
  
  if(apm_read(&apm_info) < 0)
    {
      has_apm = 0;
      olsr_printf(1, "No APM info avalible! This node will not generate powermessages!\n\n");
    }

  if(apm_info.ac_line_status)
    {
      msg->source_type = OLSR_AC_POWERED;
      msg->percentage = 0;
      msg->time_left = 0;
    }
  else
    {
      msg->source_type = OLSR_BATTERY_POWERED;
      msg->percentage = apm_info.battery_percentage;
      msg->time_left = apm_info.battery_time_left;
    }

  return 1;
}




/*************************************************************
 *                 TOOLS DERIVED FROM OLSRD                  *
 *************************************************************/


/**
 *Hashing function. Creates a key based on
 *an 32-bit address.
 *@param address the address to hash
 *@return the hash(a value in the 0-31 range)
 */
olsr_u32_t
olsr_hashing(union olsr_ip_addr *address)
{
  olsr_u32_t hash;
  char *tmp;

  if(olsr_cnf->ip_version == AF_INET)
    /* IPv4 */  
    hash = (ntohl(address->v4));
  else
    {
      /* IPv6 */
      tmp = (char *) &address->v6;
      hash = (ntohl(*tmp));
    }

  //hash &= 0x7fffffff; 
  hash &= HASHMASK;

  return hash;
}



/**
 *Checks if a timer has times out. That means
 *if it is smaller than present time.
 *@param timer the timeval struct to evaluate
 *@return positive if the timer has not timed out,
 *0 if it matches with present time and negative
 *if it is timed out.
 */
int
olsr_timed_out(struct timeval *timer)
{
  return(timercmp(timer, &now, <));
}



/**
 *Initiates a "timer", wich is a timeval structure,
 *with the value given in time_value.
 *@param time_value the value to initialize the timer with
 *@param hold_timer the timer itself
 *@return nada
 */
void
olsr_init_timer(olsr_u32_t time_value, struct timeval *hold_timer)
{ 
  olsr_u16_t  time_value_sec;
  olsr_u16_t  time_value_msec;

  time_value_sec = time_value/1000;
  time_value_msec = time_value-(time_value_sec*1000);

  hold_timer->tv_sec = time_value_sec;
  hold_timer->tv_usec = time_value_msec*1000;   
}





/**
 *Generaties a timestamp a certain number of milliseconds
 *into the future.
 *
 *@param time_value how many milliseconds from now
 *@param hold_timer the timer itself
 *@return nada
 */
void
olsr_get_timestamp(olsr_u32_t delay, struct timeval *hold_timer)
{ 
  olsr_u16_t  time_value_sec;
  olsr_u16_t  time_value_msec;

  time_value_sec = delay/1000;
  time_value_msec= delay - (delay*1000);

  hold_timer->tv_sec = now.tv_sec + time_value_sec;
  hold_timer->tv_usec = now.tv_usec + (time_value_msec*1000);   
}



