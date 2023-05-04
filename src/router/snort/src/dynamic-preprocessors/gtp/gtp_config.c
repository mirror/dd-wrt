/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 * Provides convenience functions for parsing and querying configuration.
 *
 * 7/17/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <errno.h>
#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "gtp_config.h"
#include "spp_gtp.h"
#include "gtp_debug.h"

#define METHOD_NOT_FOUND (-1)
/*
 * Default GTP port
 */
#define GTP_C_PORT	    (2123)
#define GTP_C_PORT_V0	(3386)

/*
 * Keyword strings for parsing configuration options.
 */
#define GTP_PORTS_KEYWORD			     "ports"

#define GTP_CONFIG_SECTION_SEPERATORS     ",;"
#define GTP_CONFIG_VALUE_SEPERATORS       " "

/*
 * Message type defined
 */

static GTP_MsgType GTPv0_MsgTypes[] =
{
        {1, 1, "echo_request"},
        {2, 1, "echo_response"},
        {3, 1, "version_not_supported"},
        {4, 1, "node_alive_request"},
        {5, 1, "node_alive_response"},
        {6, 1, "redirection_request"},
        {7, 1, "redirection_response"},

        {16, 1,"create_pdp_context_request"},
        {17, 1,"create_pdp_context_response"},
        {18, 1,"update_pdp_context_request"},
        {19, 1,"update_pdp_context_response"},
        {20, 1,"delete_pdp_context_request"},
        {21, 1,"delete_pdp_context_response"},
        {22, 1,"create_aa_pdp_context_request"},
        {23, 1,"create_aa_pdp_context_response"},
        {24, 1,"delete_aa_pdp_context_request"},
        {25, 1,"delete_aa_pdp_context_response"},
        {26, 1,"error_indication"},
        {27, 1,"pdu_notification_request"},
        {28, 1,"pdu_notification_response"},
        {29, 1,"pdu_notification_reject_request"},
        {30, 1,"pdu_notification_reject_response"},

        {32, 1,"send_routing_info_request"},
        {33, 1,"send_routing_info_response"},
        {34, 1,"failure_report_request"},
        {35, 1,"failure_report_response"},
        {36, 1,"note_ms_present_request"},
        {37, 1,"note_ms_present_response"},

        {48, 1,"identification_request"},
        {49, 1,"identification_response"},
        {50, 1,"sgsn_context_request"},
        {51, 1,"sgsn_context_response"},
        {52, 1,"sgsn_context_ack"},

        {240, 1,"data_record_transfer_request"},
        {241, 1,"data_record_transfer_response"},

        {255, 1,"pdu"},
        {0, 0, NULL}
};

