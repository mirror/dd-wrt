struct qmi_dms_set_event_report_request {
	struct {
		unsigned int power_state_reporting : 1;
		unsigned int battery_level_report_limits : 1;
		unsigned int pin_state_reporting : 1;
		unsigned int activation_state_reporting : 1;
		unsigned int operating_mode_reporting : 1;
		unsigned int uim_state_reporting : 1;
		unsigned int wireless_disable_state_reporting : 1;
		unsigned int prl_init_reporting : 1;
	} set;
	struct {
		bool power_state_reporting;
		struct {
			uint8_t lower_limit;
			uint8_t upper_limit;
		} battery_level_report_limits;
		bool pin_state_reporting;
		bool activation_state_reporting;
		bool operating_mode_reporting;
		bool uim_state_reporting;
		bool wireless_disable_state_reporting;
		bool prl_init_reporting;
	} data;
};

struct qmi_dms_get_capabilities_response {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint32_t max_tx_channel_rate;
			uint32_t max_rx_channel_rate;
			uint8_t data_service_capability;
			uint8_t sim_capability;
			unsigned int radio_interface_list_n;
			uint8_t *radio_interface_list;
		} info;
	} data;
};

struct qmi_dms_get_manufacturer_response {
	struct {
	} set;
	struct {
		char *manufacturer;
	} data;
};

struct qmi_dms_get_model_response {
	struct {
	} set;
	struct {
		char *model;
	} data;
};

struct qmi_dms_get_revision_response {
	struct {
	} set;
	struct {
		char *revision;
	} data;
};

struct qmi_dms_get_msisdn_response {
	struct {
	} set;
	struct {
		char *msisdn;
	} data;
};

struct qmi_dms_get_ids_response {
	struct {
	} set;
	struct {
		char *esn;
		char *imei;
		char *meid;
		char *imei_software_version;
	} data;
};

struct qmi_dms_get_power_state_response {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint8_t power_state_flags;
			uint8_t battery_level;
		} info;
	} data;
};

struct qmi_dms_uim_set_pin_protection_request {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint8_t pin_id;
			bool protection_enabled;
			char *pin;
		} info;
	} data;
};

struct qmi_dms_uim_set_pin_protection_response {
	struct {
		unsigned int pin_retries_status : 1;
	} set;
	struct {
		struct {
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} pin_retries_status;
	} data;
};

struct qmi_dms_uim_verify_pin_request {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint8_t pin_id;
			char *pin;
		} info;
	} data;
};

struct qmi_dms_uim_verify_pin_response {
	struct {
		unsigned int pin_retries_status : 1;
	} set;
	struct {
		struct {
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} pin_retries_status;
	} data;
};

struct qmi_dms_uim_unblock_pin_request {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint8_t pin_id;
			char *puk;
			char *new_pin;
		} info;
	} data;
};

struct qmi_dms_uim_unblock_pin_response {
	struct {
		unsigned int pin_retries_status : 1;
	} set;
	struct {
		struct {
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} pin_retries_status;
	} data;
};

struct qmi_dms_uim_change_pin_request {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint8_t pin_id;
			char *old_pin;
			char *new_pin;
		} info;
	} data;
};

struct qmi_dms_uim_change_pin_response {
	struct {
		unsigned int pin_retries_status : 1;
	} set;
	struct {
		struct {
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} pin_retries_status;
	} data;
};

struct qmi_dms_uim_get_pin_status_response {
	struct {
		unsigned int pin1_status : 1;
		unsigned int pin2_status : 1;
	} set;
	struct {
		struct {
			uint8_t current_status;
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} pin1_status;
		struct {
			uint8_t current_status;
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} pin2_status;
	} data;
};

struct qmi_dms_get_hardware_revision_response {
	struct {
	} set;
	struct {
		char *revision;
	} data;
};

struct qmi_dms_get_operating_mode_response {
	struct {
		unsigned int mode : 1;
		unsigned int offline_reason : 1;
		unsigned int hardware_restricted_mode : 1;
	} set;
	struct {
		uint8_t mode;
		uint16_t offline_reason;
		bool hardware_restricted_mode;
	} data;
};

