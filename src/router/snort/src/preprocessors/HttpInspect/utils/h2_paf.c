#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "encode.h"
#include "decode.h"
#include "generators.h"
#include "h2_paf.h"
#include "hi_eo_events.h"
#include "decode.h"
#include "snort.h"
#include "stream_api.h"
#include "snort_debug.h"
#include "profiler.h"
#include "snort_httpinspect.h"
#include "memory_stats.h"

#include "h2_common.h"

static uint8_t h2_paf_id = 0;

#ifdef HAVE_LIBNGHTTP2
static Packet *h2_pkt = NULL;
static const uint8_t *h2_pkt_end = NULL;

static inline int h2_pseudo (void *ssn, uint32_t bytes,uint8_t* data, H2Hdr *hd, uint8_t flags);
#endif

static inline uint32_t GetForwardDir (const Packet* p)
{
    if ( p->packet_flags & PKT_FROM_SERVER )
        return PKT_FROM_SERVER;

    else if ( p->packet_flags & PKT_FROM_CLIENT )
        return PKT_FROM_CLIENT;

    return 0;
}

static inline uint32_t GetReverseDir (const Packet* p)
{
    if (p->packet_flags & PKT_FROM_SERVER )
        return PKT_FROM_CLIENT;
    else if ( p->packet_flags & PKT_FROM_CLIENT )
        return PKT_FROM_SERVER;

    return 0;
}

#ifdef HAVE_LIBNGHTTP2
static void ShowRebuiltPacket (Packet* p)
{
    if (stream_api->is_show_rebuilt_packets_enabled())
    {
        printf("+++++++++++++++++++H2 Packet+++++++++++++++++++++\n");
        PrintIPPkt(stdout, IPPROTO_TCP, p);
        printf("+++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }
}
#endif

void h2_paf_clear(void* userdata);

static PAF_Status h2_paf (
        void* ssn, void** pv, const uint8_t* data, uint32_t len,
        uint64_t *flags, uint32_t* fp, uint32_t* fp_eoh)
{
    PAF_Status paf = PAF_IGNORE;
    PROFILE_VARS;
    
    PREPROC_PROFILE_START(hi2PerfStats);

    if (!stream_api->is_session_http2(ssn))
    {
        if (*flags & PKT_H1_ABORT)
            paf = PAF_ABORT;
        else
            paf = PAF_SEARCH;

        PREPROC_PROFILE_END(hi2PerfStats);
        return paf;
    }


#ifdef HAVE_LIBNGHTTP2
    int ret = 0;
    int i = 1;
    return_data_list_node *current;

    http2_session_data *sd = (http2_session_data *)session_api->get_application_data(ssn, PP_HTTP2);

    if(sd == NULL)
    {
        bool upg;
        
        sd = (http2_session_data*) SnortPreprocAlloc(1, sizeof(http2_session_data), 
                                        PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);  

        if (!sd)
        {
            PREPROC_PROFILE_END(hi2PerfStats);
            return PAF_ABORT;
        }

        upg = stream_api->is_session_http2_upg(ssn);

        PREPROC_PROFILE_START(hi2InitPerfStats);
        {
            initialize_nghttp2_session_snort(&(sd->session[0]), sd, 0, upg);
            initialize_nghttp2_session_snort(&(sd->session[1]), sd, 1, upg);
        }
        PREPROC_PROFILE_END(hi2InitPerfStats);

        session_api->set_application_data(ssn, PP_HTTP2, sd, h2_paf_clear);
    }

    sd->first_return_data = NULL;
    sd->num_of_return_data = 0;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: Amount of data passed for Http/2 processing: %u\n", __FUNCTION__, len);)

    PREPROC_PROFILE_START(hi2PayloadPerfStats);
    ret = process_payload_http2(sd, data, len, *flags & PKT_FROM_CLIENT);
    PREPROC_PROFILE_END(hi2PayloadPerfStats);

    if (ret < 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
            "%s: Unable to decode http2 data: %d\n", __FUNCTION__, ret);)
    }

    current = sd->first_return_data;
    while (current != NULL)
    {
        if (current->return_data.length != 0 && current->return_data.message != NULL)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                "%s: Return msg %d - Http/2 decoded data available: %u\n", __FUNCTION__, i, current->return_data.length);)
            PREPROC_PROFILE_START(hi2PseudoPerfStats);
            h2_pseudo(ssn, current->return_data.length, (uint8_t *) current->return_data.message, &(current->return_data.hd), current->return_data.flags);
            PREPROC_PROFILE_END(hi2PseudoPerfStats);
        }
        current = current->next;
        i++;
    }
    http2_free_return_data_list(sd);
