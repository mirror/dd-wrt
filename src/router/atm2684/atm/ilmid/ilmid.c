/*
 * ilmi.c - ILMI demon
 *
 * Written by Scott W. Shumate
 * 
 * Copyright (c) 1995-97 All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * I ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIM ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 */

/* Change Log:
 * 1-30-97 Added VERIFY state to allow resending of GetNext
 *         PDUs after a ColdStart.  This addresses a bug that
 *         some switches have with receiving the GetNext too
 *         soon after the ColdStart.  Also added state IDLE
 *         for a more configurable poll period.
 */

#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"
#include "message.h"
#include "util.h"
#include "io.h"
#include "atmd.h"
#include "mib.h"
#include "atmf_uni.h"
#include "sysgroup.h"

#define RESPONSE_TIMEOUT 2
#define POLL_PERIOD 15
#define COMPONENT "ILMI"

typedef enum IlmiState { NO_CHANGE, RESTART, VERIFY, NET_PREFIX,
                         ADDRESS, POLL, IDLE } IlmiState;
const char *state2text[7] = { "NO_CHANGE", "RESTART", "VERIFY", "NET_PREFIX",
                              "ADDRESS", "POLL", "IDLE" };
void usage(char *name);
void ilmi_loop(int fd, int itf);

void usage(char *name)
{
  fprintf(stderr, "usage: %s [-b] [-d] [-v] [-l logfile] [-x] [-q qos]\n"
    "%13s[ -i local_ip ] [ -u uni ] <interface>\n",name,"");
  fprintf(stderr, "%6s %s -V\n","",name);
  exit(1);
}

void ilmi_loop(int fd, int itf)
{
  int no_response;
  long int requestID;
  Message *poll_message, *set_message, *coldstart_message, *in_message;
  AsnOid *set_oid, *netprefix_oid, *esi_oid;
  VarBind *varbind;
  struct timeval timeout;
  IlmiState state, new_state;

  resetUpTime();
  poll_message = create_poll_message();
  set_message = create_set_message();
  set_oid = &((VarBind *) FIRST_LIST_ELMT(set_message->data->a.set_request->variable_bindings))->name;
  esi_oid = get_esi(fd,itf);
  coldstart_message = create_coldstart_message();
  in_message = alloc_t(Message);

  /* Address registration state machine */
  new_state = RESTART;
  for(;;)
    {
      state = new_state;
      diag(COMPONENT, DIAG_DEBUG, "entering state %s", state2text[state]);

      /* Output for the current state */
      switch(state)
	{
	case RESTART:
	  deleteNetPrefix();
	  diag(COMPONENT, DIAG_INFO, "sending cold-start");
	  coldstart_message->data->a.trap->time_stamp = accessUpTime();
          send_message(fd, coldstart_message);
          no_response = 0;

        case VERIFY:
	  diag(COMPONENT, DIAG_INFO, "sending get-next");
	  poll_message->data->a.get_next_request->request_id = ++requestID;
	  send_message(fd, poll_message);
	  break;

	case ADDRESS:
	  diag(COMPONENT, DIAG_INFO, "setting the atm address on the switch");
	  set_message->data->a.set_request->request_id = ++requestID;
	  send_message(fd, set_message);
	  break;

	case POLL:
	  diag(COMPONENT, DIAG_INFO, "sending get-next request");
	  poll_message->data->a.get_next_request->request_id = ++requestID;
	  send_message(fd, poll_message);
	  break;
	}

      /* Set the time-out period */
      if(state == IDLE)
        timeout.tv_sec = POLL_PERIOD;
      else
        timeout.tv_sec = RESPONSE_TIMEOUT;
      timeout.tv_usec = 0;

      new_state = NO_CHANGE;
      /* Input handling loop */
      while(new_state == NO_CHANGE)
	{
	  ResetNibbleMem();
	  if(wait_for_message(fd, &timeout) &&
	     !read_message(fd, in_message))
	      {
		switch(in_message->data->choiceId)
		  {
		  case PDUS_GET_REQUEST:
		    diag(COMPONENT, DIAG_INFO, "received get request");
		    MIBget(in_message->data->a.get_next_request->variable_bindings,
			   &in_message->data->a.get_next_request->error_status,
			   &in_message->data->a.get_next_request->error_index);
		    in_message->data->choiceId = PDUS_GET_RESPONSE;
		    send_message(fd, in_message);
		    break;
		    
		  case PDUS_GET_NEXT_REQUEST:
		    diag(COMPONENT, DIAG_INFO, "received get-next request");
		    MIBgetnext(in_message->data->a.get_next_request->variable_bindings,
			       &in_message->data->a.get_next_request->error_status,
			       &in_message->data->a.get_next_request->error_index);
		    in_message->data->choiceId = PDUS_GET_RESPONSE;
		    send_message(fd, in_message);
		    break;
		    
		  case PDUS_GET_RESPONSE:
		    diag(COMPONENT, DIAG_INFO, "received get response");
		    if(in_message->data->a.get_response->request_id == requestID)
		      {
			varbind = (VarBind *) FIRST_LIST_ELMT(in_message->data->a.get_response->variable_bindings);
			switch(state)
			  {
			  case RESTART:
                          case VERIFY:
			    if(in_message->data->a.get_response->error_status == NOERROR &&
			       AsnOidCompare(&atmAddressStatus, &varbind->name) == AsnOidRoot)
			      new_state = RESTART;
			    else
			      new_state = NET_PREFIX;
			    break;

			  case ADDRESS:
			    if(in_message->data->a.get_response->error_status == NOERROR)
			      {
				diag(COMPONENT, DIAG_INFO, "ATM address registered");
				update_nsap(itf, netprefix_oid, esi_oid);
				no_response = 0;
				new_state = POLL;
			      }
			    else
			      new_state = RESTART;			    
			    break;
			    
			  case POLL:
			    if(in_message->data->a.get_response->error_status == NOERROR &&
			       AsnOidCompare(&varbind->name, set_oid) == AsnOidEqual)
                            {
			       no_response = 0;
                               new_state = IDLE;
                            }
			    else
			      new_state = RESTART;
			    break;
			  }		      
		      }
		    else diag(COMPONENT, DIAG_ERROR, "received response with invalid request id");
		    break;
		    
		  case PDUS_SET_REQUEST:
		    diag(COMPONENT, DIAG_INFO, "received set request");
		    MIBset(in_message->data->a.set_request->variable_bindings,
			   &in_message->data->a.set_request->error_status,
			   &in_message->data->a.set_request->error_index);
		    in_message->data->choiceId = PDUS_GET_RESPONSE;
		    send_message(fd, in_message);
		    break;
		    
		  case PDUS_TRAP:
		    diag(COMPONENT, DIAG_INFO, "received trap");
		    if(in_message->data->a.trap->generic_trap == COLDSTART &&
		       state != RESTART)
		      new_state = RESTART;
		    break;
		  default:
		    diag(COMPONENT, DIAG_ERROR, "received message with invalid choice");
		    break;
		  }
		if((state == NET_PREFIX || new_state == NET_PREFIX) &&
		   (netprefix_oid = accessNetPrefix()) != NULL)
		  {
		    diag(COMPONENT, DIAG_INFO, "switch registered a network prefix");
		    set_oid->octetLen = ADDRESS_LEN + 1;
		    AsnOidAppend(set_oid, netprefix_oid);
		    AsnOidAppend(set_oid, esi_oid);
		    new_state = ADDRESS;
		  }
	      }
	  else /* Timeout occurred */
	    {
	      switch(state)
		{
		case RESTART:
                case VERIFY:
		  diag(COMPONENT, DIAG_INFO, "switch did not respond to get-next -- resending");
                  if(++no_response == 4)
                    new_state = RESTART;
                  else
		    new_state = VERIFY;
		  break;
		case NET_PREFIX:
		  diag(COMPONENT, DIAG_INFO, "switch did not register a network prefix -- restarting");
		  new_state = RESTART;
		  break;
		case ADDRESS:
		  diag(COMPONENT, DIAG_INFO, "switch did not respond to set request -- resending");
		  new_state = ADDRESS;
		  break;		  
		case POLL:
		  if(++no_response == 4)
		    {
		      new_state = RESTART;
		      diag(COMPONENT, DIAG_INFO, "switch is not responding");
		    }
		  else
		    new_state = IDLE;
		  break;
                case IDLE:
                  new_state = POLL;
                  break;
		}
	    }
	}
    }
}