struct qmi_dms_set_operating_mode_request {
	struct {
		unsigned int mode : 1;
	} set;
	struct {
		uint8_t mode;
	} data;
};

struct qmi_dms_get_time_response {
	struct {
		unsigned int device_time : 1;
		unsigned int system_time : 1;
		unsigned int user_time : 1;
	} set;
	struct {
		struct {
			uint64_t time_count;
			uint16_t time_source;
		} device_time;
		uint64_t system_time;
		uint64_t user_time;
	} data;
};

struct qmi_dms_get_prl_version_response {
	struct {
		unsigned int version : 1;
		unsigned int prl_only_preference : 1;
	} set;
	struct {
		uint16_t version;
		bool prl_only_preference;
	} data;
};

struct qmi_dms_get_activation_state_response {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		uint16_t info;
	} data;
};

struct qmi_dms_activate_automatic_request {
	struct {
	} set;
	struct {
		char *activation_code;
	} data;
};

struct qmi_dms_activate_manual_request {
	struct {
		unsigned int info : 1;
		unsigned int prl : 1;
	} set;
	struct {
		struct {
			char *service_programming_code;
			uint16_t system_identification_number;
			char *mobile_directory_number;
			char *mobile_identification_number;
		} info;
		char *mn_ha_key;
		char *mn_aaa_key;
		struct {
			uint16_t prl_total_length;
			unsigned int prl_segment_n;
			uint8_t *prl_segment;
		} prl;
	} data;
};

struct qmi_dms_get_user_lock_state_response {
	struct {
		unsigned int enabled : 1;
	} set;
	struct {
		bool enabled;
	} data;
};

struct qmi_dms_set_user_lock_state_request {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		struct {
			bool enabled;
			char *lock_code;
		} info;
	} data;
};

struct qmi_dms_set_user_lock_code_request {
	struct {
		unsigned int info : 1;
	} set;
	struct {
		struct {
			char *old_code;
			char *new_code;
		} info;
	} data;
};

struct qmi_dms_read_user_data_response {
	struct {
	} set;
	struct {
		unsigned int user_data_n;
		uint8_t *user_data;
	} data;
};

struct qmi_dms_write_user_data_request {
	struct {
	} set;
	struct {
		unsigned int user_data_n;
		uint8_t *user_data;
	} data;
};

struct qmi_dms_read_eri_file_response {
	struct {
	} set;
	struct {
		unsigned int eri_file_n;
		uint8_t *eri_file;
	} data;
};

struct qmi_dms_restore_factory_defaults_request {
	struct {
	} set;
	struct {
		char *service_programming_code;
	} data;
};

struct qmi_dms_validate_service_programming_code_request {
	struct {
	} set;
	struct {
		char *service_programming_code;
	} data;
};

struct qmi_dms_uim_get_iccid_response {
	struct {
	} set;
	struct {
		char *iccid;
	} data;
};

struct qmi_dms_uim_get_ck_status_request {
	struct {
		unsigned int facility : 1;
	} set;
	struct {
		uint8_t facility;
	} data;
};

struct qmi_dms_uim_get_ck_status_response {
	struct {
		unsigned int ck_status : 1;
		unsigned int operation_blocking_facility : 1;
	} set;
	struct {
		struct {
			uint8_t facility_state;
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} ck_status;
		bool operation_blocking_facility;
	} data;
};

struct qmi_dms_uim_set_ck_protection_request {
	struct {
		unsigned int facility : 1;
	} set;
	struct {
		struct {
			uint8_t facility;
			uint8_t facility_state;
			char *facility_depersonalization_control_key;
		} facility;
	} data;
};

struct qmi_dms_uim_set_ck_protection_response {
	struct {
		unsigned int verify_retries_left : 1;
	} set;
	struct {
		uint8_t verify_retries_left;
	} data;
};

struct qmi_dms_uim_unblock_ck_request {
	struct {
		unsigned int facility : 1;
	} set;
	struct {
		struct {
			uint8_t facility;
			char *facility_control_key;
		} facility;
	} data;
};

