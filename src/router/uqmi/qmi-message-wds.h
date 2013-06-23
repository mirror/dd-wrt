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

int qmi_set_wds_get_current_settings_request(struct qmi_msg *msg, struct qmi_wds_get_current_settings_request *req);
int qmi_parse_wds_get_current_settings_response(struct qmi_msg *msg, struct qmi_wds_get_current_settings_response *res);

int qmi_set_wds_get_data_bearer_technology_request(struct qmi_msg *msg);
int qmi_parse_wds_get_data_bearer_technology_response(struct qmi_msg *msg, struct qmi_wds_get_data_bearer_technology_response *res);

int qmi_set_wds_get_current_data_bearer_technology_request(struct qmi_msg *msg);
int qmi_parse_wds_get_current_data_bearer_technology_response(struct qmi_msg *msg, struct qmi_wds_get_current_data_bearer_technology_response *res);

int qmi_set_wds_set_ip_family_request(struct qmi_msg *msg, struct qmi_wds_set_ip_family_request *req);
int qmi_parse_wds_set_ip_family_response(struct qmi_msg *msg);

