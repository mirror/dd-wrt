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
		unsigned int network_reject_information : 1;
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
		struct {
			bool enable_network_reject_indications;
			bool supress_system_info_indications;
		} network_reject_information;
	} data;
};

struct qmi_nas_get_supported_messages_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		uint8_t *list;
	} data;
};

struct qmi_nas_get_signal_strength_request {
	struct {
		unsigned int request_mask : 1;
	} set;
	struct {
		uint16_t request_mask;
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
			int8_t radio_interface;
		} signal_strength;
		unsigned int strength_list_n;
		struct {
			int8_t strength;
			int8_t radio_interface;
		} *strength_list;
		unsigned int rssi_list_n;
		struct {
			uint8_t rssi;
			int8_t radio_interface;
		} *rssi_list;
		unsigned int ecio_list_n;
		struct {
			uint8_t ecio;
			int8_t radio_interface;
		} *ecio_list;
		int32_t io;
		uint8_t sinr;
		unsigned int error_rate_list_n;
		struct {
			uint16_t rate;
			int8_t radio_interface;
		} *error_rate_list;
		struct {
			int8_t rsrq;
			int8_t radio_interface;
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
		uint8_t network_type;
	} data;
};

struct qmi_nas_network_scan_response {
	struct {
		unsigned int network_scan_result : 1;
	} set;
	struct {
		unsigned int network_information_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			uint8_t network_status;
			char *description;
		} *network_information;
		unsigned int radio_access_technology_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			int8_t radio_interface;
		} *radio_access_technology;
		unsigned int mnc_pcs_digit_include_status_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			bool includes_pcs_digit;
		} *mnc_pcs_digit_include_status;
		uint32_t network_scan_result;
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
		uint8_t action;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			int8_t radio_interface;
		} manual_registration_info_3gpp;
		uint8_t change_duration;
		bool mnc_pcs_digit_include_status;
	} data;
};

struct qmi_nas_attach_detach_request {
	struct {
		unsigned int action : 1;
	} set;
	struct {
		uint8_t action;
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
		unsigned int network_name_source : 1;
	} set;
	struct {
		struct {
			uint8_t registration_state;
			uint8_t cs_attach_state;
			uint8_t ps_attach_state;
			uint8_t selected_network;
			unsigned int radio_interfaces_n;
			int8_t *radio_interfaces;
		} serving_system;
		uint8_t roaming_indicator;
		unsigned int data_service_capability_n;
		uint8_t *data_service_capability;
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
			uint8_t radio_interface;
			uint8_t roaming_indicator;
		} *roaming_indicator_list;
		uint8_t default_roaming_indicator;
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
			uint8_t status;
			uint8_t capability;
			uint8_t hdr_status;
			bool hdr_hybrid;
			bool forbidden;
		} detailed_service_status;
		struct {
			uint16_t mcc;
			uint8_t imsi_11_12;
		} cdma_system_info;
		uint8_t hdr_personality;
		uint16_t lte_tac;
		struct {
			int32_t cs_status;
			int32_t ps_status;
		} call_barring_status;
		uint16_t umts_primary_scrambling_code;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			bool includes_pcs_digit;
		} mnc_pcs_digit_include_status;
		uint32_t network_name_source;
	} data;
};

struct qmi_nas_get_home_network_response {
	struct {
		unsigned int home_network : 1;
		unsigned int home_system_id : 1;
		unsigned int home_network_3gpp2_ext : 1;
		unsigned int home_network_3gpp_mnc : 1;
		unsigned int network_name_source : 1;
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
			uint8_t display_description;
			uint8_t description_encoding;
			unsigned int description_n;
			uint8_t *description;
		} home_network_3gpp2_ext;
		struct {
			bool is_3gpp;
			bool includes_pcs_digit;
		} home_network_3gpp_mnc;
		uint32_t network_name_source;
	} data;
};

struct qmi_nas_get_preferred_networks_response {
	struct {
	} set;
	struct {
		unsigned int preferred_networks_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			uint16_t radio_access_technology;
		} *preferred_networks;
		unsigned int mnc_pcs_digit_include_status_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			bool includes_pcs_digit;
		} *mnc_pcs_digit_include_status;
	} data;
};