struct qmi_dms_uim_unblock_ck_response {
	struct {
		unsigned int unblock_retries_left : 1;
	} set;
	struct {
		uint8_t unblock_retries_left;
	} data;
};

struct qmi_dms_uim_get_imsi_response {
	struct {
	} set;
	struct {
		char *imsi;
	} data;
};

struct qmi_dms_uim_get_state_response {
	struct {
		unsigned int state : 1;
	} set;
	struct {
		uint8_t state;
	} data;
};

struct qmi_dms_get_band_capabilities_response {
	struct {
		unsigned int band_capability : 1;
		unsigned int lte_band_capability : 1;
	} set;
	struct {
		uint64_t band_capability;
		uint64_t lte_band_capability;
		unsigned int extended_lte_band_capability_n;
		uint16_t *extended_lte_band_capability;
	} data;
};

struct qmi_dms_get_factory_sku_response {
	struct {
	} set;
	struct {
		char *sku;
	} data;
};

struct qmi_dms_get_firmware_preference_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		struct {
			uint8_t type;
			uint8_t unique_id[16];
			char *build_id;
		} *list;
	} data;
};

struct qmi_dms_set_firmware_preference_request {
	struct {
		unsigned int download_override : 1;
		unsigned int modem_storage_index : 1;
	} set;
	struct {
		unsigned int list_n;
		struct {
			uint8_t type;
			uint8_t unique_id[16];
			char *build_id;
		} *list;
		bool download_override;
		uint8_t modem_storage_index;
	} data;
};

struct qmi_dms_set_firmware_preference_response {
	struct {
	} set;
	struct {
		unsigned int image_download_list_n;
		uint8_t *image_download_list;
	} data;
};

struct qmi_dms_list_stored_images_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		struct {
			uint8_t type;
			uint8_t maximum_images;
			uint8_t index_of_running_image;
			unsigned int sublist_n;
			struct {
				uint8_t storage_index;
				uint8_t failure_count;
				uint8_t unique_id[16];
				char *build_id;
			} *sublist;
		} *list;
	} data;
};

struct qmi_dms_delete_stored_image_request {
	struct {
		unsigned int image : 1;
	} set;
	struct {
		struct {
			uint8_t type;
			uint8_t unique_id[16];
			char *build_id;
		} image;
	} data;
};

struct qmi_dms_set_time_request {
	struct {
		unsigned int time_value : 1;
		unsigned int time_reference_type : 1;
	} set;
	struct {
		uint64_t time_value;
		uint32_t time_reference_type;
	} data;
};

struct qmi_dms_get_stored_image_info_request {
	struct {
		unsigned int image : 1;
	} set;
	struct {
		struct {
			uint8_t type;
			uint8_t unique_id[16];
			char *build_id;
		} image;
	} data;
};

struct qmi_dms_get_stored_image_info_response {
	struct {
		unsigned int boot_version : 1;
		unsigned int pri_version : 1;
		unsigned int oem_lock_id : 1;
	} set;
	struct {
		struct {
			uint16_t major_version;
			uint16_t minor_version;
		} boot_version;
		struct {
			uint32_t pri_version;
			char *pri_info;
		} pri_version;
		uint32_t oem_lock_id;
	} data;
};

struct qmi_dms_get_alt_net_config_response {
	struct {
		unsigned int config : 1;
	} set;
	struct {
		bool config;
	} data;
};

struct qmi_dms_set_alt_net_config_request {
	struct {
		unsigned int config : 1;
	} set;
	struct {
		bool config;
	} data;
};

struct qmi_dms_get_boot_image_download_mode_response {
	struct {
		unsigned int mode : 1;
	} set;
	struct {
		uint8_t mode;
	} data;
};

struct qmi_dms_set_boot_image_download_mode_request {
	struct {
		unsigned int mode : 1;
	} set;
	struct {
		uint8_t mode;
	} data;
};

struct qmi_dms_get_software_version_response {
	struct {
	} set;
	struct {
		char *version;
	} data;
};

