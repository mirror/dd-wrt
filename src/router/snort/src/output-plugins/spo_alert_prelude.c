/*****
*
* Copyright (C) 2005 PreludeIDS Technologies. All Rights Reserved.
* Author: Yoann Vandoorselaere <yoann.v@prelude-ids.com>
*
* This file is part of the Snort program.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License Version 2 as
* published by the Free Software Foundation.  You may not use, modify or
* distribute this program under any other version of the GNU General
* Public License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#ifdef HAVE_LIBPRELUDE

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libprelude/prelude.h>

#include "event.h"
#include "decode.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "debug.h"
#include "util.h"
#include "mstring.h"
#include "snort.h"
#include "sfdaq.h"

#define ANALYZER_CLASS "NIDS"
#define ANALYZER_MODEL "Snort"
#define ANALYZER_MANUFACTURER "http://www.snort.org"
#define ANALYZER_SID_URL "http://www.snort.org/search/sid/"

#define SNORT_MAX_OWNED_SID 1000000
#define DEFAULT_ANALYZER_NAME "snort"


extern OptTreeNode *otn_tmp;

static char *init_args = NULL;
static unsigned int info_priority = 4;
static unsigned int low_priority  = 3;
static unsigned int mid_priority  = 2;
static prelude_bool_t initialized = FALSE;


static int setup_analyzer(idmef_analyzer_t *analyzer)
{
        int ret;
        prelude_string_t *string;
        
        ret = idmef_analyzer_new_model(analyzer, &string);
        if ( ret < 0 )
                return ret;
        prelude_string_set_constant(string, ANALYZER_MODEL);

        ret = idmef_analyzer_new_class(analyzer, &string);
        if ( ret < 0 )
                return ret;
        prelude_string_set_constant(string, ANALYZER_CLASS);

        ret = idmef_analyzer_new_manufacturer(analyzer, &string);
        if ( ret < 0 ) 
                return ret;
        prelude_string_set_constant(string, ANALYZER_MANUFACTURER);

        ret = idmef_analyzer_new_version(analyzer, &string);
        if ( ret < 0 )
                return ret;
        prelude_string_set_constant(string, VERSION);

        return 0;
}



static idmef_reference_origin_t reference_to_origin(const char *name)
{
        int i, ret;
        struct {
                const char *name;
                idmef_reference_origin_t origin;
        } tbl[] = {
                { "cve", IDMEF_REFERENCE_ORIGIN_CVE             },
                { "bugtraq", IDMEF_REFERENCE_ORIGIN_BUGTRAQID   },
                { "osvdb", IDMEF_REFERENCE_ORIGIN_OSVDB         },
                { NULL, 0                                       }
        };

        for ( i = 0; tbl[i].name; i++ ) {
                ret = strcmp(tbl[i].name, name);
                if ( ret == 0 )
                        return tbl[i].origin;
        }

        return IDMEF_REFERENCE_ORIGIN_VENDOR_SPECIFIC;
}



static int event_to_source_target(Packet *p, idmef_alert_t *alert)
{
        int ret;
        idmef_node_t *node;
        idmef_source_t *source;
        idmef_target_t *target;
        idmef_address_t *address;
        idmef_service_t *service;
        prelude_string_t *string;
        static char saddr[128], daddr[128];

        if ( !p )
            return 0;

        if ( ! IPH_IS_VALID(p) )
                return 0;
        
        ret = idmef_alert_new_source(alert, &source, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;

        ret = idmef_source_new_interface(source, &string);
        if ( ret < 0 )
            return ret;
        prelude_string_set_ref(string, PRINT_INTERFACE(DAQ_GetInterfaceSpec()));
        
        ret = idmef_source_new_service(source, &service);
        if ( ret < 0 )
                return ret;

        if ( p->tcph || p->udph )
                idmef_service_set_port(service, p->sp);
        
        idmef_service_set_ip_version(service, GET_IPH_VER(p));
        idmef_service_set_iana_protocol_number(service, GET_IPH_PROTO(p));
        
        ret = idmef_source_new_node(source, &node);
        if ( ret < 0 )
                return ret;

        ret = idmef_node_new_address(node, &address, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;

        ret = idmef_address_new_address(address, &string);
        if ( ret < 0 )
                return ret;
        
        SnortSnprintf(saddr, sizeof(saddr), "%s", inet_ntoa(GET_SRC_ADDR(p)));
        prelude_string_set_ref(string, saddr);

        ret = idmef_alert_new_target(alert, &target, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;

        ret = idmef_target_new_interface(target, &string);
        if ( ret < 0 )
            return ret;
        prelude_string_set_ref(string, PRINT_INTERFACE(DAQ_GetInterfaceSpec()));

        ret = idmef_target_new_service(target, &service);
        if ( ! ret < 0 )
                return ret;
        
        if ( p->tcph || p->udph )                
                idmef_service_set_port(service, p->dp);
        
        idmef_service_set_ip_version(service, GET_IPH_VER(p));
        idmef_service_set_iana_protocol_number(service, GET_IPH_PROTO(p));
        
        ret = idmef_target_new_node(target, &node);
        if ( ret < 0 )
                return ret;
        
        ret = idmef_node_new_address(node, &address, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;
        
        ret = idmef_address_new_address(address, &string);
        if ( ret < 0 )
                return ret;
                
        SnortSnprintf(daddr, sizeof(daddr), "%s", inet_ntoa(GET_DST_ADDR(p)));
        prelude_string_set_ref(string, daddr);
        
        return 0;
}



static int add_byte_data(idmef_alert_t *alert, const char *meaning, const unsigned char *data, size_t size)
{
        int ret;
        prelude_string_t *str;
        idmef_additional_data_t *ad;

        if ( ! data || ! size )
                return 0;
        
        ret = idmef_alert_new_additional_data(alert, &ad, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;

        ret = idmef_additional_data_set_byte_string_ref(ad, data, size);
        if ( ret < 0 ) {
                ErrorMessage("%s: error setting byte string data: %s.\n",
                             prelude_strsource(ret), prelude_strerror(ret));
                return -1;
        }

        ret = idmef_additional_data_new_meaning(ad, &str);
        if ( ret < 0 ) {
                ErrorMessage("%s: error creating additional-data meaning: %s.\n",
                             prelude_strsource(ret), prelude_strerror(ret));
                return -1;
        }
        
        ret = prelude_string_set_ref(str, meaning);
        if ( ret < 0 ) {
                ErrorMessage("%s: error setting byte string data meaning: %s.\n",
                             prelude_strsource(ret), prelude_strerror(ret));
                return -1;
        }
                
        return 0;
}



static int add_string_data(idmef_alert_t *alert, const char *meaning, const char *data)
{
        int ret;
        prelude_string_t *str;
        idmef_additional_data_t *ad;

        if ( ! data )
                return 0;
        
        ret = idmef_alert_new_additional_data(alert, &ad, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;

        ret = idmef_additional_data_set_string_ref(ad, data);
        if ( ret < 0 ) {
                ErrorMessage("%s: error setting string data: %s.\n",
                             prelude_strsource(ret), prelude_strerror(ret));
                return -1;
        }

        ret = idmef_additional_data_new_meaning(ad, &str);
        if ( ret < 0 ) {
                ErrorMessage("%s: error creating additional-data meaning: %s.\n",
                             prelude_strsource(ret), prelude_strerror(ret));
                return -1;
        }
        
        ret = prelude_string_set_ref(str, meaning);
        if ( ret < 0 ) {
                ErrorMessage("%s: error setting string data meaning: %s.\n",
                             prelude_strsource(ret), prelude_strerror(ret));
                return -1;
        }
        
        return 0;
}



static int add_int_data(idmef_alert_t *alert, const char *meaning, uint32_t data)
{
        int ret;
        prelude_string_t *str;
        idmef_additional_data_t *ad;
        
        ret = idmef_alert_new_additional_data(alert, &ad, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;
        
        idmef_additional_data_set_integer(ad, data);

        ret = idmef_additional_data_new_meaning(ad, &str);
        if ( ret < 0 ) {
                ErrorMessage("%s: error creating additional-data meaning: %s.\n",
                             prelude_strsource(ret), prelude_strerror(ret));
                return -1;
        }
        
        ret = prelude_string_set_ref(str, meaning);
        if ( ret < 0 ) {
                ErrorMessage("%s: error setting integer data meaning: %s.\n",
                             prelude_strsource(ret), prelude_strerror(ret));
                return -1;
        }
        
        return 0;
}




static int packet_to_data(Packet *p, Event *event, idmef_alert_t *alert)
{
        int i;
        
        if ( ! p )
            return 0;

        add_int_data(alert, "snort_rule_sid", event->sig_id);
        add_int_data(alert, "snort_rule_rev", event->sig_rev);
        
        if ( IPH_IS_VALID(p) ) {
                add_int_data(alert, "ip_ver", GET_IPH_VER(p));
                add_int_data(alert, "ip_hlen", GET_IPH_HLEN(p));
                add_int_data(alert, "ip_tos", GET_IPH_TOS(p));
                add_int_data(alert, "ip_len", ntohs(GET_IPH_LEN(p)));
#ifdef SUP_IP6
// XXX-IPv6 need fragmentation ID
#else
                add_int_data(alert, "ip_id", ntohs(p->iph->ip_id));
#endif
#ifdef SUP_IP6
// XXX-IPv6 need fragmentation offset
#else
                add_int_data(alert, "ip_off", ntohs(p->iph->ip_off));
#endif
                add_int_data(alert, "ip_ttl", GET_IPH_TTL(p));
                add_int_data(alert, "ip_proto", GET_IPH_PROTO(p));
#ifdef SUP_IP6
// XXX-IPv6 need checksum
#else
                add_int_data(alert, "ip_sum", ntohs(p->iph->ip_csum));
#endif
                
                for ( i = 0; i < p->ip_option_count; i++ ) {
                        add_int_data(alert, "ip_option_code", p->ip_options[i].code);
                        add_byte_data(alert, "ip_option_data", 
                            p->ip_options[i].data, p->ip_options[i].len);        
                }
        }
        
        if ( p->tcph ) {
                add_int_data(alert, "tcp_seq", ntohl(p->tcph->th_seq));
                add_int_data(alert, "tcp_ack", ntohl(p->tcph->th_ack));
                
                add_int_data(alert, "tcp_off", TCP_OFFSET(p->tcph));
                add_int_data(alert, "tcp_res", TCP_X2(p->tcph));
                add_int_data(alert, "tcp_flags", p->tcph->th_flags);

                add_int_data(alert, "tcp_win", ntohs(p->tcph->th_win));
                add_int_data(alert, "tcp_sum", ntohs(p->tcph->th_sum));
                add_int_data(alert, "tcp_urp", ntohs(p->tcph->th_urp));

                
                for ( i = 0; i < p->tcp_option_count; i++ ) {
                        add_int_data(alert, "tcp_option_code", p->tcp_options[i].code);
                        add_byte_data(alert, "tcp_option_data", p->tcp_options[i].data, p->tcp_options[i].len);        
                }
        }

        else if ( p->udph ) {
                add_int_data(alert, "udp_len", ntohs(p->udph->uh_len));
                add_int_data(alert, "udp_sum", ntohs(p->udph->uh_chk));
        }

        else if ( p->icmph ) {
                add_int_data(alert, "icmp_type", p->icmph->type);
                add_int_data(alert, "icmp_code", p->icmph->code);
                add_int_data(alert, "icmp_sum", ntohs(p->icmph->csum));

                switch ( p->icmph->type ) {
                        
                case ICMP_ECHO:
                case ICMP_ECHOREPLY:
                case ICMP_INFO_REQUEST:
                case ICMP_INFO_REPLY:
                case ICMP_ADDRESS:
                case ICMP_TIMESTAMP:
                        add_int_data(alert, "icmp_id", ntohs(p->icmph->s_icmp_id));
                        add_int_data(alert, "icmp_seq", ntohs(p->icmph->s_icmp_seq));
                        break;
                        
                case ICMP_ADDRESSREPLY:
                        add_int_data(alert, "icmp_id", ntohs(p->icmph->s_icmp_id));
                        add_int_data(alert, "icmp_seq", ntohs(p->icmph->s_icmp_seq));
                        add_int_data(alert, "icmp_mask", (uint32_t) ntohl(p->icmph->s_icmp_mask));
                        break;
                
                case ICMP_REDIRECT:
#ifndef SUP_IP6
                        add_string_data(alert, "icmp_gwaddr", inet_ntoa(p->icmph->s_icmp_gwaddr));
#else
                        {
                            sfip_t gwaddr;
                            sfip_set_raw(&gwaddr, (void *)&p->icmph->s_icmp_gwaddr.s_addr, AF_INET);
                            add_string_data(alert, "icmp_gwaddr", inet_ntoa(&gwaddr));
                        }
#endif
                        break;
                
                case ICMP_ROUTER_ADVERTISE:
                        add_int_data(alert, "icmp_num_addrs", p->icmph->s_icmp_num_addrs);
                        add_int_data(alert, "icmp_wpa", p->icmph->s_icmp_wpa);
                        add_int_data(alert, "icmp_lifetime", ntohs(p->icmph->s_icmp_lifetime));
                        break;
                
                case ICMP_TIMESTAMPREPLY:
                        add_int_data(alert, "icmp_id", ntohs(p->icmph->s_icmp_id));
                        add_int_data(alert, "icmp_seq", ntohs(p->icmph->s_icmp_seq));
                        add_int_data(alert, "icmp_otime", p->icmph->s_icmp_otime);
                        add_int_data(alert, "icmp_rtime", p->icmph->s_icmp_rtime);
                        add_int_data(alert, "icmp_ttime", p->icmph->s_icmp_ttime);
                        break;
                }
        }

        add_byte_data(alert, "payload", p->data, p->dsize);
        
        return 0;
}



static int event_to_impact(Event *event, idmef_alert_t *alert)
{
        int ret;
        ClassType *classtype;
        prelude_string_t *str;
        idmef_impact_t *impact;
        idmef_assessment_t *assessment;
        idmef_impact_severity_t severity;
        
        ret = idmef_alert_new_assessment(alert, &assessment);
        if ( ret < 0 )
                return ret;

        ret = idmef_assessment_new_impact(assessment, &impact);
        if ( ret < 0 )
                return ret;

        if ( event->priority < mid_priority )
                severity = IDMEF_IMPACT_SEVERITY_HIGH;

        else if ( event->priority < low_priority )
                severity = IDMEF_IMPACT_SEVERITY_MEDIUM;

        else if ( event->priority < info_priority )
                severity = IDMEF_IMPACT_SEVERITY_LOW;

        else    severity = IDMEF_IMPACT_SEVERITY_INFO;

        idmef_impact_set_severity(impact, severity);

        if ( ! otn_tmp )
                return 0;
        
        classtype = otn_tmp->sigInfo.classType;
        if ( classtype ) {
                ret = idmef_impact_new_description(impact, &str);
                if ( ret < 0 )
                        return ret;

                prelude_string_set_ref(str, classtype->name);
        }
        
        return 0;
}



static int add_snort_reference(idmef_classification_t *class, int gen_id, int sig_id)
{
        int ret;
        prelude_string_t *str;
        idmef_reference_t *ref;

        if ( sig_id >= SNORT_MAX_OWNED_SID )
                return 0;
        
        ret = idmef_classification_new_reference(class, &ref, IDMEF_LIST_APPEND);
        if ( ret < 0 )
                return ret;
        
        ret = idmef_reference_new_name(ref, &str);
        if ( ret < 0 )
                return ret;
        
        idmef_reference_set_origin(ref, IDMEF_REFERENCE_ORIGIN_VENDOR_SPECIFIC);

        if ( gen_id == 0 )
                ret = prelude_string_sprintf(str, "%u", sig_id);
        else
                ret = prelude_string_sprintf(str, "%u:%u", gen_id, sig_id);

        if ( ret < 0 )
                return ret;

        ret = idmef_reference_new_meaning(ref, &str);
        if ( ret < 0 )
                return ret;

        ret = prelude_string_sprintf(str, "Snort Signature ID");
        if ( ret < 0 )
                return ret;
        
        ret = idmef_reference_new_url(ref, &str);
        if ( ret < 0 )
                return ret;

        if ( gen_id == 0 )
                ret = prelude_string_sprintf(str, ANALYZER_SID_URL "%u", sig_id);
        else
                ret = prelude_string_sprintf(str, ANALYZER_SID_URL "%u-%u", gen_id, sig_id);
         
        return ret;
}



static int event_to_reference(Event *event, idmef_classification_t *class)
{
        int ret;
        ReferenceNode *refs;
        prelude_string_t *str;
        idmef_reference_t *ref;
        ReferenceSystemNode *system;

        ret = idmef_classification_new_ident(class, &str);
        if ( ret < 0 )
                return ret;

        if ( event->sig_generator == 0 )
                ret = prelude_string_sprintf(str, "%u", event->sig_id);
        else
                ret = prelude_string_sprintf(str, "%u:%u", event->sig_generator, event->sig_id);
        if ( ret < 0 )
                return ret;

        ret = add_snort_reference(class, event->sig_generator, event->sig_id);
        if ( ret < 0 )
                return ret;

        /*
         * return if we have no information about the rule.
         */
        if ( ! otn_tmp )
                return 0;

        for ( refs = otn_tmp->sigInfo.refs; refs != NULL; refs = refs->next ) {

                system = refs->system;
                if ( ! system )
                        continue;
                
                ret = idmef_classification_new_reference(class, &ref, IDMEF_LIST_APPEND);
                if ( ret < 0 )
                        return ret;

                ret = idmef_reference_new_name(ref, &str);
                if ( ret < 0 )
                        return ret;
                
                idmef_reference_set_origin(ref, reference_to_origin(system->name));
                if ( idmef_reference_get_origin(ref) != IDMEF_REFERENCE_ORIGIN_VENDOR_SPECIFIC )
                        prelude_string_set_ref(str, refs->id);
                else
                        prelude_string_set_constant(str, "url");

                ret = idmef_reference_new_url(ref, &str);
                if ( ret < 0 )
                        return ret;
                
                prelude_string_sprintf(str, "%s%s", system->url ? system->url : "", refs->id ? refs->id : "");
        }        

        return 0;
}



