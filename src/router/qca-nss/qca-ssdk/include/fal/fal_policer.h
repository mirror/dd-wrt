/*
 * Copyright (c) 2016-2018, 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @defgroup fal_policer FAL_POLICER
 * @{
 */
#ifndef _FAL_POLICER_H_
#define _FAL_POLICER_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
#include "fal/fal_type.h"

typedef enum
{
	FAL_POLICER_METER_RFC = 0, /* legacy feature, add it for ipq95xx */
	FAL_POLICER_METER_MEF10_3 /* mef10-3 feature, add it for ipq95xx */
} fal_policer_meter_type_t;

typedef struct
{
	a_bool_t		meter_en;	/* meter enable or disable */
	a_bool_t		couple_en;	/*  two buckets coupling enable or disable*/
	a_uint32_t	color_mode;	/* color aware or color blind */
	a_uint32_t	frame_type;	/* frame type, bit0:unicast;bit1: unkown unicast;bit2:multicast;bit3: unknown multicast; bit4:broadcast */
	a_uint32_t 	meter_mode;
	a_uint32_t	meter_unit; /* 0:byte based; 1:packet based*/
	a_uint32_t	cir;	/* committed information rate */
	a_uint32_t	cbs;	/* committed burst size */
	a_uint32_t	eir; /* excess information rate */
	a_uint32_t	ebs; /* excess burst size */
	a_uint32_t	cir_max; /* max committed information rate, add it for ipq95xx */
	a_uint32_t	eir_max; /* max excess information rate, add it for ipq95xx */
	a_uint32_t	next_ptr; /* next entry, add it for ipq95xx */
	a_bool_t	grp_end; /* last entry or not in current group, add it for ipq95xx */
	a_bool_t	grp_couple_en; /* group coupled not, add it for ipq95xx */
	a_uint32_t	vp_meter_index; /* vp port meter index, add it for ipq95xx*/
	fal_policer_meter_type_t meter_type; /*legacy or mef10-3,add it for ipq95xx */
} fal_policer_config_t;

typedef struct
{
	a_bool_t yellow_priority_en; /* yellow traffic internal priority change enable*/
	a_bool_t yellow_drop_priority_en; /* yellow traffic internal drop priority change enable*/
	a_bool_t yellow_pcp_en; /* yellow traffic pcp change enable*/
	a_bool_t yellow_dei_en; /* yellow traffic dei change enable*/
	a_uint32_t yellow_priority; /* yellow traffic internal priority value*/
	a_uint32_t yellow_drop_priority; /* yellow traffic internal drop priority value*/
	a_uint32_t yellow_pcp; /* yellow traffic pcp value*/
	a_uint32_t yellow_dei; /* yellow traffic dei value*/
	fal_fwd_cmd_t red_action; /* red traffic drop or forward*/
	a_bool_t red_priority_en; /* red traffic internal priority change enable*/
	a_bool_t red_drop_priority_en; /* red traffic internal drop priority change enable*/
	a_bool_t red_pcp_en; /* red traffic pcp change enable*/
	a_bool_t red_dei_en; /* red traffic dei change enable*/
	a_uint32_t red_priority; /* red traffic internal priority value*/
	a_uint32_t red_drop_priority; /* red traffic internal drop priority value*/
	a_uint32_t red_pcp; /* red traffic pcp value*/
	a_uint32_t red_dei;  /* red traffic dei value*/
	a_bool_t yellow_dscp_en; /* yellow traffic dscp change enable, add it for ipq95xx*/
	a_uint32_t yellow_dscp; /* yellow traffic dscp value, add it for ipq95xx*/
	a_bool_t red_dscp_en; /* red traffic dscp change enable, add it for ipq95xx*/
	a_uint32_t red_dscp; /* red traffic dscp value, add it for ipq95xx*/
	a_bool_t yellow_remap_en; /* yellow traffic remap enable, add it for ipq95xx*/
	a_bool_t red_remap_en; /* red traffic remap enable, add it for ipq95xx*/
}fal_policer_action_t;

typedef struct
{
    a_uint32_t green_packet_counter; /*green packet counter */
    a_uint64_t green_byte_counter; /*green byte counter */
    a_uint32_t yellow_packet_counter; /*yellow packet counter */
    a_uint64_t yellow_byte_counter; /*yellow byte counter */
    a_uint32_t red_packet_counter; /*red packet counter */
    a_uint64_t red_byte_counter; /*red byte counter */
} fal_policer_counter_t;

