/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "uqmi.h"
#include "qmi-message.h"
#include "commands.h"

#include <libubox/blobmsg.h>

/* According to libqmi, a value of -32768 in 5G
 * indicates that the modem is not connected. */
#define _5GNR_NOT_CONNECTED_VALUE	-32768

static struct qmi_nas_get_tx_rx_info_request tx_rx_req;
static struct qmi_nas_set_system_selection_preference_request sel_req;
static struct	{
	bool mcc_is_set;
	bool mnc_is_set;
} plmn_code_flag;

static void
print_earfcn_info(uint32_t earfcn)
{
	/* https://www.sqimway.com/lte_band.php */
	static const struct {
		uint32_t    min;
		uint32_t    max;
		uint16_t    band;
		uint16_t    freq;
		const char *duplex;
	} earfcn_ranges[] = {
		{     0,   599, 1,  2100, "FDD" },
		{   600,  1199, 2,  1800, "FDD" },
		{  1200,  1949, 3,  1800, "FDD" },
		{  1950,  2399, 4,  1700, "FDD" },
		{  2400,  2649, 5,  850,  "FDD" },
		{  2650,  2749, 6,  800,  "FDD" },
		{  2750,  3449, 7,  2600, "FDD" },
		{  3450,  3799, 8,  900,  "FDD" },
		{  3800,  4149, 9,  1800, "FDD" },
		{  4150,  4749, 10, 1700, "FDD" },
		{  4750,  4999, 11, 1500, "FDD" },
		{  5000,  5179, 12, 700,  "FDD" },
		{  5180,  5279, 13, 700,  "FDD" },
		{  5280,  5379, 14, 700,  "FDD" },
		{  5730,  5849, 17, 700,  "FDD" },
		{  5850,  5999, 18, 850,  "FDD" },
		{  6000,  6149, 19, 850,  "FDD" },
		{  6150,  6449, 20, 800,  "FDD" },
		{  6450,  6599, 21, 1500, "FDD" },
		{  6600,  7399, 22, 3500, "FDD" },
		{  7500,  7699, 23, 2000, "FDD" },
		{  7700,  8039, 24, 1600, "FDD" },
		{  8040,  8689, 25, 1900, "FDD" },
		{  8690,  9039, 26, 850,  "FDD" },
		{  9040,  9209, 27, 800,  "FDD" },
		{  9210,  9659, 28, 700,  "FDD" },
		{  9660,  9769, 29, 700,  "SDL" },
		{  9770,  9869, 30, 2300, "FDD" },
		{  9870,  9919, 31, 450,  "FDD" },
		{  9920, 10359, 32, 1500, "SDL" },
		{ 36000, 36199, 33, 1900, "TDD" },
		{ 36200, 36349, 34, 2000, "TDD" },
		{ 36350, 36949, 35, 1900, "TDD" },
		{ 36950, 37549, 36, 1900, "TDD" },
		{ 37550, 37749, 37, 1900, "TDD" },
		{ 37750, 38249, 38, 2600, "TDD" },
		{ 38250, 38649, 39, 1900, "TDD" },
		{ 38650, 39649, 40, 2300, "TDD" },
		{ 39650, 41589, 41, 2500, "TDD" },
		{ 41590, 43589, 42, 3500, "TDD" },
		{ 43590, 45589, 43, 3700, "TDD" },
		{ 45590, 46589, 44, 700,  "TDD" },
	};

	for (int i = 0; i < (sizeof(earfcn_ranges) / sizeof(*earfcn_ranges)); i++) {
		if (earfcn <= earfcn_ranges[i].max && earfcn >= earfcn_ranges[i].min) {
			blobmsg_add_u32(&status, "band", earfcn_ranges[i].band);
			blobmsg_add_u32(&status, "frequency", earfcn_ranges[i].freq);
			blobmsg_add_string(&status, "duplex", earfcn_ranges[i].duplex);
			return;
		}
	}
}

static char *
print_radio_interface(int8_t radio_interface)
{
	switch (radio_interface) {
		case QMI_NAS_RADIO_INTERFACE_NONE:
			return "none";
		case QMI_NAS_RADIO_INTERFACE_CDMA_1X:
			return "cdma-1x";
		case QMI_NAS_RADIO_INTERFACE_CDMA_1XEVDO:
			return "cdma-1x_evdo";
		case QMI_NAS_RADIO_INTERFACE_AMPS:
			return "amps";
		case QMI_NAS_RADIO_INTERFACE_GSM:
			return "gsm";
		case QMI_NAS_RADIO_INTERFACE_UMTS:
			return "umts";
		case QMI_NAS_RADIO_INTERFACE_LTE:
			return "lte";
		case QMI_NAS_RADIO_INTERFACE_TD_SCDMA:
			return "td-scdma";
		case QMI_NAS_RADIO_INTERFACE_5GNR:
			return "5gnr";
		default:
			return "unknown";
	}
}

#define cmd_nas_do_set_system_selection_cb no_cb
static enum qmi_cmd_result
cmd_nas_do_set_system_selection_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_set_system_selection_preference_request(msg, &sel_req);
	return QMI_CMD_REQUEST;
}

static enum qmi_cmd_result
do_sel_network(void)
{
	static bool use_sel_req = false;

	if (!use_sel_req) {
		use_sel_req = true;
		uqmi_add_command(NULL, __UQMI_COMMAND_nas_do_set_system_selection);
	}

	return QMI_CMD_DONE;
}

#define cmd_nas_set_network_modes_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_network_modes_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static const struct {
		const char *name;
		QmiNasRatModePreference val;
	} modes[] = {
		{ "cdma", QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1X | QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1XEVDO },
		{ "td-scdma", QMI_NAS_RAT_MODE_PREFERENCE_TD_SCDMA },
		{ "gsm", QMI_NAS_RAT_MODE_PREFERENCE_GSM },
		{ "umts", QMI_NAS_RAT_MODE_PREFERENCE_UMTS },
		{ "lte", QMI_NAS_RAT_MODE_PREFERENCE_LTE },
		{ "5gnr", QMI_NAS_RAT_MODE_PREFERENCE_5GNR },
	};
	QmiNasRatModePreference val = 0;
	char *word;
	int i;

	for (word = strtok(arg, ",");
	     word;
	     word = strtok(NULL, ",")) {
		bool found = false;

		for (i = 0; i < ARRAY_SIZE(modes); i++) {
			if (strcmp(word, modes[i].name) != 0 &&
				strcmp(word, "all") != 0)
				continue;

			val |= modes[i].val;
			found = true;
		}

		if (!found) {
			uqmi_add_error("Invalid network mode");
			return QMI_CMD_EXIT;
		}
	}

	qmi_set(&sel_req, mode_preference, val);
	return do_sel_network();
}

#define cmd_nas_set_network_preference_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_network_preference_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	QmiNasGsmWcdmaAcquisitionOrderPreference pref = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_AUTOMATIC;

	if (!strcmp(arg, "gsm"))
		pref = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_GSM;
	else if (!strcmp(arg, "wcdma"))
		pref = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_WCDMA;

	qmi_set(&sel_req, gsm_wcdma_acquisition_order_preference, pref);
	return do_sel_network();
}

