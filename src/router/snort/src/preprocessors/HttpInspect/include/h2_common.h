#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "decode.h"
#include "snort.h"
#ifdef HAVE_LIBNGHTTP2
#include "nghttp2/nghttp2.h"
#endif /* HAVE_LIBNGHTTP2 */

typedef enum {
    // No flag set
    MESSAGE_FLAG_NONE = 0,
    // The rebuilt packet is the start of one PDU
    MESSAGE_FLAG_START_PDU = 0x01,
    // The rebuilt packet is the end of one PDU
    MESSAGE_FLAG_END_PDU = 0x02
} rebuilt_message_flag;

struct output_data {
    char *message;
    uint32_t length;
    H2Hdr hd;
    uint8_t flags;
};

#ifdef HAVE_LIBNGHTTP2

#define MAX_DIR 2

typedef struct return_data_list_node {
    struct return_data_list_node *next;
    struct output_data return_data;
} return_data_list_node;

typedef struct {
    uint8_t *name;
    uint8_t *value;
    size_t namelen;
    size_t valuelen;
} header_nv;

typedef struct nv_list_node{
    struct nv_list_node *next;
    header_nv nv;
} nv_list_node;

typedef struct http2_stream_data {
    // First 9 bytes of the header
    H2Hdr hd;
    // Headers saved here.
    nv_list_node *headers;
    // Data buffer. We might have to buffer if there are multiple data frames.
    uint8_t *data;
    uint32_t databuf_off;
    uint32_t data_to_flush;
    struct http2_stream_data *next;
    struct http2_stream_data *prev;
} http2_stream_data;

typedef struct http2_session_data {
    // 0 is for client->server
    // 1 is for server->client
    nghttp2_session *session[MAX_DIR];
    http2_stream_data *root[MAX_DIR];
    // The root of the return message list
    return_data_list_node *first_return_data;
    // Number of return messages in the current packet
    int num_of_return_data;
} http2_session_data;

#define _U_ __attribute__((unused))

void free_http2_session_data(void *userdata);
void initialize_nghttp2_session_snort(nghttp2_session **session, http2_session_data *user_data,
                                            int type, bool upg);
ssize_t process_payload_http2(http2_session_data *session_data,
                const uint8_t *in, size_t inlen, bool to_server);

struct nv_list_node *create_nv(int namelen, int valuelen);
int free_headers(struct nv_list_node **headers);
uint32_t no_of_nodes(nv_list_node *headers);
void print_headers(struct nv_list_node *headers);
void print_data(uint8_t *data, uint32_t length);
void convert_title_case(uint8_t *name, uint32_t namelen);
void copy_hd(H2Hdr *dst, H2Hdr src);
int copy_headers(struct nv_list_node *headers, struct output_data *out);

http2_stream_data* http2_find_stream(http2_session_data *session_data,
                                    uint32_t stream_id);
void http2_add_stream(http2_session_data *session_data,
                            http2_stream_data *stream_data);
http2_stream_data *http2_create_stream(nghttp2_frame_hd hd, nghttp2_priority_spec pri_spec);
void http2_remove_stream(http2_session_data *session_data,
                        http2_stream_data *stream_data);
int http2_add_header(http2_stream_data *stream_data, const uint8_t *name,
                        size_t namelen, const uint8_t *value, size_t valuelen);
void http2_free_stream(http2_stream_data **stream_data);
void http2_free_return_data_list(http2_session_data *session_data);
#if 0
void initialize_nghttp2_client_session_snort(nghttp2_session **session, http2_session_data *user_data);
#endif
#endif /* HAVE_LIBNGHTTP2 */
#endif

