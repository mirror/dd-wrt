/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "json_helpers.h"
#include "olsr.h"
#include "ipcalc.h"

#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

/* JSON support functions */

void abuf_json_reset_entry_number_and_depth(struct json_session *session, bool pretty) {
  assert(session);

  memset(session, 0, sizeof(*session));
  session->pretty = pretty;
}

static void abuf_json_new_indent(struct json_session *session, struct autobuf *abuf) {
  assert(session);
  assert(abuf);

  if (session->currentjsondepth) {
    int i = session->currentjsondepth;

    if (session->pretty) {
      abuf_puts(abuf, "\n");
      while (i-- > 0) {
        abuf_puts(abuf, "  ");
      }
    }
  }
}

void abuf_json_insert_comma(struct json_session *session, struct autobuf *abuf) {
  assert(session);
  assert(abuf);

  if (session->entrynumber[session->currentjsondepth])
    abuf_appendf(abuf, ",");
}

void abuf_json_mark_output(struct json_session *session, bool open, struct autobuf *abuf) {
  assert(session);
  assert(abuf);

  if (open) {
    assert(!session->currentjsondepth);
    abuf_json_new_indent(session, abuf);
    abuf_puts(abuf, "{");
    session->currentjsondepth++;
    session->entrynumber[session->currentjsondepth] = 0;
  } else {
    assert(session->currentjsondepth == 1);
    session->entrynumber[session->currentjsondepth] = 0;
    session->currentjsondepth--;
    abuf_json_new_indent(session, abuf);
    if (session->pretty) {
      abuf_puts(abuf, "\n");
    }
    abuf_puts(abuf, "}");
  }
}

void abuf_json_mark_object(struct json_session *session, bool open, bool array, struct autobuf *abuf, const char* header) {
  assert(session);
  assert(abuf);

  if (open) {
    abuf_json_insert_comma(session, abuf);
    abuf_json_new_indent(session, abuf);
    if (header) {
      abuf_appendf(abuf, "\"%s\": %s", header, array ? "[" : "{");
    } else {
      abuf_appendf(abuf, "%s", array ? "[" : "{");
    }
    session->entrynumber[session->currentjsondepth]++;
    session->currentjsondepth++;
    assert(session->currentjsondepth < INFO_JSON_ENTRY_MAX_DEPTH);
    session->entrynumber[session->currentjsondepth] = 0;
  } else {
    session->entrynumber[session->currentjsondepth] = 0;
    session->currentjsondepth--;
    assert(session->currentjsondepth >= 0);
    abuf_json_new_indent(session, abuf);
    abuf_appendf(abuf, "%s", array ? "]" : "}");
  }
}

void abuf_json_mark_array_entry(struct json_session *session, bool open, struct autobuf *abuf) {
  assert(session);
  assert(abuf);

  abuf_json_mark_object(session, open, false, abuf, NULL);
}

void abuf_json_boolean(struct json_session *session, struct autobuf *abuf, const char* key, bool value) {
  assert(session);
  assert(abuf);

  abuf_json_insert_comma(session, abuf);
  abuf_json_new_indent(session, abuf);
  if (key) {
    abuf_appendf(abuf, "\"%s\": ", key);
  }
  abuf_appendf(abuf, "%s", value ? "true" : "false");
  session->entrynumber[session->currentjsondepth]++;
}

void abuf_json_string(struct json_session *session, struct autobuf *abuf, const char* key, const char* value) {
  assert(session);
  assert(abuf);

  abuf_json_insert_comma(session, abuf);
  abuf_json_new_indent(session, abuf);
  if (key) {
    abuf_appendf(abuf, "\"%s\": ", key);
  }
  abuf_appendf(abuf, "\"%s\"", !value ? "" : value);
  session->entrynumber[session->currentjsondepth]++;
}

void abuf_json_int(struct json_session *session, struct autobuf *abuf, const char* key, long long value) {
  const char * fmt;
  assert(session);
  assert(abuf);

#ifndef _WIN32
  fmt = "%lld";
#else
  fmt = "%ld";
#endif

  abuf_json_insert_comma(session, abuf);
  abuf_json_new_indent(session, abuf);
  if (key) {
    abuf_appendf(abuf, "\"%s\": ", key);
  }
  abuf_appendf(abuf, fmt, value);
  session->entrynumber[session->currentjsondepth]++;
}

void abuf_json_float(struct json_session *session, struct autobuf *abuf, const char* key, double value) {
  double v = value;
  int isInf = isinf(v);

  assert(session);
  assert(abuf);

  if (isnan(v)) {
    v = 0.0;
  } else if (isInf < 0) {
    v = -DBL_MAX;
  } else if (isInf > 0) {
    v = DBL_MAX;
  }

  abuf_json_insert_comma(session, abuf);
  abuf_json_new_indent(session, abuf);
  if (key) {
    abuf_appendf(abuf, "\"%s\": ", key);
  }
  abuf_appendf(abuf, "%f", v);
  session->entrynumber[session->currentjsondepth]++;
}

void abuf_json_ip_address(struct json_session *session, struct autobuf *abuf, const char* key, union olsr_ip_addr *ip) {
  struct ipaddr_str ipStr;

  assert(session);
  assert(abuf);

  abuf_json_insert_comma(session, abuf);
  abuf_json_new_indent(session, abuf);
  if (key) {
    abuf_appendf(abuf, "\"%s\": ", key);
  }
  abuf_appendf(abuf, "\"%s\"", !ip ? "" : olsr_ip_to_string(&ipStr, ip));
  session->entrynumber[session->currentjsondepth]++;
}

void abuf_json_ip_address46(struct json_session *session, struct autobuf *abuf, const char* key, void *ip, int af) {
  struct ipaddr_str ipStr;
  const char * value;

  assert(session);
  assert(abuf);

  if (!ip) {
    value = "";
  } else if (af == AF_INET) {
    value = ip4_to_string(&ipStr, *((const struct in_addr*) ip));
  } else {
    value = ip6_to_string(&ipStr, (const struct in6_addr * const ) ip);
  }

  abuf_json_insert_comma(session, abuf);
  abuf_json_new_indent(session, abuf);
  if (key) {
    abuf_appendf(abuf, "\"%s\": ", key);
  }
  abuf_appendf(abuf, "\"%s\"", value);
  session->entrynumber[session->currentjsondepth]++;
}

void abuf_json_prefix(struct json_session *session, struct autobuf *abuf, const char* key, struct olsr_ip_prefix *prefix) {
  struct ipaddr_str ipStr;
  const char * value;
  int prefixLen;

  assert(session);
  assert(abuf);

  abuf_json_insert_comma(session, abuf);
  abuf_json_new_indent(session, abuf);
  if (key) {
    abuf_appendf(abuf, "\"%s\": ", key);
  }
  if (!prefix) {
    abuf_puts(abuf, "\"\"");
  } else {
    value = olsr_ip_to_string(&ipStr, &prefix->prefix);
    prefixLen = prefix->prefix_len;
    abuf_appendf(abuf, "\"%s/%d\"", value, prefixLen);
  }

  session->entrynumber[session->currentjsondepth]++;
}