#define cmd_nas_set_roaming_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_roaming_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	QmiNasRoamingPreference pref;

	if (!strcmp(arg, "any"))
		pref = QMI_NAS_ROAMING_PREFERENCE_ANY;
	else if (!strcmp(arg, "only"))
		pref = QMI_NAS_ROAMING_PREFERENCE_NOT_OFF;
	else if (!strcmp(arg, "off"))
		pref = QMI_NAS_ROAMING_PREFERENCE_OFF;
	else
		return uqmi_add_error("Invalid argument");

	qmi_set(&sel_req, roaming_preference, pref);
	return do_sel_network();
}

#define cmd_nas_set_mcc_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_mcc_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	char *err;
	int value = strtoul(arg, &err, 10);
	if (err && *err) {
		uqmi_add_error("Invalid MCC value");
		return QMI_CMD_EXIT;
	}

	sel_req.data.network_selection_preference.mcc = value;
	plmn_code_flag.mcc_is_set = true;
	return QMI_CMD_DONE;
}

#define cmd_nas_set_mnc_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_mnc_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	char *err;
	int value = strtoul(arg, &err, 10);
	if (err && *err) {
		uqmi_add_error("Invalid MNC value");
		return QMI_CMD_EXIT;
	}

	sel_req.data.network_selection_preference.mnc = value;
	plmn_code_flag.mnc_is_set = true;
	return QMI_CMD_DONE;
}

#define cmd_nas_set_plmn_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_plmn_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	sel_req.set.network_selection_preference = 1;
	sel_req.data.network_selection_preference.mode = QMI_NAS_NETWORK_SELECTION_PREFERENCE_AUTOMATIC;

	if (!plmn_code_flag.mcc_is_set && plmn_code_flag.mnc_is_set) {
		uqmi_add_error("No MCC value");
		return QMI_CMD_EXIT;
	}

	if (plmn_code_flag.mcc_is_set && sel_req.data.network_selection_preference.mcc) {
		if (!plmn_code_flag.mnc_is_set) {
			uqmi_add_error("No MNC value");
			return QMI_CMD_EXIT;
		} else {
			sel_req.data.network_selection_preference.mode = QMI_NAS_NETWORK_SELECTION_PREFERENCE_MANUAL;
		}
	}

	return do_sel_network();
}

#define cmd_nas_initiate_network_register_cb no_cb
static enum qmi_cmd_result
cmd_nas_initiate_network_register_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static struct qmi_nas_initiate_network_register_request register_req = {
		QMI_INIT(action, QMI_NAS_NETWORK_REGISTER_TYPE_AUTOMATIC)
	};

	qmi_set_nas_initiate_network_register_request(msg, &register_req);
	return QMI_CMD_REQUEST;
}

static void
cmd_nas_get_signal_info_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_nas_get_signal_info_response res;
	void *c, *a;
	bool is_5gnr_connected = false;
	bool is_5gnr_endc = false;

	qmi_parse_nas_get_signal_info_response(msg, &res);

	/* If 5G NR EN-DC (dual connectivity) is enabled, the mobile device has two connections,
	 * one with the LTE base station, and one with the NR base station.
	 * Therefore an array of signals has to be reported in this case. */
	is_5gnr_connected = ((res.set._5g_signal_strength &&
		((res.data._5g_signal_strength.rsrp != _5GNR_NOT_CONNECTED_VALUE) ||
		 (res.data._5g_signal_strength.snr != _5GNR_NOT_CONNECTED_VALUE))) ||
		(res.set._5g_signal_strength_extended &&
		(res.data._5g_signal_strength_extended != _5GNR_NOT_CONNECTED_VALUE)));
	is_5gnr_endc = (res.set.lte_signal_strength && is_5gnr_connected);

	if (is_5gnr_endc) {
		a = blobmsg_open_array(&status, NULL);
	}

	c = blobmsg_open_table(&status, NULL);
	if (res.set.cdma_signal_strength) {
		blobmsg_add_string(&status, "type", "cdma");
		blobmsg_add_u32(&status, "rssi", (int32_t) res.data.cdma_signal_strength.rssi);
		blobmsg_add_u32(&status, "ecio", (int32_t) res.data.cdma_signal_strength.ecio);
	}

	if (res.set.hdr_signal_strength) {
		blobmsg_add_string(&status, "type", "hdr");
		blobmsg_add_u32(&status, "rssi", (int32_t) res.data.hdr_signal_strength.rssi);
		blobmsg_add_u32(&status, "ecio", (int32_t) res.data.hdr_signal_strength.ecio);
		blobmsg_add_u32(&status, "io", res.data.hdr_signal_strength.io);
	}

	if (res.set.gsm_signal_strength) {
		blobmsg_add_string(&status, "type", "gsm");
		blobmsg_add_u32(&status, "signal", (int32_t) res.data.gsm_signal_strength);
	}

	if (res.set.wcdma_signal_strength) {
		blobmsg_add_string(&status, "type", "wcdma");
		blobmsg_add_u32(&status, "rssi", (int32_t) res.data.wcdma_signal_strength.rssi);
		blobmsg_add_u32(&status, "ecio", (int32_t) res.data.wcdma_signal_strength.ecio);
	}

	if (res.set.lte_signal_strength) {
		blobmsg_add_string(&status, "type", "lte");
		blobmsg_add_u32(&status, "rssi", (int32_t) res.data.lte_signal_strength.rssi);
		blobmsg_add_u32(&status, "rsrq", (int32_t) res.data.lte_signal_strength.rsrq);
		blobmsg_add_u32(&status, "rsrp", (int32_t) res.data.lte_signal_strength.rsrp);
		blobmsg_add_double(&status, "snr", (double) res.data.lte_signal_strength.snr*0.1);
	}

	if (res.set.tdma_signal_strength) {
		blobmsg_add_string(&status, "type", "tdma");
		blobmsg_add_u32(&status, "signal", (int32_t) res.data.tdma_signal_strength);
	}

	if (is_5gnr_connected) {
		if (is_5gnr_endc) {
			blobmsg_close_table(&status, c);
			c = blobmsg_open_table(&status, NULL);
		}
		blobmsg_add_string(&status, "type", "5gnr");
		if (res.set._5g_signal_strength) {
			if (res.data._5g_signal_strength.rsrp != _5GNR_NOT_CONNECTED_VALUE)
				blobmsg_add_u32(&status, "rsrp", (int32_t) res.data._5g_signal_strength.rsrp);
			if (res.data._5g_signal_strength.snr != _5GNR_NOT_CONNECTED_VALUE)
				blobmsg_add_double(&status, "snr", (double) res.data._5g_signal_strength.snr*0.1);
		}

		if (res.set._5g_signal_strength_extended &&
			(res.data._5g_signal_strength_extended != _5GNR_NOT_CONNECTED_VALUE)) {
			blobmsg_add_u32(&status, "rsrq", (int32_t) res.data._5g_signal_strength_extended);
		}
	}

	blobmsg_close_table(&status, c);

	if (is_5gnr_endc) {
		blobmsg_close_array(&status, a);
	}
}