void snort_alert_prelude(Packet *p, char *msg, void *data, Event *event)
{
        int ret;
        idmef_time_t *time;
        idmef_alert_t *alert;
        prelude_string_t *str;
        idmef_message_t *idmef;
        idmef_classification_t *class;
        prelude_client_t *client = data;
        struct timeval tv;

        if ( !p )
            return;

        ret = idmef_message_new(&idmef);
        if ( ret < 0 )
                return;

        ret = idmef_message_new_alert(idmef, &alert);
        if ( ret < 0 )
                goto err;

        ret = idmef_alert_new_classification(alert, &class);
        if ( ret < 0 )
                goto err;

        ret = idmef_classification_new_text(class, &str);
        if ( ret < 0 )
                goto err;

        prelude_string_set_ref(str, msg);

        ret = event_to_impact(event, alert);
        if ( ret < 0 )
                goto err;

        ret = event_to_reference(event, class);
        if ( ret < 0 )
                goto err;
        
        ret = event_to_source_target(p, alert);
        if ( ret < 0 )
                goto err;
        
        ret = packet_to_data(p, event, alert);
        if ( ret < 0 )
                goto err;
        
        ret = idmef_alert_new_detect_time(alert, &time);
        if ( ret < 0 )
                goto err;

        tv.tv_sec = p->pkth->ts.tv_sec;
        tv.tv_usec = p->pkth->ts.tv_usec;
        idmef_time_set_from_timeval(time, &tv);
        
        ret = idmef_time_new_from_gettimeofday(&time);
        if ( ret < 0 )
                goto err; 
        idmef_alert_set_create_time(alert, time);
                
        idmef_alert_set_analyzer(alert, idmef_analyzer_ref(prelude_client_get_analyzer(client)), IDMEF_LIST_PREPEND);
        prelude_client_send_idmef(client, idmef);
                
 err:
        idmef_message_destroy(idmef);
}



