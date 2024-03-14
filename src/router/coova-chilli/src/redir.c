/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "system.h"
#include "chilli.h"
#ifdef ENABLE_MODULES
#include "chilli_module.h"
#endif
#ifdef ENABLE_EWTAPI
#include "ewt.h"
#endif
#include "json/json.h"



static int optionsdebug = 0; /* TODO: Should be changed to instance */

static int termstate = REDIR_TERM_INIT;    /* When we were terminated */

char credits[] =
    "<H1>CoovaChilli " VERSION "</H1>"
    "<p>Copyright 2002-2005 Mondru AB</p>"
    "<p>Copyright 2006-2012 David Bird (Coova Technologies)</p>"
    "CoovaChilli is an Open Source captive portal or wireless LAN access point "
    "controller developed by the community at <a href=\"http://coova.github.io\">"
    "coova.github.io</a>. It is licensed under the GNU General Public License (GPL). ";

static uint8_t radius_packet_id = 0;
extern time_t mainclock;

/* Termination handler for clean shutdown
   static void redir_termination(int signum) {
   syslog(LOG_DEBUG, "Terminating redir client!");
   exit(0);
   }*/

/* Alarm handler for ensured shutdown */
static void redir_alarm(int signum) {
  syslog(LOG_WARNING, "Client process timed out: %d", termstate);
  exit(0);
}

/* Generate a 16 octet random challenge */
static int redir_challenge(unsigned char *dst) {
  FILE *file;

  if (_options.nochallenge) return 0;

  if ((file = fopen("/dev/urandom", "r")) == NULL) {
    syslog(LOG_ERR, "%s: fopen(/dev/urandom)", strerror(errno));
    return -1;
  }

  if (fread(dst, 1, REDIR_MD5LEN, file) != REDIR_MD5LEN) {
    syslog(LOG_ERR, "%s: fread() failed", strerror(errno));
    fclose(file);
    return -1;
  }

  fclose(file);
  return 0;
}

int redir_hextochar(unsigned char *src, int slen, unsigned char * dst, int len) {
  char x[3];
  int n;
  int i;
  int y;

  for (n=0; n < len; n++) {
    i = n * 2;
    y = 0;
    if (i < slen) {
      x[0] = src[i];
      x[1] = src[i+1];
      x[2] = 0;
      switch (sscanf(x, "%2x", &y)) {
        case 0:  y = 0;
        case 1:  break;
        default:
          syslog(LOG_ERR, "%s: HEX conversion failed (src='%s', len=%d, n=%d, y=%d)!",
                 strerror(errno), src, len, n, y);
          return -1;
      }
    }
    dst[n] = (unsigned char) y;
  }

  return 0;
}

/* Convert 'len' octet unsigned char to 'len'*2+1 octet ASCII hex string */
int redir_chartohex(unsigned char *src, char *dst, size_t len) {
  char x[3];
  int i = 0;
  int n;

  for (n=0; n < len; n++) {
    snprintf(x, 3, "%.2x", src[n]);
    dst[i++] = x[0];
    dst[i++] = x[1];
  }

  dst[i] = 0;
  return 0;
}

static int bytetohex(uint8_t *src, const size_t IN_LEN, char *dst,
                     const int MAX_OUT_SIZE) {
  char x[3];
  int n = 0;

  while (n < IN_LEN && n*2 < MAX_OUT_SIZE-1) {
    snprintf(x, 3, "%.2x", src[n]);
    dst[n*2+0] = x[0];
    dst[n*2+1] = x[1];
    n++;
  }

  dst[n*2] = 0;
  return 0;
}

static int bytetosphex(uint8_t *src, const size_t IN_LEN, char *dst,
		       const int MAX_OUT_SIZE) {
  char x[3];
  int i = 0;
  int o = 0;

  for (i = 0; i < IN_LEN; i++){
    if (o >= MAX_OUT_SIZE -2)
      break;

    if (i%4 == 0 && i> 0 && i<IN_LEN){
      if (o >= MAX_OUT_SIZE -1)
	break;
      dst[o++] = 0x20;   /* Add a space character */
    }

    snprintf(x, 3, "%.2x", src[i]);
    dst[o++] = x[0];
    dst[o++] = x[1];
  }

  dst[o] = 0;
  return 0;
}

/*
  static int redir_xmlencode(char *src, int srclen, char *dst, int dstsize) {
  char *x;
  int n;
  int i = 0;

  for (n=0; n<srclen; n++) {
  x=0;
  switch(src[n]) {
  case '&':  x = "&amp;";  break;
  case '\"': x = "&quot;"; break;
  case '<':  x = "&lt;";   break;
  case '>':  x = "&gt;";   break;
  default:
  if (i < dstsize - 1) dst[i++] = src[n];
  break;
  }
  if (x) {
  if (i < dstsize - strlen(x)) {
  memcpy(dst + i, x, strlen(x));
  i += strlen(x);
  }
  }
  }
  dst[i] = 0;
  return 0;
  }
*/

static void redir_http(bstring s, char *code) {
  bassigncstr(s, "HTTP/1.0 ");
  bcatcstr(s, code);
  bcatcstr(s, "\r\n");
  bcatcstr(s,
	   "Connection: close\r\n"
	   "Pragma: no-cache\r\n"
	   "Expires: Fri, 01 Jan 1971 00:00:00 GMT\r\n"
	   "Cache-Control: no-cache, must-revalidate\r\n");
  bcatcstr(s, "P3P: CP=\"IDC DSP COR ADM DEVi TAIi PSA PSD IVAi IVDi CONi HIS OUR IND CNT\"\r\n");
}

static int bstrtocstr(bstring src, char *dst, unsigned int len) {
  if (!src || src->slen == 0) {
    dst[0] = 0;
    return 0;
  }

  strlcpy(dst, (char*)src->data, len);
  return 0;
}

/* Encode src as urlencoded and place null terminated result in dst */
int redir_urlencode(bstring src, bstring dst) {
  char x[3];
  int n;

  bassigncstr(dst, "");
  for (n=0; n < src->slen; n++) {
    if ((('A' <= src->data[n]) && (src->data[n] <= 'Z')) ||
	(('a' <= src->data[n]) && (src->data[n] <= 'z')) ||
	(('0' <= src->data[n]) && (src->data[n] <= '9')) ||
	('-' == src->data[n]) ||
	('_' == src->data[n]) ||
	('.' == src->data[n]) ||
	('!' == src->data[n]) ||
	('~' == src->data[n]) ||
	('*' == src->data[n])) {
      bconchar(dst,src->data[n]);
    }
    else {
      snprintf(x, 3, "%.2x", src->data[n]);
      bconchar(dst, '%');
      bconchar(dst, x[0]);
      bconchar(dst, x[1]);
    }
  }
  return 0;
}

/* Decode urlencoded src and place null terminated result in dst */
int redir_urldecode(bstring src, bstring dst) {
  char x[3];
  int n = 0;
  unsigned int c;

  bassigncstr(dst, "");
  while (n<src->slen) {
    if (src->data[n] == '%') {
      if ((n+2) < src->slen) {
	x[0] = src->data[n+1];
	x[1] = src->data[n+2];
	x[2] = 0;
	c = '_';
	sscanf(x, "%x", &c);
	bconchar(dst,c);
      }
      n += 3;
    }
    else {
      bconchar(dst,src->data[n]);
      n++;
    }
  }
  return 0;
}

/* Creates a binary EAP request identity message with the a given EAP identity value */
static void eapidentityreq(struct eapmsg_t * eapmsg, uint8_t identity) {
  if (eapmsg == NULL)
    return;

  eapmsg->len = 5;
  eapmsg->data[0] = 0x01;        /* EAP request */
  eapmsg->data[1] = identity;
  eapmsg->data[2] = 0x00;        /* Length */
  eapmsg->data[3] = 0x05;        /* Length */
  eapmsg->data[4] = 0x01;        /* Identity type */
}

/* Encode an EAP msg into a string using the base64 algorithm.
 * Returns 0 if encoding was successfull. Returns 1 if the capacity
 * of the str is not enough to hold the encoded EAP msg*/
static int base64encoder(struct eapmsg_t * eapmsg, char * eapstr,
			 const unsigned int max_str_out) {

  const static char table64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  uint8_t i0;
  uint8_t i1;
  uint8_t i2;

  uint8_t o0;
  uint8_t o1;
  uint8_t o2;
  uint8_t o3;

  unsigned int x_in = 0;
  unsigned int x_out = 0;

  /* input length */
  unsigned int len_in = eapmsg->len;


  unsigned int len_out = (len_in*4+2)/3;	 /* output length without padding */
  unsigned int len_out_padd = ((len_in+2)/3)*4;  /* output length with padding */

  /* Check if we have enough space in the output buffer to store the encoding
     result */
  if (len_out_padd >= max_str_out){
    return 1;
  }

  /*
    if (optionsdebug) syslog(LOG_DEBUG, "Base64 encoder: input len: %d\n", len_in);
    if (optionsdebug) syslog(LOG_DEBUG, "Base64 encoder: output len without padding: %d\n", len_out);
    if (optionsdebug) syslog(LOG_DEBUG, "Base64 encoder: output len with padding: %d\n", len_out_padd);
  */

  while(x_in < len_in) {

    i0 =  eapmsg->data[x_in++] & 0xff;
    i1 = (x_in < len_in)?eapmsg->data[x_in++] & 0xff:0;
    i2 = (x_in < len_in)?eapmsg->data[x_in++] & 0xff:0;

    o0 = i0 >> 2;
    o1 = (((i0 & 3) << 4) | (i1 >> 4));
    o2 = ((i1 & 0xf) << 2) | (i2 >> 6);
    o3 = i2 & 0x3F;

    eapstr[x_out++] = table64[o0];
    eapstr[x_out++] = table64[o1];
    eapstr[x_out] = (x_out < len_out) ? table64[o2] : '=';
    x_out++;
    eapstr[x_out] = (x_out < len_out) ? table64[o3] : '=';
    x_out++;
  }

  eapstr[x_out] = 0;
  return 0;
}


/* Decode a string using the base64 algorithm into an EAP msg.
 * Returns 0 if decoding was successfull, returns 1 if the eapstr is too big
 * for the EAP msg. Return 2 for other errors */
static int base64decoder (char * eapstr, struct eapmsg_t * eapmsg)
{
  if (eapstr == NULL || eapmsg == NULL)
    return 1;

  char car;
  char in64[4];
  int  compt;
  int  x_in = 0;
  int  x_out = 0;
  unsigned int  len_in = strlen(eapstr);
  unsigned int  len_out;

  eapmsg->len = 0;       /* To avoid invalid data if decoding fails */

  if ((len_in % 4) != 0)
    return 2;

  /* Remove all trailing '=' characters */
  while (eapstr [len_in-1] == '='){
    eapstr [len_in-1] = 0;
    len_in--;
  }

  len_out = (len_in*3) / 4;

  /* Check of that the size of the resulting message fits into
     the EAP msg buffer. */
  if (len_out > MAX_EAP_LEN)
    return 1;

  /* while not end of string */
  while (x_in < len_in) {
    for (compt = 0; compt < 4 && x_in < len_in; compt++) {
      car = eapstr [x_in++];

      /* decode the char */
      if ('A' <= car && car <= 'Z')
	in64[compt] = car - 'A';
      else if ('a' <= car && car <= 'z')
	in64[compt] = car + 26 - 'a';
      else if ('0' <= car && car <= '9')
	in64[compt] = car + 52 - '0';
      else if (car == '+')
	in64[compt] = 62;
      else if (car == '/')
	in64[compt] = 63;
      else
	return 2;   /* Invalid character */
    }


    eapmsg->data[x_out++] = (in64[0] << 2) | (in64[1] >> 4);
    if (x_out < len_out)
      eapmsg->data[x_out++] = (in64[1] << 4) | (in64[2] >> 2);
    if (x_out < len_out)
      eapmsg->data[x_out++] = (in64[2] << 6) | (in64[3]);
  }

  eapmsg->len = x_out;

  /* indicate success of decoding */
  return 0;
}

