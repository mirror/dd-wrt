struct qmi_nas_abort_request {
	struct {
		unsigned int transaction_id : 1;
	} set;
	struct {
		uint16_t transaction_id;
	} data;
};

struct qmi_nas_set_event_report_request {
	struct {
		unsigned int signal_strength_indicator : 1;
		unsigned int rf_band_information : 1;
		unsigned int registration_reject_reason : 1;
		unsigned int rssi_indicator : 1;
		unsigned int ecio_indicator : 1;
		unsigned int io_indicator : 1;
		unsigned int sinr_indicator : 1;
		unsigned int error_rate_indicator : 1;
		unsigned int ecio_threshold : 1;
		unsigned int sinr_threshold : 1;
		unsigned int lte_snr_delta : 1;
		unsigned int lte_rsrp_delta : 1;
	} set;
	struct {
		struct {
			bool report;
			unsigned int thresholds_n;
			int8_t *thresholds;
		} signal_strength_indicator;
		bool rf_band_information;
		bool registration_reject_reason;
		struct {
			bool report;
			uint8_t rssi_delta;
		} rssi_indicator;
		struct {
			bool report;
			uint8_t ecio_delta;
		} ecio_indicator;
		struct {
			bool report;
			uint8_t io_delta;
		} io_indicator;
		struct {
			bool report;
			uint8_t sinr_delta;
		} sinr_indicator;
		bool error_rate_indicator;
		struct {
			bool report;
			unsigned int thresholds_n;
			int16_t *thresholds;
		} ecio_threshold;
		struct {
			bool report;
			unsigned int thresholds_n;
			uint8_t *thresholds;
		} sinr_threshold;
		struct {
			bool report;
			uint8_t snr_delta;
		} lte_snr_delta;
		struct {
			bool report;
			uint8_t rsrp_delta;
		} lte_rsrp_delta;
	} data;
};

struct qmi_nas_register_indications_request {
	struct {
		unsigned int system_selection_preference : 1;
		unsigned int ddtm_events : 1;
		unsigned int serving_system_events : 1;
		unsigned int dual_standby_preference : 1;
		unsigned int subscription_info : 1;
		unsigned int network_time : 1;
		unsigned int system_info : 1;
		unsigned int signal_info : 1;
		unsigned int error_rate : 1;
		unsigned int hdr_new_uati_assigned : 1;
		unsigned int hdr_session_closed : 1;
		unsigned int managed_roaming : 1;
		unsigned int current_plmn_name : 1;
		unsigned int embms_status : 1;
		unsigned int rf_band_information : 1;
	} set;
	struct {
		bool system_selection_preference;
		bool ddtm_events;
		bool serving_system_events;
		bool dual_standby_preference;
		bool subscription_info;
		bool network_time;
		bool system_info;
		bool signal_info;
		bool error_rate;
		bool hdr_new_uati_assigned;
		bool hdr_session_closed;
		bool managed_roaming;
		bool current_plmn_name;
		bool embms_status;
		bool rf_band_information;
	} data;
};

struct qmi_nas_get_signal_strength_request {
	struct {
		unsigned int request_mask : 1;
	} set;
	struct {
		QmiNasSignalStrengthRequest request_mask;
	} data;
};

struct qmi_nas_get_signal_strength_response {
	struct {
		unsigned int signal_strength : 1;
		unsigned int io : 1;
		unsigned int sinr : 1;
		unsigned int rsrq : 1;
		unsigned int lte_snr : 1;
		unsigned int lte_rsrp : 1;
	} set;
	struct {
		struct {
			int8_t strength;
			QmiNasRadioInterface radio_interface;
		} signal_strength;
		unsigned int strength_list_n;
		struct {
			int8_t strength;
			QmiNasRadioInterface radio_interface;
		} *strength_list;
		unsigned int rssi_list_n;
		struct {
			uint8_t rssi;
			QmiNasRadioInterface radio_interface;
		} *rssi_list;
		unsigned int ecio_list_n;
		struct {
			int8_t ecio;
			QmiNasRadioInterface radio_interface;
		} *ecio_list;
		int32_t io;
		QmiNasEvdoSinrLevel sinr;
		unsigned int error_rate_list_n;
		struct {
			uint16_t rate;
			QmiNasRadioInterface radio_interface;
		} *error_rate_list;
		struct {
			int8_t rsrq;
			QmiNasRadioInterface radio_interface;
		} rsrq;
		int16_t lte_snr;
		int16_t lte_rsrp;
	} data;
};

