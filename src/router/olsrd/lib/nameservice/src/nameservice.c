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
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

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
#include "compat.h"


/* config parameters */
static char my_hosts_file[MAX_FILE + 1];
static char my_sighup_pid_file[MAX_FILE + 1];

static char my_add_hosts[MAX_FILE + 1];
static char my_suffix[MAX_SUFFIX];
static int my_interval = EMISSION_INTERVAL;
static double my_timeout = NAME_VALID_TIME;
static char my_resolv_file[MAX_FILE +1];
static char my_services_file[MAX_FILE + 1];
static char my_name_change_script[MAX_FILE + 1];
static char my_services_change_script[MAX_FILE + 1];
static char latlon_in_file[MAX_FILE + 1];
static char my_latlon_file[MAX_FILE + 1];
float my_lat = 0.0, my_lon = 0.0;

/* the databases (using hashing)
 * for hostnames, service_lines and dns-servers
 *
 * my own hostnames, service_lines and dns-servers
 * are store in a linked list (without hashing)
 * */
static struct list_node name_list[HASHSIZE];
struct name_entry *my_names = NULL;
struct timer_entry *name_table_write = NULL;
static olsr_bool name_table_changed = OLSR_TRUE;

static struct list_node service_list[HASHSIZE];
static struct name_entry *my_services = NULL;
static olsr_bool service_table_changed = OLSR_TRUE;

static struct list_node forwarder_list[HASHSIZE];
static struct name_entry *my_forwarders = NULL;
static olsr_bool forwarder_table_changed = OLSR_TRUE;

struct list_node latlon_list[HASHSIZE];
static olsr_bool latlon_table_changed = OLSR_TRUE;

/* backoff timer for writing changes into a file */
struct timer_entry *write_file_timer = NULL;

/* periodic message generation */
struct timer_entry *msg_gen_timer = NULL;

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
		strscat(my_hosts_file, "\\", sizeof(my_host_file));
	strscat(my_hosts_file, "hosts_olsr", sizeof(my_host_file));
	
	len = strlen(my_services_file);
	if (my_services_file[len - 1] != '\\')
		strscat(my_services_file, "\\", sizeof(my_services_file));
	strscat(my_services_file, "services_olsr", sizeof(my_services_file));

	len = strlen(my_resolv_file);
	if (my_resolv_file[len - 1] != '\\')
		strscat(my_resolv_file, "\\", sizeof(my_resolv_file));
	strscat(my_resolv_file, "resolvconf_olsr", sizeof(my_resolv_file));
#else
	strscpy(my_hosts_file, "/var/run/hosts_olsr", sizeof(my_hosts_file));
	strscpy(my_services_file, "/var/run/services_olsr", sizeof(my_services_file));
	strscpy(my_resolv_file, "/var/run/resolvconf_olsr", sizeof(my_resolv_file));
	*my_sighup_pid_file = 0;
