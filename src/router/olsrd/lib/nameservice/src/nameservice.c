/*
 * Copyright (c) 2006, Jens Nachtigall <nachtigall@web.de>
 * Copyright (c) 2005, Bruno Randolf <bruno.randolf@4g-systems.biz>
 * Copyright (c) 2004, Andreas Tønnesen(andreto-at-olsr.org)
 * Copyright (c) 2007, Sven-Ola <sven-ola@gmx.de>
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


/*
 * Dynamic linked library for UniK OLSRd
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <regex.h>

#include "olsr.h"
#include "ipcalc.h"
#include "net_olsr.h"
#include "routing_table.h"
#include "mantissa.h"
#include "scheduler.h"
#include "parser.h"
#include "duplicate_set.h"
#include "tc_set.h"
#include "hna_set.h"
#include "mid_set.h"
#include "link_set.h"

#include "plugin_util.h"
#include "nameservice.h"
#include "mapwrite.h"
#include "olsrd_copy.h"
#include "compat.h"


/* config parameters */
static char my_hosts_file[MAX_FILE + 1];
static char my_add_hosts[MAX_FILE + 1];
static char my_suffix[MAX_SUFFIX];
static int my_interval = EMISSION_INTERVAL;
static double my_timeout = NAME_VALID_TIME;
static char my_resolv_file[MAX_FILE +1];
static char my_services_file[MAX_FILE + 1];
static char latlon_in_file[MAX_FILE + 1];
static char my_latlon_file[MAX_FILE + 1];
float my_lat = 0.0, my_lon = 0.0;

/* the databases (using hashing)
 * for hostnames, service_lines and dns-servers
 *
 * my own hostnames, service_lines and dns-servers
 * are store in a linked list (without hashing)
 * */
static struct db_entry* list[HASHSIZE];
struct name_entry *my_names = NULL;
static olsr_bool name_table_changed = OLSR_TRUE;

static struct db_entry* service_list[HASHSIZE];
static struct name_entry *my_services = NULL;
static olsr_bool service_table_changed = OLSR_TRUE;

static struct db_entry* forwarder_list[HASHSIZE];
static struct name_entry *my_forwarders = NULL;
static olsr_bool forwarder_table_changed = OLSR_TRUE;

struct db_entry* latlon_list[HASHSIZE];
static olsr_bool latlon_table_changed = OLSR_TRUE;

/* regular expression to be matched by valid hostnames, compiled in name_init() */
static regex_t regex_t_name;
static regmatch_t regmatch_t_name;

/* regular expression to be matched by valid service_lines, compiled in name_init() */
static regex_t regex_t_service;
static int pmatch_service = 10;
static regmatch_t regmatch_t_service[10];

/**
 * do initialization
 */
void
name_constructor(void) 
{
	int i;
	
#ifdef WIN32
	int len;

	GetWindowsDirectory(my_hosts_file, MAX_FILE - 12);
	GetWindowsDirectory(my_services_file, MAX_FILE - 12);
	GetWindowsDirectory(my_resolv_file, MAX_FILE - 12);

	len = strlen(my_hosts_file);
	if (my_hosts_file[len - 1] != '\\')
		my_hosts_file[len++] = '\\';
	strcpy(my_hosts_file + len, "hosts_olsr");
	
	len = strlen(my_services_file);
	if (my_services_file[len - 1] != '\\')
		my_services_file[len++] = '\\';
	strcpy(my_services_file + len, "services_olsr");

	len = strlen(my_resolv_file);
	if (my_resolv_file[len - 1] != '\\')
		my_resolv_file[len++] = '\\';
	strcpy(my_resolv_file + len, "resolvconf_olsr");
#else
	strcpy(my_hosts_file, "/var/run/hosts_olsr");
	strcpy(my_services_file, "/var/run/services_olsr");
	strcpy(my_resolv_file, "/var/run/resolvconf_olsr");
#endif

	my_suffix[0] = '\0';
	my_add_hosts[0] = '\0';
	my_latlon_file[0] = '\0';
	latlon_in_file[0] = '\0';
	
	/* init lists */
	for(i = 0; i < HASHSIZE; i++) {
		list[i] = NULL;
		forwarder_list[i] = NULL;
		service_list[i] = NULL;
		latlon_list[i] = NULL;
	}
	

}


static int set_nameservice_server(const char *value, void *data, set_plugin_parameter_addon addon)
{
	union olsr_ip_addr ip;
	struct name_entry **v = data;
	if (0 == strlen(value))
	{
		*v = add_name_to_list(*v, "", addon.ui, NULL);
		OLSR_PRINTF(1, "%s got %s (main address)\n", "Got", value);
		return 0;
	}
	else if (0 < inet_pton(olsr_cnf->ip_version, value, &ip))
	{
		*v = add_name_to_list(*v, "", addon.ui, &ip);
		OLSR_PRINTF(1, "%s got %s\n", "Got", value);
		return 0;
	}
	else
	{
		OLSR_PRINTF(0, "Illegal IP address \"%s\"", value);
	}
	return 1;
}

static int set_nameservice_name(const char *value, void *data, set_plugin_parameter_addon addon)
{
	struct name_entry **v = data;
	if (0 < strlen(value))
	{
		*v = add_name_to_list(*v, value, addon.ui, NULL);
		OLSR_PRINTF(1, "%s got %s (main address)\n", "Got", value);
		return 0;
	}
	else
	{
		OLSR_PRINTF(0, "Illegal name \"%s\"", value);
	}
	return 1;
}

static int set_nameservice_host(const char *value, void *data, set_plugin_parameter_addon addon)
{
	union olsr_ip_addr ip;
	struct name_entry **v = data;
	if (0 < inet_pton(olsr_cnf->ip_version, addon.pc, &ip))
	{
		// the IP is validated later
		*v = add_name_to_list(*v, value, NAME_HOST, &ip);
		OLSR_PRINTF(1, "%s: %s got %s\n", "Got", addon.pc, value);
		return 0;
	}
	else
	{
		OLSR_PRINTF(0, "%s: Illegal IP address \"%s\"", addon.pc, value);
	}
	return 1;
}