struct qmi_nas_network_scan_request {
	struct {
		unsigned int network_type : 1;
	} set;
	struct {
		QmiNasNetworkScanType network_type;
	} data;
};

struct qmi_nas_network_scan_response {
	struct {
	} set;
	struct {
		unsigned int network_information_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			QmiNasNetworkStatus network_status;
			char *description;
		} *network_information;
		unsigned int radio_access_technology_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			QmiNasRadioInterface radio_interface;
		} *radio_access_technology;
		unsigned int mnc_pcs_digit_include_status_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			bool includes_pcs_digit;
		} *mnc_pcs_digit_include_status;
	} data;
};

struct qmi_nas_initiate_network_register_request {
	struct {
		unsigned int action : 1;
		unsigned int manual_registration_info_3gpp : 1;
		unsigned int change_duration : 1;
		unsigned int mnc_pcs_digit_include_status : 1;
	} set;
	struct {
		QmiNasNetworkRegisterType action;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			QmiNasRadioInterface radio_interface;
		} manual_registration_info_3gpp;
		QmiNasChangeDuration change_duration;
		bool mnc_pcs_digit_include_status;
	} data;
};

struct qmi_nas_get_serving_system_response {
	struct {
		unsigned int serving_system : 1;
		unsigned int roaming_indicator : 1;
		unsigned int current_plmn : 1;
		unsigned int cdma_system_id : 1;
		unsigned int cdma_base_station_info : 1;
		unsigned int default_roaming_indicator : 1;
		unsigned int time_zone_3gpp2 : 1;
		unsigned int cdma_p_rev : 1;
		unsigned int time_zone_3gpp : 1;
		unsigned int daylight_saving_time_adjustment_3gpp : 1;
		unsigned int lac_3gpp : 1;
		unsigned int cid_3gpp : 1;
		unsigned int concurrent_service_info_3gpp2 : 1;
		unsigned int prl_indicator_3gpp2 : 1;
		unsigned int dtm_support : 1;
		unsigned int detailed_service_status : 1;
		unsigned int cdma_system_info : 1;
		unsigned int hdr_personality : 1;
		unsigned int lte_tac : 1;
		unsigned int call_barring_status : 1;
		unsigned int umts_primary_scrambling_code : 1;
		unsigned int mnc_pcs_digit_include_status : 1;
	} set;
	struct {
		struct {
			QmiNasRegistrationState registration_state;
			QmiNasAttachState cs_attach_state;
			QmiNasAttachState ps_attach_state;
			QmiNasNetworkType selected_network;
			unsigned int radio_interfaces_n;
			QmiNasRadioInterface *radio_interfaces;
		} serving_system;
		QmiNasRoamingIndicatorStatus roaming_indicator;
		unsigned int data_service_capability_n;
		QmiNasDataCapability *data_service_capability;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			char *description;
		} current_plmn;
		struct {
			uint16_t sid;
			uint16_t nid;
		} cdma_system_id;
		struct {
			uint16_t base_station_id;
			int32_t base_station_latitude;
			int32_t base_station_longitude;
		} cdma_base_station_info;
		unsigned int roaming_indicator_list_n;
		struct {
			QmiNasRadioInterface radio_interface;
			QmiNasRoamingIndicatorStatus roaming_indicator;
		} *roaming_indicator_list;
		QmiNasRoamingIndicatorStatus default_roaming_indicator;
		struct {
			uint8_t leap_seconds;
			int8_t local_time_offset;
			bool daylight_saving_time;
		} time_zone_3gpp2;
		uint8_t cdma_p_rev;
		int8_t time_zone_3gpp;
		uint8_t daylight_saving_time_adjustment_3gpp;
		uint16_t lac_3gpp;
		uint32_t cid_3gpp;
		bool concurrent_service_info_3gpp2;
		bool prl_indicator_3gpp2;
		bool dtm_support;
		struct {
			QmiNasServiceStatus status;
			QmiNasNetworkServiceDomain capability;
			QmiNasServiceStatus hdr_status;
			bool hdr_hybrid;
			bool forbidden;
		} detailed_service_status;
		struct {
			uint16_t mcc;
			uint8_t imsi_11_12;
		} cdma_system_info;
		QmiNasHdrPersonality hdr_personality;
		uint16_t lte_tac;
		struct {
			QmiNasCallBarringStatus cs_status;
			QmiNasCallBarringStatus ps_status;
		} call_barring_status;
		uint16_t umts_primary_scrambling_code;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			bool includes_pcs_digit;
		} mnc_pcs_digit_include_status;
	} data;
};