#endif

	my_suffix[0] = '\0';
	my_add_hosts[0] = '\0';
	my_latlon_file[0] = '\0';
	latlon_in_file[0] = '\0';
	my_name_change_script[0] = '\0';
	my_services_change_script[0] = '\0';
	
	/* init the lists heads */
	for(i = 0; i < HASHSIZE; i++) {
		list_head_init(&name_list[i]);
		list_head_init(&forwarder_list[i]);
		list_head_init(&service_list[i]);
		list_head_init(&latlon_list[i]);
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
	if (data != NULL)
	{
		sscanf(value, "%f", (float*)data);
		OLSR_PRINTF(1, "%s float %f\n", "Got", *(float*)data);
	}
	else
	{
		OLSR_PRINTF(0, "%s float %s\n", "Ignored", value);
	}
	return 0;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
    { .name = "interval",      .set_plugin_parameter = &set_plugin_int,         .data = &my_interval },
    { .name = "timeout",       .set_plugin_parameter = &set_nameservice_float,  .data = &my_timeout },
    { .name = "sighup-pid-file",.set_plugin_parameter = &set_plugin_string,      .data = &my_sighup_pid_file, .addon = {sizeof(my_sighup_pid_file)} },
    { .name = "hosts-file",    .set_plugin_parameter = &set_plugin_string,      .data = &my_hosts_file,    .addon = {sizeof(my_hosts_file)} },
    { .name = "name-change-script", .set_plugin_parameter = &set_plugin_string,  .data = &my_name_change_script, .addon = {sizeof(my_name_change_script)} },
    { .name = "services-change-script", .set_plugin_parameter = &set_plugin_string, .data = &my_services_change_script, .addon = {sizeof(my_services_change_script)} },
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
	size_t regex_size = 256 * sizeof(char) + strlen(my_suffix);
	char *regex_service = olsr_malloc(regex_size, "new *char from name_init for regex_service");
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
	strscpy(regex_service, "^[[:alnum:]]+://(([[:alnum:]_.-]+", regex_size);
	strscat(regex_service, my_suffix, regex_size);
	strscat(regex_service, ")|([[:digit:]]{1,3}\\.[[:digit:]]{1,3}\\.[[:digit:]]{1,3}\\.[[:digit:]]{1,3})):[[:digit:]]+[[:alnum:]/?._=#-]*\\|(tcp|udp)\\|[^|[:cntrl:]]+$", regex_size);

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

	/* periodic message generation */
	msg_gen_timer = olsr_start_timer(my_interval * MSEC_PER_SEC, EMISSION_JITTER,
									 OLSR_TIMER_PERIODIC, &olsr_namesvc_gen,
									 NULL, 0);

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
		struct ipaddr_str strbuf;
		OLSR_PRINTF(1, "NAME PLUGIN: invalid or malformed parameter %s (%s), fix your config!\n", my_list->name, olsr_ip_to_string(&strbuf, &my_list->ip));
		next = my_list->next;
		free(my_list->name);
		my_list->name = NULL;
		free(my_list);
		my_list = NULL;
		return remove_nonvalid_names_from_list(next, type);
	} else {
		struct ipaddr_str strbuf;
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

	free_all_list_entries(name_list);
	free_all_list_entries(service_list);
	free_all_list_entries(forwarder_list);
	free_all_list_entries(latlon_list);

	olsr_stop_timer(write_file_timer);
	olsr_stop_timer(msg_gen_timer);

	regfree(&regex_t_name);
	regfree(&regex_t_service);
	mapwrite_exit();
}

/* free all list entries */
void 
free_all_list_entries(struct list_node *this_db_list) 
{
	struct db_entry *db;
	struct list_node *list_head, *list_node, *list_node_next;

	int i;
	
	for (i = 0; i < HASHSIZE; i++) {

		list_head = &this_db_list[i];

		for (list_node = list_head->next;
			 list_node != list_head;
			 list_node = list_node_next) {

			/* prefetch next node before loosing context */
			list_node_next = list_node->next;

			db = list2db(list_node);
			olsr_namesvc_delete_db_entry(db);
		}
	}
}


/**
 * The write file timer has fired.
 */
void
olsr_expire_write_file_timer(void *context __attribute__((unused)))
{
	write_file_timer = NULL;

	write_resolv_file();   /* if forwarder_table_changed */
	write_hosts_file();    /* if name_table_changed */
	write_services_file(); /* if service_table_changed */
#ifdef WIN32
	write_latlon_file();   /* if latlon_table_changed */
#endif
}


/*
 * Kick a timer to write everything into a file.
 * This also paces things a bit.
 */
static void
olsr_start_write_file_timer(void)
{
	if (write_file_timer) {
		return;
	}

	write_file_timer = olsr_start_timer(5 * MSEC_PER_SEC, 5, OLSR_TIMER_ONESHOT,
										olsr_expire_write_file_timer, NULL, 0);
}

/*
 * Delete and unlink db_entry.
 */
void
olsr_namesvc_delete_db_entry(struct db_entry *db)
{
	struct ipaddr_str strbuf;
	OLSR_PRINTF(2, "NAME PLUGIN: %s timed out... deleting\n", 
				olsr_ip_to_string(&strbuf, &db->originator));

	olsr_start_write_file_timer();
	olsr_stop_timer(db->db_timer); /* stop timer if running */
	
	/* Delete */
	free_name_entry_list(&db->names);
	list_remove(&db->db_list);
	free(db);
}

/**
 * Callback for the db validity timer.
 */
static void
olsr_nameservice_expire_db_timer(void *context)
{
  struct db_entry *db;

  db = (struct db_entry *)context;
  db->db_timer = NULL; /* be pedandic */

  olsr_namesvc_delete_db_entry(db);
}

/**
 * Scheduled event: generate and send NAME packet
 */
