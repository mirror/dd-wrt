/*
 **
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Copyright (C) 2012-2013 Sourcefire, Inc.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License Version 2 as
 **  published by the Free Software Foundation.  You may not use, modify or
 **  distribute this program under any other version of the GNU General
 **  Public License.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  5.25.12 - Initial Source Code. Hui Cao
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "sf_types.h"
#include <sys/types.h>
#include "file_api.h"
#include "file_config.h"
#include "file_mime_config.h"
#include "file_capture.h"
#include "file_stats.h"

#include "session_api.h"
#include "stream_api.h"
#include "mstring.h"
#include "preprocids.h"
#include "detect.h"
#include "plugbase.h"
#include "active.h"

#include "file_mime_process.h"
#include "file_resume_block.h"
#include "snort_httpinspect.h"
#include "file_service.h"
#include "file_segment_process.h"

static bool file_type_force = false;

static uint32_t file_config_version = 0;

FileServiceConfig cur_config;
static FileServiceConfig new_config;
static FileServiceConfig init_config;

/*Main File Processing functions */
static int file_process(void* ssnptr, uint8_t* file_data, int data_size,
        FilePosition position, bool upload, bool suspend_block_verdict, bool do_flush);

/*File properties*/
static int get_file_name(void* ssnptr, uint8_t **fname, uint32_t *name_size);
static uint64_t get_file_size(void* ssnptr);
static uint64_t get_file_processed_size(void* ssnptr);
static bool get_file_direction(void* ssnptr);
static uint8_t *get_file_sig_sha256(void* ssnptr);

static void set_file_name(void* ssnptr, uint8_t * fname, uint32_t name_size,
        bool save_in_context);
static void set_file_direction(void* ssnptr, bool upload);

static void set_file_policy_callback(File_policy_callback_func);
static void enable_file_type(struct _SnortConfig* sc, File_type_callback_func);
static void enable_file_signature(struct _SnortConfig* sc, File_signature_callback_func);
static void enable_file_capture(struct _SnortConfig* sc, File_signature_callback_func );
static void set_file_action_log_callback(Log_file_action_func);

static int64_t get_max_file_depth(struct _SnortConfig *snort_conf, bool next);
static bool is_file_signature_enabled(void);

static uint32_t str_to_hash(uint8_t *str, int length );

static void file_signature_lookup(void* p, bool is_retransmit);
static void file_signature_callback(Packet* p);

static inline void finish_signature_lookup(FileContext *context);
static File_Verdict get_file_verdict(void *ssnptr);
static void render_block_verdict(void *ctx, void *p);

static bool is_file_service_enabled(void);
static uint32_t get_file_type_id(void *ssnptr);
static uint32_t get_new_file_instance(void *ssnptr);
static void set_file_partial(void *p, FilePosition position, bool upload, bool is_partial);

/* File context based file processing*/
FileContext* create_file_context(void *ssnptr);
static void init_file_context(void *ssnptr, bool upload, FileContext *context);
bool set_current_file_context(void *ssnptr, FileContext *ctx);
FileContext* get_current_file_context(void *ssnptr);
FileContext* get_main_file_context(void *ssnptr);
static int process_file_context(FileContext *ctx, void *p, uint8_t *file_data,
        int data_size, FilePosition position, bool suspend_block_verdict);
static FilePosition get_file_position(void *pkt);
static bool check_paf_abort(void* ssn);
static int64_t get_max_file_capture_size(void *ssn);
static void file_session_free(void *session_data);
extern FileEntry *file_cache_get(FileCache *fileCache, void* p, uint64_t file_id,
    bool can_create);
static FileCharEncoding get_character_encoding(uint8_t *buffer, uint32_t length);
void file_event_log_dump(FileCache *fileCache, void* p, uint64_t file_id);
static void file_signature_reset(void* ssnptr);
static char* file_get_filetype (void *ssnptr);

FileAPI fileAPI;
FileAPI* file_api = NULL;

static unsigned s_cb_id = 0;