struct qmi_nas_get_home_network_response {
	struct {
		unsigned int home_network : 1;
		unsigned int home_system_id : 1;
		unsigned int home_network_3gpp2 : 1;
	} set;
	struct {
		struct {
			uint16_t mcc;
			uint16_t mnc;
			char *description;
		} home_network;
		struct {
			uint16_t sid;
			uint16_t nid;
		} home_system_id;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			QmiNasNetworkDescriptionDisplay display_description;
			QmiNasNetworkDescriptionEncoding description_encoding;
			char *description;
		} home_network_3gpp2;
	} data;
};

struct qmi_nas_set_technology_preference_request {
	struct {
		unsigned int current : 1;
	} set;
	struct {
		struct {
			QmiNasRadioTechnologyPreference technology_preference;
			QmiNasPreferenceDuration technology_preference_duration;
		} current;
	} data;
};

struct qmi_nas_get_technology_preference_response {
	struct {
		unsigned int active : 1;
		unsigned int persistent : 1;
	} set;
	struct {
		struct {
			QmiNasRadioTechnologyPreference technology_preference;
			QmiNasPreferenceDuration technology_preference_duration;
		} active;
		QmiNasRadioTechnologyPreference persistent;
	} data;
};

struct qmi_nas_get_rf_band_information_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		struct {
			QmiNasRadioInterface radio_interface;
			QmiNasActiveBand active_band_class;
			uint16_t active_channel;
		} *list;
	} data;
};

struct qmi_nas_set_system_selection_preference_request {
	struct {
		unsigned int emergency_mode : 1;
		unsigned int mode_preference : 1;
		unsigned int band_preference : 1;
		unsigned int cdma_prl_preference : 1;
		unsigned int roaming_preference : 1;
		unsigned int lte_band_preference : 1;
		unsigned int network_selection_preference : 1;
		unsigned int change_duration : 1;
		unsigned int service_domain_preference : 1;
		unsigned int gsm_wcdma_acquisition_order_preference : 1;
		unsigned int mnc_pds_digit_include_status : 1;
		unsigned int td_scdma_band_preference : 1;
	} set;
	struct {
		bool emergency_mode;
		QmiNasRatModePreference mode_preference;
		QmiNasBandPreference band_preference;
		QmiNasCdmaPrlPreference cdma_prl_preference;
		QmiNasRoamingPreference roaming_preference;
		QmiNasLteBandPreference lte_band_preference;
		struct {
			QmiNasNetworkSelectionPreference mode;
			uint16_t mcc;
			uint16_t mnc;
		} network_selection_preference;
		QmiNasChangeDuration change_duration;
		QmiNasServiceDomainPreference service_domain_preference;
		QmiNasGsmWcdmaAcquisitionOrderPreference gsm_wcdma_acquisition_order_preference;
		bool mnc_pds_digit_include_status;
		QmiNasTdScdmaBandPreference td_scdma_band_preference;
	} data;
};

struct qmi_nas_get_system_selection_preference_response {
	struct {
		unsigned int emergency_mode : 1;
		unsigned int mode_preference : 1;
		unsigned int band_preference : 1;
		unsigned int cdma_prl_preference : 1;
		unsigned int roaming_preference : 1;
		unsigned int lte_band_preference : 1;
		unsigned int network_selection_preference : 1;
		unsigned int service_domain_preference : 1;
		unsigned int gsm_wcdma_acquisition_order_preference : 1;
		unsigned int td_scdma_band_preference : 1;
		unsigned int manual_network_selection : 1;
	} set;
	struct {
		bool emergency_mode;
		QmiNasRatModePreference mode_preference;
		QmiNasBandPreference band_preference;
		QmiNasCdmaPrlPreference cdma_prl_preference;
		QmiNasRoamingPreference roaming_preference;
		QmiNasLteBandPreference lte_band_preference;
		QmiNasNetworkSelectionPreference network_selection_preference;
		QmiNasServiceDomainPreference service_domain_preference;
		QmiNasGsmWcdmaAcquisitionOrderPreference gsm_wcdma_acquisition_order_preference;
		QmiNasTdScdmaBandPreference td_scdma_band_preference;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			bool includes_pcs_digit;
		} manual_network_selection;
	} data;
};

