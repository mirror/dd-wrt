#include "protocols/http.c"

#define DO_HTTP_PROTO(_type) do { \
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_ ## _type) != 0) { \
			IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_s->callback_buffer[*a].detection_bitmask, IPOQUE_PROTOCOL_ ## _type); \
			http = true; \
		} \
	} while (0)

static void proto_common_detection_mask(struct ipoque_static_data *ipoque_s,
					IPOQUE_PROTOCOL_BITMASK *detection_bitmask, int *a)
{
	bool http = false;

	DO_HTTP_PROTO(MPEG);
	DO_HTTP_PROTO(FLASH);
	DO_HTTP_PROTO(QUICKTIME);
	DO_HTTP_PROTO(REALMEDIA);
	DO_HTTP_PROTO(WINDOWSMEDIA);
	DO_HTTP_PROTO(MMS);
	DO_HTTP_PROTO(OFF);
	DO_HTTP_PROTO(XBOX);
	DO_HTTP_PROTO(QQ);
	DO_HTTP_PROTO(AVI);
	DO_HTTP_PROTO(OGG);
	DO_HTTP_PROTO(MOVE);
	DO_HTTP_PROTO(RTSP);

	/* HTTP DETECTION MUST BE BEFORE DDL BUT AFTER ALL OTHER PROTOCOLS WHICH USE HTTP ALSO */
	if (http || IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_HTTP) != 0) {
		ipoque_s->callback_buffer[*a].func = ipoque_search_http_tcp;
		ipoque_s->callback_buffer[*a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_s->callback_buffer[*a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_s->callback_buffer[*a].detection_bitmask, IPOQUE_PROTOCOL_HTTP);


		IPOQUE_BITMASK_SET(ipoque_s->callback_buffer[*a].excluded_protocol_bitmask,
						   ipoque_s->callback_buffer[*a].detection_bitmask);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_s->callback_buffer[*a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_s->callback_buffer[*a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_QQ);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_s->callback_buffer[*a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_FLASH);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_s->callback_buffer[*a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_MMS);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_s->callback_buffer[*a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_RTSP);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_s->callback_buffer[*a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_XBOX);

		IPOQUE_BITMASK_SET(ipoque_s->generic_http_packet_bitmask,
						   ipoque_s->callback_buffer[*a].detection_bitmask);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_s->generic_http_packet_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		(*a)++;
	}
}