static int set_nameservice_float(const char *value, void *data, set_plugin_parameter_addon addon __attribute__((unused)))
{
	const float thefloat = atof(value);
	if (data != NULL)
	{
		float *v = data;
		*v = thefloat;
		OLSR_PRINTF(1, "%s float %f\n", "Got", thefloat);
	}
	else
	{
		OLSR_PRINTF(0, "%s float %f\n", "Ignored", thefloat);
	}
	return 0;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
    { .name = "interval",      .set_plugin_parameter = &set_plugin_int,         .data = &my_interval },
    { .name = "timeout",       .set_plugin_parameter = &set_nameservice_float,  .data = &my_timeout },
    { .name = "hosts-file",    .set_plugin_parameter = &set_plugin_string,      .data = &my_hosts_file,    .addon = {sizeof(my_hosts_file)} },
    { .name = "resolv-file",   .set_plugin_parameter = &set_plugin_string,      .data = &my_resolv_file,   .addon = {sizeof(my_resolv_file)} },
    { .name = "suffix",        .set_plugin_parameter = &set_plugin_string,      .data = &my_suffix,        .addon = {sizeof(my_suffix)} },
    { .name = "add-hosts",     .set_plugin_parameter = &set_plugin_string,      .data = &my_add_hosts,     .addon = {sizeof(my_add_hosts)} },
    { .name = "services-file", .set_plugin_parameter = &set_plugin_string,      .data = &my_services_file, .addon = {sizeof(my_services_file)} },
    { .name = "lat",           .set_plugin_parameter = &set_nameservice_float,  .data = &my_lat },
    { .name = "lon",           .set_plugin_parameter = &set_nameservice_float,  .data = &my_lon },
    { .name = "latlon-file",   .set_plugin_parameter = &set_plugin_string,      .data = &my_latlon_file,   .addon = {sizeof(my_latlon_file)} },
    { .name = "latlon-infile", .set_plugin_parameter = &set_plugin_string,      .data = &latlon_in_file,   .addon = {sizeof(latlon_in_file)} },
    { .name = "dns-server",    .set_plugin_parameter = &set_nameservice_server, .data = &my_forwarders,    .addon = {NAME_FORWARDER} },
    { .name = "name",          .set_plugin_parameter = &set_nameservice_name,   .data = &my_names,         .addon = {NAME_HOST} },
    { .name = "service",       .set_plugin_parameter = &set_nameservice_name,   .data = &my_services,      .addon = {NAME_SERVICE} },
    { .name = "",              .set_plugin_parameter = &set_nameservice_host,   .data = &my_names },
};

void olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
	*params = plugin_parameters;
	*size = sizeof(plugin_parameters)/sizeof(*plugin_parameters);
}


/**
 * queue the name/forwarder/service given in value
 * to the front of my_list
 */
struct name_entry* 
add_name_to_list(struct name_entry *my_list, const char *value, int type, const union olsr_ip_addr *ip) 
{
	struct name_entry *tmp = olsr_malloc(sizeof(struct name_entry), "new name_entry add_name_to_list");
	tmp->name = strndup( value, MAX_NAME );
	tmp->len = strlen( tmp->name );
	tmp->type = type;
	// all IPs with value 0 will be set to main_addr later
	if (ip==NULL) 
		memset(&tmp->ip, 0, sizeof(tmp->ip));
	else
		tmp->ip = *ip;
	tmp->next = my_list;
	return tmp;
}


/**
 * last initialization
 *
 * we have to do this here because some things like main_addr 
 * or the dns suffix (for validation) are not known before
 *
 * this is beause of the order in which the plugin is initialized 
 * by the plugin loader:
 *   - first the parameters are sent
 *   - then register_olsr_data() from olsrd_plugin.c is called
 *     which sets up main_addr and some other variables
 *   - register_olsr_data() then then finally calls this function
 */
int
name_init(void)
{
	struct name_entry *name;
	union olsr_ip_addr ipz;
	int ret;

	//regex string for validating the hostnames
	const char *regex_name = "^[[:alnum:]_.-]+$";
	//regex string for the service line
	char *regex_service = olsr_malloc(256*sizeof(char) + strlen(my_suffix), "new *char from name_init for regex_service");
	memset(&ipz, 0, sizeof(ipz));

	//compile the regex from the string
	if ((ret = regcomp(&regex_t_name, regex_name , REG_EXTENDED)) != 0)
	{
		/* #2: call regerror() if regcomp failed 
		 * commented out, because only for debuggin needed
		 *
		int errmsgsz = regerror(ret, &regex_t_name, NULL, 0);
		char *errmsg = malloc(errmsgsz);
		regerror(ret, &regex_t_name, errmsg, errmsgsz);
		fprintf(stderr, "regcomp: %s", errmsg);
		free(errmsg);
		regfree(&regex_t_name);
		* */
		OLSR_PRINTF(0, "compilation of regex \"%s\" for hostname failed", regex_name);
	}

	// a service line is something like prot://hostname.suffix:port|tcp|my little description about this service
	//                  for example     http://router.olsr:80|tcp|my homepage
	//                     prot     ://  (hostname.suffix     OR         ip)
	//regex_service = "^[[:alnum:]]+://(([[:alnum:]_.-]+.olsr)|([[:digit:]]{1,3}\\.[[:digit:]]{1,3}\\.[[:digit:]]{1,3}\\.[[:digit:]]{1,3}))
	//                 :    port              /path      |(tcp OR udp) |short description
	//                 :[[:digit:]]+[[:alnum:]/?._=#-]*\\|(tcp|udp)\\|[^|[:cntrl:]]+$";
    strcpy(regex_service, "^[[:alnum:]]+://(([[:alnum:]_.-]+");
    strcat(regex_service, my_suffix);
	strcat(regex_service, ")|([[:digit:]]{1,3}\\.[[:digit:]]{1,3}\\.[[:digit:]]{1,3}\\.[[:digit:]]{1,3})):[[:digit:]]+[[:alnum:]/?._=#-]*\\|(tcp|udp)\\|[^|[:cntrl:]]+$");

	/* #1: call regcomp() to compile the regex */
	if ((ret = regcomp(&regex_t_service, regex_service , REG_EXTENDED )) != 0)
	{
		/* #2: call regerror() if regcomp failed 
		 * commented out, because only for debuggin needed
		 *
		int errmsgsz = regerror(ret, &regex_t_service, NULL, 0);
		char *errmsg = malloc(errmsgsz);
		regerror(ret, &regex_t_service, errmsg, errmsgsz);
		fprintf(stderr, "regcomp: %s", errmsg);
		free(errmsg);
		regfree(&regex_t_service);
		* */
		OLSR_PRINTF(0, "compilation of regex \"%s\" for hostname failed", regex_name);
	}
	free(regex_service);
	regex_service = NULL;

	//fill in main addr for all entries with ip==0
	//this does not matter for service, because the ip does not matter
	//for service

	for (name = my_names; name != NULL; name = name->next) {
		if (ipequal(&name->ip, &ipz)) {
			OLSR_PRINTF(2, "NAME PLUGIN: insert main addr for name %s \n", name->name);
			name->ip = olsr_cnf->main_addr;
		}
	}
	for (name = my_forwarders; name != NULL; name = name->next) {
		if (name->ip.v4.s_addr == 0) {
			OLSR_PRINTF(2, "NAME PLUGIN: insert main addr for name %s \n", name->name);
			name->ip = olsr_cnf->main_addr;
		}
	}

	//check if entries I want to announce myself are valid and allowed
	my_names = remove_nonvalid_names_from_list(my_names, NAME_HOST);
	my_forwarders = remove_nonvalid_names_from_list(my_forwarders, NAME_FORWARDER);
	my_services = remove_nonvalid_names_from_list(my_services, NAME_SERVICE);


	/* register functions with olsrd */
	olsr_parser_add_function(&olsr_parser, PARSER_TYPE, 1);
	olsr_register_timeout_function(&olsr_timeout, OLSR_TRUE);
	olsr_register_scheduler_event(&olsr_event, NULL, my_interval, 0, NULL);
	mapwrite_init(my_latlon_file);

	return 1;
}