struct qmi_nas_get_system_info_response {
	struct {
		unsigned int cdma_service_status : 1;
		unsigned int hdr_service_status : 1;
		unsigned int gsm_service_status : 1;
		unsigned int wcdma_service_status : 1;
		unsigned int lte_service_status : 1;
		unsigned int cdma_system_info : 1;
		unsigned int hdr_system_info : 1;
		unsigned int gsm_system_info : 1;
		unsigned int wcdma_system_info : 1;
		unsigned int lte_system_info : 1;
		unsigned int additional_cdma_system_info : 1;
		unsigned int additional_hdr_system_info : 1;
		unsigned int additional_gsm_system_info : 1;
		unsigned int additional_wcdma_system_info : 1;
		unsigned int additional_lte_system_info : 1;
		unsigned int gsm_call_barring_status : 1;
		unsigned int wcdma_call_barring_status : 1;
		unsigned int lte_voice_support : 1;
		unsigned int gsm_cipher_domain : 1;
		unsigned int wcdma_cipher_domain : 1;
		unsigned int td_scdma_service_status : 1;
		unsigned int td_scdma_system_info : 1;
		unsigned int lte_embms_coverage_info_support : 1;
		unsigned int sim_reject_info : 1;
	} set;
	struct {
		struct {
			QmiNasServiceStatus service_status;
			bool preferred_data_path;
		} cdma_service_status;
		struct {
			QmiNasServiceStatus service_status;
			bool preferred_data_path;
		} hdr_service_status;
		struct {
			QmiNasServiceStatus service_status;
			QmiNasServiceStatus true_service_status;
			bool preferred_data_path;
		} gsm_service_status;
		struct {
			QmiNasServiceStatus service_status;
			QmiNasServiceStatus true_service_status;
			bool preferred_data_path;
		} wcdma_service_status;
		struct {
			QmiNasServiceStatus service_status;
			QmiNasServiceStatus true_service_status;
			bool preferred_data_path;
		} lte_service_status;
		struct {
			bool domain_valid;
			QmiNasNetworkServiceDomain domain;
			bool service_capability_valid;
			QmiNasNetworkServiceDomain service_capability;
			bool roaming_status_valid;
			QmiNasRoamingStatus roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool prl_match_valid;
			bool prl_match;
			bool p_rev_valid;
			uint8_t p_rev;
			bool base_station_p_rev_valid;
			uint8_t base_station_p_rev;
			bool concurrent_service_support_valid;
			bool concurrent_service_support;
			bool cdma_system_id_valid;
			uint16_t sid;
			uint16_t nid;
			bool base_station_info_valid;
			uint16_t base_station_id;
			int32_t base_station_latitude;
			int32_t base_station_longitude;
			bool packet_zone_valid;
			uint16_t packet_zone;
			bool network_id_valid;
			char *mcc;
			char *mnc;
		} cdma_system_info;
		struct {
			bool domain_valid;
			QmiNasNetworkServiceDomain domain;
			bool service_capability_valid;
			QmiNasNetworkServiceDomain service_capability;
			bool roaming_status_valid;
			QmiNasRoamingStatus roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool prl_match_valid;
			bool prl_match;
			bool personality_valid;
			QmiNasHdrPersonality personality;
			bool protocol_revision_valid;
			QmiNasHdrProtocolRevision protocol_revision;
			bool is_856_system_id_valid;
			char *is_856_system_id;
		} hdr_system_info;
		struct {
			bool domain_valid;
			QmiNasNetworkServiceDomain domain;
			bool service_capability_valid;
			QmiNasNetworkServiceDomain service_capability;
			bool roaming_status_valid;
			QmiNasRoamingStatus roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			QmiNasNetworkServiceDomain registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool egprs_support_valid;
			bool egprs_support;
			bool dtm_support_valid;
			bool dtm_support;
		} gsm_system_info;
		struct {
			bool domain_valid;
			QmiNasNetworkServiceDomain domain;
			bool service_capability_valid;
			QmiNasNetworkServiceDomain service_capability;
			bool roaming_status_valid;
			QmiNasRoamingStatus roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			QmiNasNetworkServiceDomain registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool hs_call_status_valid;
			QmiNasWcdmaHsService hs_call_status;
			bool hs_service_valid;
			QmiNasWcdmaHsService hs_service;
			bool primary_scrambling_code_valid;
			uint16_t primary_scrambling_code;
		} wcdma_system_info;
		struct {
			bool domain_valid;
			QmiNasNetworkServiceDomain domain;
			bool service_capability_valid;
			QmiNasNetworkServiceDomain service_capability;
			bool roaming_status_valid;
			QmiNasRoamingStatus roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			QmiNasNetworkServiceDomain registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool tac_valid;
			uint16_t tac;
		} lte_system_info;
		struct {
			uint16_t geo_system_index;
			uint16_t registration_period;
		} additional_cdma_system_info;
		struct {
			uint16_t geo_system_index;
		} additional_hdr_system_info;
		struct {
			uint16_t geo_system_index;
			QmiNasCellBroadcastCapability cell_broadcast_support;
		} additional_gsm_system_info;
		struct {
			uint16_t geo_system_index;
			QmiNasCellBroadcastCapability cell_broadcast_support;
		} additional_wcdma_system_info;
		struct {
			uint16_t geo_system_index;
		} additional_lte_system_info;
		struct {
			QmiNasCallBarringStatus cs_status;
			QmiNasCallBarringStatus ps_status;
		} gsm_call_barring_status;
		struct {
			QmiNasCallBarringStatus cs_status;
			QmiNasCallBarringStatus ps_status;
		} wcdma_call_barring_status;
		bool lte_voice_support;
		QmiNasNetworkServiceDomain gsm_cipher_domain;
		QmiNasNetworkServiceDomain wcdma_cipher_domain;
		struct {
			QmiNasServiceStatus service_status;
			QmiNasServiceStatus true_service_status;
			bool preferred_data_path;
		} td_scdma_service_status;
		struct {
			bool domain_valid;
			QmiNasNetworkServiceDomain domain;
			bool service_capability_valid;
			QmiNasNetworkServiceDomain service_capability;
			bool roaming_status_valid;
			QmiNasRoamingStatus roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			QmiNasNetworkServiceDomain registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool hs_call_status_valid;
			QmiNasWcdmaHsService hs_call_status;
			bool hs_service_valid;
			QmiNasWcdmaHsService hs_service;
			bool cell_parameter_id_valid;
			uint16_t cell_parameter_id;
			bool cell_broadcast_support_valid;
			QmiNasCellBroadcastCapability cell_broadcast_support;
			bool cs_call_barring_status_valid;
			QmiNasCallBarringStatus cs_call_barring_status;
			bool ps_call_barring_status_valid;
			QmiNasCallBarringStatus ps_call_barring_status;
			bool cipher_domain_valid;
			QmiNasNetworkServiceDomain cipher_domain;
		} td_scdma_system_info;
		bool lte_embms_coverage_info_support;
		QmiNasSimRejectState sim_reject_info;
	} data;
};