struct qmi_nas_set_preferred_networks_request {
	struct {
		unsigned int clear_previous_preferred_networks : 1;
	} set;
	struct {
		unsigned int preferred_networks_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			uint16_t radio_access_technology;
		} *preferred_networks;
		unsigned int mnc_pcs_digit_include_status_n;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			bool includes_pcs_digit;
		} *mnc_pcs_digit_include_status;
		bool clear_previous_preferred_networks;
	} data;
};

struct qmi_nas_set_technology_preference_request {
	struct {
		unsigned int current : 1;
	} set;
	struct {
		struct {
			uint16_t technology_preference;
			uint8_t technology_preference_duration;
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
			uint16_t technology_preference;
			uint8_t technology_preference_duration;
		} active;
		uint16_t persistent;
	} data;
};

struct qmi_nas_get_rf_band_information_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		struct {
			int8_t radio_interface;
			uint16_t active_band_class;
			uint16_t active_channel;
		} *list;
		unsigned int extended_list_n;
		struct {
			int8_t radio_interface;
			uint16_t active_band_class;
			uint32_t active_channel;
		} *extended_list;
		unsigned int bandwidth_list_n;
		struct {
			int8_t radio_interface;
			uint32_t bandwidth;
		} *bandwidth_list;
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
		unsigned int mnc_pcs_digit_include_status : 1;
		unsigned int td_scdma_band_preference : 1;
		unsigned int network_selection_registration_restriction : 1;
		unsigned int usage_preference : 1;
		unsigned int voice_domain_preference : 1;
		unsigned int extended_lte_band_preference : 1;
	} set;
	struct {
		bool emergency_mode;
		uint16_t mode_preference;
		uint64_t band_preference;
		uint16_t cdma_prl_preference;
		uint16_t roaming_preference;
		uint64_t lte_band_preference;
		struct {
			uint8_t mode;
			uint16_t mcc;
			uint16_t mnc;
		} network_selection_preference;
		uint8_t change_duration;
		uint32_t service_domain_preference;
		uint32_t gsm_wcdma_acquisition_order_preference;
		bool mnc_pcs_digit_include_status;
		uint64_t td_scdma_band_preference;
		unsigned int acquisition_order_preference_n;
		int8_t *acquisition_order_preference;
		uint32_t network_selection_registration_restriction;
		uint32_t usage_preference;
		uint32_t voice_domain_preference;
		struct {
			uint64_t mask_low;
			uint64_t mask_mid_low;
			uint64_t mask_mid_high;
			uint64_t mask_high;
		} extended_lte_band_preference;
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
		unsigned int network_selection_registration_restriction : 1;
		unsigned int usage_preference : 1;
		unsigned int voice_domain_preference : 1;
		unsigned int disabled_modes : 1;
		unsigned int extended_lte_band_preference : 1;
	} set;
	struct {
		bool emergency_mode;
		uint16_t mode_preference;
		uint64_t band_preference;
		uint16_t cdma_prl_preference;
		uint16_t roaming_preference;
		uint64_t lte_band_preference;
		uint8_t network_selection_preference;
		uint32_t service_domain_preference;
		uint32_t gsm_wcdma_acquisition_order_preference;
		uint64_t td_scdma_band_preference;
		unsigned int acquisition_order_preference_n;
		int8_t *acquisition_order_preference;
		struct {
			uint16_t mcc;
			uint16_t mnc;
			bool includes_pcs_digit;
		} manual_network_selection;
		uint32_t network_selection_registration_restriction;
		uint32_t usage_preference;
		uint32_t voice_domain_preference;
		uint16_t disabled_modes;
		struct {
			uint64_t mask_low;
			uint64_t mask_mid_low;
			uint64_t mask_mid_high;
			uint64_t mask_high;
		} extended_lte_band_preference;
	} data;
};