static void
print_system_info(uint8_t svc_status, uint8_t tsvc_status, bool preferred, bool system_info,
		  bool domain_valid, uint8_t domain,
		  bool service_cap_valid, uint8_t service_cap,
		  bool roaming_status_valid, uint8_t roaming_status,
		  bool forbidden_valid, bool forbidden,
		  bool network_id_valid, char *mcc, char *mnc,
		  bool lac_valid, uint16_t lac)
{
	static const char *map_service[] = {
		[QMI_NAS_SERVICE_STATUS_NONE] = "none",
		[QMI_NAS_SERVICE_STATUS_LIMITED] = "limited",
		[QMI_NAS_SERVICE_STATUS_AVAILABLE] = "available",
		[QMI_NAS_SERVICE_STATUS_LIMITED_REGIONAL] = "limited regional",
		[QMI_NAS_SERVICE_STATUS_POWER_SAVE] = "power save",
	};

	static const char *map_roaming[] = {
		[QMI_NAS_ROAMING_STATUS_OFF] = "off",
		[QMI_NAS_ROAMING_STATUS_ON] = "on",
		[QMI_NAS_ROAMING_STATUS_BLINK] = "blink",
		[QMI_NAS_ROAMING_STATUS_OUT_OF_NEIGHBORHOOD] = "out of neighborhood",
		[QMI_NAS_ROAMING_STATUS_OUT_OF_BUILDING] = "out of building",
		[QMI_NAS_ROAMING_STATUS_PREFERRED_SYSTEM] = "preferred system",
		[QMI_NAS_ROAMING_STATUS_AVAILABLE_SYSTEM] = "available system",
		[QMI_NAS_ROAMING_STATUS_ALLIANCE_PARTNER] = "alliance partner",
		[QMI_NAS_ROAMING_STATUS_PREMIUM_PARTNER] = "premium partner",
		[QMI_NAS_ROAMING_STATUS_FULL_SERVICE] = "full service",
		[QMI_NAS_ROAMING_STATUS_PARTIAL_SERVICE] = "partial service",
		[QMI_NAS_ROAMING_STATUS_BANNER_ON] = "banner on",
		[QMI_NAS_ROAMING_STATUS_BANNER_OFF] = "banner off",
	};

	static const char *map_network[] = {
		[QMI_NAS_NETWORK_SERVICE_DOMAIN_NONE] = "none",
		[QMI_NAS_NETWORK_SERVICE_DOMAIN_CS] = "cs",
		[QMI_NAS_NETWORK_SERVICE_DOMAIN_PS] = "ps",
		[QMI_NAS_NETWORK_SERVICE_DOMAIN_CS_PS] = "cs-ps",
		[QMI_NAS_NETWORK_SERVICE_DOMAIN_UNKNOWN] = "unknown",
		};

	blobmsg_add_string(&status, "service_status", map_service[svc_status]);
	blobmsg_add_string(&status, "true_service_status", map_service[tsvc_status]);
	blobmsg_add_u8(&status, "preferred_data_path", preferred);

	if (system_info) {
		if (domain_valid)
			blobmsg_add_string(&status, "domain", map_network[domain]);
		if (service_cap_valid)
			blobmsg_add_string(&status, "service", map_network[service_cap]);
		if (roaming_status_valid)
			blobmsg_add_string(&status, "roaming_status", map_roaming[roaming_status]);
		if (forbidden_valid)
			blobmsg_add_u8(&status, "forbidden", forbidden);
		if (network_id_valid) {
			blobmsg_add_string(&status, "mcc", mcc);
			if ((uint8_t)mnc[2] == 255)
				mnc[2] = 0;
			blobmsg_add_string(&status, "mnc", mnc);
		}
		if (lac_valid)
			blobmsg_add_u32(&status, "location_area_code", (int32_t) lac);
	}
}

