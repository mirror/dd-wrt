struct qmi_pds_set_event_report_request {
	struct {
		unsigned int nmea_position_reporting : 1;
		unsigned int extended_nmea_position_reporting : 1;
		unsigned int parsed_position_reporting : 1;
		unsigned int external_xtra_data_request_reporting : 1;
		unsigned int external_time_injection_request_reporting : 1;
		unsigned int external_wifi_position_request_reporting : 1;
		unsigned int satellite_information_reporting : 1;
		unsigned int vx_network_initiated_request_reporting : 1;
		unsigned int supl_network_initiated_prompt_reporting : 1;
		unsigned int umts_cp_network_initiated_prompt_reporting : 1;
		unsigned int pds_comm_event_reporting : 1;
		unsigned int accelerometer_data_streaming_ready_reporting : 1;
		unsigned int gyro_data_streaming_ready_reporting : 1;
		unsigned int time_sync_request_reporting : 1;
		unsigned int position_reliability_indicator_reporting : 1;
		unsigned int sensor_data_usage_indicator_reporting : 1;
		unsigned int time_source_information_reporting : 1;
		unsigned int heading_uncertainty_reporting : 1;
		unsigned int nmea_debug_strings_reporting : 1;
		unsigned int extended_external_xtra_data_request_reporting : 1;
	} set;
	struct {
		bool nmea_position_reporting;
		bool extended_nmea_position_reporting;
		bool parsed_position_reporting;
		bool external_xtra_data_request_reporting;
		bool external_time_injection_request_reporting;
		bool external_wifi_position_request_reporting;
		bool satellite_information_reporting;
		bool vx_network_initiated_request_reporting;
		bool supl_network_initiated_prompt_reporting;
		bool umts_cp_network_initiated_prompt_reporting;
		bool pds_comm_event_reporting;
		bool accelerometer_data_streaming_ready_reporting;
		bool gyro_data_streaming_ready_reporting;
		bool time_sync_request_reporting;
		bool position_reliability_indicator_reporting;
		bool sensor_data_usage_indicator_reporting;
		bool time_source_information_reporting;
		bool heading_uncertainty_reporting;
		bool nmea_debug_strings_reporting;
		bool extended_external_xtra_data_request_reporting;
	} data;
};

struct qmi_pds_get_gps_service_state_response {
	struct {
		unsigned int state : 1;
	} set;
	struct {
		struct {
			bool gps_service_state;
			QmiPdsTrackingSessionState tracking_session_state;
		} state;
	} data;
};

struct qmi_pds_set_gps_service_state_request {
	struct {
		unsigned int state : 1;
	} set;
	struct {
		struct {
			bool gps_service_state;
		} state;
	} data;
};

struct qmi_pds_get_auto_tracking_state_response {
	struct {
		unsigned int state : 1;
	} set;
	struct {
		struct {
			bool auto_tracking_state;
		} state;
	} data;
};

struct qmi_pds_set_auto_tracking_state_request {
	struct {
		unsigned int state : 1;
	} set;
	struct {
		struct {
			bool auto_tracking_state;
		} state;
	} data;
};

int qmi_set_pds_reset_request(struct qmi_msg *msg);
int qmi_parse_pds_reset_response(struct qmi_msg *msg);

int qmi_set_pds_set_event_report_request(struct qmi_msg *msg, struct qmi_pds_set_event_report_request *req);
int qmi_parse_pds_set_event_report_response(struct qmi_msg *msg);

int qmi_set_pds_get_gps_service_state_request(struct qmi_msg *msg);
int qmi_parse_pds_get_gps_service_state_response(struct qmi_msg *msg, struct qmi_pds_get_gps_service_state_response *res);

int qmi_set_pds_set_gps_service_state_request(struct qmi_msg *msg, struct qmi_pds_set_gps_service_state_request *req);
int qmi_parse_pds_set_gps_service_state_response(struct qmi_msg *msg);

int qmi_set_pds_get_auto_tracking_state_request(struct qmi_msg *msg);
int qmi_parse_pds_get_auto_tracking_state_response(struct qmi_msg *msg, struct qmi_pds_get_auto_tracking_state_response *res);

int qmi_set_pds_set_auto_tracking_state_request(struct qmi_msg *msg, struct qmi_pds_set_auto_tracking_state_request *req);
int qmi_parse_pds_set_auto_tracking_state_response(struct qmi_msg *msg);