struct qmi_nas_get_operator_name_response {
	struct {
		unsigned int service_provider_name : 1;
		unsigned int nitz_information : 1;
	} set;
	struct {
		struct {
			uint8_t name_display_condition;
			char *name;
		} service_provider_name;
		unsigned int operator_plmn_list_n;
		struct {
			char *mcc;
			char *mnc;
			uint16_t lac1;
			uint16_t lac2;
			uint8_t plmn_name_record_identifier;
		} *operator_plmn_list;
		unsigned int operator_plmn_name_n;
		struct {
			uint8_t name_encoding;
			uint8_t short_country_initials;
			uint8_t long_name_spare_bits;
			uint8_t short_name_spare_bits;
			unsigned int long_name_n;
			uint8_t *long_name;
			unsigned int short_name_n;
			uint8_t *short_name;
		} *operator_plmn_name;
		char *operator_string_name;
		struct {
			uint8_t name_encoding;
			uint8_t short_country_initials;
			uint8_t long_name_spare_bits;
			uint8_t short_name_spare_bits;
			unsigned int long_name_n;
			uint8_t *long_name;
			unsigned int short_name_n;
			uint8_t *short_name;
		} nitz_information;
	} data;
};

struct qmi_nas_get_cell_location_info_response {
	struct {
		unsigned int geran_info_v2 : 1;
		unsigned int umts_info_v2 : 1;
		unsigned int cdma_info : 1;
		unsigned int intrafrequency_lte_info_v2 : 1;
		unsigned int interfrequency_lte_info : 1;
		unsigned int lte_info_neighboring_gsm : 1;
		unsigned int lte_info_neighboring_wcdma : 1;
		unsigned int umts_cell_id : 1;
		unsigned int umts_info_neighboring_lte : 1;
		unsigned int lte_info_timing_advance : 1;
		unsigned int nr5g_arfcn : 1;
		unsigned int nr5g_cell_information : 1;
	} set;
	struct {
		struct {
			uint32_t cell_id;
			uint8_t plmn[3];
			uint16_t lac;
			uint16_t geran_absolute_rf_channel_number;
			uint8_t base_station_identity_code;
			uint32_t timing_advance;
			uint16_t rx_level;
			unsigned int cell_n;
			struct {
				uint32_t cell_id;
				uint8_t plmn[3];
				uint16_t lac;
				uint16_t geran_absolute_rf_channel_number;
				uint8_t base_station_identity_code;
				uint16_t rx_level;
			} *cell;
		} geran_info_v2;
		struct {
			uint16_t cell_id;
			uint8_t plmn[3];
			uint16_t lac;
			uint16_t utra_absolute_rf_channel_number;
			uint16_t primary_scrambling_code;
			int16_t rscp;
			int16_t ecio;
			unsigned int cell_n;
			struct {
				uint16_t utra_absolute_rf_channel_number;
				uint16_t primary_scrambling_code;
				int16_t rscp;
				int16_t ecio;
			} *cell;
			unsigned int neighboring_geran_n;
			struct {
				uint16_t geran_absolute_rf_channel_number;
				uint8_t network_color_code;
				uint8_t base_station_color_code;
				int16_t rssi;
			} *neighboring_geran;
		} umts_info_v2;
		struct {
			uint16_t system_id;
			uint16_t network_id;
			uint16_t base_station_id;
			uint16_t reference_pn;
			uint32_t latitude;
			uint32_t longitude;
		} cdma_info;
		struct {
			bool ue_in_idle;
			uint8_t plmn[3];
			uint16_t tracking_area_code;
			uint32_t global_cell_id;
			uint16_t eutra_absolute_rf_channel_number;
			uint16_t serving_cell_id;
			uint8_t cell_reselection_priority;
			uint8_t s_non_intra_search_threshold;
			uint8_t serving_cell_low_threshold;
			uint8_t s_intra_search_threshold;
			unsigned int cell_n;
			struct {
				uint16_t physical_cell_id;
				int16_t rsrq;
				int16_t rsrp;
				int16_t rssi;
				int16_t cell_selection_rx_level;
			} *cell;
		} intrafrequency_lte_info_v2;
		struct {
			bool ue_in_idle;
			unsigned int frequency_n;
			struct {
				uint16_t eutra_absolute_rf_channel_number;
				uint8_t cell_selection_rx_level_low_threshold;
				uint8_t cell_selection_rx_level_high_threshold;
				uint8_t cell_reselection_priority;
				unsigned int cell_n;
				struct {
					uint16_t physical_cell_id;
					int16_t rsrq;
					int16_t rsrp;
					int16_t rssi;
					int16_t cell_selection_rx_level;
				} *cell;
			} *frequency;
		} interfrequency_lte_info;
		struct {
			bool ue_in_idle;
			unsigned int frequency_n;
			struct {
				uint8_t cell_reselection_priority;
				uint8_t cell_reselection_high_threshold;
				uint8_t cell_reselection_low_threshold;
				uint8_t ncc_permitted;
				unsigned int cell_n;
				struct {
					uint16_t geran_absolute_rf_channel_number;
					bool band_is_1900;
					bool cell_id_valid;
					uint8_t base_station_identity_code;
					int16_t rssi;
					int16_t cell_selection_rx_level;
				} *cell;
			} *frequency;
		} lte_info_neighboring_gsm;
		struct {
			bool ue_in_idle;
			unsigned int frequency_n;
			struct {
				uint16_t utra_absolute_rf_channel_number;
				uint8_t cell_reselection_priority;
				uint16_t cell_reselection_high_threshold;
				uint16_t cell_reselection_low_threshold;
				unsigned int cell_n;
				struct {
					uint16_t primary_scrambling_code;
					int16_t cpich_rscp;
					int16_t cpich_ecno;
					int16_t cell_selection_rx_level;
				} *cell;
			} *frequency;
		} lte_info_neighboring_wcdma;
		uint32_t umts_cell_id;
		struct {
			uint32_t rrc_state;
			unsigned int frequency_n;
			struct {
				uint16_t eutra_absolute_rf_channel_number;
				uint16_t physical_cell_id;
				float rsrp;
				float rsrq;
				int16_t cell_selection_rx_level;
				bool is_tdd;
			} *frequency;
		} umts_info_neighboring_lte;
		uint32_t lte_info_timing_advance;
		uint32_t nr5g_arfcn;
		struct {
			uint8_t plmn[3];
			uint8_t tracking_area_code[3];
			uint64_t global_cell_id;
			uint16_t physical_cell_id;
			int16_t rsrq;
			int16_t rsrp;
			int16_t snr;
		} nr5g_cell_information;
	} data;
};