static void
cmd_nas_get_system_info_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	static const char *cell_status[] = {
		[QMI_NAS_CELL_ACCESS_STATUS_NORMAL_ONLY] = "normal",
		[QMI_NAS_CELL_ACCESS_STATUS_EMERGENCY_ONLY] = "emergency",
		[QMI_NAS_CELL_ACCESS_STATUS_NO_CALLS] = "no calls",
		[QMI_NAS_CELL_ACCESS_STATUS_ALL_CALLS] = "all calls",
		[QMI_NAS_CELL_ACCESS_STATUS_UNKNOWN] = "unknown",
	};

	struct qmi_nas_get_system_info_response res;
	void *c, *t;

	qmi_parse_nas_get_system_info_response(msg, &res);
	t = blobmsg_open_table(&status, NULL);
	if (res.set.gsm_service_status) {
		c = blobmsg_open_table(&status, "gsm");
		print_system_info(res.data.gsm_service_status.service_status,
				  res.data.gsm_service_status.true_service_status,
				  res.data.gsm_service_status.preferred_data_path,
				  res.set.gsm_system_info_v2,
				  res.data.gsm_system_info_v2.domain_valid,
				  res.data.gsm_system_info_v2.domain,
				  res.data.gsm_system_info_v2.service_capability_valid,
				  res.data.gsm_system_info_v2.service_capability,
				  res.data.gsm_system_info_v2.roaming_status_valid,
				  res.data.gsm_system_info_v2.roaming_status,
				  res.data.gsm_system_info_v2.forbidden_valid,
				  res.data.gsm_system_info_v2.forbidden,
				  res.data.gsm_system_info_v2.network_id_valid,
				  res.data.gsm_system_info_v2.mcc,
				  res.data.gsm_system_info_v2.mnc,
				  res.data.gsm_system_info_v2.lac_valid,
				  res.data.gsm_system_info_v2.lac);
		if (res.set.gsm_system_info_v2 && res.data.gsm_system_info_v2.cid_valid)
			blobmsg_add_u32(&status, "cell_id",
					res.data.gsm_system_info_v2.cid);
		if (res.set.additional_gsm_system_info &&
		    res.data.additional_gsm_system_info.geo_system_index != 0xFFFF)
			blobmsg_add_u32(&status, "geo_system_index",
					res.data.additional_gsm_system_info.geo_system_index);
		blobmsg_close_table(&status, c);
	}

	if (res.set.wcdma_service_status) {
		c = blobmsg_open_table(&status, "wcdma");
		print_system_info(res.data.wcdma_service_status.service_status,
				  res.data.wcdma_service_status.true_service_status,
				  res.data.wcdma_service_status.preferred_data_path,
				  res.set.wcdma_system_info_v2,
				  res.data.wcdma_system_info_v2.domain_valid,
				  res.data.wcdma_system_info_v2.domain,
				  res.data.wcdma_system_info_v2.service_capability_valid,
				  res.data.wcdma_system_info_v2.service_capability,
				  res.data.wcdma_system_info_v2.roaming_status_valid,
				  res.data.wcdma_system_info_v2.roaming_status,
				  res.data.wcdma_system_info_v2.forbidden_valid,
				  res.data.wcdma_system_info_v2.forbidden,
				  res.data.wcdma_system_info_v2.network_id_valid,
				  res.data.wcdma_system_info_v2.mcc,
				  res.data.wcdma_system_info_v2.mnc,
				  res.data.wcdma_system_info_v2.lac_valid,
				  res.data.wcdma_system_info_v2.lac);
		if (res.set.wcdma_system_info_v2 && res.data.wcdma_system_info_v2.cid_valid) {
			blobmsg_add_u32(&status, "rnc_id",res.data.wcdma_system_info_v2.cid/65536);
			blobmsg_add_u32(&status, "cell_id",res.data.wcdma_system_info_v2.cid%65536);
		}
		if (res.set.additional_wcdma_system_info &&
		    res.data.additional_wcdma_system_info.geo_system_index != 0xFFFF)
			blobmsg_add_u32(&status, "geo_system_index",
					res.data.additional_wcdma_system_info.geo_system_index);
		blobmsg_close_table(&status, c);
	}

	if (res.set.lte_service_status) {
		c = blobmsg_open_table(&status, "lte");
		print_system_info(res.data.lte_service_status.service_status,
				  res.data.lte_service_status.true_service_status,
				  res.data.lte_service_status.preferred_data_path,
				  res.set.lte_system_info_v2,
				  res.data.lte_system_info_v2.domain_valid,
				  res.data.lte_system_info_v2.domain,
				  res.data.lte_system_info_v2.service_capability_valid,
				  res.data.lte_system_info_v2.service_capability,
				  res.data.lte_system_info_v2.roaming_status_valid,
				  res.data.lte_system_info_v2.roaming_status,
				  res.data.lte_system_info_v2.forbidden_valid,
				  res.data.lte_system_info_v2.forbidden,
				  res.data.lte_system_info_v2.network_id_valid,
				  res.data.lte_system_info_v2.mcc,
				  res.data.lte_system_info_v2.mnc,
				  res.data.lte_system_info_v2.lac_valid,
				  res.data.lte_system_info_v2.lac);
		if (res.set.lte_system_info_v2 && res.data.lte_system_info_v2.tac_valid)
			blobmsg_add_u32(&status, "tracking_area_code",
					res.data.lte_system_info_v2.tac);
		if (res.set.lte_system_info_v2 && res.data.lte_system_info_v2.cid_valid) {
			blobmsg_add_u32(&status, "enodeb_id",res.data.lte_system_info_v2.cid/256);
			blobmsg_add_u32(&status, "cell_id",res.data.lte_system_info_v2.cid%256);
		}
		if (res.set.additional_lte_system_info &&
		    res.data.additional_lte_system_info.geo_system_index != 0xFFFF)
			blobmsg_add_u32(&status, "geo_system_index",
					res.data.additional_lte_system_info.geo_system_index);
		if (res.set.lte_voice_support)
			blobmsg_add_u8(&status, "voice_support", res.data.lte_voice_support);
		if (res.set.ims_voice_support)
			blobmsg_add_u8(&status, "ims_voice_support", res.data.ims_voice_support);
		if (res.set.lte_cell_access_status)
			blobmsg_add_string(&status, "cell_access_status",
					   cell_status[res.data.lte_cell_access_status]);
		if (res.set.network_selection_registration_restriction)
			blobmsg_add_u32(&status, "registration_restriction",
					res.data.network_selection_registration_restriction);
		if (res.set.lte_registration_domain)
			blobmsg_add_u32(&status, "registration_domain",
					res.data.lte_registration_domain);
		if (res.set.eutra_with_nr5g_availability)
			blobmsg_add_u8(&status, "5g_nsa_available",
				       res.data.eutra_with_nr5g_availability);
		if (res.set.dcnr_restriction_info)
			blobmsg_add_u8(&status, "dcnr_restriction", res.data.dcnr_restriction_info);

		blobmsg_close_table(&status, c);
	}

	if (res.set.nr5g_service_status_info) {
		c = blobmsg_open_table(&status, "5gnr");
		print_system_info(res.data.nr5g_service_status_info.service_status,
				  res.data.nr5g_service_status_info.true_service_status,
				  res.data.nr5g_service_status_info.preferred_data_path,
				  res.set.nr5g_system_info,
				  res.data.nr5g_system_info.domain_valid,
				  res.data.nr5g_system_info.domain,
				  res.data.nr5g_system_info.service_capability_valid,
				  res.data.nr5g_system_info.service_capability,
				  res.data.nr5g_system_info.roaming_status_valid,
				  res.data.nr5g_system_info.roaming_status,
				  res.data.nr5g_system_info.forbidden_valid,
				  res.data.nr5g_system_info.forbidden,
				  res.data.nr5g_system_info.network_id_valid,
				  res.data.nr5g_system_info.mcc,
				  res.data.nr5g_system_info.mnc,
				  res.data.nr5g_system_info.lac_valid,
				  res.data.nr5g_system_info.lac);
		if (res.set.nr5g_system_info && res.data.nr5g_system_info.tac_valid)
			blobmsg_add_u32(&status, "tracking_area_code",
					res.data.nr5g_system_info.tac);
		if (res.set.nr5g_system_info && res.data.nr5g_system_info.cid_valid) {
			blobmsg_add_u32(&status, "enodeb_id",res.data.nr5g_system_info.cid/256);
			blobmsg_add_u32(&status, "cell_id",res.data.nr5g_system_info.cid%256);
		}

		blobmsg_close_table(&status, c);
	}

	blobmsg_close_table(&status, t);
}

static enum qmi_cmd_result
cmd_nas_get_system_info_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_get_system_info_request(msg);
	return QMI_CMD_REQUEST;
}

static void
print_channel_info(int32_t cell_id, int32_t channel, uint32_t bw)
{
	static const char *map_bandwidth[] = {
		[QMI_NAS_DL_BANDWIDTH_1_4] = "1.4",
		[QMI_NAS_DL_BANDWIDTH_3] = "3",
		[QMI_NAS_DL_BANDWIDTH_5] = "5",
		[QMI_NAS_DL_BANDWIDTH_10] = "10",
		[QMI_NAS_DL_BANDWIDTH_15] = "15",
		[QMI_NAS_DL_BANDWIDTH_20] = "20",
		[QMI_NAS_DL_BANDWIDTH_INVALID] = "invalid",
		[QMI_NAS_DL_BANDWIDTH_UNKNOWN] = "unknown",
	};

	blobmsg_add_u32(&status, "cell_id", cell_id);
	blobmsg_add_u32(&status, "channel", channel);
	print_earfcn_info(channel);
	blobmsg_add_string(&status, "bandwidth", map_bandwidth[bw]);
}