void
olsr_namesvc_gen(void *foo __attribute__((unused)))
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
		message->v4.olsr_vtime = reltime_to_me(my_timeout * MSEC_PER_SEC);
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
		message->v6.olsr_vtime = reltime_to_me(my_timeout * MSEC_PER_SEC);
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
olsr_parser(union olsr_message *m,
			struct interface *in_if __attribute__((unused)),
			union olsr_ip_addr *ipaddr)
{
	struct namemsg *namemessage;
	union olsr_ip_addr originator;
	olsr_reltime vtime;
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
		vtime = me_to_reltime(m->v4.olsr_vtime);
		size = ntohs(m->v4.olsr_msgsize);
		namemessage = (struct namemsg*)&m->v4.message;
	}
	else {
		vtime = me_to_reltime(m->v6.olsr_vtime);
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
		struct ipaddr_str strbuf;
		OLSR_PRINTF(3, "NAME PLUGIN: Received msg from NON SYM neighbor %s\n", olsr_ip_to_string(&strbuf, ipaddr));
		return;
	}

	update_name_entry(&originator, namemessage, size, vtime);

	/* Forward the message if nessecary
	* default_fwd does all the work for us! */
	olsr_forward_message(m, ipaddr);
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
	struct ipaddr_str strbuf;
	OLSR_PRINTF(3, "NAME PLUGIN: Announcing name %s (%s) %d\n", 
		from->name, olsr_ip_to_string(&strbuf, &from->ip), from->len);
	to->type = htons(from->type);
	to->len = htons(from->len);
	to->ip = from->ip;
	pos += sizeof(struct name);
	memcpy(pos, from->name, from->len);
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
	struct ipaddr_str strbuf;
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
				strscpy(already_saved_name_entries->name, name, len_of_name + 1);

				*this_table_changed = OLSR_TRUE;
				olsr_start_write_file_timer();
			}
			if (!ipequal(&already_saved_name_entries->ip, &from_packet->ip))
			{
				struct ipaddr_str strbuf2, strbuf3;
				OLSR_PRINTF(4, "NAME PLUGIN: updating ip %s -> %s (%s)\n",
					olsr_ip_to_string(&strbuf, &already_saved_name_entries->ip),
					olsr_ip_to_string(&strbuf2, &from_packet->ip),
					olsr_ip_to_string(&strbuf3, &already_saved_name_entries->ip));
				already_saved_name_entries->ip = from_packet->ip;

				*this_table_changed = OLSR_TRUE;
				olsr_start_write_file_timer();
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
	tmp->name = olsr_malloc(tmp->len + 1, "new name_entry name");
	tmp->ip = from_packet->ip;
	strscpy(tmp->name, name, tmp->len + 1);

	OLSR_PRINTF(3, "\nNAME PLUGIN: create new name/service/forwarder entry %s (%s) [len=%d] [type=%d] in linked list\n", 
		tmp->name, olsr_ip_to_string(&strbuf, &tmp->ip), tmp->len, tmp->type);

	*this_table_changed = OLSR_TRUE;
	olsr_start_write_file_timer();

	// queue to front
	tmp->next = *to;
	*to = tmp;
}


/**
 * unpack the received message and delegate to the decapsulation function for each
 * name/service/forwarder entry in the message
 */
