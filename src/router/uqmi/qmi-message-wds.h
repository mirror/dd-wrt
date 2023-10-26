struct qmi_wds_set_event_report_request {
	struct {
		unsigned int channel_rate : 1;
		unsigned int transfer_statistics : 1;
		unsigned int data_bearer_technology : 1;
		unsigned int dormancy_status : 1;
		unsigned int mip_status : 1;
		unsigned int current_data_bearer_technology : 1;
		unsigned int data_call_status : 1;
		unsigned int preferred_data_system : 1;
		unsigned int evdo_pm_change : 1;
		unsigned int data_systems : 1;
		unsigned int uplink_flow_control : 1;
		unsigned int limited_data_system_status : 1;
		unsigned int pdn_filter_removals : 1;
		unsigned int extended_data_bearer_technology : 1;
	} set;
	struct {
		bool channel_rate;
		struct {
			uint8_t interval_seconds;
			int32_t indicators;
		} transfer_statistics;
		bool data_bearer_technology;
		bool dormancy_status;
		uint8_t mip_status;
		bool current_data_bearer_technology;
		bool data_call_status;
		bool preferred_data_system;
		bool evdo_pm_change;
		bool data_systems;
		bool uplink_flow_control;
		bool limited_data_system_status;
		bool pdn_filter_removals;
		bool extended_data_bearer_technology;
	} data;
};

struct qmi_wds_abort_request {
	struct {
		unsigned int transaction_id : 1;
	} set;
	struct {
		uint16_t transaction_id;
	} data;
};

struct qmi_wds_get_supported_messages_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		uint8_t *list;
	} data;
};

struct qmi_wds_start_network_request {
	struct {
		unsigned int primary_dns_address_preference : 1;
		unsigned int secondary_dns_address_preference : 1;
		unsigned int primary_nbns_address_preference : 1;
		unsigned int secondary_nbns_address_preference : 1;
		unsigned int ipv4_address_preference : 1;
		unsigned int authentication_preference : 1;
		unsigned int ip_family_preference : 1;
		unsigned int technology_preference : 1;
		unsigned int profile_index_3gpp : 1;
		unsigned int profile_index_3gpp2 : 1;
		unsigned int enable_autoconnect : 1;
		unsigned int extended_technology_preference : 1;
		unsigned int call_type : 1;
	} set;
	struct {
		uint32_t primary_dns_address_preference;
		uint32_t secondary_dns_address_preference;
		uint32_t primary_nbns_address_preference;
		uint32_t secondary_nbns_address_preference;
		char *apn;
		uint32_t ipv4_address_preference;
		uint8_t authentication_preference;
		char *username;
		char *password;
		uint8_t ip_family_preference;
		uint8_t technology_preference;
		uint8_t profile_index_3gpp;
		uint8_t profile_index_3gpp2;
		bool enable_autoconnect;
		uint16_t extended_technology_preference;
		uint8_t call_type;
	} data;
};

struct qmi_wds_start_network_response {
	struct {
		unsigned int packet_data_handle : 1;
		unsigned int call_end_reason : 1;
		unsigned int verbose_call_end_reason : 1;
	} set;
	struct {
		uint32_t packet_data_handle;
		uint16_t call_end_reason;
		struct {
			uint16_t type;
			int16_t reason;
		} verbose_call_end_reason;
	} data;
};

struct qmi_wds_stop_network_request {
	struct {
		unsigned int packet_data_handle : 1;
		unsigned int disable_autoconnect : 1;
	} set;
	struct {
		uint32_t packet_data_handle;
		bool disable_autoconnect;
	} data;
};

struct qmi_wds_get_packet_service_status_response {
	struct {
		unsigned int connection_status : 1;
	} set;
	struct {
		uint8_t connection_status;
	} data;
};

struct qmi_wds_get_channel_rates_response {
	struct {
		unsigned int channel_rates : 1;
	} set;
	struct {
		struct {
			uint32_t channel_tx_rate_bps;
			uint32_t channel_rx_rate_bps;
			uint32_t max_channel_tx_rate_bps;
			uint32_t max_channel_rx_rate_bps;
		} channel_rates;
	} data;
};

struct qmi_wds_get_packet_statistics_request {
	struct {
		unsigned int mask : 1;
	} set;
	struct {
		uint32_t mask;
	} data;
};

