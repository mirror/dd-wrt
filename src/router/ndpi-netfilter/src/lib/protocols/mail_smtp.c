/*
 * mail_smtp.c
 *
 * Copyright (C) 2011-25 - ntop.org
 * Copyright (C) 2009-11 - ipoque GmbH
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_MAIL_SMTP

#include "ndpi_api.h"
#include "ndpi_private.h"


#define SMTP_BIT_220		0x01
#define SMTP_BIT_250		0x02
#define SMTP_BIT_235		0x04
#define SMTP_BIT_334		0x08
#define SMTP_BIT_354		0x10
#define SMTP_BIT_HELO_EHLO	0x20
#define SMTP_BIT_MAIL		0x40
#define SMTP_BIT_RCPT		0x80
#define SMTP_BIT_AUTH_LOGIN	0x100
#define SMTP_BIT_STARTTLS	0x200
#define SMTP_BIT_DATA		0x400
#define SMTP_BIT_NOOP		0x800
#define SMTP_BIT_RSET		0x1000
#define SMTP_BIT_TlRM		0x2000
#define SMTP_BIT_AUTH_PLAIN	0x4000

/* #define SMTP_DEBUG 1 */


static void ndpi_int_mail_smtp_add_connection(struct ndpi_detection_module_struct
					      *ndpi_struct, struct ndpi_flow_struct *flow) {
#ifdef SMTP_DEBUG
  printf("**** %s()\n", __FUNCTION__);
#endif

  ndpi_set_detected_protocol(ndpi_struct, flow,
			     NDPI_PROTOCOL_MAIL_SMTP, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
}

/* **************************************** */

static void smtpInitExtraPacketProcessing(struct ndpi_flow_struct *flow);

/* **************************************** */

static void get_credentials_auth_plain(struct ndpi_detection_module_struct *ndpi_struct,
                                       struct ndpi_flow_struct *flow,
				       const u_int8_t *line, u_int16_t line_len)
{
  u_int8_t buf[255];
  u_char *out;
  size_t i, out_len;
  unsigned int user_len = 0;

  /* AUTH PLAIN XXXXXX */
  if(line_len <= 11)
    return;

  line += 11;
  line_len -= 11;

  ndpi_user_pwd_payload_copy(buf, sizeof(buf), 0, line, line_len);
  out = ndpi_base64_decode(buf, strlen((char *)buf), &out_len);
  if(!out)
    return;
  /* No guarantee that out is null terminated:
     UTF8NUL authcid UTF8NUL passwd */
  for(i = 1; i < out_len; i++) {
    if(out[i] == '\0')
      user_len = i - 1;
  }
  if(user_len > 0) {
    char buf[64];
    
    user_len = ndpi_min(user_len, sizeof(flow->l4.tcp.ftp_imap_pop_smtp.username) - 1);

    memcpy(flow->l4.tcp.ftp_imap_pop_smtp.username, out + 1, user_len);
    flow->l4.tcp.ftp_imap_pop_smtp.username[user_len] = '\0';

    snprintf(buf, sizeof(buf), "Found username (%s)",
	     flow->l4.tcp.ftp_imap_pop_smtp.username);
    ndpi_set_risk(ndpi_struct, flow, NDPI_CLEAR_TEXT_CREDENTIALS, buf);

    if(1 + user_len + 1 < out_len) {
      unsigned int pwd_len;

      pwd_len = ndpi_min(out_len - (1 + user_len + 1), sizeof(flow->l4.tcp.ftp_imap_pop_smtp.password) - 1);
      memcpy(flow->l4.tcp.ftp_imap_pop_smtp.password, out + 1 + user_len + 1, pwd_len);
      flow->l4.tcp.ftp_imap_pop_smtp.password[pwd_len] = '\0';
    }
  }
  ndpi_free(out);
}

/* **************************************** */

static void ndpi_search_mail_smtp_tcp(struct ndpi_detection_module_struct *ndpi_struct,
			       struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);

  NDPI_LOG_DBG(ndpi_struct, "search mail_smtp\n");

  if((packet->payload_packet_len > 2)
     && (packet->parsed_lines <  NDPI_MAX_PARSE_LINES_PER_PACKET)
     && (ntohs(get_u_int16_t(packet->payload, packet->payload_packet_len - 2)) == 0x0d0a)
     ) {
    u_int16_t a;
    u_int8_t bit_count = 0;

    NDPI_PARSE_PACKET_LINE_INFO(ndpi_struct, flow, packet);

    for(a = 0; a < packet->parsed_lines; a++) {
      // expected server responses
      if(packet->line[a].len >= 3) {
	if(memcmp(packet->line[a].ptr, "220", 3) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_220;

	  if(flow->host_server_name[0] == '\0') {
	    if(packet->line[a].len > 4) {
	      int i;
	      unsigned int len;

	      if(packet->line[a].ptr[4] != '(') {
		for(i=5; (i<packet->line[a].len-1) && (packet->line[a].ptr[i] != ' '); i++)
		  ;

		if((packet->line[a].ptr[i+1] != '\r')
		   && (packet->line[a].ptr[i+1] != '\n')) {
		  len = i-4;
		  /* Copy result for nDPI apps */
		  ndpi_hostname_sni_set(flow, &packet->line[a].ptr[4], len, NDPI_HOSTNAME_NORM_ALL);
		  NDPI_LOG_DBG(ndpi_struct, "SMTP: hostname [%s]\n", flow->host_server_name);

		  if (ndpi_match_hostname_protocol(ndpi_struct, flow, NDPI_PROTOCOL_MAIL_SMTP,
						   flow->host_server_name,
						   strlen(flow->host_server_name))) {
		    /* We set the protocols; we need to initialize extra dissection
		       to search for credentials */
		    NDPI_LOG_DBG(ndpi_struct, "SMTP: hostname matched\n");
		    smtpInitExtraPacketProcessing(flow);
		  }
		}
	      }
	    }
	  }
	} else if(memcmp(packet->line[a].ptr, "250", 3) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_250;
	} else if(memcmp(packet->line[a].ptr, "235", 3) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_235;
	} else if(memcmp(packet->line[a].ptr, "334", 3) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_334;
	} else if(memcmp(packet->line[a].ptr, "354", 3) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_354;
	}
      }

      // expected client requests
      if(packet->line[a].len >= 5) {
	if(ndpi_memcasecmp(packet->line[a].ptr, "HELO ", 5) == 0 ||
	   ndpi_memcasecmp(packet->line[a].ptr, "EHLO ", 5) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_HELO_EHLO;
	  flow->l4.tcp.ftp_imap_pop_smtp.auth_found = 0;
	} else if(ndpi_memcasecmp(packet->line[a].ptr, "MAIL ", 5) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_MAIL;
	  flow->l4.tcp.ftp_imap_pop_smtp.auth_found = 0;
	  /* We shouldn't be here if there are credentials */
	  flow->l4.tcp.ftp_imap_pop_smtp.auth_done = 1;
	} else if(ndpi_memcasecmp(packet->line[a].ptr, "RCPT ", 5) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_RCPT;
	  flow->l4.tcp.ftp_imap_pop_smtp.auth_found = 0;
	  /* We shouldn't be here if there are credentials */
	  flow->l4.tcp.ftp_imap_pop_smtp.auth_done = 1;
	} else if(ndpi_memcasecmp(packet->line[a].ptr, "AUTH ", 5) == 0) {
#ifdef SMTP_DEBUG
	  printf("%s() AUTH [%.*s]\n", __FUNCTION__, packet->line[a].len, packet->line[a].ptr);
#endif
	  flow->l4.tcp.ftp_imap_pop_smtp.auth_found = 1;
	  if(packet->line[a].len >= 6) {
            if(packet->line[a].ptr[5] == 'L' || packet->line[a].ptr[5] == 'l') {
	      flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_AUTH_LOGIN;
	      /* AUTH LOGIN: Username and pwd on the next messages */
	    } else if(packet->line[a].ptr[5] == 'P' || packet->line[a].ptr[5] == 'p') {
	      flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_AUTH_PLAIN;
	      /* AUTH PLAIN: username and pwd here */
	      get_credentials_auth_plain(ndpi_struct, flow,
					 packet->line[a].ptr, packet->line[a].len);
	      flow->l4.tcp.ftp_imap_pop_smtp.auth_done = 1;
	    }
	  }
	} else {
	  if(packet->line[a].ptr[3] != ' ') {
#ifdef SMTP_DEBUG
	    printf("%s() => [%.*s]\n", __FUNCTION__, packet->line[a].len, packet->line[a].ptr);
#endif

	    if(flow->l4.tcp.ftp_imap_pop_smtp.auth_found &&
	       (flow->l4.tcp.smtp_command_bitmask & SMTP_BIT_AUTH_LOGIN)) {
	      if(flow->l4.tcp.ftp_imap_pop_smtp.username[0] == '\0') {
		/* Username */
		u_int8_t buf[48];
		u_char *out;
		size_t out_len;
		char msg[64];
		  
		ndpi_user_pwd_payload_copy(buf, sizeof(buf), 0,
					   packet->line[a].ptr, packet->line[a].len);

#ifdef SMTP_DEBUG
		printf("%s() => [auth: %u] (username) [%s]\n", __FUNCTION__, flow->l4.tcp.ftp_imap_pop_smtp.auth_found, buf);
#endif

		out = ndpi_base64_decode((const u_char*)buf, (size_t)strlen((const char*)buf), &out_len);

		if(out) {
		  size_t len = ndpi_min(out_len, sizeof(flow->l4.tcp.ftp_imap_pop_smtp.username) - 1);

		  memcpy(flow->l4.tcp.ftp_imap_pop_smtp.username, out, len);
		  flow->l4.tcp.ftp_imap_pop_smtp.username[len] = '\0';

		  ndpi_free(out);
		}
		
		snprintf(msg, sizeof(msg), "Found SMTP username (%s)",
			 flow->l4.tcp.ftp_imap_pop_smtp.username);
		ndpi_set_risk(ndpi_struct, flow, NDPI_CLEAR_TEXT_CREDENTIALS, msg);
	      } else if(flow->l4.tcp.ftp_imap_pop_smtp.password[0] == '\0') {
		/* Password */
		u_int8_t buf[48];
		u_char *out;
		size_t out_len;

		ndpi_user_pwd_payload_copy(buf, sizeof(buf), 0,
					   packet->line[a].ptr, packet->line[a].len);

#ifdef SMTP_DEBUG
		printf("%s() => [auth: %u] (password) [%s]\n", __FUNCTION__, flow->l4.tcp.ftp_imap_pop_smtp.auth_found, buf);
#endif

		out = ndpi_base64_decode((const u_char*)buf, (size_t)strlen((const char*)buf), &out_len);

		if(out) {
		  size_t len = ndpi_min(out_len, sizeof(flow->l4.tcp.ftp_imap_pop_smtp.password) - 1);

		  memcpy(flow->l4.tcp.ftp_imap_pop_smtp.password, out, len);
		  flow->l4.tcp.ftp_imap_pop_smtp.password[len] = '\0';

		  ndpi_free(out);
		}

		ndpi_set_risk(ndpi_struct, flow, NDPI_CLEAR_TEXT_CREDENTIALS, "Found password");

		flow->l4.tcp.ftp_imap_pop_smtp.auth_done = 1;
	      } else {
		flow->host_server_name[0] = '\0';
		NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
		return;
	      }
	    }
	  }
	}
      }

  if(packet->line[a].len >= 8) {
    if(ndpi_memcasecmp(packet->line[a].ptr, "STARTTLS", 8) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_STARTTLS;
	  flow->l4.tcp.ftp_imap_pop_smtp.auth_tls = 1;
      flow->l4.tcp.ftp_imap_pop_smtp.auth_done = 0;
    }
  }

      if(packet->line[a].len >= 14) {
        if(ndpi_memcasecmp(packet->line[a].ptr, "X-AnonymousTLS", 14) == 0) {
          flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_STARTTLS;
          flow->l4.tcp.ftp_imap_pop_smtp.auth_tls = 1;
          flow->l4.tcp.ftp_imap_pop_smtp.auth_done = 0;
        }
      }

      if(packet->line[a].len >= 4) {
	if(ndpi_memcasecmp(packet->line[a].ptr, "DATA", 4) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_DATA;
	} else if(ndpi_memcasecmp(packet->line[a].ptr, "NOOP", 4) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_NOOP;
	} else if(ndpi_memcasecmp(packet->line[a].ptr, "RSET", 4) == 0) {
	  flow->l4.tcp.smtp_command_bitmask |= SMTP_BIT_RSET;
	}
      }
    }

    // now count the bits set in the bitmask
    if(flow->l4.tcp.smtp_command_bitmask != 0) {
      for(a = 0; a < 16; a++) {
	bit_count += (flow->l4.tcp.smtp_command_bitmask >> a) & 0x01;
      }
    }
    NDPI_LOG_DBG2(ndpi_struct, "seen smtp commands and responses: %u\n",
		  bit_count);

    if(bit_count >= 3) {
      NDPI_LOG_INFO(ndpi_struct, "mail smtp identified\n");

#ifdef SMTP_DEBUG
      printf("%s() [bit_count: %u][%s]\n", __FUNCTION__,
	     bit_count, flow->l4.tcp.ftp_imap_pop_smtp.password);
#endif

      /* Only if we don't have already set the protocol via hostname matching */
      if(flow->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN &&
         flow->detected_protocol_stack[1] == NDPI_PROTOCOL_UNKNOWN) {
        ndpi_int_mail_smtp_add_connection(ndpi_struct, flow);
        smtpInitExtraPacketProcessing(flow);
      }
      return;
    }

    if(bit_count >= 1 && flow->packet_counter < 12) {
      return;
    }
  }

  /* when the first or second packets are split into two packets, those packets are ignored. */
  if(flow->packet_counter <= 4 &&
     packet->payload_packet_len >= 4 &&
     (ntohs(get_u_int16_t(packet->payload, packet->payload_packet_len - 2)) == 0x0d0a
      || memcmp(packet->payload, "220", 3) == 0 || memcmp(packet->payload, "EHLO", 4) == 0)) {
    NDPI_LOG_DBG2(ndpi_struct, "maybe SMTP, need next packet\n");
    return;
  }

  if((!flow->extra_packets_func) || (flow->packet_counter > 12))
    NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
}

