struct qmi_wds_abort_request {
	struct {
		unsigned int transaction_id : 1;
	} set;
	struct {
		uint16_t transaction_id;
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
		QmiWdsAuthentication authentication_preference;
		char *username;
		char *password;
		QmiWdsIpFamily ip_family_preference;
		QmiWdsTechnologyPreference technology_preference;
		uint8_t profile_index_3gpp;
		uint8_t profile_index_3gpp2;
		bool enable_autoconnect;
		QmiWdsExtendedTechnologyPreference extended_technology_preference;
		QmiWdsCallType call_type;
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
		QmiWdsCallEndReason call_end_reason;
		struct {
			QmiWdsVerboseCallEndReasonType type;
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
		QmiWdsConnectionStatus connection_status;
	} data;
};

struct qmi_wds_get_packet_statistics_request {
	struct {
		unsigned int mask : 1;
	} set;
	struct {
		QmiWdsPacketStatisticsMaskFlag mask;
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

struct qmi_wds_get_profile_list_request {
	struct {
		unsigned int profile_type : 1;
	} set;
	struct {
		QmiWdsProfileType profile_type;
	} data;
};

struct qmi_wds_get_profile_list_response {
	struct {
		unsigned int extended_error_code : 1;
	} set;
	struct {
		unsigned int profile_list_n;
		struct {
			QmiWdsProfileType profile_type;
			uint8_t profile_index;
			char *profile_name;
		} *profile_list;
		QmiWdsDsProfileError extended_error_code;
	} data;
};

struct qmi_wds_get_profile_settings_request {
	struct {
		unsigned int profile_id : 1;
	} set;
	struct {
		struct {
			QmiWdsProfileType profile_type;
			uint8_t profile_index;
		} profile_id;
	} data;
};

struct qmi_wds_get_profile_settings_response {
	struct {
		unsigned int pdp_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
		unsigned int gprs_requested_qos : 1;
		unsigned int gprs_minimum_qos : 1;
		unsigned int authentication : 1;
		unsigned int ipv4_address_preference : 1;
		unsigned int pcscf_address_using_pco : 1;
		unsigned int pcscf_address_using_dhcp : 1;
		unsigned int imcn_flag : 1;
		unsigned int ipv6_address_preference : 1;
		unsigned int ipv6_primary_dns_address_preference : 1;
		unsigned int ipv6_secondary_dns_address_preference : 1;
		unsigned int extended_error_code : 1;
	} set;
	struct {
		char *profile_name;
		QmiWdsPdpType pdp_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
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
		QmiWdsAuthentication authentication;
		uint32_t ipv4_address_preference;
		bool pcscf_address_using_pco;
		bool pcscf_address_using_dhcp;
		bool imcn_flag;
		struct {
			uint16_t address[8];
		} ipv6_address_preference;
		uint16_t ipv6_primary_dns_address_preference[8];
		uint16_t ipv6_secondary_dns_address_preference[8];
		QmiWdsDsProfileError extended_error_code;
	} data;
};

struct qmi_wds_get_default_settings_request {
	struct {
		unsigned int profile_type : 1;
	} set;
	struct {
		QmiWdsProfileType profile_type;
	} data;
};

struct qmi_wds_get_default_settings_response {
	struct {
		unsigned int pdp_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
		unsigned int gprs_requested_qos : 1;
		unsigned int gprs_minimum_qos : 1;
		unsigned int authentication : 1;
		unsigned int ipv4_address_preference : 1;
		unsigned int pcscf_address_using_pco : 1;
		unsigned int pcscf_address_using_dhcp : 1;
		unsigned int imcn_flag : 1;
		unsigned int ipv6_address_preference : 1;
		unsigned int ipv6_primary_dns_address_preference : 1;
		unsigned int ipv6_secondary_dns_address_preference : 1;
		unsigned int extended_error_code : 1;
	} set;
	struct {
		char *profile_name;
		QmiWdsPdpType pdp_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
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
		QmiWdsAuthentication authentication;
		uint32_t ipv4_address_preference;
		bool pcscf_address_using_pco;
		bool pcscf_address_using_dhcp;
		bool imcn_flag;
		struct {
			uint16_t address[8];
		} ipv6_address_preference;
		uint16_t ipv6_primary_dns_address_preference[8];
		uint16_t ipv6_secondary_dns_address_preference[8];
		QmiWdsDsProfileError extended_error_code;
	} data;
};

struct qmi_wds_get_current_settings_request {
	struct {
		unsigned int requested_settings : 1;
	} set;
	struct {
		QmiWdsGetCurrentSettingsRequestedSettings requested_settings;
	} data;
};

struct qmi_wds_get_current_settings_response {
	struct {
		unsigned int pdp_type : 1;
		unsigned int primary_ipv4_dns_address : 1;
		unsigned int secondary_ipv4_dns_address : 1;
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
		QmiWdsPdpType pdp_type;
		char *apn_name;
		uint32_t primary_ipv4_dns_address;
		uint32_t secondary_ipv4_dns_address;
		struct {
			uint32_t precedence_class;
			uint32_t delay_class;
			uint32_t reliability_class;
			uint32_t peak_throughput_class;
			uint32_t mean_throughput_class;
		} gprs_granted_qos;
		char *username;
		QmiWdsAuthentication authentication;
		uint32_t ipv4_address;
		struct {
			QmiWdsProfileType profile_type;
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
		QmiWdsIpFamily ip_family;
		int8_t imcn_flag;
		QmiWdsExtendedTechnologyPreference extended_technology_preference;
	} data;
};

struct qmi_wds_get_autoconnect_setting_response {
	struct {
		unsigned int setting : 1;
		unsigned int roaming : 1;
	} set;
	struct {
		QmiWdsAutoconnectSetting setting;
		bool roaming;
	} data;
};

struct qmi_wds_get_data_bearer_technology_response {
	struct {
		unsigned int current : 1;
		unsigned int last : 1;
	} set;
	struct {
		QmiWdsDataBearerTechnology current;
		QmiWdsDataBearerTechnology last;
	} data;
};

struct qmi_wds_get_current_data_bearer_technology_response {
	struct {
		unsigned int current : 1;
		unsigned int last : 1;
	} set;
	struct {
		struct {
			QmiWdsNetworkType network_type;
			uint32_t rat_mask;
			uint32_t so_mask;
		} current;
		struct {
			QmiWdsNetworkType network_type;
			uint32_t rat_mask;
			uint32_t so_mask;
		} last;
	} data;
};

struct qmi_wds_set_ip_family_request {
	struct {
		unsigned int preference : 1;
	} set;
	struct {
		QmiWdsIpFamily preference;
	} data;
};

struct qmi_wds_set_autoconnect_setting_request {
	struct {
		unsigned int setting : 1;
		unsigned int roaming : 1;
	} set;
	struct {
		QmiWdsAutoconnectSetting setting;
		bool roaming;
	} data;
};

int qmi_set_wds_reset_request(struct qmi_msg *msg);
int qmi_parse_wds_reset_response(struct qmi_msg *msg);

int qmi_set_wds_abort_request(struct qmi_msg *msg, struct qmi_wds_abort_request *req);
int qmi_parse_wds_abort_response(struct qmi_msg *msg);

int qmi_set_wds_start_network_request(struct qmi_msg *msg, struct qmi_wds_start_network_request *req);
int qmi_parse_wds_start_network_response(struct qmi_msg *msg, struct qmi_wds_start_network_response *res);

int qmi_set_wds_stop_network_request(struct qmi_msg *msg, struct qmi_wds_stop_network_request *req);
int qmi_parse_wds_stop_network_response(struct qmi_msg *msg);

int qmi_set_wds_get_packet_service_status_request(struct qmi_msg *msg);
int qmi_parse_wds_get_packet_service_status_response(struct qmi_msg *msg, struct qmi_wds_get_packet_service_status_response *res);

int qmi_set_wds_get_packet_statistics_request(struct qmi_msg *msg, struct qmi_wds_get_packet_statistics_request *req);
int qmi_parse_wds_get_packet_statistics_response(struct qmi_msg *msg, struct qmi_wds_get_packet_statistics_response *res);

int qmi_set_wds_get_profile_list_request(struct qmi_msg *msg, struct qmi_wds_get_profile_list_request *req);
int qmi_parse_wds_get_profile_list_response(struct qmi_msg *msg, struct qmi_wds_get_profile_list_response *res);

int qmi_set_wds_get_profile_settings_request(struct qmi_msg *msg, struct qmi_wds_get_profile_settings_request *req);
int qmi_parse_wds_get_profile_settings_response(struct qmi_msg *msg, struct qmi_wds_get_profile_settings_response *res);

int qmi_set_wds_get_default_settings_request(struct qmi_msg *msg, struct qmi_wds_get_default_settings_request *req);
int qmi_parse_wds_get_default_settings_response(struct qmi_msg *msg, struct qmi_wds_get_default_settings_response *res);

int qmi_set_wds_get_current_settings_request(struct qmi_msg *msg, struct qmi_wds_get_current_settings_request *req);
int qmi_parse_wds_get_current_settings_response(struct qmi_msg *msg, struct qmi_wds_get_current_settings_response *res);

int qmi_set_wds_get_autoconnect_setting_request(struct qmi_msg *msg);
int qmi_parse_wds_get_autoconnect_setting_response(struct qmi_msg *msg, struct qmi_wds_get_autoconnect_setting_response *res);

int qmi_set_wds_get_data_bearer_technology_request(struct qmi_msg *msg);
int qmi_parse_wds_get_data_bearer_technology_response(struct qmi_msg *msg, struct qmi_wds_get_data_bearer_technology_response *res);

int qmi_set_wds_get_current_data_bearer_technology_request(struct qmi_msg *msg);
int qmi_parse_wds_get_current_data_bearer_technology_response(struct qmi_msg *msg, struct qmi_wds_get_current_data_bearer_technology_response *res);

int qmi_set_wds_set_ip_family_request(struct qmi_msg *msg, struct qmi_wds_set_ip_family_request *req);
int qmi_parse_wds_set_ip_family_response(struct qmi_msg *msg);

int qmi_set_wds_set_autoconnect_setting_request(struct qmi_msg *msg, struct qmi_wds_set_autoconnect_setting_request *req);
int qmi_parse_wds_set_autoconnect_setting_response(struct qmi_msg *msg);