void init_fileAPI(void)
{
    fileAPI.version = FILE_API_VERSION;
    fileAPI.is_file_service_enabled = &is_file_service_enabled;
    fileAPI.file_process = &file_process;
    fileAPI.get_file_name = &get_file_name;
    fileAPI.get_file_size = &get_file_size;
    fileAPI.get_file_processed_size = &get_file_processed_size;
    fileAPI.get_file_direction = &get_file_direction;
    fileAPI.get_sig_sha256 = &get_file_sig_sha256;
    fileAPI.set_file_name = &set_file_name;
    fileAPI.set_file_direction = &set_file_direction;
    fileAPI.set_file_policy_callback = &set_file_policy_callback;
    fileAPI.enable_file_type = &enable_file_type;
    fileAPI.enable_file_signature = &enable_file_signature;
    fileAPI.enable_file_capture = &enable_file_capture;
    fileAPI.set_file_action_log_callback = &set_file_action_log_callback;
    fileAPI.install_file_service = &FileServiceInstall;
    fileAPI.get_max_file_depth = &get_max_file_depth;
    fileAPI.is_file_signature_enabled = &is_file_signature_enabled;
    fileAPI.set_log_buffers = &set_log_buffers;
#ifdef SNORT_RELOAD
    fileAPI.update_mime_mempool = &update_mime_mempool;
    fileAPI.update_log_mempool = &update_log_mempool;
#ifdef REG_TEST
    fileAPI.displayMimeMempool = &displayMimeMempool;
    fileAPI.displayLogMempool = &displayLogMempool;
    fileAPI.displayDecodeDepth = &displayDecodeDepth;
#endif
#endif
    fileAPI.init_mime_mempool = &init_mime_mempool;
    fileAPI.init_log_mempool=  &init_log_mempool;
    fileAPI.file_resume_block_add_file = &file_resume_block_add_file;
    fileAPI.file_resume_block_check = &file_resume_block_check;
    fileAPI.str_to_hash = &str_to_hash;
    fileAPI.file_signature_lookup = &file_signature_lookup;
    fileAPI.set_mime_decode_config_defauts = &set_mime_decode_config_defauts;
    fileAPI.set_mime_log_config_defauts = &set_mime_log_config_defauts;
    fileAPI.parse_mime_decode_args = &parse_mime_decode_args;
    fileAPI.process_mime_data = &process_mime_data;
    fileAPI.free_mime_session = &free_mime_session;
    fileAPI.is_decoding_enabled = &is_decoding_enabled;
    fileAPI.check_decoding_conf = &check_decode_config;
    fileAPI.is_mime_log_enabled = &is_mime_log_enabled;
    fileAPI.finalize_mime_position = &finalize_mime_position;
    fileAPI.get_file_verdict = &get_file_verdict;
    fileAPI.render_block_verdict = &render_block_verdict;
    fileAPI.reserve_file = &file_capture_reserve;
    fileAPI.read_file = &file_capture_read;
    fileAPI.release_file = &file_capture_release;
    fileAPI.get_file_capture_size = &file_capture_size;
    fileAPI.get_file_type_id = &get_file_type_id;
    fileAPI.get_new_file_instance = &get_new_file_instance;

    fileAPI.create_file_context = &create_file_context;
    fileAPI.init_file_context = &init_file_context;
    fileAPI.set_current_file_context = &set_current_file_context;
    fileAPI.get_current_file_context = &get_current_file_context;
    fileAPI.get_main_file_context = &get_main_file_context;
    fileAPI.process_file = &process_file_context;
    fileAPI.get_file_position = &get_file_position;
    fileAPI.reset_mime_paf_state = &reset_mime_paf_state;
    fileAPI.process_mime_paf_data = &process_mime_paf_data;
    fileAPI.check_data_end = check_data_end;
    fileAPI.check_paf_abort = &check_paf_abort;
    fileAPI.get_max_file_capture_size = get_max_file_capture_size;
    fileAPI.file_cache_update_entry = &file_cache_update_entry;
    fileAPI.file_segment_process = &file_segment_process;
    fileAPI.file_cache_create = &file_cache_create;
    fileAPI.file_cache_free = &file_cache_free;
    fileAPI.file_cache_status = &file_cache_status;
    fileAPI.file_config_malware_check = &file_config_malware_check;
    fileAPI.get_character_encoding = &get_character_encoding;
    fileAPI.file_cache_shrink_to_memcap = &file_cache_shrink_to_memcap;
    fileAPI.file_cache_set_memcap = &file_cache_set_memcap;
    fileAPI.file_event_log_dump = &file_event_log_dump;
    fileAPI.file_signature_reset= &file_signature_reset;
    fileAPI.set_file_partial = &set_file_partial;
    fileAPI.file_get_filetype = &file_get_filetype;
    file_api = &fileAPI;
    init_mime();
    RegisterMemoryStatsFunction(PP_FILE, FilePrintMemStats);
}


#if defined(DEBUG_MSGS) || defined (REG_TEST)
static void printFileServiceChanges()
{
   FileConfig *file_config =  (FileConfig *)(snort_conf->file_config);
   if(cur_config.file_signature_enabled != new_config.file_signature_enabled)
       printf("File service Install: file_signature %s\n",new_config.file_signature_enabled ? "enabled":"disabled");
   if(cur_config.file_type_id_enabled != new_config.file_type_id_enabled)
       printf("File service Install: file_type %s\n",new_config.file_type_id_enabled ? "enabled":"disabled");
   if(cur_config.file_capture_enabled != new_config.file_capture_enabled)
       printf("File service Install: file_capture %s\n",new_config.file_capture_enabled ? "enabled":"disabled");

   if (!file_config)
       return;
   if(new_config.file_type_id_enabled)
       printf("File service Install: file_type_depth:%u \n",(unsigned)file_config->file_type_depth);
   if(new_config.file_signature_enabled)
       printf("File service Install: file_signature_depth:%u \n",(unsigned)file_config->file_signature_depth);
   fflush(stdout);
}
#endif

void FileServiceInstall(void)
{
#if defined(DEBUG_MSGS) || defined (REG_TEST)
    printFileServiceChanges();
#endif
    cur_config = new_config;
    new_config = init_config;

    if (stream_api)
    {
        if(cur_config.file_signature_enabled &&
                !s_cb_id )
            s_cb_id = stream_api->register_event_handler(file_signature_callback);
    }
}

static void start_file_processing(struct _SnortConfig* sc, bool capture)
{
    static bool file_processing_initiated = false;

    if (!file_processing_initiated)
    {
        file_resume_block_init();
        RegisterPreprocStats("file", print_file_stats);
        file_processing_initiated = true;
    }

    if (!sc)
        sc = snort_conf;

    if (!sc->file_config)
        sc->file_config = file_service_config_create();

    if (capture)
    {
        FileConfig* file_config = sc->file_config;
        file_capture_init_mempool(file_config->file_capture_memcap,
                file_config->file_capture_block_size);
    }
}

void free_file_config(void *conf)
{
    file_config_version++;
    file_rule_free(conf);
    file_identifiers_free(conf);
    SnortPreprocFree(conf, sizeof(FileConfig), PP_FILE, PP_MEM_CATEGORY_CONFIG);
}

void close_fileAPI(void)
{
    file_resume_block_cleanup();
    free_mime();
    file_caputure_close();
}

FileSession* get_file_session(void *ssnptr)
{
    return ((FileSession*)session_api->get_application_data(ssnptr, PP_FILE));
}

static inline FileSession* get_create_file_session(void *ssnptr)
{
    FileSession *file_session = get_file_session(ssnptr);
    if(!file_session)
    {
        file_session = (FileSession *)SnortPreprocAlloc(1, sizeof(*file_session), 
                PP_FILE, PP_MEM_CATEGORY_SESSION);
        if (session_api->set_application_data(ssnptr, PP_FILE, file_session,
                file_session_free))
        {
            SnortPreprocFree(file_session, sizeof(*file_session), 
                    PP_FILE, PP_MEM_CATEGORY_SESSION);
            return NULL;
        }
    }
    return(file_session);
}

/*Get the current working file context*/
FileContext* get_current_file_context(void *ssnptr)
{
    FileSession *file_session = get_file_session (ssnptr);
    if (file_session)
        return file_session->current_context;
    else
    {
        FILE_WARNING("Failed to get current file context: file session not found");
        return NULL;
    }
}