struct qmi_wds_get_packet_statistics_response {
	struct {
		unsigned int tx_packets_ok : 1;
		unsigned int rx_packets_ok : 1;
		unsigned int tx_packets_error : 1;
		unsigned int rx_packets_error : 1;
		unsigned int tx_overflows : 1;
		unsigned int rx_overflows : 1;
		unsigned int tx_bytes_ok : 1;
		unsigned int rx_bytes_ok : 1;
		unsigned int last_call_tx_bytes_ok : 1;
		unsigned int last_call_rx_bytes_ok : 1;
		unsigned int tx_packets_dropped : 1;
		unsigned int rx_packets_dropped : 1;
	} set;
	struct {
		uint32_t tx_packets_ok;
		uint32_t rx_packets_ok;
		uint32_t tx_packets_error;
		uint32_t rx_packets_error;
		uint32_t tx_overflows;
		uint32_t rx_overflows;
		uint64_t tx_bytes_ok;
		uint64_t rx_bytes_ok;
		uint64_t last_call_tx_bytes_ok;
		uint64_t last_call_rx_bytes_ok;
		uint32_t tx_packets_dropped;
		uint32_t rx_packets_dropped;
	} data;
};

struct qmi_wds_create_profile_request {
	struct {
		unsigned int profile_type : 1;
		unsigned int pdp_type : 1;
		unsigned int pdp_header_compression_type : 1;
		unsigned int pdp_data_compression_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
		unsigned int umts_requested_qos : 1;
		unsigned int umts_minimum_qos : 1;
		unsigned int gprs_requested_qos : 1;
		unsigned int gprs_minimum_qos : 1;
		unsigned int authentication : 1;
		unsigned int ipv4_address_preference : 1;
		unsigned int pcscf_address_using_pco : 1;
		unsigned int pcscf_address_using_dhcp : 1;
		unsigned int imcn_flag : 1;
		unsigned int pdp_context_number : 1;
		unsigned int pdp_context_secondary_flag : 1;
		unsigned int pdp_context_primary_id : 1;
		unsigned int ipv6_address_preference : 1;
		unsigned int umts_requested_qos_with_signaling_indication_flag : 1;
		unsigned int umts_minimum_qos_with_signaling_indication_flag : 1;
		unsigned int ipv6_primary_dns_address_preference : 1;
		unsigned int ipv6_secondary_dns_address_preference : 1;
		unsigned int lte_qos_parameters : 1;
		unsigned int apn_disabled_flag : 1;
		unsigned int roaming_disallowed_flag : 1;
		unsigned int apn_type_mask : 1;
	} set;
	struct {
		uint8_t profile_type;
		char *profile_name;
		uint8_t pdp_type;
		uint8_t pdp_header_compression_type;
		uint8_t pdp_data_compression_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_requested_qos;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_minimum_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_requested_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_minimum_qos;
		char *username;
		char *password;
		uint8_t authentication;
		uint32_t ipv4_address_preference;
		bool pcscf_address_using_pco;
		bool pcscf_address_using_dhcp;
		bool imcn_flag;
		uint8_t pdp_context_number;
		bool pdp_context_secondary_flag;
		uint8_t pdp_context_primary_id;
		struct {
			uint16_t address[8];
		} ipv6_address_preference;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
			int8_t signaling_indication;
		} umts_requested_qos_with_signaling_indication_flag;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
			int8_t signaling_indication;
		} umts_minimum_qos_with_signaling_indication_flag;
		uint16_t ipv6_primary_dns_address_preference[8];
		uint16_t ipv6_secondary_dns_address_preference[8];
		struct {
			uint8_t qos_class_identifier;
			uint32_t guaranteed_downlink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t max_uplink_bitrate;
		} lte_qos_parameters;
		bool apn_disabled_flag;
		bool roaming_disallowed_flag;
		uint64_t apn_type_mask;
	} data;
};

struct qmi_wds_create_profile_response {
	struct {
		unsigned int profile_identifier : 1;
		unsigned int extended_error_code : 1;
	} set;
	struct {
		struct {
			uint8_t profile_type;
			uint8_t profile_index;
		} profile_identifier;
		uint16_t extended_error_code;
	} data;
};