static GTP_MsgType GTPv1_MsgTypes[] =
{
        {1, 1, "echo_request"},
        {2, 1, "echo_response"},
        {3, 1, "version_not_supported"},
        {4, 1, "node_alive_request"},
        {5, 1, "node_alive_response"},
        {6, 1, "redirection_request"},
        {7, 1, "redirection_response"},

        {16, 1,"create_pdp_context_request"},
        {17, 1,"create_pdp_context_response"},
        {18, 1,"update_pdp_context_request"},
        {19, 1,"update_pdp_context_response"},
        {20, 1,"delete_pdp_context_request"},
        {21, 1,"delete_pdp_context_response"},
        {22, 1,"init_pdp_context_activation_request"},
        {23, 1,"init_pdp_context_activation_response"},

        {26, 1,"error_indication"},
        {27, 1,"pdu_notification_request"},
        {28, 1,"pdu_notification_response"},
        {29, 1,"pdu_notification_reject_request"},
        {30, 1,"pdu_notification_reject_response"},
        {31, 1,"supported_ext_header_notification"},
        {32, 1,"send_routing_info_request"},
        {33, 1,"send_routing_info_response"},
        {34, 1,"failure_report_request"},
        {35, 1,"failure_report_response"},
        {36, 1,"note_ms_present_request"},
        {37, 1,"note_ms_present_response"},

        {48, 1,"identification_request"},
        {49, 1,"identification_response"},
        {50, 1,"sgsn_context_request"},
        {51, 1,"sgsn_context_response"},
        {52, 1,"sgsn_context_ack"},
        {53, 1,"forward_relocation_request"},
        {54, 1,"forward_relocation_response"},
        {55, 1,"forward_relocation_complete"},
        {56, 1,"relocation_cancel_request"},
        {57, 1,"relocation_cancel_response"},
        {58, 1,"forward_srns_contex"},
        {59, 1,"forward_relocation_complete_ack"},
        {60, 1,"forward_srns_contex_ack"},

        {70, 1,"ran_info_relay"},

        {96, 1,"mbms_notification_request"},
        {97, 1,"mbms_notification_response"},
        {98, 1,"mbms_notification_reject_request"},
        {99, 1,"mbms_notification_reject_response"},
        {100,1,"create_mbms_context_request"},
        {101,1,"create_mbms_context_response"},
        {102,1,"update_mbms_context_request"},
        {103,1,"update_mbms_context_response"},
        {104,1,"delete_mbms_context_request"},
        {105,1,"delete_mbms_context_response"},

        {112,1,"mbms_register_request"},
        {113,1,"mbms_register_response"},
        {114,1,"mbms_deregister_request"},
        {115,1,"mbms_deregister_response"},
        {116,1,"mbms_session_start_request"},
        {117,1,"mbms_session_start_response"},
        {118,1,"mbms_session_stop_request"},
        {119,1,"mbms_session_stop_response"},
        {120,1,"mbms_session_update_request"},
        {121,1,"mbms_session_update_response"},

        {128, 1,"ms_info_change_request"},
        {129, 1,"ms_info_change_response"},

        {240, 1,"data_record_transfer_request"},
        {241, 1,"data_record_transfer_response"},

        {254, 1,"end_marker"},
        {255, 1,"pdu"},
        {0, 0, NULL}
};

static GTP_MsgType GTPv2_MsgTypes[] =
{
        {1, 1, "echo_request"},
        {2, 1, "echo_response"},
        {3, 1, "version_not_supported"},

        {32, 1,"create_session_request"},
        {33, 1,"create_session_response"},
        {34, 1,"modify_bearer_request"},
        {35, 1,"modify_bearer_response"},
        {36, 1,"delete_session_request"},
        {37, 1,"delete_session_response"},
        {38, 1,"change_notification_request"},
        {39, 1,"change_notification_response"},

        {64, 1,"modify_bearer_command"},
        {65, 1,"modify_bearer_failure_indication"},
        {66, 1,"delete_bearer_command"},
        {67, 1,"delete_bearer_failure_indication"},
        {68, 1,"bearer_resource_command"},
        {69, 1,"bearer_resource_failure_indication"},
        {70, 1,"downlink_failure_indication"},
        {71, 1,"trace_session_activation"},
        {72, 1,"trace_session_deactivation"},
        {73, 1,"stop_paging_indication"},

        {95, 1,"create_bearer_request"},
        {96, 1,"create_bearer_response"},
        {97, 1,"update_bearer_request"},
        {98, 1,"update_bearer_response"},
        {99, 1,"delete_bearer_request"},
        {100,1,"delete_bearer_response"},
        {101,1,"delete_pdn_request"},
        {102,1,"delete_pdn_response"},

        {128, 1,"identification_request"},
        {129, 1,"identification_response"},
        {130, 1,"sgsn_context_request"},
        {131, 1,"sgsn_context_response"},
        {132, 1,"sgsn_context_ack"},
        {133, 1,"forward_relocation_request"},
        {134, 1,"forward_relocation_response"},
        {135, 1,"forward_relocation_complete"},
        {136, 1,"forward_relocation_complete_ack"},
        {137, 1,"forward_access"},
        {138, 1,"forward_access_ack"},
        {139, 1,"relocation_cancel_request"},
        {140, 1,"relocation_cancel_response"},
        {141, 1,"configuration_transfer_tunnel"},

        {149, 1,"detach"},
        {150, 1,"detach_ack"},
        {151, 1,"cs_paging"},
        {152, 1,"ran_info_relay"},
        {153, 1,"alert_mme"},
        {154, 1,"alert_mme_ack"},
        {155, 1,"ue_activity"},
        {156, 1,"ue_activity_ack"},

        {160,1,"create_forward_tunnel_request"},
        {161,1,"create_forward_tunnel_response"},
        {162, 1,"suspend"},
        {163, 1,"suspend_ack"},
        {164, 1,"resume"},
        {165, 1,"resume_ack"},
        {166,1,"create_indirect_forward_tunnel_request"},
        {167,1,"create_indirect_forward_tunnel_response"},
        {168,1,"delete_indirect_forward_tunnel_request"},
        {169,1,"delete_indirect_forward_tunnel_response"},
        {170,1,"release_access_bearer_request"},
        {171,1,"release_access_bearer_response"},

        {176,1,"downlink_data"},
        {177,1,"downlink_data_ack"},

        {179,1,"pgw_restart"},
        {180,1,"pgw_restart_ack"},

        {200,1,"update_pdn_request"},
        {201,1,"update_pdn_response"},

        {211,1,"modify_access_bearer_request"},
        {212,1,"modify_access_bearer_response"},

        {231,1,"mbms_session_start_request"},
        {232,1,"mbms_session_start_response"},
        {233,1,"mbms_session_update_request"},
        {234,1,"mbms_session_update_response"},
        {235,1,"mbms_session_stop_request"},
        {236,1,"mbms_session_stop_response"},

        {0, 0, NULL}
};