/*Get the current main file context*/
FileContext* get_main_file_context(void *ssnptr)
{
    FileSession *file_session = get_file_session (ssnptr);
    if (file_session)
        return file_session->main_context;
    else
    {
        FILE_WARNING("Failed to get main file context: file session not found");
        return NULL;
    }
}

/*Get the current working file context*/
static inline void save_to_pending_context(void *ssnptr)
{
    FileSession *file_session = get_create_file_session (ssnptr);
    /* Save to pending_context */
    if (!file_session)
        return;

    if (file_session->main_context)
    {
        if (file_session->pending_context != file_session->main_context)
            file_context_free(file_session->pending_context);
        file_session->pending_context = file_session->main_context;
    }
    else
    {
        file_session->pending_context = file_session->current_context;
    }
}

/*Set the current working file context*/
bool set_current_file_context(void *ssnptr, FileContext *ctx)
{
    FileSession *file_session = get_create_file_session (ssnptr);

    if (!file_session)
    {
        FILE_WARNING("Failed to set current file context: file session not found");
        return false;
    }

    file_session->current_context = ctx;
    return true;
}

static void file_session_free(void *session_data)
{
    FileSession *file_session = (FileSession *)session_data;
    if (!file_session)
        return;

    /*Clean up all the file contexts*/
    if (file_session->main_context)
    {
        if ( file_session->pending_context &&
                (file_session->main_context != file_session->pending_context))
        {
            file_context_free(file_session->pending_context);
        }

        file_context_free(file_session->main_context);
    }
    SnortPreprocFree(file_session, sizeof(FileSession), PP_FILE,
             PP_MEM_CATEGORY_SESSION);
}

static void init_file_context(void *ssnptr, bool upload, FileContext *context)
{
    context->file_type_enabled = cur_config.file_type_id_enabled;
    context->file_signature_enabled = cur_config.file_signature_enabled;
    context->file_capture_enabled = cur_config.file_capture_enabled;
    context->file_config = snort_conf->file_config;
    context->file_config_version = file_config_version;
    context->smb_unknown_file_size = false;
    context->partial_file = false;
    context->attached_file_entry = NULL;
    file_direction_set(context,upload);
    file_stats.files_total++;
#ifdef TARGET_BASED
    /* Check file policy to see whether we want to do either file type, file
     * signature,  or file capture
     * Note: this happen only on the start of session*/
    if (cur_config.file_policy_cb)
    {
        uint32_t policy_flags = 0;
        context->app_id = session_api->get_application_protocol_id(ssnptr);

        policy_flags = cur_config.file_policy_cb(ssnptr, context->app_id, upload);

        if ( !file_type_force && !(policy_flags & ENABLE_FILE_TYPE_IDENTIFICATION) )
                context->file_type_enabled = false;

        if ( !(policy_flags & ENABLE_FILE_SIGNATURE_SHA256) )
            context->file_signature_enabled = false;

        if ( !(policy_flags & ENABLE_FILE_CAPTURE) )
            context->file_capture_enabled = false;
    }
#endif
}

FileContext* create_file_context(void *ssnptr)
{
    FileContext *context = file_context_create();

    /* Create file session if not yet*/
    get_create_file_session (ssnptr);
    FILE_DEBUG("Successfully created file context %p",context);

    return context;
}

static inline FileContext* find_main_file_context(void* p, FilePosition position,
        bool upload)
{
    FileContext* context = NULL;
    Packet *pkt = (Packet *)p;
    void *ssnptr = pkt->ssnptr;
    FileSession *file_session = get_file_session (ssnptr);

    /* Attempt to get a previously allocated context. */
    if (file_session)
        context  = file_session->main_context;

    if (context && (((position == SNORT_FILE_MIDDLE) || (position == SNORT_FILE_END)) ||
                ((context->partial_file) && (SNORT_FILE_START == position))))
        return context;
    else if (context)
    {
        /*Push file event when there is another file in the same packet*/
        if (pkt->packet_flags & PKT_FILE_EVENT_SET)
        {
            SnortEventqLog(snort_conf->event_queue, p);
            SnortEventqReset();
            pkt->packet_flags &= ~PKT_FILE_EVENT_SET;
        }

        if (context->verdict != FILE_VERDICT_PENDING)
        {
            /* Reuse the same context */
            file_context_reset(context);
            init_file_context(ssnptr, upload, context);
            context->file_id = file_session->max_file_id++;
            FILE_DEBUG("Reusing existing context from last session");
            return context;
        }
    }

    context = create_file_context(ssnptr);
    file_session = get_create_file_session (ssnptr);
    file_session->main_context = context;
    init_file_context(ssnptr, upload, context);
    context->file_id = file_session->max_file_id++;
    return context;
}

static inline void updateFileSize(FileContext* context, int data_size,
        FilePosition position)
{
    context->processed_bytes += data_size;
    if ((position == SNORT_FILE_END) || (position == SNORT_FILE_FULL) || (context->file_state.sig_state == FILE_SIG_FLUSH))
    {
        if (get_max_file_depth(snort_conf, false) == (int64_t)context->processed_bytes)
            context->file_size = 0;
        else
            context->file_size = context->processed_bytes;
    }
    FILE_DEBUG("Processed bytes: %u, Updated file size is: %u " ,context->processed_bytes, context->file_size);
    if((SNORT_FILE_FULL == position) || (SNORT_FILE_END == position))
    {
        context->processed_bytes = 0;
    }
}

int file_eventq_add(uint32_t gid, uint32_t sid, char *msg, RuleType type)
{
    OptTreeNode *otn;
    RuleTreeNode *rtn;

    otn = GetApplicableOtn(gid, sid, 1, 0, 3, msg);
    if (otn == NULL)
    {
        FILE_ERROR("Failed to add event: no otn");
        return 0;

    }
    rtn = getRtnFromOtn(otn, getIpsRuntimePolicy());
    if (rtn == NULL)
    {
        FILE_ERROR("Failed to add event: no rtn");
        return 0;
    }

    rtn->type = type;

    return SnortEventqAdd(gid, sid, 1, 0, 3, msg, otn);
}