struct qmi_wds_modify_profile_request {
	struct {
		unsigned int profile_identifier : 1;
		unsigned int pdp_type : 1;
		unsigned int pdp_header_compression_type : 1;
		unsigned int pdp_data_compression_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
		unsigned int umts_requested_qos : 1;
		unsigned int umts_minimum_qos : 1;
		unsigned int gprs_requested_qos : 1;
		unsigned int gprs_minimum_qos : 1;
		unsigned int authentication : 1;
		unsigned int ipv4_address_preference : 1;
		unsigned int pcscf_address_using_pco : 1;
		unsigned int pcscf_address_using_dhcp : 1;
		unsigned int imcn_flag : 1;
		unsigned int pdp_context_number : 1;
		unsigned int pdp_context_secondary_flag : 1;
		unsigned int pdp_context_primary_id : 1;
		unsigned int ipv6_address_preference : 1;
		unsigned int umts_requested_qos_with_signaling_indication_flag : 1;
		unsigned int umts_minimum_qos_with_signaling_indication_flag : 1;
		unsigned int ipv6_primary_dns_address_preference : 1;
		unsigned int ipv6_secondary_dns_address_preference : 1;
		unsigned int lte_qos_parameters : 1;
		unsigned int apn_disabled_flag : 1;
		unsigned int roaming_disallowed_flag : 1;
		unsigned int apn_type_mask : 1;
	} set;
	struct {
		struct {
			uint8_t profile_type;
			uint8_t profile_index;
		} profile_identifier;
		char *profile_name;
		uint8_t pdp_type;
		uint8_t pdp_header_compression_type;
		uint8_t pdp_data_compression_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_requested_qos;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_minimum_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_requested_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_minimum_qos;
		char *username;
		char *password;
		uint8_t authentication;
		uint32_t ipv4_address_preference;
		bool pcscf_address_using_pco;
		bool pcscf_address_using_dhcp;
		bool imcn_flag;
		uint8_t pdp_context_number;
		bool pdp_context_secondary_flag;
		uint8_t pdp_context_primary_id;
		struct {
			uint16_t address[8];
		} ipv6_address_preference;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
			int8_t signaling_indication;
		} umts_requested_qos_with_signaling_indication_flag;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
			int8_t signaling_indication;
		} umts_minimum_qos_with_signaling_indication_flag;
		uint16_t ipv6_primary_dns_address_preference[8];
		uint16_t ipv6_secondary_dns_address_preference[8];
		struct {
			uint8_t qos_class_identifier;
			uint32_t guaranteed_downlink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t max_uplink_bitrate;
		} lte_qos_parameters;
		bool apn_disabled_flag;
		bool roaming_disallowed_flag;
		uint64_t apn_type_mask;
	} data;
};

struct qmi_wds_modify_profile_response {
	struct {
		unsigned int extended_error_code : 1;
	} set;
	struct {
		uint16_t extended_error_code;
	} data;
};

struct qmi_wds_delete_profile_request {
	struct {
		unsigned int profile_identifier : 1;
	} set;
	struct {
		struct {
			uint8_t profile_type;
			uint8_t profile_index;
		} profile_identifier;
	} data;
};

struct qmi_wds_delete_profile_response {
	struct {
		unsigned int extended_error_code : 1;
	} set;
	struct {
		uint16_t extended_error_code;
	} data;
};

struct qmi_wds_get_profile_list_request {
	struct {
		unsigned int profile_type : 1;
	} set;
	struct {
		uint8_t profile_type;
	} data;
};

struct qmi_wds_get_profile_list_response {
	struct {
		unsigned int extended_error_code : 1;
	} set;
	struct {
		unsigned int profile_list_n;
		struct {
			uint8_t profile_type;
			uint8_t profile_index;
			char *profile_name;
		} *profile_list;
		uint16_t extended_error_code;
	} data;
};

struct qmi_wds_get_profile_settings_request {
	struct {
		unsigned int profile_id : 1;
	} set;
	struct {
		struct {
			uint8_t profile_type;
			uint8_t profile_index;
		} profile_id;
	} data;
};

