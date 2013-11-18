/* $Id: packet_states.h 4049 2006-12-01 18:21:29Z esr $ */
   GROUND_STATE,	/* we don't know what packet type to expect */

   COMMENT_BODY,	/* pound comment for a test load */
   COMMENT_RECOGNIZED,	/* comment recognized */

#ifdef NMEA_ENABLE
   NMEA_DOLLAR,		/* we've seen first character of NMEA leader */
   NMEA_PUB_LEAD,	/* seen second character of NMEA G leader */
   NMEA_LEADER_END,	/* seen end char of NMEA leader, in body */
   NMEA_CR,	   	/* seen terminating \r of NMEA packet */
   NMEA_RECOGNIZED,	/* saw trailing \n of NMEA packet */

   SIRF_ACK_LEAD_1,	/* seen A of possible SiRF Ack */
   SIRF_ACK_LEAD_2,	/* seen c of possible SiRF Ack */

   SEATALK_LEAD_1,	/* SeaTalk/Garmin packet leader 'I' */
#endif /* NMEA_ENABLE */

   DLE_LEADER,		/* we've seen the TSIP/EverMore leader (DLE) */

#ifdef TRIPMATE_ENABLE
   ASTRAL_1,		/* ASTRAL leader A */
   ASTRAL_2,	 	/* ASTRAL leader S */
   ASTRAL_3,		/* ASTRAL leader T */
   ASTRAL_4,		/* ASTRAL leader R */
   ASTRAL_5,		/* ASTRAL leader A */
#endif /* TRIPMATE_ENABLE */

#ifdef EARTHMATE_ENABLE
   EARTHA_1,		/* EARTHA leader E */
   EARTHA_2,		/* EARTHA leader A */
   EARTHA_3,		/* EARTHA leader R */
   EARTHA_4,		/* EARTHA leader T */
   EARTHA_5,		/* EARTHA leader H */
#endif /* EARTHMATE_ENABLE */

#ifdef SIRF_ENABLE
   SIRF_LEADER_1,	/* we've seen first character of SiRF leader */
   SIRF_LEADER_2,	/* seen second character of SiRF leader */
   SIRF_LENGTH_1,	/* seen first byte of SiRF length */
   SIRF_PAYLOAD,	/* we're in a SiRF payload part */
   SIRF_DELIVERED,	/* saw last byte of SiRF payload/checksum */
   SIRF_TRAILER_1,	/* saw first byte of SiRF trailer */ 
   SIRF_RECOGNIZED,	/* saw second byte of SiRF trailer */
#endif /* SIRF_ENABLE */

#ifdef ZODIAC_ENABLE
   ZODIAC_EXPECTED,	/* expecting Zodiac packet */
   ZODIAC_LEADER_1,	/* saw leading 0xff */
   ZODIAC_LEADER_2,	/* saw leading 0x81 */
   ZODIAC_ID_1, 	/* saw first byte of ID */
   ZODIAC_ID_2, 	/* saw second byte of ID */
   ZODIAC_LENGTH_1,	/* saw first byte of Zodiac packet length */
   ZODIAC_LENGTH_2,	/* saw second byte of Zodiac packet length */
   ZODIAC_FLAGS_1, 	/* saw first byte of FLAGS */
   ZODIAC_FLAGS_2, 	/* saw second byte of FLAGS */
   ZODIAC_HSUM_1, 	/* saw first byte of Header sum */
   ZODIAC_PAYLOAD,	/* we're in a Zodiac payload */
   ZODIAC_RECOGNIZED,	/* found end of the Zodiac packet */
#endif /* ZODIAC_ENABLE */

#ifdef TNT_ENABLE
   TNT_LEADER,          /* saw True North status leader '@' */
#endif

#ifdef EVERMORE_ENABLE
   EVERMORE_LEADER_1,	/* a DLE after having seen EverMore data */
   EVERMORE_LEADER_2,	/* seen opening STX of EverMore packet */
   EVERMORE_PAYLOAD,	/* in payload part of EverMore packet */
   EVERMORE_PAYLOAD_DLE,/* DLE in payload part of EverMore packet */
   EVERMORE_RECOGNIZED,	/* found end of EverMore packet */
#endif /* EVERMORE_ENABLE */

#ifdef ITALK_ENABLE
   ITALK_LEADER_1,	/* saw leading < of iTalk packet */
   ITALK_LEADER_2,	/* saw leading ! of iTalk packet */
   ITALK_LENGTH,	/* saw packet length */
   ITALK_PAYLOAD,	/* in payload part of iTalk Packet */
   ITALK_DELIVERED,	/* seen end of payload */
   ITALK_TRAILER,	/* saw iTalk trailer byte */
   ITALK_RECOGNIZED,	/* found end of the iTalk packet */
#endif /* ITALK_ENABLE */

/*
 * Packet formats without checksums start here.  We list them last so
 * that if a format with a conflicting structure *and* a checksum can
 * be recognized, that will be preferred.
 */

#if defined(TSIP_ENABLE) || defined(GARMIN_ENABLE)
   TSIP_LEADER,		/* a DLE after having seen TSIP data */
   TSIP_PAYLOAD,	/* we're in TSIP payload */
   TSIP_DLE,		/* we've seen a DLE in TSIP payload */
   TSIP_RECOGNIZED,	/* found end of the TSIP packet */
   GARMIN_RECOGNIZED,	/* found end of Garmin packet */
#endif /* TSIP_ENABLE GARMIN_ENABLE */

#ifdef RTCM104_ENABLE
   RTCM_SYNC_STATE,	/* we have sync lock */
   RTCM_SKIP_STATE,	/* we have sync lock, but this character is bad */
   RTCM_RECOGNIZED,	/* we have an RTCM packet */
#endif /* RTCM104_ENABLE */


/* end of packet_states.h */