static inline void add_file_to_block(Packet *p, FileContext* context,
        bool signature_available)
{
    uint8_t *buf = NULL;
    uint32_t len = 0;
    uint32_t type = 0;
    uint32_t file_sig = 0;
    uint8_t* signature = signature_available ? context->sha256 : NULL;
    Packet *pkt = (Packet *)p;
    FileConfig *file_config =  (FileConfig *)(snort_conf->file_config);

    Active_ForceDropPacket();
    DisableAllDetect( p );
    pkt->packet_flags |= PKT_FILE_EVENT_SET;

    /*Use URI as the identifier for file*/
    if (GetHttpUriData(p->ssnptr, &buf, &len, &type))
    {
        file_sig = str_to_hash(buf, len);
        file_resume_block_add_file(p, file_sig,
                (uint32_t)file_config->file_block_timeout, context->verdict,
                context->file_type_id, signature, 0, 0, true, 0);
    }
    /*use the file name for smb2*/
    else if(context->attached_file_entry && context->file_name_size > 0)
    {
        file_sig = str_to_hash(context->file_name, context->file_name_size);
        file_resume_block_add_file(p, file_sig,
                (uint32_t)file_config->file_block_timeout, context->verdict,
                context->file_type_id, signature, 0, 0, true, 0);
        /*We cant call file_entry_free directly as that will delete the context, but we still may be using it.
          So we are unlinking the context from the file entry. this way the context will not be deleted now,
          but it will be deleted as part of tcp cleanup. As no context is linked to the file entry now, it will
          be set to do resume check. */
        ((FileEntry*)(context->attached_file_entry))->context = NULL;
        context->attached_file_entry = NULL;
    }

    FILE_INFO("File blocked");
    if (pkt_trace_enabled)
        addPktTraceData(VERDICT_REASON_FILE, snprintf(trace_line, MAX_TRACE_LINE,
            "File Process: %s %s\n", getPktTraceActMsg(), (buf && len)? (char *)buf : ""));
    else addPktTraceData(VERDICT_REASON_FILE, 0);
}
/*
 * Check HTTP partial content header
 * Return: 1: partial content header
 *         0: not http partial content header
 */
static inline int check_http_partial_content(Packet *p)
{
    uint8_t *buf = NULL;
    uint32_t len = 0;
    uint32_t type = 0;
    uint32_t file_sig;
    const HttpBuffer* hb = GetHttpBuffer(HTTP_BUFFER_STAT_CODE);
    uint8_t partial_cont = isHttpRespPartialCont(p->ssnptr);
    int is_not_partial_ret_code = 0;

    /* Not partial content, return */
    if (hb)
    {
        if (hb->length != 3)
        {
            is_not_partial_ret_code = 1;
        }
        else
        {
            is_not_partial_ret_code = strncmp((const char*)hb->buf, "206", 3);
        }
        if (((is_not_partial_ret_code) && !(partial_cont &= PARTIAL_CONTENT)) ||
            ((!is_not_partial_ret_code) && (partial_cont &= FULL_CONTENT)))
        {
            return 0;
        }
    }
    else if (!(partial_cont &= PARTIAL_CONTENT))
    {
        return 0;
    }

    /*Use URI as the identifier for file*/
    if (GetHttpUriData(p->ssnptr, &buf, &len, &type))
    {
        file_sig = str_to_hash(buf, len);
        file_resume_block_check(p, file_sig);
    }
    FILE_DEBUG("HTTP partial content header found");

    return 1;
}

/* File signature lookup at the end of file
 * File signature callback can be used for malware lookup, file capture etc
 */
static inline void _file_signature_lookup(FileContext* context,
        void* p, bool is_retransmit, bool suspend_block_verdict)
{
    File_Verdict verdict = FILE_VERDICT_UNKNOWN;
    Packet *pkt = (Packet *)p;
    void *ssnptr = pkt->ssnptr;

    if (cur_config.file_signature_cb)
    {
        FILE_DEBUG("Doing file signature callback...");
        verdict = cur_config.file_signature_cb(p, ssnptr, context->sha256,
                context->file_size, &(context->file_state), context->upload,
                context->file_id, context->partial_file);
        if(context->file_state.sig_state != FILE_SIG_FLUSH)
            file_stats.verdicts_signature[verdict]++;
    }
    FILE_INFO("File Signature lookup verdict: %d", verdict);

    if (suspend_block_verdict)
        context->suspend_block_verdict = true;

    context->verdict = verdict;

    if ((verdict == FILE_VERDICT_LOG ) && (context->file_state.sig_state != FILE_SIG_FLUSH))
    {
        file_eventq_add(GENERATOR_FILE_SIGNATURE, FILE_SIGNATURE_SHA256,
                FILE_SIGNATURE_SHA256_STR, RULE_TYPE__ALERT);
        pkt->packet_flags |= PKT_FILE_EVENT_SET;
        context->file_signature_enabled = false;
    }
    else if (verdict == FILE_VERDICT_PENDING)
    {
        /*Can't decide verdict, drop packet and waiting...*/
        if (is_retransmit)
        {
            FileConfig *file_config =  (FileConfig *)context->file_config;
            /*Drop packets if not timeout*/
            if (pkt->pkth->ts.tv_sec <= context->expires)
            {
                if( !Active_DAQRetryPacket(pkt) )
                    Active_ForceDropPacket();
                if (pkt_trace_enabled)
                {
                    addPktTraceData(VERDICT_REASON_FILE, snprintf(trace_line, MAX_TRACE_LINE,
                        "File Process: malware detected, gid %u, sid %u, %s\n",
                        GENERATOR_FILE_SIGNATURE, FILE_SIGNATURE_SHA256, getPktTraceActMsg()));
                }
                else addPktTraceData(VERDICT_REASON_FILE, 0);
                FILE_INFO("Malware detected");
                return;
            }
            /*Timeout, let packet go through OR block based on config*/
            context->file_signature_enabled = false;
            if (pkt_trace_enabled)
            {
                addPktTraceData(VERDICT_REASON_FILE, snprintf(trace_line, MAX_TRACE_LINE,
                            "File Process: file signature lookup verdict pending timeout, %s\n", getPktTraceActMsg()));
            }
            else addPktTraceData(VERDICT_REASON_FILE, 0);

            if (file_config && file_config->block_timeout_lookup)
                file_eventq_add(GENERATOR_FILE_SIGNATURE, FILE_SIGNATURE_SHA256,
                        FILE_SIGNATURE_SHA256_STR, RULE_TYPE__REJECT);
            else
                file_eventq_add(GENERATOR_FILE_SIGNATURE, FILE_SIGNATURE_SHA256,
                        FILE_SIGNATURE_SHA256_STR, RULE_TYPE__ALERT);
            pkt->packet_flags |= PKT_FILE_EVENT_SET;
        }
        else
        {
            FileConfig *file_config =  (FileConfig *)context->file_config;
            if (file_config)
                context->expires = (time_t)(file_config->file_lookup_timeout + pkt->pkth->ts.tv_sec);

            if( !Active_DAQRetryPacket(pkt) )
                Active_ForceDropPacket();
            if (pkt_trace_enabled)
            {
                addPktTraceData(VERDICT_REASON_FILE, snprintf(trace_line, MAX_TRACE_LINE,
                    "File Process: can't decide verdict and waiting, %s\n", getPktTraceActMsg()));
            }
            else addPktTraceData(VERDICT_REASON_FILE, 0);
            if (!context->suspend_block_verdict)
                stream_api->set_event_handler(ssnptr, s_cb_id, SE_REXMIT);

            save_to_pending_context(ssnptr);
            return;
        }
    }
    else if ((verdict == FILE_VERDICT_BLOCK) || (verdict == FILE_VERDICT_REJECT))
    {
        if (!context->suspend_block_verdict)
            render_block_verdict(context, p);
        context->file_signature_enabled = false;
        return;
    }

    if(context->file_state.sig_state != FILE_SIG_FLUSH)
        finish_signature_lookup(context);
}