/*
 * Information elements defined
 */

static GTP_InfoElement GTPv0_InfoElements[] =
{
        {1, 1, "cause", 2},
        {2, 1, "imsi", 9},
        {3, 1, "rai", 7},
        {4, 1, "tlli", 5},
        {5, 1, "p_tmsi", 5},
        {6, 1, "qos", 4},

        {8, 1, "recording_required", 2},
        {9, 1, "authentication", 29},

        {11, 1, "map_cause", 2},
        {12, 1, "p_tmsi_sig", 4},
        {13, 1, "ms_validated", 2},
        {14, 1, "recovery", 2},
        {15, 1, "selection_mode", 2},
        {16, 1, "flow_label_data_1", 3},
        {17, 1, "flow_label_signalling", 3},
        {18, 1, "flow_label_data_2", 4},
        {19, 1, "ms_unreachable", 2},

        {127, 1, "charge_id", 5},
        {128, 1, "end_user_address", 0},
        {129, 1, "mm_context", 0},
        {130, 1, "pdp_context", 0},
        {131, 1, "apn", 0},
        {132, 1, "protocol_config", 0},
        {133, 1, "gsn", 0},
        {134, 1, "msisdn", 0},

        {251, 1, "charging_gateway_addr", 0},

        {255, 1, "private_extension", 0},

        {0, 0, NULL, 0},
};

