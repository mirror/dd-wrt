#include "h2_common.h"
#include "memory_stats.h"

enum type_of_message
{
    REQUEST,
    RESPONSE,
    NONE
};

#ifdef HAVE_LIBNGHTTP2
static bool dir;
#define MAKE_NV2(NAME, VALUE)                                                  \
  {                                                                            \
    (uint8_t *) NAME, (uint8_t *)VALUE, sizeof(NAME) - 1, sizeof(VALUE) - 1,   \
        NGHTTP2_NV_FLAG_NONE                                                   \
  }

#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))

nv_list_node *create_nv(int namelen, int valuelen)
{
    if (namelen < 0 || valuelen < 0)
        return NULL;

    nv_list_node *temp = (nv_list_node *)SnortPreprocAlloc(1, sizeof(nv_list_node),
                                              PP_HTTPINSPECT, 
                                              PP_MEM_CATEGORY_SESSION);
    if (NULL == temp)
        return NULL;

    temp->nv.name = (uint8_t *)SnortPreprocAlloc(1, namelen, 
                                    PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
    if (NULL == temp->nv.name)
    {
        SnortPreprocFree(temp, sizeof(nv_list_node), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
        return NULL;
    }
    temp->nv.value = (uint8_t  *)SnortPreprocAlloc(1, valuelen, PP_HTTPINSPECT, 
                                      PP_MEM_CATEGORY_SESSION);
    if (NULL == temp->nv.value)
    {
        SnortPreprocFree(temp->nv.name, sizeof(*(temp->nv.name)), 
             PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
        SnortPreprocFree(temp, sizeof(nv_list_node), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
        return NULL;
    }
    temp->nv.namelen = namelen;
    temp->nv.valuelen = valuelen;
    temp->next = NULL;
    return temp;
}

int free_headers(nv_list_node **headers)
{
    if (NULL == headers)
        return 0;

    nv_list_node *temp = *headers;

    while (NULL != temp)
    {
        nv_list_node *prev = temp;
        temp = temp->next;
        SnortPreprocFree(prev->nv.name, sizeof(*(prev->nv.name)), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
        SnortPreprocFree(prev->nv.value, sizeof(*(prev->nv.value)), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
        SnortPreprocFree(prev, sizeof(nv_list_node), PP_HTTPINSPECT,
             PP_MEM_CATEGORY_SESSION);
    }
    *headers = NULL;
    return 0;
}

uint32_t no_of_nodes(nv_list_node *headers)
{
    uint32_t n = 0;
    nv_list_node *temp = headers;

    while (NULL != temp)
    {
        n++;
        temp = temp->next;
    }
    return n;
}

void print_headers(nv_list_node *headers)
{
    while (NULL != headers)
    {
        if (NULL != headers->nv.name)
            fwrite(headers->nv.name, headers->nv.namelen, 1, stdout);
        fprintf(stdout, ": ");
        if (NULL != headers->nv.value)
            fwrite(headers->nv.value, headers->nv.valuelen, 1, stdout);
        headers = headers->next;
    }
}

void print_data(uint8_t *data, uint32_t length)
{
    uint32_t i;

    if (NULL != data)
    {
        printf("\n");
        for (i=0; i<length ; i++)
        {
            printf("%c",data[i]);
        }
    }
}

void convert_title_case(uint8_t *name, uint32_t namelen)
{
    uint32_t i;

    if (NULL != name)
    {
        for (i=0; i<namelen; i++)
        {
            if (i==0 || (name[i-1] =='-'))
                name[i]= toupper(name[i]);
        }
    }
}

void copy_hd(H2Hdr *dst, H2Hdr src)
{
    if (NULL != dst)
    {
        dst->stream_id = src.stream_id;
        dst->length = src.length;
        dst->type = src.type;
        dst->flags = src.flags;
        dst->reserved = src.reserved;
        dst->pri.stream_id = src.pri.stream_id;
        dst->pri.weight = src.pri.weight;
    }

}

int copy_headers(nv_list_node *headers, struct output_data *out)
{
    char *path = NULL;
    char *host = NULL;
    char *method = NULL;
    char *version ="HTTP/1.1";
    char *status = NULL;
    int type = NONE;
    uint32_t scheme_status_method_len = 7;
    uint32_t path_len = 5;
    uint32_t authority_len = 10;
    uint32_t length = 0;
    int ret = 0;
    uint32_t plen = 0, hlen = 0, mlen = 0, slen = 0;

    nv_list_node *nvtemp = headers;
    if ((NULL == headers) || (NULL == out))
    {
        return 0;
    }

    while (NULL != nvtemp)
    {
        if (nvtemp->nv.name[0] == ':')
        {
            if (nvtemp->nv.namelen == scheme_status_method_len)
            {
                if (!status &&
                    (memcmp(nvtemp->nv.name,":status", scheme_status_method_len) == 0))
                {
                    type = RESPONSE;
                    status = (char *) SnortPreprocAlloc(1, (nvtemp->nv.valuelen +1) * sizeof(char), 
                                           PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
                    slen = nvtemp->nv.valuelen + 1;
                    if (NULL == status)
                    {
                        ret = -1;
                        goto cleanup;
                    }
                    memcpy(status,nvtemp->nv.value,nvtemp->nv.valuelen);
                    status[nvtemp->nv.valuelen] = '\0';
                    length += nvtemp->nv.valuelen;
                }
                else if (!method && (
                    memcmp(nvtemp->nv.name,":method", scheme_status_method_len) == 0))
                {
                    type = REQUEST;
                    method = (char *)SnortPreprocAlloc(1, (nvtemp->nv.valuelen +1) * sizeof(char),
                                          PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
                    mlen = nvtemp->nv.valuelen + 1;
                    if (NULL == method)
                    {
                        ret = -1;
                        goto cleanup;
                    }
                    memcpy(method,nvtemp->nv.value,nvtemp->nv.valuelen);
                    method[nvtemp->nv.valuelen] = '\0';
                    length += nvtemp->nv.valuelen;
                }

            }
            else if (nvtemp->nv.namelen == path_len)
            {
                if(!path && (memcmp(nvtemp->nv.name,":path",path_len)==0))
                {
                    path = (char *)SnortPreprocAlloc(1, (nvtemp->nv.valuelen +1) * sizeof(char),
                                        PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
                    plen = nvtemp->nv.valuelen + 1;
                    if (NULL == path)
                    {
                        ret = -1;
                        goto cleanup;
                    }
                    memcpy(path,nvtemp->nv.value,nvtemp->nv.valuelen);
                    path[nvtemp->nv.valuelen] = '\0';
                    length += nvtemp->nv.valuelen;
                }
            }
            else if (nvtemp->nv.namelen == authority_len)
            {
                // Http 2.0 has host parameter in authority
                if (!host && (memcmp(nvtemp->nv.name,":authority",authority_len)==0))
                {
                    host = (char *)SnortPreprocAlloc(1, (nvtemp->nv.valuelen +1) * sizeof(char),
                                        PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
                    hlen = nvtemp->nv.valuelen + 1;
                    if (NULL == host)
                    {
                        ret = -1;
                        goto cleanup;
                    }
                    memcpy(host,nvtemp->nv.value,nvtemp->nv.valuelen);
                    host[nvtemp->nv.valuelen] = '\0';
                    length += nvtemp->nv.valuelen + 4 + 4;
                }
            }
        }
        else
        {
            length += (nvtemp->nv.namelen + nvtemp->nv.valuelen + 4);
        }
        nvtemp = nvtemp->next;
    }

    length += (8+2+1); // For Version +Extra "\r\n" at the end
    //Print the psuedo headers
    if (type == REQUEST)
    {
        length += 4;

        if (NULL != method && NULL != path && NULL != host)
        {
            out->message = (char *)SnortPreprocAlloc(1, (length) * sizeof(char),
                                        PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
            if(NULL != out->message)
            {
                snprintf(out->message, length, "%s %s %s\r\n", method, path, version);
                out->length += strlen(method) + strlen(path) + strlen(version) + 4;

                if (out->length < length)
                {
                    snprintf(out->message +out->length, length - out->length, "Host: %s\r\n", host);
                    out->length += strlen(host) + 4 + 4;
                }
            }
        }
        else if (NULL != method && NULL != host) {

            if(strcmp(method, "CONNECT") == 0)
            {
                length += strlen(host);
                out->message = (char *)SnortPreprocAlloc(1, (length) * sizeof(char),
                                            PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
                if(NULL != out->message)
                {
                    snprintf(out->message, length, "%s %s %s\r\n", method, host, version);
                    out->length += strlen(method) + strlen(host) + strlen(version) + 4;
                    if (out->length < length)
                    {
                        snprintf(out->message +out->length, length - out->length, "Host: %s\r\n", host);
                        out->length += strlen(host) + 4 + 4;
                    }
                }
            }
            else
            {
                ret = -2;
                goto cleanup;
            }
        }
        else
        {
            ret = -2;
            goto cleanup;
        }

        //length = length +4 +4 ; //one 4 is for host and one is for <CRLF>
    }
    else if (type == RESPONSE) { //RESPONSE
        //length = length + strlen(version) + +strlen(status)+3;
        length+= 3;

        out->message = (char *)SnortPreprocAlloc(1, (length) * sizeof(char),
                                    PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
        if (NULL != out->message)
        {
            snprintf(out->message, length, "%s %s\r\n",version,status);
            out->length = strlen(version)+strlen(status) + 3;
        }
    }
    else
    {
        // Only headers, not a valid request/response
        ret = -2;
        goto cleanup;
    }

    //Iterate from the beginning again and copy the other headers
    if (NULL != out->message)
    {
        nvtemp = headers;
        while (NULL != nvtemp)
        {
            if (nvtemp->nv.name[0] != ':')
            {
                if ((out->length + nvtemp->nv.namelen + nvtemp->nv.valuelen + 4) <= length)
                {
                    memcpy(out->message+out->length,nvtemp->nv.name,
                        nvtemp->nv.namelen);
                    out->length += nvtemp->nv.namelen;
                    snprintf(out->message+out->length, length - out->length, ": ");
                    out->length += 2;
                    memcpy(out->message+out->length,nvtemp->nv.value,
                        nvtemp->nv.valuelen);
                    out->length +=nvtemp->nv.valuelen;
                    snprintf(out->message+out->length, length - out->length, "\r\n");
                    out->length += 2;
                }
            }
            nvtemp = nvtemp->next;
        }
        //Include \r\n at the end
        if (out->length < length)
        {
            snprintf(out->message+out->length, length - out->length, "\r\n");
        }

        out->length += 2;
    }
cleanup:
    if (NULL != path)
        SnortPreprocFree(path, plen * sizeof(char), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
    if (NULL != host)
        SnortPreprocFree(host , hlen * sizeof(char), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
    if (NULL != method)
        SnortPreprocFree(method , mlen * sizeof(char), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
    if (NULL != status)
        SnortPreprocFree(status , slen * sizeof(char), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);

    return ret;
}

http2_stream_data* http2_find_stream(http2_session_data *session_data,
        uint32_t stream_id)
{
    if (NULL != session_data)
    {
        http2_stream_data *temp = session_data->root[dir];
        while (NULL != temp)
        {
            if ((uint32_t)temp->hd.stream_id == stream_id)
                return temp;
            temp=temp->next;
        }
    }
    return NULL;
}

// This doesn't look if there is a duplicate stream with the same streamid
// http2_find_stream should be called before adding the stream.
void http2_add_stream(http2_session_data *session_data,
     http2_stream_data *stream_data)
{
    http2_stream_data *temp;

    if (NULL != session_data && NULL != stream_data)
    {
        if (NULL == session_data->root[dir])
        {
            session_data->root[dir] = stream_data;
        }
        else
        {
            temp = session_data->root[dir];
            while (NULL != temp->next)
                temp = temp->next;
            temp->next = stream_data;
            stream_data->prev= temp;
        }
    }
}

http2_stream_data *http2_create_stream(nghttp2_frame_hd hd,
    nghttp2_priority_spec pri_spec)
{
    http2_stream_data *temp = NULL;

    temp = (http2_stream_data *)SnortPreprocAlloc(1, sizeof(http2_stream_data),
                                     PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
    if (NULL != temp)
    {
        temp->hd.stream_id = hd.stream_id;
        temp->hd.length = hd.length;
        temp->hd.type = hd.type;
        temp->hd.flags = hd.flags;
        temp->hd.reserved = hd.reserved;
        temp->hd.pri.stream_id = pri_spec.stream_id;
        temp->hd.pri.weight = pri_spec.weight;
        temp->next = NULL;
        temp->prev = NULL;
    }
    return temp;
}

//http2_remove_stream should be followed by http2_free_stream
void http2_remove_stream(http2_session_data *session_data,
        http2_stream_data *stream_data)
{
    if ((NULL != session_data) && (NULL != stream_data))
    {
        if (session_data->root[dir] == stream_data)
        {
            if (session_data->root[dir]->next)
            {
                session_data->root[dir] = session_data->root[dir]->next;
                session_data->root[dir]->prev = NULL;
            }
            else
            {
                session_data->root[dir] = NULL;
            }

        } else {
            if (stream_data->prev)
            {
                stream_data->prev->next = stream_data->next;
            }
            if (stream_data->next)
            {
                stream_data->next->prev = stream_data->prev;
            }
        }
    }
}

int http2_add_header(http2_stream_data *stream_data, const uint8_t *name,
        size_t namelen, const uint8_t *value, size_t valuelen)
{
    if (NULL != stream_data && NULL != name && NULL != value)
    {
        nv_list_node *newnode = create_nv(namelen, valuelen);
        nv_list_node *temp = NULL;

        if (NULL == newnode)
            return -1;

        memcpy(newnode->nv.name, name, namelen);
        memcpy(newnode->nv.value, value, valuelen);

        convert_title_case(newnode->nv.name, namelen);

        temp = stream_data->headers;

        if (NULL == temp)
            stream_data->headers = newnode;
        else
        {
            // TODO: Try to optimize here if order of headers is not important.
            while (NULL != temp->next)
                temp = temp->next;
            temp->next = newnode;
        }
    }
    return 0;
}

void http2_free_stream(http2_stream_data **stream_data)
{
    if (NULL != stream_data)
    {
        http2_stream_data *temp = *stream_data;

        if (NULL != temp)
        {
            free_headers(&(temp->headers));
            if (NULL != temp->data)
                free(temp->data);  // unable to find the size 
            SnortPreprocFree(*stream_data, sizeof(http2_stream_data),
                 PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
            *stream_data = NULL;
        }
    }
}

return_data_list_node *http2_add_return_data(http2_session_data *session_data)
{
    return_data_list_node *temp = NULL;
    return_data_list_node *current;

    if (NULL == session_data)
        return temp;

    temp = (return_data_list_node *)SnortPreprocAlloc(1, sizeof(return_data_list_node),
                                         PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);

    if (NULL != temp)
    {
        temp->return_data.message = NULL;
        temp->return_data.length = 0;
        temp->return_data.flags = MESSAGE_FLAG_NONE;
        temp->next = NULL;

        if (NULL == session_data->first_return_data)
        {
            session_data->first_return_data = temp;
        }
        else
        {
            current = session_data->first_return_data;
            while (NULL != current->next)
                current = current->next;
            current->next = temp;
        }
    }

    return temp;
}

void http2_free_return_data_list(http2_session_data *session_data)
{
    if (NULL != session_data)
    {
        return_data_list_node *current = session_data->first_return_data;
        return_data_list_node *temp;

        while (NULL != current)
        {
            temp = current;
            current = current->next;
            free(temp->return_data.message);
            free(temp);
        }
        session_data->first_return_data = NULL;
    }
}

/* Stream related functions - End */


static int on_begin_frame_callback(nghttp2_session *session _U_,
        const nghttp2_frame_hd *hd,
        void *user_data) {

    http2_stream_data *temp;
    http2_session_data *session_data = (http2_session_data*) user_data;
    nghttp2_priority_spec p_spec={0,16,0};

    if (hd->type != NGHTTP2_DATA){
        return 0;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: StreamId:%d\n",__FUNCTION__,
                            hd->stream_id);)

    temp = http2_find_stream(session_data,hd->stream_id);
    if (NULL != temp)
    {
        //Old one might be stale. Remove and create a new one
        http2_remove_stream(session_data, temp);
        http2_free_stream(&temp);
    }

    //Create a stream only when it doesnt exist.
    temp = http2_create_stream(*hd, p_spec);
    if (NULL != temp)
    {
        temp->data_to_flush = hd->length;
        http2_add_stream(session_data, temp);
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
            "%s: Unable to create a stream\n",__FUNCTION__);)
    }
    return 0;
}

static int on_begin_headers_callback(nghttp2_session *session _U_,
    const nghttp2_frame *frame,
    void *user_data) {
    http2_stream_data *temp;
    http2_session_data *session_data = (http2_session_data*) user_data;

    if (frame->hd.type != NGHTTP2_HEADERS){
        return 0;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: StreamId:%d\n",__FUNCTION__,
                frame->hd.stream_id);)
    temp = http2_find_stream(session_data,frame->hd.stream_id);
    if (NULL != temp)
    {
        //Old one might be stale. Remove and create a new stream
        http2_remove_stream(session_data, temp);
        http2_free_stream(&temp);
    }

    temp = http2_create_stream(frame->hd, frame->headers.pri_spec);
    if (NULL != temp)
        http2_add_stream(session_data, temp);
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
           "%s: Unable to create a stream\n",__FUNCTION__);)
    }

    return 0;
}

/* nghttp2_on_header_callback: Called when nghttp2 library emits
   single header name/value pair. */
static int on_header_callback(nghttp2_session *session _U_,
    const nghttp2_frame *frame, const uint8_t *name,
    size_t namelen, const uint8_t *value,
    size_t valuelen, uint8_t flags _U_,
    void *user_data ) {

    http2_stream_data *temp= NULL;
    http2_session_data *session_data = (http2_session_data*) user_data;

    switch (frame->hd.type) {
        case NGHTTP2_HEADERS:
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: StreamId:%d\n",__FUNCTION__,
                        frame->hd.stream_id);)
            temp = http2_find_stream(session_data,frame->hd.stream_id);
            if (NULL != temp)
            {
                if (http2_add_header(temp, name, namelen, value, valuelen) < 0)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                        "%s: Failed to add the header\n",__FUNCTION__);)
                }
            }
            //else will never happen as on_begin_headers_callback is
            //called before this callback
            break;
    }
    return 0;
}

static int on_data_chunk_recv_callback(nghttp2_session *session,
        uint8_t flags _U_, int32_t stream_id,
        const uint8_t *data, size_t len,
        void *user_data) {

    http2_session_data *session_data = (http2_session_data*) user_data;
    http2_stream_data *temp = http2_find_stream(session_data,
            stream_id);
    return_data_list_node *current;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: StreamId:%d\n",__FUNCTION__,
                stream_id);)
    if (NULL != temp)
    {
        if (NULL == temp->data)
        {
            // Accumulate data till it reaches PAF_MAX
            temp->data = (uint8_t*)SnortPreprocAlloc(1, 
                                        (ScPafMax() + ETHERNET_MTU)*sizeof(char),
                                        PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
            if (NULL == temp->data)
                return 0;

            temp->databuf_off = 0;
        }

        memcpy(temp->data+ temp->databuf_off, data, len);
        temp->databuf_off +=len;

        if (temp->databuf_off > ScPafMax())
        {
            if (temp->data_to_flush - temp->databuf_off > 0)
            {
                current = http2_add_return_data(session_data);
                if (NULL == current)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: Failed to add new return message #%d\n",
                               __FUNCTION__, session_data->num_of_return_data +1);)
                    return 0;
                }
                current->return_data.message = (char *)SnortPreprocAlloc(1, 
                                                            temp->databuf_off * sizeof(char),
                                                            PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
                if (NULL != current->return_data.message)
                {
                    memcpy(current->return_data.message,temp->data, temp->databuf_off);
                    current->return_data.length = temp->databuf_off;
                    copy_hd(&(current->return_data.hd), temp->hd);
                    memset(temp->data, 0, ScPafMax() + ETHERNET_MTU);
                    temp->databuf_off = 0;
                    temp->data_to_flush -= temp->databuf_off;
                    session_data->num_of_return_data++;
                }
            }
        }
    }
    return 0;
}

static int on_frame_recv_callback(nghttp2_session *session _U_,
        const nghttp2_frame *frame,
        void *user_data) {

    http2_stream_data *temp;
    http2_session_data *session_data = (http2_session_data*) user_data;
    return_data_list_node *current;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: StreamId:%d\n",__FUNCTION__,
                frame->hd.stream_id);)
    switch (frame->hd.type)
    {

        case NGHTTP2_DATA:
            temp = http2_find_stream(session_data, frame->hd.stream_id);

            if (NULL != temp)
            {
                if (temp->databuf_off > 0)
                {
                    current = http2_add_return_data(session_data);
                    if (NULL == current)
                    {
                        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: Failed to add new return message #%d\n",
                                   __FUNCTION__, session_data->num_of_return_data + 1);)
                        http2_remove_stream(session_data, temp);
                        http2_free_stream(&temp);
                        return 0;
                    }
                    current->return_data.message = (char *)SnortPreprocAlloc(1, 
                                                                temp->databuf_off*sizeof(char),
                                                                PP_HTTPINSPECT, 
                                                                PP_MEM_CATEGORY_SESSION);
                    if (NULL == current->return_data.message)
                    {
                        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                            "%s: Failed to allocate return message #%d\n", __FUNCTION__,
                            session_data->num_of_return_data + 1);)
                        http2_remove_stream(session_data, temp);
                        http2_free_stream(&temp);
                        return 0;
                    }
                    memcpy(current->return_data.message, temp->data,
                            temp->databuf_off);
                    current->return_data.length = temp->databuf_off;
                    copy_hd(&(current->return_data.hd), temp->hd);
                    if (temp->hd.flags & NGHTTP2_FLAG_END_STREAM)
                    {
                        current->return_data.flags |= MESSAGE_FLAG_END_PDU;
                    }
                    session_data->num_of_return_data++;
                }
                http2_remove_stream(session_data, temp);
                http2_free_stream(&temp);
            }
            break;

        case NGHTTP2_HEADERS:
            temp = http2_find_stream(session_data, frame->hd.stream_id);

            if (NULL != temp)
            {
                //DEBUG_WRAP(print_headers(temp->headers);)
                current = http2_add_return_data(session_data);
                if (NULL == current)
                {
                    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: Failed to add new return message #%d\n",
                               __FUNCTION__, session_data->num_of_return_data + 1);)
                }
                else
                {
                    copy_headers(temp->headers,&(current->return_data));
                    if (NULL != current->return_data.message)
                    {
                        copy_hd(&(current->return_data.hd), temp->hd);
                        current->return_data.flags |= MESSAGE_FLAG_START_PDU;
                        if (temp->hd.flags & NGHTTP2_FLAG_END_STREAM)
                        {
                            current->return_data.flags |= MESSAGE_FLAG_END_PDU;
                        }
                        session_data->num_of_return_data++;
                    }
                }
                http2_remove_stream(session_data,temp);
                http2_free_stream(&temp);

                if (!dir)
                {
                    if (session_data->session[!dir])
                    {
                        const uint8_t *datap;
                        int rv;
                        nghttp2_nv hdrs[] = {
                            MAKE_NV2(":method", "GET"),
                            MAKE_NV2(":scheme", "https"),
                            MAKE_NV2(":path","/"),
                            MAKE_NV2(":authority","www.dummy.com")};
                        //from client
                        nghttp2_session_set_next_stream_id(session_data->session[!dir], frame->hd.stream_id);
                        nghttp2_submit_request(session_data->session[!dir],
                                NULL, hdrs, ARRLEN(hdrs), NULL, session_data->session[!dir]);
                        do
                        {
                            rv= nghttp2_session_mem_send (session_data->session[!dir], &datap);
                        }while(rv>0);
                    }
                }
            }
            break;
        case NGHTTP2_SETTINGS:
            {
                const uint8_t *datap;
                int rv;

                if (session_data->session[!dir])
                {
                    nghttp2_submit_settings (session_data->session[!dir],
                            frame->hd.flags,
                            frame->settings.iv,
                            frame->settings.niv);
                    do
                    {
                        rv = nghttp2_session_mem_send (session_data->session[!dir], &datap);
                    } while (rv > 0);
                }
                break;
            }
        case NGHTTP2_WINDOW_UPDATE:
            {
                const uint8_t *datap;
                int rv;

                if (session_data->session[!dir])
                {
                    nghttp2_submit_window_update (session_data->session[!dir],
                            frame->hd.flags,
                            frame->hd.stream_id,
                            frame->window_update.window_size_increment);
                    do
                    {
                        rv = nghttp2_session_mem_send (session_data->session[!dir], &datap);
                    } while (rv > 0);
                }
                break;
            }
        default:
            break;
    }
    return 0;
}


