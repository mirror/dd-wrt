struct qmi_wda_get_supported_messages_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		uint8_t *list;
	} data;
};

struct qmi_wda_set_data_format_request {
	struct {
		unsigned int qos_format : 1;
		unsigned int link_layer_protocol : 1;
		unsigned int uplink_data_aggregation_protocol : 1;
		unsigned int downlink_data_aggregation_protocol : 1;
		unsigned int ndp_signature : 1;
		unsigned int downlink_data_aggregation_max_datagrams : 1;
		unsigned int downlink_data_aggregation_max_size : 1;
		unsigned int endpoint_info : 1;
	} set;
	struct {
		bool qos_format;
		uint32_t link_layer_protocol;
		uint32_t uplink_data_aggregation_protocol;
		uint32_t downlink_data_aggregation_protocol;
		uint32_t ndp_signature;
		uint32_t downlink_data_aggregation_max_datagrams;
		uint32_t downlink_data_aggregation_max_size;
		struct {
			uint32_t endpoint_type;
			uint32_t interface_number;
		} endpoint_info;
	} data;
};

struct qmi_wda_set_data_format_response {
	struct {
		unsigned int qos_format : 1;
		unsigned int link_layer_protocol : 1;
		unsigned int uplink_data_aggregation_protocol : 1;
		unsigned int downlink_data_aggregation_protocol : 1;
		unsigned int ndp_signature : 1;
		unsigned int downlink_data_aggregation_max_datagrams : 1;
		unsigned int downlink_data_aggregation_max_size : 1;
		unsigned int uplink_data_aggregation_max_datagrams : 1;
		unsigned int uplink_data_aggregation_max_size : 1;
		unsigned int download_minimum_padding : 1;
		unsigned int flow_control : 1;
	} set;
	struct {
		bool qos_format;
		uint32_t link_layer_protocol;
		uint32_t uplink_data_aggregation_protocol;
		uint32_t downlink_data_aggregation_protocol;
		uint32_t ndp_signature;
		uint32_t downlink_data_aggregation_max_datagrams;
		uint32_t downlink_data_aggregation_max_size;
		uint32_t uplink_data_aggregation_max_datagrams;
		uint32_t uplink_data_aggregation_max_size;
		uint32_t download_minimum_padding;
		uint8_t flow_control;
	} data;
};

struct qmi_wda_get_data_format_request {
	struct {
		unsigned int endpoint_info : 1;
	} set;
	struct {
		struct {
			uint32_t endpoint_type;
			uint32_t interface_number;
		} endpoint_info;
	} data;
};

struct qmi_wda_get_data_format_response {
	struct {
		unsigned int qos_format : 1;
		unsigned int link_layer_protocol : 1;
		unsigned int uplink_data_aggregation_protocol : 1;
		unsigned int downlink_data_aggregation_protocol : 1;
		unsigned int ndp_signature : 1;
		unsigned int downlink_data_aggregation_max_datagrams : 1;
		unsigned int downlink_data_aggregation_max_size : 1;
		unsigned int uplink_data_aggregation_max_datagrams : 1;
		unsigned int uplink_data_aggregation_max_size : 1;
		unsigned int download_minimum_padding : 1;
		unsigned int flow_control : 1;
	} set;
	struct {
		bool qos_format;
		uint32_t link_layer_protocol;
		uint32_t uplink_data_aggregation_protocol;
		uint32_t downlink_data_aggregation_protocol;
		uint32_t ndp_signature;
		uint32_t downlink_data_aggregation_max_datagrams;
		uint32_t downlink_data_aggregation_max_size;
		uint32_t uplink_data_aggregation_max_datagrams;
		uint32_t uplink_data_aggregation_max_size;
		uint32_t download_minimum_padding;
		uint8_t flow_control;
	} data;
};

int qmi_set_wda_get_supported_messages_request(struct qmi_msg *msg);
int qmi_parse_wda_get_supported_messages_response(struct qmi_msg *msg, struct qmi_wda_get_supported_messages_response *res);

int qmi_set_wda_set_data_format_request(struct qmi_msg *msg, struct qmi_wda_set_data_format_request *req);
int qmi_parse_wda_set_data_format_response(struct qmi_msg *msg, struct qmi_wda_set_data_format_response *res);

int qmi_set_wda_get_data_format_request(struct qmi_msg *msg, struct qmi_wda_get_data_format_request *req);
int qmi_parse_wda_get_data_format_response(struct qmi_msg *msg, struct qmi_wda_get_data_format_response *res);