static void bstring_buildurl(bstring str, struct redir_conn_t *conn,
			     struct redir_t *redir, char *redir_url, char *resp,
			     long int timeleft, char* hexchal, char* uid,
			     char* userurl, char* reply, char* redirurl,
			     uint8_t *hismac, struct in_addr *hisip, char *amp) {
  bstring bt = bfromcstr("");
  bstring bt2 = bfromcstr("");

  bassignformat(str, "%s%sres=%s%suamip=%s%suamport=%d",
		redir_url, strchr(redir_url, '?') ? amp : "?", resp, amp,
		inet_ntoa(redir->addr), amp,
		redir->port);

  if (!_options.nochallenge && hexchal) {
    bcatcstr(str, amp);
    bassignformat(bt, "challenge=%s", hexchal);
    bconcat(str, bt);
    bassigncstr(bt,"");
  }

  if (conn->type == REDIR_STATUS) {
    time_t starttime = conn->s_state.start_time;
    if (starttime) {
      time_t sessiontime;
      time_t timenow = mainclock_now();

      sessiontime = timenow - starttime;

      bcatcstr(str, amp);
      bassignformat(bt, "starttime=%ld", (long) starttime);
      bconcat(str, bt);
      bcatcstr(str, amp);
      bassignformat(bt, "sessiontime=%ld", (long) sessiontime);
      bconcat(str, bt);
    }

    if (conn->s_params.sessiontimeout) {
      bcatcstr(str, amp);
      bassignformat(bt, "sessiontimeout=%ld", (long) conn->s_params.sessiontimeout);
      bconcat(str, bt);
    }

    if (conn->s_params.sessionterminatetime) {
      bcatcstr(str, amp);
      bassignformat(bt, "stoptime=%ld", (long) conn->s_params.sessionterminatetime);
      bconcat(str, bt);
    }
  }

  bcatcstr(str, amp);
  bcatcstr(str, "called=");
  if (_options.nasmac)
    bassigncstr(bt, _options.nasmac);
  else
    bassignformat(bt, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
		  redir->nas_hwaddr[0], redir->nas_hwaddr[1], redir->nas_hwaddr[2],
		  redir->nas_hwaddr[3], redir->nas_hwaddr[4], redir->nas_hwaddr[5]);

  redir_urlencode(bt, bt2);
  bconcat(str, bt2);

  if (uid) {
    bcatcstr(str, amp);
    bcatcstr(str, "uid=");
    bassigncstr(bt, uid);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

  if (timeleft) {
    bcatcstr(str, amp);
    bassignformat(bt, "timeleft=%ld", timeleft);
    bconcat(str, bt);
  }

  if (hismac) {
    bcatcstr(str, amp);
    bcatcstr(str, "mac=");
    bassignformat(bt, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
		  hismac[0], hismac[1],
		  hismac[2], hismac[3],
		  hismac[4], hismac[5]);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

  if (hisip) {
    bcatcstr(str, amp);
    bassignformat(bt, "ip=%s", inet_ntoa(*hisip));
    bconcat(str, bt);
  }

  if (reply) {
    bcatcstr(str, amp);
    bcatcstr(str, "reply=");
    bassigncstr(bt, reply);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

  if (redir->ssid) {
    bcatcstr(str, amp);
    bcatcstr(str, "ssid=");
    bassigncstr(bt, redir->ssid);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

  if (_options.radiusnasid) {
    bcatcstr(str, amp);
    bcatcstr(str, "nasid=");
    bassigncstr(bt, _options.radiusnasid);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q && conn->s_state.tag8021q) {
    bcatcstr(str, amp);
    bcatcstr(str, "vlan=");
    bassignformat(bt, "%d",
		  (int)ntohs(conn->s_state.tag8021q &
			     PKT_8021Q_MASK_VID));
    bconcat(str, bt);
  } else
#endif
#ifdef ENABLE_MULTILAN
    if (conn->s_state.lanidx > 0) {
      bcatcstr(str, amp);
      bcatcstr(str, "vlan=");
      bassignformat(bt, "%s",
                    _options.moreif[conn->s_state.lanidx-1].vlan ?
                    _options.moreif[conn->s_state.lanidx-1].vlan :
                    _options.moreif[conn->s_state.lanidx-1].dhcpif);
      bconcat(str, bt);
    } else
#endif
      if (redir->vlan) {
        bcatcstr(str, amp);
        bcatcstr(str, "vlan=");
        bassigncstr(bt, redir->vlan);
        redir_urlencode(bt, bt2);
        bconcat(str, bt2);
      }

#ifdef ENABLE_LOCATION
  if (conn->s_state.location[0]) {
    bcatcstr(str, amp);
    bcatcstr(str, "loc=");
    bassigncstr(bt, conn->s_state.location);
    redir_urlencode(bt, bt2);

    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): found %.*s", __FUNCTION__, __LINE__,
             bt->slen, bt->data);

    bconcat(str, bt2);
  }
#endif

  if (conn->lang[0]) {
    bcatcstr(str, amp);
    bcatcstr(str, "lang=");
    bassigncstr(bt, conn->lang);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

  if (conn->s_state.sessionid[0]) {
    bcatcstr(str, amp);
    bcatcstr(str, "sessionid=");
    bassigncstr(bt, conn->s_state.sessionid);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

#ifdef ENABLE_UAMUIPORT
  if (_options.uamuissl && _options.uamuiport) {
    /*
     *  When we have uamuissl, a key/cert, and a uamuiport,
     *  then let's inform the captive portal of an SSL enabled
     *  services.
     */
    bcatcstr(str, amp);
    bcatcstr(str, "ssl=");
    if (_options.uamaliasname && _options.domain) {
      bassignformat(bt, "https://%s.%s:%d/",
                    _options.uamaliasname,
                    _options.domain,
                    _options.uamuiport);
    } else {
      bassignformat(bt, "https://%s:%d/",
                    inet_ntoa(_options.uamalias),
                    _options.uamuiport);
    }
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }
#endif

  if (_options.redirurl && redirurl) {
    bcatcstr(str, amp);
    bcatcstr(str, "redirurl=");
    bassigncstr(bt, redirurl);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

  if (userurl) {
    bcatcstr(str, amp);
    bcatcstr(str, "userurl=");
    bassigncstr(bt, userurl);
    redir_urlencode(bt, bt2);
    bconcat(str, bt2);
  }

  if (redir->secret && *redir->secret) {
    /* take the md5 of the url+uamsecret as a checksum */
    redir_md_param(str, redir->secret, amp);
  }

  bdestroy(bt);
  bdestroy(bt2);
}

int redir_md_param(bstring str, char *secret, char *amp) {
  MD5_CTX context;
  unsigned char cksum[16];
  char hex[32+1];
  int i;

  MD5Init(&context);
  MD5Update(&context, (uint8_t *)str->data, str->slen);
  MD5Update(&context, (uint8_t *)secret, strlen(secret));
  MD5Final(cksum, &context);

  hex[0]=0;
  for (i=0; i<16; i++) {
    snprintf(hex+2*i, 3, "%.2X", cksum[i]);
  }

  bcatcstr(str, amp);
  bcatcstr(str, "md=");
  bcatcstr(str, hex);
  return 0;
}

#ifdef ENABLE_CHILLIXML
/* Make a XML Chilli reply */
static void redir_xmlchilli_reply (struct redir_t *redir, struct redir_conn_t *conn,
				   int res, long int timeleft, char* hexchal,
				   char* reply, char* redirurl, bstring b) {
  bstring bt = bfromcstr("");;

  bcatcstr(b, "<CoovaChilliSession>\r\n");
  switch (res) {
    case REDIR_NOTYET:
      bassignformat(bt, "<Challenge>%s</Challenge>\r\n", hexchal);
      bconcat(b, bt);
      break;
    case REDIR_STATUS:
      if (conn->s_state.authenticated == 1) {
        time_t timenow = time(0);
        uint32_t sessiontime;

        sessiontime = timenow - conn->s_state.start_time;

        bcatcstr(b, "<State>1</State>\r\n");

        bassignformat(bt, "<StartTime>%d</StartTime>\r\n" ,
                      conn->s_state.start_time);
        bconcat(b, bt);

        bassignformat(bt, "<SessionTime>%d</SessionTime>\r\n",
                      sessiontime);
        bconcat(b, bt);

        if (timeleft) {
          bassignformat(bt, "<TimeLeft>%d</TimeLeft>\r\n",
                        timeleft);
          bconcat(b, bt);
        }

        bassignformat(bt, "<Timeout>%d</Timeout>\r\n",
                      conn->s_params.sessiontimeout);
        bconcat(b, bt);

        bassignformat(bt, "<InputOctets>%d</InputOctets>\r\n",
                      conn->s_state.input_octets);
        bconcat(b, bt);

        bassignformat(bt, "<OutputOctets>%d</OutputOctets>\r\n",
                      conn->s_state.output_octets);
        bconcat(b, bt);

        bassignformat(bt, "<MaxInputOctets>%d</MaxInputOctets>\r\n",
                      conn->s_params.maxinputoctets);
        bconcat(b, bt);

        bassignformat(bt, "<MaxOutputOctets>%d</MaxOutputOctets>\r\n",
                      conn->s_params.maxoutputoctets);
        bconcat(b, bt);

        bassignformat(bt, "<MaxTotalOctets>%d</MaxTotalOctets>\r\n",
                      conn->s_params.maxtotaloctets);
        bconcat(b, bt);
      }
      else {
        bcatcstr(b, "<State>0</State>\r\n");
      }

      break;

    case REDIR_ALREADY:
      bcatcstr(b, "<Already>1</Already>\r\n");
      break;

    case REDIR_ERROR_PROTOCOL:
    case REDIR_FAILED_NOROUTE:
    case REDIR_FAILED_MTU:
    case REDIR_FAILED_TIMEOUT:
    case REDIR_FAILED_REJECT:
    case REDIR_FAILED_OTHER:
      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      }
      bcatcstr(b, "<State>0</State>\r\n");

      break;
    case REDIR_SUCCESS:
      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      }
      bcatcstr(b, "<State>1</State>\r\n");
      break;
    case REDIR_LOGOFF:
      bcatcstr(b, "<State>0</State>\r\n");
      break;
    case REDIR_ABORT_ACK:
      bcatcstr(b, "<Abort_ack>1</Abort_ack>\r\n");
      break;
    case REDIR_ABORT_NAK:
      bcatcstr(b, "<Abort_nak>1</Abort_nak>\r\n");
      break;

    case REDIR_REQERROR:
      break;

    default:
      syslog(LOG_ERR, "redir_wispr1_reply: Unhandled response code in switch: %d", res);
  }
  bcatcstr(b, "</CoovaChilliSession>\r\n"
	   "-->\r\n");
  bdestroy(bt);
}
#endif

/* Make a WISPr 1.0 XML reply
 * Note: This method must not be called if Coova advertises the support of both
 * WISPr 1.0 and WISPr 2.0 in the "NOTYET" phase. The method redir_wispr2_reply
 * must be called instead */
void redir_wispr1_reply (struct redir_t *redir, struct redir_conn_t *conn,
			 int res, long int timeleft, char* hexchal,
			 char* reply, char* redirurl, bstring b) {
  bstring bt = bfromcstr("");;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d):", __FUNCTION__, __LINE__);

  bcatcstr(b,
	   "<!--\r\n"
	   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
	   "<WISPAccessGatewayParam\r\n"
	   "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n"
	   "  xsi:noNamespaceSchemaLocation=\"http://www.wballiance.net/wispr_2_0.xsd\""
	   ">\r\n");
  switch (res) {
    case REDIR_ALREADY:
      bcatcstr(b,
               "<AuthenticationPollReply>\r\n"
               "<MessageType>140</MessageType>\r\n"  /* response to authentication notification*/
               "<ResponseCode>102</ResponseCode>\r\n"  /* RADIUS serveur error/timeout */
               "<ReplyMessage>Already logged on</ReplyMessage>\r\n"
               "</AuthenticationPollReply>\r\n");
      break;

    case REDIR_NOTYET:
      bcatcstr(b,
               "<Redirect>\r\n"
               "<MessageType>100</MessageType>\r\n"
               "<ResponseCode>0</ResponseCode>\r\n" /* no error */
               "<AccessProcedure>1.0</AccessProcedure>\r\n");

      if (_options.radiuslocationid) {
        bassignformat(bt, "<AccessLocation>%s</AccessLocation>\r\n", _options.radiuslocationid);
        bconcat(b, bt);
      }

      if (_options.radiuslocationname) {
        bassignformat(bt, "<LocationName>%s</LocationName>\r\n", _options.radiuslocationname);
        bconcat(b, bt);
      }

      bassignformat(bt, "<LoginURL>%s%sres=wispr&amp;uamip=%s&amp;uamport=%d&amp;challenge=%s</LoginURL>\r\n",
                    _options.wisprlogin ? _options.wisprlogin : redir->url,
                    strchr(_options.wisprlogin ? _options.wisprlogin : redir->url, '?') ? "&amp;" : "?",
                    inet_ntoa(redir->addr), redir->port, hexchal);
      bconcat(b, bt);

      bassignformat(bt, "<AbortLoginURL>http://%s:%d/abort</AbortLoginURL>\r\n",
                    inet_ntoa(redir->addr), redir->port);
      bconcat(b, bt);

      bcatcstr(b,
               "</Redirect>\r\n");
      break;

    case REDIR_FAILED_REJECT:
      bcatcstr(b,
               "<AuthenticationPollReply>\r\n"
               "<MessageType>140</MessageType>\r\n"  /* response to authentication notification */
               "<ResponseCode>100</ResponseCode>\r\n");  /* login failed (Access REJECT) */

      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      }else {
        bcatcstr(b, "<ReplyMessage>Invalid Username/Password</ReplyMessage>\r\n");
      }

      bcatcstr(b, "</AuthenticationPollReply>\r\n");
      break;

    case REDIR_FAILED_NOROUTE:
      bcatcstr(b,
               "<AuthenticationPollReply>\r\n"
               "<MessageType>140</MessageType>\r\n" /* response to authentication notification */
               "<ResponseCode>105</ResponseCode>\r\n"); /* RADIUS serveur error/timeout */

      bcatcstr(b, "<ReplyMessage>no route for realm</ReplyMessage>\r\n");

      bcatcstr(b, "</AuthenticationPollReply>\r\n");
      break;

    case REDIR_ERROR_PROTOCOL:
    case REDIR_FAILED_MTU:
    case REDIR_FAILED_TIMEOUT:
    case REDIR_FAILED_OTHER:
      bcatcstr(b,
               "<AuthenticationPollReply>\r\n"
               "<MessageType>140</MessageType>\r\n" /* response to authentication notification */
               "<ResponseCode>102</ResponseCode>\r\n"); /* RADIUS serveur error/timeout */

      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      } else {
        bcatcstr(b, "<ReplyMessage>Radius error</ReplyMessage>\r\n");
      }

      bcatcstr(b, "</AuthenticationPollReply>\r\n");
      break;

    case REDIR_SUCCESS:
      bcatcstr(b,
               "<AuthenticationPollReply>\r\n"
               "<MessageType>140</MessageType>\r\n"  /* response to authentication notification */
               "<ResponseCode>50</ResponseCode>\r\n"); /* login succeeded */

      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      }

      /* Add the logoff URL */
      bassignformat(bt, "<LogoffURL>http://%s:%d/logoff</LogoffURL>\r\n",
                    inet_ntoa(redir->addr), redir->port);
      bconcat(b, bt);

      /* Add the status URL */
      bassignformat(bt, "<StatusURL>http://%s:%d/status</StatusURL>\r\n",
                    inet_ntoa(redir->addr), redir->port);
      bconcat(b, bt);

      if (redirurl) {
        bassignformat(bt, "<RedirectionURL>%s</RedirectionURL>\r\n", redirurl);
        bconcat(b, bt);
      }
      bcatcstr(b,
               "<MaxSessionTime>3600</MaxSessionTime>\r\n"); /* Max time of the session in seconds */

      bcatcstr(b, "</AuthenticationPollReply>\r\n");
      break;

    case REDIR_LOGOFF:
      bcatcstr(b,
               "<LogoffReply>\r\n"
               "<MessageType>130</MessageType>\r\n"  /* logoff notification */
               "<ResponseCode>150</ResponseCode>\r\n" /* logoff succeeded */
               "</LogoffReply>\r\n");
      break;

    case REDIR_ABORT_ACK:
      bcatcstr(b,
               "<AbortLoginReply>\r\n"
               "<MessageType>150</MessageType>\r\n"  /* response to abord login */
               "<ResponseCode>151</ResponseCode>\r\n" /* login aborded */
               "</AbortLoginReply>\r\n");
      break;

    case REDIR_ABORT_NAK:
      bcatcstr(b,
               "<AbortLoginReply>\r\n"
               "<MessageType>150</MessageType>\r\n"  /* response to abord login */
               "<ResponseCode>50</ResponseCode>\r\n");   /* login succeeded */
      bassignformat(bt, "<LogoffURL>http://%s:%d/logoff</LogoffURL>\r\n",
                    inet_ntoa(redir->addr), redir->port);
      bconcat(b, bt);
      bcatcstr(b, "</AbortLoginReply>\r\n");
      break;

    case REDIR_STATUS:
      bcatcstr(b,
               "<AuthenticationPollReply>\r\n"
               "<MessageType>140</MessageType>\r\n"); /* response to authentication Poll */
      if (conn->s_state.authenticated != 1) {
        bcatcstr(b,
                 "<ResponseCode>150</ResponseCode>\r\n"  /*logoff succeeded */
                 "<ReplyMessage>Not logged on</ReplyMessage>\r\n");
      } else {
        bcatcstr(b,
                 "<ResponseCode>50</ResponseCode>\r\n" /* login succeeded */
                 "<ReplyMessage>Already logged on</ReplyMessage>\r\n");
      }
      bcatcstr(b, "</AuthenticationPollReply>\r\n");
      break;

    case REDIR_REQERROR:
      break;

    default:
      syslog(LOG_ERR, "redir_wispr1_reply: Unhandled response code in switch: %d", res);
  }
  bcatcstr(b, "</WISPAccessGatewayParam>\r\n"
	   "-->\r\n");
  bdestroy(bt);
}

void write_authentication_msg_header (struct redir_conn_t *conn, bstring b) {
  if (conn->authdata.type == REDIR_AUTH_EAP) {
    bcatcstr(b,
             "<EAPAuthenticationReply>\r\n"
             "<MessageType>121</MessageType>\r\n"); /* response to authentication notification */
  } else {
    bcatcstr(b,
             "<AuthenticationReply>\r\n"
             "<MessageType>120</MessageType>\r\n"); /* response to authentication notification */
  }
}

void write_authentication_msg_footer(struct redir_conn_t *conn, bstring b) {
  if (conn->authdata.type == REDIR_AUTH_EAP) {
    bcatcstr(b, "</EAPAuthenticationReply>\r\n");
  } else {
    bcatcstr(b, "</AuthenticationReply>\r\n");
  }
}

/* Make a WISPr 2.0 XML reply
 * Note: This method must be called if Coova advertises the support of both
 * WISPr 1.0 and WISPr 2.0 in the "NOTYET" phase */
void redir_wispr2_reply (struct redir_t *redir, struct redir_conn_t *conn,
			 int res, long int timeleft, char* hexchal,
			 char* reply, char* redirurl, bstring b) {
  bstring bt = bfromcstr("");
  char eap64str [MAX_EAP_LEN*2];

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d):", __FUNCTION__, __LINE__);

  bcatcstr(b,
	   "<!--\r\n"
	   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
	   "<WISPAccessGatewayParam\r\n"
	   "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n"
	   "  xsi:noNamespaceSchemaLocation=\"http://www.wballiance.net/wispr_2_0.xsd\""
	   ">\r\n");

  switch (res) {
    case REDIR_ALREADY:
      /* TO-DO need to verify when this can happen and what to return */
      bcatcstr(b,
               "<AuthenticationReply>\r\n"
               "<MessageType>140</MessageType>\r\n"  /* response to authentication notification*/
               "<ResponseCode>102</ResponseCode>\r\n"  /* RADIUS serveur error/timeout */
               "<ReplyMessage>Already logged on</ReplyMessage>\r\n"
               "</AuthenticationReply>\r\n");
      break;

    case REDIR_NOTYET:
      bcatcstr(b,
               "<Redirect>\r\n"
               "<MessageType>100</MessageType>\r\n"
               "<ResponseCode>0</ResponseCode>\r\n"
               "<VersionHigh>2.0</VersionHigh>\r\n");

      if (!_options.no_wispr1)
        bcatcstr(b,
                 "<VersionLow>1.0</VersionLow>\r\n"
                 "<AccessProcedure>1.0</AccessProcedure>\r\n"); /* Indicate support for WISPr 1.0 */
      else
        bcatcstr(b,
                 "<VersionLow>2.0</VersionLow>\r\n");  /* Indicate only support for WISPr 2.0 */

      if (_options.radiuslocationid) {
        bassignformat(bt, "<AccessLocation>CDATA[[%s]]</AccessLocation>\r\n", _options.radiuslocationid);
        bconcat(b, bt);
      }

      if (_options.radiuslocationname) {
        bassignformat(bt, "<LocationName>CDATA[[%s]]</LocationName>\r\n", _options.radiuslocationname);
        bconcat(b, bt);
      }

      bassignformat(bt, "<LoginURL>%s%sres=wispr&amp;uamip=%s&amp;uamport=%d&amp;challenge=%s</LoginURL>\r\n",
                    _options.wisprlogin ? _options.wisprlogin : redir->url,
                    strchr(_options.wisprlogin ? _options.wisprlogin : redir->url, '?') ? "&amp;" : "?",
                    inet_ntoa(redir->addr), redir->port, hexchal);
      bconcat(b, bt);

      bassignformat(bt, "<AbortLoginURL>http://%s:%d/abort</AbortLoginURL>\r\n",
                    inet_ntoa(redir->addr), redir->port);
      bconcat(b, bt);

      /* Create an EAP identity message */
      eapidentityreq(&(conn->authdata.v.eapmsg),
                     ++conn->s_state.redir.eap_identity);

      if (!base64encoder(&(conn->authdata.v.eapmsg),
                         eap64str, MAX_EAP_LEN*2)){
        /*	syslog(LOG_DEBUG, "Encoded radius eap msg: %s", eap64str);  */
        bassignformat(bt, "<EAPMsg>%s</EAPMsg>\r\n", eap64str);
        bconcat(b, bt);
      } else {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): Base64 encoding of eap identity request failed", __FUNCTION__, __LINE__);
      }
      bcatcstr(b, "</Redirect>\r\n");
      break;

    case REDIR_FAILED_REJECT:
      write_authentication_msg_header(conn,b);

      bcatcstr(b,
               "<ResponseCode>100</ResponseCode>\r\n"); /* login failed (Access REJECT) */

      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      }
      else {
        bcatcstr(b, "<ReplyMessage>Invalid Password</ReplyMessage>\r\n");
      }

      write_authentication_msg_footer(conn,b);
      break;

    case REDIR_FAILED_NOROUTE:
      write_authentication_msg_header(conn,b);

      bcatcstr(b,
               "<ResponseCode>105</ResponseCode>\r\n"); /* RADIUS serveur error/timeout */

      bcatcstr(b, "<ReplyMessage>no route for realm</ReplyMessage>\r\n");

      write_authentication_msg_footer(conn,b);
      break;

    case REDIR_FAILED_MTU:
      write_authentication_msg_header(conn,b);

      bcatcstr(b,
               "<ResponseCode>253</ResponseCode>\r\n"); /* MTU is too big */

      bcatcstr(b, "<ReplyMessage>AAA MTU is too big</ReplyMessage>\r\n");

      write_authentication_msg_footer(conn,b);
      break;

    case REDIR_FAILED_TIMEOUT:
      write_authentication_msg_header(conn,b);

      bcatcstr(b,
               "<ResponseCode>102</ResponseCode>\r\n"); /* RADIUS serveur timeout */

      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      } else {
        bcatcstr(b, "<ReplyMessage>Radius timeout</ReplyMessage>\r\n");
      }

      write_authentication_msg_footer(conn,b);
      break;

    case REDIR_ERROR_PROTOCOL:
      write_authentication_msg_header(conn,b);

      bcatcstr(b,
               "<ResponseCode>254</ResponseCode>\r\n"); /* WISPr 2.0 protocol error */

      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      } else {
        bcatcstr(b, "<ReplyMessage>WISPr 2.0 protocol error</ReplyMessage>\r\n");
      }

      write_authentication_msg_footer(conn,b);
      break;

    case REDIR_FAILED_OTHER:
      write_authentication_msg_header(conn,b);

      bcatcstr(b,
               "<ResponseCode>255</ResponseCode>\r\n"); /* RADIUS serveur error */

      if (reply) {
        bassignformat(bt, "<ReplyMessage>%s</ReplyMessage>\r\n", reply);
        bconcat(b, bt);
      } else {
        bcatcstr(b, "<ReplyMessage>Radius error</ReplyMessage>\r\n");
      }

      write_authentication_msg_footer(conn,b);
      break;

    case REDIR_CHALLENGE:

      bcatcstr(b,
               "<EAPAuthenticationReply>\r\n"
               "<MessageType>121</MessageType>\r\n"
               "<ResponseCode>10</ResponseCode>\r\n");

      if (!base64encoder(&(conn->authdata.v.eapmsg),
                         eap64str, MAX_EAP_LEN*2)){
        bassignformat(bt, "<EAPMsg>%s</EAPMsg>\r\n", eap64str);
        bconcat(b, bt);
      } else {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): Base64 encoding of radius eap message failed", __FUNCTION__, __LINE__);
      }

      bassignformat(bt, "<LoginURL>%s%sres=wispr&amp;uamip=%s&amp;continue=1&amp;uamport=%d&amp;challenge=%s</LoginURL>\r\n",
                    _options.wisprlogin ? _options.wisprlogin : redir->url,
                    strchr(_options.wisprlogin ? _options.wisprlogin : redir->url, '?') ? "&amp;" : "?",
                    inet_ntoa(redir->addr), redir->port, hexchal);
      bconcat(b, bt);

      bcatcstr(b, "</EAPAuthenticationReply>\r\n");
      break;

    case REDIR_SUCCESS:
      write_authentication_msg_header(conn,b);

      bcatcstr(b,
               "<ResponseCode>50</ResponseCode>\r\n");

      if (conn->authdata.type == REDIR_AUTH_EAP) {
        if (!base64encoder(&(conn->authdata.v.eapmsg),
                           eap64str, MAX_EAP_LEN*2)){
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Encoded radius eap msg: %s", __FUNCTION__, __LINE__, eap64str);
          bassignformat(bt, "<EAPMsg>%s</EAPMsg>\r\n", eap64str);
          bconcat(b, bt);
        } else {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Base64 encoding of radius eap message failed", __FUNCTION__, __LINE__);
        }
      }

      /* Add the logoff URL */
      bassignformat(bt, "<LogoffURL>http://%s:%d/logoff</LogoffURL>\r\n",
                    inet_ntoa(redir->addr), redir->port);
      bconcat(b, bt);

      /* Add the status URL */
      bassignformat(bt, "<StatusURL>http://%s:%d/status</StatusURL>\r\n",
                    inet_ntoa(redir->addr), redir->port);
      bconcat(b, bt);

      /* Add the maximum session time */
      bassignformat(bt,
                    "<MaxSessionTime>%d</MaxSessionTime>\r\n", conn->s_params.sessiontimeout); /* Max time of the session in seconds */
      bconcat(b, bt);

      /* Add the redirection URL if present */
      if (redirurl) {
        bassignformat(bt, "<RedirectionURL>%s</RedirectionURL>\r\n", redirurl);
        bconcat(b, bt);
      }

      write_authentication_msg_footer(conn,b);
      break;

    case REDIR_LOGOFF:
      bcatcstr(b,
               "<LogoffReply>\r\n"
               "<MessageType>130</MessageType>\r\n"  /* logoff notification */
               "<ResponseCode>150</ResponseCode>\r\n" /* logoff succeeded */
               "</LogoffReply>\r\n");
      break;

    case REDIR_ABORT_ACK:
      bcatcstr(b,
               "<AbortLoginReply>\r\n"
               "<MessageType>150</MessageType>\r\n"  /* response to abord login */
               "<ResponseCode>151</ResponseCode>\r\n" /* login aborded */
               "</AbortLoginReply>\r\n");
      break;

    case REDIR_ABORT_NAK:
      bcatcstr(b,
               "<AbortLoginReply>\r\n"
               "<MessageType>150</MessageType>\r\n"     /* response to abord login */
               "<ResponseCode>50</ResponseCode>\r\n"     /* login succeeded */
               "</AbortLoginReply>\r\n");
      break;

    case REDIR_STATUS:
      bcatcstr(b,
               "<StatusReply>\r\n"
               "<MessageType>160</MessageType>\r\n"	/* response to authentication Poll */
               "<ResponseCode>0</ResponseCode>\r\n");  /* no error*/

      if (conn->s_state.authenticated != 1) {
        bcatcstr(b, "<Status>1</Status>\r\n");
      } else {
        bcatcstr(b, "<Status>0</Status>\r\n");
      }
      bcatcstr(b, "</StatusReply>\r\n");
      break;

    case REDIR_REQERROR:
      break;

    default:
      syslog(LOG_ERR, "redir_wispr1_reply: Unhandled response code in switch: %d", res);
  }

  bcatcstr(b, "</WISPAccessGatewayParam>\r\n"
	   "-->\r\n");
  bdestroy(bt);
}

