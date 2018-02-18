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

#ifndef _OLSRD_LIB_INFO_JSON_HELPERS_H_
#define _OLSRD_LIB_INFO_JSON_HELPERS_H_

#include <stdio.h>
#include <stdbool.h>

#include "common/autobuf.h"

#define INFO_JSON_ENTRY_MAX_DEPTH 16

/* JSON does not allow commas dangling at the end of arrays, so we need to
 * count which entry number we're at in order to make sure we don't tack a
 * dangling comma on at the end
 */
struct json_session {
    bool pretty;
    int entrynumber[INFO_JSON_ENTRY_MAX_DEPTH];
    int currentjsondepth;
};

void abuf_json_reset_entry_number_and_depth(struct json_session *session, bool pretty);

void abuf_json_insert_comma(struct json_session *session, struct autobuf *abuf);

void abuf_json_mark_output(struct json_session *session, bool open, struct autobuf *abuf);

void abuf_json_mark_object(struct json_session *session, bool open, bool array, struct autobuf *abuf, const char* header);

void abuf_json_mark_array_entry(struct json_session *session, bool open, struct autobuf *abuf);

void abuf_json_boolean(struct json_session *session, struct autobuf *abuf, const char* key, bool value);

void abuf_json_string(struct json_session *session, struct autobuf *abuf, const char* key, const char* value);

void abuf_json_int(struct json_session *session, struct autobuf *abuf, const char* key, long long value);

void abuf_json_float(struct json_session *session, struct autobuf *abuf, const char* key, double value);

void abuf_json_ip_address(struct json_session *session, struct autobuf *abuf, const char* key, union olsr_ip_addr *ip);

void abuf_json_ip_address46(struct json_session *session, struct autobuf *abuf, const char* key, void *ip, int af);

void abuf_json_prefix(struct json_session *session, struct autobuf *abuf, const char* key, struct olsr_ip_prefix *prefix);

#endif /* _OLSRD_LIB_INFO_JSON_HELPERS_H_ */