static GTP_InfoElement GTPv1_InfoElements[] =
{
        {1, 1, "cause", 2},
        {2, 1, "imsi", 9},
        {3, 1, "rai", 7},
        {4, 1, "tlli", 5},
        {5, 1, "p_tmsi", 5},

        {8, 1, "recording_required", 2},
        {9, 1, "authentication", 29},

        {11, 1, "map_cause", 2},
        {12, 1, "p_tmsi_sig", 4},
        {13, 1, "ms_validated", 2},
        {14, 1, "recovery", 2},
        {15, 1, "selection_mode", 2},
        {16, 1, "teid_1", 5},
        {17, 1, "teid_control", 5},
        {18, 1, "teid_2", 6},
        {19, 1, "teardown_ind", 2},
        {20, 1, "nsapi", 2},
        {21, 1, "ranap", 2},
        {22, 1, "rab_context", 10},
        {23, 1, "radio_priority_sms", 2},
        {24, 1, "radio_priority", 2},
        {25, 1, "packet_flow_id", 3},
        {26, 1, "charging_char", 3},
        {27, 1, "trace_ref", 3},
        {28, 1, "trace_type", 3},
        {29, 1, "ms_unreachable", 2},

        {127, 1, "charge_id", 5},
        {128, 1, "end_user_address", 0},
        {129, 1, "mm_context", 0},
        {130, 1, "pdp_context", 0},
        {131, 1, "apn", 0},
        {132, 1, "protocol_config", 0},
        {133, 1, "gsn", 0},
        {134, 1, "msisdn", 0},
        {135, 1, "qos", 0},
        {136, 1, "authentication_qu", 0},
        {137, 1, "tft", 0},
        {138, 1, "target_id", 0},
        {139, 1, "utran_trans", 0},
        {140, 1, "rab_setup", 0},
        {141, 1, "ext_header", 0},
        {142, 1, "trigger_id", 0},
        {143, 1, "omc_id", 0},
        {144, 1, "ran_trans", 0},
        {145, 1, "pdp_context_pri", 0},
        {146, 1, "addi_rab_setup", 0},
        {147, 1, "sgsn_number", 0},
        {148, 1, "common_flag", 0},
        {149, 1, "apn_restriction", 0},
        {150, 1, "radio_priority_lcs", 4},
        {151, 1, "rat_type", 0},
        {152, 1, "user_loc_info", 0},
        {153, 1, "ms_time_zone", 0},
        {154, 1, "imei_sv", 0},
        {155, 1, "camel", 0},
        {156, 1, "mbms_ue_context", 0},
        {157, 1, "tmp_mobile_group_id", 0},
        {158, 1, "rim_routing_addr", 0},
        {159, 1, "mbms_config", 0},
        {160, 1, "mbms_service_area", 0},
        {161, 1, "src_rnc_pdcp", 0},
        {162, 1, "addi_trace_info", 0},
        {163, 1, "hop_counter", 0},
        {164, 1, "plmn_id", 0},
        {165, 1, "mbms_session_id", 0},
        {166, 1, "mbms_2g3g_indicator", 0},
        {167, 1, "enhanced_nsapi", 0},
        {168, 1, "mbms_session_duration", 0},
        {169, 1, "addi_mbms_trace_info", 0},
        {170, 1, "mbms_session_repetition_num", 0},
        {171, 1, "mbms_time_to_data", 0},

        {173, 1, "bss", 0},
        {174, 1, "cell_id", 0},
        {175, 1, "pdu_num", 0},
        {177, 1, "mbms_bearer_capab", 0},
        {178, 1, "rim_routing_disc", 0},
        {179, 1, "list_pfc", 0},
        {180, 1, "ps_xid", 0},
        {181, 1, "ms_info_change_report", 4},
        {182, 1, "direct_tunnel_flags", 0},
        {183, 1, "correlation_id", 0},
        {184, 1, "bearer_control_mode", 0},
        {185, 1, "mbms_flow_id", 0},
        {186, 1, "mbms_ip_multicast", 0},
        {187, 1, "mbms_distribution_ack", 4},
        {188, 1, "reliable_inter_rat_handover", 0},
        {189, 1, "rfsp_index", 0},
        {190, 1, "fqdn", 0},
        {191, 1, "evolved_allocation1", 0},
        {192, 1, "evolved_allocation2", 0},
        {193, 1, "extended_flags", 0},
        {194, 1, "uci", 0},
        {195, 1, "csg_info", 0},
        {196, 1, "csg_id", 0},
        {197, 1, "cmi", 4},
        {198, 1, "apn_ambr", 0},
        {199, 1, "ue_network", 0},
        {200, 1, "ue_ambr", 0},
        {201, 1, "apn_ambr_nsapi", 0},
        {202, 1, "ggsn_backoff_timer", 0},
        {203, 1, "signalling_priority_indication", 0},
        {204, 1, "signalling_priority_indication_nsapi", 0},
        {205, 1, "high_bitrate", 4},
        {206, 1, "max_mbr", 0},

        {251, 1, "charging_gateway_addr", 0},

        {255, 1, "private_extension", 0},

        {0, 0, NULL, 0},
};

