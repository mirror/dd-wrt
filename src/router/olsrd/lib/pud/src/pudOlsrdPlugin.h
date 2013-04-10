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
	{	.name = PUD_NODE_ID_TYPE_NAME, .set_plugin_parameter = &setNodeIdType, .data = NULL},
	{	.name = PUD_NODE_ID_NAME, .set_plugin_parameter = &setNodeId, .data = NULL},

	/* OLSR */
	{	.name = PUD_OLSR_TTL_NAME, .set_plugin_parameter = &setOlsrTtl, .data = NULL},
	{	.name = PUD_UPDATE_INTERVAL_STATIONARY_NAME, .set_plugin_parameter = &setUpdateIntervalStationary, .data = NULL},
	{	.name = PUD_UPDATE_INTERVAL_MOVING_NAME, .set_plugin_parameter = &setUpdateIntervalMoving, .data = NULL},

	/* RX */
	{	.name = PUD_RX_NON_OLSR_IF_NAME, .set_plugin_parameter = &addRxNonOlsrInterface, .data = NULL},
	{	.name = PUD_RX_ALLOWED_SOURCE_IP_NAME, .set_plugin_parameter = &addRxAllowedSourceIpAddress, .data = NULL},
	{	.name = PUD_RX_MC_ADDR_NAME, .set_plugin_parameter = &setRxMcAddr, .data = NULL},
	{	.name = PUD_RX_MC_PORT_NAME, .set_plugin_parameter = &setRxMcPort, .data = NULL},
	{	.name = PUD_POSFILE_NAME, .set_plugin_parameter = &setPositionFile, .data = NULL},
	{	.name = PUD_POSFILEPERIOD_NAME, .set_plugin_parameter = &setPositionFilePeriod, .data = NULL},

	/* TX */
	{	.name = PUD_TX_NON_OLSR_IF_NAME, .set_plugin_parameter = &addTxNonOlsrInterface, .data = NULL},
	{	.name = PUD_TX_MC_ADDR_NAME, .set_plugin_parameter = &setTxMcAddr, .data = NULL},
	{	.name = PUD_TX_MC_PORT_NAME, .set_plugin_parameter = &setTxMcPort, .data = NULL},
	{	.name = PUD_TX_TTL_NAME, .set_plugin_parameter = &setTxTtl, .data = NULL},
	{	.name = PUD_TX_NMEAMESSAGEPREFIX_NAME, .set_plugin_parameter = &setTxNmeaMessagePrefix, .data = NULL},

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
