/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_MANAGER_H_INCLUDED
#define CHAN_DONGLE_MANAGER_H_INCLUDED

#ifdef BUILD_MANAGER

#include "export.h"			/* EXPORT_DECL EXPORT_DEF */

EXPORT_DECL void manager_register();
EXPORT_DECL void manager_unregister();

EXPORT_DECL void manager_event_message(const char * event, const char * devname, const char * message);
EXPORT_DECL void manager_event_message_raw(const char * event, const char * devname, const char * message);

EXPORT_DECL void manager_event_new_ussd(const char * devname, char * message);
EXPORT_DECL void manager_event_new_sms(const char * devname, char * number, char * message);
EXPORT_DECL void manager_event_new_sms_base64 (const char * devname, char * number, char * message_base64);
EXPORT_DECL void manager_event_cend(const char * devname, int call_index, int duration, int end_status, int cc_cause);
EXPORT_DECL void manager_event_call_state_change(const char * devname, int call_index, const char * newstate);
EXPORT_DECL void manager_event_device_status(const char * devname, const char * newstatus);
EXPORT_DECL void manager_event_sent_notify(const char * devname, const char * type, const void * id, const char * result);

#else  /* BUILD_MANAGER */

#define manager_register()
#define manager_unregister()

#define manager_event_message(event, devname, message)
#define manager_event_message_raw(event, devname, message)
#define manager_event_new_ussd(devname, message)
#define manager_event_new_sms(devname, number, message)
#define manager_event_new_sms_base64(devname, number, message_base64)
#define manager_event_cend(devname, call_index, duration, end_status, cc_cause)
#define manager_event_call_state_change(devname, call_index, newstate)
#define manager_event_device_status(devname, newstatus)
#define manager_event_sent_notify(devname, type, id, result)

#endif /* BUILD_MANAGER */

#endif /* CHAN_DONGLE_MANAGER_H_INCLUDED */