static GTP_InfoElement GTPv2_InfoElements[] =
{
        {1, 1, "imsi", 0},
        {2, 1, "cause", 0},
        {3, 1, "recovery", 0},

        {71, 1, "apn", 0},
        {72, 1, "ambr", 0},
        {73, 1, "ebi", 0},
        {74, 1, "ip_addr", 0},
        {75, 1, "mei", 0},
        {76, 1, "msisdn", 0},
        {77, 1, "indication", 0},
        {78, 1, "pco", 0},
        {79, 1, "paa", 0},
        {80, 1, "bearer_qos", 0},
        {81, 1, "flow_qos", 0},
        {82, 1, "rat_type", 0},
        {83, 1, "serving_network", 0},
        {84, 1, "bearer_tft", 0},
        {85, 1, "tad", 0},
        {86, 1, "uli", 0},
        {87, 1, "f_teid", 0},
        {88, 1, "tmsi", 0},
        {89, 1, "cn_id", 0},
        {90, 1, "s103pdf", 0},
        {91, 1, "s1udf", 0},
        {92, 1, "delay_value", 0},
        {93, 1, "bearer_context", 0},
        {94, 1, "charging_id", 0},
        {95, 1, "charging_char", 0},
        {96, 1, "trace_info", 0},
        {97, 1, "bearer_flag", 0},

        {99,  1, "pdn_type", 0},
        {100, 1, "pti", 0},
        {101, 1, "drx_parameter", 0},

        {103, 1, "gsm_key_tri", 0},
        {104, 1, "umts_key_cipher_quin", 0},
        {105, 1, "gsm_key_cipher_quin", 0},
        {106, 1, "umts_key_quin", 0},
        {107, 1, "eps_quad", 0},
        {108, 1, "umts_key_quad_quin", 0},
        {109, 1, "pdn_connection", 0},
        {110, 1, "pdn_number", 0},
        {111, 1, "p_tmsi", 0},
        {112, 1, "p_tmsi_sig", 0},
        {113, 1, "hop_counter", 0},
        {114, 1, "ue_time_zone", 0},
        {115, 1, "trace_ref", 0},
        {116, 1, "complete_request_msg", 0},
        {117, 1, "guti", 0},
        {118, 1, "f_container", 0},
        {119, 1, "f_cause", 0},
        {120, 1, "plmn_id", 0},
        {121, 1, "target_id", 0},

        {123, 1, "packet_flow_id", 0},
        {124, 1, "rab_contex", 0},
        {125, 1, "src_rnc_pdcp", 0},
        {126, 1, "udp_src_port", 0},
        {127, 1, "apn_restriction", 0},
        {128, 1, "selection_mode", 0},
        {129, 1, "src_id", 0},

        {131, 1, "change_report_action", 0},
        {132, 1, "fq_csid", 0},
        {133, 1, "channel", 0},
        {134, 1, "emlpp_pri", 0},
        {135, 1, "node_type", 0},
        {136, 1, "fqdn", 0},
        {137, 1, "ti", 0},
        {138, 1, "mbms_session_duration", 0},
        {139, 1, "mbms_service_area", 0},
        {140, 1, "mbms_session_id", 0},
        {141, 1, "mbms_flow_id", 0},
        {142, 1, "mbms_ip_multicast", 0},
        {143, 1, "mbms_distribution_ack", 0},
        {144, 1, "rfsp_index", 0},
        {145, 1, "uci", 0},
        {146, 1, "csg_info", 0},
        {147, 1, "csg_id", 0},
        {148, 1, "cmi", 0},
        {149, 1, "service_indicator", 0},
        {150, 1, "detach_type", 0},
        {151, 1, "ldn", 0},
        {152, 1, "node_feature", 0},
        {153, 1, "mbms_time_to_transfer", 0},
        {154, 1, "throttling", 0},
        {155, 1, "arp", 0},
        {156, 1, "epc_timer", 0},
        {157, 1, "signalling_priority_indication", 0},
        {158, 1, "tmgi", 0},
        {159, 1, "mm_srvcc", 0},
        {160, 1, "flags_srvcc", 0},
        {161, 1, "mmbr", 0},

        {255, 1, "private_extension", 0},

        {0, 0, NULL, 0},
};


/*
 * Function prototype(s)
 */
static void InitGTPInfoElementTable(GTPConfig *);
static void DisplayGTPConfig(GTPConfig *);
static void GTP_ParsePortList(char **, uint8_t *);

/* Update the information elements table for one GTP version.
 *
 * PARAMETERS:
 *
 * GTPConfig *config: GTP preprocessor configuration.
 * GTP_InfoElement *: Information elements
 * uint8_t: version number for information elements
 *
 * RETURNS: Nothing.
 */

static void UpdateGTPInfoElementTable(GTPConfig *config, GTP_InfoElement *InfoElements, uint8_t version)
{
    int i = 0;

    while(NULL != InfoElements[i].name)
    {
        config->infoElementTable[version][InfoElements[i].type] =  &InfoElements[i];
        i++;
    }
}

/* Update the information elements table for the GTP preprocessor.
 *
 * PARAMETERS:
 *
 * GTPConfig *config: GTP preprocessor configuration.
 *
 * RETURNS: Nothing.
 */