#ifdef ENABLE_JSON
static int redir_json_reply(struct redir_t *redir, int res, struct redir_conn_t *conn,
			    char *hexchal, char *userurl, char *redirurl,
			    uint8_t *hismac, struct in_addr *hisip,
			    char *reply, char *qs, bstring s) {
  bstring tmp = bfromcstr("");
  bstring json = bfromcstr("");

  unsigned char flg = 0;
#define FLG_cb     1
#define FLG_chlg   2
#define FLG_sess   4
#define FLG_loc    8
#define FLG_redir 16

  int state = conn->s_state.authenticated;
  int splash = (conn->s_params.flags & REQUIRE_UAM_SPLASH) == REQUIRE_UAM_SPLASH;

  redir_getparam(redir, qs, "callback", tmp);

  if (tmp->slen) {
    bconcat(json, tmp);
    bcatcstr(json, "(");
    flg |= FLG_cb;
  }

  switch (res) {
    case REDIR_ALREADY:
      flg |= FLG_sess;
      break;

    case REDIR_FAILED_REJECT:
    case REDIR_FAILED_OTHER:
      flg |= FLG_chlg;
      flg |= FLG_redir;
      break;

    case REDIR_SUCCESS:
      flg |= FLG_sess;
      flg |= FLG_redir;
      state = 1;
      break;

    case REDIR_LOGOFF:
      flg |= FLG_sess | FLG_chlg;
      break;

    case REDIR_SPLASH:
    case REDIR_NOTYET:
      flg |= FLG_chlg;
      flg |= FLG_loc;
      flg |= FLG_redir;
      break;

    case REDIR_ABORT_ACK:
    case REDIR_ABORT_NAK:
    case REDIR_ABOUT:
      break;

    case REDIR_STATUS:
      if (state && !splash) {
        flg |= FLG_sess;
      } else {
        flg |= FLG_chlg;
        flg |= FLG_loc;
      }
      flg |= FLG_redir;
      break;

    default:
      break;
  }

  if (state && splash)
    state = 3;

  bcatcstr(json, "{\"version\":\"1.0\",\"clientState\":");

  bassignformat(tmp, "%d", state);
  bconcat(json, tmp);

  if (_options.radiusnasid) {
    bcatcstr(json, ",\"nasid\":\"");
    bcatcstr(json, _options.radiusnasid);
    bcatcstr(json, "\"");
  }

  if (reply) {
    struct json_object* reply_json_obj;
    reply_json_obj = json_object_new_string(reply);

    bcatcstr(json, ",\"message\":");
    bcatcstr(json, json_object_to_json_string(reply_json_obj));
    json_object_put(reply_json_obj);
  }

  if ((flg & FLG_chlg) && hexchal) {
    bcatcstr(json, ",\"challenge\":\"");
    bcatcstr(json, hexchal);
    bcatcstr(json, "\"");
  }

  if (flg & FLG_loc) {
    bcatcstr(json,",\"location\":{\"name\":\"");
    if (_options.locationname)
      bcatcstr(json, _options.locationname);
    else if (_options.radiuslocationname)
      bcatcstr(json, _options.radiuslocationname);
    bcatcstr(json,"\"");
    bcatcstr(json,"}");
  }

  if (flg & FLG_redir) {
    bassignformat(tmp , "http://%s:%d/logoff",
		  inet_ntoa(redir->addr), redir->port);

    session_redir_json_fmt(json, userurl, redirurl, tmp, hismac, hisip);
  }

  if (flg & FLG_sess)
    session_json_fmt(&conn->s_state, &conn->s_params,
		     json, res == REDIR_SUCCESS);

  bcatcstr(json, "}");

  if (flg & FLG_cb) {
    bcatcstr(json, ")");
  }

  redir_http(s, "200 OK");

  bcatcstr(s, "Content-Length: ");
  bassignformat(tmp , "%d", blength(json));
  bconcat(s, tmp);

  bcatcstr(s, "\r\nContent-Type: ");
  if (tmp->slen) bcatcstr(s, "text/javascript");
  else bcatcstr(s, "application/json");

  bcatcstr(s, "\r\n\r\n");
  bconcat(s, json);

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): sending json: %s\n", __FUNCTION__, __LINE__, json->data);
#endif

  bdestroy(json);
  bdestroy(tmp);

  return 0;
}
#endif

static void redir_buildurl(struct redir_conn_t *conn, bstring str,
			   struct redir_t *redir, char *resp,
			   long int timeleft, char* hexchal, char* uid,
			   char* userurl, char* reply, char* redirurl,
			   uint8_t *hismac, struct in_addr *hisip) {
  char *redir_url = redir->url;

  if ((conn->s_params.flags & REQUIRE_UAM_SPLASH ||
       conn->s_params.flags & REQUIRE_REDIRECT) &&
      conn->s_params.url[0]) {
    redir_url = (char *)conn->s_params.url;
    redirurl = 0;
  }

  bstring_buildurl(str, conn, redir, redir_url, resp, timeleft,
		   hexchal, uid, userurl, reply, redirurl, hismac, hisip, "&");
}

ssize_t
tcp_write_timeout(int timeout, struct redir_socket_t *sock, char *buf, size_t len) {
  fd_set fdset;
  struct timeval tv;
  int fd = sock->fd[1];

  FD_ZERO(&fdset);
  FD_SET(fd,&fdset);

  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  if (select(fd + 1,(fd_set *) 0,&fdset,(fd_set *) 0,&tv) == -1)
    return -1;

  if (FD_ISSET(fd, &fdset))
#if WIN32
    return send(fd, buf, len, 0);
#else
  return safe_write(fd, buf, len);
#endif

  return -1;
}

static int timeout = 10;

ssize_t
redir_write(struct redir_socket_t *sock, char *buf, size_t len) {
  ssize_t c;
  size_t r = 0;

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): redir_write(%zd)", __FUNCTION__, __LINE__, len);
#endif

  while (r < len) {
#ifdef HAVE_SSL
    if (sock->sslcon) {
      c = openssl_write(sock->sslcon, buf, len, 0);
    } else
#endif
    {
      c = tcp_write_timeout(timeout, sock, buf+r, len-r);
    }
    if (c <= 0) return (ssize_t) r;
    r += (size_t)c;
  }

  return (ssize_t)r;
}

/* Make an HTTP redirection reply and send it to the client */
int redir_reply(struct redir_t *redir, struct redir_socket_t *sock,
		struct redir_conn_t *conn, int res, bstring url,
		long int timeleft, char* hexchal, char* uid,
		char* userurl, char* reply, char* redirurl,
		uint8_t *hismac, struct in_addr *hisip, char *qs) {

  char *resp = NULL;
  bstring buffer;

  switch (res) {
    case REDIR_ALREADY:
      resp = "already";
      break;
    case REDIR_FAILED_REJECT:
      resp = "failed&reason=reject";
      break;
    case REDIR_FAILED_TIMEOUT:
      resp = "failed&reason=timeout";
      break;
    case REDIR_FAILED_MTU:
      resp = "failed&reason=mtu";
      break;
    case REDIR_FAILED_OTHER:
    case REDIR_ERROR_PROTOCOL:
      resp = "failed&reason=other";
      break;
    case REDIR_REQERROR:
      resp = "failed";
      break;
    case REDIR_SUCCESS:
      resp = "success";
      break;
    case REDIR_LOGOFF:
      resp = "logoff";
      break;
    case REDIR_NOTYET:
      resp = "notyet";
      break;
    case REDIR_SPLASH:
      resp = "splash";
      break;
    case REDIR_ABORT_ACK:
      resp = "logoff";
      break;
    case REDIR_ABORT_NAK:
      resp = "already";
      break;
    case REDIR_ABOUT:
    case REDIR_ABORT:
      break;
    case REDIR_STATUS:
      resp = conn->s_state.authenticated == 1 ? "already" : "notyet";
      break;
    case REDIR_CHALLENGE:
      resp = "challenge";
      break;
    default:
      syslog(LOG_ERR, "Unknown res in switch");
      return -1;
  }

  buffer = bfromcstralloc(1024, "");
  if (!buffer) {
    syslog(LOG_ERR, "%s: bfromcstralloc() memory allocation error.", __FUNCTION__);
    return -1;
  }

#ifdef ENABLE_JSON
  if (conn->format == REDIR_FMT_JSON) {

    redir_json_reply(redir, res, conn, hexchal, userurl, redirurl,
		     hismac, hisip, reply, qs, buffer);

  } else
#endif
    if (resp) {
      bstring bt;
      bstring bbody;

      redir_http(buffer, "302 Moved Temporarily");
      bcatcstr(buffer, "Location: ");

      if (url) {

        bconcat(buffer, url);

      } else if (!_options.redirurl && redirurl && *redirurl) {
	bcatcstr(buffer, redirurl);
      } else {
        bt = bfromcstralloc(1024,"");
        redir_buildurl(conn, bt, redir, resp, timeleft, hexchal,
                       uid, userurl, reply, redirurl, hismac, hisip);
        bconcat(buffer, bt);
        bdestroy(bt);
      }

      bcatcstr(buffer, "\r\nContent-Type: text/html; charset=UTF-8\r\n");

      bbody = bfromcstralloc(512,
                             "<HTML><BODY><H2>Browser error!</H2>"
                             "Browser does not support redirects!</BODY>\r\n");

      if (res == REDIR_NOTYET) {

        if (!_options.no_wispr1 && _options.no_wispr2)
          redir_wispr1_reply(redir, conn, REDIR_NOTYET, timeleft, hexchal, reply, redirurl, bbody);

        else if (!_options.no_wispr2)
          redir_wispr2_reply(redir, conn, REDIR_NOTYET, timeleft, hexchal, reply, redirurl, bbody);

#ifdef ENABLE_CHILLIXML
        if (_options.chillixml)
          redir_xmlchilli_reply(redir, conn, REDIR_NOTYET, timeleft, hexchal, reply, redirurl, bbody);
#endif

      } else {

        if (conn->s_state.redir.uamprotocol & REDIR_UAMPROT_WISPR2)
          redir_wispr2_reply(redir, conn, res, timeleft, hexchal, reply, redirurl, bbody);
        else
          redir_wispr1_reply(redir, conn, res, timeleft, hexchal, reply, redirurl, bbody);

#ifdef ENABLE_CHILLIXML
        if (conn->s_state.redir.uamprotocol == REDIR_UAMPROT_CHILLI)
          redir_xmlchilli_reply(redir, conn, res, timeleft, hexchal, reply, redirurl, bbody);
#endif
      }

      /*redir_xmlreply(redir, conn, res, timeleft, hexchal, reply, redirurl, bbody);*/

      bcatcstr(bbody, "\r\n</HTML>\r\n");

      bt = bfromcstralloc(128, "");
      bassignformat(bt, "Content-Length: %d\r\n", blength(bbody));
      bconcat(buffer, bt);

      bcatcstr(buffer, "\r\n"); /* end of headers */
      bconcat(buffer, bbody);

      bdestroy(bbody);
      bdestroy(bt);

    } else {
      redir_http(buffer, "200 OK");
      bcatcstr(buffer,
               "Content-type: text/html\r\n\r\n"
               "<HTML><HEAD><TITLE>CoovaChilli</TITLE></HEAD><BODY>");
      bcatcstr(buffer, credits);
      bcatcstr(buffer, "</BODY></HTML>\r\n");
    }

  if (redir_write(sock, (char*)buffer->data, buffer->slen) < 0) {
    syslog(LOG_ERR, "%s: redir_write()", strerror(errno));
    bdestroy(buffer);
    return -1;
  }

  bdestroy(buffer);
  return 0;
}

/* Allocate new instance of redir */
int redir_new(struct redir_t **redir,
	      struct in_addr *addr, int port, int uiport) {

  if (!(*redir = calloc(1, sizeof(struct redir_t)))) {
    syslog(LOG_ERR, "%s: calloc() failed", strerror(errno));
    return EOF;
  }

  (*redir)->addr = *addr;
  (*redir)->port = port;
#ifdef ENABLE_UAMUIPORT
  (*redir)->uiport = uiport;
#endif
  (*redir)->starttime = 0;

  return 0;
}