static inline void finish_signature_lookup(FileContext *context)
{
    if (context->sha256)
    {
        context->file_signature_enabled = false;
        file_capture_stop(context);
        file_stats.signatures_processed[context->file_type_id][context->upload]++;
#ifdef TARGET_BASED
        file_stats.signatures_by_proto[context->app_id]++;
#endif
    }
}

static File_Verdict get_file_verdict(void *ssnptr)
{
    FileContext *context = get_current_file_context(ssnptr);

    if (context == NULL)
        return FILE_VERDICT_UNKNOWN;

    return context->verdict;
}

static void render_block_verdict(void *ctx, void *p)
{
    FileContext *context = (FileContext *)ctx;
    Packet *pkt = (Packet *)p;
    SAVE_DAQ_PKT_HDR(p);

    if (p == NULL)
        return;

    if (context == NULL)
    {
        context = get_current_file_context(pkt->ssnptr);
        if (context == NULL)
            return;
    }

    if (context->verdict == FILE_VERDICT_BLOCK)
    {
        file_eventq_add(GENERATOR_FILE_SIGNATURE, FILE_SIGNATURE_SHA256,
                FILE_SIGNATURE_SHA256_STR, RULE_TYPE__DROP);
        add_file_to_block(p, context, true);
    }
    else if (context->verdict == FILE_VERDICT_REJECT)
    {
        file_eventq_add(GENERATOR_FILE_SIGNATURE, FILE_SIGNATURE_SHA256,
                FILE_SIGNATURE_SHA256_STR, RULE_TYPE__REJECT);
        add_file_to_block(p, context, true);
    }

    finish_signature_lookup(context);
}

static uint32_t get_file_type_id(void *ssnptr)
{
    // NOTE: 'ssnptr' NULL checked in get_application_data
    FileContext *context = get_current_file_context(ssnptr);

    if ( !context )
        return FILE_VERDICT_UNKNOWN;

    return context->file_type_id;
}

static uint32_t get_new_file_instance(void *ssnptr)
{
    FileSession *file_session = get_create_file_session (ssnptr);

    if (file_session)
    {
        return file_session->max_file_id++;
    }
    else
    {
        return 0;
    }
}

static void file_signature_lookup(void* p, bool is_retransmit)
{
    Packet *pkt = (Packet *)p;
    SAVE_DAQ_PKT_HDR(p);

    FileContext* context  = get_current_file_context(pkt->ssnptr);

    if (context && context->file_signature_enabled && context->sha256)
    {
        _file_signature_lookup(context, p, is_retransmit, false);
    }
}

static void file_signature_callback(Packet* p)
{
    /* During retransmission */
    Packet *pkt = (Packet *)p;
    void *ssnptr = pkt->ssnptr;
    FileSession *file_session;
    FileEntry *fileEntry;
    SAVE_DAQ_PKT_HDR(p);

    if (!ssnptr)
    {
        FILE_ERROR("Signature callback failed: no session");
        return;
    }
    file_session = get_file_session (ssnptr);
    if (!file_session)
    {
        FILE_ERROR("Signature callback failed: no file session");
        return;
    }

    if(file_session->file_cache)
    {
        fileEntry = file_cache_get(file_session->file_cache, p, file_session->file_id, false);
        if (fileEntry && fileEntry->context &&
            (fileEntry->context->verdict == FILE_VERDICT_PENDING))
        {
            file_session->current_context = fileEntry->context;
            file_signature_lookup(p, 1);
        }
    }
    else
    {
        if(file_session->pending_context)
        {
            file_session->current_context = file_session->pending_context;
        }
        file_signature_lookup(p, 1);
    }
}

static bool is_file_service_enabled()
{
    return (cur_config.file_type_id_enabled ||
            cur_config.file_signature_enabled);
}

/*
 * Return:
 *    1: continue processing/log/block this file
 *    0: ignore this file
 */