struct qmi_wds_get_profile_settings_response {
	struct {
		unsigned int pdp_type : 1;
		unsigned int pdp_header_compression_type : 1;
		unsigned int pdp_data_compression_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
		unsigned int umts_requested_qos : 1;
		unsigned int umts_minimum_qos : 1;
		unsigned int gprs_requested_qos : 1;
		unsigned int gprs_minimum_qos : 1;
		unsigned int authentication : 1;
		unsigned int ipv4_address_preference : 1;
		unsigned int pcscf_address_using_pco : 1;
		unsigned int pcscf_address_using_dhcp : 1;
		unsigned int imcn_flag : 1;
		unsigned int pdp_context_number : 1;
		unsigned int pdp_context_secondary_flag : 1;
		unsigned int pdp_context_primary_id : 1;
		unsigned int ipv6_address_preference : 1;
		unsigned int umts_requested_qos_with_signaling_indication_flag : 1;
		unsigned int umts_minimum_qos_with_signaling_indication_flag : 1;
		unsigned int ipv6_primary_dns_address_preference : 1;
		unsigned int ipv6_secondary_dns_address_preference : 1;
		unsigned int lte_qos_parameters : 1;
		unsigned int apn_disabled_flag : 1;
		unsigned int roaming_disallowed_flag : 1;
		unsigned int apn_type_mask : 1;
		unsigned int extended_error_code : 1;
	} set;
	struct {
		char *profile_name;
		uint8_t pdp_type;
		uint8_t pdp_header_compression_type;
		uint8_t pdp_data_compression_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_requested_qos;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_minimum_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_requested_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_minimum_qos;
		char *username;
		char *password;
		uint8_t authentication;
		uint32_t ipv4_address_preference;
		bool pcscf_address_using_pco;
		bool pcscf_address_using_dhcp;
		bool imcn_flag;
		uint8_t pdp_context_number;
		bool pdp_context_secondary_flag;
		uint8_t pdp_context_primary_id;
		struct {
			uint16_t address[8];
		} ipv6_address_preference;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
			int8_t signaling_indication;
		} umts_requested_qos_with_signaling_indication_flag;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
			int8_t signaling_indication;
		} umts_minimum_qos_with_signaling_indication_flag;
		uint16_t ipv6_primary_dns_address_preference[8];
		uint16_t ipv6_secondary_dns_address_preference[8];
		struct {
			uint8_t qos_class_identifier;
			uint32_t guaranteed_downlink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t max_uplink_bitrate;
		} lte_qos_parameters;
		bool apn_disabled_flag;
		bool roaming_disallowed_flag;
		uint64_t apn_type_mask;
		uint16_t extended_error_code;
	} data;
};

struct qmi_wds_get_default_settings_request {
	struct {
		unsigned int profile_type : 1;
	} set;
	struct {
		uint8_t profile_type;
	} data;
};

struct qmi_wds_get_default_settings_response {
	struct {
		unsigned int pdp_type : 1;
		unsigned int pdp_header_compression_type : 1;
		unsigned int pdp_data_compression_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
		unsigned int umts_requested_qos : 1;
		unsigned int umts_minimum_qos : 1;
		unsigned int gprs_requested_qos : 1;
		unsigned int gprs_minimum_qos : 1;
		unsigned int authentication : 1;
		unsigned int ipv4_address_preference : 1;
		unsigned int pcscf_address_using_pco : 1;
		unsigned int pcscf_address_using_dhcp : 1;
		unsigned int imcn_flag : 1;
		unsigned int pdp_context_number : 1;
		unsigned int pdp_context_secondary_flag : 1;
		unsigned int pdp_context_primary_id : 1;
		unsigned int ipv6_address_preference : 1;
		unsigned int umts_requested_qos_with_signaling_indication_flag : 1;
		unsigned int umts_minimum_qos_with_signaling_indication_flag : 1;
		unsigned int ipv6_primary_dns_address_preference : 1;
		unsigned int ipv6_secondary_dns_address_preference : 1;
		unsigned int lte_qos_parameters : 1;
		unsigned int extended_error_code : 1;
	} set;
	struct {
		char *profile_name;
		uint8_t pdp_type;
		uint8_t pdp_header_compression_type;
		uint8_t pdp_data_compression_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_requested_qos;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_minimum_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_requested_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_minimum_qos;
		char *username;
		char *password;
		uint8_t authentication;
		uint32_t ipv4_address_preference;
		bool pcscf_address_using_pco;
		bool pcscf_address_using_dhcp;
		bool imcn_flag;
		uint8_t pdp_context_number;
		bool pdp_context_secondary_flag;
		uint8_t pdp_context_primary_id;
		struct {
			uint16_t address[8];
		} ipv6_address_preference;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
			int8_t signaling_indication;
		} umts_requested_qos_with_signaling_indication_flag;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
			int8_t signaling_indication;
		} umts_minimum_qos_with_signaling_indication_flag;
		uint16_t ipv6_primary_dns_address_preference[8];
		uint16_t ipv6_secondary_dns_address_preference[8];
		struct {
			uint8_t qos_class_identifier;
			uint32_t guaranteed_downlink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t max_uplink_bitrate;
		} lte_qos_parameters;
		uint16_t extended_error_code;
	} data;
};