int redir_listen(struct redir_t *redir) {
  struct sockaddr_in address;
  int n = 0, tries = 0, success = 0;
  int optval;

  if ((redir->fd[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    syslog(LOG_ERR, "%s: socket() failed", strerror(errno));
    return -1;
  }

  ndelay_on(redir->fd[0]);

#ifdef ENABLE_UAMUIPORT
  if (redir->uiport) {
    if ((redir->fd[1] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      syslog(LOG_ERR, "%s: socket() failed", strerror(errno));
      close(redir->fd[0]);
      return -1;
    }

    ndelay_on(redir->fd[1]);
  }
#endif

  /* Set up address */
  address.sin_family = AF_INET;
#if defined(__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  address.sin_len = sizeof (struct sockaddr_in);
#endif

  for (n = 0; n < 2 && redir->fd[n]; n++) {

    address.sin_addr.s_addr = redir->addr.s_addr;
    switch(n) {
      case 0:
        address.sin_port = htons(redir->port);
        break;
#ifdef ENABLE_UAMUIPORT
      case 1:
        /* XXX: binding to 0.0.0.0:uiport (should be configurable?) */
        /*address.sin_addr.s_addr = INADDR_ANY;*/
        address.sin_port = htons(redir->uiport);
        break;
#endif
    }

    optval = 1;
    if (setsockopt(redir->fd[n], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
      syslog(LOG_ERR, "%s: setsockopt(SO_REUSEADDR)", strerror(errno));
      safe_close(redir->fd[n]);
      redir->fd[n]=0;
      break;
    }

#ifdef SO_REUSEPORT
    optval = 1;
    if (setsockopt(redir->fd[n], SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))) {
      syslog(LOG_ERR, "%s: setsockopt(SO_REUSEPORT)", strerror(errno));
      if (errno != ENOPROTOOPT) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): setsockopt(SO_REUSEPORT) failed hard, aborting.", __FUNCTION__, __LINE__);
	safe_close(redir->fd[n]);
	redir->fd[n]=0;
	return -1;
      } else {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): setsockopt(SO_REUSEPORT) failed due to proto not available "
                 "(probably compiled with newer header files), continueing anyways...", __FUNCTION__, __LINE__);
      }
    }
#endif

    while (bind(redir->fd[n], (struct sockaddr *)&address, sizeof(address)) == -1) {
      if ((EADDRINUSE == errno) && (10 > tries++)) {
	syslog(LOG_WARNING, "%d IP: %s Port: %d - Waiting for retry.",
               errno, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	if (sleep(5)) { /* In case we got killed */
	  safe_close(redir->fd[n]);
	  redir->fd[n]=0;
	  break;
	}
      }
      else {
	syslog(LOG_ERR, "%d bind() failed for %s:%d",
               errno, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	if (n == 0 && address.sin_addr.s_addr != INADDR_ANY) {
	  syslog(LOG_WARNING, "trying INADDR_ANY instead");
	  address.sin_addr.s_addr = INADDR_ANY;
	} else {
	  safe_close(redir->fd[n]);
	  redir->fd[n]=0;
	  break;
	}
      }
    }

    if (redir->fd[n]) {
      if (listen(redir->fd[n], REDIR_MAXLISTEN)) {
	syslog(LOG_ERR, "%d listen() failed for %s:%d",
               errno, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	safe_close(redir->fd[n]);
	redir->fd[n]=0;
	break;
      } else {
	success++;
      }
    }
  }

  return success ? 0 : -1;
}

int redir_ipc(struct redir_t *redir) {
#ifdef USING_IPC_UNIX
  struct sockaddr_un local;
  int sock;

  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {

    syslog(LOG_ERR, "%s: could not allocate UNIX Socket!", strerror(errno));

  } else {

    char filedest[512];

    statedir_file(filedest, sizeof(filedest), _options.unixipc, "chilli.ipc");

    local.sun_family = AF_UNIX;

    strlcpy(local.sun_path, filedest,
            sizeof(local.sun_path));
    unlink(local.sun_path);

    if (bind(sock, (struct sockaddr *)&local,
	     sizeof(struct sockaddr_un)) == -1) {
      syslog(LOG_ERR, "%s: could bind UNIX Socket to %s!", strerror(errno), filedest);
      safe_close(sock);
    } else {
      if (listen(sock, 128) == -1) {
	syslog(LOG_ERR, "%s: could listen to UNIX Socket!", strerror(errno));
	safe_close(sock);
      } else {
	redir->msgfd = sock;

	if (_options.uid) {
	  if (chown(filedest, _options.uid, _options.gid)) {
	    syslog(LOG_ERR, "%s: could not chown() %s", strerror(errno), filedest);
	  }
	}
      }
    }
  }
#else
  if ((redir->msgid = msgget(IPC_PRIVATE, 0)) < 0) {
    syslog(LOG_ERR, "%s: msgget() failed", strerror(errno));
    syslog(LOG_ERR, "Most likely your computer does not have System V IPC installed");
    return -1;
  }

  if (_options.uid) {
    struct msqid_ds ds;
    memset(&ds, 0, sizeof(ds));
    if (msgctl(redir->msgid, IPC_STAT, &ds) < 0) {
      syslog(LOG_ERR, "%s: msgctl(stat) failed", strerror(errno));
      return -1;
    }
    ds.msg_perm.uid = _options.uid;
    if (_options.gid) ds.msg_perm.gid = _options.gid;
    ds.msg_perm.mode = (ds.msg_perm.mode & ~0777) | 0600;
    if (msgctl(redir->msgid, IPC_SET, &ds) < 0) {
      syslog(LOG_ERR, "%s: msgctl(set) failed", strerror(errno));
      return -1;
    }
  }
#endif

  return 0;
}


/* Free instance of redir */
int redir_free(struct redir_t *redir) {
  int n;
  int fd;

  for (n = 0; n < 2 && redir->fd[n]; n++) {
    fd = redir->fd[n];
    if (safe_close(redir->fd[n])) {
      syslog(LOG_ERR, "redir: %s: close(fd=%d[%d]) failed", strerror(errno),
	     fd, n);
    }
  }

#ifdef USING_IPC_UNIX
  safe_close(redir->msgfd);
#else
  if (msgctl(redir->msgid, IPC_RMID, NULL)) {
    syslog(LOG_ERR, "%s: msgctl() failed", strerror(errno));
  }
#endif

  free(redir);
  return 0;
}

/* Set redir parameters */
void redir_set(struct redir_t *redir, uint8_t *hwaddr, int debug) {
  optionsdebug = debug; /* TODO: Do not change static variable from instance */
  redir->debug = debug;

  redir->url = _options.uamurl;
  redir->homepage = _options.uamhomepage;
  redir->secret = _options.uamsecret;
  redir->ssid = _options.ssid;
  redir->vlan = _options.vlan;
  redir->nasmac = _options.nasmac;
  redir->nasip = _options.nasip;

  if (hwaddr) {
    memcpy(redir->nas_hwaddr, hwaddr, sizeof(redir->nas_hwaddr));
  }

  return;
}

/* Get a parameter of an HTTP request. Parameter is url decoded */
/* TODO: Should be merged with other parsers */
int redir_getparam(struct redir_t *redir, char *src, char *param, bstring dst) {
  char *p1;
  char *p2;
  bstring s = NULL;
  char sstr[255];
  ssize_t len = 0;

  snprintf(sstr, sizeof(sstr), "&%s=", param);

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): getparam(%s)", __FUNCTION__, __LINE__, sstr);
#endif

  len = strlen(sstr);
  if (!strncmp(src, sstr+1, len-1)) {
    p1 = src;
    p1 += len-1;
  }
  else if ((p1 = strstr(src, sstr))) {
    p1 += len;
  }
  else return -1;

  /* The parameter ends with a & or null */
  p2 = strstr(p1, "&");

  if (p2) len = p2 - p1;
  else len = strlen(p1);

  if ((len) && ((s = blk2bstr(p1, len)) != NULL)) {
    redir_urldecode(s, dst);
    bdestroy(s);
  } else
    bassigncstr(dst, "");

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): The parameter %s is: [%.*s]", __FUNCTION__, __LINE__, param, dst->slen, dst->data);

  return 0;
}

/* Read the an HTTP request from a client */
static int redir_getreq(struct redir_t *redir, struct redir_socket_t *sock,
			struct redir_conn_t *conn, struct redir_httpreq_t *httpreq,
			redir_request *rreq) {
  int fd = sock->fd[0];
  fd_set fds;
  struct timeval idleTime;
  int status;
  ssize_t recvlen = 0;
  size_t buflen = 0;
  char buffer[REDIR_MAXBUFFER];
  int i, lines=0;
  char *eol;

  char read_waiting;

  char *path = httpreq->path;

  char forked = (rreq == 0);

  char wblock = 0, done = 0, eoh = 0;

  memset(buffer, 0, sizeof(buffer));

  /* read whatever the client send to us */
  while (!done && (redir->starttime + REDIR_HTTP_MAX_TIME) > mainclock_now()) {

    read_waiting = 0;

#ifdef HAVE_SSL
    if (sock->sslcon) {
      read_waiting = (openssl_pending(sock->sslcon) > 0);
    }
#endif

    if (!read_waiting && forked) {

      /*
       *  If not already with data from the SSL layer, wait for it.
       */

      do {

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	memset((void *)&idleTime, 0, sizeof(idleTime));
	idleTime.tv_sec = REDIR_HTTP_SELECT_TIME;

	status = select(fd + 1, &fds, NULL, NULL, &idleTime);

      } while (status == -1 && errno == EINTR);

      switch(status) {
        case -1:
          syslog(LOG_ERR, "%s: select(%d)", strerror(errno), fd);
          return -1;

          /*
            case 0:
            syslog(LOG_DEBUG, "HTTP request timeout!");
            done = 1;
            break;
          */

        default:
          break;
      }

      if ((status > 0) && FD_ISSET(fd, &fds)) {
	/*
	 *  We have data pending a read
	 */
	read_waiting = 1;
      }
    }

    if (httpreq->data_in && !forked) {
      buflen = 0;
    }

    if (read_waiting || !forked) {

      if (buflen + 2 >= sizeof(buffer)) { /* ensure space for a least one more byte + null */
        syslog(LOG_ERR, "Too much data in http request! %d", (int) buflen);
        return -1;
      }

      /* if post is allowed, we do not buffer on the read (to not eat post data) */
#ifdef HAVE_SSL
      if (sock->sslcon) {
	recvlen = openssl_read(sock->sslcon,
			       buffer + buflen,
			       httpreq->allow_post ? 1 : (sizeof(buffer) - 1 - buflen),
			       0);
      } else
#endif
        recvlen = recv(fd, buffer + buflen,
                       httpreq->allow_post ? 1 : sizeof(buffer) - 1 - buflen, 0);

      if (recvlen < 0) {
	recvlen = 0;
	if (errno == EWOULDBLOCK && !forked) {
#if(_debug_ > 1)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Continue... (would block)", __FUNCTION__, __LINE__);
#endif
	  wblock = 1;
	} else {
#if(_debug_ > 1)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): %s_read(%d) failed!", __FUNCTION__, __LINE__,
#ifdef HAVE_SSL
                   sock->sslcon ? "SSL" :
#endif
                   "redir", fd);
#endif
	  return -1;
	}
      }

      if (recvlen == 0) done=1;
      else if (httpreq->data_in) {
	bcatblk(httpreq->data_in, buffer + buflen, recvlen);

	if (httpreq->allow_post &&
	    httpreq->data_in->slen > 4 &&
	    strncmp((char *)httpreq->data_in->data, "POST", 4))
	  httpreq->allow_post = 0;
      }

      buflen += recvlen;
      buffer[buflen] = 0;
    }

    if (httpreq->data_in && forked) {
      syslog(LOG_ERR,"should not happen");
      exit(1);
    }

    if (httpreq->data_in && !forked) {
      /*syslog(LOG_DEBUG, "buffer (%d)", httpreq->data_in->slen);*/
      if (httpreq->data_in->slen >= sizeof(buffer)) {
	syslog(LOG_ERR, "buffer too long (%d)", httpreq->data_in->slen);
	return -1;
      } else {
	buflen = httpreq->data_in->slen;
	memcpy(buffer, httpreq->data_in->data, buflen);
	buffer[buflen] = 0;
      }
    }

    if (buflen == 0) {
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): No data in HTTP request!", __FUNCTION__, __LINE__);
      if (!forked && wblock) return 1;
      return -1;
    }

    while ((eol = strstr(buffer, "\r\n"))) {
      size_t linelen = eol - buffer;
      *eol = 0;

      if (lines++ == 0) { /* first line */
	char *p1 = buffer;
	char qs_delim = '?';
	char *p2;

	if      (!strncmp("GET ",  p1, 4)) { p1 += 4; }
	else if (!strncmp("HEAD ", p1, 5)) { p1 += 5; }
	else if (httpreq->allow_post && !strncmp("POST ", p1, 5)) {
	  p1 += 5;
	  httpreq->is_post = 1;
	} else {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Unhandled http request: %s %d", __FUNCTION__, __LINE__, p1,
                   _options.uamallowpost);
	  return -1;
	}

	while (*p1 == ' ') p1++; /* Advance through additional white space */

	/* A proxy request, skip over the initial URL */
	if (!strncmp(p1, "http://", 7) && strlen(p1) > 8) {
	  p1 += 7;
	  while (*p1 && *p1 != '/') p1++;
	}
	else if (!strncmp(p1, "https://", 8) && strlen(p1) > 9) {
	  p1 += 8;
	  while (*p1 && *p1 != '/') p1++;
	}

	if (*p1 == '/') p1++;
	else { syslog(LOG_ERR, "parse error"); return -1; }

	/* The path ends with a ? or a space */
	p2 = strchr(p1, qs_delim);
	if (!p2) { qs_delim = ' '; p2 = strchr(p1, qs_delim); }
	if (!p2) { syslog(LOG_ERR, "parse error"); return -1; }
	*p2 = 0;

	strlcpy(path, p1, sizeof(httpreq->path));

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): The path: %s", __FUNCTION__, __LINE__, path);

	/* TODO: Should also check the Host: to make sure we are talking directly to uamlisten */

#ifdef ENABLE_JSON
	if (!strncmp(path, "json/", 5) && strlen(path) > 6) {
	  int i, last=strlen(path)-5;

	  conn->format = REDIR_FMT_JSON;

	  for (i=0; i < last; i++)
	    path[i] = path[i+5];

	  path[last]=0;

          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): The (json format) path: %s", __FUNCTION__, __LINE__, path);
	}
#endif

	if ((!strcmp(path, "logon")) || (!strcmp(path, "login")))
	  conn->type = REDIR_LOGIN;
	else if ((!strcmp(path, "logoff")) || (!strcmp(path, "logout")))
	  conn->type = REDIR_LOGOUT;
	else if (!strncmp(path, "www/", 4) && strlen(path) > 4)
	  conn->type = REDIR_WWW;
	else if (!strcmp(path, "status"))
	  conn->type = REDIR_STATUS;
	else if (!strncmp(path, "msdownload", 10))
        { conn->type = REDIR_MSDOWNLOAD; return 0; }
	else if (!strcmp(path, "prelogin"))
        { conn->type = REDIR_PRELOGIN; return 0; }
	else if (!strcmp(path, "macreauth"))
        { conn->type = REDIR_MACREAUTH; return 0; }
	else if (!strcmp(path, "abort"))
        { conn->type = REDIR_ABORT; return 0; }
#ifdef ENABLE_EWTAPI
	else if (!strncmp(path, "ewt/json", 8))
	  conn->type = REDIR_EWTAPI;
#endif
#ifdef ENABLE_WPAD
	else if (!strncmp(path, "wpad.dat", 8))
	  conn->type = REDIR_WPAD;
#endif

	if (qs_delim == '?') {
	  p1 = p2 + 1;
	  p2 = strchr(p1, ' ');

	  if (p2) {
	    *p2 = 0;

	    strlcpy(httpreq->qs, p1, sizeof(httpreq->qs));

#if(_debug_ > 1)
            if (_options.debug)
              syslog(LOG_DEBUG, "%s(%d): Query string: %s", __FUNCTION__, __LINE__, httpreq->qs);
#endif
	  }
	}
      } else if (linelen == 0) {
	/* end of headers */
#if(_debug_ > 1)
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): end of http-request", __FUNCTION__, __LINE__);
#endif
	done = 1;
	eoh = 1;
	break;
      } else {
	/* headers */
	char *p;
	size_t len;

	if (!strncasecmp(buffer,"Host:",5)) {
	  p = buffer + 5;
	  while (*p && isspace((int) *p)) p++;
	  strlcpy(httpreq->host, p, sizeof(httpreq->host));
#if(_debug_ > 1)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Host: %s", __FUNCTION__, __LINE__, httpreq->host);
#endif
	}
	else if (!strncasecmp(buffer,"Content-Length:",15)) {
	  p = buffer + 15;
	  while (*p && isspace((int) *p)) p++;
	  len = strlen(p);
	  if (len > 0) httpreq->clen = atoi(p);
#if(_debug_ > 1)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Content-Length: %s", __FUNCTION__, __LINE__, p);
#endif
	}
#ifdef ENABLE_USERAGENT
	else if (!strncasecmp(buffer,"User-Agent:",11)) {
	  p = buffer + 11;
	  while (*p && isspace((int) *p)) p++;
	  strlcpy(conn->s_state.redir.useragent,
                  p, sizeof(conn->s_state.redir.useragent));
#if(_debug_)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): User-Agent: %s", __FUNCTION__, __LINE__, conn->s_state.redir.useragent);
#endif
	}
#endif
#ifdef ENABLE_ACCEPTLANGUAGE
	else if (!strncasecmp(buffer,"Accept-Language:",16)) {
	  p = buffer + 16;
	  while (*p && isspace((int) *p)) p++;
	  strlcpy(conn->s_state.redir.acceptlanguage,
                  p, sizeof(conn->s_state.redir.acceptlanguage));
#if(_debug_ > 1)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Accept-Language: %s", __FUNCTION__, __LINE__, conn->s_state.redir.acceptlanguage);
#endif
	}
#endif
	else if (!strncasecmp(buffer,"Cookie:",7)) {
	  p = buffer + 7;
	  while (*p && isspace((int) *p)) p++;
	  strlcpy(conn->httpcookie, p, sizeof(conn->httpcookie));
#if(_debug_ > 1)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Cookie: %s", __FUNCTION__, __LINE__, conn->httpcookie);
#endif
	}
      }

      /* shift buffer */
      linelen += 2;
      for (i = 0; i < (int)(buflen - linelen); i++)
	buffer[i] = buffer[(int)linelen + i];

      /*syslog(LOG_DEBUG, "linelen=%d buflen=%d", linelen, buflen);*/
      buflen -= linelen;
    }

    if (!forked && !eoh && wblock) {
#if(_debug_ > 1)
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Didn't see end of headers, continue...", __FUNCTION__, __LINE__);
#endif
      /*syslog(LOG_DEBUG, "%.*s",httpreq->data_in->slen,httpreq->data_in->data);*/
      return 1;
    }
  }

  switch(conn->type) {

    case REDIR_STATUS:
      return 0;

#ifdef ENABLE_EWTAPI
    case REDIR_EWTAPI:
#ifdef HAVE_SSL
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): EWT API pre-process", __FUNCTION__, __LINE__);
      if (sock->sslcon) {
      }
#endif
      return 0;
