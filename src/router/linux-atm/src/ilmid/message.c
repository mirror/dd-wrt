/*
 * message.c - commonly used ilmi messages
 *
 * Written by Scott W. Shumate
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdarg.h>

#include "message.h"
#include "atmf_uni.h"
#include "util.h"
#include "sysgroup.h"

int no_var_bindings = 0;

// AsnOid atmAddressStatus = {ADDRESS_LEN, ADDRESS_OID};

Message *create_va_getnext_message( int n, ... ){
	Message *m = alloc_t( Message );
	VarBind *entry;
	AsnOid *what;
	va_list ap;
	int count;

	if( m == NULL ) return NULL;
	m->version = VERSION_1;
	m->community.octs = "ILMI";
	m->community.octetLen = 4;
	m->data = alloc_t( PDUs );
	m->data->choiceId = PDUS_GET_NEXT_REQUEST;
	m->data->a.get_next_request = alloc_t( GetRequest_PDU );
	m->data->a.get_next_request->error_status = 0;
	m->data->a.get_next_request->error_index = 0;
	m->data->a.get_next_request->variable_bindings = alloc_t( VarBindList );
	AsnListInit( m->data->a.get_next_request->variable_bindings,
							sizeof( VarBind ));
	va_start( ap, n );
	count = 0;
	while( count < n ){
		what = va_arg( ap, AsnOid* );
		entry = AppendVarBind(
			m->data->a.get_next_request->variable_bindings );
		entry->name.octs = alloc( what->octetLen );
		entry->name.octetLen = what->octetLen;
		memcpy( entry->name.octs, what->octs, what->octetLen );
		entry->value = alloc_t( ObjectSyntax );
		entry->value->choiceId = OBJECTSYNTAX_SIMPLE;
		entry->value->a.simple = alloc_t( SimpleSyntax );
		entry->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
		count++;
	}
	va_end(ap);
	return m;
}

Message *create_va_get_message( int n, ... ){
	Message *m = alloc_t( Message );
	VarBind *entry;
	AsnOid *what;
	va_list ap;
	int count;

	if( m == NULL ) return NULL;
	m->version = VERSION_1;
	m->community.octs = "ILMI";
	m->community.octetLen = 4;
	m->data = alloc_t( PDUs );
	m->data->choiceId = PDUS_GET_REQUEST;
	m->data->a.get_request = alloc_t( GetRequest_PDU );
	m->data->a.get_request->error_status = 0;
	m->data->a.get_request->error_index = 0;
	m->data->a.get_request->variable_bindings = alloc_t( VarBindList );
	AsnListInit( m->data->a.get_request->variable_bindings,
							sizeof( VarBind ));
	va_start( ap, n );
	count = 0;
	while( count < n ){
		what = va_arg( ap, AsnOid* );
		entry = AppendVarBind(
			m->data->a.get_request->variable_bindings );
		entry->name.octs = alloc( what->octetLen );
		entry->name.octetLen = what->octetLen;
		memcpy( entry->name.octs, what->octs, what->octetLen );
		entry->value = alloc_t( ObjectSyntax );
		entry->value->choiceId = OBJECTSYNTAX_SIMPLE;
		entry->value->a.simple = alloc_t( SimpleSyntax );
		entry->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
		count++;
	}
	va_end(ap);
	return m;
}


Message *create_get_message( AsnOid what ){
	return create_va_get_message( 1, &what );
}

Message *create_getnext_message( AsnOid what ){
	return create_va_getnext_message( 1, &what );
}

#ifdef notdef
Message *create_poll_message(void)
{
  Message *poll_message;
  VarBind *entry;

  poll_message = alloc_t(Message);
  poll_message->version = VERSION_1;
  poll_message->community.octs = "ILMI";
  poll_message->community.octetLen = 4;
  poll_message->data = alloc_t(PDUs);
  poll_message->data->choiceId = PDUS_GET_NEXT_REQUEST;
  poll_message->data->a.get_next_request = alloc_t(GetNextRequest_PDU);
  poll_message->data->a.get_next_request->error_status = 0;
  poll_message->data->a.get_next_request->error_index = 0;
  poll_message->data->a.get_next_request->variable_bindings = alloc_t(VarBindList);

  AsnListInit(poll_message->data->a.get_next_request->variable_bindings, sizeof(VarBind));
  entry = AppendVarBind(poll_message->data->a.get_next_request->variable_bindings);
  entry->name.octs = alloc(ADDRESS_LEN);
  entry->name.octetLen = ADDRESS_LEN;
  memcpy(entry->name.octs, ADDRESS_OID, ADDRESS_LEN);
  entry->value = alloc_t(ObjectSyntax);
  entry->value->choiceId = OBJECTSYNTAX_SIMPLE;
  entry->value->a.simple = alloc_t(SimpleSyntax);
  entry->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
  entry->value->a.simple->a.number = 0;

  return poll_message;
}
#endif

Message *create_set_message(void)
{
  Message *set_message;
  VarBind *entry;

  set_message = alloc_t(Message);
  set_message->version = VERSION_1;
  set_message->community.octs = "ILMI";
  set_message->community.octetLen = 4;
  set_message->data = alloc_t(PDUs);
  set_message->data->choiceId = PDUS_SET_REQUEST;
  set_message->data->a.set_request = alloc_t(SetRequest_PDU);
  set_message->data->a.set_request->error_status = 0;
  set_message->data->a.set_request->error_index = 0;
  set_message->data->a.set_request->variable_bindings = alloc_t(VarBindList);

  AsnListInit(set_message->data->a.set_request->variable_bindings, sizeof(VarBind));
  entry = AppendVarBind(set_message->data->a.set_request->variable_bindings);
  /* Allocate enough memory to hold the largest possible address */
  entry->name.octs = alloc(ADDRESS_LEN + 41);
  entry->name.octetLen = ADDRESS_LEN + 1;
  memcpy(entry->name.octs, ADDRESS_OID, ADDRESS_LEN);
  entry->name.octs[ADDRESS_LEN] = '\24'; /* 20 octet address */
  entry->value = alloc_t(ObjectSyntax);
  entry->value->choiceId = OBJECTSYNTAX_SIMPLE;
  entry->value->a.simple = alloc_t(SimpleSyntax);
  entry->value->a.simple->choiceId = SIMPLESYNTAX_NUMBER;
  entry->value->a.simple->a.number = 1;

  return set_message;
}