static int process_file_context(FileContext *context, void *p, uint8_t *file_data,
        int data_size, FilePosition position, bool suspend_block_verdict)
{
    Packet *pkt = (Packet *)p;
    void *ssnptr = pkt->ssnptr;
    bool file_capture_enabled = false;
    SAVE_DAQ_PKT_HDR(p);

    if (!context)
        return 0;

    file_capture_enabled = context->file_capture_enabled;
    set_current_file_context(ssnptr, context);
    file_stats.file_data_total += data_size;

    /* if file config is changed, update it*/
    if ((context->file_config != snort_conf->file_config) ||
            (context->file_config_version != file_config_version))
    {
        context->file_config = snort_conf->file_config;
        context->file_config_version = file_config_version;
        /* Reset file type context that relies on file_conf.
         * File type id will become UNKNOWN after file_type_id()
         * if in the middle of file and file type is CONTINUE (undecided) */
        context->file_type_context = NULL;
        FILE_DEBUG("Updated file config.");
    }

    if ((!context->file_type_enabled) && (!context->file_signature_enabled))
    {
        updateFileSize(context, data_size, position);
        FILE_DEBUG("Signature and Type lookup not enabled");
        return 0;
    }

    if(check_http_partial_content(p))
    {
        context->file_type_enabled = false;
        context->file_signature_enabled = false;
        return 0;
    }

    /*file type id*/
    if (context->file_type_enabled)
    {
        File_Verdict verdict = FILE_VERDICT_UNKNOWN;

        file_type_id(context, file_data, data_size, position);
        FILE_DEBUG("File type ID: %u", context->file_type_id);

        /*Don't care unknown file type*/
        if (context->file_type_id == SNORT_FILE_TYPE_UNKNOWN)
        {
            context->file_type_enabled = false;
            context->file_signature_enabled = false;
            updateFileSize(context, data_size, position);
            file_capture_stop(context);
            return 0;
        }

        if (context->file_type_id != SNORT_FILE_TYPE_CONTINUE)
        {
            if (cur_config.file_type_cb)
            {
                FILE_DEBUG("Doing file type callback...");
                verdict = cur_config.file_type_cb(p, ssnptr, context->file_type_id,
                        context->upload, context->file_id);
                FILE_INFO("File type verdict: %d",verdict);
                file_stats.verdicts_type[verdict]++;
                context->verdict = verdict;
            }
            context->file_type_enabled = false;
            file_stats.files_processed[context->file_type_id][context->upload]++;
#ifdef TARGET_BASED
            file_stats.files_by_proto[context->app_id]++;
#endif
        }

        if (verdict == FILE_VERDICT_LOG )
        {
            file_eventq_add(GENERATOR_FILE_TYPE, context->file_type_id,
                    file_type_name(context->file_config, context->file_type_id),
                    RULE_TYPE__ALERT);
            context->file_signature_enabled = false;
            pkt->packet_flags |= PKT_FILE_EVENT_SET;
        }
        else if (verdict == FILE_VERDICT_BLOCK)
        {
            file_eventq_add(GENERATOR_FILE_TYPE, context->file_type_id,
                    file_type_name(context->file_config, context->file_type_id),
                    RULE_TYPE__DROP);
            updateFileSize(context, data_size, position);
            context->file_signature_enabled = false;
            add_file_to_block(p, context, false);
            return 1;
        }
        else if (verdict == FILE_VERDICT_REJECT)
        {
            file_eventq_add(GENERATOR_FILE_TYPE, context->file_type_id,
                    file_type_name(context->file_config, context->file_type_id),
                    RULE_TYPE__REJECT);
            updateFileSize(context, data_size, position);
            context->file_signature_enabled = false;
            add_file_to_block(p, context, false);
            return 1;
        }
        else if (verdict == FILE_VERDICT_STOP)
        {
            context->file_signature_enabled = false;
        }
        else if (verdict == FILE_VERDICT_STOP_CAPTURE)
        {
            file_capture_stop(context);
        }
    }

    /* file signature calculation */
    if (context->file_signature_enabled)
    {
        if (!context->sha256)
            file_signature_sha256(context, file_data, data_size, position);
        file_stats.data_processed[context->file_type_id][context->upload]
                                                         += data_size;
        updateFileSize(context, data_size, position);

        /*Fails to capture, when out of memory or size limit, need lookup*/
        if (context->file_capture_enabled &&
                file_capture_process(context, file_data, data_size, position))
        {
            file_capture_stop(context);
            _file_signature_lookup(context, p, false, suspend_block_verdict);
            if (context->verdict != FILE_VERDICT_UNKNOWN)
                return 1;
        }

        /*Either get SHA or exceeding the SHA limit, need lookup*/

        if (context->file_state.sig_state == FILE_SIG_DEPTH_FAIL)
        {
            file_stats.files_sig_depth++;
            _file_signature_lookup(context, p, false, suspend_block_verdict);
        }
        else if ((context->file_state.sig_state == FILE_SIG_DONE) && isFileEnd(position))
        {
            FILE_REG_DEBUG_WRAP(if (context->sha256) file_sha256_print(context->sha256);)
            _file_signature_lookup(context, p, false, suspend_block_verdict);
        }
        else if(context->file_state.sig_state == FILE_SIG_FLUSH)
        {
            _file_signature_lookup(context, p, false, suspend_block_verdict);
            context->file_signature_enabled = true;
            context->file_capture_enabled = file_capture_enabled;
            if((context->verdict == FILE_VERDICT_BLOCK) || (context->verdict == FILE_VERDICT_REJECT))
            {
                FILE_REG_DEBUG_WRAP(if (context->sha256) file_sha256_print(context->sha256);)
            }
        }
    }
    else
    {
        updateFileSize(context, data_size, position);
    }
    return 1;
}

/*
 * Return:
 *    1: continue processing/log/block this file
 *    0: ignore this file
 */