struct qmi_wds_get_current_settings_request {
	struct {
		unsigned int requested_settings : 1;
	} set;
	struct {
		uint32_t requested_settings;
	} data;
};

struct qmi_wds_get_current_settings_response {
	struct {
		unsigned int pdp_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
		unsigned int umts_granted_qos : 1;
		unsigned int gprs_granted_qos : 1;
		unsigned int authentication : 1;
		unsigned int ipv4_address : 1;
		unsigned int profile_id : 1;
		unsigned int ipv4_gateway_address : 1;
		unsigned int ipv4_gateway_subnet_mask : 1;
		unsigned int pcscf_address_using_pco : 1;
		unsigned int ipv6_address : 1;
		unsigned int ipv6_gateway_address : 1;
		unsigned int ipv6_primary_dns_address : 1;
		unsigned int ipv6_secondary_dns_address : 1;
		unsigned int mtu : 1;
		unsigned int ip_family : 1;
		unsigned int imcn_flag : 1;
		unsigned int extended_technology_preference : 1;
	} set;
	struct {
		char *profile_name;
		uint8_t pdp_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
		struct {
			uint8_t traffic_class;
			uint32_t max_uplink_bitrate;
			uint32_t max_downlink_bitrate;
			uint32_t guaranteed_uplink_bitrate;
			uint32_t guaranteed_downlink_bitrate;
			uint8_t qos_delivery_order;
			uint32_t maximum_sdu_size;
			uint8_t sdu_error_ratio;
			uint8_t residual_bit_error_ratio;
			uint8_t delivery_erroneous_sdu;
			uint32_t transfer_delay;
			uint32_t traffic_handling_priority;
		} umts_granted_qos;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_granted_qos;
		char *username;
		uint8_t authentication;
		uint32_t ipv4_address;
		struct {
			uint8_t profile_type;
			uint8_t profile_index;
		} profile_id;
		uint32_t ipv4_gateway_address;
		uint32_t ipv4_gateway_subnet_mask;
		uint8_t pcscf_address_using_pco;
		unsigned int pcscf_server_address_list_n;
		uint32_t *pcscf_server_address_list;
		unsigned int pcscf_domain_name_list_n;
		char **pcscf_domain_name_list;
		struct {
			uint16_t address[8];
			uint8_t prefix_length;
		} ipv6_address;
		struct {
			uint16_t address[8];
			uint8_t prefix_length;
		} ipv6_gateway_address;
		uint16_t ipv6_primary_dns_address[8];
		uint16_t ipv6_secondary_dns_address[8];
		uint32_t mtu;
		unsigned int domain_name_list_n;
		char **domain_name_list;
		uint8_t ip_family;
		int8_t imcn_flag;
		uint16_t extended_technology_preference;
	} data;
};

struct qmi_wds_get_dormancy_status_response {
	struct {
		unsigned int dormancy_status : 1;
	} set;
	struct {
		uint8_t dormancy_status;
	} data;
};

struct qmi_wds_get_autoconnect_settings_response {
	struct {
		unsigned int status : 1;
		unsigned int roaming : 1;
	} set;
	struct {
		uint8_t status;
		uint8_t roaming;
	} data;
};

struct qmi_wds_get_data_bearer_technology_response {
	struct {
		unsigned int current : 1;
		unsigned int last : 1;
	} set;
	struct {
		int8_t current;
		int8_t last;
	} data;
};