struct qmi_dms_set_service_programming_code_request {
	struct {
	} set;
	struct {
		char *current_code;
		char *new_code;
	} data;
};

struct qmi_dms_get_mac_address_request {
	struct {
		unsigned int device : 1;
	} set;
	struct {
		uint32_t device;
	} data;
};

struct qmi_dms_get_mac_address_response {
	struct {
	} set;
	struct {
		unsigned int mac_address_n;
		uint8_t *mac_address;
	} data;
};

struct qmi_dms_get_supported_messages_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		uint8_t *list;
	} data;
};

struct qmi_dms_hp_change_device_mode_request {
	struct {
		unsigned int mode : 1;
	} set;
	struct {
		uint8_t mode;
	} data;
};

struct qmi_dms_swi_get_current_firmware_response {
	struct {
	} set;
	struct {
		char *model;
		char *boot_version;
		char *amss_version;
		char *sku_id;
		char *package_id;
		char *carrier_id;
		char *pri_version;
		char *carrier;
		char *config_version;
	} data;
};

struct qmi_dms_swi_get_usb_composition_response {
	struct {
		unsigned int current : 1;
	} set;
	struct {
		uint8_t current;
		unsigned int supported_n;
		uint8_t *supported;
	} data;
};

struct qmi_dms_swi_set_usb_composition_request {
	struct {
		unsigned int current : 1;
	} set;
	struct {
		uint8_t current;
	} data;
};

struct qmi_dms_foxconn_get_firmware_version_request {
	struct {
		unsigned int version_type : 1;
	} set;
	struct {
		uint8_t version_type;
	} data;
};

struct qmi_dms_foxconn_get_firmware_version_response {
	struct {
	} set;
	struct {
		char *version;
	} data;
};

struct qmi_dms_foxconn_change_device_mode_request {
	struct {
		unsigned int mode : 1;
	} set;
	struct {
		uint8_t mode;
	} data;
};

struct qmi_dms_foxconn_set_fcc_authentication_request {
	struct {
		unsigned int value : 1;
	} set;
	struct {
		uint8_t value;
	} data;
};

int qmi_set_dms_reset_request(struct qmi_msg *msg);
int qmi_parse_dms_reset_response(struct qmi_msg *msg);

int qmi_set_dms_set_event_report_request(struct qmi_msg *msg, struct qmi_dms_set_event_report_request *req);
int qmi_parse_dms_set_event_report_response(struct qmi_msg *msg);

int qmi_set_dms_get_capabilities_request(struct qmi_msg *msg);
int qmi_parse_dms_get_capabilities_response(struct qmi_msg *msg, struct qmi_dms_get_capabilities_response *res);

int qmi_set_dms_get_manufacturer_request(struct qmi_msg *msg);
int qmi_parse_dms_get_manufacturer_response(struct qmi_msg *msg, struct qmi_dms_get_manufacturer_response *res);

int qmi_set_dms_get_model_request(struct qmi_msg *msg);
int qmi_parse_dms_get_model_response(struct qmi_msg *msg, struct qmi_dms_get_model_response *res);

int qmi_set_dms_get_revision_request(struct qmi_msg *msg);
int qmi_parse_dms_get_revision_response(struct qmi_msg *msg, struct qmi_dms_get_revision_response *res);

int qmi_set_dms_get_msisdn_request(struct qmi_msg *msg);
int qmi_parse_dms_get_msisdn_response(struct qmi_msg *msg, struct qmi_dms_get_msisdn_response *res);

int qmi_set_dms_get_ids_request(struct qmi_msg *msg);
int qmi_parse_dms_get_ids_response(struct qmi_msg *msg, struct qmi_dms_get_ids_response *res);

int qmi_set_dms_get_power_state_request(struct qmi_msg *msg);
int qmi_parse_dms_get_power_state_response(struct qmi_msg *msg, struct qmi_dms_get_power_state_response *res);

int qmi_set_dms_uim_set_pin_protection_request(struct qmi_msg *msg, struct qmi_dms_uim_set_pin_protection_request *req);
int qmi_parse_dms_uim_set_pin_protection_response(struct qmi_msg *msg, struct qmi_dms_uim_set_pin_protection_response *res);

