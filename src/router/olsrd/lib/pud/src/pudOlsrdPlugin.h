/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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
 */

#ifndef _PUD_OLSRD_PLUGIN_H_
#define _PUD_OLSRD_PLUGIN_H_

/* Plugin includes */
#include "configuration.h"

/* OLSRD includes */
#include "olsrd_plugin.h"

/* System includes */
#include <stddef.h>

/** The interface version supported by the plugin */
#define PUD_PLUGIN_INTERFACE_VERSION	5

/**
 The plugin parameter configuration, containing the parameter names, pointers
 to their setters, and an optional data pointer that is given to the setter
 when it is called.
 */
static const struct olsrd_plugin_parameters plugin_parameters[] = {
	/* ID */
	{	.name = PUD_NODE_ID_NAME, .set_plugin_parameter = &setNodeId, .data = NULL},

	/* OLSR */
	{	.name = PUD_OLSR_TTL_NAME, .set_plugin_parameter = &setOlsrTtl, .data = NULL},
	{	.name = PUD_UPDATE_INTERVAL_STATIONARY_NAME, .set_plugin_parameter = &setUpdateIntervalStationary, .data = NULL},
	{	.name = PUD_UPDATE_INTERVAL_MOVING_NAME, .set_plugin_parameter = &setUpdateIntervalMoving, .data = NULL},

	/* gpsd */
	{	.name = PUD_GPSD_USE_NAME, .set_plugin_parameter = &setGpsdUse, .data = NULL},
	{	.name = PUD_GPSD_NAME, .set_plugin_parameter = &setGpsd, .data = NULL},

	/* position input file */
	{	.name = PUD_POSFILE_NAME, .set_plugin_parameter = &setPositionFile, .data = NULL},
	{	.name = PUD_POSFILEPERIOD_NAME, .set_plugin_parameter = &setPositionFilePeriod, .data = NULL},

	/* TX */
	{	.name = PUD_TX_NON_OLSR_IF_NAME, .set_plugin_parameter = &addTxNonOlsrInterface, .data = NULL},
	{	.name = PUD_TX_MC_ADDR_NAME, .set_plugin_parameter = &setTxMcAddr, .data = NULL},
	{	.name = PUD_TX_MC_PORT_NAME, .set_plugin_parameter = &setTxMcPort, .data = NULL},
	{	.name = PUD_TX_TTL_NAME, .set_plugin_parameter = &setTxTtl, .data = NULL},
	{	.name = PUD_TX_NMEAMESSAGEPREFIX_NAME, .set_plugin_parameter = &setTxNmeaMessagePrefix, .data = NULL},

	/* position output file */
	{	.name = PUD_POSOUTFILE_NAME, .set_plugin_parameter = &setPositionOutputFile, .data = NULL},

	/* UPLINK / DOWNLINK */
	{	.name = PUD_UPLINK_ADDR_NAME, .set_plugin_parameter = &setUplinkAddr, .data = NULL},
	{	.name = PUD_UPLINK_PORT_NAME, .set_plugin_parameter = &setUplinkPort, .data = NULL},
	{	.name = PUD_DOWNLINK_PORT_NAME, .set_plugin_parameter = &setDownlinkPort, .data = NULL},
	{	.name = PUD_UPLINK_UPDATE_INTERVAL_STATIONARY_NAME, .set_plugin_parameter = &setUplinkUpdateIntervalStationary, .data = NULL},
	{	.name = PUD_UPLINK_UPDATE_INTERVAL_MOVING_NAME, .set_plugin_parameter = &setUplinkUpdateIntervalMoving, .data = NULL},

	/* MOVEMENT DETECTION */
	{	.name = PUD_AVERAGE_DEPTH_NAME, .set_plugin_parameter = &setAverageDepth, .data = NULL},
	{	.name = PUD_GATEWAY_DETERMINATION_INTERVAL_NAME, .set_plugin_parameter = &setGatewayDeterminationInterval, .data = NULL},
	{	.name = PUD_MOVING_SPEED_THRESHOLD_NAME, .set_plugin_parameter = &setMovingSpeedThreshold, .data = NULL},
	{	.name = PUD_MOVING_DISTANCE_THRESHOLD_NAME, .set_plugin_parameter = &setMovingDistanceThreshold, .data = NULL},
	{	.name = PUD_DOP_MULTIPLIER_NAME, .set_plugin_parameter = &setDopMultiplier, .data = NULL},
	{	.name = PUD_HYSTERESIS_COUNT_2STAT_NAME, .set_plugin_parameter = &setHysteresisCountToStationary, .data = NULL},
	{	.name = PUD_HYSTERESIS_COUNT_2MOV_NAME, .set_plugin_parameter = &setHysteresisCountToMoving, .data = NULL},
	{	.name = PUD_GAT_HYSTERESIS_COUNT_2STAT_NAME, .set_plugin_parameter = &setGatewayHysteresisCountToStationary, .data = NULL},
	{	.name = PUD_GAT_HYSTERESIS_COUNT_2MOV_NAME, .set_plugin_parameter = &setGatewayHysteresisCountToMoving, .data = NULL},

	/* DEFAULTS */
	{	.name = PUD_DEFAULT_HDOP_NAME, .set_plugin_parameter = &setDefaultHdop, .data = NULL},
	{	.name = PUD_DEFAULT_VDOP_NAME, .set_plugin_parameter = &setDefaultVdop, .data = NULL},

	/* DEDUPLICATION */
	{	.name = PUD_USE_DEDUP_NAME, .set_plugin_parameter = &setUseDeDup, .data = NULL},
	{	.name = PUD_DEDUP_DEPTH_NAME, .set_plugin_parameter = &setDeDupDepth, .data = NULL},

	/* LOOPBACK */
	{	.name = PUD_USE_LOOPBACK_NAME, .set_plugin_parameter = &setUseLoopback, .data = NULL}
};

#endif /* _PUD_OLSRD_PLUGIN_H_ */