struct name_entry*
remove_nonvalid_names_from_list(struct name_entry *my_list, int type) 
{
	struct name_entry *next = my_list;
	olsr_bool valid = OLSR_FALSE;
	if (my_list == NULL) {
		return NULL;
	} 

	switch (type) {
		case NAME_HOST:
			valid = is_name_wellformed(my_list->name) && allowed_ip(&my_list->ip); 
			break;
		case NAME_FORWARDER:
			valid = allowed_ip(&my_list->ip);
			break;
		case NAME_SERVICE:
			valid = allowed_service(my_list->name);
			break;
		case NAME_LATLON:
			valid = is_latlon_wellformed(my_list->name);
			break;
	}

	if ( !valid  ) {
#ifndef NODEBUG
		struct ipaddr_str strbuf;
#endif
		OLSR_PRINTF(1, "NAME PLUGIN: invalid or malformed parameter %s (%s), fix your config!\n", my_list->name, olsr_ip_to_string(&strbuf, &my_list->ip));
		next = my_list->next;
		free(my_list->name);
		my_list->name = NULL;
		free(my_list);
		my_list = NULL;
		return remove_nonvalid_names_from_list(next, type);
	} else {
#ifndef NODEBUG
		struct ipaddr_str strbuf;
#endif
		OLSR_PRINTF(2, "NAME PLUGIN: validate parameter %s (%s) -> OK\n", my_list->name, olsr_ip_to_string(&strbuf, &my_list->ip));
		my_list->next = remove_nonvalid_names_from_list(my_list->next, type);
		return my_list;
	}
}



/**
 * called at unload: free everything
 *
 * XXX: should I delete the hosts/services/resolv.conf files on exit?
 */
void
name_destructor(void)
{
	OLSR_PRINTF(2, "NAME PLUGIN: exit. cleaning up...\n");
	
	free_name_entry_list(&my_names);
	free_name_entry_list(&my_services);
	free_name_entry_list(&my_forwarders);

	free_all_list_entries(list);
	free_all_list_entries(service_list);
	free_all_list_entries(forwarder_list);
	free_all_list_entries(latlon_list);

	regfree(&regex_t_name);
	regfree(&regex_t_service);
	mapwrite_exit();
}

/* free all list entries */
void 
free_all_list_entries(struct db_entry **this_db_list) 
{
	int i;
	
	for(i = 0; i < HASHSIZE; i++)
	{
		struct db_entry **tmp = &this_db_list[i];
		while(*tmp != NULL)
		{
			struct db_entry *to_delete = *tmp;
			*tmp = (*tmp)->next;
			free_name_entry_list(&to_delete->names);
			free(to_delete);
			to_delete = NULL;
		}
	}
}


/**
 * A timeout function called every time
 *
 * XXX:the scheduler is polled (by default 10 times a sec,
 * which is far too often):
 *
 * time out old list entries
 * and write changes to file
 */

static int timeout_roundrobin = 0;

void
olsr_timeout(void)
{
	switch(timeout_roundrobin++)
	{
		case 0:
			timeout_old_names(list, &name_table_changed);
			timeout_old_names(forwarder_list, &forwarder_table_changed);
			timeout_old_names(service_list, &service_table_changed);
			timeout_old_names(latlon_list, &latlon_table_changed);
			break;
		case 1:
			write_resolv_file(); // if forwarder_table_changed
			break;
		case 2:
			write_hosts_file(); // if name_table_changed
			break;
		case 3:
			write_services_file(); // if service_table_changed
			break;
#ifdef WIN32
		case 4:
			write_latlon_file(); // latlon_table_changed
			break;
#endif
		default:
			timeout_roundrobin = 0;
	} // switch
}

void
timeout_old_names(struct db_entry **this_list, olsr_bool *this_table_changed)
{
	struct db_entry **tmp;
	struct db_entry *to_delete;
	int index;

	for(index=0;index<HASHSIZE;index++)
	{
		for (tmp = &this_list[index]; *tmp != NULL; )
		{
			/* check if the entry for this ip is timed out */
			if (olsr_timed_out(&(*tmp)->timer))
			{
#ifndef NODEBUG
				struct ipaddr_str strbuf;
#endif
				to_delete = *tmp;
				/* update the pointer in the linked list */
				*tmp = (*tmp)->next;
				
				OLSR_PRINTF(2, "NAME PLUGIN: %s timed out... deleting\n", 
					olsr_ip_to_string(&strbuf, &to_delete->originator));
	
				/* Delete */
				free_name_entry_list(&to_delete->names);
				free(to_delete);
				*this_table_changed = OLSR_TRUE;
			} else {
				tmp = &(*tmp)->next;
			}
		}
	}
}


/**
 * Scheduled event: generate and send NAME packet
 */