int qmi_set_dms_uim_verify_pin_request(struct qmi_msg *msg, struct qmi_dms_uim_verify_pin_request *req);
int qmi_parse_dms_uim_verify_pin_response(struct qmi_msg *msg, struct qmi_dms_uim_verify_pin_response *res);

int qmi_set_dms_uim_unblock_pin_request(struct qmi_msg *msg, struct qmi_dms_uim_unblock_pin_request *req);
int qmi_parse_dms_uim_unblock_pin_response(struct qmi_msg *msg, struct qmi_dms_uim_unblock_pin_response *res);

int qmi_set_dms_uim_change_pin_request(struct qmi_msg *msg, struct qmi_dms_uim_change_pin_request *req);
int qmi_parse_dms_uim_change_pin_response(struct qmi_msg *msg, struct qmi_dms_uim_change_pin_response *res);

int qmi_set_dms_uim_get_pin_status_request(struct qmi_msg *msg);
int qmi_parse_dms_uim_get_pin_status_response(struct qmi_msg *msg, struct qmi_dms_uim_get_pin_status_response *res);

int qmi_set_dms_get_hardware_revision_request(struct qmi_msg *msg);
int qmi_parse_dms_get_hardware_revision_response(struct qmi_msg *msg, struct qmi_dms_get_hardware_revision_response *res);

int qmi_set_dms_get_operating_mode_request(struct qmi_msg *msg);
int qmi_parse_dms_get_operating_mode_response(struct qmi_msg *msg, struct qmi_dms_get_operating_mode_response *res);

int qmi_set_dms_set_operating_mode_request(struct qmi_msg *msg, struct qmi_dms_set_operating_mode_request *req);
int qmi_parse_dms_set_operating_mode_response(struct qmi_msg *msg);

int qmi_set_dms_get_time_request(struct qmi_msg *msg);
int qmi_parse_dms_get_time_response(struct qmi_msg *msg, struct qmi_dms_get_time_response *res);

int qmi_set_dms_get_prl_version_request(struct qmi_msg *msg);
int qmi_parse_dms_get_prl_version_response(struct qmi_msg *msg, struct qmi_dms_get_prl_version_response *res);

int qmi_set_dms_get_activation_state_request(struct qmi_msg *msg);
int qmi_parse_dms_get_activation_state_response(struct qmi_msg *msg, struct qmi_dms_get_activation_state_response *res);

int qmi_set_dms_activate_automatic_request(struct qmi_msg *msg, struct qmi_dms_activate_automatic_request *req);
int qmi_parse_dms_activate_automatic_response(struct qmi_msg *msg);

int qmi_set_dms_activate_manual_request(struct qmi_msg *msg, struct qmi_dms_activate_manual_request *req);
int qmi_parse_dms_activate_manual_response(struct qmi_msg *msg);

int qmi_set_dms_get_user_lock_state_request(struct qmi_msg *msg);
int qmi_parse_dms_get_user_lock_state_response(struct qmi_msg *msg, struct qmi_dms_get_user_lock_state_response *res);

int qmi_set_dms_set_user_lock_state_request(struct qmi_msg *msg, struct qmi_dms_set_user_lock_state_request *req);
int qmi_parse_dms_set_user_lock_state_response(struct qmi_msg *msg);

int qmi_set_dms_set_user_lock_code_request(struct qmi_msg *msg, struct qmi_dms_set_user_lock_code_request *req);
int qmi_parse_dms_set_user_lock_code_response(struct qmi_msg *msg);

int qmi_set_dms_read_user_data_request(struct qmi_msg *msg);
int qmi_parse_dms_read_user_data_response(struct qmi_msg *msg, struct qmi_dms_read_user_data_response *res);

int qmi_set_dms_write_user_data_request(struct qmi_msg *msg, struct qmi_dms_write_user_data_request *req);
int qmi_parse_dms_write_user_data_response(struct qmi_msg *msg);

