struct qmi_uim_get_supported_messages_response {
	struct {
	} set;
	struct {
		unsigned int list_n;
		uint8_t *list;
	} data;
};

struct qmi_uim_read_transparent_request {
	struct {
		unsigned int session : 1;
		unsigned int file : 1;
		unsigned int read_information : 1;
		unsigned int response_in_indication_token : 1;
		unsigned int encrypt_data : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			uint16_t file_id;
			unsigned int file_path_n;
			uint8_t *file_path;
		} file;
		struct {
			uint16_t offset;
			uint16_t length;
		} read_information;
		uint32_t response_in_indication_token;
		bool encrypt_data;
	} data;
};

struct qmi_uim_read_transparent_response {
	struct {
		unsigned int card_result : 1;
		unsigned int response_in_indication_token : 1;
		unsigned int encrypted_data : 1;
	} set;
	struct {
		struct {
			uint8_t sw1;
			uint8_t sw2;
		} card_result;
		unsigned int read_result_n;
		uint8_t *read_result;
		uint32_t response_in_indication_token;
		bool encrypted_data;
	} data;
};

struct qmi_uim_read_record_request {
	struct {
		unsigned int session : 1;
		unsigned int file : 1;
		unsigned int record : 1;
		unsigned int last_record : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			uint16_t file_id;
			unsigned int file_path_n;
			uint8_t *file_path;
		} file;
		struct {
			uint16_t record_number;
			uint16_t record_length;
		} record;
		uint16_t last_record;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_read_record_response {
	struct {
		unsigned int card_result : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t sw1;
			uint8_t sw2;
		} card_result;
		unsigned int read_result_n;
		uint8_t *read_result;
		unsigned int additional_read_result_n;
		uint8_t *additional_read_result;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_get_file_attributes_request {
	struct {
		unsigned int session : 1;
		unsigned int file : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			uint16_t file_id;
			unsigned int file_path_n;
			uint8_t *file_path;
		} file;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_get_file_attributes_response {
	struct {
		unsigned int card_result : 1;
		unsigned int file_attributes : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t sw1;
			uint8_t sw2;
		} card_result;
		struct {
			uint16_t file_size;
			uint16_t file_id;
			uint8_t file_type;
			uint16_t record_size;
			uint16_t record_count;
			uint8_t read_security_attributes_logic;
			uint16_t read_security_attributes;
			uint8_t write_security_attributes_logic;
			uint16_t write_security_attributes;
			uint8_t increase_security_attributes_logic;
			uint16_t increase_security_attributes;
			uint8_t deactivate_security_attributes_logic;
			uint16_t deactivate_security_attributes;
			uint8_t activate_security_attributes_logic;
			uint16_t activate_security_attributes;
			unsigned int raw_data_n;
			uint8_t *raw_data;
		} file_attributes;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_set_pin_protection_request {
	struct {
		unsigned int session : 1;
		unsigned int info : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			uint8_t pin_id;
			bool pin_enabled;
			char *pin_value;
		} info;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_set_pin_protection_response {
	struct {
		unsigned int retries_remaining : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} retries_remaining;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_verify_pin_request {
	struct {
		unsigned int session : 1;
		unsigned int info : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			uint8_t pin_id;
			char *pin_value;
		} info;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_verify_pin_response {
	struct {
		unsigned int retries_remaining : 1;
		unsigned int response_in_indication_token : 1;
		unsigned int card_result : 1;
	} set;
	struct {
		struct {
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} retries_remaining;
		uint32_t response_in_indication_token;
		struct {
			uint8_t sw1;
			uint8_t sw2;
		} card_result;
	} data;
};

struct qmi_uim_unblock_pin_request {
	struct {
		unsigned int session : 1;
		unsigned int info : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			uint8_t pin_id;
			char *puk;
			char *new_pin;
		} info;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_unblock_pin_response {
	struct {
		unsigned int retries_remaining : 1;
		unsigned int response_in_indication_token : 1;
		unsigned int card_result : 1;
	} set;
	struct {
		struct {
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} retries_remaining;
		uint32_t response_in_indication_token;
		struct {
			uint8_t sw1;
			uint8_t sw2;
		} card_result;
	} data;
};

struct qmi_uim_change_pin_request {
	struct {
		unsigned int session : 1;
		unsigned int info : 1;
		unsigned int response_in_indication_token : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			uint8_t pin_id;
			char *old_pin;
			char *new_pin;
		} info;
		uint32_t response_in_indication_token;
	} data;
};

struct qmi_uim_change_pin_response {
	struct {
		unsigned int retries_remaining : 1;
		unsigned int response_in_indication_token : 1;
		unsigned int card_result : 1;
	} set;
	struct {
		struct {
			uint8_t verify_retries_left;
			uint8_t unblock_retries_left;
		} retries_remaining;
		uint32_t response_in_indication_token;
		struct {
			uint8_t sw1;
			uint8_t sw2;
		} card_result;
	} data;
};

struct qmi_uim_refresh_register_request {
	struct {
		unsigned int session : 1;
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			bool register_flag;
			bool vote_for_init;
			unsigned int files_n;
			struct {
				uint16_t file_id;
				unsigned int path_n;
				uint8_t *path;
			} *files;
		} info;
	} data;
};

struct qmi_uim_refresh_complete_request {
	struct {
		unsigned int session : 1;
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			bool refresh_success;
		} info;
	} data;
};

struct qmi_uim_register_events_request {
	struct {
		unsigned int event_registration_mask : 1;
	} set;
	struct {
		uint32_t event_registration_mask;
	} data;
};

struct qmi_uim_register_events_response {
	struct {
		unsigned int event_registration_mask : 1;
	} set;
	struct {
		uint32_t event_registration_mask;
	} data;
};

struct qmi_uim_get_card_status_response {
	struct {
		unsigned int card_status : 1;
	} set;
	struct {
		struct {
			uint16_t index_gw_primary;
			uint16_t index_1x_primary;
			uint16_t index_gw_secondary;
			uint16_t index_1x_secondary;
			unsigned int cards_n;
			struct {
				uint8_t card_state;
				uint8_t upin_state;
				uint8_t upin_retries;
				uint8_t upuk_retries;
				uint8_t error_code;
				unsigned int applications_n;
				struct {
					uint8_t type;
					uint8_t state;
					uint8_t personalization_state;
					uint8_t personalization_feature;
					uint8_t personalization_retries;
					uint8_t personalization_unblock_retries;
					unsigned int application_identifier_value_n;
					uint8_t *application_identifier_value;
					bool upin_replaces_pin1;
					uint8_t pin1_state;
					uint8_t pin1_retries;
					uint8_t puk1_retries;
					uint8_t pin2_state;
					uint8_t pin2_retries;
					uint8_t puk2_retries;
				} *applications;
			} *cards;
		} card_status;
	} data;
};

struct qmi_uim_power_off_sim_request {
	struct {
		unsigned int slot : 1;
	} set;
	struct {
		uint8_t slot;
	} data;
};

struct qmi_uim_power_on_sim_request {
	struct {
		unsigned int slot : 1;
	} set;
	struct {
		uint8_t slot;
	} data;
};

struct qmi_uim_change_provisioning_session_request {
	struct {
		unsigned int session_change : 1;
		unsigned int application_information : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			bool activate;
		} session_change;
		struct {
			uint8_t slot;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} application_information;
	} data;
};

struct qmi_uim_depersonalization_request {
	struct {
		unsigned int info : 1;
		unsigned int slot : 1;
	} set;
	struct {
		struct {
			uint8_t feature;
			uint8_t operation;
			char *control_key;
		} info;
		uint8_t slot;
	} data;
};

struct qmi_uim_depersonalization_response {
	struct {
		unsigned int retries_remaining : 1;
	} set;
	struct {
		struct {
			uint8_t verify_left;
			uint8_t unblock_left;
		} retries_remaining;
	} data;
};

struct qmi_uim_get_configuration_request {
	struct {
		unsigned int configuration_mask : 1;
	} set;
	struct {
		uint32_t configuration_mask;
	} data;
};

struct qmi_uim_get_configuration_response {
	struct {
		unsigned int automatic_selection : 1;
		unsigned int halt_subscription : 1;
	} set;
	struct {
		bool automatic_selection;
		unsigned int personalization_status_n;
		struct {
			uint8_t feature;
			uint8_t verify_left;
			uint8_t unblock_left;
		} *personalization_status;
		bool halt_subscription;
		unsigned int personalization_status_other_slots_n;
		unsigned int personalization_status_other_slots_i;
		struct {
			uint8_t feature;
			uint8_t verify_left;
			uint8_t unblock_left;
		} **personalization_status_other_slots;
	} data;
};

struct qmi_uim_refresh_register_all_request {
	struct {
		unsigned int session : 1;
		unsigned int info : 1;
	} set;
	struct {
		struct {
			uint8_t session_type;
			unsigned int application_identifier_n;
			uint8_t *application_identifier;
		} session;
		struct {
			bool register_flag;
		} info;
	} data;
};

struct qmi_uim_switch_slot_request {
	struct {
		unsigned int logical_slot : 1;
		unsigned int physical_slot : 1;
	} set;
	struct {
		uint8_t logical_slot;
		uint32_t physical_slot;
	} data;
};

struct qmi_uim_get_slot_status_response {
	struct {
	} set;
	struct {
		unsigned int physical_slot_status_n;
		struct {
			uint32_t physical_card_status;
			uint32_t physical_slot_status;
			uint8_t logical_slot;
			unsigned int iccid_n;
			uint8_t *iccid;
		} *physical_slot_status;
		unsigned int physical_slot_information_n;
		struct {
			uint32_t card_protocol;
			uint8_t valid_applications;
			unsigned int atr_value_n;
			uint8_t *atr_value;
			bool is_euicc;
		} *physical_slot_information;
		unsigned int slot_eid_information_n;
		unsigned int slot_eid_information_i;
		uint8_t **slot_eid_information;
	} data;
};

int qmi_set_uim_reset_request(struct qmi_msg *msg);
int qmi_parse_uim_reset_response(struct qmi_msg *msg);

int qmi_set_uim_get_supported_messages_request(struct qmi_msg *msg);
int qmi_parse_uim_get_supported_messages_response(struct qmi_msg *msg, struct qmi_uim_get_supported_messages_response *res);

int qmi_set_uim_read_transparent_request(struct qmi_msg *msg, struct qmi_uim_read_transparent_request *req);
int qmi_parse_uim_read_transparent_response(struct qmi_msg *msg, struct qmi_uim_read_transparent_response *res);

int qmi_set_uim_read_record_request(struct qmi_msg *msg, struct qmi_uim_read_record_request *req);
int qmi_parse_uim_read_record_response(struct qmi_msg *msg, struct qmi_uim_read_record_response *res);

int qmi_set_uim_get_file_attributes_request(struct qmi_msg *msg, struct qmi_uim_get_file_attributes_request *req);
int qmi_parse_uim_get_file_attributes_response(struct qmi_msg *msg, struct qmi_uim_get_file_attributes_response *res);

int qmi_set_uim_set_pin_protection_request(struct qmi_msg *msg, struct qmi_uim_set_pin_protection_request *req);
int qmi_parse_uim_set_pin_protection_response(struct qmi_msg *msg, struct qmi_uim_set_pin_protection_response *res);

int qmi_set_uim_verify_pin_request(struct qmi_msg *msg, struct qmi_uim_verify_pin_request *req);
int qmi_parse_uim_verify_pin_response(struct qmi_msg *msg, struct qmi_uim_verify_pin_response *res);

int qmi_set_uim_unblock_pin_request(struct qmi_msg *msg, struct qmi_uim_unblock_pin_request *req);
int qmi_parse_uim_unblock_pin_response(struct qmi_msg *msg, struct qmi_uim_unblock_pin_response *res);

int qmi_set_uim_change_pin_request(struct qmi_msg *msg, struct qmi_uim_change_pin_request *req);
int qmi_parse_uim_change_pin_response(struct qmi_msg *msg, struct qmi_uim_change_pin_response *res);

int qmi_set_uim_refresh_register_request(struct qmi_msg *msg, struct qmi_uim_refresh_register_request *req);
int qmi_parse_uim_refresh_register_response(struct qmi_msg *msg);

int qmi_set_uim_refresh_complete_request(struct qmi_msg *msg, struct qmi_uim_refresh_complete_request *req);
int qmi_parse_uim_refresh_complete_response(struct qmi_msg *msg);

int qmi_set_uim_register_events_request(struct qmi_msg *msg, struct qmi_uim_register_events_request *req);
int qmi_parse_uim_register_events_response(struct qmi_msg *msg, struct qmi_uim_register_events_response *res);

int qmi_set_uim_get_card_status_request(struct qmi_msg *msg);
int qmi_parse_uim_get_card_status_response(struct qmi_msg *msg, struct qmi_uim_get_card_status_response *res);

int qmi_set_uim_power_off_sim_request(struct qmi_msg *msg, struct qmi_uim_power_off_sim_request *req);
int qmi_parse_uim_power_off_sim_response(struct qmi_msg *msg);

int qmi_set_uim_power_on_sim_request(struct qmi_msg *msg, struct qmi_uim_power_on_sim_request *req);
int qmi_parse_uim_power_on_sim_response(struct qmi_msg *msg);

int qmi_set_uim_change_provisioning_session_request(struct qmi_msg *msg, struct qmi_uim_change_provisioning_session_request *req);
int qmi_parse_uim_change_provisioning_session_response(struct qmi_msg *msg);

int qmi_set_uim_depersonalization_request(struct qmi_msg *msg, struct qmi_uim_depersonalization_request *req);
int qmi_parse_uim_depersonalization_response(struct qmi_msg *msg, struct qmi_uim_depersonalization_response *res);

int qmi_set_uim_get_configuration_request(struct qmi_msg *msg, struct qmi_uim_get_configuration_request *req);
int qmi_parse_uim_get_configuration_response(struct qmi_msg *msg, struct qmi_uim_get_configuration_response *res);

int qmi_set_uim_refresh_register_all_request(struct qmi_msg *msg, struct qmi_uim_refresh_register_all_request *req);
int qmi_parse_uim_refresh_register_all_response(struct qmi_msg *msg);

int qmi_set_uim_switch_slot_request(struct qmi_msg *msg, struct qmi_uim_switch_slot_request *req);
int qmi_parse_uim_switch_slot_response(struct qmi_msg *msg);

int qmi_set_uim_get_slot_status_request(struct qmi_msg *msg);
int qmi_parse_uim_get_slot_status_response(struct qmi_msg *msg, struct qmi_uim_get_slot_status_response *res);