void initialize_nghttp2_session_snort(nghttp2_session **session, http2_session_data *user_data,
    int type, bool upg) {
    nghttp2_option *option;
    nghttp2_session_callbacks *callbacks;

    nghttp2_option_new(&option);
    nghttp2_session_callbacks_new(&callbacks);
    nghttp2_session_callbacks_set_on_begin_frame_callback(callbacks,
            on_begin_frame_callback);
    nghttp2_session_callbacks_set_on_begin_headers_callback(
            callbacks, on_begin_headers_callback);
    nghttp2_session_callbacks_set_on_header_callback(callbacks,
            on_header_callback);
    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(
            callbacks, on_data_chunk_recv_callback);
    nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks,
            on_frame_recv_callback);

    if (0 == type) {
        //Decoding of Http/2 messages received from client
        nghttp2_option_set_no_http_messaging(option, 1);
        nghttp2_session_server_new2(session, callbacks, user_data, option);
    } else {
        //Decoding of Http/2 messages received from server
        nghttp2_option_set_no_http_messaging(option, 1);
        nghttp2_session_client_new2(session, callbacks, user_data, option);
    }

    if (upg)
    {
        nghttp2_settings_entry iv[16];
        uint8_t settings_payload[128];
        size_t settings_payloadlen;
        iv[0].settings_id = NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS;
        iv[0].value = 100;
        iv[1].settings_id = NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE;
        iv[1].value = 65535;
        settings_payloadlen = (size_t)nghttp2_pack_settings_payload(
                             settings_payload, sizeof(settings_payload), iv, 2);
        nghttp2_session_upgrade2(*session, settings_payload,
                                          settings_payloadlen, 0, &callbacks);
    }

    nghttp2_session_callbacks_del(callbacks);
    nghttp2_option_del(option);
}


ssize_t process_payload_http2 (http2_session_data *session_data,
        const uint8_t *in, size_t inlen, bool to_server)
{
    int readlen=0;

    dir = !to_server; // 0 is for client->server

    if (NULL != session_data->session[dir])
    {
        readlen = nghttp2_session_mem_recv(session_data->session[dir], in, inlen);
    }

    return readlen;
}

void free_http2_session_data(void *userdata)
{
    http2_session_data* sd = (http2_session_data*) userdata;
    int i;

    if (!sd)
        return;

    for ( i=0; i<MAX_DIR; i++)
    {
        http2_stream_data *current = sd->root[i];
        http2_stream_data *prev = NULL;

        while(current)
        {
            prev = current;
            current = current->next;
            http2_remove_stream(sd, prev);
            http2_free_stream(&prev);
        }
        
        nghttp2_session_del(sd->session[i]);
        sd->root[i] = NULL;
        sd->session[i] = NULL;
    }
    http2_free_return_data_list(sd);
    free(sd);
}

#endif /* HAVE_LIBNGHTTP2 */