int qmi_set_dms_read_eri_file_request(struct qmi_msg *msg);
int qmi_parse_dms_read_eri_file_response(struct qmi_msg *msg, struct qmi_dms_read_eri_file_response *res);

int qmi_set_dms_restore_factory_defaults_request(struct qmi_msg *msg, struct qmi_dms_restore_factory_defaults_request *req);
int qmi_parse_dms_restore_factory_defaults_response(struct qmi_msg *msg);

int qmi_set_dms_validate_service_programming_code_request(struct qmi_msg *msg, struct qmi_dms_validate_service_programming_code_request *req);
int qmi_parse_dms_validate_service_programming_code_response(struct qmi_msg *msg);

int qmi_set_dms_uim_get_iccid_request(struct qmi_msg *msg);
int qmi_parse_dms_uim_get_iccid_response(struct qmi_msg *msg, struct qmi_dms_uim_get_iccid_response *res);

int qmi_set_dms_set_firmware_id_request(struct qmi_msg *msg);
int qmi_parse_dms_set_firmware_id_response(struct qmi_msg *msg);

int qmi_set_dms_uim_get_ck_status_request(struct qmi_msg *msg, struct qmi_dms_uim_get_ck_status_request *req);
int qmi_parse_dms_uim_get_ck_status_response(struct qmi_msg *msg, struct qmi_dms_uim_get_ck_status_response *res);

int qmi_set_dms_uim_set_ck_protection_request(struct qmi_msg *msg, struct qmi_dms_uim_set_ck_protection_request *req);
int qmi_parse_dms_uim_set_ck_protection_response(struct qmi_msg *msg, struct qmi_dms_uim_set_ck_protection_response *res);

int qmi_set_dms_uim_unblock_ck_request(struct qmi_msg *msg, struct qmi_dms_uim_unblock_ck_request *req);
int qmi_parse_dms_uim_unblock_ck_response(struct qmi_msg *msg, struct qmi_dms_uim_unblock_ck_response *res);

int qmi_set_dms_uim_get_imsi_request(struct qmi_msg *msg);
int qmi_parse_dms_uim_get_imsi_response(struct qmi_msg *msg, struct qmi_dms_uim_get_imsi_response *res);

int qmi_set_dms_uim_get_state_request(struct qmi_msg *msg);
int qmi_parse_dms_uim_get_state_response(struct qmi_msg *msg, struct qmi_dms_uim_get_state_response *res);

int qmi_set_dms_get_band_capabilities_request(struct qmi_msg *msg);
int qmi_parse_dms_get_band_capabilities_response(struct qmi_msg *msg, struct qmi_dms_get_band_capabilities_response *res);

int qmi_set_dms_get_factory_sku_request(struct qmi_msg *msg);
int qmi_parse_dms_get_factory_sku_response(struct qmi_msg *msg, struct qmi_dms_get_factory_sku_response *res);

int qmi_set_dms_get_firmware_preference_request(struct qmi_msg *msg);
int qmi_parse_dms_get_firmware_preference_response(struct qmi_msg *msg, struct qmi_dms_get_firmware_preference_response *res);

int qmi_set_dms_set_firmware_preference_request(struct qmi_msg *msg, struct qmi_dms_set_firmware_preference_request *req);
int qmi_parse_dms_set_firmware_preference_response(struct qmi_msg *msg, struct qmi_dms_set_firmware_preference_response *res);

int qmi_set_dms_list_stored_images_request(struct qmi_msg *msg);
int qmi_parse_dms_list_stored_images_response(struct qmi_msg *msg, struct qmi_dms_list_stored_images_response *res);

int qmi_set_dms_delete_stored_image_request(struct qmi_msg *msg, struct qmi_dms_delete_stored_image_request *req);
int qmi_parse_dms_delete_stored_image_response(struct qmi_msg *msg);

int qmi_set_dms_set_time_request(struct qmi_msg *msg, struct qmi_dms_set_time_request *req);
int qmi_parse_dms_set_time_response(struct qmi_msg *msg);