void
update_name_entry(union olsr_ip_addr *originator, struct namemsg *msg, int msg_size, olsr_reltime vtime)
{
	struct ipaddr_str strbuf;
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
				insert_new_name_in_list(originator, name_list, from_packet,
										&name_table_changed, vtime); 
				break;
			case NAME_FORWARDER:
				insert_new_name_in_list(originator, forwarder_list, from_packet,
										&forwarder_table_changed, vtime); 
				break;
			case NAME_SERVICE:
				insert_new_name_in_list(originator, service_list, from_packet,
										&service_table_changed, vtime); 
				break;
			case NAME_LATLON:
				insert_new_name_in_list(originator, latlon_list, from_packet,
										&latlon_table_changed, vtime);
				break;
			default:
				OLSR_PRINTF(3, "NAME PLUGIN: Received Message of unknown type [%d] from (%s)\n",
							from_packet->type, olsr_ip_to_string(&strbuf, originator));
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
insert_new_name_in_list(union olsr_ip_addr *originator,
						struct list_node *this_list,
						struct name *from_packet, olsr_bool *this_table_changed,
						olsr_reltime vtime)
{
	int hash;
	struct db_entry *entry;
	struct list_node *list_head, *list_node;

	olsr_bool entry_found = OLSR_FALSE;

	hash = olsr_ip_hashing(originator);

	/* find the entry for originator, if there is already one */
	list_head = &this_list[hash];
	for (list_node = list_head->next; list_node != list_head;
		 list_node = list_node->next) {

		entry = list2db(list_node);

		if (ipequal(originator, &entry->originator)) {
			struct ipaddr_str strbuf;
			// found
			OLSR_PRINTF(4, "NAME PLUGIN: found entry for (%s) in its hash table\n",
						olsr_ip_to_string(&strbuf, originator));

			//delegate to function for parsing the packet and linking it to entry->names
			decap_namemsg(from_packet, &entry->names, this_table_changed);

			olsr_set_timer(&entry->db_timer, vtime,
						   OLSR_NAMESVC_DB_JITTER, OLSR_TIMER_ONESHOT,
						   &olsr_nameservice_expire_db_timer, entry, 0);

			entry_found = OLSR_TRUE;
		}
	}

	if (! entry_found)
	{
		struct ipaddr_str strbuf;
		OLSR_PRINTF(3, "NAME PLUGIN: create new db entry for ip (%s) in hash table\n",
					olsr_ip_to_string(&strbuf, originator));

		/* insert a new entry */
		entry = olsr_malloc(sizeof(struct db_entry), "new db_entry");
		memset(entry, 0, sizeof(struct db_entry));

		entry->originator = *originator;

		olsr_set_timer(&entry->db_timer, vtime,
					   OLSR_LINK_LOSS_JITTER, OLSR_TIMER_ONESHOT,
					   &olsr_nameservice_expire_db_timer, entry, 0);

		entry->names = NULL;

		/* insert to the list */
		list_add_before(&this_list[hash], &entry->db_list);
		
		//delegate to function for parsing the packet and linking it to entry->names
		decap_namemsg(from_packet, &entry->names, this_table_changed);
	}
}

#ifndef WIN32
static void
send_sighup_to_pidfile(char * pid_file){
	int fd;
	int i=0;
	int result;
	pid_t ipid;
	char line[20];
	char * endptr;

	fd = open(pid_file, O_RDONLY);
	if (fd<0) {
		OLSR_PRINTF(2, "NAME PLUGIN: can't open file %s\n", pid_file);
		return;
	}

	while (i<19) {
		result = read(fd, line+i, 19-i);
		if (!result) { /* EOF */
			break;
		} else if (result>0) {
			i += result;
		} else if(errno!=EINTR && errno!=EAGAIN) {
			OLSR_PRINTF(2, "NAME PLUGIN: can't read file %s\n", pid_file);
			return;
		}
	}
	line[i]=0;
	close(fd);
	ipid = strtol(line, &endptr, 0);
	if (endptr==line) {
		OLSR_PRINTF(2, "NAME PLUGIN: invalid pid at file %s\n", pid_file);
		return;	
	}

	result=kill(ipid, SIGHUP);
	if (result==0){
		OLSR_PRINTF(2, "NAME PLUGIN: SIGHUP sent to pid %i\n", ipid);	
	} else {
		OLSR_PRINTF(2, "NAME PLUGIN: failed to send SIGHUP to pid %i\n", ipid);
	}

}
#endif

/**
 * write names to a file in /etc/hosts compatible format
 */
void
write_hosts_file(void)
{
	int hash;
	struct name_entry *name;
	struct db_entry *entry;
	struct list_node *list_head, *list_node;
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
	for (hash = 0; hash < HASHSIZE; hash++) {
		list_head = &name_list[hash];
		for (list_node = list_head->next; list_node != list_head;
			 list_node = list_node->next) {

			entry = list2db(list_node);

			for (name = entry->names; name != NULL; name = name->next) {
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

#ifndef WIN32
	if (*my_sighup_pid_file)
		send_sighup_to_pidfile(my_sighup_pid_file);
#endif
	name_table_changed = OLSR_FALSE;

	// Executes my_name_change_script after writing the hosts file
        if (my_name_change_script[0] != '\0') {
		if(system(my_name_change_script) != -1) {
			OLSR_PRINTF(2, "NAME PLUGIN: Name changed, %s executed\n", my_name_change_script);
		}
		else {
			OLSR_PRINTF(2, "NAME PLUGIN: WARNING! Failed to execute %s on hosts change\n", my_name_change_script);
		}
	}
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
	struct list_node *list_head, *list_node;
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
	for (hash = 0; hash < HASHSIZE; hash++) {
		list_head = &service_list[hash];
		for (list_node = list_head->next; list_node != list_head;
			 list_node = list_node->next) {

			entry = list2db(list_node);

			for (name = entry->names; name != NULL; name = name->next) {
				struct ipaddr_str strbuf;
				OLSR_PRINTF(6, "%s\t",  name->name);
				OLSR_PRINTF(6, "\t#%s\n",
							olsr_ip_to_string(&strbuf, &entry->originator));

				fprintf(services_file, "%s\t", name->name );
				fprintf(services_file, "\t#%s\n",
						olsr_ip_to_string(&strbuf, &entry->originator));
			}
		}
	}

	if (time(&currtime)) {
		fprintf(services_file, "\n### written by olsrd at %s", ctime(&currtime));
	}
	  
	fclose(services_file);
	service_table_changed = OLSR_FALSE;
	
	// Executes my_services_change_script after writing the services file
	if (my_services_change_script[0] != '\0') {
		if(system(my_services_change_script) != -1) {
			OLSR_PRINTF(2, "NAME PLUGIN: Service changed, %s executed\n", my_services_change_script);
		}
		else {
			OLSR_PRINTF(2, "NAME PLUGIN: WARNING! Failed to execute %s on service change\n", my_services_change_script);
		}
	}
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
			struct lqtextbuffer lqbuffer;
#endif
			/*
			 * first is better, swap the pointers.
			 */
			OLSR_PRINTF(6, "NAME PLUGIN: nameserver %s, cost %s\n",
						olsr_ip_to_string(&strbuf, &rt1->rt_dst.prefix),
						get_linkcost_text(rt1->rt_best->rtp_metric.cost, OLSR_TRUE, &lqbuffer));

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
	struct list_node *list_head, *list_node;
	struct rt_entry *route;
	static struct rt_entry *nameserver_routes[NAMESERVER_COUNT+1];
	FILE* resolv;
	int i=0;
	time_t currtime;

	if (!forwarder_table_changed || my_forwarders != NULL || my_resolv_file[0] == '\0')
		return;

	/* clear the array of 3+1 nameserver routes */
	memset(nameserver_routes, 0, sizeof(nameserver_routes));

	for (hash = 0; hash < HASHSIZE; hash++) {
		list_head = &forwarder_list[hash];
		for (list_node = list_head->next; list_node != list_head;
			 list_node = list_node->next) {

			entry = list2db(list_node);

			for (name = entry->names; name != NULL; name = name->next) {
#ifndef NODEBUG
				struct ipaddr_str strbuf;
				struct lqtextbuffer lqbuffer;
#endif
				route = olsr_lookup_routing_table(&name->ip);

				OLSR_PRINTF(6, "NAME PLUGIN: check route for nameserver %s %s",
							olsr_ip_to_string(&strbuf, &name->ip),
							route ? "suceeded" : "failed");

				if (route==NULL) // it's possible that route is not present yet
					continue;

				/* enqueue it on the head of list */
				*nameserver_routes = route;
				OLSR_PRINTF(6, "NAME PLUGIN: found nameserver %s, cost %s",
							olsr_ip_to_string(&strbuf, &name->ip),
							get_linkcost_text(route->rt_best->rtp_metric.cost, OLSR_TRUE, &lqbuffer));

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

		/* flag changes */
		switch (to_delete->type) {
		case NAME_HOST:
			name_table_changed = OLSR_TRUE;
			break;
		case NAME_FORWARDER:
			forwarder_table_changed = OLSR_TRUE;
			break;
		case NAME_SERVICE:
			service_table_changed = OLSR_TRUE;
			break;
		case NAME_LATLON:
			latlon_table_changed = OLSR_TRUE;
			break;
		default:
			break;
		}

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
	struct ipaddr_str strbuf;
	
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
			struct ipaddr_str strbuf;
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
  struct rt_entry *rt;
  struct avl_node *rt_tree_node;
  struct olsr_ip_prefix prefix;

  memset(ip, 0, sizeof(ip));
  memset(&prefix, 0, sizeof(prefix));

  if (NULL != (rt_tree_node = avl_find(&routingtree, &prefix))) {
	rt = rt_tree2rt(rt_tree_node);
    *ip = rt->rt_best->rtp_nexthop.gateway;
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
	struct list_node *list_head, *list_node;
	struct name_entry *name;

	for (hash = 0; hash < HASHSIZE; hash++) {
		list_head = &name_list[hash];
		for (list_node = list_head->next; list_node != list_head;
			 list_node = list_node->next) {

			entry = list2db(list_node);

			for (name = entry->names; name != NULL; name = name->next) {
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