typedef struct
{
    a_uint32_t policer_drop_packet_counter; /*drop packet counter by policer*/
    a_uint64_t policer_drop_byte_counter; /*drop byte counter by policer */
    a_uint32_t policer_forward_packet_counter; /*forward packet counter by policer*/
    a_uint64_t policer_forward_byte_counter; /*forward byte counter by policer*/
    a_uint32_t policer_bypass_packet_counter; /*bypass packet counter by policer*/
    a_uint64_t policer_bypass_byte_counter; /*bypass byte counter by policer */
} fal_policer_global_counter_t;

typedef struct
{
	a_uint32_t head; /* linklist head, add it for ipq95xx */
	a_uint32_t tail; /* linklist tail, add it for ipq95xx */
} fal_policer_ctrl_t;

typedef struct
{
	a_uint32_t dscp; /*remap new dscp value, add it for ipq95xx */
	a_uint32_t pcp; /*remap new pcp value, add it for ipq95xx */
	a_uint32_t dei; /*remap new dei value, add it for ipq95xx */
} fal_policer_remap_t;

typedef enum {
	FAL_POLICER_METER_YELLOW = 0, /* meter yellow traffic, add it for ipq95xx */
	FAL_POLICER_METER_RED /* meter red traffic, add it for ipq95xx */
} fal_policer_meter_color_t;

typedef struct
{
	a_uint32_t internal_pri; /*internal prioirty, add it for ipq95xx */
	a_uint32_t internal_dp; /*internal drop priority, add it for ipq95xx */
	fal_policer_meter_color_t meter_color; /*meter color value, add it for ipq95xx */
} fal_policer_priority_t;

typedef enum {
	FAL_FRAME_DROPPED = 0,
} fal_policer_frame_type_t;

sw_error_t
fal_port_policer_entry_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_policer_config_t *policer, fal_policer_action_t *atcion);

sw_error_t
fal_port_policer_entry_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_policer_config_t *policer, fal_policer_action_t *action);

sw_error_t
fal_acl_policer_entry_set(a_uint32_t dev_id, a_uint32_t index,
		fal_policer_config_t *policer, fal_policer_action_t *action);

sw_error_t
fal_acl_policer_entry_get(a_uint32_t dev_id, a_uint32_t index,
		fal_policer_config_t *policer, fal_policer_action_t *action);

#ifndef IN_POLICER_MINI
sw_error_t
fal_port_policer_counter_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_policer_counter_t *counter);

sw_error_t
fal_acl_policer_counter_get(a_uint32_t dev_id, a_uint32_t index,
		fal_policer_counter_t *counter);

sw_error_t
fal_policer_global_counter_get(a_uint32_t dev_id,
		fal_policer_global_counter_t *counter);

sw_error_t
fal_policer_priority_remap_get(a_uint32_t dev_id, fal_policer_priority_t *priority,
	fal_policer_remap_t *remap);

sw_error_t
fal_policer_priority_remap_set(a_uint32_t dev_id, fal_policer_priority_t *priority,
	fal_policer_remap_t *remap);
#endif

sw_error_t
fal_port_policer_compensation_byte_get(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t *length);

sw_error_t
fal_port_policer_compensation_byte_set(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t length);

sw_error_t
fal_policer_timeslot_get(a_uint32_t dev_id, a_uint32_t *timeslot);

sw_error_t
fal_policer_timeslot_set(a_uint32_t dev_id, a_uint32_t timeslot);

sw_error_t
fal_policer_bypass_en_get(a_uint32_t dev_id, fal_policer_frame_type_t frame_type,
	a_bool_t *enable);

sw_error_t
fal_policer_bypass_en_set(a_uint32_t dev_id, fal_policer_frame_type_t frame_type,
	a_bool_t enable);

sw_error_t
fal_policer_ctrl_get(a_uint32_t dev_id, fal_policer_ctrl_t *ctrl);

sw_error_t
fal_policer_ctrl_set(a_uint32_t dev_id, fal_policer_ctrl_t *ctrl);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_POLICER_H_ */
/**
 * @}
 */