static void InitGTPInfoElementTable(GTPConfig *config)
{

    GTP_InfoElement *InfoElements;


    InfoElements =  GTPv0_InfoElements;
    UpdateGTPInfoElementTable(config,InfoElements, 0);

    InfoElements =  GTPv1_InfoElements;
    UpdateGTPInfoElementTable(config,InfoElements, 1);

    InfoElements =  GTPv2_InfoElements;
    UpdateGTPInfoElementTable(config,InfoElements, 2);

}

/* Update the message types table for one GTP version.
 *
 * PARAMETERS:
 *
 * GTPConfig *config: GTP preprocessor configuration.
 * GTP_MsgType *: message types
 * uint8_t: version number for message types
 *
 * RETURNS: Nothing.
 */

static void UpdateGTPMsgTypeTable(GTPConfig *config, GTP_MsgType *MsgTypes, uint8_t version)
{
    int i = 0;

    while(NULL != MsgTypes[i].name)
    {
        config->msgTypeTable[version][MsgTypes[i].type] =  &MsgTypes[i];
        gtp_stats.msgTypeTable[version][MsgTypes[i].type] =  &MsgTypes[i];
        i++;
    }
}

/* Update the message types table for the GTP preprocessor.
 *
 * PARAMETERS:
 *
 * GTPConfig *config: GTP preprocessor configuration.
 *
 * RETURNS: Nothing.
 */

static void InitGTPMsgTypeTable(GTPConfig *config)
{

    GTP_MsgType *MsgTypes;


    MsgTypes =  GTPv0_MsgTypes;
    UpdateGTPMsgTypeTable(config,MsgTypes, 0);

    MsgTypes =  GTPv1_MsgTypes;
    UpdateGTPMsgTypeTable(config,MsgTypes, 1);

    MsgTypes =  GTPv2_MsgTypes;
    UpdateGTPMsgTypeTable(config,MsgTypes, 2);

}

#ifdef DEBUG_MSGS
/* Display the message types for the GTP preprocessor.
 *
 * PARAMETERS:
 *
 * GTPConfig *config: GTP preprocessor configuration.
 *
 * RETURNS: Nothing.
 */
static void DisplayMsgTypes(GTPConfig *config)
{
    int i, j;

    _dpd.logMsg("    Supported message types:\n");

    for(i = 0; i < MAX_GTP_TYPE_CODE + 1; i++)
    {
        _dpd.logMsg("\t%3d ", i);
        for (j = 0; j < MAX_GTP_VERSION_CODE + 1; j++)
        {
            if (config->msgTypeTable[j][i])
            {
                _dpd.logMsg("%40s ", config->msgTypeTable[j][i]->name);
            }
            else
                _dpd.logMsg("%40s ", "N/A");

        }
        _dpd.logMsg("\n");
    }
}
/* Display the information element for the GTP preprocessor.
 *
 * PARAMETERS:
 *
 * GTPConfig *config: GTP preprocessor configuration.
 *
 * RETURNS: Nothing.
 */
static void DisplayInfoElements(GTPConfig *config)
{
    int i, j;

    _dpd.logMsg("    Supported information elements:\n");

    for(i = 0; i < MAX_GTP_IE_CODE + 1; i++)
    {
        _dpd.logMsg("\t%3d ", i);
        for (j = 0; j < MAX_GTP_VERSION_CODE + 1 ; j++)
        {
            if (config->infoElementTable[j][i])
                _dpd.logMsg(" %40s ", config->infoElementTable[j][i]->name);
            else
                _dpd.logMsg(" %40s ", "N/A");

        }
        _dpd.logMsg("\n");
    }
}
#endif

/* Display the configuration for the GTP preprocessor.
 *
 * PARAMETERS:
 *
 * GTPConfig *config: GTP preprocessor configuration.
 *
 * RETURNS: Nothing.
 */
static void DisplayGTPConfig(GTPConfig *config)
{
    int index;
    int newline;

    if (config == NULL)
        return;

    _dpd.logMsg("GTP config: \n");

    /* Traverse list, printing ports, 5 per line */
    newline = 1;
    _dpd.logMsg("    Ports:\n");
    for(index = 0; index < MAXPORTS; index++)
    {
        if( config->ports[ PORT_INDEX(index) ] & CONV_PORT(index) )
        {
            _dpd.logMsg("\t%d", index);
            if ( !((newline++)% 5) )
                _dpd.logMsg("\n");
        }
    }
    _dpd.logMsg("\n");
    DEBUG_WRAP(DisplayMsgTypes(config));
    DEBUG_WRAP(DisplayInfoElements(config));

}