void
olsr_event(void *foo __attribute__((unused)))
{
	/* send buffer: huge */
	char buffer[10240];
	union olsr_message *message = (union olsr_message *)buffer;
	struct interface *ifn;
	int namesize;

	/* fill message */
	if(olsr_cnf->ip_version == AF_INET)
	{
		/* IPv4 */
		message->v4.olsr_msgtype = MESSAGE_TYPE;
		message->v4.olsr_vtime = double_to_me(my_timeout);
		memcpy(&message->v4.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
		message->v4.ttl = MAX_TTL;
		message->v4.hopcnt = 0;
		message->v4.seqno = htons(get_msg_seqno());

		namesize = encap_namemsg((struct namemsg*)&message->v4.message);
		namesize = namesize + sizeof(struct olsrmsg);

		message->v4.olsr_msgsize = htons(namesize);
	}
	else
	{
		/* IPv6 */
		message->v6.olsr_msgtype = MESSAGE_TYPE;
		message->v6.olsr_vtime = double_to_me(my_timeout);
		memcpy(&message->v6.originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
		message->v6.ttl = MAX_TTL;
		message->v6.hopcnt = 0;
		message->v6.seqno = htons(get_msg_seqno());

		namesize = encap_namemsg((struct namemsg*)&message->v6.message);
		namesize = namesize + sizeof(struct olsrmsg6);
		
		message->v6.olsr_msgsize = htons(namesize);
	}

	/* looping trough interfaces */
	for (ifn = ifnet; ifn ; ifn = ifn->int_next) 
	{
		OLSR_PRINTF(3, "NAME PLUGIN: Generating packet - [%s]\n", ifn->int_name);

		if(net_outbuffer_push(ifn, message, namesize) != namesize ) {
			/* send data and try again */
			net_output(ifn);
			if(net_outbuffer_push(ifn, message, namesize) != namesize ) {
				OLSR_PRINTF(1, "NAME PLUGIN: could not send on interface: %s\n", ifn->int_name);
			}
		}
	}
}


/**
 * Parse name olsr message of NAME type
 */
void
olsr_parser(union olsr_message *m, struct interface *in_if, union olsr_ip_addr *ipaddr)
{
	struct namemsg *namemessage;
	union olsr_ip_addr originator;
	double vtime;
	int size;
	olsr_u16_t seqno;

	/* Fetch the originator of the messsage */
	if(olsr_cnf->ip_version == AF_INET) {
		memcpy(&originator, &m->v4.originator, olsr_cnf->ipsize);
		seqno = ntohs(m->v4.seqno);
	} else {
		memcpy(&originator, &m->v6.originator, olsr_cnf->ipsize);
		seqno = ntohs(m->v6.seqno);
	}
		
	/* Fetch the message based on IP version */
	if(olsr_cnf->ip_version == AF_INET) {
		vtime = me_to_double(m->v4.olsr_vtime);
		size = ntohs(m->v4.olsr_msgsize);
		namemessage = (struct namemsg*)&m->v4.message;
	}
	else {
		vtime = me_to_double(m->v6.olsr_vtime);
		size = ntohs(m->v6.olsr_msgsize);
		namemessage = (struct namemsg*)&m->v6.message;
	}

	/* Check if message originated from this node. 
	If so - back off */
	if(ipequal(&originator, &olsr_cnf->main_addr))
		return;

	/* Check that the neighbor this message was received from is symmetric. 
	If not - back off*/
	if(check_neighbor_link(ipaddr) != SYM_LINK) {
#ifndef NODEBUG
		struct ipaddr_str strbuf;
#endif
		OLSR_PRINTF(3, "NAME PLUGIN: Received msg from NON SYM neighbor %s\n", olsr_ip_to_string(&strbuf, ipaddr));
		return;
	}

	/* Check if this message has been processed before
	* Remeber that this also registeres the message as
	* processed if nessecary
	*/
	if(olsr_check_dup_table_proc(&originator, seqno)) {
		/* If not so - process */
		update_name_entry(&originator, namemessage, size, vtime);
	}

	/* Forward the message if nessecary
	* default_fwd does all the work for us! */
	olsr_forward_message(m, &originator, seqno, in_if, ipaddr);
}

/**
 * Encapsulate a name message into a packet. 
 *
 * It assumed that there is enough space in the buffer to do this!
 *
 * Returns: the length of the message that was appended
 */
int
encap_namemsg(struct namemsg* msg)
{
	struct name_entry *my_name;
	struct name_entry *my_service;

	// add the hostname, service and forwarder entries after the namemsg header
	char* pos = (char*)msg + sizeof(struct namemsg);
	short i=0;

	// names
	for (my_name = my_names; my_name!=NULL; my_name = my_name->next)
	{
		pos = create_packet( (struct name*) pos, my_name);
		i++;
	}
	// forwarders
	for (my_name = my_forwarders; my_name!=NULL; my_name = my_name->next)
	{
		pos = create_packet( (struct name*) pos, my_name);
		i++;
	}
	// services
	for (my_service = my_services; my_service!=NULL; my_service = my_service->next)
	{
		pos = create_packet( (struct name*) pos, my_service);
		i++;
	}
	// latlon
	if ('\0' != latlon_in_file[0])
	{
		FILE* in = fopen( latlon_in_file, "r" );
		if (in != NULL) {
			fscanf(in, "%f,%f", &my_lat, &my_lon);
			fclose(in);
		}
		else
		{
			OLSR_PRINTF(0, "NAME PLUGIN: cant read latlon in file %s\n", latlon_in_file);
		}
	}
	if (0.0 != my_lat && 0.0 != my_lon)
	{
		char s[64];
		struct name_entry e;
		memset(&e, 0, sizeof(e));
		sprintf(s, "%f,%f,%d", my_lat, my_lon, get_isdefhna_latlon());
		e.len = strlen(s);
		e.type = NAME_LATLON;
		e.name = s;
		lookup_defhna_latlon(&e.ip);
		pos = create_packet( (struct name*) pos, &e);
		i++;
	}

	// write the namemsg header with the number of announced entries and the protocol version
	msg->nr_names = htons(i);
	msg->version = htons(NAME_PROTOCOL_VERSION);

	return pos - (char*)msg; //length
}


/**
 * convert each of my to be announced name_entries into network
 * compatible format
 *
 * return the length of the name packet
 */
char*  
create_packet(struct name* to, struct name_entry *from)
{
	char *pos = (char*) to;
	int k;
#ifndef NODEBUG
	struct ipaddr_str strbuf;
#endif
	OLSR_PRINTF(3, "NAME PLUGIN: Announcing name %s (%s) %d\n", 
		from->name, olsr_ip_to_string(&strbuf, &from->ip), from->len);
	to->type = htons(from->type);
	to->len = htons(from->len);
	to->ip = from->ip;
	pos += sizeof(struct name);
	strncpy(pos, from->name, from->len);
	pos += from->len;
	for (k = from->len; (k & 3) != 0; k++)
		*pos++ = '\0';
	return pos;
}

/**
 * decapsulate a received name, service or forwarder and update the corresponding hash table if necessary
 */
void
decap_namemsg(struct name *from_packet, struct name_entry **to, olsr_bool *this_table_changed )
{
#ifndef NODEBUG
	struct ipaddr_str strbuf;
#endif
	struct name_entry *tmp;
	struct name_entry *already_saved_name_entries;
	char *name = (char*)from_packet + sizeof(struct name);
	int type_of_from_packet = ntohs(from_packet->type);
	unsigned int len_of_name = ntohs(from_packet->len);
	OLSR_PRINTF(4, "NAME PLUGIN: decap type=%d, len=%d, name=%s\n",
		type_of_from_packet, len_of_name, name);

	//XXX: should I check the from_packet->ip here? If so, why not also check the ip from HOST and SERVICE?
	if( (type_of_from_packet==NAME_HOST && !is_name_wellformed(name)) ||
		(type_of_from_packet==NAME_SERVICE && !is_service_wellformed(name)) ||
		(type_of_from_packet==NAME_LATLON && !is_latlon_wellformed(name)))
	{
		OLSR_PRINTF(4, "NAME PLUGIN: invalid name [%s] received, skipping.\n", name );
		return;
	}

	//ignore all packets with a too long name
	//or a spoofed len of its included name string
	if (len_of_name > MAX_NAME || strlen(name) != len_of_name || NULL != strchr(name, '\\') || NULL != strchr(name, '\'')) {
		OLSR_PRINTF(4, "NAME PLUGIN: from_packet->len %d > MAX_NAME %d or from_packet->len %d !0 strlen(name [%s] in packet)\n",
			len_of_name, MAX_NAME, len_of_name, name );
		return;
	}

	// don't insert the received entry again, if it has already been inserted in the hash table. 
	// Instead only the validity time is set in insert_new_name_in_list function, which calls this one
	for (already_saved_name_entries = (*to); already_saved_name_entries != NULL ; already_saved_name_entries = already_saved_name_entries->next)
	{
		if ( (type_of_from_packet==NAME_HOST || type_of_from_packet==NAME_SERVICE) && strncmp(already_saved_name_entries->name, name, len_of_name) == 0 ) {
			OLSR_PRINTF(4, "NAME PLUGIN: received name or service entry %s (%s) already in hash table\n",
				name, olsr_ip_to_string(&strbuf, &already_saved_name_entries->ip));
			return;
		} else if (type_of_from_packet==NAME_FORWARDER && ipequal(&already_saved_name_entries->ip, &from_packet->ip) ) {
			OLSR_PRINTF(4, "NAME PLUGIN: received forwarder entry %s (%s) already in hash table\n",
				name, olsr_ip_to_string(&strbuf, &already_saved_name_entries->ip));
			return;
		} else if (type_of_from_packet==NAME_LATLON ) {
			if (0 != strncmp(already_saved_name_entries->name, name, len_of_name))
			{
				OLSR_PRINTF(4, "NAME PLUGIN: updating name %s -> %s (%s)\n",
					already_saved_name_entries->name, name,
					olsr_ip_to_string(&strbuf, &already_saved_name_entries->ip));
				free(already_saved_name_entries->name);
				already_saved_name_entries->name = olsr_malloc(len_of_name + 1, "upd name_entry name");
				strncpy(already_saved_name_entries->name, name, len_of_name);
				*this_table_changed = OLSR_TRUE;
			}
			if (!ipequal(&already_saved_name_entries->ip, &from_packet->ip))
			{
#ifndef NODEBUG
				struct ipaddr_str strbuf2, strbuf3;
#endif
				OLSR_PRINTF(4, "NAME PLUGIN: updating ip %s -> %s (%s)\n",
					olsr_ip_to_string(&strbuf, &already_saved_name_entries->ip),
					olsr_ip_to_string(&strbuf2, &from_packet->ip),
					olsr_ip_to_string(&strbuf3, &already_saved_name_entries->ip));
				already_saved_name_entries->ip = from_packet->ip;
				*this_table_changed = OLSR_TRUE;
			}
			if (!*this_table_changed)
			{
				OLSR_PRINTF(4, "NAME PLUGIN: received latlon entry %s (%s) already in hash table\n",
					name, olsr_ip_to_string(&strbuf, &already_saved_name_entries->ip));
			}
			return;
		}
	}

	//if not yet known entry 
	tmp = olsr_malloc(sizeof(struct name_entry), "new name_entry");		
	tmp->type = ntohs(from_packet->type);
	tmp->len = len_of_name > MAX_NAME ? MAX_NAME : ntohs(from_packet->len);
	tmp->name = olsr_malloc(tmp->len+1, "new name_entry name");
	tmp->ip = from_packet->ip;
	strncpy(tmp->name, name, tmp->len);
	tmp->name[tmp->len] = '\0';

	OLSR_PRINTF(3, "\nNAME PLUGIN: create new name/service/forwarder entry %s (%s) [len=%d] [type=%d] in linked list\n", 
		tmp->name, olsr_ip_to_string(&strbuf, &tmp->ip), tmp->len, tmp->type);

	*this_table_changed = OLSR_TRUE;

	// queue to front
	tmp->next = *to;
	*to = tmp;
}


/**
 * unpack the received message and delegate to the decapsulation function for each
 * name/service/forwarder entry in the message
 */
void
update_name_entry(union olsr_ip_addr *originator, struct namemsg *msg, int msg_size, double vtime)
{
#ifndef NODEBUG
	struct ipaddr_str strbuf;
#endif
	char *pos, *end_pos;
	struct name *from_packet; 
	int i;

	OLSR_PRINTF(3, "NAME PLUGIN: Received Message from %s\n", 
				olsr_ip_to_string(&strbuf, originator));
	
	if (ntohs(msg->version) != NAME_PROTOCOL_VERSION) {
		OLSR_PRINTF(3, "NAME PLUGIN: ignoring wrong version %d\n", msg->version);
		return;
	}
	
	/* now add the names from the message */
	pos = (char*)msg + sizeof(struct namemsg);
	end_pos = pos + msg_size - sizeof(struct name*); // at least one struct name has to be left	

	for (i=ntohs(msg->nr_names); i > 0 && pos<end_pos; i--) 
	{
		from_packet = (struct name*)pos;
		
		switch (ntohs(from_packet->type)) {
			case NAME_HOST: 
				insert_new_name_in_list(originator, list, from_packet, &name_table_changed, vtime); 
				break;
			case NAME_FORWARDER:
				insert_new_name_in_list(originator, forwarder_list, from_packet, &forwarder_table_changed, vtime); 
				break;
			case NAME_SERVICE:
				insert_new_name_in_list(originator, service_list, from_packet, &service_table_changed, vtime); 
				break;
			case NAME_LATLON:
				insert_new_name_in_list(originator, latlon_list, from_packet, &latlon_table_changed, vtime);
				break;
			default:
				OLSR_PRINTF(3, "NAME PLUGIN: Received Message of unknown type [%d] from (%s)\n", from_packet->type, olsr_ip_to_string(&strbuf, originator));
				break;
		}

		pos += sizeof(struct name);
		pos += 1 + (( ntohs(from_packet->len) - 1) | 3);
	}
	if (i!=0)
		OLSR_PRINTF(4, "NAME PLUGIN: Lost %d entries in received packet due to length inconsistency (%s)\n", i, olsr_ip_to_string(&strbuf, originator));
}


/**
 * insert all the new names,services and forwarders from a received packet into the
 * corresponding entry for this ip in the corresponding hash table
 */
void
insert_new_name_in_list(union olsr_ip_addr *originator, struct db_entry **this_list, struct name *from_packet, olsr_bool *this_table_changed, double vtime)
{
	int hash;
	struct db_entry *entry;

	olsr_bool entry_found = OLSR_FALSE;

	hash = olsr_hashing(originator);

	/* find the entry for originator, if there is already one */
	for (entry = this_list[hash]; entry != NULL; entry = entry->next)
	{
		if (ipequal(originator, &entry->originator)) {
#ifndef NODEBUG
			struct ipaddr_str strbuf;
#endif
			// found
			OLSR_PRINTF(4, "NAME PLUGIN: found entry for (%s) in its hash table\n", olsr_ip_to_string(&strbuf, originator));

			//delegate to function for parsing the packet and linking it to entry->names
			decap_namemsg(from_packet, &entry->names, this_table_changed);

			olsr_get_timestamp(vtime * 1000, &entry->timer);

			entry_found = OLSR_TRUE;
		}
	}
	if (! entry_found)
	{
#ifndef NODEBUG
		struct ipaddr_str strbuf;
#endif
		OLSR_PRINTF(3, "NAME PLUGIN: create new db entry for ip (%s) in hash table\n", olsr_ip_to_string(&strbuf, originator));

		/* insert a new entry */
		entry = olsr_malloc(sizeof(struct db_entry), "new db_entry");

		entry->originator = *originator;
		olsr_get_timestamp(vtime * 1000, &entry->timer);
		entry->names = NULL;

		// queue to front
		entry->next = this_list[hash];
		this_list[hash] = entry;

		//delegate to function for parsing the packet and linking it to entry->names
		decap_namemsg(from_packet, &entry->names, this_table_changed);
	}
}

/**
 * write names to a file in /etc/hosts compatible format
 */
void
write_hosts_file(void)
{
	int hash;
	struct name_entry *name;
	struct db_entry *entry;
	FILE* hosts;
	FILE* add_hosts;
	int c=0;
	time_t currtime;

#ifdef MID_ENTRIES
	struct mid_address *alias;
#endif

	if (!name_table_changed)
		return;

	OLSR_PRINTF(2, "NAME PLUGIN: writing hosts file\n");

	hosts = fopen( my_hosts_file, "w" );
	if (hosts == NULL) {
		OLSR_PRINTF(2, "NAME PLUGIN: cant write hosts file\n");
		return;
	}
	
	fprintf(hosts, "### this /etc/hosts file is overwritten regularly by olsrd\n");
	fprintf(hosts, "### do not edit\n\n");

	fprintf(hosts, "127.0.0.1\tlocalhost\n");
	fprintf(hosts, "::1\t\tlocalhost\n\n");
	
	// copy content from additional hosts filename
	if (my_add_hosts[0] != '\0') {
		add_hosts = fopen( my_add_hosts, "r" );
		if (add_hosts == NULL) {
			OLSR_PRINTF(2, "NAME PLUGIN: cant open additional hosts file\n");
		}
		else {
			fprintf(hosts, "### contents from '%s' ###\n\n", my_add_hosts);
			while ((c = getc(add_hosts)) != EOF)
				putc(c, hosts);
		}
		fclose(add_hosts);		
		fprintf(hosts, "\n### olsr names ###\n\n");
	}
	
	// write own names
	for (name = my_names; name != NULL; name = name->next) {
		struct ipaddr_str strbuf;
		fprintf(hosts, "%s\t%s%s\t# myself\n", olsr_ip_to_string(&strbuf, &name->ip), name->name, my_suffix );
	}
	
	// write received names
	for (hash = 0; hash < HASHSIZE; hash++) 
	{
		for(entry = list[hash]; entry != NULL; entry = entry->next)
		{
			for (name = entry->names; name != NULL; name = name->next) 
			{
				struct ipaddr_str strbuf;

				OLSR_PRINTF(
					6, "%s\t%s%s\t#%s\n",
						olsr_ip_to_string( &strbuf, &name->ip ), name->name, my_suffix,
						olsr_ip_to_string( &strbuf, &entry->originator )
				);

				fprintf(
					hosts, "%s\t%s%s\t# %s\n",
						olsr_ip_to_string( &strbuf, &name->ip ), name->name, my_suffix,
						olsr_ip_to_string( &strbuf, &entry->originator )
				);

#ifdef MID_ENTRIES
				// write mid entries
				if( ( alias = mid_lookup_aliases( &name->ip ) ) != NULL )
				{
					unsigned short mid_num = 1;
					char	   mid_prefix[MID_MAXLEN];

					while( alias != NULL )
					{
						struct ipaddr_str midbuf;

						// generate mid prefix
						sprintf( mid_prefix, MID_PREFIX, mid_num );

						OLSR_PRINTF(
							6, "%s\t%s%s%s\t# %s (mid #%i)\n",
								olsr_ip_to_string( &midbuf, &alias->alias ),
								mid_prefix, name->name, my_suffix,
								olsr_ip_to_string( &strbuf, &entry->originator ),
								mid_num
						);

						fprintf(
							hosts, "%s\t%s%s%s\t# %s (mid #%i)\n",
								olsr_ip_to_string( &midbuf, &alias->alias ),
								mid_prefix, name->name, my_suffix,
								olsr_ip_to_string( &strbuf, &entry->originator ),
								mid_num
						);

						alias = alias->next_alias;
						mid_num++;
					}
				}
#endif
			}
		}
	}

	if (time(&currtime)) {
		fprintf(hosts, "\n### written by olsrd at %s", ctime(&currtime));
	}
	  
	fclose(hosts);
	name_table_changed = OLSR_FALSE;
}


/**
 * write services to a file in the format:
 * service  #originator ip
 *
 * since service has a special format
 * each line will look similar to e.g.
 * http://me.olsr:80|tcp|my little homepage
 */
void
write_services_file(void)
{
	int hash;
	struct name_entry *name;
	struct db_entry *entry;
	FILE* services_file;
	time_t currtime;


	if (!service_table_changed)
		return;

	OLSR_PRINTF(2, "NAME PLUGIN: writing services file\n");

	services_file = fopen( my_services_file, "w" );
	if (services_file == NULL) {
		OLSR_PRINTF(2, "NAME PLUGIN: cant write services_file file\n");
		return;
	}
	
	fprintf(services_file, "### this file is overwritten regularly by olsrd\n");
	fprintf(services_file, "### do not edit\n\n");

	
	// write own services
	for (name = my_services; name != NULL; name = name->next) {
		fprintf(services_file, "%s\t# my own service\n", name->name);
	}
	
	// write received services
	for (hash = 0; hash < HASHSIZE; hash++) 
	{
		for(entry = service_list[hash]; entry != NULL; entry = entry->next)
		{
			for (name = entry->names; name != NULL; name = name->next) 
			{
				struct ipaddr_str strbuf;
				OLSR_PRINTF(6, "%s\t",  name->name);
				OLSR_PRINTF(6, "\t#%s\n", olsr_ip_to_string(&strbuf, &entry->originator));

				fprintf(services_file, "%s\t", name->name );
				fprintf(services_file, "\t#%s\n", olsr_ip_to_string(&strbuf, &entry->originator));
			}
		}
	}

	if (time(&currtime)) {
		fprintf(services_file, "\n### written by olsrd at %s", ctime(&currtime));
	}
	  
	fclose(services_file);
	service_table_changed = OLSR_FALSE;
}

/**
 * Sort the nameserver pointer array.
 *
 * fresh entries are at the beginning of the array and
 * the best entry is at the end of the array.
 */
static void
select_best_nameserver(struct rt_entry **rt)
{
	int nameserver_idx;
	struct rt_entry *rt1, *rt2;

	for (nameserver_idx = 0;
		 nameserver_idx < NAMESERVER_COUNT;
		 nameserver_idx++) {

		rt1 = rt[nameserver_idx];
		rt2 = rt[nameserver_idx+1];

		/*
		 * compare the next two pointers in the array.
		 * if the second pointer is NULL then percolate it up.
		 */
		if (!rt2 || olsr_cmp_rt(rt1, rt2)) {
#ifndef NODEBUG
			struct ipaddr_str strbuf;
#endif
			/*
			 * first is better, swap the pointers.
			 */
			OLSR_PRINTF(6, "NAME PLUGIN: nameserver %s, etx %.3f\n",
						olsr_ip_to_string(&strbuf, &rt1->rt_dst.prefix),
						rt1->rt_best->rtp_metric.etx);

			rt[nameserver_idx] = rt2;
			rt[nameserver_idx+1] = rt1;
		}
	}
}

/**
 * write the 3 best upstream DNS servers to resolv.conf file
 * best means the 3 with the best etx value in routing table
 */
void
write_resolv_file(void)
{
	int hash;
	struct name_entry *name;
	struct db_entry *entry;
	struct rt_entry *route;
	static struct rt_entry *nameserver_routes[NAMESERVER_COUNT+1];
	FILE* resolv;
	int i=0;
	time_t currtime;

	if (!forwarder_table_changed || my_forwarders != NULL || my_resolv_file[0] == '\0')
		return;

	/* clear the array of 3+1 nameserver routes */
	memset(nameserver_routes, 0, sizeof(nameserver_routes));

	for (hash = 0; hash < HASHSIZE; hash++) 
	{
		for(entry = forwarder_list[hash]; entry != NULL; entry = entry->next)
		{
			for (name = entry->names; name != NULL; name = name->next) 
			{
#ifndef NODEBUG
				struct ipaddr_str strbuf;
#endif
				route = olsr_lookup_routing_table(&name->ip);

				OLSR_PRINTF(6, "NAME PLUGIN: check route for nameserver %s %s",
							olsr_ip_to_string(&strbuf, &name->ip),
							route ? "suceeded" : "failed");

				if (route==NULL) // it's possible that route is not present yet
					continue;

				/* enqueue it on the head of list */
				*nameserver_routes = route;
				OLSR_PRINTF(6, "NAME PLUGIN: found nameserver %s, etx %.3f",
							olsr_ip_to_string(&strbuf, &name->ip),
							route->rt_best->rtp_metric.etx);

				/* find the closet one */
				select_best_nameserver(nameserver_routes);
			}
		}
	}

	/* if there is no best route we are done */
	if (nameserver_routes[NAMESERVER_COUNT]==NULL)
		return;
		 
	/* write to file */
	OLSR_PRINTF(2, "NAME PLUGIN: try to write to resolv file\n");
	resolv = fopen( my_resolv_file, "w" );
	if (resolv == NULL) {
		OLSR_PRINTF(2, "NAME PLUGIN: can't write resolv file\n");
		return;
	}
	fprintf(resolv, "### this file is overwritten regularly by olsrd\n");
	fprintf(resolv, "### do not edit\n\n");

	for (i = NAMESERVER_COUNT; i >= 0; i--) {
		struct ipaddr_str strbuf;

		route = nameserver_routes[i];

		OLSR_PRINTF(2, "NAME PLUGIN: nameserver_routes #%d %p\n", i, route);

		if (!route) {
			continue;
		}

		OLSR_PRINTF(2, "NAME PLUGIN: nameserver %s\n",
					olsr_ip_to_string(&strbuf, &route->rt_dst.prefix));
		fprintf(resolv, "nameserver %s\n",
				olsr_ip_to_string(&strbuf, &route->rt_dst.prefix));
	}
	if (time(&currtime)) {
		fprintf(resolv, "\n### written by olsrd at %s", ctime(&currtime));
	}
	fclose(resolv);
	forwarder_table_changed = OLSR_FALSE;
}


/**
 * completely free a list of name_entries
 */
void 
free_name_entry_list(struct name_entry **list) 
{
	struct name_entry **tmp = list;
	struct name_entry *to_delete;
	while (*tmp != NULL) {
		to_delete = *tmp;
		*tmp = (*tmp)->next;
		free( to_delete->name );
		to_delete->name = NULL;
		free( to_delete );
		to_delete = NULL;
	}
}


/**
 * we only allow names for IP addresses which we are
 * responsible for: 
 * so the IP must either be from one of the interfaces
 * or inside a HNA which we have configured 
 */
olsr_bool
allowed_ip(const union olsr_ip_addr *addr)
{
	struct ip_prefix_list *hna;
	struct interface *iface;
	union olsr_ip_addr tmp_ip, tmp_msk;
#ifndef NODEBUG
	struct ipaddr_str strbuf;
#endif
	
	OLSR_PRINTF(6, "checking %s\n", olsr_ip_to_string(&strbuf, addr));
	
	for(iface = ifnet; iface; iface = iface->int_next)
	{
		OLSR_PRINTF(6, "interface %s\n", olsr_ip_to_string(&strbuf, &iface->ip_addr));
		if (ipequal(&iface->ip_addr, addr)) {
			OLSR_PRINTF(6, "MATCHED\n");
			return OLSR_TRUE;
		}
	}
	
	if (olsr_cnf->ip_version == AF_INET) {
		for (hna = olsr_cnf->hna_entries; hna != NULL; hna = hna->next) {
			union olsr_ip_addr netmask;
			OLSR_PRINTF(6, "HNA %s/%d\n", 
						olsr_ip_to_string(&strbuf, &hna->net.prefix),
						hna->net.prefix_len);
			if (hna->net.prefix_len == 0) {
				continue;
			}
			olsr_prefix_to_netmask(&netmask, hna->net.prefix_len);
			if ((addr->v4.s_addr & netmask.v4.s_addr) == hna->net.prefix.v4.s_addr) {
				OLSR_PRINTF(6, "MATCHED\n");
				return OLSR_TRUE;
			}
		}
	} else {
		for (hna = olsr_cnf->hna_entries; hna != NULL; hna = hna->next)
		{
			unsigned int i;
			OLSR_PRINTF(6, "HNA %s/%d\n", 
				olsr_ip_to_string(&strbuf, &hna->net.prefix),
				hna->net.prefix_len);
			if ( hna->net.prefix_len == 0 )
				continue;
			olsr_prefix_to_netmask(&tmp_msk, hna->net.prefix_len);
			for (i = 0; i < sizeof(tmp_ip.v6.s6_addr); i++) {
				tmp_ip.v6.s6_addr[i] = addr->v6.s6_addr[i] & tmp_msk.v6.s6_addr[i];
			}
			if (ipequal(&tmp_ip, &hna->net.prefix)) {
				OLSR_PRINTF(6, "MATCHED\n");
				return OLSR_TRUE;
			}
		}
	}
	return OLSR_FALSE;
}

/** check if name has the right syntax, i.e. it must adhere to a special regex 
 * stored in regex_t_name
 * necessary to avaid names like "0.0.0.0 google.de\n etc"
 */
olsr_bool
is_name_wellformed(const char *name) {
	return regexec(&regex_t_name, name, 1, &regmatch_t_name, 0) == 0 ;
}


/**
 * check if the service is in the right syntax and also that the hostname 
 * or ip whithin the service is allowed
 */
olsr_bool
allowed_service(const char *service_line)
{
	/* the call of is_service_wellformed generates the submatches stored in regmatch_t_service
	 * these are then used by allowed_hostname_or_ip_in_service
	 * see regexec(3) for more infos */
	if (!is_service_wellformed(service_line)) {
		return OLSR_FALSE;
	} else if (!allowed_hostname_or_ip_in_service(service_line, &(regmatch_t_service[1]))) {
		return OLSR_FALSE;
	} 

	return OLSR_TRUE;
}

olsr_bool
allowed_hostname_or_ip_in_service(const char *service_line, const regmatch_t *hostname_or_ip_match) 
{
	char *hostname_or_ip;
	union olsr_ip_addr olsr_ip;
	struct name_entry *name;
	if (hostname_or_ip_match->rm_so < 0 || hostname_or_ip_match->rm_eo < 0) {
		return OLSR_FALSE;
	} 

	hostname_or_ip = strndup(&service_line[hostname_or_ip_match->rm_so], hostname_or_ip_match->rm_eo - hostname_or_ip_match->rm_so);
	//hostname is one of the names, that I announce (i.e. one that i am allowed to announce)
	for (name = my_names; name != NULL; name = name->next) {
		if (strncmp(name->name, hostname_or_ip, name->len - strlen(my_suffix)) == 0 ) {
			OLSR_PRINTF(4, "NAME PLUGIN: hostname %s in service %s is OK\n", hostname_or_ip, service_line);
			free(hostname_or_ip);
			hostname_or_ip = NULL;
			return OLSR_TRUE;
		}
	}

	//ip in service-line is allowed 
	if (inet_pton(olsr_cnf->ip_version, hostname_or_ip, &olsr_ip) > 0) {
		if (allowed_ip(&olsr_ip)) {
#ifndef NODEBUG
			struct ipaddr_str strbuf;
#endif
			OLSR_PRINTF(2, "NAME PLUGIN: ip %s in service %s is OK\n", olsr_ip_to_string(&strbuf, &olsr_ip), service_line);
			free(hostname_or_ip);
			hostname_or_ip = NULL;
			return OLSR_TRUE;
		}
	}

	OLSR_PRINTF(1, "NAME PLUGIN: ip or hostname %s in service %s is NOT allowed (does not belong to you)\n", hostname_or_ip, service_line);
	free(hostname_or_ip);
	hostname_or_ip = NULL;

	return OLSR_FALSE;
}

/**
 * check if the service matches the syntax 
 * of "protocol://host:port/path|tcp_or_udp|a short description", 
 * which is given in the regex regex_t_service
 */
olsr_bool
is_service_wellformed(const char *service_line)
{
	return regexec(&regex_t_service, service_line, pmatch_service, regmatch_t_service, 0) == 0;
}

/**
 * check if the latlot matches the syntax 
 */
olsr_bool
is_latlon_wellformed(const char *latlon_line)
{
	int hna = -1;
	float a = 0.0, b = 0.0;
	sscanf(latlon_line, "%f,%f,%d", &a, &b, &hna);
	return (a != 0.0 && b != 0.0 && -1 != hna);
}

/**
 * Returns 1 if this olsrd announces inet
 */
olsr_bool get_isdefhna_latlon(void)
{
	struct ip_prefix_list *hna;
	for (hna = olsr_cnf->hna_entries; hna != NULL; hna = hna->next){
		if (hna->net.prefix_len == 0) {
			return OLSR_TRUE;
		}
	}
	return OLSR_FALSE;
}

/**
 * Grabs the current HNA selected default route
 */
void lookup_defhna_latlon(union olsr_ip_addr *ip)
{
  struct avl_node *rt_tree_node;
  struct olsr_ip_prefix prefix;

  memset(ip, 0, sizeof(ip));
  memset(&prefix, 0, sizeof(prefix));

  if (NULL != (rt_tree_node = avl_find(&routingtree, &prefix)))
  {
    *ip = ((struct rt_entry *)rt_tree_node->data)->rt_best->rtp_nexthop.gateway;
  }
}

/**
 * lookup a nodes name
 */
const char*
lookup_name_latlon(union olsr_ip_addr *ip)
{
	int hash;
	struct db_entry *entry;
	struct name_entry *name;
	for (hash = 0; hash < HASHSIZE; hash++) 
	{
		for(entry = list[hash]; entry != NULL; entry = entry->next)
		{
			for (name = entry->names; name != NULL; name = name->next) 
			{
				if (ipequal(&name->ip, ip)) return name->name;
			}
		}
	}
	return "";
}

#ifdef WIN32
/**
 * write latlon positions to a javascript file
 */
void
write_latlon_file(void)
{
  FILE* fmap;
  
  if (!my_names || !latlon_table_changed) return;
  
  OLSR_PRINTF(2, "NAME PLUGIN: writing latlon file\n");

  if (NULL == (fmap = fopen(my_latlon_file, "w"))) {
    OLSR_PRINTF(0, "NAME PLUGIN: cant write latlon file\n");
    return;
  }
  fprintf(fmap, "/* This file is overwritten regularly by olsrd */\n");
  mapwrite_work(fmap);  
  fclose(fmap);
  latlon_table_changed = OLSR_FALSE;
}
#endif

/*
 * Local Variables:
 * mode: c
 * c-indent-tabs-mode: t
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