static int file_process( void* p, uint8_t* file_data, int data_size,
        FilePosition position, bool upload, bool suspend_block_verdict, bool do_flush)
{
    FileContext* context;
    SAVE_DAQ_PKT_HDR(p);
    int fileverdict;
#if defined(DAQ_VERSION) && DAQ_VERSION > 9
    uint64_t start = 0, end = 0;
#endif    

    FILE_DEBUG("Processing file data:: size:%d, position:%d, direction:%d, flush:%d",data_size,position,upload,do_flush);

    /* if both disabled, return immediately*/
    if (!is_file_service_enabled())
    {
        FILE_DEBUG("File service not enabled.");
        return 0;
    }

    if (position == SNORT_FILE_POSITION_UNKNOWN)
        return 0;
    FILE_REG_DEBUG_WRAP(DumpHexFile(stdout, file_data, data_size);)

    context = find_main_file_context(p, position, upload);
    if((context->file_state.sig_state == FILE_SIG_FLUSH) && context->sha256)
    {
        SnortPreprocFree(context->sha256, sizeof(SHA256_HASH_SIZE), PP_FILE, 
                PP_MEM_CATEGORY_SESSION);
        context->sha256 = NULL;
    }
    if(do_flush)
        context->file_state.sig_state = FILE_SIG_FLUSH;
    else
    {
        context->file_state.sig_state = FILE_SIG_PROCESSING;
    }

#if defined(DAQ_VERSION) && DAQ_VERSION > 9
    Packet *pkt = (Packet *)p;
    if ( pkt && pkt->pkth && (pkt->pkth->flags & DAQ_PKT_FLAG_DEBUG_ON))
    {
       get_clockticks(start);
       fileverdict = process_file_context(context, p, file_data, data_size, position,suspend_block_verdict);
       get_clockticks(end);
       print_flow(p,"PROCESS_FILE_CONTEXT",0,start,end);
    }
    else
       fileverdict = process_file_context(context, p, file_data, data_size, position,suspend_block_verdict);
#else
    fileverdict = process_file_context(context, p, file_data, data_size, position,suspend_block_verdict);
#endif

    return fileverdict;
}

static void set_file_name (void* ssnptr, uint8_t* fname, uint32_t name_size,
        bool save_in_context)
{
    FileContext* context = get_current_file_context(ssnptr);
    file_name_set(context, fname, name_size, save_in_context);
    FILE_REG_DEBUG_WRAP(printFileContext(context);)
}

/*
 *  set_file_partial API, used to mark a file partial/incomplete.
 *  This information is required by FW signature lookup for FTP PP
 *  See CSCvi28409 for more details.
 */
static void set_file_partial(void *p, FilePosition position,bool upload, bool is_partial)
{
    SAVE_DAQ_PKT_HDR(p);
    FileContext *context = find_main_file_context(p,position,upload);
    FILE_DEBUG("Partial file: %d",is_partial);
    context->partial_file = is_partial;
}

/* Return 1: file name available,
 *        0: file name is unavailable
 */
static int get_file_name (void* ssnptr, uint8_t **fname, uint32_t *name_size)
{
    return file_name_get(get_current_file_context(ssnptr), fname, name_size);
}

static uint64_t  get_file_size(void* ssnptr)
{
    return file_size_get(get_current_file_context(ssnptr));
}

static uint64_t  get_file_processed_size(void* ssnptr)
{
    FileContext *context = get_main_file_context(ssnptr);
    if (context)
        return (context->processed_bytes);
    else
        return 0;
}

static void set_file_direction(void* ssnptr, bool upload)
{
    file_direction_set(get_current_file_context(ssnptr),upload);
}

static bool get_file_direction(void* ssnptr)
{
    return file_direction_get(get_current_file_context(ssnptr));
}

static uint8_t *get_file_sig_sha256(void* ssnptr)
{
    return file_sig_sha256_get(get_current_file_context(ssnptr));
}

static void set_file_policy_callback(File_policy_callback_func policy_func_cb)
{
    new_config.file_policy_cb = policy_func_cb;
}

/*
 * - Only accepts 1 (ONE) callback being registered.
 *
 * - Call with NULL callback to "force" (guarantee) file type identification.
 *
 * TBD: Remove per-context "file_type_enabled" checking to simplify implementation.
 *
 */
static void enable_file_type(struct _SnortConfig* sc, File_type_callback_func callback)
{
    new_config.file_type_id_enabled = true;
    start_file_processing(sc, false);
    LogMessage("File service: file type enabled.\n");

    if (!callback)
    {
        file_type_force = true;
    }
    else if(!new_config.file_type_cb)
    {
        new_config.file_type_cb = callback;
    }
    else if(new_config.file_type_cb != callback)
    {
        FatalError("Attempt to register multiple file_type callbacks.");
    }
}

/* set file signature callback function*/
static void enable_file_signature(struct _SnortConfig* sc, File_signature_callback_func callback)
{

    new_config.file_signature_enabled = true;
    start_file_processing(sc, false);

    if(!new_config.file_signature_cb)
    {
        new_config.file_signature_cb = callback;
        LogMessage("File service: file signature enabled.\n");
    }
    else if(new_config.file_signature_cb != callback)
    {
        WarningMessage("File service: signature callback redefined.\n");
    }
}

/* Enable file capture, also enable file signature */
static void enable_file_capture(struct _SnortConfig* sc, File_signature_callback_func callback)
{
    new_config.file_capture_enabled = true;
    LogMessage("File service: file capture enabled.\n");
    start_file_processing(sc, true);
    /* Enable file signature*/
    enable_file_signature(sc, callback);
}

static void set_file_action_log_callback(Log_file_action_func log_func)
{
    new_config.log_file_action = log_func;
}

/* Get maximal file depth based on configuration
 * This function must be called after all file services are configured/enabled.
 */
static int64_t get_max_file_depth(struct _SnortConfig *sc, bool next)
{
    FileConfig *file_config;
    FileServiceConfig *fs_config;

    if (!sc)
        sc = snort_conf;
    file_config =  (FileConfig *)(sc->file_config);
    fs_config = next ? &new_config:&cur_config;

    if (!file_config)
        return -1;

     /* If next is set, proceed further to check the depth */
    if (!next && file_config->file_depth)
        return file_config->file_depth;

    file_config->file_depth = -1;


    if (fs_config->file_type_id_enabled)
    {
        file_config->file_depth = file_config->file_type_depth;
    }

    if (fs_config->file_signature_enabled)
    {
        if (file_config->file_signature_depth > file_config->file_depth)
            file_config->file_depth = file_config->file_signature_depth;
    }

    if (file_config->file_depth > 0)
    {
        /*Extra byte for deciding whether file data will be over limit*/
        file_config->file_depth++;
        return (file_config->file_depth);
    }
    else
    {
        return -1;
    }
}

static bool is_file_signature_enabled()
{
    return cur_config.file_signature_enabled;
}