static void
cmd_nas_get_lte_cphy_ca_info_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_nas_get_lte_cphy_ca_info_response res;
	static const char *scell_state[] = {
		[QMI_NAS_SCELL_STATE_DECONFIGURED] = "deconfigured",
		[QMI_NAS_SCELL_STATE_DEACTIVATED] = "deactivated",
		[QMI_NAS_SCELL_STATE_ACTIVATED] = "activated",
	};
	char idx_buf[16];
	void *t, *c;
	int i;

	qmi_parse_nas_get_lte_cphy_ca_info_response(msg, &res);
	t = blobmsg_open_table(&status, NULL);
	if (res.set.phy_ca_agg_pcell_info) {
		c = blobmsg_open_table(&status, "primary");
		print_channel_info(res.data.phy_ca_agg_pcell_info.physical_cell_id,
				   res.data.phy_ca_agg_pcell_info.rx_channel,
				   res.data.phy_ca_agg_pcell_info.dl_bandwidth);
		blobmsg_close_table(&status, c);
	}
	if (res.set.phy_ca_agg_scell_info && res.data.phy_ca_agg_secondary_cells_n) {
		for (i = 0; i < res.data.phy_ca_agg_secondary_cells_n; i++) {
			if (res.data.phy_ca_agg_secondary_cells[i].rx_channel == 0)
				break;
			sprintf(idx_buf, "secondary_%d",
				res.data.phy_ca_agg_secondary_cells[i].cell_index);
			c = blobmsg_open_table(&status, idx_buf);
			print_channel_info(res.data.phy_ca_agg_secondary_cells[i].physical_cell_id,
					   res.data.phy_ca_agg_secondary_cells[i].rx_channel,
					   res.data.phy_ca_agg_secondary_cells[i].dl_bandwidth);
			blobmsg_add_string(&status, "state",
					   scell_state[res.data.phy_ca_agg_secondary_cells[i].state]);
			blobmsg_close_table(&status, c);
		}
	} else {
		if (res.set.scell_index)
			sprintf(idx_buf, "secondary_%d", res.data.scell_index);
		else
			sprintf(idx_buf, "secondary");
		if (res.set.phy_ca_agg_scell_info && res.data.phy_ca_agg_scell_info.rx_channel != 0) {
			c = blobmsg_open_table(&status, idx_buf);
			print_channel_info(res.data.phy_ca_agg_scell_info.physical_cell_id,
					   res.data.phy_ca_agg_scell_info.rx_channel,
					   res.data.phy_ca_agg_scell_info.dl_bandwidth);
			blobmsg_add_string(&status, "state",
					   scell_state[res.data.phy_ca_agg_scell_info.state]);
			blobmsg_close_table(&status, c);
		}
	}
	blobmsg_close_table(&status, t);
}

static enum qmi_cmd_result
cmd_nas_get_lte_cphy_ca_info_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_get_lte_cphy_ca_info_request(msg);
	return QMI_CMD_REQUEST;
}

static void
print_chain_info(int8_t radio, bool tuned, int32_t rssi, int32_t ecio, int32_t rsrp, int32_t rscp, uint32_t phase)
{
	blobmsg_add_u8(&status, "tuned", tuned);
	blobmsg_add_double(&status, "rssi", (double) rssi*0.1);
	if (radio == QMI_NAS_RADIO_INTERFACE_5GNR) {
		blobmsg_add_double(&status, "rsrp", (double) rsrp*-0.1);
	}
	else if (radio == QMI_NAS_RADIO_INTERFACE_LTE) {
		blobmsg_add_double(&status, "rsrq", (double) ecio*-0.1);
		blobmsg_add_double(&status, "rsrp", (double) rsrp*-0.1);
	}
	else if (radio == QMI_NAS_RADIO_INTERFACE_UMTS) {
		blobmsg_add_double(&status, "ecio", (double) ecio*-0.1);
		blobmsg_add_double(&status, "rscp", (double) rscp*-0.1);
	}
	if (phase != 0xFFFFFFFF)
		blobmsg_add_double(&status, "phase", (double) phase*0.01);
}

static void
cmd_nas_get_tx_rx_info_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_nas_get_tx_rx_info_response res;
	void *c, *t;

	qmi_parse_nas_get_tx_rx_info_response(msg, &res);
	t = blobmsg_open_table(&status, NULL);
	if (res.set.rx_chain_0_info) {
		c = blobmsg_open_table(&status, "rx_chain_0");
		print_chain_info(tx_rx_req.data.radio_interface,
				 res.data.rx_chain_0_info.is_radio_tuned,
				 res.data.rx_chain_0_info.rx_power,
				 res.data.rx_chain_0_info.ecio,
				 res.data.rx_chain_0_info.rsrp,
				 res.data.rx_chain_0_info.rscp,
				 res.data.rx_chain_0_info.phase);
		blobmsg_close_table(&status, c);
	}
	if (res.set.rx_chain_1_info) {
		c = blobmsg_open_table(&status, "rx_chain_1");
		print_chain_info(tx_rx_req.data.radio_interface,
				 res.data.rx_chain_1_info.is_radio_tuned,
				 res.data.rx_chain_1_info.rx_power,
				 res.data.rx_chain_1_info.ecio,
				 res.data.rx_chain_1_info.rsrp,
				 res.data.rx_chain_1_info.rscp,
				 res.data.rx_chain_1_info.phase);
		blobmsg_close_table(&status, c);
	}
	if (res.set.rx_chain_2_info) {
		c = blobmsg_open_table(&status, "rx_chain_2");
		print_chain_info(tx_rx_req.data.radio_interface,
				 res.data.rx_chain_2_info.is_radio_tuned,
				 res.data.rx_chain_2_info.rx_power,
				 res.data.rx_chain_2_info.ecio,
				 res.data.rx_chain_2_info.rsrp,
				 res.data.rx_chain_2_info.rscp,
				 res.data.rx_chain_2_info.phase);
		blobmsg_close_table(&status, c);
	}
	if (res.set.rx_chain_3_info) {
		c = blobmsg_open_table(&status, "rx_chain_3");
		print_chain_info(tx_rx_req.data.radio_interface,
				 res.data.rx_chain_3_info.is_radio_tuned,
				 res.data.rx_chain_3_info.rx_power,
				 res.data.rx_chain_3_info.ecio,
				 res.data.rx_chain_3_info.rsrp,
				 res.data.rx_chain_3_info.rscp,
				 res.data.rx_chain_3_info.phase);
		blobmsg_close_table(&status, c);
	}
	if (res.set.tx_info) {
		c = blobmsg_open_table(&status, "tx");
		blobmsg_add_u8(&status, "traffic", res.data.tx_info.is_in_traffic);
		if (res.data.tx_info.is_in_traffic)
			blobmsg_add_double(&status, "tx_power",
					   (double) res.data.tx_info.tx_power*0.1);
		blobmsg_close_table(&status, c);
	}
	blobmsg_close_table(&status, t);
}