/* **************************************** */

NDPI_STATIC int ndpi_extra_search_mail_smtp_tcp(struct ndpi_detection_module_struct *ndpi_struct,
				    struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
  int rc = 0;

  if(flow->l4.tcp.smtp_command_bitmask & SMTP_BIT_STARTTLS) {

    /* RFC 3207:
       "After the client gives the STARTTLS command, the server responds with
        one of the following reply codes:
         220 Ready to start TLS
         501 Syntax error (no parameters allowed)
         454 TLS not available due to temporary reason"
    */

    if(ndpi_struct->cfg.smtp_opportunistic_tls_enabled &&
       packet->payload_packet_len > 3 && memcmp(packet->payload, "220", 3) == 0) {
      rc = 1;
      /* Switch classification to SMTPS, keeping the hostname sub-classification (if any) */
      if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_UNKNOWN &&
	 flow->detected_protocol_stack[0] != NDPI_PROTOCOL_MAIL_SMTP) {
	ndpi_set_detected_protocol(ndpi_struct, flow,
                                   flow->detected_protocol_stack[0], NDPI_PROTOCOL_MAIL_SMTPS, NDPI_CONFIDENCE_DPI);
	/* Now it is safe to write to `flow->protos.tls_quic` union */
	flow->protos.tls_quic.subprotocol_detected = 1;
      } else {
        ndpi_set_detected_protocol(ndpi_struct, flow,
                                   NDPI_PROTOCOL_MAIL_SMTPS, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
      }
      NDPI_LOG_DBG(ndpi_struct, "Switching to [%d/%d]\n",
                   flow->detected_protocol_stack[0], flow->detected_protocol_stack[1]);
      /* We are done (in SMTP dissector): delegating TLS... */
      switch_extra_dissection_to_tls(ndpi_struct, flow);
    } else {
      rc = 0; /* Something went wrong. Stop extra dissection */
    }

  } else {
    ndpi_search_mail_smtp_tcp(ndpi_struct, flow);
    rc = ((flow->l4.tcp.ftp_imap_pop_smtp.password[0] == '\0') &&
          (flow->l4.tcp.ftp_imap_pop_smtp.auth_tls == 1 ||
           flow->l4.tcp.ftp_imap_pop_smtp.auth_done == 0)) ? 1 : 0;
  }

#ifdef SMTP_DEBUG
  printf("**** %s() [rc: %d]\n", __FUNCTION__, rc);
#endif

  return(rc);
}

/* **************************************** */

static void smtpInitExtraPacketProcessing(struct ndpi_flow_struct *flow) {
#ifdef SMTP_DEBUG
  static u_int num = 0;

  printf("**** %s(%u)\n", __FUNCTION__, ++num);
#endif

  /* At most 12 packets should almost always be enough */
  flow->max_extra_packets_to_check = 12;
  flow->extra_packets_func = ndpi_extra_search_mail_smtp_tcp;
}

/* **************************************** */

void init_mail_smtp_dissector(struct ndpi_detection_module_struct *ndpi_struct) {
  register_dissector("MAIL_SMTP", ndpi_struct,
                     ndpi_search_mail_smtp_tcp,
                     NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
                      1, NDPI_PROTOCOL_MAIL_SMTP);
}