struct qmi_wds_get_current_data_bearer_technology_response {
	struct {
		unsigned int current : 1;
		unsigned int last : 1;
	} set;
	struct {
		struct {
			uint8_t network_type;
			uint32_t rat_mask;
			uint32_t so_mask;
		} current;
		struct {
			uint8_t network_type;
			uint32_t rat_mask;
			uint32_t so_mask;
		} last;
	} data;
};

struct qmi_wds_get_default_profile_number_request {
	struct {
		unsigned int profile_type : 1;
	} set;
	struct {
		struct {
			uint8_t type;
			uint8_t family;
		} profile_type;
	} data;
};

struct qmi_wds_get_default_profile_number_response {
	struct {
		unsigned int index : 1;
		unsigned int extended_error_code : 1;
	} set;
	struct {
		uint8_t index;
		uint16_t extended_error_code;
	} data;
};

struct qmi_wds_set_default_profile_number_request {
	struct {
		unsigned int profile_identifier : 1;
	} set;
	struct {
		struct {
			uint8_t type;
			uint8_t family;
			uint8_t index;
		} profile_identifier;
	} data;
};

struct qmi_wds_set_default_profile_number_response {
	struct {
		unsigned int extended_error_code : 1;
	} set;
	struct {
		uint16_t extended_error_code;
	} data;
};

struct qmi_wds_set_ip_family_request {
	struct {
		unsigned int preference : 1;
	} set;
	struct {
		uint8_t preference;
	} data;
};

struct qmi_wds_set_autoconnect_settings_request {
	struct {
		unsigned int status : 1;
		unsigned int roaming : 1;
	} set;
	struct {
		uint8_t status;
		uint8_t roaming;
	} data;
};

struct qmi_wds_get_pdn_throttle_info_request {
	struct {
		unsigned int network_type : 1;
	} set;
	struct {
		uint8_t network_type;
	} data;
};

struct qmi_wds_get_pdn_throttle_info_response {
	struct {
	} set;
	struct {
		unsigned int info_n;
		struct {
			bool ipv4_throttled;
			bool ipv6_throttled;
			uint32_t ipv4_throttle_time_left_ms;
			uint32_t ipv6_throttle_time_left_ms;
			char *apn;
		} *info;
	} data;
};

struct qmi_wds_get_lte_attach_parameters_response {
	struct {
		unsigned int ip_support_type : 1;
		unsigned int ota_attach_performed : 1;
	} set;
	struct {
		char *apn;
		uint8_t ip_support_type;
		bool ota_attach_performed;
	} data;
};

struct qmi_wds_bind_data_port_request {
	struct {
		unsigned int data_port : 1;
	} set;
	struct {
		uint16_t data_port;
	} data;
};

struct qmi_wds_get_max_lte_attach_pdn_number_response {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		uint8_t info;
	} data;
};

struct qmi_wds_set_lte_attach_pdn_list_request {
	struct {
		unsigned int action : 1;
	} set;
	struct {
		unsigned int list_n;
		uint16_t *list;
		uint32_t action;
	} data;
};

struct qmi_wds_get_lte_attach_pdn_list_response {
	struct {
	} set;
	struct {
		unsigned int current_list_n;
		uint16_t *current_list;
		unsigned int pending_list_n;
		uint16_t *pending_list;
	} data;
};

struct qmi_wds_bind_mux_data_port_request {
	struct {
		unsigned int endpoint_info : 1;
		unsigned int mux_id : 1;
		unsigned int client_type : 1;
	} set;
	struct {
		struct {
			uint32_t endpoint_type;
			uint32_t interface_number;
		} endpoint_info;
		uint8_t mux_id;
		uint32_t client_type;
	} data;
};

struct qmi_wds_swi_create_profile_indexed_request {
	struct {
		unsigned int profile_identifier : 1;
		unsigned int pdp_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
		unsigned int authentication : 1;
		unsigned int ipv4_address_preference : 1;
		unsigned int pdp_context_number : 1;
		unsigned int apn_disabled_flag : 1;
		unsigned int roaming_disallowed_flag : 1;
	} set;
	struct {
		struct {
			uint8_t profile_type;
			uint8_t profile_index;
		} profile_identifier;
		char *profile_name;
		uint8_t pdp_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
		char *username;
		char *password;
		uint8_t authentication;
		uint32_t ipv4_address_preference;
		uint8_t pdp_context_number;
		bool apn_disabled_flag;
		bool roaming_disallowed_flag;
	} data;
};