static enum qmi_cmd_result
cmd_nas_get_tx_rx_info_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	int radio = 0;

	if (!strcmp(arg, "5gnr"))
		radio = QMI_NAS_RADIO_INTERFACE_5GNR;
	else if (!strcmp(arg, "lte"))
		radio = QMI_NAS_RADIO_INTERFACE_LTE;
	else if (!strcmp(arg, "umts"))
		radio = QMI_NAS_RADIO_INTERFACE_UMTS;
	else if (!strcmp(arg, "gsm"))
		radio = QMI_NAS_RADIO_INTERFACE_GSM;
	else
		return uqmi_add_error("Invalid argument");

	qmi_set(&tx_rx_req, radio_interface, radio);
	qmi_set_nas_get_tx_rx_info_request(msg, &tx_rx_req);
	return QMI_CMD_REQUEST;
}

static void
print_lte_info(int32_t cell_id, int16_t rsrp, int16_t rsrq, int16_t rssi)
{
	blobmsg_add_u32(&status, "physical_cell_id", cell_id);
	blobmsg_add_double(&status, "rsrq", ((double)rsrq)/10);
	blobmsg_add_double(&status, "rsrp", ((double)rsrp)/10);
	blobmsg_add_double(&status, "rssi", ((double)rssi)/10);
}

static void
print_sel_info(int32_t priority, int32_t high, int32_t low)
{
	blobmsg_add_u32(&status, "cell_reselection_priority", priority);
	blobmsg_add_u32(&status, "cell_reselection_low", low);
	blobmsg_add_u32(&status, "cell_reselection_high", high);
}

