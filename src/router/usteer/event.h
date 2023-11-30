#ifndef __USTEER_EVENT_H
#define __USTEER_EVENT_H

enum uevent_type {
	UEV_PROBE_REQ_ACCEPT,
	UEV_PROBE_REQ_DENY,
	UEV_AUTH_REQ_ACCEPT,
	UEV_AUTH_REQ_DENY,
	UEV_ASSOC_REQ_ACCEPT,
	UEV_ASSOC_REQ_DENY,
	UEV_LOAD_KICK_TRIGGER,
	UEV_LOAD_KICK_RESET,
	UEV_LOAD_KICK_MIN_CLIENTS,
	UEV_LOAD_KICK_NO_CLIENT,
	UEV_LOAD_KICK_CLIENT,
	UEV_SIGNAL_KICK,
};

enum uevent_reason {
	UEV_REASON_NONE,
	UEV_REASON_RETRY_EXCEEDED,
	UEV_REASON_LOW_SIGNAL,
	UEV_REASON_CONNECT_DELAY,
	UEV_REASON_BETTER_CANDIDATE,
};

enum uevent_select_reason {
	UEV_SELECT_REASON_NUM_ASSOC,
	UEV_SELECT_REASON_SIGNAL,
	UEV_SELECT_REASON_LOAD,
};

#define UEV_SELECT_REASON_ALL ((1 << UEV_SELECT_REASON_NUM_ASSOC) | (1 << UEV_SELECT_REASON_SIGNAL) | (1 << UEV_SELECT_REASON_LOAD))

struct uevent {
	enum uevent_type type;
	enum uevent_reason reason;
	uint32_t select_reasons;

	struct usteer_node *node_local;
	struct sta *sta;

	struct sta_info *si_cur;
	struct sta_info *si_other;

	struct usteer_node *node_cur;
	struct usteer_node *node_other;

	unsigned int count;

	struct {
		int cur;
		int ref;
	} threshold;
};

void usteer_event(struct uevent *ev);
void config_set_event_log_types(struct blob_attr *attr);
void config_get_event_log_types(struct blob_buf *buf);

#endif
