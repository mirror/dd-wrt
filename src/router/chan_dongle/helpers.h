/*
   Copyright (C) 2010,2011 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_HELPERS_H_INCLUDED
#define CHAN_DONGLE_HELPERS_H_INCLUDED

#include "export.h"			/* EXPORT_DECL EXPORT_DEF */
#include "dc_config.h"			/* call_waiting_t */
#include "chan_dongle.h"		/* restate_time_t */

EXPORT_DECL int get_at_clir_value (struct pvt* pvt, int clir);

/* return status string of sending, status arg is optional */
EXPORT_DECL const char * send_ussd(const char * dev_name, const char* ussd, int * status, void ** id);
EXPORT_DECL const char * send_sms(const char * dev_name, const char* number, const char* message, const char * validity, const char * report, int * status, void ** id);
EXPORT_DECL const char * send_pdu(const char * dev_name, const char * pdu, int * status, void ** id);
EXPORT_DECL const char * send_reset(const char * dev_name, int * status);
EXPORT_DECL const char * send_ccwa_set(const char * dev_name, call_waiting_t enable, int * status);
EXPORT_DECL const char * send_at_command(const char * dev_name, const char* command);
EXPORT_DECL const char * schedule_restart_event(dev_state_t event, restate_time_t when, const char * dev_name, int * status);
EXPORT_DECL int is_valid_phone_number(const char * number);

#endif /* CHAN_DONGLE_HELPERS_H_INCLUDED */