static void
cmd_nas_get_cell_location_info_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_nas_get_cell_location_info_response res;
	void *c = NULL, *t, *cell, *freq;
	int i, j;

	qmi_parse_nas_get_cell_location_info_response(msg, &res);
	t = blobmsg_open_table(&status, NULL);

	if (res.set.umts_info_v2) {
		c = blobmsg_open_table(&status, "umts_info");
		blobmsg_add_u32(&status, "location_area_code", res.data.umts_info_v2.lac);
		blobmsg_add_u32(&status, "cell_id", res.data.umts_info_v2.cell_id);
		blobmsg_add_u32(&status, "channel",
				res.data.umts_info_v2.utra_absolute_rf_channel_number);
		blobmsg_add_u32(&status, "primary_scrambling_code",
				res.data.umts_info_v2.primary_scrambling_code);
		blobmsg_add_u32(&status, "rscp", res.data.umts_info_v2.rscp);
		blobmsg_add_u32(&status, "ecio", res.data.umts_info_v2.ecio);
		for (j = 0; j < res.data.umts_info_v2.cell_n; j++) {
			cell = blobmsg_open_table(&status, NULL);
			blobmsg_add_u32(&status, "channel",
					res.data.umts_info_v2.cell[j].utra_absolute_rf_channel_number);
			blobmsg_add_u32(&status, "primary_scrambling_code",
					res.data.umts_info_v2.cell[j].primary_scrambling_code);
			blobmsg_add_u32(&status, "rscp", res.data.umts_info_v2.cell[j].rscp);
			blobmsg_add_u32(&status, "ecio", res.data.umts_info_v2.cell[j].ecio);
			blobmsg_close_table(&status, cell);
		}
		for (j = 0; j < res.data.umts_info_v2.neighboring_geran_n; j++) {
			cell = blobmsg_open_table(&status, "neighboring_geran");
			blobmsg_add_u32(&status, "channel",
					res.data.umts_info_v2.neighboring_geran[j].geran_absolute_rf_channel_number);
			blobmsg_add_u8(&status, "network_color_code",
				       res.data.umts_info_v2.neighboring_geran[j].network_color_code);
			blobmsg_add_u8(&status, "base_station_color_code",
				       res.data.umts_info_v2.neighboring_geran[j].base_station_color_code);
			blobmsg_add_u32(&status, "rssi",
					res.data.umts_info_v2.neighboring_geran[j].rssi);
			blobmsg_close_table(&status, cell);
		}
		blobmsg_close_table(&status, c);
	}
	if (res.set.intrafrequency_lte_info_v2) {
		c = blobmsg_open_table(&status, "intrafrequency_lte_info");
		blobmsg_add_u32(&status, "tracking_area_code",
				res.data.intrafrequency_lte_info_v2.tracking_area_code);
		blobmsg_add_u32(&status, "enodeb_id",
				res.data.intrafrequency_lte_info_v2.global_cell_id/256);
		blobmsg_add_u32(&status, "cell_id",
				res.data.intrafrequency_lte_info_v2.global_cell_id%256);
		blobmsg_add_u32(&status, "channel",
				res.data.intrafrequency_lte_info_v2.eutra_absolute_rf_channel_number);
		print_earfcn_info(res.data.intrafrequency_lte_info_v2.eutra_absolute_rf_channel_number);
		blobmsg_add_u32(&status, "serving_cell_id",
				res.data.intrafrequency_lte_info_v2.serving_cell_id);
		if (res.data.intrafrequency_lte_info_v2.ue_in_idle) {
			blobmsg_add_u32(&status, "cell_reselection_priority",
					res.data.intrafrequency_lte_info_v2.cell_reselection_priority);
			blobmsg_add_u32(&status, "s_non_intra_search_threshold",
					res.data.intrafrequency_lte_info_v2.s_non_intra_search_threshold);
			blobmsg_add_u32(&status, "serving_cell_low_threshold",
					res.data.intrafrequency_lte_info_v2.serving_cell_low_threshold);
			blobmsg_add_u32(&status, "s_intra_search_threshold",
					res.data.intrafrequency_lte_info_v2.s_intra_search_threshold);
		}
		for (i = 0; i < res.data.intrafrequency_lte_info_v2.cell_n; i++) {
			cell = blobmsg_open_table(&status, NULL);
			print_lte_info(res.data.intrafrequency_lte_info_v2.cell[i].physical_cell_id,
				       res.data.intrafrequency_lte_info_v2.cell[i].rsrq,
				       res.data.intrafrequency_lte_info_v2.cell[i].rsrp,
				       res.data.intrafrequency_lte_info_v2.cell[i].rssi);
			if (res.data.intrafrequency_lte_info_v2.ue_in_idle)
				blobmsg_add_u32(&status, "cell_selection_rx_level",
						res.data.intrafrequency_lte_info_v2.cell[i].cell_selection_rx_level);
			blobmsg_close_table(&status, cell);
		}
		blobmsg_close_table(&status, c);
	}
	if (res.set.interfrequency_lte_info) {
		if (res.data.interfrequency_lte_info.frequency_n > 0)
			c = blobmsg_open_table(&status, "interfrequency_lte_info");
		for (i = 0; i < res.data.interfrequency_lte_info.frequency_n; i++) {
			freq = blobmsg_open_table(&status, NULL);
			blobmsg_add_u32(&status, "channel",
					res.data.interfrequency_lte_info.frequency[i].eutra_absolute_rf_channel_number);
			print_earfcn_info(res.data.interfrequency_lte_info.frequency[i].eutra_absolute_rf_channel_number);
			if (res.data.interfrequency_lte_info.ue_in_idle) {
				print_sel_info(res.data.interfrequency_lte_info.frequency[i].cell_reselection_priority,
					       res.data.interfrequency_lte_info.frequency[i].cell_selection_rx_level_high_threshold,
					       res.data.interfrequency_lte_info.frequency[i].cell_selection_rx_level_low_threshold);
			}
			for (j = 0; j < res.data.interfrequency_lte_info.frequency[i].cell_n; j++) {
				cell = blobmsg_open_table(&status, NULL);
				print_lte_info(res.data.interfrequency_lte_info.frequency[i].cell[j].physical_cell_id,
					       res.data.interfrequency_lte_info.frequency[i].cell[j].rsrq,
					       res.data.interfrequency_lte_info.frequency[i].cell[j].rsrp,
					       res.data.interfrequency_lte_info.frequency[i].cell[j].rssi);
				if (res.data.interfrequency_lte_info.ue_in_idle)
					blobmsg_add_u32(&status, "cell_selection_rx_level",
							res.data.interfrequency_lte_info.frequency[i].cell[j].cell_selection_rx_level);
				blobmsg_close_table(&status, cell);
			}
			blobmsg_close_table(&status, freq);
		}
		if (res.data.interfrequency_lte_info.frequency_n > 0)
			blobmsg_close_table(&status, c);
	}
	if (res.set.lte_info_neighboring_gsm) {
		if (res.data.lte_info_neighboring_gsm.frequency_n > 0)
			c = blobmsg_open_table(&status, "lte_info_neighboring_gsm");
		for (i = 0; i < res.data.lte_info_neighboring_gsm.frequency_n; i++) {
			freq = blobmsg_open_table(&status, NULL);
			blobmsg_add_u32(&status, "ncc_permitted",
					res.data.lte_info_neighboring_gsm.frequency[i].ncc_permitted);
			if (res.data.lte_info_neighboring_gsm.ue_in_idle) {
				print_sel_info(res.data.lte_info_neighboring_gsm.frequency[i].cell_reselection_priority,
					       res.data.lte_info_neighboring_gsm.frequency[i].cell_reselection_high_threshold,
					       res.data.lte_info_neighboring_gsm.frequency[i].cell_reselection_low_threshold);
			}
			for (j = 0; j < res.data.lte_info_neighboring_gsm.frequency[i].cell_n; j++) {
				cell = blobmsg_open_table(&status, NULL);
				blobmsg_add_u32(&status, "channel",
						res.data.lte_info_neighboring_gsm.frequency[i].cell[j].geran_absolute_rf_channel_number);
				blobmsg_add_u32(&status, "base_station_identity_code",
						res.data.lte_info_neighboring_gsm.frequency[i].cell[j].base_station_identity_code);
				blobmsg_add_double(&status, "rssi",
						   ((double)res.data.lte_info_neighboring_gsm.frequency[i].cell[j].rssi)/10);
				if (res.data.lte_info_neighboring_gsm.ue_in_idle)
					blobmsg_add_u32(&status, "cell_selection_rx_level",
							res.data.lte_info_neighboring_gsm.frequency[i].cell[j].cell_selection_rx_level);
				blobmsg_close_table(&status, cell);
			}
			blobmsg_close_table(&status, freq);
		}
		if (res.data.lte_info_neighboring_gsm.frequency_n > 0)
			blobmsg_close_table(&status, c);
	}
	if (res.set.lte_info_neighboring_wcdma) {
		if (res.data.lte_info_neighboring_wcdma.frequency_n > 0)
			c = blobmsg_open_table(&status, "lte_info_neighboring_wcdma");
		for (i = 0; i < res.data.lte_info_neighboring_wcdma.frequency_n; i++) {
			freq = blobmsg_open_table(&status, NULL);
			blobmsg_add_u32(&status, "channel",
					res.data.lte_info_neighboring_wcdma.frequency[i].utra_absolute_rf_channel_number);
			if (res.data.lte_info_neighboring_wcdma.ue_in_idle) {
				print_sel_info(res.data.lte_info_neighboring_wcdma.frequency[i].cell_reselection_priority,
					       res.data.lte_info_neighboring_wcdma.frequency[i].cell_reselection_high_threshold,
					       res.data.lte_info_neighboring_wcdma.frequency[i].cell_reselection_low_threshold);
			}
			for (j = 0; j < res.data.lte_info_neighboring_wcdma.frequency[i].cell_n; j++) {
				cell = blobmsg_open_table(&status, NULL);
				blobmsg_add_u32(&status, "primary_scrambling_code",
						res.data.lte_info_neighboring_wcdma.frequency[i].cell[j].primary_scrambling_code);
				blobmsg_add_double(&status, "rscp",
						   ((double)res.data.lte_info_neighboring_wcdma.frequency[i].cell[j].cpich_rscp)/10);
				blobmsg_add_double(&status, "ecno",
						   ((double)res.data.lte_info_neighboring_wcdma.frequency[i].cell[j].cpich_ecno)/10);
				if (res.data.lte_info_neighboring_wcdma.ue_in_idle)
					blobmsg_add_u32(&status, "cell_selection_rx_level",
							res.data.lte_info_neighboring_wcdma.frequency[i].cell[j].cell_selection_rx_level);
				blobmsg_close_table(&status, cell);
			}
			blobmsg_close_table(&status, freq);
		}
		if (res.data.lte_info_neighboring_wcdma.frequency_n > 0)
			blobmsg_close_table(&status, c);
	}
	if (res.set.umts_info_neighboring_lte) {
		if (res.data.umts_info_neighboring_lte.frequency_n > 0)
			c = blobmsg_open_table(&status, "umts_info_neighboring_lte");
		for (i = 0; i < res.data.umts_info_neighboring_lte.frequency_n; i++) {
			freq = blobmsg_open_table(&status, NULL);
			blobmsg_add_u32(&status, "channel",
					res.data.umts_info_neighboring_lte.frequency[i].eutra_absolute_rf_channel_number);
			print_earfcn_info(res.data.umts_info_neighboring_lte.frequency[i].eutra_absolute_rf_channel_number);
			blobmsg_add_u32(&status, "physical_cell_id",
					res.data.umts_info_neighboring_lte.frequency[i].physical_cell_id);
			blobmsg_add_double(&status, "rsrp",
					   (double) res.data.umts_info_neighboring_lte.frequency[i].rsrp);
			blobmsg_add_double(&status, "rsrq",
					   (double) res.data.umts_info_neighboring_lte.frequency[i].rsrq);
			blobmsg_add_u32(&status, "cell_selection_rx_level",
					res.data.umts_info_neighboring_lte.frequency[i].cell_selection_rx_level);
			blobmsg_close_table(&status, freq);
		}
		if (res.data.umts_info_neighboring_lte.frequency_n > 0)
			blobmsg_close_table(&status, c);
	}
	if (res.set.nr5g_cell_information) {
		c = blobmsg_open_table(&status, "nr5g_cell_information");
		blobmsg_add_u32(&status, "enodeb_id",
				res.data.nr5g_cell_information.global_cell_id/256);
		blobmsg_add_u32(&status, "cell_id",
				res.data.nr5g_cell_information.global_cell_id%256);
		blobmsg_add_u32(&status, "physical_cell_id",
				res.data.nr5g_cell_information.physical_cell_id);
		blobmsg_add_double(&status, "rsrq", ((double)res.data.nr5g_cell_information.rsrq)/10);
		blobmsg_add_double(&status, "rsrp", ((double)res.data.nr5g_cell_information.rsrp)/10);
		blobmsg_add_double(&status, "snr", ((double)res.data.nr5g_cell_information.snr)/10);
		blobmsg_close_table(&status, c);
	}
	if (res.set.nr5g_arfcn) {
		c = blobmsg_open_table(&status, "nr5g_arfcn");
		blobmsg_add_u32(&status, "arfcn",
				res.data.nr5g_arfcn);
		blobmsg_close_table(&status, c);
	}
	blobmsg_close_table(&status, t);
}