#endif /* HAVE_LIBNGHTTP2 */
    *fp = len;

    PREPROC_PROFILE_END(hi2PerfStats);

    return paf;
}

void h2_paf_clear(void* userdata)
{
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,"%s: Clearing h2_paf\n", __FUNCTION__);)
#ifdef HAVE_LIBNGHTTP2
    free_http2_session_data(userdata);
    if (h2_pkt)
    {
        if (h2_pkt->h2Hdr)
            SnortPreprocFree(h2_pkt->h2Hdr, sizeof(H2Hdr), PP_HTTPINSPECT, 
                 PP_MEM_CATEGORY_SESSION);
        Encode_Delete(h2_pkt);
        h2_pkt = NULL;
    }
#endif /* HAVE_LIBNGHTTP2 */
}

int h2_paf_register_port (
        struct _SnortConfig *sc, uint16_t port, bool client, bool server, tSfPolicyId pid, bool auto_on)
{

    if ( !ScPafEnabled() )
        return 0;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                "%s: policy %u, port %u\n", __FUNCTION__, pid, port);)

    if ( !stream_api )
            return -1;

    if ( client )
        h2_paf_id = stream_api->register_paf_port(sc, pid, port, true, h2_paf, auto_on);

    if ( server )
        h2_paf_id = stream_api->register_paf_port(sc, pid, port, false, h2_paf, auto_on);

    return 0;
}

static void h2_paf_cleanup(void *pafData)
{
#ifdef HAVE_LIBNGHTTP2
    if (pafData)
        SnortPreprocFree(pafData, sizeof(http2_session_data), PP_HTTPINSPECT, 
             PP_MEM_CATEGORY_SESSION);
#endif
}

int h2_paf_register_service (
        struct _SnortConfig *sc, uint16_t service, bool client, bool server, tSfPolicyId pid, bool auto_on)
{
    if ( !ScPafEnabled() )
        return 0;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                "%s: policy %u, service %u\n", __FUNCTION__, pid, service);)

    if ( !stream_api )
            return -1;

    if ( client )
    {
        h2_paf_id = stream_api->register_paf_service(sc, pid, service, true, h2_paf, auto_on);
        stream_api->register_paf_free(h2_paf_id, h2_paf_cleanup);
    }
    if ( server )
    {
        h2_paf_id = stream_api->register_paf_service(sc, pid, service, false, h2_paf, auto_on);
        stream_api->register_paf_free(h2_paf_id, h2_paf_cleanup);
    }
    return 0;
}