/********************************************************************
 * Function: GTP_ParsePortList()
 *
 * Parses a port list and adds bits associated with the ports
 * parsed to a bit array.
 *
 * Arguments:
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the IP list.
 *  uint8_t *
 *      Pointer to the port array mask to set bits for the ports
 *      parsed.
 *
 * Returns:
 *  GTP_Ret
 *      GTP_SUCCESS if we were able to successfully parse the
 *          port list.
 *      GTP_FAILURE if an error occured in parsing the port list.
 *
 ********************************************************************/
static void GTP_ParsePortList(char **ptr, uint8_t *port_array)
{
    long int port = -1;
    char* cur_tokenp = *ptr;
    /* If the user specified ports, remove GTP_C_PORT for now since
     * it now needs to be set explicitly. */
    port_array[ PORT_INDEX( GTP_C_PORT ) ] = 0;
    port_array[ PORT_INDEX( GTP_C_PORT_V0 ) ] = 0;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Port configurations: %s\n",*ptr ););

    /* Eat the open brace. */
    cur_tokenp = strtok( NULL, GTP_CONFIG_VALUE_SEPERATORS);

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Port token: %s\n",cur_tokenp ););

    /* Check the space after '{'*/
    if (( !cur_tokenp ) || ( 0 != strncmp (cur_tokenp,  "{", 2 )))
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s, make sure space before and after '{'.\n",
                *(_dpd.config_file), *(_dpd.config_line), GTP_PORTS_KEYWORD);
    }

    cur_tokenp = strtok( NULL, GTP_CONFIG_VALUE_SEPERATORS);
    while (( cur_tokenp ) && (  0 != strncmp (cur_tokenp,  "}", 2 )))
    {
        char *endStr = NULL;

        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Port token: %s\n",cur_tokenp ););

        if ( !cur_tokenp )
        {
            DynamicPreprocessorFatalMessage(" %s(%d) => No option to '%s'.\n",
                    *(_dpd.config_file), *(_dpd.config_line), GTP_PORTS_KEYWORD);
        }

        port = _dpd.SnortStrtol( cur_tokenp, &endStr, 10);

        if (*endStr)
        {
            DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                    "Please specify an integer between %d and %d.\n",
                    *(_dpd.config_file), *(_dpd.config_line),
                    GTP_PORTS_KEYWORD, 1, MAXPORTS-1);
        }

        if ((port < 0 || port > MAXPORTS-1) || (errno == ERANGE))
        {
            DynamicPreprocessorFatalMessage(" %s(%d) => Value specified for %s is out of "
                    "bounds.  Please specify an integer between %d and %d.\n",
                    *(_dpd.config_file), *(_dpd.config_line),
                    GTP_PORTS_KEYWORD, 1, MAXPORTS-1);
        }
        port_array[ PORT_INDEX( port ) ] |= CONV_PORT(port);

        cur_tokenp = strtok( NULL, GTP_CONFIG_VALUE_SEPERATORS);
    }
    if ( NULL == cur_tokenp )
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s, missing '}'.\n",
                *(_dpd.config_file), *(_dpd.config_line), GTP_PORTS_KEYWORD);
    }
    if ( -1 == port)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => No ports specified.\n",
                        *(_dpd.config_file), *(_dpd.config_line), GTP_PORTS_KEYWORD);
    }
    *ptr = cur_tokenp;
}

/* Parses and processes the configuration arguments
 * supplied in the GTP preprocessor rule.
 *
 * PARAMETERS:
 *
 * GTPConfig *config: GTP preprocessor configuration.
 * argp:              Pointer to string containing the config arguments.
 *
 * RETURNS:     Nothing.
 */