struct qmi_wds_swi_create_profile_indexed_response {
	struct {
		unsigned int profile_identifier : 1;
	} set;
	struct {
		struct {
			uint8_t profile_type;
			uint8_t profile_index;
		} profile_identifier;
	} data;
};

int qmi_set_wds_reset_request(struct qmi_msg *msg);
int qmi_parse_wds_reset_response(struct qmi_msg *msg);

int qmi_set_wds_set_event_report_request(struct qmi_msg *msg, struct qmi_wds_set_event_report_request *req);
int qmi_parse_wds_set_event_report_response(struct qmi_msg *msg);

int qmi_set_wds_abort_request(struct qmi_msg *msg, struct qmi_wds_abort_request *req);
int qmi_parse_wds_abort_response(struct qmi_msg *msg);

int qmi_set_wds_get_supported_messages_request(struct qmi_msg *msg);
int qmi_parse_wds_get_supported_messages_response(struct qmi_msg *msg, struct qmi_wds_get_supported_messages_response *res);

int qmi_set_wds_start_network_request(struct qmi_msg *msg, struct qmi_wds_start_network_request *req);
int qmi_parse_wds_start_network_response(struct qmi_msg *msg, struct qmi_wds_start_network_response *res);

int qmi_set_wds_stop_network_request(struct qmi_msg *msg, struct qmi_wds_stop_network_request *req);
int qmi_parse_wds_stop_network_response(struct qmi_msg *msg);

int qmi_set_wds_get_packet_service_status_request(struct qmi_msg *msg);
int qmi_parse_wds_get_packet_service_status_response(struct qmi_msg *msg, struct qmi_wds_get_packet_service_status_response *res);

int qmi_set_wds_get_channel_rates_request(struct qmi_msg *msg);
int qmi_parse_wds_get_channel_rates_response(struct qmi_msg *msg, struct qmi_wds_get_channel_rates_response *res);

int qmi_set_wds_get_packet_statistics_request(struct qmi_msg *msg, struct qmi_wds_get_packet_statistics_request *req);
int qmi_parse_wds_get_packet_statistics_response(struct qmi_msg *msg, struct qmi_wds_get_packet_statistics_response *res);

int qmi_set_wds_go_dormant_request(struct qmi_msg *msg);
int qmi_parse_wds_go_dormant_response(struct qmi_msg *msg);

int qmi_set_wds_go_active_request(struct qmi_msg *msg);
int qmi_parse_wds_go_active_response(struct qmi_msg *msg);

int qmi_set_wds_create_profile_request(struct qmi_msg *msg, struct qmi_wds_create_profile_request *req);
int qmi_parse_wds_create_profile_response(struct qmi_msg *msg, struct qmi_wds_create_profile_response *res);

int qmi_set_wds_modify_profile_request(struct qmi_msg *msg, struct qmi_wds_modify_profile_request *req);
int qmi_parse_wds_modify_profile_response(struct qmi_msg *msg, struct qmi_wds_modify_profile_response *res);

int qmi_set_wds_delete_profile_request(struct qmi_msg *msg, struct qmi_wds_delete_profile_request *req);
int qmi_parse_wds_delete_profile_response(struct qmi_msg *msg, struct qmi_wds_delete_profile_response *res);

int qmi_set_wds_get_profile_list_request(struct qmi_msg *msg, struct qmi_wds_get_profile_list_request *req);
int qmi_parse_wds_get_profile_list_response(struct qmi_msg *msg, struct qmi_wds_get_profile_list_response *res);

int qmi_set_wds_get_profile_settings_request(struct qmi_msg *msg, struct qmi_wds_get_profile_settings_request *req);
int qmi_parse_wds_get_profile_settings_response(struct qmi_msg *msg, struct qmi_wds_get_profile_settings_response *res);

int qmi_set_wds_get_default_settings_request(struct qmi_msg *msg, struct qmi_wds_get_default_settings_request *req);
int qmi_parse_wds_get_default_settings_response(struct qmi_msg *msg, struct qmi_wds_get_default_settings_response *res);

int qmi_set_wds_get_current_settings_request(struct qmi_msg *msg, struct qmi_wds_get_current_settings_request *req);
int qmi_parse_wds_get_current_settings_response(struct qmi_msg *msg, struct qmi_wds_get_current_settings_response *res);