int main(int argc, char *argv[])
{
  int fd, opt, itf = 0, bg = 0;
  pid_t pid;
  const char *qos;

  set_application("ilmid");
  set_verbosity(NULL, DIAG_WARN);
  qos = NULL;
  while((opt = getopt(argc, argv, "bdhi:l:q:xvu:V")) != EOF)
    switch (opt)
      {
      case 'd':
	set_verbosity(NULL, DIAG_INFO);
	break;
      case 'v':
        set_verbosity(NULL, DIAG_DEBUG);
        break;
      case 'b':
	bg = 1;
	break;
      case 'i':
	{
	  uint32_t ip;

	  ip = text2ip(optarg,NULL,T2I_ERROR);
	  if (!ip) return 1;
	  set_local_ip(ip);
	}
	break;
      case 'l':
	set_logfile(optarg);
	break;
      case 'q':
	qos = optarg;
	break;
      case 'x':
	no_var_bindings = 1;
	break;
      case 'u':
	{
	  int version;

	  if (!strcmp(optarg,"3.0") || !strcmp(optarg,"30"))
	    version = 2; // version3point0(2)
	  else if (!strcmp(optarg,"3.1") || !strcmp(optarg,"31"))
	    version = 3; // version3point1(3)
	  else if (!strcmp(optarg,"4.0") || !strcmp(optarg,"40"))
	    version = 4; // version4point0(4)
	  else usage(argv[0]);
#ifdef DYNAMIC_UNI
	  atmfAtmLayerUniVersionValue = version;
#else
	  if (atmfAtmLayerUniVersionValue != version) {
	    fprintf(stderr,"UNI version not available\n");
	    exit(1);
	  }
#endif
	}
	break;
      case 'V':
	printf("%s\n",VERSION);
	return 0;
      case 'h':
      default:
	usage(argv[0]);
      }
  if(argc == optind + 1)
    itf = atoi(argv[optind++]);

  if(argc != optind)
    usage(argv[0]);

  diag(COMPONENT, DIAG_INFO, "Linux ATM ILMI, version %s", VERSION);

  if(bg)
    {
      pid = fork();
      if(pid < 0)
	diag(COMPONENT, DIAG_FATAL, "fork: %s", strerror(errno));
      if(pid > 0) exit(0);
    }
  
  InitNibbleMem(512, 512);
  fd = open_ilmi(itf,qos);
  ilmi_loop(fd, itf);
  close(fd);
  return 0;
}