#endif

    case REDIR_LOGIN:
      {
        bstring bt = bfromcstr("");

        if (!redir_getparam(redir, httpreq->qs, "lang", bt))
          bstrtocstr(bt, conn->lang, sizeof(conn->lang));

        if (redir_getparam(redir, httpreq->qs, "username", bt)) {
          syslog(LOG_ERR, "No username found in login request");
          conn->response = REDIR_ERROR_PROTOCOL;
          bdestroy(bt);
          return -1;
        }

        bstrtocstr(bt, conn->s_state.redir.username,
                   sizeof(conn->s_state.redir.username));

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): -->> Setting username=[%s]", __FUNCTION__, __LINE__, conn->s_state.redir.username);

        if (!redir_getparam(redir, httpreq->qs, "userurl", bt)) {
          bstring bt2 = bfromcstr("");
          redir_urldecode(bt, bt2);
          bstrtocstr(bt2, conn->s_state.redir.userurl,
                     sizeof(conn->s_state.redir.userurl));
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): -->> Setting userurl=[%s]", __FUNCTION__, __LINE__, conn->s_state.redir.userurl);
          bdestroy(bt2);
        }

        /* Indicates the reply to a challenge */
        if (!redir_getparam(redir, httpreq->qs, "continue", bt)) {
          conn->type = REDIR_LOGIN_CONTINUE;
        }

        if (!redir_getparam(redir, httpreq->qs, "WISPrVersion", bt)) {
          char rxversion[10];
          btrimws(bt);
          bstrtocstr(bt, rxversion, sizeof(rxversion));
          if (!strcmp(rxversion, "2.0")) {
            conn->s_state.redir.uamprotocol = REDIR_UAMPROT_WISPR2;
            if (_options.debug)
              syslog(LOG_DEBUG, "%s(%d): using uamprotocol: WISPr 2.0 (%d)", __FUNCTION__, __LINE__, conn->s_state.redir.uamprotocol);
          } else {
            conn->s_state.redir.uamprotocol = REDIR_UAMPROT_WISPR1;
            if (_options.debug)
              syslog(LOG_DEBUG, "%s(%d): using uamprotocol: WISPr 1.0 (%d)", __FUNCTION__, __LINE__, conn->s_state.redir.uamprotocol);
          }
        } else {
          conn->s_state.redir.uamprotocol = REDIR_UAMPROT_WISPR1;
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): using uamprotocol: WISPr 1.0 (%d)", __FUNCTION__, __LINE__, conn->s_state.redir.uamprotocol);
        }

        if (!redir_getparam(redir, httpreq->qs, "ntresponse", bt)) {
          conn->authdata.type = REDIR_AUTH_MSCHAPv2;
          redir_hextochar(bt->data, bt->slen,
                          conn->authdata.v.chapmsg.password,
                          RADIUS_CHAPSIZE);
        }
        else if (!redir_getparam(redir, httpreq->qs, "response", bt)) {
          conn->authdata.type = REDIR_AUTH_CHAP;
          redir_hextochar(bt->data, bt->slen,
                          conn->authdata.v.chapmsg.password,
                          RADIUS_CHAPSIZE);

          if (!redir_getparam(redir, httpreq->qs, "ident", bt) && bt->slen)
            conn->authdata.v.chapmsg.identity = atoi((char*)bt->data);
          else
            conn->authdata.v.chapmsg.identity = 0;
        }
        else if (!redir_getparam(redir, httpreq->qs, "password", bt)) {
          conn->authdata.type = REDIR_AUTH_PAP;
          if (_options.nochallenge) {
            /* Not using the challenge, therefore we expect the
             * password in "plain" text.
             */
            bstrtocstr(bt, (char*)conn->authdata.v.papmsg.password,
                       sizeof(conn->authdata.v.papmsg.password));
            conn->authdata.v.papmsg.len =
                strlen((char*)conn->authdata.v.papmsg.password);
          } else {
            conn->authdata.v.papmsg.len = bt->slen / 2;

            if (conn->authdata.v.papmsg.len > RADIUS_PWSIZE)
              conn->authdata.v.papmsg.len = RADIUS_PWSIZE;

            redir_hextochar(bt->data, bt->slen,
                            conn->authdata.v.papmsg.password,
                            conn->authdata.v.papmsg.len);
          }
        }
        else {
          if ((conn->s_state.redir.uamprotocol == REDIR_UAMPROT_WISPR2) &&
              !redir_getparam(redir, httpreq->qs, "WISPrEAPMsg", bt)){
            int rc;

            if (conn->authdata.type != REDIR_AUTH_NONE){
              syslog(LOG_ERR, "Request contains both password and eap message");
              conn->response = REDIR_ERROR_PROTOCOL;
              bdestroy(bt);
              return 0;
            }

            rc = base64decoder((char *)  bt->data,
                               &(conn->authdata.v.eapmsg));

            if (rc == 1) {
              syslog(LOG_ERR, "EAP message is too big, max allowed 1265 bytes");
              conn->response = REDIR_FAILED_MTU;
              bdestroy(bt);
              return 0;
            }

            if (rc == 2) {
              syslog(LOG_ERR, "Invalid EAP message encoding");
              conn->response = REDIR_ERROR_PROTOCOL;
              bdestroy(bt);
              return 0;
            }

            if (_options.debug) {
              char buffer[conn->authdata.v.eapmsg.len*2+1];
              bytetohex(conn->authdata.v.eapmsg.data,
                        conn->authdata.v.eapmsg.len, buffer,
                        conn->authdata.v.eapmsg.len*2+1);
              if (_options.debug)
                syslog(LOG_DEBUG, "%s(%d): decoded eap message from WISPr request (%d): %s", __FUNCTION__, __LINE__,
                       conn->authdata.v.eapmsg.len, buffer);
            }

            conn->authdata.type = REDIR_AUTH_EAP;
          }
        }

        if (conn->authdata.type == REDIR_AUTH_NONE) {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): No password found!", __FUNCTION__, __LINE__);
          bdestroy(bt);
          return -1;
        }
        bdestroy(bt);
      }
      break;

    case REDIR_PRELOGIN:
    case REDIR_LOGOUT:
      {
        bstring bt = bfromcstr("");
        if (!redir_getparam(redir, httpreq->qs, "userurl", bt)) {
          bstring bt2 = bfromcstr("");
          redir_urldecode(bt, bt2);
          bstrtocstr(bt2, conn->s_state.redir.userurl,
                     sizeof(conn->s_state.redir.userurl));
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): -->> Setting userurl=[%s]", __FUNCTION__, __LINE__, conn->s_state.redir.userurl);
          bdestroy(bt2);
        }
        bdestroy(bt);
      }
      break;

    case REDIR_WWW:
      {
        bstring bt = bfromcstr(path+4);
        bstring bt2 = bfromcstr("");
        redir_urldecode(bt, bt2);
        bstrtocstr(bt2, conn->wwwfile, sizeof(conn->wwwfile));
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): Serving file %s", __FUNCTION__, __LINE__, conn->wwwfile);
        bdestroy(bt2);
        bdestroy(bt);
      }
      break;

#ifdef ENABLE_WPAD
    case REDIR_WPAD:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): WPAD", __FUNCTION__, __LINE__);
      break;
#endif

    default:
      {
        snprintf(conn->s_state.redir.userurl,
		      sizeof(conn->s_state.redir.userurl),
		      "http://%s/%s%s%s",
		      httpreq->host, httpreq->path,
		      httpreq->qs[0] ? "?" : "",
		      httpreq->qs[0] ? httpreq->qs : "");

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): -->> Setting userurl=[%s]", __FUNCTION__, __LINE__, conn->s_state.redir.userurl);
      }
      break;

  }

  return 0;
}

/* Radius callback when access accept/reject/challenge has been received */
static int redir_cb_radius_auth_conf(struct radius_t *radius,
				     struct radius_packet_t *pack,
				     struct radius_packet_t *pack_req, void *cbp) {
  struct redir_conn_t *conn = (struct redir_conn_t*) cbp;
  struct radius_attr_t *attr = NULL;
  char attrs[RADIUS_ATTR_VLEN+1];
  int instance = 0;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Received RADIUS response", __FUNCTION__, __LINE__);

  if (!conn) {
    syslog(LOG_ERR, "No peer protocol defined");
    return 0;
  }

  if (!pack) { /* Timeout */
    syslog(LOG_ERR, "Radius request timed out");
    if (_options.noradallow) {
      conn->response = REDIR_SUCCESS;
      return 0;
    }
    conn->response = REDIR_FAILED_TIMEOUT;
    return 0;
  }

  /* We expect ACCESS-ACCEPT, ACCESS-REJECT (or ACCESS-CHALLENGE) */
  if ((pack->code != RADIUS_CODE_ACCESS_REJECT) &&
      (pack->code != RADIUS_CODE_ACCESS_CHALLENGE) &&
      (pack->code != RADIUS_CODE_ACCESS_ACCEPT)) {
    syslog(LOG_ERR, "Unknown radius access reply code %d", pack->code);
    conn->response = REDIR_FAILED_OTHER;
    return 0;
  }

  /* Reply message (might be present in both ACCESS-ACCEPT and ACCESS-REJECT */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_REPLY_MESSAGE, 0, 0, 0)) {
    memcpy(conn->replybuf, attr->v.t, attr->l-2);
    conn->replybuf[attr->l-2] = 0;
    conn->reply = conn->replybuf;
  }
  else {
    conn->replybuf[0] = 0;
    conn->reply = NULL;
  }

  config_radius_session(&conn->s_params, pack, 0, 0);

  /* Class */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_CLASS, 0, 0, 0)) {
    conn->s_state.redir.classlen = attr->l-2;
    memcpy(conn->s_state.redir.classbuf, attr->v.t, attr->l-2);
    if (_options.debug) {
      char buffer[conn->s_state.redir.classlen*2+1];
      bytetohex(conn->s_state.redir.classbuf,conn->s_state.redir.classlen,buffer, conn->s_state.redir.classlen*2+1);
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): +attribute Class (%zu bytes): %s", __FUNCTION__, __LINE__, conn->s_state.redir.classlen, buffer);
    }
  } else {
    conn->s_state.redir.classlen = 0;
  }

  /* Save CUI attribute in case of success, clear it otherwise */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_CHARGEABLE_USER_IDENTITY, 0, 0, 0)) {
    conn->s_state.redir.cuilen = attr->l-2;
    memcpy(conn->s_state.redir.cuibuf, attr->v.t, attr->l-2);
    if (_options.debug){
      char buffer[conn->s_state.redir.cuilen*2+1];
      bytetohex(conn->s_state.redir.cuibuf,conn->s_state.redir.cuilen,buffer, conn->s_state.redir.cuilen*2+1);
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): +attribute CUI (%zu bytes): %s", __FUNCTION__, __LINE__, conn->s_state.redir.cuilen, buffer);
    }
  } else {
    conn->s_state.redir.cuilen = 0;
  }

  /* Save Radius state attribute in case of challenge, clear it otherwise */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_STATE, 0, 0, 0)) {
    conn->s_state.redir.statelen = attr->l-2;
    memcpy(conn->s_state.redir.statebuf, attr->v.t, attr->l-2);
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): +attribute State (%d bytes)", __FUNCTION__, __LINE__, conn->s_state.redir.statelen);
  } else {
    conn->s_state.redir.statelen = 0;
  }

  /* Get returned EAP attributes and rebuild the EAP message */
  conn->authdata.v.eapmsg.len = 0;
  instance = 0;
  do {
    attr=NULL;
    if (!radius_getattr(pack, &attr, RADIUS_ATTR_EAP_MESSAGE, 0, 0,
                        instance++)) {
      if ((conn->authdata.v.eapmsg.len + (size_t)attr->l-2) > MAX_EAP_LEN) {
	syslog(LOG_ERR, "received EAP message from Radius packet is too big");
	conn->authdata.v.eapmsg.len = 0;
	return -1;
      }
      memcpy(conn->authdata.v.eapmsg.data + conn->authdata.v.eapmsg.len,
	     attr->v.t, (size_t)attr->l-2);
      conn->authdata.v.eapmsg.len += (size_t)attr->l-2;
    }
  } while (attr != NULL);

  /* Store the EAP msg id */
  if (conn->authdata.v.eapmsg.len >=3){
    conn->s_state.redir.eap_identity = conn->authdata.v.eapmsg.data[2];
  }

  if (_options.debug) {
    char buffer[conn->authdata.v.eapmsg.len*2+1];
    bytetohex(conn->authdata.v.eapmsg.data,
	      conn->authdata.v.eapmsg.len,buffer,
	      conn->authdata.v.eapmsg.len*2+1);
    syslog(LOG_DEBUG, "%s(%d): +attribute EAP msg (%d bytes): %s", __FUNCTION__, __LINE__,
           conn->authdata.v.eapmsg.len, buffer);
  }

  /* Get sendkey */
  if (_options.debug) {
    char hexString[RADIUS_ATTR_VLEN*2+1+64];
    uint8_t dstbuffer[RADIUS_ATTR_VLEN];
    size_t dstlen;

    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_MS,
			RADIUS_ATTR_MS_MPPE_SEND_KEY, 0)) {
      bytetosphex(&attr->t, attr->l, hexString, sizeof(hexString));

      /* Now decode the MPPE attribue */
      if (!radius_keydecode(radius, dstbuffer, RADIUS_ATTR_VLEN, &dstlen,
			    (uint8_t *)&attr->v.t, attr->l-2,
			    pack_req->authenticator, radius->secret,
			    radius->secretlen) != 0) {
	bytetosphex(dstbuffer, dstlen, hexString, sizeof(hexString));
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): plainstring MPPE_SEND_KEY: len %zu key %s", __FUNCTION__, __LINE__, dstlen, hexString);
      } else {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): received radius MPPE_SEND_KEY attribute (%d bytes): %s", __FUNCTION__, __LINE__, attr->l, hexString);
	syslog(LOG_ERR, "Decryption of MPPE_SEND_KEY failed");
      }
    }
    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_MS,
			RADIUS_ATTR_MS_MPPE_RECV_KEY, 0)) {

      bytetosphex(&attr->t, attr->l, hexString, sizeof(hexString));

      /* Now decode the MPPE attribue */
      if (!radius_keydecode(radius, dstbuffer, RADIUS_ATTR_VLEN, &dstlen,
			    (uint8_t *)&attr->v.t, attr->l-2,
			    pack_req->authenticator, radius->secret,
			    radius->secretlen) != 0) {
	bytetosphex(dstbuffer, dstlen, hexString, sizeof(hexString));
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): plainstring MPPE_RECV_KEY: len %zu key %s", __FUNCTION__, __LINE__, dstlen, hexString);
      } else {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): received radius MPPE_RECV_KEY attribute (%d bytes): %s", __FUNCTION__, __LINE__, attr->l, hexString);
	syslog(LOG_ERR, "Decryption of MPPE_RECV_KEY failed");
      }
    }
  }

  if (conn->s_params.sessionterminatetime) {
    time_t timenow = mainclock_now();
    if (timenow > conn->s_params.sessionterminatetime) {
      conn->response = REDIR_FAILED_OTHER;
      syslog(LOG_WARNING, "WISPr-Session-Terminate-Time in the past received: %s", attrs);
      return 0;
    }
  }

  switch (pack->code){
    case RADIUS_CODE_ACCESS_ACCEPT:
      conn->response = REDIR_SUCCESS;
      break;
    case RADIUS_CODE_ACCESS_CHALLENGE:
      conn->response = REDIR_CHALLENGE;
      break;
    case RADIUS_CODE_ACCESS_REJECT:
      conn->response = REDIR_FAILED_REJECT;
      break;
    default:
      syslog(LOG_ERR, "Unsupported radius access reply code %d", pack->code);
      return -1;
  }
  return 0;
}


/* Send radius Access-Request and wait for answer */
static int redir_radius(struct redir_t *redir, struct in_addr *addr,
			struct redir_conn_t *conn, char reauth) {
  uint8_t user_password[RADIUS_PWSIZE + 1];
  uint8_t chap_password[REDIR_MD5LEN + 2];
  uint8_t pap_challenge[REDIR_SHA256LEN];
  uint8_t chap_challenge[REDIR_MD5LEN];
  struct radius_packet_t radius_pack;
  struct radius_t *radius;      /* Radius client instance */
  struct timeval idleTime;	/* How long to select() */
  time_t endtime, now;          /* for radius wait */
  int maxfd = 0;	        /* For select() */
  fd_set fds;			/* For select() */
  int status;

  SHA256_CONTEXT context;

  char url[REDIR_URL_LEN];
  int n, m;

  if (radius_new(&radius, &redir->radiuslisten, 0, 0, 0)) {
    syslog(LOG_ERR, "radius_new: Failed to create radius");
    return -1;
  } 
  
  if (radius_init_q(radius, 8)) {
    syslog(LOG_ERR, "radius_init: Failed to create radius");
    radius_free(radius);
    return -1;
  }

  radius->nextid = radius_packet_id;

  if (radius->fd > maxfd)
    maxfd = radius->fd;

  radius_set(radius, redir->nas_hwaddr, (_options.debug & DEBUG_RADIUS));

  radius_set_cb_auth_conf(radius, redir_cb_radius_auth_conf);

  radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REQUEST);

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): created radius packet (code=%d, id=%d, len=%d)\n", __FUNCTION__, __LINE__, 
           radius_pack.code, radius_pack.id, ntohs(radius_pack.length));

  if(conn->lang[0])
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_COOVACHILLI, RADIUS_ATTR_COOVACHILLI_LANG,
		   0, (uint8_t*) conn->lang, strlen(conn->lang));

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 (uint8_t*) conn->s_state.redir.username,
		 strlen(conn->s_state.redir.username));

  if (redir->secret && *redir->secret) {
    //syslog(LOG_DEBUG, "SECRET: [%s]",redir->secret);
    /* Get MD5 hash on challenge and uamsecret */
    SHA256Init(&context);
    SHA256Update(&context, conn->s_state.redir.uamchal, REDIR_MD5LEN);
    SHA256Update(&context, (uint8_t *) redir->secret, strlen(redir->secret));
    SHA256Final(&context, pap_challenge);
  }
  else {
    memcpy(chap_challenge, conn->s_state.redir.uamchal, REDIR_MD5LEN);
  }


  switch (conn->authdata.type) {

    case REDIR_AUTH_PAP:
      if (_options.nochallenge) {
        strlcpy((char*)user_password,
                (char*)conn->authdata.v.papmsg.password,
                sizeof(user_password));
      } else {
        for (m=0; m < RADIUS_PWSIZE;) {
          for (n=0; n < REDIR_SHA256LEN; m++, n++) {
            user_password[m] =
                conn->authdata.v.papmsg.password[m] ^ pap_challenge[n];
          }
        }
      }
      user_password[conn->authdata.v.papmsg.len] = 0;

      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): User password %d [%s]", __FUNCTION__, __LINE__,
               conn->authdata.v.papmsg.len, user_password);