struct qmi_nas_get_signal_info_response {
	struct {
		unsigned int cdma_signal_strength : 1;
		unsigned int hdr_signal_strength : 1;
		unsigned int gsm_signal_strength : 1;
		unsigned int wcdma_signal_strength : 1;
		unsigned int lte_signal_strength : 1;
		unsigned int tdma_signal_strength : 1;
	} set;
	struct {
		struct {
			int8_t rssi;
			int16_t ecio;
		} cdma_signal_strength;
		struct {
			int8_t rssi;
			int16_t ecio;
			QmiNasEvdoSinrLevel sinr;
			int32_t io;
		} hdr_signal_strength;
		int8_t gsm_signal_strength;
		struct {
			int8_t rssi;
			int16_t ecio;
		} wcdma_signal_strength;
		struct {
			int8_t rssi;
			int8_t rsrq;
			int16_t rsrp;
			int16_t snr;
		} lte_signal_strength;
		int8_t tdma_signal_strength;
	} data;
};

struct qmi_nas_config_signal_info_request {
	struct {
		unsigned int lte_report : 1;
	} set;
	struct {
		unsigned int rssi_threshold_n;
		int8_t *rssi_threshold;
		unsigned int ecio_threshold_n;
		int16_t *ecio_threshold;
		unsigned int sinr_threshold_n;
		uint8_t *sinr_threshold;
		unsigned int lte_snr_threshold_n;
		int16_t *lte_snr_threshold;
		unsigned int io_threshold_n;
		int32_t *io_threshold;
		unsigned int rsrq_threshold_n;
		int8_t *rsrq_threshold;
		unsigned int rsrp_threshold_n;
		int16_t *rsrp_threshold;
		struct {
			uint8_t rate;
			uint8_t average_period;
		} lte_report;
		unsigned int rscp_threshold_n;
		int8_t *rscp_threshold;
	} data;
};