int qmi_set_wds_get_dormancy_status_request(struct qmi_msg *msg);
int qmi_parse_wds_get_dormancy_status_response(struct qmi_msg *msg, struct qmi_wds_get_dormancy_status_response *res);

int qmi_set_wds_get_autoconnect_settings_request(struct qmi_msg *msg);
int qmi_parse_wds_get_autoconnect_settings_response(struct qmi_msg *msg, struct qmi_wds_get_autoconnect_settings_response *res);

int qmi_set_wds_get_data_bearer_technology_request(struct qmi_msg *msg);
int qmi_parse_wds_get_data_bearer_technology_response(struct qmi_msg *msg, struct qmi_wds_get_data_bearer_technology_response *res);

int qmi_set_wds_get_current_data_bearer_technology_request(struct qmi_msg *msg);
int qmi_parse_wds_get_current_data_bearer_technology_response(struct qmi_msg *msg, struct qmi_wds_get_current_data_bearer_technology_response *res);

int qmi_set_wds_get_default_profile_number_request(struct qmi_msg *msg, struct qmi_wds_get_default_profile_number_request *req);
int qmi_parse_wds_get_default_profile_number_response(struct qmi_msg *msg, struct qmi_wds_get_default_profile_number_response *res);

int qmi_set_wds_set_default_profile_number_request(struct qmi_msg *msg, struct qmi_wds_set_default_profile_number_request *req);
int qmi_parse_wds_set_default_profile_number_response(struct qmi_msg *msg, struct qmi_wds_set_default_profile_number_response *res);

int qmi_set_wds_set_ip_family_request(struct qmi_msg *msg, struct qmi_wds_set_ip_family_request *req);
int qmi_parse_wds_set_ip_family_response(struct qmi_msg *msg);

int qmi_set_wds_set_autoconnect_settings_request(struct qmi_msg *msg, struct qmi_wds_set_autoconnect_settings_request *req);
int qmi_parse_wds_set_autoconnect_settings_response(struct qmi_msg *msg);

int qmi_set_wds_get_pdn_throttle_info_request(struct qmi_msg *msg, struct qmi_wds_get_pdn_throttle_info_request *req);
int qmi_parse_wds_get_pdn_throttle_info_response(struct qmi_msg *msg, struct qmi_wds_get_pdn_throttle_info_response *res);

int qmi_set_wds_get_lte_attach_parameters_request(struct qmi_msg *msg);
int qmi_parse_wds_get_lte_attach_parameters_response(struct qmi_msg *msg, struct qmi_wds_get_lte_attach_parameters_response *res);

int qmi_set_wds_bind_data_port_request(struct qmi_msg *msg, struct qmi_wds_bind_data_port_request *req);
int qmi_parse_wds_bind_data_port_response(struct qmi_msg *msg);

int qmi_set_wds_get_max_lte_attach_pdn_number_request(struct qmi_msg *msg);
int qmi_parse_wds_get_max_lte_attach_pdn_number_response(struct qmi_msg *msg, struct qmi_wds_get_max_lte_attach_pdn_number_response *res);

int qmi_set_wds_set_lte_attach_pdn_list_request(struct qmi_msg *msg, struct qmi_wds_set_lte_attach_pdn_list_request *req);
int qmi_parse_wds_set_lte_attach_pdn_list_response(struct qmi_msg *msg);

int qmi_set_wds_get_lte_attach_pdn_list_request(struct qmi_msg *msg);
int qmi_parse_wds_get_lte_attach_pdn_list_response(struct qmi_msg *msg, struct qmi_wds_get_lte_attach_pdn_list_response *res);

int qmi_set_wds_bind_mux_data_port_request(struct qmi_msg *msg, struct qmi_wds_bind_mux_data_port_request *req);
int qmi_parse_wds_bind_mux_data_port_response(struct qmi_msg *msg);

int qmi_set_wds_swi_create_profile_indexed_request(struct qmi_msg *msg, struct qmi_wds_swi_create_profile_indexed_request *req);
int qmi_parse_wds_swi_create_profile_indexed_response(struct qmi_msg *msg, struct qmi_wds_swi_create_profile_indexed_response *res);