static void snort_alert_prelude_clean_exit(int signal, void *data)

{
        /*
         * A Snort sensor reporting to Prelude shall never go offline,
         * which is why we use PRELUDE_CLIENT_EXIT_STATUS_FAILURE.
         */
        prelude_client_destroy(data, PRELUDE_CLIENT_EXIT_STATUS_FAILURE);

        /*
         * Free libprelude relevant data and synchronize asynchronous thread.
         */
        prelude_deinit();
}



static void parse_args(char *args, char **profile)
{
        int i, tokens, ret;
        char **args_table, *value, *key;
                
        args_table = mSplit(args, " \t", 0, &tokens, '\\');
        for ( i = 0; i < tokens; i++ ) {
                
                key = args_table[i];
                strtok(key, "=");
                
                value = strtok(NULL, "");
                if ( ! value )
                        FatalError("spo_alert_prelude: missing value for keyword '%s'.\n", key);
                
                ret = strcasecmp("profile", key);
                if ( ret == 0 ) {
                        if ( *profile )
                                free(*profile);
                        
                        *profile = strdup(value);
                        continue;
                }
                
                ret = strcasecmp("info", key);
                if ( ret == 0 ) {
                        info_priority = atoi(value);
                        continue;
                }

                ret = strcasecmp("low", key);
                if ( ret == 0 ) {
                        low_priority = atoi(value);
                        continue;
                }

                ret = strcasecmp("medium", key);
                if ( ret == 0 ) {
                        mid_priority = atoi(value);
                        continue;
                }

                FatalError("spo_alert_prelude: Invalid parameter found: '%s'.\n", key);
        }

        mSplitFree(&args_table, tokens);
}