#ifdef HAVE_OPENSSL
      if (_options.mschapv2) {
        uint8_t response[50];
        uint8_t ntresponse[24];

        GenerateNTResponse(chap_challenge, chap_challenge,
                           (u_char *)conn->s_state.redir.username,
                           strlen(conn->s_state.redir.username),
                           user_password, strlen((char *)user_password),
                           ntresponse);

        /* peer challenge - same as auth challenge */
        memset(&response[0], 0, sizeof(response));
        memcpy(&response[2], chap_challenge, 16);
        memcpy(&response[26], ntresponse, 24);

        radius_addattr(radius, &radius_pack,
                       RADIUS_ATTR_VENDOR_SPECIFIC,
                       RADIUS_VENDOR_MS, RADIUS_ATTR_MS_CHAP_CHALLENGE, 0,
                       chap_challenge, 16);

        radius_addattr(radius, &radius_pack,
                       RADIUS_ATTR_VENDOR_SPECIFIC,
                       RADIUS_VENDOR_MS, RADIUS_ATTR_MS_CHAP2_RESPONSE, 0,
                       response, 50);
      } else {
#endif
        radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
                       (uint8_t*)user_password, conn->authdata.v.papmsg.len);
#ifdef HAVE_OPENSSL
      }
#endif
      break;
    case REDIR_AUTH_CHAP:
      chap_password[0] = conn->authdata.v.chapmsg.identity; /* Chap ident found on logon url */
      memcpy(chap_password+1, conn->authdata.v.chapmsg.password, REDIR_MD5LEN);
      radius_addattr(radius, &radius_pack, RADIUS_ATTR_CHAP_CHALLENGE, 0, 0, 0,
                     chap_challenge, REDIR_MD5LEN);
      radius_addattr(radius, &radius_pack, RADIUS_ATTR_CHAP_PASSWORD, 0, 0, 0,
                     chap_password, REDIR_MD5LEN+1);
      break;
    case REDIR_AUTH_MSCHAPv2:
      {
        uint8_t response[50];

        /* peer challenge - same as auth challenge */
        memcpy(response + 2, chap_challenge, 16);
        memcpy(response + 26, conn->authdata.v.chapmsg.password, 24);

        radius_addattr(radius, &radius_pack,
                       RADIUS_ATTR_VENDOR_SPECIFIC,
                       RADIUS_VENDOR_MS, RADIUS_ATTR_MS_CHAP_CHALLENGE, 0,
                       chap_challenge, 16);

        radius_addattr(radius, &radius_pack,
                       RADIUS_ATTR_VENDOR_SPECIFIC,
                       RADIUS_VENDOR_MS, RADIUS_ATTR_MS_CHAP2_RESPONSE, 0,
                       response, 50);
      }
      break;
    case REDIR_AUTH_EAP:
      /* Add one or more EAP-MSG attribute in Radius packet  */
      {
        size_t offset = 0;
        while (offset < conn->authdata.v.eapmsg.len) {
          size_t eaplen = 0;
          if ((conn->authdata.v.eapmsg.len - offset) > RADIUS_ATTR_VLEN)
            eaplen = RADIUS_ATTR_VLEN;
          else
            eaplen = conn->authdata.v.eapmsg.len - offset;

          if (radius_addattr(radius, &radius_pack,
                             RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
                             conn->authdata.v.eapmsg.data + offset, eaplen)) {
            syslog(LOG_ERR, "EAP message segmentation in EAP attributes failed");
            radius_free(radius);
            return -1;
          }
          offset += eaplen;
        }
        if (_options.debug) {
          char buffer[conn->authdata.v.eapmsg.len*2+1];
          bytetohex(conn->authdata.v.eapmsg.data,
                    conn->authdata.v.eapmsg.len,buffer,
                    conn->authdata.v.eapmsg.len*2+1);
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): +attribute EAP msg (%d bytes): %s", __FUNCTION__, __LINE__,
                   conn->authdata.v.eapmsg.len, buffer);
        }
      }
      break;
    default:
      syslog(LOG_ERR, "Invalid authentication type: %d",
             conn->authdata.type);
      radius_free(radius);
      return -1;
  }

  chilli_req_attrs(radius, &radius_pack,
		   ACCT_USER,
		   _options.framedservice ? RADIUS_SERVICE_TYPE_FRAMED :
		   RADIUS_SERVICE_TYPE_LOGIN, 0,
		   conn->nasport, conn->hismac,
		   &conn->hisip,
		   &conn->s_state);

  snprintf(url, sizeof(url), "http://%s:%d/logoff",
                inet_ntoa(redir->addr), redir->port);

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		 RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOGOFF_URL, 0,
		 (uint8_t*)url, strlen(url));

  if (_options.openidauth)
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_COOVACHILLI, RADIUS_ATTR_COOVACHILLI_CONFIG,
		   0, (uint8_t*)"allow-openidauth", 16);

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR,
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): sending radius packet (code=%d, id=%d, len=%d)\n", __FUNCTION__, __LINE__,
           radius_pack.code, radius_pack.id, ntohs(radius_pack.length));

  radius_req(radius, &radius_pack, conn);

  now = mainclock_now();
  endtime = now + REDIR_RADIUS_MAX_TIME;

  while (endtime > now) {

    FD_ZERO(&fds);
    if (radius->fd != -1) FD_SET(radius->fd, &fds);

    idleTime.tv_sec = 0;
    idleTime.tv_usec = REDIR_RADIUS_SELECT_TIME;
    radius_timeleft(radius, &idleTime);

    switch (status = select(maxfd + 1, &fds, NULL, NULL, &idleTime)) {
      case -1:
        syslog(LOG_ERR, "%s: select() returned -1!", strerror(errno));
        break;
      case 0:
        radius_timeout(radius);
        break;
      default:
        break;
    }

    if (status > 0) {
      if ((radius->fd != -1) && FD_ISSET(radius->fd, &fds) &&
	  radius_decaps(radius, 0) < 0) {
	syslog(LOG_ERR, "radius_ind() failed!");
      }
    }

    if (conn->response) {
      radius_free(radius);
      return 0;
    }

    now = mainclock_now();
  }

  radius_free(radius);
  return 0;
}

int is_local_user(struct redir_t *redir, struct redir_conn_t *conn) {
  uint8_t user_password[RADIUS_PWSIZE+1];
  uint8_t pap_challenge[REDIR_SHA256LEN];
  uint8_t chap_challenge[REDIR_MD5LEN];
  char u[256]; char p[256];
  size_t usernamelen, sz=1024;
  ssize_t len;
  int match=0;
  char *line=0;
  MD5_CTX context;
  SHA256_CONTEXT SHA256context;
  FILE *f;

  if (!_options.localusers) return 0;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): checking %s for user %s", __FUNCTION__, __LINE__, _options.localusers, conn->s_state.redir.username);

  if (!(f = fopen(_options.localusers, "r"))) {
    syslog(LOG_ERR, "%s: fopen() failed opening %s!", strerror(errno), _options.localusers);
    return 0;
  }

  if (_options.debug) {/*debug*/
    char buffer[64];
    redir_chartohex(conn->s_state.redir.uamchal, buffer, REDIR_MD5LEN);
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): challenge: %s", __FUNCTION__, __LINE__, buffer);
  }/**/

  if (redir->secret && *redir->secret) {
    SHA256Init(&SHA256context);
    SHA256Update(&SHA256context, (uint8_t*)conn->s_state.redir.uamchal, REDIR_MD5LEN);
    SHA256Update(&SHA256context, (uint8_t*)redir->secret, strlen(redir->secret));
    SHA256Final(&SHA256context, pap_challenge);
  }
  else {
    memcpy(chap_challenge, conn->s_state.redir.uamchal, REDIR_MD5LEN);
  }

  if (_options.debug) {/*debug*/
    char buffer[64];
    redir_chartohex(chap_challenge, buffer, REDIR_MD5LEN);
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): chap challenge: %s", __FUNCTION__, __LINE__, buffer);
  }/**/

  switch (conn->authdata.type){
    case REDIR_AUTH_PAP:
      if (_options.nochallenge) {
        strlcpy((char*)user_password,
                (char*)conn->authdata.v.papmsg.password,
                sizeof(user_password));
      } else {
        int n, m;
        for (m=0; m < RADIUS_PWSIZE;)
          for (n=0; n < REDIR_SHA256LEN; m++, n++)
            user_password[m] =
                conn->authdata.v.papmsg.password[m] ^ pap_challenge[n];
      }
      break;
    case REDIR_AUTH_CHAP:
      memcpy(user_password, conn->authdata.v.chapmsg.password, REDIR_MD5LEN);
      break;
    default:
      syslog(LOG_ERR, "Authentication method not supported for locally authenticated users: %d",
             conn->authdata.type);
      fclose(f);
      return 0;
  }

  user_password[RADIUS_PWSIZE] = 0;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): looking for %s", __FUNCTION__, __LINE__, conn->s_state.redir.username);
  usernamelen = strlen(conn->s_state.redir.username);

  line=(char*)malloc(sz);
  while ((len = getline(&line, &sz, f)) > 0) {
    if (len > 3 && len < sizeof(u) && line[0] != '#') {
      char *pl=line,  /* pointer to current line */
          *pu=u,     /* pointer to username     */
          *pp=p;     /* pointer to password     */

      /* username until the first ':' */
      while (*pl && *pl != ':')	*pu++ = *pl++;

      /* skip over ':' otherwise error */
      if (*pl == ':') pl++;
      else {
	syslog(LOG_WARNING, "not a valid localusers line: %s", line);
	continue;
      }

      /* password until the next ':' */
      while (*pl && *pl != ':' && *pl != '\n') *pp++ = *pl++;

      *pu = 0; /* null terminate */
      *pp = 0;

      if (usernamelen == strlen(u) &&
	  !strncmp(conn->s_state.redir.username, u, usernamelen)) {

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): found %s, checking password", __FUNCTION__, __LINE__, u);

	if (conn->authdata.type == REDIR_AUTH_PAP) {
	  if (!strcmp((char*)user_password, p))
	    match = 1;
	}
	else if (conn->authdata.type == REDIR_AUTH_CHAP) {
	  uint8_t tmp[REDIR_MD5LEN];
	  MD5Init(&context);
	  MD5Update(&context, (uint8_t*)&conn->authdata.v.chapmsg.identity, 1);
	  MD5Update(&context, (uint8_t*)p, strlen(p));
	  MD5Update(&context, chap_challenge, REDIR_MD5LEN);
	  MD5Final(tmp, &context);

	  if (!memcmp(user_password, tmp,  REDIR_MD5LEN))
	    match = 1;
	  else {
            if (_options.debug)
              syslog(LOG_DEBUG, "%s(%d): bad password for %s", __FUNCTION__, __LINE__, u);
	  }
	}

	break;
      }
    }
  }

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): user %s %s", __FUNCTION__, __LINE__,
           conn->s_state.redir.username,
           match ? "found" : "not found");

  fclose(f);
  free(line);
  return match;
}

/* redir_accept() does the following:
   1) forks a child process
   2) Accepts the tcp connection
   3) Analyses a HTTP get request
   4) GET request can be one of the following:
   a) Logon request with username and challenge response
   - Does a radius request
   - If OK send result to parent and redirect to welcome page
   - Else redirect to error login page
   b) Logoff request
   - Send logoff request to parent
   - Redirect to login page?
   c) Request for another server
   - Redirect to login server.

   Incoming requests are identified only by their IP address. No MAC
   address information is obtained. The main security problem is denial
   of service attacks by malicious hosts sending logoff requests for
   clients. This can be prevented by checking incoming packets for
   matching MAC and src IP addresses.
*/

int redir_accept(struct redir_t *redir, int idx) {
  int status;
  int new_socket;
  struct sockaddr_in address;
  struct sockaddr_in baddress;
  socklen_t addrlen;
  char buffer[128];

  addrlen = sizeof(struct sockaddr_in);

  if ((new_socket = safe_accept(redir->fd[idx],
				(struct sockaddr *)&address, &addrlen)) < 0) {
    if (errno != ECONNABORTED)
      syslog(LOG_ERR, "%s: accept() failed!", strerror(errno));
    return 0;
  }

  addrlen = sizeof(struct sockaddr_in);

  if (getsockname(redir->fd[idx], (struct sockaddr *)&baddress, &addrlen) < 0) {
    syslog(LOG_WARNING, "%s: getsockname() failed!", strerror(errno));
  }

  radius_packet_id++;

  /* This forks a new process. The child really should close all
     unused file descriptors and free memory allocated. This however
     is performed when the process exits, so currently we don't
     care */

  if ((status = redir_fork(new_socket, new_socket)) < 0) {
    syslog(LOG_ERR, "%s: fork() returned -1!", strerror(errno));
    safe_close(new_socket);
    return 0;
  }

  if (status > 0) { /* parent */
    safe_close(new_socket);
    return 0;
  }

  snprintf(buffer,sizeof(buffer),"%s",inet_ntoa(address.sin_addr));
  setenv("TCPREMOTEIP",buffer,1);
  setenv("REMOTE_ADDR",buffer,1);
  snprintf(buffer,sizeof(buffer),"%d",ntohs(address.sin_port));
  setenv("TCPREMOTEPORT",buffer,1);
  setenv("REMOTE_PORT",buffer,1);

  if (idx == 1 && _options.uamui && *_options.uamui) {

    char *binqqargs[2] = { _options.uamui, 0 } ;

    execv(*binqqargs, binqqargs);

  } else {
    int ret = redir_main(redir, new_socket, new_socket, &address, &baddress, idx, 0);
    safe_close(new_socket);
    return ret;
  }

  safe_close(new_socket);
  return 0;
}

static int _redir_close(int infd, int outfd) {
  /*
    char b[128];
    int max = 1000;
    if (shutdown(outfd, SHUT_WR) != 0)
    syslog(LOG_DEBUG, "shutdown socket for writing");
    if (!ndelay_on(infd))
    while(safe_read(infd, b, sizeof(b)) > 0 && max--);
    if (shutdown(infd, SHUT_RD) != 0)
    syslog(LOG_DEBUG, "shutdown socket for reading");
  */
  safe_close(outfd);
  safe_close(infd);
  return 0;
}

static int _redir_close_exit(int infd, int outfd) {
#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): close_exit", __FUNCTION__, __LINE__);
#endif
  _redir_close(infd,outfd);
  chilli_freeconn();
  dhcp_free(dhcp);
  options_destroy();
  exit(0);
}

#ifdef USING_IPC_UNIX
int redir_send_msg(struct redir_t *this, struct redir_msg_t *msg) {
  struct sockaddr_un remote;
  size_t len = sizeof(remote);
  int s;

  char filedest[512];

  statedir_file(filedest, sizeof(filedest), _options.unixipc, "chilli.ipc");

  if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return -1;
  }

  remote.sun_family = AF_UNIX;
  strlcpy(remote.sun_path, filedest,
          sizeof(remote.sun_path));

#if defined (__FreeBSD__)  || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  remote.sun_len = strlen(remote.sun_path) + 1;
#endif

  len = offsetof(struct sockaddr_un, sun_path) + strlen(remote.sun_path);

  if (safe_connect(s, (struct sockaddr *)&remote, len) == -1) {
    syslog(LOG_ERR, "%s: could not connect to %s", strerror(errno), remote.sun_path);
    safe_close(s);
    return -1;
  }

  if (safe_write(s, msg, sizeof(*msg)) != sizeof(*msg)) {
    syslog(LOG_ERR, "%s: could not write to %s", strerror(errno), remote.sun_path);
    safe_close(s);
    return -1;
  }

  shutdown(s, 2);
  safe_close(s);
  return 0;
}
#endif

pid_t redir_fork(int in, int out) {
  pid_t pid = chilli_fork(CHILLI_PROC_REDIR, "[redir]");
  if (pid < 0) return -1;
  if (pid == 0) {
    /*
     *  Setup child process
     */
    struct itimerval itval;

    set_signal(SIGALRM, redir_alarm);

    memset(&itval, 0, sizeof(itval));
    itval.it_interval.tv_sec = REDIR_MAXTIME;
    itval.it_interval.tv_usec = 0;
    itval.it_value.tv_sec = REDIR_MAXTIME;
    itval.it_value.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &itval, NULL)) {
      syslog(LOG_ERR, "%s: setitimer() failed!", strerror(errno));
    }

#if defined(F_DUPFD)
    if (fcntl(in,F_GETFL,0) == -1) return -1;
    safe_close(0);
    if (fcntl(in,F_DUPFD,0) == -1) return -1;
    if (fcntl(out,F_GETFL,1) == -1) return -1;
    safe_close(1);
    if (fcntl(out,F_DUPFD,1) == -1) return -1;
#else
    if (dup2(in,0) == -1) return -1;
    if (dup2(out,1) == -1) return -1;
#endif
  }

  return pid;
}

int redir_main_exit(struct redir_socket_t *socket, int forked, redir_request *rreq) {
  /* if (httpreq->data_in) bdestroy(httpreq->data_in); */
  /* if (!forked) return 0; XXXX*/
#ifdef HAVE_SSL
  if (socket->sslcon) {
#if(_debug_ > 1)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Shutting down SSL", __FUNCTION__, __LINE__);
#endif
    openssl_shutdown(socket->sslcon, 2);
    openssl_free(socket->sslcon);
    socket->sslcon = 0;
    if (rreq)
      rreq->sslcon = 0;
  }
#endif
  if (forked) _redir_close_exit(socket->fd[0], socket->fd[1]);
  return _redir_close(socket->fd[0], socket->fd[1]);
}

