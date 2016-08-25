struct qmi_wda_set_data_format_request {
	struct {
		unsigned int qos_format : 1;
		unsigned int link_layer_protocol : 1;
		unsigned int uplink_data_aggregation_protocol : 1;
		unsigned int downlink_data_aggregation_protocol : 1;
		unsigned int ndp_signature : 1;
		unsigned int downlink_data_aggregation_max_datagrams : 1;
		unsigned int downlink_data_aggregation_max_size : 1;
	} set;
	struct {
		bool qos_format;
		QmiWdaLinkLayerProtocol link_layer_protocol;
		QmiWdaDataAggregationProtocol uplink_data_aggregation_protocol;
		QmiWdaDataAggregationProtocol downlink_data_aggregation_protocol;
		uint32_t ndp_signature;
		uint32_t downlink_data_aggregation_max_datagrams;
		uint32_t downlink_data_aggregation_max_size;
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
	} set;
	struct {
		bool qos_format;
		QmiWdaLinkLayerProtocol link_layer_protocol;
		QmiWdaDataAggregationProtocol uplink_data_aggregation_protocol;
		QmiWdaDataAggregationProtocol downlink_data_aggregation_protocol;
		uint32_t ndp_signature;
		uint32_t downlink_data_aggregation_max_datagrams;
		uint32_t downlink_data_aggregation_max_size;
	} data;
};

struct qmi_wda_get_data_format_response {
	struct {
		unsigned int qos_format : 1;
		unsigned int link_layer_protocol : 1;
		unsigned int uplink_data_aggregation_protocol : 1;
		unsigned int downlink_data_aggregation_protocol : 1;
		unsigned int ndp_signature : 1;
		unsigned int uplink_data_aggregation_max_size : 1;
		unsigned int downlink_data_aggregation_max_size : 1;
	} set;
	struct {
		bool qos_format;
		QmiWdaLinkLayerProtocol link_layer_protocol;
		QmiWdaDataAggregationProtocol uplink_data_aggregation_protocol;
		QmiWdaDataAggregationProtocol downlink_data_aggregation_protocol;
		uint32_t ndp_signature;
		uint32_t uplink_data_aggregation_max_size;
		uint32_t downlink_data_aggregation_max_size;
	} data;
};

int qmi_set_wda_set_data_format_request(struct qmi_msg *msg, struct qmi_wda_set_data_format_request *req);
int qmi_parse_wda_set_data_format_response(struct qmi_msg *msg, struct qmi_wda_set_data_format_response *res);

int qmi_set_wda_get_data_format_request(struct qmi_msg *msg);
int qmi_parse_wda_get_data_format_response(struct qmi_msg *msg, struct qmi_wda_get_data_format_response *res);