static enum qmi_cmd_result
cmd_nas_get_cell_location_info_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_get_cell_location_info_request(msg);
	return QMI_CMD_REQUEST;
}

static enum qmi_cmd_result
cmd_nas_get_signal_info_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_get_signal_info_request(msg);
	return QMI_CMD_REQUEST;
}

static void
cmd_nas_get_serving_system_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_nas_get_serving_system_response res;
	static const char *reg_states[] = {
		[QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED] = "not_registered",
		[QMI_NAS_REGISTRATION_STATE_REGISTERED] = "registered",
		[QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED_SEARCHING] = "searching",
		[QMI_NAS_REGISTRATION_STATE_REGISTRATION_DENIED] = "registering_denied",
		[QMI_NAS_REGISTRATION_STATE_UNKNOWN] = "unknown",
	};
	void *c, *a;

	qmi_parse_nas_get_serving_system_response(msg, &res);

	c = blobmsg_open_table(&status, NULL);
	if (res.set.serving_system) {
		int state = res.data.serving_system.registration_state;

		if (state > QMI_NAS_REGISTRATION_STATE_UNKNOWN)
			state = QMI_NAS_REGISTRATION_STATE_UNKNOWN;

		blobmsg_add_string(&status, "registration", reg_states[state]);

		a = blobmsg_open_array(&status, "radio_interface");
		for (int i = 0; i < res.data.serving_system.radio_interfaces_n; i++) {
			int8_t r_i = res.data.serving_system.radio_interfaces[i];

			blobmsg_add_string(&status, "radio", print_radio_interface(r_i));
		}
		blobmsg_close_array(&status, a);
	}
	if (res.set.current_plmn) {
		blobmsg_add_u32(&status, "plmn_mcc", res.data.current_plmn.mcc);
		blobmsg_add_u32(&status, "plmn_mnc", res.data.current_plmn.mnc);
		if (res.data.current_plmn.description)
			blobmsg_add_string(&status, "plmn_description", res.data.current_plmn.description);
	}

	if (res.set.roaming_indicator)
		blobmsg_add_u8(&status, "roaming", !res.data.roaming_indicator);

	blobmsg_close_table(&status, c);
}

static enum qmi_cmd_result
cmd_nas_get_serving_system_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_get_serving_system_request(msg);
	return QMI_CMD_REQUEST;
}

static void
cmd_nas_get_plmn_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_nas_get_system_selection_preference_response res;
	static const char *modes[] = {
		[QMI_NAS_NETWORK_SELECTION_PREFERENCE_AUTOMATIC] = "automatic",
		[QMI_NAS_NETWORK_SELECTION_PREFERENCE_MANUAL] = "manual",
	};
	void *c;

	qmi_parse_nas_get_system_selection_preference_response(msg, &res);

	c = blobmsg_open_table(&status, NULL);
	if (res.set.network_selection_preference) {
		blobmsg_add_string(&status, "mode", modes[res.data.network_selection_preference]);
	}
	if (res.set.manual_network_selection) {
		blobmsg_add_u32(&status, "mcc", res.data.manual_network_selection.mcc);
		blobmsg_add_u32(&status, "mnc", res.data.manual_network_selection.mnc);
	}

	blobmsg_close_table(&status, c);
}

static enum qmi_cmd_result
cmd_nas_get_plmn_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_get_system_selection_preference_request(msg);
	return QMI_CMD_REQUEST;
}

static void
cmd_nas_network_scan_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	static struct qmi_nas_network_scan_response res;
	const char *network_status[] = {
		"current_serving",
		"available",
		"home",
		"roaming",
		"forbidden",
		"not_forbidden",
		"preferred",
		"not_preferred",
	};
	void *t, *c, *info, *stat;
	int i, j;

	qmi_parse_nas_network_scan_response(msg, &res);

	t = blobmsg_open_table(&status, NULL);

	c = blobmsg_open_array(&status, "network_info");
	for (i = 0; i < res.data.network_information_n; i++) {
		info = blobmsg_open_table(&status, NULL);
		blobmsg_add_u32(&status, "mcc", res.data.network_information[i].mcc);
		blobmsg_add_u32(&status, "mnc", res.data.network_information[i].mnc);
		if (res.data.network_information[i].description)
			blobmsg_add_string(&status, "description", res.data.network_information[i].description);
		stat = blobmsg_open_array(&status, "status");
		for (j = 0; j < ARRAY_SIZE(network_status); j++) {
			if (!(res.data.network_information[i].network_status & (1 << j)))
				continue;

			blobmsg_add_string(&status, NULL, network_status[j]);
		}
		blobmsg_close_array(&status, stat);
		blobmsg_close_table(&status, info);
	}
	blobmsg_close_array(&status, c);

	c = blobmsg_open_array(&status, "radio_access_technology");
	for (i = 0; i < res.data.radio_access_technology_n; i++) {
		int8_t r_i = res.data.radio_access_technology[i].radio_interface;

		info = blobmsg_open_table(&status, NULL);
		blobmsg_add_u32(&status, "mcc", res.data.radio_access_technology[i].mcc);
		blobmsg_add_u32(&status, "mnc", res.data.radio_access_technology[i].mnc);
		blobmsg_add_string(&status, "radio", print_radio_interface(r_i));
		blobmsg_close_table(&status, info);
	}
	blobmsg_close_array(&status, c);

	blobmsg_close_table(&status, t);
}

static enum qmi_cmd_result
cmd_nas_network_scan_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_nas_network_scan_request sreq = {
		QMI_INIT(network_type,
	             QMI_NAS_NETWORK_SCAN_TYPE_GSM |
	             QMI_NAS_NETWORK_SCAN_TYPE_UMTS |
	             QMI_NAS_NETWORK_SCAN_TYPE_LTE |
	             QMI_NAS_NETWORK_SCAN_TYPE_TD_SCDMA),
	};

	qmi_set_nas_network_scan_request(msg, &sreq);
	return QMI_CMD_REQUEST;
}