int redir_main(struct redir_t *redir,
	       int infd, int outfd,
	       struct sockaddr_in *address,
	       struct sockaddr_in *baddress,
	       int isui, redir_request *rreq) {

  char hexchal[1+(2*REDIR_MD5LEN)];
  unsigned char challenge[REDIR_MD5LEN];
  size_t bufsize = REDIR_MAXBUFFER;
  char buffer[bufsize+1];
  struct redir_msg_t msg;
  ssize_t buflen;

  /**
   * connection state
   *  0 == un-authenticated
   *  1 == authenticated
   */
  int state = 0;

  /**
   * require splash or not
   */
  int splash = 0;

  struct redir_conn_t conn;
  struct redir_socket_t socket;
  struct redir_httpreq_t httpreq;

  /* We are forked when the redir_request is null */
  int forked = (rreq == 0);
  int err;


  memset(&httpreq,0,sizeof(httpreq));
  httpreq.allow_post = isui || _options.uamallowpost;

  mainclock_tick();

  if (rreq) {
    httpreq.data_in = rreq->wbuf;
  }

#define redir_memcopy(msgtype)                                          \
  redir_challenge(challenge);                                           \
  redir_chartohex(challenge, hexchal, REDIR_MD5LEN);                    \
  msg.mtype = msgtype;                                                  \
  memcpy(conn.s_state.redir.uamchal, challenge, REDIR_MD5LEN);          \
  if (_options.debug)							\
    syslog(LOG_DEBUG, "%s(%d): ---->>> resetting challenge: %s", __FUNCTION__, __LINE__, hexchal)

#ifdef USING_IPC_UNIX
#define redir_msg_send(msgopt)                                          \
  msg.mdata.opt = msgopt;                                               \
  memcpy(&msg.mdata.address, address, sizeof(msg.mdata.address));       \
  memcpy(&msg.mdata.baddress, baddress, sizeof(msg.mdata.baddress));    \
  memcpy(&msg.mdata.params, &conn.s_params, sizeof(msg.mdata.params));  \
  memcpy(&msg.mdata.redir, &conn.s_state.redir, sizeof(msg.mdata.redir)); \
  if (redir_send_msg(redir, &msg) < 0) {                                \
    syslog(LOG_ERR, "%s: write() failed! msgfd=%d type=%ld len=%d",     \
           strerror(errno), redir->msgfd, msg.mtype, (int)sizeof(msg.mdata)); \
    return redir_main_exit(&socket, forked, rreq);                      \
  }
#else
#define redir_msg_send(msgopt)                                          \
  msg.mdata.opt = msgopt;                                               \
  memcpy(&msg.mdata.address, address, sizeof(msg.mdata.address));       \
  memcpy(&msg.mdata.baddress, baddress, sizeof(msg.mdata.baddress));    \
  memcpy(&msg.mdata.params, &conn.s_params, sizeof(msg.mdata.params));  \
  memcpy(&msg.mdata.redir, &conn.s_state.redir, sizeof(msg.mdata.redir)); \
  if (msgsnd(redir->msgid, (void *)&msg, sizeof(msg.mdata), 0) < 0) {   \
    syslog(LOG_ERR, "%s: msgsnd() failed! msgid=%d type=%ld len=%d",    \
           strerror(errno), redir->msgid, msg.mtype, (int)sizeof(msg.mdata)); \
    return redir_main_exit(&socket, forked, rreq);                      \
  }
#endif

  /*
   *  Initializations
   */
  memset(&socket, 0, sizeof(socket));
  memset(hexchal, 0, sizeof(hexchal));
  memset(&conn, 0, sizeof(conn));
  memset(&msg, 0, sizeof(msg));

  socket.fd[0] = infd;
  socket.fd[1] = outfd;

  redir->starttime = mainclock_now();

  /*
    if (ndelay_on(socket.fd[0])) {
    syslog(LOG_ERR, "%s: fcntl() failed", strerror(errno));
    return redir_main_exit(&socket, forked, rreq);
    }
  */

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Calling redir_getstate()", __FUNCTION__, __LINE__);
#endif

  /*
   *  Fetch the state of the client
   */

  termstate = REDIR_TERM_GETSTATE;

  if (!redir->cb_getstate) {
    syslog(LOG_ERR, "No cb_getstate() defined!");
    return redir_main_exit(&socket, forked, rreq);
  }

  /* get_state returns 0 for unauth'ed and 1 for auth'ed */
  state = redir->cb_getstate(redir, address, baddress, &conn);

  if (state == -1) {
#if(_debug_ > 1)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): getstate() session not found", __FUNCTION__, __LINE__);
#endif

#ifdef ENABLE_EWTAPI
    if (_options.uamuissl && isui) {
      /*
       *  Allow external (WAN) access to EWT API if available,
       *  always under SSL.
       */
#if(_debug_ > 1)
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): redir connection is SSL", __FUNCTION__, __LINE__);
#endif
      conn.flags |= USING_SSL;
    } else
#endif
    {
      return redir_main_exit(&socket, forked, rreq);
    }
  }

  splash = (conn.s_params.flags & REQUIRE_UAM_SPLASH) == REQUIRE_UAM_SPLASH;

  /*
   *  Parse the request, updating the status
   */
#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Receiving HTTP%s Request", __FUNCTION__, __LINE__, (conn.flags & USING_SSL) ? "S" : "");
#endif

#ifdef HAVE_SSL
  if ((conn.flags & USING_SSL) == USING_SSL) {
    char done = 0, loop = 0;

    if (!rreq || !rreq->sslcon) {
      socket.sslcon = openssl_accept_fd(initssl(), socket.fd[0], 10, &conn);
      if (rreq) {
	/* we were forked */
	rreq->sslcon = socket.sslcon;
      } else {
	/* not forked, so loop */
	loop = 1;
      }
    } else {
      socket.sslcon = rreq->sslcon;
    }

#if(_debug_ > 1)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): SSL loop %d", __FUNCTION__, __LINE__, loop);
#endif

    while (!done) {
      switch(openssl_check_accept(socket.sslcon, &conn)) {
        case -1:
#if(_debug_ > 1)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): redir error, redir_main_exit", __FUNCTION__, __LINE__);
#endif
          return redir_main_exit(&socket, forked, rreq);
        case 1:
          if (!loop) {
            if (_options.debug)
              syslog(LOG_DEBUG, "%s(%d): Continue... SSL pending", __FUNCTION__, __LINE__);
            return 1;
          }
          break;
        case 0: done = 1;
        default: break;
      }
      if (!loop) done = 1;
    }

#if(_debug_ > 1)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): HTTPS Accepted", __FUNCTION__, __LINE__);
#endif
  }
#endif


  termstate = REDIR_TERM_GETREQ;
  switch (err = redir_getreq(redir, &socket, &conn, &httpreq, rreq)) {
    case 0:
      break;
    case 1:
#if(_debug_ > 1)
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Continue...", __FUNCTION__, __LINE__);
#endif
      return 1;
    default:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Error calling get_req. Terminating %d", __FUNCTION__, __LINE__, err);
      return redir_main_exit(&socket, forked, rreq);
  }

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Processing HTTP%s Request", __FUNCTION__, __LINE__, (conn.flags & USING_SSL) ? "S" : "");
#endif

  switch (conn.type) {
#ifdef ENABLE_WPAD
    case REDIR_WPAD:
#endif
#ifdef ENABLE_EWTAPI
    case REDIR_EWTAPI:
#endif
    case REDIR_WWW:
      {
#ifdef ENABLE_EWTAPI
        char isEWT = conn.type == REDIR_EWTAPI;
#endif
#ifdef ENABLE_WPAD
        char isWPAD = conn.type == REDIR_WPAD;
#endif
        pid_t forkpid;
        int fd = -1;

        if (_options.wwwdir && (*conn.wwwfile
#ifdef ENABLE_EWTAPI
                                || isEWT
#endif
#ifdef ENABLE_WPAD
                                || isWPAD
#endif
                                )) {
          char *ctype = "text/plain";
          char *filename = conn.wwwfile;
          size_t namelen = strlen(filename);
          int parse = 0;

          /* check filename */
#ifdef ENABLE_EWTAPI
          if (isEWT) {
            if (!(conn.s_params.flags & ADMIN_LOGIN)) {
              syslog(LOG_WARNING, "Permission denied to EWT API");
              return redir_main_exit(&socket, forked, rreq);
            }
          } else
#endif
#ifdef ENABLE_WPAD
            if (isWPAD) {
              filename = "wpad.dat";
              namelen = strlen(filename);
            } else
#endif
            {
              char *p;
              int cnt = 0;
              for (p=filename; *p; p++) {
                if (*p == '.' || *p == '_'|| *p == '-' || *p == '/') {
                  cnt++;
                  if (cnt == 1)
                    continue; /*ok*/
                } else {
                  cnt = 0;
                }
                if (*p >= 'a' && *p <= 'z') continue;
                if (*p >= 'A' && *p <= 'Z') continue;
                if (*p >= '0' && *p <= '9') continue;
                /* invalid file name! */
                syslog(LOG_ERR, "invalid www request [%s]!", filename);
                return redir_main_exit(&socket, forked, rreq);
              }
            }

          /* serve the local content */

#ifdef ENABLE_EWTAPI
          if (isEWT) { ctype = "application/json"; parse = 1; } else
#endif

            if      (!strcmp(filename + (namelen - 5), ".html")) ctype = "text/html";
            else if (!strcmp(filename + (namelen - 4), ".gif"))  ctype = "image/gif";
            else if (!strcmp(filename + (namelen - 3), ".js"))   ctype = "text/javascript";
            else if (!strcmp(filename + (namelen - 4), ".css"))  ctype = "text/css";
            else if (!strcmp(filename + (namelen - 4), ".jpg"))  ctype = "image/jpeg";
            else if (!strcmp(filename + (namelen - 4), ".mp4"))  ctype = "video/mp4";
            else if (!strcmp(filename + (namelen - 4), ".ogv"))  ctype = "video/ogg";
            else if (!strcmp(filename + (namelen - 4), ".png"))  ctype = "image/png";
            else if (!strcmp(filename + (namelen - 4), ".swf"))  ctype = "application/x-shockwave-flash";
            else if (!strcmp(filename + (namelen - 4), ".chi")){ ctype = "text/html"; parse = 1; }
            else if (!strcmp(filename + (namelen - 4), ".cjs")){ ctype = "text/javascript"; parse = 1; }
            else if (!strcmp(filename + (namelen - 5), ".json")) ctype = "application/json";
            else if (!strcmp(filename + (namelen - 4), ".dat")){ ctype = "application/x-ns-proxy-autoconfig";
#ifdef ENABLE_WPAD
              if (isWPAD && _options.wpadpacfile) {
                struct stat statbuf;
                if (stat(_options.wpadpacfile, &statbuf) == 0)
                  if (statbuf.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))
                    parse = 1;
              }
#endif
            }
            else {
              /* we do not serve it! */
              syslog(LOG_ERR, "invalid file extension! [%s]", filename);
              return redir_main_exit(&socket, forked, rreq);
            }

          if (!forked) {
            /*
             *  If not forked off the main process already, fork now
             *  before doing the chroot(), chrdir(), and so on..
             */
            forkpid = redir_fork(infd, outfd);
            if (forkpid) { /* parent or error */
              return redir_main_exit(&socket, forked, rreq);
            }
          }

          if (parse) {

            if (!_options.wwwbin) {
              syslog(LOG_ERR, "the 'wwwbin' setting must be configured for CGI use");
              return redir_main_exit(&socket, forked, rreq);
            }

            if (ndelay_off(socket.fd[0])) {
              syslog(LOG_ERR, "%s: fcntl() failed", strerror(errno));
            }

#ifdef HAVE_SSL
            if (socket.sslcon) {

              /*
               * If the connection is SSL, we need to fork again. The child will do the exec
               * while the parent will provide the SSL wrapping.
               */
              int ptoc[2];
              int ctop[2];

              if (pipe(ptoc) == -1 || pipe(ctop) == -1) {
                syslog(LOG_ERR, "%s: pipe() failed", strerror(errno));
                return redir_main_exit(&socket, forked, rreq);
              }

              forkpid = redir_fork(ptoc[0], ctop[1]);

              if (forkpid < 0) {
                syslog(LOG_ERR, "%s: fork() failed", strerror(errno));
                return redir_main_exit(&socket, forked, rreq);
              }

              forked = 1;
              if (forkpid > 0) {
                /* parent */

                int rd, clen = httpreq.clen;

                safe_close(ptoc[0]);
                safe_close(ctop[1]);

#if(_debug_ > 1)
                if (_options.debug)
                  syslog(LOG_DEBUG, "%s(%d): ssl_wrapper(%d)", __FUNCTION__, __LINE__, getpid());
#endif

                while (clen > 0) {
                  rd = clen > bufsize ? bufsize : clen;
#if(_debug_ > 1)
                  if (_options.debug)
                    syslog(LOG_DEBUG, "%s(%d): reading(%d)", __FUNCTION__, __LINE__, rd);
#endif
                  if ((buflen = openssl_read(socket.sslcon, buffer, rd, 0)) > 0) {
                    if (safe_write(ptoc[1], buffer, (size_t) buflen) < 0) {
                      syslog(LOG_ERR, "%s: error", strerror(errno));
                      return redir_main_exit(&socket, forked, rreq);
                    }
                    clen -= buflen;
                  }
                }

                while (1) {
#if(_debug_ > 1)
                  if (_options.debug)
                    syslog(LOG_DEBUG, "%s(%d): script_read", __FUNCTION__, __LINE__);
#endif
                  if ((buflen = safe_read(ctop[0], buffer, bufsize)) > 0) {
#if(_debug_ > 1)
                    if (_options.debug)
                      syslog(LOG_DEBUG, "%s(%d): script_read(%zd)", __FUNCTION__, __LINE__, buflen);
#endif
                    if (redir_write(&socket, buffer, (size_t) buflen) < 0) {
                      syslog(LOG_ERR, "%s: redir_write() failed!", strerror(errno));
                      break;
                    }
#if(_debug_ > 1)
                    if (_options.debug)
                      syslog(LOG_DEBUG, "%s(%d): ssl_write(%zd)", __FUNCTION__, __LINE__, buflen);
#endif
                  } else {
#if(_debug_ > 1)
                    if (_options.debug)
                      syslog(LOG_DEBUG, "%s(%d): done", __FUNCTION__, __LINE__);
#endif
                    break;
                  }
                }

#if(_debug_ > 1)
                if (_options.debug)
                  syslog(LOG_DEBUG, "%s(%d): ssl_wrapper(%d) done", __FUNCTION__, __LINE__, getpid());
#endif

                safe_close(ptoc[1]);
                safe_close(ctop[0]);

                return redir_main_exit(&socket, forked, rreq);

              } else {
                /* child */

                safe_close(ptoc[1]);
                safe_close(ctop[0]);

#if(_debug_ > 1)
                if (_options.debug)
                  syslog(LOG_DEBUG, "%s(%d): script(%d)", __FUNCTION__, __LINE__, getpid());
#endif
              }
            }
#endif

#ifdef HAVE_SSL
            if (socket.sslcon) {
              setenv("HTTPS", "on", 1);
              sleep(1);
            }
#endif

            snprintf(buffer, sizeof(buffer), "%zd", httpreq.clen > 0 ? httpreq.clen : 0);
            setenv("CONTENT_LENGTH", buffer, 1);

            setenv("REQUEST_METHOD", httpreq.is_post ? "POST" : "GET", 1);
            setenv("REQUEST_URI", httpreq.path, 1);
            setenv("QUERY_STRING", httpreq.qs, 1);
            setenv("SERVER_NAME", httpreq.host, 1);
            setenv("HTTP_COOKIE", conn.httpcookie, 1);

            snprintf(buffer, sizeof(buffer), "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
                          conn.hismac[0], conn.hismac[1], conn.hismac[2],
                          conn.hismac[3], conn.hismac[4], conn.hismac[5]);
            setenv("REMOTE_MAC", buffer, 1);

            setenv("AUTHENTICATED", conn.s_state.authenticated &&
                   (conn.s_params.flags&REQUIRE_UAM_SPLASH)==0 ? "1" : "0", 1);

            setenv("CHI_SESSION_ID", conn.s_state.sessionid, 1);
            setenv("CHI_USERNAME", conn.s_state.redir.username, 1);
            setenv("CHI_USERURL", conn.s_state.redir.userurl, 1);
            snprintf(buffer, sizeof(buffer), "%" PRIu64, conn.s_state.input_octets);
            setenv("CHI_INPUT_BYTES", buffer, 1);
            snprintf(buffer, sizeof(buffer), "%" PRIu64, conn.s_state.output_octets);
            setenv("CHI_OUTPUT_BYTES", buffer, 1);
            snprintf(buffer, sizeof(buffer), "%" PRIu64, conn.s_params.sessiontimeout);
            setenv("CHI_SESSION_TIMEOUT", buffer, 1);

            redir_chartohex(conn.s_state.redir.uamchal, buffer, REDIR_MD5LEN);
            setenv("CHI_CHALLENGE", buffer, 1);

            switch (conn.type) {
#ifdef ENABLE_EWTAPI
              case REDIR_EWTAPI:
                {
                  ewtapi(redir, &socket, &conn, &httpreq);
                }
                break;
#endif
#ifdef ENABLE_WPAD
              case REDIR_WPAD:
                if (isWPAD && _options.wpadpacfile) {
                  char *binqqargs[3] = { _options.wpadpacfile, 0 } ;
                  if (_options.debug)
                    syslog(LOG_DEBUG, "%s(%d): Running: %s", __FUNCTION__, __LINE__, _options.wpadpacfile);
                  execv(*binqqargs, binqqargs);
                  break;
                }
#endif
              case REDIR_WWW:
                /* XXX: Todo: look for malicious content! */
                {
                  char *binqqargs[3] = { _options.wwwbin, buffer, 0 } ;

                  if (_options.debug)
                    syslog(LOG_DEBUG, "%s(%d): Running: %s %s/%s", __FUNCTION__, __LINE__, _options.wwwbin, _options.wwwdir, filename);
                  snprintf(buffer, sizeof(buffer), "%s/%s", _options.wwwdir, filename);

                  execv(*binqqargs, binqqargs);
                }
                break;
            }

            return redir_main_exit(&socket, forked, rreq);
          }

          if ( (_options.uid == 0 && !chroot(_options.wwwdir) && !chdir("/")) ||
               (_options.uid != 0 && !chdir(_options.wwwdir)) ) {
            int gzip = 1;
            char filebuff[1024];

            snprintf(filebuff, sizeof(filebuff), "%s.gz", filename);

            fd = open(filebuff, O_RDONLY);

            if (fd < 0) {
              gzip = 0;
              fd = open(filename, O_RDONLY);
            }

            if (fd > 0) {

              if (ndelay_off(socket.fd[0])) {
                syslog(LOG_ERR, "%s: fcntl() failed", strerror(errno));
              }

              snprintf(buffer, bufsize,
                            "HTTP/1.1 200 OK\r\n%s"
                            "Connection: close\r\n"
                            "Content-type: %s\r\n\r\n",
                            gzip ? "Content-Encoding: gzip\r\n" : "",
                            ctype);

              if (redir_write(&socket, buffer, strlen(buffer)) < 0) {
                syslog(LOG_ERR, "%s: redir_write()", strerror(errno));
              }

              while ((buflen = safe_read(fd, buffer, bufsize)) > 0)
                if (redir_write(&socket, buffer, (size_t) buflen) < 0)
                  syslog(LOG_ERR, "%s: redir_write()", strerror(errno));

              safe_close(fd);
            }
            else syslog(LOG_ERR, "could not open local content file %s!", filename);
          }
          else syslog(LOG_ERR, "chroot/chdir to %s was not successful\n", _options.wwwdir);

          return _redir_close_exit(infd, outfd); /* which exits */
        }
        else syslog(LOG_ERR, "Required: 'wwwdir' (in chilli.conf) and 'file' query-string param");

        return redir_main_exit(&socket, forked, rreq);
      }
  }

  termstate = REDIR_TERM_PROCESS;

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Processing received request", __FUNCTION__, __LINE__);
#endif

  /* default hexchal for use in replies */
  redir_chartohex(conn.s_state.redir.uamchal, hexchal, REDIR_MD5LEN);

  switch (conn.type) {

    case REDIR_LOGIN: {
      char reauth = 0;

      /* Was client was already logged on? */
      if (state == 1) {

        if (splash) {

          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): redir_accept: SPLASH reauth", __FUNCTION__, __LINE__);
          reauth = 1;

        } else {

          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): redir_accept: already logged on", __FUNCTION__, __LINE__);

          redir_reply(redir, &socket, &conn, REDIR_ALREADY, NULL, 0,
                      NULL, NULL, conn.s_state.redir.userurl, NULL,
                      (char *)conn.s_params.url, conn.hismac,
                      &conn.hisip, httpreq.qs);

          return redir_main_exit(&socket, forked, rreq);
        }
      }

      /* Did the challenge expire? */
      if (_options.challengetimeout2 &&
          (conn.s_state.uamtime + _options.challengetimeout2) <
          mainclock_now()) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): redir_accept: challenge expired: %ld : %ld", __FUNCTION__, __LINE__,
                 (long) conn.s_state.uamtime, (long) mainclock_now());

        redir_memcopy(REDIR_CHALLENGE);
        redir_msg_send(REDIR_MSG_OPT_REDIR);

        redir_reply(redir, &socket, &conn, REDIR_FAILED_OTHER, NULL,
                    0, hexchal, NULL, NULL, NULL,
                    0, conn.hismac, &conn.hisip, httpreq.qs);

        return redir_main_exit(&socket, forked, rreq);
      }

      if (is_local_user(redir, &conn)) {
        session_param_defaults(&conn.s_params);
        conn.response = REDIR_SUCCESS;
      }
      else {

#ifdef ENABLE_MODULES
        int i;
        int flags = 0;
#endif

        if (!forked) {
          /*
           *  When waiting for RADIUS, we need to be forked.
           *  TODO: make redir_radius asynchronous.
           */
          pid_t forkpid = redir_fork(infd, outfd);
          if (forkpid) { /* parent or error */
            return redir_main_exit(&socket, forked, rreq);
          }
        }

#ifdef ENABLE_MODULES
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): checking modules...", __FUNCTION__, __LINE__);
        for (i=0; i < MAX_MODULES; i++) {
          if (!_options.modules[i].name[0]) break;
          if (_options.modules[i].ctx) {
            struct chilli_module *m =
                (struct chilli_module *)_options.modules[i].ctx;
            if (m->redir_login) {
              int modresult = m->redir_login(redir, &conn, &socket);
              flags |= modresult;
              switch(chilli_mod_state(modresult)) {
                case CHILLI_MOD_ERROR:
                  return redir_main_exit(&socket, forked, rreq);
                default:
                  break;
              }
            }
          }
        }
        if (flags & CHILLI_MOD_REDIR_SKIP_RADIUS) {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Skipping RADIUS authentication", __FUNCTION__, __LINE__);
        } else {
#endif

          termstate = REDIR_TERM_RADIUS;

          if (optionsdebug)
            syslog(LOG_DEBUG, "%s(%d): redir_accept: Sending RADIUS request", __FUNCTION__, __LINE__);

          redir_radius(redir, &address->sin_addr, &conn, reauth);
          termstate = REDIR_TERM_REPLY;

#ifdef ENABLE_MODULES
        }