void AlertPreludeSetupAfterSetuid(void)
{
        int ret;
        char *profile = NULL;
        prelude_client_t *client;
        prelude_client_flags_t flags;

        if ( ! initialized )
                return;
        
        parse_args(init_args, &profile);
        free(init_args);
       
        ret = prelude_thread_init(NULL);
        if ( ret < 0 )
            FatalError("%s: Unable to initialize the Prelude thread subsystem: %s.\n",
                       prelude_strsource(ret), prelude_strerror(ret));

        ret = prelude_init(NULL, NULL);
        if ( ret < 0 )
                FatalError("%s: Unable to initialize the Prelude library: %s.\n",
                           prelude_strsource(ret), prelude_strerror(ret));
        
        ret = prelude_client_new(&client, profile ? profile : DEFAULT_ANALYZER_NAME);
        if ( profile )
                free(profile);
        
        if ( ret < 0 )
                FatalError("%s: Unable to create a prelude client object: %s.\n",
                           prelude_strsource(ret), prelude_strerror(ret));

        
        flags = PRELUDE_CLIENT_FLAGS_ASYNC_SEND|PRELUDE_CLIENT_FLAGS_ASYNC_TIMER;
        
        ret = prelude_client_set_flags(client, prelude_client_get_flags(client) | flags);
        if ( ret < 0 )
                FatalError("%s: Unable to set asynchronous send and timer: %s.\n",
                           prelude_strsource(ret), prelude_strerror(ret));

        
        setup_analyzer(prelude_client_get_analyzer(client));
        
        ret = prelude_client_start(client);
        if ( ret < 0 ) {
                if ( prelude_client_is_setup_needed(ret) )
                        prelude_client_print_setup_error(client);

                FatalError("%s: Unable to initialize prelude client: %s.\n",
                           prelude_strsource(ret), prelude_strerror(ret));
        }
                
        AddFuncToOutputList(snort_alert_prelude, OUTPUT_TYPE__ALERT, client);
        AddFuncToCleanExitList(snort_alert_prelude_clean_exit, client);
        AddFuncToRestartList(snort_alert_prelude_clean_exit, client);
}


static void snort_alert_prelude_init(char *args)
{
        /*
         * Do nothing here. Wait until AlertPreludeSetupAfterSetuid is called.
         */
        if ( args )
                init_args = strdup((char *) args);

        initialized = TRUE;
}


void AlertPreludeSetup(void)
{
    RegisterOutputPlugin("alert_prelude", OUTPUT_TYPE_FLAG__ALERT, snort_alert_prelude_init);
}


#endif /* HAVE_LIBPRELUDE */
