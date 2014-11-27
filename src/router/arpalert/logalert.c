/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: logalert.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include "log.h"
#include "loadconfig.h"
#include "alertes.h"

void alerte_log(int num_seq,
                char *mac_sender,
                char *ip_sender,
                int type,
                char *ref,
                char *interface,
                char *vendor){
	
	// log with mac vendor
	if(config[CF_LOG_VENDOR].valeur.integer == TRUE){
		switch(type){
			case AL_IP_CHANGE:
			case AL_UNAUTHRQ:
			case AL_MAC_ERROR:
			case AL_MAC_CHANGE:
				logmsg(LOG_NOTICE,
				       "seq=%d, mac=%s, ip=%s, reference=%s, "
				       "type=%s, dev=%s, vendor=\"%s\"",
				        num_seq, mac_sender, ip_sender, ref,
				        alert_type[type], interface, vendor);
				break;

			default:
				logmsg(LOG_NOTICE,
				       "seq=%d, mac=%s, ip=%s, type=%s, "
				       "dev=%s, vendor=\"%s\"",
				        num_seq, mac_sender, ip_sender,
				        alert_type[type], interface, vendor);
				break;
		}
	}

	// log whitout mac vendor
	else {
		switch(type){
			case AL_IP_CHANGE:
			case AL_UNAUTHRQ:
			case AL_MAC_ERROR:
			case AL_MAC_CHANGE:
				logmsg(LOG_NOTICE,
				       "seq=%d, mac=%s, ip=%s, reference=%s, "
				       "type=%s, dev=%s",
				        num_seq, mac_sender, ip_sender,
				        ref, alert_type[type], interface);
				break;

			default:
				logmsg(LOG_NOTICE,
				       "seq=%d, mac=%s, ip=%s, type=%s, "
				       "dev=%s",
				        num_seq, mac_sender, ip_sender,
				        alert_type[type], interface);
				break;
		}
	}
}