Message *create_coldstart_message(void)
{
  Message *trap_message;
  VarBind *entry;

  trap_message = alloc_t(Message);
  trap_message->version = VERSION_1;
  trap_message->community.octs = "ILMI";
  trap_message->community.octetLen = 4;
  trap_message->data = alloc_t(PDUs);
  trap_message->data->choiceId = PDUS_TRAP;
  trap_message->data->a.trap = alloc_t(Trap_PDU);
  trap_message->data->a.trap->enterprise.octs = "\53\06\01\04\01\03\01\01";
  trap_message->data->a.trap->enterprise.octetLen = 8;
  trap_message->data->a.trap->agent_addr = alloc_t(NetworkAddress);
  trap_message->data->a.trap->agent_addr->choiceId = NETWORKADDRESS_INTERNET;
  trap_message->data->a.trap->agent_addr->a.internet = alloc_t(IpAddress);
  trap_message->data->a.trap->agent_addr->a.internet->octetLen = 4; 
  trap_message->data->a.trap->agent_addr->a.internet->octs = "\0\0\0\0";
  trap_message->data->a.trap->generic_trap = COLDSTART;
  trap_message->data->a.trap->specific_trap = 0;
  trap_message->data->a.trap->time_stamp = 0;
  trap_message->data->a.trap->variable_bindings = alloc_t(VarBindList);

  if (no_var_bindings)
    AsnListInit(trap_message->data->a.trap->variable_bindings,0);
  else {
    AsnListInit(trap_message->data->a.trap->variable_bindings, sizeof(VarBind));
    entry = AppendVarBind(trap_message->data->a.trap->variable_bindings);
    entry->name.octs = "\53\06\01\02\01\01\03\00";
    entry->name.octetLen = 8;
    entry->value = alloc_t(ObjectSyntax);
    entry->value->choiceId = OBJECTSYNTAX_SIMPLE;
    entry->value->a.simple = alloc_t(SimpleSyntax);
    entry->value->a.simple->choiceId = SIMPLESYNTAX_EMPTY;
    entry->value->a.simple->a.empty = '\0';
  }

  return trap_message;
}