#endif

#if(_debug_ > 1)
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): Received RADIUS reply", __FUNCTION__, __LINE__);
#endif
      }

      if (conn.response == REDIR_SUCCESS) { /* Accept-Accept */

        conn.s_params.flags &= ~REQUIRE_UAM_SPLASH;

        if (reauth) {
          conn.s_params.flags |= IS_UAM_REAUTH;
        }

        msg.mtype = REDIR_LOGIN;

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): handling Access-Accept", __FUNCTION__, __LINE__);

        redir_reply(redir, &socket, &conn, REDIR_SUCCESS, NULL,
                    conn.s_params.sessiontimeout, NULL,
                    conn.s_state.redir.username,
                    conn.s_state.redir.userurl, conn.reply,
                    (char *)conn.s_params.url,
                    conn.hismac, &conn.hisip, httpreq.qs);

        /* set params and redir data */
        redir_msg_send(REDIR_MSG_OPT_REDIR | REDIR_MSG_OPT_PARAMS);

      } else { /* Access-Reject */

        int hasnexturl = (strlen((char *)conn.s_params.url) > 5);

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): handling Access-Reject", __FUNCTION__, __LINE__);

        if (!hasnexturl) {
          if (_options.challengetimeout) {
            redir_memcopy(REDIR_CHALLENGE);
	  }
        } else {
          msg.mtype = REDIR_NOTYET;
        }

        redir_reply(redir, &socket, &conn, conn.response,
                    NULL,
                    0, hexchal, NULL, conn.s_state.redir.userurl, conn.reply,
                    (char *)conn.s_params.url, conn.hismac,
                    &conn.hisip, httpreq.qs);

        /* set params, redir data, and reset session-id */
        redir_msg_send(REDIR_MSG_OPT_REDIR | REDIR_MSG_OPT_PARAMS |
                       (conn.response == REDIR_CHALLENGE ? 0 : REDIR_MSG_NSESSIONID));
      }

      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): -->> Msg userurl=[%s]\n", __FUNCTION__, __LINE__, conn.s_state.redir.userurl);
      return redir_main_exit(&socket, forked, rreq);
    }

    case REDIR_LOGOUT:
      {
        redir_memcopy(REDIR_LOGOUT);
        redir_msg_send(REDIR_MSG_OPT_REDIR);

        conn.s_state.authenticated=0;

        redir_reply(redir, &socket, &conn, REDIR_LOGOFF, NULL, 0,
                    hexchal, NULL, conn.s_state.redir.userurl, NULL,
                    NULL, conn.hismac, &conn.hisip, httpreq.qs);

        return redir_main_exit(&socket, forked, rreq);
      }

    case REDIR_MACREAUTH:
      if (_options.macauth) {
        msg.mtype = REDIR_MACREAUTH;
        redir_msg_send(0);
      }
      /* drop down */

    case REDIR_PRELOGIN:
      /* Did the challenge expire? */
      if (!conn.s_state.uamtime ||
          (_options.challengetimeout &&
           (conn.s_state.uamtime + _options.challengetimeout) <
           mainclock_now())) {
        redir_memcopy(REDIR_CHALLENGE);
        redir_msg_send(REDIR_MSG_OPT_REDIR);
      }

      if (state == 1) {
        redir_reply(redir, &socket, &conn, REDIR_ALREADY,
                    NULL, 0, NULL, NULL, conn.s_state.redir.userurl, NULL,
                    (char *)conn.s_params.url, conn.hismac,
                    &conn.hisip, httpreq.qs);
      }
      else {
        redir_reply(redir, &socket, &conn, REDIR_NOTYET,
                    NULL, 0, hexchal, NULL, conn.s_state.redir.userurl, NULL,
                    NULL, conn.hismac, &conn.hisip, httpreq.qs);
      }
      return redir_main_exit(&socket, forked, rreq);

    case REDIR_ABORT:
      if (state == 1) {
        redir_reply(redir, &socket, &conn, REDIR_ABORT_NAK,
                    NULL, 0, NULL, NULL, conn.s_state.redir.userurl, NULL,
                    NULL, conn.hismac, &conn.hisip, httpreq.qs);
      }
      else {
        redir_memcopy(REDIR_ABORT);
        redir_msg_send(0);

        redir_reply(redir, &socket, &conn, REDIR_ABORT_ACK,
                    NULL, 0, hexchal, NULL, conn.s_state.redir.userurl, NULL,
                    NULL, conn.hismac, &conn.hisip, httpreq.qs);
      }
      return redir_main_exit(&socket, forked, rreq);

    case REDIR_ABOUT:
      redir_reply(redir, &socket, &conn, REDIR_ABOUT, NULL,
                  0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, httpreq.qs);
      return redir_main_exit(&socket, forked, rreq);

    case REDIR_STATUS:
      {
        uint32_t sessiontime;
        uint32_t timeleft;
        time_t timenow = mainclock_now();

        /* Did the challenge expire? */
        if (_options.challengetimeout &&
            (conn.s_state.uamtime + _options.challengetimeout) < timenow) {
          redir_memcopy(REDIR_CHALLENGE);
          redir_msg_send(REDIR_MSG_OPT_REDIR);
        }

        sessiontime = timenow - conn.s_state.start_time;

        if (conn.s_params.sessiontimeout)
          timeleft = conn.s_params.sessiontimeout - sessiontime;
        else
          timeleft = 0;

        redir_reply(redir, &socket, &conn, REDIR_STATUS, NULL, timeleft,
                    hexchal, conn.s_state.redir.username,
                    conn.s_state.redir.userurl, conn.reply,
                    0, conn.hismac, &conn.hisip, httpreq.qs);

        return redir_main_exit(&socket, forked, rreq);
      }

    case REDIR_MSDOWNLOAD:
      snprintf(buffer, bufsize, "HTTP/1.0 403 Forbidden\r\n\r\n");
      redir_write(&socket, buffer, strlen(buffer));
      return redir_main_exit(&socket, forked, rreq);

#if(0)
      {
        char * hdr =
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: application/x-ns-proxy-autoconfig\r\n"
            "Content-Length: %d\r\n"
            "\r\n";

        char * cnt =
            "function FindProxyForURL(url, host) {\r\n"
            "debugPAC =\"PAC Debug Information\\n\";\r\n"
            "debugPAC +=\"-----------------------------------\\n\";\r\n"
            "debugPAC +=\"Machine IP: \" + myIpAddress() + \"\\n\";\r\n"
            "debugPAC +=\"Hostname: \" + host + \"\\n\";\r\n"
            "if (isResolvable(host)) {resolvableHost = \"True\"} else {resolvableHost = \"False\"};\r\n"
            "debugPAC +=\"Host Resolvable: \" + resolvableHost + \"\\n\";\r\n"
            "debugPAC +=\"Hostname IP: \" + dnsResolve(host) + \"\\n\";\r\n"
            "if (isPlainHostName(host)) {plainHost = \"True\"} else {plainHost = \"False\"};\r\n"
            "debugPAC +=\"Plain Hostname: \" + plainHost + \"\\n\";\r\n"
            "debugPAC +=\"Domain Levels: \" + dnsDomainLevels(host) + \"\\n\";\r\n"
            "debugPAC +=\"URL: \" + url + \"\\n\";\r\n"
            "if (url.substring(0,5)==\"http:\") {protocol=\"HTTP\";} else\r\n"
            "if (url.substring(0,6)==\"https:\") {protocol=\"HTTPS\";} else\r\n"
            "if (url.substring(0,4)==\"ftp:\") {protocol=\"FTP\";}\r\n"
            "else {protocol=\"Unknown\";}\r\n"
            "debugPAC +=\"Protocol: \" + protocol + \"\\n\";\r\n"
            "if (!shExpMatch(url,\"*.(js|xml|ico|gif|png|jpg|jpeg|css|swf)*\")) {alert(debugPAC);}\r\n"
            "return \"PROXY 1.2.3.4:8080\";\r\n"
            "}\r\n";

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): WPAD", __FUNCTION__, __LINE__);

        snprintf(buffer, bufsize, hdr, strlen(cnt));
        redir_write(&socket, buffer, strlen(buffer));

        snprintf(buffer, bufsize, cnt);
        redir_write(&socket, buffer, strlen(buffer));
        return redir_main_exit(&socket, forked, rreq);
      }
#endif

  }

  /*
   *  It was not a request for a known path.
   *  It must be an original request
   */
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): redir_accept: Original request host=%s", __FUNCTION__, __LINE__, httpreq.host);

#ifdef ENABLE_REDIRDNSREQ
  if (_options.redirdnsreq && tun) {
    uint8_t answer[PKT_BUFFER];

    /*struct pkt_ethhdr_t *answer_ethh;*/
    struct pkt_iphdr_t  *answer_iph;
    struct pkt_udphdr_t *answer_udph;
    struct dns_packet_t *answer_dns;

    uint8_t query[512];
    size_t query_len = 0;
    size_t udp_len = 0;
    size_t length;

    char *host = httpreq.host;
    char *idx;
    int i;

    do {
      idx = strchr(host, '.');
      size_t len = idx ? idx - host : strlen(host);

      query[query_len++] = (uint8_t) len;

      for (i=0; i < len; i++)
	query[query_len++] = host[i];

      if (idx) host = idx + 1;
    } while(idx);

    query[query_len++] = 0;

    query[query_len++] = 0;
    query[query_len++] = 1;
    query[query_len++] = 0;
    query[query_len++] = 1;

    memset(answer, 0, sizeof(answer));

    /*answer_ethh = ethhdr(answer);*/
    answer_iph = pkt_iphdr(answer);
    answer_udph = pkt_udphdr(answer);
    answer_dns = pkt_dnspkt(answer);

    /* DNS Header */
    answer_dns->id      = 1;
    answer_dns->flags   = htons(0x0100);
    answer_dns->qdcount = htons(0x0001);
    answer_dns->ancount = htons(0x0000);
    answer_dns->nscount = htons(0x0000);
    answer_dns->arcount = htons(0x0000);

    memcpy(answer_dns->records, query, query_len);

    /* UDP header */
    udp_len = query_len + DHCP_DNS_HLEN + PKT_UDP_HLEN;
    answer_udph->len = htons(udp_len);
    answer_udph->src = htons(10000);
    answer_udph->dst = htons(53);

    /* Ip header */
    answer_iph->version_ihl = PKT_IP_VER_HLEN;
    answer_iph->tos = 0;
    answer_iph->tot_len = htons(udp_len + PKT_IP_HLEN);
    answer_iph->id = 0;
    answer_iph->opt_off_high = 0;
    answer_iph->off_low = 0;
    answer_iph->ttl = 0x10;
    answer_iph->protocol = 0x11;
    answer_iph->check = 0; /* Calculate at end of packet */
    memcpy(&answer_iph->daddr, &_options.dns1.s_addr, PKT_IP_ALEN);
    memcpy(&answer_iph->saddr, &conn.hisip.s_addr, PKT_IP_ALEN);

    /* Ethernet header
       memcpy(answer_ethh->dst, &ethh->src, PKT_ETH_ALEN);
       memcpy(answer_ethh->src, &ethh->dst, PKT_ETH_ALEN);
    */

    /* Work out checksums */
    chksum(answer_iph);

    /* Calculate total length */
    length = udp_len + sizeofip(answer);

    tun_encaps(tun, answer, length, 0);
  }
#endif

  /*
   *  XXX: chilli_redir
   */
  if (redir->cb_handle_url) {
    switch (redir->cb_handle_url(redir, &conn, &httpreq,
				 &socket, address, rreq)) {
      case -1:
        return -1;
      case 0:
        return 1;
      default:
        break;
    }
  }

  /* Did the challenge expire, or never existed? */
  if (!conn.s_state.uamtime ||
      (_options.challengetimeout &&
       (conn.s_state.uamtime + _options.challengetimeout) < mainclock_now())) {
    redir_memcopy(REDIR_CHALLENGE);
    redir_msg_send(REDIR_MSG_OPT_REDIR);
  }
  else {
    redir_chartohex(conn.s_state.redir.uamchal, hexchal, REDIR_MD5LEN);
    msg.mtype = splash ? REDIR_ALREADY : REDIR_NOTYET;
    redir_msg_send(REDIR_MSG_OPT_REDIR);
  }

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): ---->>> challenge: %s", __FUNCTION__, __LINE__, hexchal);
#endif

  if (_options.macreauth && !conn.s_state.authenticated) {
    msg.mtype = REDIR_MACREAUTH;
    redir_msg_send(0);
  }

  if (redir->homepage) { /* ||
			    ((conn.s_params.flags & REQUIRE_REDIRECT) && conn.s_params.url[0])) {*/

    char * base_url = /*(conn.s_params.flags & REQUIRE_REDIRECT &&
                        conn.s_params.url[0]) ?
                        (char *)conn.s_params.url : */ redir->homepage;

    bstring url = bfromcstralloc(1024,"");
    bstring urlenc = bfromcstralloc(1024,"");

    char *resp = splash ? "splash" : "notyet";

    redir_buildurl(&conn, url, redir, resp, 0, hexchal, NULL,
		   conn.s_state.redir.userurl, NULL, NULL,
		   conn.hismac, &conn.hisip);
    redir_urlencode(url, urlenc);

    bassignformat(url, "%s%cloginurl=", base_url,
		  strchr(base_url, '?') ? '&' : '?');

    bconcat(url, urlenc);

    redir_reply(redir, &socket, &conn,
		splash ? REDIR_SPLASH : REDIR_NOTYET, url,
		0, hexchal, NULL, conn.s_state.redir.userurl, NULL,
		NULL, conn.hismac, &conn.hisip, httpreq.qs);

    bdestroy(url);
    bdestroy(urlenc);
  }
  else if (state == 1) {
    redir_reply(redir, &socket, &conn,
		splash ? REDIR_SPLASH : REDIR_ALREADY, NULL, 0,
		splash ? hexchal : NULL, NULL, conn.s_state.redir.userurl, NULL,
		(char *)conn.s_params.url, conn.hismac, &conn.hisip, httpreq.qs);
  }
  else {
    redir_reply(redir, &socket, &conn,
		splash ? REDIR_SPLASH : REDIR_NOTYET, NULL,
		0, hexchal, NULL, conn.s_state.redir.userurl, NULL,
		NULL, conn.hismac, &conn.hisip, httpreq.qs);
  }

  return redir_main_exit(&socket, forked, rreq);
}


/* Set callback to determine state information for the connection */
int redir_set_cb_getstate(struct redir_t *redir,
                          int (*cb_getstate) (struct redir_t *redir,
                                              struct sockaddr_in *address,
                                              struct sockaddr_in *baddress,
                                              struct redir_conn_t *conn)) {
  redir->cb_getstate = cb_getstate;
  return 0;
}