struct qmi_nas_get_plmn_name_request {
	struct {
		unsigned int plmn : 1;
		unsigned int suppress_sim_error : 1;
		unsigned int mnc_pcs_digit_include_status : 1;
		unsigned int always_send_plmn_name : 1;
		unsigned int use_static_table_only : 1;
		unsigned int csg_id : 1;
		unsigned int radio_access_technology : 1;
		unsigned int send_all_information : 1;
	} set;
	struct {
		struct {
			uint16_t mcc;
			uint16_t mnc;
		} plmn;
		bool suppress_sim_error;
		bool mnc_pcs_digit_include_status;
		bool always_send_plmn_name;
		bool use_static_table_only;
		uint32_t csg_id;
		uint8_t radio_access_technology;
		bool send_all_information;
	} data;
};

struct qmi_nas_get_plmn_name_response {
	struct {
		unsigned int _3gpp_eons_plmn_name : 1;
		unsigned int display_bit_information : 1;
		unsigned int network_information : 1;
		unsigned int network_name_source : 1;
	} set;
	struct {
		struct {
			uint8_t service_provider_name_encoding;
			unsigned int service_provider_name_n;
			uint8_t *service_provider_name;
			uint8_t short_name_encoding;
			uint8_t short_name_country_initials;
			uint8_t short_name_spare_bits;
			unsigned int short_name_n;
			uint8_t *short_name;
			uint8_t long_name_encoding;
			uint8_t long_name_country_initials;
			uint8_t long_name_spare_bits;
			unsigned int long_name_n;
			uint8_t *long_name;
		} _3gpp_eons_plmn_name;
		struct {
			uint32_t service_provider_name_set;
			uint32_t plmn_name_set;
		} display_bit_information;
		uint32_t network_information;
		unsigned int plmn_name_with_language_id_n;
		struct {
			unsigned int long_name_n;
			uint16_t *long_name;
			unsigned int short_name_n;
			uint16_t *short_name;
			uint32_t language_id;
		} *plmn_name_with_language_id;
		unsigned int additional_information_n;
		uint16_t *additional_information;
		uint32_t network_name_source;
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
		unsigned int gsm_system_info_v2 : 1;
		unsigned int wcdma_system_info_v2 : 1;
		unsigned int lte_system_info_v2 : 1;
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
		unsigned int td_scdma_system_info_v2 : 1;
		unsigned int lte_embms_coverage_info_support : 1;
		unsigned int sim_reject_info : 1;
		unsigned int ims_voice_support : 1;
		unsigned int lte_voice_domain : 1;
		unsigned int cdma_registration_zone_id : 1;
		unsigned int gsm_routing_area_code : 1;
		unsigned int wcdma_routing_area_code : 1;
		unsigned int cdma_resolved_mcc : 1;
		unsigned int network_selection_registration_restriction : 1;
		unsigned int lte_registration_domain : 1;
		unsigned int lte_embms_coverage_info_trace_id : 1;
		unsigned int lte_cell_access_status : 1;
		unsigned int nr5g_service_status_info : 1;
		unsigned int nr5g_system_info : 1;
		unsigned int eutra_with_nr5g_availability : 1;
		unsigned int dcnr_restriction_info : 1;
		unsigned int nr5g_tracking_area_code : 1;
	} set;
	struct {
		struct {
			uint8_t service_status;
			bool preferred_data_path;
		} cdma_service_status;
		struct {
			uint8_t service_status;
			bool preferred_data_path;
		} hdr_service_status;
		struct {
			uint8_t service_status;
			uint8_t true_service_status;
			bool preferred_data_path;
		} gsm_service_status;
		struct {
			uint8_t service_status;
			uint8_t true_service_status;
			bool preferred_data_path;
		} wcdma_service_status;
		struct {
			uint8_t service_status;
			uint8_t true_service_status;
			bool preferred_data_path;
		} lte_service_status;
		struct {
			bool domain_valid;
			uint8_t domain;
			bool service_capability_valid;
			uint8_t service_capability;
			bool roaming_status_valid;
			uint8_t roaming_status;
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
			uint8_t domain;
			bool service_capability_valid;
			uint8_t service_capability;
			bool roaming_status_valid;
			uint8_t roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool prl_match_valid;
			bool prl_match;
			bool personality_valid;
			uint8_t personality;
			bool protocol_revision_valid;
			uint8_t protocol_revision;
			bool is_856_system_id_valid;
			char *is_856_system_id;
		} hdr_system_info;
		struct {
			bool domain_valid;
			uint8_t domain;
			bool service_capability_valid;
			uint8_t service_capability;
			bool roaming_status_valid;
			uint8_t roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			uint8_t registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool egprs_support_valid;
			bool egprs_support;
			bool dtm_support_valid;
			bool dtm_support;
		} gsm_system_info_v2;
		struct {
			bool domain_valid;
			uint8_t domain;
			bool service_capability_valid;
			uint8_t service_capability;
			bool roaming_status_valid;
			uint8_t roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			uint8_t registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool hs_call_status_valid;
			uint8_t hs_call_status;
			bool hs_service_valid;
			uint8_t hs_service;
			bool primary_scrambling_code_valid;
			uint16_t primary_scrambling_code;
		} wcdma_system_info_v2;
		struct {
			bool domain_valid;
			uint8_t domain;
			bool service_capability_valid;
			uint8_t service_capability;
			bool roaming_status_valid;
			uint8_t roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			uint8_t registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool tac_valid;
			uint16_t tac;
		} lte_system_info_v2;
		struct {
			uint16_t geo_system_index;
			uint16_t registration_period;
		} additional_cdma_system_info;
		struct {
			uint16_t geo_system_index;
		} additional_hdr_system_info;
		struct {
			uint16_t geo_system_index;
			uint32_t cell_broadcast_support;
		} additional_gsm_system_info;
		struct {
			uint16_t geo_system_index;
			uint32_t cell_broadcast_support;
		} additional_wcdma_system_info;
		struct {
			uint16_t geo_system_index;
		} additional_lte_system_info;
		struct {
			int32_t cs_status;
			int32_t ps_status;
		} gsm_call_barring_status;
		struct {
			int32_t cs_status;
			int32_t ps_status;
		} wcdma_call_barring_status;
		bool lte_voice_support;
		uint8_t gsm_cipher_domain;
		uint8_t wcdma_cipher_domain;
		struct {
			uint8_t service_status;
			uint8_t true_service_status;
			bool preferred_data_path;
		} td_scdma_service_status;
		struct {
			bool domain_valid;
			uint8_t domain;
			bool service_capability_valid;
			uint8_t service_capability;
			bool roaming_status_valid;
			uint8_t roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			uint8_t registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool hs_call_status_valid;
			uint8_t hs_call_status;
			bool hs_service_valid;
			uint8_t hs_service;
			bool cell_parameter_id_valid;
			uint16_t cell_parameter_id;
			bool cell_broadcast_support_valid;
			uint32_t cell_broadcast_support;
			bool cs_call_barring_status_valid;
			int32_t cs_call_barring_status;
			bool ps_call_barring_status_valid;
			int32_t ps_call_barring_status;
			bool cipher_domain_valid;
			uint8_t cipher_domain;
		} td_scdma_system_info_v2;
		bool lte_embms_coverage_info_support;
		uint32_t sim_reject_info;
		bool ims_voice_support;
		uint32_t lte_voice_domain;
		uint16_t cdma_registration_zone_id;
		uint8_t gsm_routing_area_code;
		uint8_t wcdma_routing_area_code;
		uint16_t cdma_resolved_mcc;
		uint32_t network_selection_registration_restriction;
		uint32_t lte_registration_domain;
		uint16_t lte_embms_coverage_info_trace_id;
		uint32_t lte_cell_access_status;
		struct {
			uint8_t service_status;
			uint8_t true_service_status;
			bool preferred_data_path;
		} nr5g_service_status_info;
		struct {
			bool domain_valid;
			uint8_t domain;
			bool service_capability_valid;
			uint8_t service_capability;
			bool roaming_status_valid;
			uint8_t roaming_status;
			bool forbidden_valid;
			bool forbidden;
			bool lac_valid;
			uint16_t lac;
			bool cid_valid;
			uint32_t cid;
			bool registration_reject_info_valid;
			uint8_t registration_reject_domain;
			uint8_t registration_reject_cause;
			bool network_id_valid;
			char *mcc;
			char *mnc;
			bool tac_valid;
			uint16_t tac;
		} nr5g_system_info;
		bool eutra_with_nr5g_availability;
		bool dcnr_restriction_info;
		uint8_t nr5g_tracking_area_code[3];
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
		unsigned int tdma_signal_strength_extended : 1;
		unsigned int _5g_signal_strength : 1;
		unsigned int _5g_signal_strength_extended : 1;
	} set;
	struct {
		struct {
			int8_t rssi;
			int16_t ecio;
		} cdma_signal_strength;
		struct {
			int8_t rssi;
			int16_t ecio;
			uint8_t sinr;
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
		struct {
			int32_t rssi;
			int32_t rscp;
			int32_t ecio;
			int32_t sinr;
		} tdma_signal_strength_extended;
		struct {
			int16_t rsrp;
			int16_t snr;
		} _5g_signal_strength;
		int16_t _5g_signal_strength_extended;
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

struct qmi_nas_get_tx_rx_info_request {
	struct {
		unsigned int radio_interface : 1;
	} set;
	struct {
		int8_t radio_interface;
	} data;
};

struct qmi_nas_get_tx_rx_info_response {
	struct {
		unsigned int rx_chain_0_info : 1;
		unsigned int rx_chain_1_info : 1;
		unsigned int tx_info : 1;
		unsigned int rx_chain_2_info : 1;
		unsigned int rx_chain_3_info : 1;
	} set;
	struct {
		struct {
			bool is_radio_tuned;
			int32_t rx_power;
			int32_t ecio;
			int32_t rscp;
			int32_t rsrp;
			uint32_t phase;
		} rx_chain_0_info;
		struct {
			bool is_radio_tuned;
			int32_t rx_power;
			int32_t ecio;
			int32_t rscp;
			int32_t rsrp;
			uint32_t phase;
		} rx_chain_1_info;
		struct {
			bool is_in_traffic;
			int32_t tx_power;
		} tx_info;
		struct {
			bool is_radio_tuned;
			int32_t rx_power;
			int32_t ecio;
			int32_t rscp;
			int32_t rsrp;
			uint32_t phase;
		} rx_chain_2_info;
		struct {
			bool is_radio_tuned;
			int32_t rx_power;
			int32_t ecio;
			int32_t rscp;
			int32_t rsrp;
			uint32_t phase;
		} rx_chain_3_info;
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
				uint32_t pilot_type;
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

struct qmi_nas_get_drx_response {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		uint32_t info;
	} data;
};

struct qmi_nas_get_lte_cphy_ca_info_response {
	struct {
		unsigned int dl_bandwidth : 1;
		unsigned int phy_ca_agg_scell_info : 1;
		unsigned int phy_ca_agg_pcell_info : 1;
		unsigned int scell_index : 1;
	} set;
	struct {
		uint32_t dl_bandwidth;
		struct {
			uint16_t physical_cell_id;
			uint16_t rx_channel;
			uint32_t dl_bandwidth;
			uint16_t lte_band;
			uint32_t state;
		} phy_ca_agg_scell_info;
		struct {
			uint16_t physical_cell_id;
			uint16_t rx_channel;
			uint32_t dl_bandwidth;
			uint16_t lte_band;
		} phy_ca_agg_pcell_info;
		uint8_t scell_index;
		unsigned int phy_ca_agg_secondary_cells_n;
		struct {
			uint16_t physical_cell_id;
			uint16_t rx_channel;
			uint32_t dl_bandwidth;
			uint16_t lte_band;
			uint32_t state;
			uint8_t cell_index;
		} *phy_ca_agg_secondary_cells;
	} data;
};

struct qmi_nas_swi_get_status_response {
	struct {
		unsigned int common_info_v2 : 1;
		unsigned int lte_info : 1;
	} set;
	struct {
		struct {
			int8_t temperature;
			uint8_t modem_mode;
			uint8_t system_mode;
			uint8_t ims_registration_state;
			uint8_t packet_service_state;
		} common_info_v2;
		struct {
			uint8_t band;
			uint8_t bandwidth;
			uint16_t rx_channel;
			uint16_t tx_channel;
			uint8_t emm_state;
			uint8_t emm_sub_state;
			uint8_t emm_connection_state;
		} lte_info;
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

int qmi_set_nas_get_supported_messages_request(struct qmi_msg *msg);
int qmi_parse_nas_get_supported_messages_response(struct qmi_msg *msg, struct qmi_nas_get_supported_messages_response *res);

int qmi_set_nas_get_signal_strength_request(struct qmi_msg *msg, struct qmi_nas_get_signal_strength_request *req);
int qmi_parse_nas_get_signal_strength_response(struct qmi_msg *msg, struct qmi_nas_get_signal_strength_response *res);

int qmi_set_nas_network_scan_request(struct qmi_msg *msg, struct qmi_nas_network_scan_request *req);
int qmi_parse_nas_network_scan_response(struct qmi_msg *msg, struct qmi_nas_network_scan_response *res);

int qmi_set_nas_initiate_network_register_request(struct qmi_msg *msg, struct qmi_nas_initiate_network_register_request *req);
int qmi_parse_nas_initiate_network_register_response(struct qmi_msg *msg);

int qmi_set_nas_attach_detach_request(struct qmi_msg *msg, struct qmi_nas_attach_detach_request *req);
int qmi_parse_nas_attach_detach_response(struct qmi_msg *msg);

int qmi_set_nas_get_serving_system_request(struct qmi_msg *msg);
int qmi_parse_nas_get_serving_system_response(struct qmi_msg *msg, struct qmi_nas_get_serving_system_response *res);

int qmi_set_nas_get_home_network_request(struct qmi_msg *msg);
int qmi_parse_nas_get_home_network_response(struct qmi_msg *msg, struct qmi_nas_get_home_network_response *res);

int qmi_set_nas_get_preferred_networks_request(struct qmi_msg *msg);
int qmi_parse_nas_get_preferred_networks_response(struct qmi_msg *msg, struct qmi_nas_get_preferred_networks_response *res);

int qmi_set_nas_set_preferred_networks_request(struct qmi_msg *msg, struct qmi_nas_set_preferred_networks_request *req);
int qmi_parse_nas_set_preferred_networks_response(struct qmi_msg *msg);

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

int qmi_set_nas_get_operator_name_request(struct qmi_msg *msg);
int qmi_parse_nas_get_operator_name_response(struct qmi_msg *msg, struct qmi_nas_get_operator_name_response *res);

int qmi_set_nas_get_cell_location_info_request(struct qmi_msg *msg);
int qmi_parse_nas_get_cell_location_info_response(struct qmi_msg *msg, struct qmi_nas_get_cell_location_info_response *res);

int qmi_set_nas_get_plmn_name_request(struct qmi_msg *msg, struct qmi_nas_get_plmn_name_request *req);
int qmi_parse_nas_get_plmn_name_response(struct qmi_msg *msg, struct qmi_nas_get_plmn_name_response *res);

int qmi_set_nas_get_system_info_request(struct qmi_msg *msg);
int qmi_parse_nas_get_system_info_response(struct qmi_msg *msg, struct qmi_nas_get_system_info_response *res);

int qmi_set_nas_get_signal_info_request(struct qmi_msg *msg);
int qmi_parse_nas_get_signal_info_response(struct qmi_msg *msg, struct qmi_nas_get_signal_info_response *res);

int qmi_set_nas_config_signal_info_request(struct qmi_msg *msg, struct qmi_nas_config_signal_info_request *req);
int qmi_parse_nas_config_signal_info_response(struct qmi_msg *msg);

int qmi_set_nas_get_tx_rx_info_request(struct qmi_msg *msg, struct qmi_nas_get_tx_rx_info_request *req);
int qmi_parse_nas_get_tx_rx_info_response(struct qmi_msg *msg, struct qmi_nas_get_tx_rx_info_response *res);

int qmi_set_nas_get_cdma_position_info_request(struct qmi_msg *msg);
int qmi_parse_nas_get_cdma_position_info_response(struct qmi_msg *msg, struct qmi_nas_get_cdma_position_info_response *res);

int qmi_set_nas_force_network_search_request(struct qmi_msg *msg);
int qmi_parse_nas_force_network_search_response(struct qmi_msg *msg);

int qmi_set_nas_get_drx_request(struct qmi_msg *msg);
int qmi_parse_nas_get_drx_response(struct qmi_msg *msg, struct qmi_nas_get_drx_response *res);

int qmi_set_nas_get_lte_cphy_ca_info_request(struct qmi_msg *msg);
int qmi_parse_nas_get_lte_cphy_ca_info_response(struct qmi_msg *msg, struct qmi_nas_get_lte_cphy_ca_info_response *res);

int qmi_set_nas_swi_get_status_request(struct qmi_msg *msg);
int qmi_parse_nas_swi_get_status_response(struct qmi_msg *msg, struct qmi_nas_swi_get_status_response *res);