int qmi_set_dms_get_stored_image_info_request(struct qmi_msg *msg, struct qmi_dms_get_stored_image_info_request *req);
int qmi_parse_dms_get_stored_image_info_response(struct qmi_msg *msg, struct qmi_dms_get_stored_image_info_response *res);

int qmi_set_dms_get_alt_net_config_request(struct qmi_msg *msg);
int qmi_parse_dms_get_alt_net_config_response(struct qmi_msg *msg, struct qmi_dms_get_alt_net_config_response *res);

int qmi_set_dms_set_alt_net_config_request(struct qmi_msg *msg, struct qmi_dms_set_alt_net_config_request *req);
int qmi_parse_dms_set_alt_net_config_response(struct qmi_msg *msg);

int qmi_set_dms_get_boot_image_download_mode_request(struct qmi_msg *msg);
int qmi_parse_dms_get_boot_image_download_mode_response(struct qmi_msg *msg, struct qmi_dms_get_boot_image_download_mode_response *res);

int qmi_set_dms_set_boot_image_download_mode_request(struct qmi_msg *msg, struct qmi_dms_set_boot_image_download_mode_request *req);
int qmi_parse_dms_set_boot_image_download_mode_response(struct qmi_msg *msg);

int qmi_set_dms_get_software_version_request(struct qmi_msg *msg);
int qmi_parse_dms_get_software_version_response(struct qmi_msg *msg, struct qmi_dms_get_software_version_response *res);

int qmi_set_dms_set_service_programming_code_request(struct qmi_msg *msg, struct qmi_dms_set_service_programming_code_request *req);
int qmi_parse_dms_set_service_programming_code_response(struct qmi_msg *msg);

int qmi_set_dms_get_mac_address_request(struct qmi_msg *msg, struct qmi_dms_get_mac_address_request *req);
int qmi_parse_dms_get_mac_address_response(struct qmi_msg *msg, struct qmi_dms_get_mac_address_response *res);

int qmi_set_dms_get_supported_messages_request(struct qmi_msg *msg);
int qmi_parse_dms_get_supported_messages_response(struct qmi_msg *msg, struct qmi_dms_get_supported_messages_response *res);

int qmi_set_dms_hp_change_device_mode_request(struct qmi_msg *msg, struct qmi_dms_hp_change_device_mode_request *req);
int qmi_parse_dms_hp_change_device_mode_response(struct qmi_msg *msg);

int qmi_set_dms_swi_get_current_firmware_request(struct qmi_msg *msg);
int qmi_parse_dms_swi_get_current_firmware_response(struct qmi_msg *msg, struct qmi_dms_swi_get_current_firmware_response *res);

int qmi_set_dms_swi_get_usb_composition_request(struct qmi_msg *msg);
int qmi_parse_dms_swi_get_usb_composition_response(struct qmi_msg *msg, struct qmi_dms_swi_get_usb_composition_response *res);

int qmi_set_dms_swi_set_usb_composition_request(struct qmi_msg *msg, struct qmi_dms_swi_set_usb_composition_request *req);
int qmi_parse_dms_swi_set_usb_composition_response(struct qmi_msg *msg);

int qmi_set_dms_foxconn_get_firmware_version_request(struct qmi_msg *msg, struct qmi_dms_foxconn_get_firmware_version_request *req);
int qmi_parse_dms_foxconn_get_firmware_version_response(struct qmi_msg *msg, struct qmi_dms_foxconn_get_firmware_version_response *res);

int qmi_set_dms_set_fcc_authentication_request(struct qmi_msg *msg);
int qmi_parse_dms_set_fcc_authentication_response(struct qmi_msg *msg);

int qmi_set_dms_foxconn_change_device_mode_request(struct qmi_msg *msg, struct qmi_dms_foxconn_change_device_mode_request *req);
int qmi_parse_dms_foxconn_change_device_mode_response(struct qmi_msg *msg);

int qmi_set_dms_foxconn_set_fcc_authentication_request(struct qmi_msg *msg, struct qmi_dms_foxconn_set_fcc_authentication_request *req);
int qmi_parse_dms_foxconn_set_fcc_authentication_response(struct qmi_msg *msg);