struct qmi_nas_get_cdma_position_info_response {
	struct {
		unsigned int cdma_position_info : 1;
	} set;
	struct {
		struct {
			int8_t ui_in_idle_mode;
			unsigned int basestations_n;
			struct {
				QmiNasCdmaPilotType pilot_type;
				uint16_t system_id;
				uint16_t network_id;
				uint16_t base_station_id;
				uint16_t pilot_pn;
				uint16_t pilot_strength;
				int32_t latitude;
				int32_t longitude;
				uint64_t gps_time_in_milliseconds;
			} *basestations;
		} cdma_position_info;
	} data;
};

int qmi_set_nas_reset_request(struct qmi_msg *msg);
int qmi_parse_nas_reset_response(struct qmi_msg *msg);

int qmi_set_nas_abort_request(struct qmi_msg *msg, struct qmi_nas_abort_request *req);
int qmi_parse_nas_abort_response(struct qmi_msg *msg);

int qmi_set_nas_set_event_report_request(struct qmi_msg *msg, struct qmi_nas_set_event_report_request *req);
int qmi_parse_nas_set_event_report_response(struct qmi_msg *msg);

int qmi_set_nas_register_indications_request(struct qmi_msg *msg, struct qmi_nas_register_indications_request *req);
int qmi_parse_nas_register_indications_response(struct qmi_msg *msg);

int qmi_set_nas_get_signal_strength_request(struct qmi_msg *msg, struct qmi_nas_get_signal_strength_request *req);
int qmi_parse_nas_get_signal_strength_response(struct qmi_msg *msg, struct qmi_nas_get_signal_strength_response *res);

int qmi_set_nas_network_scan_request(struct qmi_msg *msg, struct qmi_nas_network_scan_request *req);
int qmi_parse_nas_network_scan_response(struct qmi_msg *msg, struct qmi_nas_network_scan_response *res);

int qmi_set_nas_initiate_network_register_request(struct qmi_msg *msg, struct qmi_nas_initiate_network_register_request *req);
int qmi_parse_nas_initiate_network_register_response(struct qmi_msg *msg);

int qmi_set_nas_get_serving_system_request(struct qmi_msg *msg);
int qmi_parse_nas_get_serving_system_response(struct qmi_msg *msg, struct qmi_nas_get_serving_system_response *res);

int qmi_set_nas_get_home_network_request(struct qmi_msg *msg);
int qmi_parse_nas_get_home_network_response(struct qmi_msg *msg, struct qmi_nas_get_home_network_response *res);

int qmi_set_nas_set_technology_preference_request(struct qmi_msg *msg, struct qmi_nas_set_technology_preference_request *req);
int qmi_parse_nas_set_technology_preference_response(struct qmi_msg *msg);

int qmi_set_nas_get_technology_preference_request(struct qmi_msg *msg);
int qmi_parse_nas_get_technology_preference_response(struct qmi_msg *msg, struct qmi_nas_get_technology_preference_response *res);

int qmi_set_nas_get_rf_band_information_request(struct qmi_msg *msg);
int qmi_parse_nas_get_rf_band_information_response(struct qmi_msg *msg, struct qmi_nas_get_rf_band_information_response *res);

int qmi_set_nas_set_system_selection_preference_request(struct qmi_msg *msg, struct qmi_nas_set_system_selection_preference_request *req);
int qmi_parse_nas_set_system_selection_preference_response(struct qmi_msg *msg);

int qmi_set_nas_get_system_selection_preference_request(struct qmi_msg *msg);
int qmi_parse_nas_get_system_selection_preference_response(struct qmi_msg *msg, struct qmi_nas_get_system_selection_preference_response *res);

int qmi_set_nas_get_system_info_request(struct qmi_msg *msg);
int qmi_parse_nas_get_system_info_response(struct qmi_msg *msg, struct qmi_nas_get_system_info_response *res);

int qmi_set_nas_get_signal_info_request(struct qmi_msg *msg);
int qmi_parse_nas_get_signal_info_response(struct qmi_msg *msg, struct qmi_nas_get_signal_info_response *res);

int qmi_set_nas_config_signal_info_request(struct qmi_msg *msg, struct qmi_nas_config_signal_info_request *req);
int qmi_parse_nas_config_signal_info_response(struct qmi_msg *msg);

int qmi_set_nas_get_cdma_position_info_request(struct qmi_msg *msg);
int qmi_parse_nas_get_cdma_position_info_response(struct qmi_msg *msg, struct qmi_nas_get_cdma_position_info_response *res);