void ParseGTPArgs(GTPConfig *config, u_char* argp)
{
    char* cur_sectionp = NULL;
    char* next_sectionp = NULL;
    char* argcpyp = NULL;

    if (NULL == config)
        return;

    /* Set up default port to listen on */
    config->ports[ PORT_INDEX( GTP_C_PORT ) ] |= CONV_PORT(GTP_C_PORT);
    config->ports[ PORT_INDEX( GTP_C_PORT_V0 ) ] |= CONV_PORT(GTP_C_PORT_V0);

    InitGTPInfoElementTable(config);
    InitGTPMsgTypeTable(config);

    /* Sanity check(s) */
    if (NULL == argp)
    {
        DisplayGTPConfig(config);
        return;
    }

    argcpyp = strdup( (char*) argp );

    if ( !argcpyp )
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory to parse GTP options.\n");
        return;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "GTP configurations: %s\n",argcpyp ););

    cur_sectionp = strtok_r( argcpyp, GTP_CONFIG_SECTION_SEPERATORS, &next_sectionp);
    DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Arguments token: %s\n",cur_sectionp ););

    while ( cur_sectionp )
    {

        char* cur_config;
        char* cur_tokenp = 	strtok( cur_sectionp, GTP_CONFIG_VALUE_SEPERATORS);

        if (!cur_tokenp)
        {
            cur_sectionp = strtok_r( next_sectionp, GTP_CONFIG_SECTION_SEPERATORS, &next_sectionp);
            continue;
        }

        cur_config = cur_tokenp;

        if ( !strcmp( cur_tokenp, GTP_PORTS_KEYWORD ))
        {
            GTP_ParsePortList(&cur_tokenp, config->ports);

        }
        else
        {
            DynamicPreprocessorFatalMessage(" %s(%d) => Invalid argument: %s\n",
                    *(_dpd.config_file), *(_dpd.config_line), cur_tokenp);
            return;
        }
        /*Check whether too many parameters*/
        if (NULL != strtok( NULL, GTP_CONFIG_VALUE_SEPERATORS))
        {
            DynamicPreprocessorFatalMessage("%s(%d) => To many arguments: %s\n",
                    *(_dpd.config_file), *(_dpd.config_line), cur_config);

        }
        cur_sectionp = strtok_r( next_sectionp, GTP_CONFIG_SECTION_SEPERATORS, &next_sectionp);
        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "Arguments token: %s\n",cur_sectionp ););
    }

    DisplayGTPConfig(config);
    free(argcpyp);
}

/* Search the message type information
 *
 * PARAMETERS:
 *
 * uint8_t: version number for the message type
 * char* the message type name
 *
 * RETURNS:
 *
 * GTP_MsgType*: the message type, NULL if not found
 */

GTP_MsgType*  GetMsgTypeByName(uint8_t version, char *name)
{
    int i = 0;
    GTP_MsgType *MsgTypes;

    switch (version)
    {
    case 0:
        MsgTypes =  GTPv0_MsgTypes;
        break;
    case 1:
        MsgTypes =  GTPv1_MsgTypes;
        break;
    case 2:
        MsgTypes =  GTPv2_MsgTypes;
        break;
    default:
        return NULL;
    }

    while(NULL != MsgTypes[i].name)
    {
        if ( MsgTypes[i].isKeyword
                &&(strlen(MsgTypes[i].name) == strlen(name))
                && (0 == strncmp(MsgTypes[i].name, name, strlen(name))))
            return (&(MsgTypes[i]));
        i++;
    }

    return NULL;
}


/* Search the information element information
 *
 * PARAMETERS:
 *
 * uint8_t: version number for information elements
 * char* the information element name
 *
 * RETURNS:
 *
 * GTP_InfoElement*: the information element, NULL if not found
 */
GTP_InfoElement*  GetInfoElementByName(uint8_t version, char *name)
{
    int i = 0;
    GTP_InfoElement *InfoElements;

    switch (version)
    {
    case 0:
        InfoElements =  GTPv0_InfoElements;
        break;
    case 1:
        InfoElements =  GTPv1_InfoElements;
        break;
    case 2:
        InfoElements =  GTPv2_InfoElements;
        break;
    default:
        return NULL;
    }

    while(NULL != InfoElements[i].name)
    {
        if (InfoElements[i].isKeyword
                && (strlen(InfoElements[i].name) == strlen(name))
                && (0 == strncmp(InfoElements[i].name, name, strlen(name))))
            return (&InfoElements[i]);
        i++;
    }

    return NULL;
}