#ifdef HAVE_LIBNGHTTP2
static inline int h2_pseudo(void *ssn, uint32_t bytes, uint8_t* data, H2Hdr *hd, uint8_t flags)
{

    Packet *p = stream_api->get_wire_packet();

    uint8_t fp = stream_api->get_flush_policy_dir();
    uint32_t dir;
    int ret;

    if (fp)
    {
        dir = GetForwardDir(p);
    } else {
        dir = GetReverseDir(p);
    }

    EncodeFlags enc_flags = 0;

    if (!h2_pkt)
        h2_pkt = Encode_New();
    else if (h2_pkt->h2Hdr)
        SnortPreprocFree(h2_pkt->h2Hdr, sizeof(H2Hdr), PP_HTTPINSPECT, 0);

    if ( !p->packet_flags || (dir & p->packet_flags) )
        enc_flags = ENC_FLAG_FWD;

    Encode_Format(enc_flags, p, h2_pkt, PSEUDO_PKT_TCP);

    h2_pkt_end = h2_pkt->data + h2_pkt->max_dsize;
    // TBD in ips mode, these should be coming from current packet (tdb)
    //((TCPHdr *)h2_pkt->tcph)->th_ack = htonl(st->l_unackd);
    //((TCPHdr *)h2_pkt->tcph)->th_win = htons((uint16_t)st->l_window);

    do
    {
        if (bytes > h2_pkt->max_dsize)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                    "%s: Data frame is much bigger than max_dsize",
                    __FUNCTION__);)
            return 0;
        }

        ret = SafeMemcpy((uint8_t *)h2_pkt->data, data, bytes, (uint8_t *)h2_pkt->data, h2_pkt_end);
        if (ret != SAFEMEM_SUCCESS)
        {
           DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                   "%s: SafeMemcpy() Failed !!!",  __FUNCTION__);)
           return 0;
        }

        h2_pkt->packet_flags |= (PKT_REBUILT_STREAM|PKT_STREAM_EST|PKT_STREAM_ORDER_OK);
        h2_pkt->dsize = (uint16_t)bytes;

        h2_pkt->h2Hdr = (H2Hdr *)SnortPreprocAlloc(1, sizeof(H2Hdr), PP_HTTPINSPECT, 
                                     PP_MEM_CATEGORY_SESSION); 
        if (h2_pkt->h2Hdr != NULL)
        {
            copy_hd(h2_pkt->h2Hdr, *hd);
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                "%s: Unable to allcoate memory for h2Hdr\n", __FUNCTION__);)
        }

        if (flags & MESSAGE_FLAG_START_PDU)
            h2_pkt->packet_flags |= PKT_PDU_HEAD;

        if (flags & MESSAGE_FLAG_END_PDU)
            h2_pkt->packet_flags |= PKT_PDU_TAIL;

        Encode_Update(h2_pkt);

        if(IS_IP4(h2_pkt))
        {
            h2_pkt->inner_ip4h.ip_len = h2_pkt->iph->ip_len;
        }
        else
        {
            IP6RawHdr* ip6h = (IP6RawHdr*)h2_pkt->raw_ip6h;
            if ( ip6h ) h2_pkt->inner_ip6h.len = ip6h->ip6plen;
        }

        ((DAQ_PktHdr_t*)h2_pkt->pkth)->ts.tv_sec = ((DAQ_PktHdr_t*)p->pkth)->ts.tv_sec;
        ((DAQ_PktHdr_t*)h2_pkt->pkth)->ts.tv_usec = ((DAQ_PktHdr_t*)p->pkth)->ts.tv_usec;

        sfBase.iStreamFlushes++;
        h2_pkt->packet_flags |= dir;
        h2_pkt->ssnptr = (void *)ssn;
#ifdef TARGET_BASED
        h2_pkt->application_protocol_ordinal = p->application_protocol_ordinal;
#endif
        //Need to see how to update statistics for h2_paf
        hi_stats.h2_rebuilt_packets++;
        UpdateStreamReassStats(&sfBase, bytes);
        {
            int tmp_do_detect, tmp_do_detect_content;

            tmp_do_detect = do_detect;
            tmp_do_detect_content = do_detect_content;

            SnortEventqPush();
            ShowRebuiltPacket(h2_pkt);
            Preprocess(h2_pkt);
            SnortEventqPop();
            DetectReset(h2_pkt->data, h2_pkt->dsize);

            do_detect = tmp_do_detect;
            do_detect_content = tmp_do_detect_content;
        }

    } while (0);
    return bytes;
}
#endif