static FilePosition get_file_position(void *pkt)
{
    FilePosition position = SNORT_FILE_POSITION_UNKNOWN;
    Packet *p = (Packet *)pkt;
    SAVE_DAQ_PKT_HDR(p);

    if(ScPafEnabled())
    {
        if (PacketHasFullPDU(p))
            position = SNORT_FILE_FULL;
        else if (PacketHasStartOfPDU(p))
            position = SNORT_FILE_START;
        else if (p->packet_flags & PKT_PDU_TAIL)
            position = SNORT_FILE_END;
        else if (get_file_processed_size(p->ssnptr))
            position = SNORT_FILE_MIDDLE;
    }

    return position;
}

/*
 *  This function determines whether we shold abort PAF.  Will return
 *  true if the current packet is midstream, or unestablisted session
 *
 *  PARAMS:
 *      uint32_t - session flags passed in to callback.
 *
 *  RETURNS:
 *      true - if we should abort paf
 *      false - if we should continue using paf
 */
static bool check_paf_abort(void* ssn)
{
    uint32_t flags = session_api->get_session_flags(ssn);
    if (flags & SSNFLAG_MIDSTREAM)
    {
        FILE_DEBUG("Aborting PAF because of midstream pickup.");
        return true;
    }
    else if (!(flags & SSNFLAG_ESTABLISHED))
    {
        FILE_DEBUG("Aborting PAF because of unestablished session.");
        return true;
    }
    return false;
}

static int64_t get_max_file_capture_size(void *ssn)
{
    if (snort_conf->file_config)
        return snort_conf->file_config->file_capture_max_size;

    return 0;
}

static uint32_t str_to_hash(uint8_t *str, int length )
{
    uint32_t a,b,c,tmp;
    int i,j,k,l;
    a = b = c = 0;
    for (i=0,j=0;i<length;i+=4)
    {
        tmp = 0;
        k = length - i;
        if (k > 4)
            k=4;

        for (l=0;l<k;l++)
        {
            tmp |= *(str + i + l) << l*8;
        }

        switch (j)
        {
        case 0:
            a += tmp;
            break;
        case 1:
            b += tmp;
            break;
        case 2:
            c += tmp;
            break;
        }
        j++;

        if (j == 3)
        {
            mix(a,b,c);
            j = 0;
        }
    }
    final(a,b,c);
    return c;
}

bool file_config_malware_check(void *ssnptr, uint16_t app_id)
{
	uint32_t policy_flags = 0;
	if (cur_config.file_policy_cb)
	{
        //Upload
	    policy_flags = cur_config.file_policy_cb(ssnptr, app_id, true);
        if ( (policy_flags & ENABLE_FILE_SIGNATURE_SHA256) || (policy_flags & ENABLE_FILE_TYPE_IDENTIFICATION) )
            return true;

        //Download
	        policy_flags = cur_config.file_policy_cb(ssnptr, app_id, false);
        if ( (policy_flags & ENABLE_FILE_SIGNATURE_SHA256) || (policy_flags & ENABLE_FILE_TYPE_IDENTIFICATION) )
            return true;
   }
   return false;
}

static FileCharEncoding get_character_encoding(uint8_t *buffer, uint32_t length)
{
    FileCharEncoding encoding = SNORT_CHAR_ENCODING_ASCII;

    if(length > UTF_16_LE_BOM_LEN)
    {
        if(memcmp(buffer, UTF_16_LE_BOM, UTF_16_LE_BOM_LEN) == 0)
            encoding = SNORT_CHAR_ENCODING_UTF_16LE;
        else if(memcmp(buffer, UTF_16_BE_BOM, UTF_16_BE_BOM_LEN) == 0)
            encoding = SNORT_CHAR_ENCODING_UTF_16BE;
    }

    return encoding;
}

 /* It generates the file event if event logging is enabled. */
void file_event_log_dump(FileCache *fileCache, void* p, uint64_t file_id)
{
    FileEntry *fileEntry;

    fileEntry = file_cache_get(fileCache, p, file_id, true);

    if (NULL != fileEntry && fileEntry->context)
    {
        Packet *pkt = (Packet *)p;

        if (FILE_VERDICT_LOG == fileEntry->context->verdict && 
                !(pkt->packet_flags & PKT_FILE_EVENT_SET))
        {
            file_eventq_add(GENERATOR_FILE_SIGNATURE, FILE_SIGNATURE_SHA256,
                    FILE_SIGNATURE_SHA256_STR, RULE_TYPE__ALERT);
            pkt->packet_flags |= PKT_FILE_EVENT_SET;
            fileEntry->context->file_signature_enabled = false;
        }
    }
}

/*      file_signature_reset  API to restore the file detection state. 
 *      This is done because once we flush and we get a cloud verdict, all the detection states are erased.
 *      To ensure that we continue the detection SSL has to use this API
 */
static void file_signature_reset (void *ssnptr)
{
    FileContext* context = NULL;
    FileSession *file_session = get_file_session (ssnptr);

    if (file_session)
        context = file_session->main_context;

    if (!context)
    {
        return;
    }

    if (context->file_state.sig_state == FILE_SIG_FLUSH)
    {
        context->file_signature_enabled = true;
        context->file_state.sig_state = FILE_SIG_PROCESSING;
        context->verdict = FILE_VERDICT_UNKNOWN;
        if (context->sha256)
        {
            SnortPreprocFree(context->sha256, sizeof(SHA256_HASH_SIZE), PP_FILE,
                    PP_MEM_CATEGORY_SESSION);
            context->sha256 = NULL;
        }
    }
    return;
}

static char* file_get_filetype (void *ssnptr)
{
    FileContext *context = get_current_file_context(ssnptr);

    if (!context)
    {
        return NULL;
    }

#ifdef TARGET_BASED
    if (cur_config.file_policy_cb)
    {
        bool policy_flags;
        policy_flags = cur_config.file_policy_cb(ssnptr, context->app_id, context->upload);
        if (!(policy_flags & ENABLE_FILE_TYPE_IDENTIFICATION))
        {
            return NULL;
        }
    }
#endif

    return file_type_name (context->file_config, context->file_type_id);
}

