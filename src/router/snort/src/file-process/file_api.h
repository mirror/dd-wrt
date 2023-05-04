/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * ** Copyright (C) 2012-2013 Sourcefire, Inc.
 * ** AUTHOR: Hui Cao
 * **
 * ** This program is free software; you can redistribute it and/or modify
 * ** it under the terms of the GNU General Public License Version 2 as
 * ** published by the Free Software Foundation.  You may not use, modify or
 * ** distribute this program under any other version of the GNU General
 * ** Public License.
 * **
 * ** This program is distributed in the hope that it will be useful,
 * ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 * ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * ** GNU General Public License for more details.
 * **
 * ** You should have received a copy of the GNU General Public License
 * ** along with this program; if not, write to the Free Software
 * ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * */

/* file_api.h
 *
 * Purpose: Definition of the FileAPI.  To be used as a common interface
 *          for file process access for other preprocessors and detection
 *          plugins.
 *
 *  Author(s):  Hui Cao <hcao@sourcefire.com>
 *
 *  NOTES
 *  5.25.12 - Initial Source Code. Hcao
 */

#ifndef FILE_API_H_
#define FILE_API_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include "sfPolicy.h"

#define ENABLE_FILE_TYPE_IDENTIFICATION     0x1
#define ENABLE_FILE_SIGNATURE_SHA256        0x2
#define ENABLE_FILE_CAPTURE                 0x4
#define FILE_ALL_ON                         0xFFFFFFFF
#define FILE_ALL_OFF                        0x00000000
#define MAX_FILE                            1024
#define MAX_EMAIL                           1024
#define MAX_UNICODE_FILE_NAME               1024

#define FILE_RESUME_BLOCK                   0x01
#define FILE_RESUME_LOG                     0x02

/*
 * Generator id. Define here the same as the official register
 * in generators.h
 */
#define GENERATOR_FILE_TYPE         146
#define GENERATOR_FILE_SIGNATURE    147

#define FILE_SIGNATURE_SHA256       1
#define FILE_SIGNATURE_SHA256_STR   "(file) malware detected"

#define UTF_16_BE_BOM "\xFE\xFF"
#define UTF_16_LE_BOM "\xFF\xFE"

#define UTF_16_BE_BOM_LEN 2
#define UTF_16_LE_BOM_LEN 2

typedef enum _File_Verdict
{
    FILE_VERDICT_UNKNOWN = 0,
    FILE_VERDICT_LOG,
    FILE_VERDICT_STOP,
    FILE_VERDICT_BLOCK,
    FILE_VERDICT_REJECT,
    FILE_VERDICT_PENDING,
    FILE_VERDICT_STOP_CAPTURE,
    FILE_VERDICT_MAX
} File_Verdict;

typedef enum _FilePosition
{
    SNORT_FILE_POSITION_UNKNOWN,
    SNORT_FILE_START,
    SNORT_FILE_MIDDLE,
    SNORT_FILE_END,
    SNORT_FILE_FULL
} FilePosition;

typedef enum _FileCaptureState
{
    FILE_CAPTURE_SUCCESS = 0,
    FILE_CAPTURE_MIN,                 /*smaller than file capture min*/
    FILE_CAPTURE_MAX,                 /*larger than file capture max*/
    FILE_CAPTURE_MEMCAP,              /*memcap reached, no more file buffer*/
    FILE_CAPTURE_FAIL                 /*Other file capture failures*/
} FileCaptureState;

typedef enum _FileSigState
{
    FILE_SIG_PROCESSING = 0,
    FILE_SIG_DEPTH_FAIL,              /*larger than file signature depth*/
    FILE_SIG_FLUSH,
    FILE_SIG_DONE
} FileSigState;

typedef enum _FileProcessType
{
    SNORT_FILE_TYPE_ID,
    SNORT_FILE_SHA256,
    SNORT_FILE_CAPTURE
} FileProcessType;

typedef enum _FileCharEncoding
{
    SNORT_CHAR_ENCODING_ASCII = 0,
    SNORT_CHAR_ENCODING_UTF_16LE,
    SNORT_CHAR_ENCODING_UTF_16BE,
}FileCharEncoding;

typedef struct _FileState
{
    FileCaptureState capture_state;
    FileSigState     sig_state;
} FileState;

typedef struct _FileCacheStatus
{
    uint64_t prunes;  /* number of file entries pruned due to memcap*/
    uint64_t segment_mem_in_use; /* memory used currently */
    uint64_t segment_mem_in_use_max; /* Maximal memory usage */
} FileCacheStatus;

struct s_MAIL_LogState;
struct _DecodeConfig;
struct s_MAIL_LogConfig;
struct _MimeDataPafInfo;
struct _MimeState;

struct _FileCaptureInfo;
typedef struct _FileCaptureInfo FileCaptureInfo;
struct _SnortConfig;
struct _FileContext;
struct _FileCache;

struct _MemPool;

typedef struct _FileSession
{
    struct _FileContext *current_context;
    struct _FileContext *main_context;
    struct _FileContext *pending_context;
    uint32_t  max_file_id;
    struct _FileCache *file_cache;
    uint64_t file_id;

} FileSession;

#define FILE_API_VERSION   5

#define DEFAULT_FILE_ID    0

typedef uint32_t (*File_policy_callback_func) (void* ssnptr, int16_t app_id, bool upload);
typedef File_Verdict (*File_type_callback_func) (void* p, void* ssnptr,
        uint32_t file_type_id, bool upload, uint32_t file_id);
typedef File_Verdict (*File_signature_callback_func) (void* p, void* ssnptr,
        uint8_t* file_sig, uint64_t file_size, FileState *state, bool upload,
        uint32_t file_id, bool partial_file);
typedef void (*Log_file_action_func) (void* ssnptr, int action);

typedef int (*File_process_func)( void* p, uint8_t* file_data, int data_size, FilePosition position,
        bool upload, bool suspend_block_verdict, bool do_flush);
typedef int (*Get_file_name_func) (void* ssnptr, uint8_t **file_name, uint32_t *name_len);
typedef uint64_t (*Get_file_size_func) (void* ssnptr);
typedef bool (*Get_file_direction_func) (void* ssnptr);
typedef uint8_t *(*Get_file_sig_sha256_func) (void* ssnptr);

typedef void (*Set_file_name_func) (void* ssnptr, uint8_t *, uint32_t, bool);
typedef void (*Set_file_direction_func) (void* ssnptr, bool);

typedef int64_t (*Get_file_depth_func) (struct _SnortConfig *snort_conf, bool next);
typedef bool (*Is_file_signature_enabled_func) (void);

typedef void (*Set_file_policy_func)(File_policy_callback_func);
typedef void (*Enable_file_type_func)(struct _SnortConfig* sc, File_type_callback_func);
typedef void (*Enable_file_signature_func)(struct _SnortConfig* sc, File_signature_callback_func);
typedef void (*Enable_file_capture_func)(struct _SnortConfig* sc, File_signature_callback_func);
typedef void (*Set_file_action_log_func)(Log_file_action_func);
typedef void (*Install_file_service_func)(void);

typedef int (*Set_log_buffers_func)(struct s_MAIL_LogState **log_state, struct s_MAIL_LogConfig *conf, void *mempool, 
                                    void* scbPtr, uint32_t preproc_id);
typedef void (*Update_mime_mempool_func)(void*, int, int);
typedef void (*Update_log_mempool_func)(void*, int, int);
typedef void (*Display_mime_mempool_func)(void *memory_pool, struct _DecodeConfig *decode_conf_old, struct _DecodeConfig *decode_conf_new);
typedef void (*Display_log_mempool_func)(void *memory_pool, unsigned memcap_old, unsigned memcap_new);
typedef void (*Display_decode_depth_func)(struct _DecodeConfig *decode_conf_old, struct _DecodeConfig *decode_conf_new);
typedef void* (*Init_mime_mempool_func)(int max_mime_mem, int max_depth, void *mempool, const char *preproc_name);
typedef void* (*Init_log_mempool_func)(uint32_t email_hdrs_log_depth, uint32_t memcap,  void *mempool, const char *preproc_name);

typedef int (*File_resume_block_add_file_func)(void *pkt, uint32_t file_sig,
        uint32_t timeout, File_Verdict verdict, uint32_t file_type_id, uint8_t *signature,
        uint16_t cli_port, uint16_t srv_port, bool create_pinhole, bool direction);
typedef File_Verdict (*File_resume_block_check_func)(void *pkt, uint32_t file_sig);
typedef uint32_t (*Str_to_hash_func)(uint8_t *str, int length );
typedef void (*File_signature_lookup_func)(void* p, bool is_retransmit);
typedef void (*Set_mime_decode_config_defaults_func)(struct _DecodeConfig *decode_conf);
typedef void (*Set_mime_log_config_defaults_func)(struct s_MAIL_LogConfig *log_config);
typedef int (*Parse_mime_decode_args_func)(struct _DecodeConfig *decode_conf, char *arg, const char *preproc_name, char **saveptr);
typedef const uint8_t * (*Process_mime_data_func)(void *packet, const uint8_t *start, const uint8_t *end,
        struct _MimeState *mime_ssn, bool upload, bool paf_enabled, char *protocol, uint32_t preproc_id);
typedef void (*Free_mime_session_func)(struct _MimeState *mime_ssn);
typedef bool (*Is_decoding_enabled_func)(struct _DecodeConfig *decode_conf);
typedef bool (*Check_decoding_conf_func)(struct _DecodeConfig *configNext, struct _DecodeConfig *config, const char *preproc_name);
typedef bool (*Is_mime_log_enabled_func)(struct s_MAIL_LogConfig *log_config);
typedef void (*Finalize_mime_position_func)(void *ssnptr, void *decode_state, FilePosition *position);
typedef File_Verdict (*Get_file_verdict_func)(void *ssnptr);
typedef void (*Render_block_verdict_func)(void *ctx, void *p);
typedef FileCaptureState (*Reserve_file_func)(void *ssnptr, FileCaptureInfo **file_mem);
typedef void* (*Get_file_func)(FileCaptureInfo *file_mem, uint8_t **buff, int *size);
typedef void (*Release_file_func)(FileCaptureInfo *data);
typedef size_t (*File_capture_size_func)(FileCaptureInfo *file_mem);

typedef bool (*Is_file_service_enabled)(void);
typedef bool (*Check_paf_abort_func)(void* ssn);
typedef void (*Update_file_name_func) (struct s_MAIL_LogState *log_state);
typedef FilePosition (*GetFilePosition)(void *pkt);
typedef void (*Reset_mime_paf_state_func)(struct _MimeDataPafInfo *data_info);
/*  Process data boundary and flush each file based on boundary*/
typedef bool (*Process_mime_paf_data_func)(struct _MimeDataPafInfo *data_info,  uint8_t data);
typedef bool (*Check_data_end_func)(void *end_state,  uint8_t data);
typedef uint32_t (*Get_file_type_id)(void *);
typedef uint32_t (*Get_new_file_instance)(void *);

/*Context based file process functions*/
typedef struct _FileContext* (*Create_file_context_func)(void *ssnptr);
typedef void (*Init_file_context_func)(void *ssnptr, bool upload, struct _FileContext  *ctx);
typedef struct _FileContext* (*Get_file_context_func)(void *ssnptr);
typedef bool (*Set_file_context_func)(void *ssnptr, struct _FileContext *ctx);
typedef int (*Process_file_func)( struct _FileContext *ctx, void *p,
        uint8_t *file_data, int data_size, FilePosition position,
        bool suspend_block_verdict);
typedef void *(*File_cache_update_entry_func) (struct _FileCache *fileCache, void* p, uint64_t file_id,
        uint8_t *file_name, uint32_t file_name_size,  uint64_t file_size, bool reset, bool no_update_size);
typedef int (*File_segment_process_func)( struct _FileCache *fileCache, void* p, uint64_t file_id,
        uint64_t file_size, const uint8_t* file_data, int data_size, uint64_t offset,
        bool upload);
typedef struct _FileCache * (*File_cache_create_func)(uint64_t memcap, uint32_t cleanup_files);
typedef void (*File_cache_free_func)(struct _FileCache *fileCache);
typedef FileCacheStatus * (*File_cache_status_func)(struct _FileCache *fileCache);
typedef int64_t (*Get_max_file_capture_size)(void *ssn);
typedef bool (*File_config_malware_check)(void *ssn, uint16_t app_id);
typedef FileCharEncoding (*Get_character_encoding)(uint8_t *, uint32_t);
typedef bool (*File_cache_mem_adjust_func)(struct _FileCache *fileCache, uint8_t *pWork);
typedef void (*File_cache_mem_set_func)(struct _FileCache *fileCache, uint64_t memcap);
typedef void (*File_event_log_dump_func)( struct _FileCache *fileCache, void* p, uint64_t file_id);
typedef void (*File_signature_reset)(void *ssnptr);
typedef void (*Set_file_partial_func)(void *p, FilePosition position, bool upload, bool is_partial);
typedef char* (*File_get_filetype_func) (void *ssnptr); 

typedef struct _file_api
{
    int version;

    /* Check if file type id is enabled.
     *
     * Arguments: None
     * 
     * Returns:
     *   (bool) true   file processing is enabled
     *   (bool) false  file processing is disabled
     */
    Is_file_service_enabled is_file_service_enabled;

    /* File process function, called by preprocessors that provides file data
     *
     * Arguments:
     *    void* p: packet pointer
     *    uint8_t* file_data: file data
     *    int data_size: file data size
     *    FilePosition: file position
     *    bool upload: upload or not
     * Returns:
     *    1: continue processing/log/block this file
     *    0: ignore this file (no further processing needed)
     */
    File_process_func file_process;

    /*-----File property functions--------*/

    /* Get file name and the length of file name
     * Note: this is updated after file processing. It will be available
     * for file event logging, but might not be available during file type
     * callback or file signature callback, because those callbacks are called
     * during file processing.
     *
     * Arguments:
     *    void* ssnptr: session pointer
     *    uint8_t **file_name: address for file name to be saved
     *    uint32_t *name_len: address to save file name length
     * Returns
     *    1: file name available,
     *    0: file name is unavailable
     */
    Get_file_name_func get_file_name;

    /* Get file size
     * Note: this is updated after file processing. It will be available
     * for file event logging, but might not be available during file type
     * callback or file signature callback, because those callbacks are called
     * during file processing.
     *
     * Arguments:
     *    void* ssnptr: session pointer
     *
     * Returns
     *    uint64_t: file size
     *    Note: 0 means file size is unavailable
     */
    Get_file_size_func get_file_size;

    /* Get number of bytes processed
     *
     * Arguments:
     *    void* ssnptr: session pointer
     *
     * Returns
     *    uint64_t: processed file data size
     */
    Get_file_size_func get_file_processed_size;

    /* Get file direction
     *
     * Arguments:
     *    void* ssnptr: session pointer
     *
     * Returns
     *    1: upload
     *    0: download
     */
    Get_file_direction_func get_file_direction;

    /* Get file signature sha256
     *
     * Arguments:
     *    void* ssnptr: session pointer
     *
     * Returns
     *    char *: pointer to sha256
     *    NULL: sha256 is not available
     */
    Get_file_sig_sha256_func get_sig_sha256;

    /* Set file name and the length of file name
     *
     * Arguments:
     *    void* ssnptr: session pointer
     *    uint8_t *file_name: file name to be saved
     *    uint32_t name_len: file name length
     *    bool save_in_context: true if file name is saved in context
     *                          instead of session
     * Returns
     *    None
     */
    Set_file_name_func set_file_name;

    /* Get file direction
     *
     * Arguments:
     *    void* ssnptr: session pointer
     *    bool:
     *       1 - upload
     *       0 - download
     * Returns
     *    None
     */
    Set_file_direction_func set_file_direction;

    /*----------File call backs--------------*/

    /* Set file policy callback. This callback is called in the beginning
     * of session. This callback will decide whether to do file type ID,
     * file signature, or file capture
     *
     * Arguments:
     *    File_policy_callback_func
     * Returns
     *    None
     */
    Set_file_policy_func set_file_policy_callback;

    /* Enable file type ID and set file type callback.
     * File type callback is called when file type is identified. Callback
     * will return a verdict based on file type
     *
     * Arguments:
     *    File_type_callback_func
     * Returns
     *    None
     */
    Enable_file_type_func enable_file_type;

    /* Enable file signature and set file signature callback.
     * File signature callback is called when file signature is calculated.
     * Callback will return a verdict based on file signature.
     * SHA256 is calculated after file transfer is finished.
     *
     * Arguments:
     *    File_signature_callback_func
     * Returns
     *    None
     */
    Enable_file_signature_func enable_file_signature;

    /* Enable file capture and set file signature callback.
     * File signature callback is called when file signature is calculated.
     * Callback will return a verdict based on file signature.
     * SHA256 is calculated after file transfer is finished.
     *
     * Note: file signature and file capture will use the same callback, but
     * enabled separately.
     *
     * Arguments:
     *    File_signature_callback_func
     * Returns
     *    None
     */
    Enable_file_signature_func enable_file_capture;

    /* Set file action log callback.
     * File action log callback is called when file resume is detected.
     * It allows file events to be generated for a resumed file download
     *
     * Arguments:
     *    Log_file_action_func
     * Returns
     *    None
     */
    Set_file_action_log_func set_file_action_log_callback;

    /* Install file service.
     * This must be called in band with packets.
     * It makes the functions set in the other enable calls active.
     *
     * Arguments:
     *    None
     * Returns
     *    None
     */
    Install_file_service_func install_file_service;

    /*--------------File configurations-------------*/

    /* Get file depth required for all file processings enabled
     *
     * Arguments:
     *    None
     *
     * Returns:
     *    int64_t: file depth in bytes
     */
    Get_file_depth_func get_max_file_depth;

    /* Is file signature enabled
     *
     * Arguments:
     *    None
     *
     * Returns:
     *    bool: true if file_signature_enabled is set
     */
    Is_file_signature_enabled_func is_file_signature_enabled;


    /*--------------Common functions used for MIME processing-------------*/
    Set_log_buffers_func set_log_buffers;
    Update_mime_mempool_func update_mime_mempool;
    Update_log_mempool_func update_log_mempool;
    Display_mime_mempool_func displayMimeMempool;
    Display_log_mempool_func displayLogMempool;
    Display_decode_depth_func displayDecodeDepth;
    Init_mime_mempool_func init_mime_mempool;
    Init_log_mempool_func init_log_mempool;
    Set_mime_decode_config_defaults_func set_mime_decode_config_defauts;
    Set_mime_log_config_defaults_func set_mime_log_config_defauts;
    Parse_mime_decode_args_func parse_mime_decode_args;
    Process_mime_data_func process_mime_data;
    Free_mime_session_func free_mime_session;
    Is_decoding_enabled_func is_decoding_enabled;
    Check_decoding_conf_func check_decoding_conf;
    Is_mime_log_enabled_func is_mime_log_enabled;
    Finalize_mime_position_func finalize_mime_position;
    Reset_mime_paf_state_func reset_mime_paf_state;
    Process_mime_paf_data_func process_mime_paf_data;
    Check_data_end_func check_data_end;
    Check_paf_abort_func check_paf_abort;

    /*--------------Other helper functions-------------*/
    File_resume_block_add_file_func file_resume_block_add_file;
    File_resume_block_check_func file_resume_block_check;
    Str_to_hash_func str_to_hash;
    File_signature_lookup_func file_signature_lookup;
    Get_file_verdict_func get_file_verdict;
    Render_block_verdict_func render_block_verdict;
    /*
     * Preserve the file in memory until it is released
     * This function must be called in packet processing thread
     * Arguments:
     *   void *ssnptr: session pointer
     *   void **file_mem: the pointer to store the memory block
     *       that stores file and its metadata.
     *       It will set  NULL if no memory or fail to store
     *
     * Returns:
     *   FileCaptureState:
     *      FILE_CAPTURE_SUCCESS = 0,
     *      FILE_CAPTURE_MIN,
     *      FILE_CAPTURE_MAX,
     *      FILE_CAPTURE_MEMCAP,
     *      FILE_CAPTURE_FAIL
     */
    Reserve_file_func reserve_file;

    /*
     * Get the file that is reserved in memory. To get a full file,
     * this function must be called iteratively until NULL is returned
     * This function can be called in out of band thread
     *
     * Arguments:
     *   void *file_mem: the memory block working on
     *   uint8_t **buff: address to store buffer address
     *   int *size: address to store size of file
     *
     * Returns:
     *   the next memory block
     *   If NULL: no memory or fail to get file
     */
    Get_file_func read_file;

    /*
     * Get the file size captured in the file buffer
     * This function can be called in out of band thread
     *
     * Arguments:
     *   void *file_mem: the first memory block of file buffer
     *
     * Returns:
     *   the size of file
     *   If 0: no memory or fail to read file
     */
    File_capture_size_func get_file_capture_size;

    /*
     * Release the file that is reserved in memory.
     * This function can be called in out of band thread.
     *
     * Arguments:
     *   void *data: the memory block that stores file and its metadata
     *
     * Returns:
     *   None
     */
    Release_file_func release_file;

    /* Return the file rule id associated with a session.
     *
     * Arguments:
     *   void *ssnptr: session pointer
     * 
     * Returns:
     *   (u32) file-rule id on session; FILE_TYPE_UNKNOWN otherwise.
     */
    Get_file_type_id get_file_type_id;

    /* Create a file context to use
     *
     * Arguments:
     *    void* ssnptr: session pointer
     * Returns:
     *    FileContext *: file context created.
     */
    Create_file_context_func create_file_context;

    /* Intialize a file context
     *
     * Arguments:
     *    void* ssnptr: session pointer
     * Returns:
     *    FileContext *: file context.
     */
    Init_file_context_func init_file_context;

    /* Set file context to be the current
     *
     * Arguments:
     *    void* ssnptr: session pointer
     *    FileContext *: file context that will be current
     * Returns:
     *    True: changed successfully
     *    False: fail to change
     */
    Set_file_context_func set_current_file_context;

    /* Get current file context
     *
     * Arguments:
     *    void* ssnptr: session pointer
     * Returns:
     *    FileContext *: current file context
     */
    Get_file_context_func get_current_file_context;

    /* Get main file context that used by preprocessors
     *
     * Arguments:
     *    void* ssnptr: session pointer
     * Returns:
     *    FileContext *: main file context
     */
    Get_file_context_func get_main_file_context;

    /* Process file function, called by preprocessors that provides file data
     *
     * Arguments:
     *    void* ctx: file context that will be processed
     *    void* p: packet pointer
     *    uint8_t* file_data: file data
     *    int data_size: file data size
     *    FilePosition: file position
     *    bool suspend_block_verdict: used for smb to allow file pass
     * Returns:
     *    1: continue processing/log/block this file
     *    0: ignore this file (no further processing needed)
     */
    Process_file_func process_file;

    /* Create the file cache that store file segments and properties.
     *
     * Arguments:
     *    uint64_t: total memory available for file cache, including file contexts
     *    uint32_t: maximal number of files pruned when memcap is reached
     * Returns:
     *    struct _FileCache *: file cache pointer
     */
    File_cache_create_func file_cache_create;

    /* Free the file cache that store file segments and properties.
     *
     * Arguments:
     *    struct _FileCache *: file cache pointer
     * Returns:
     *    None
     */
    File_cache_free_func file_cache_free;

    /* Get the status of file cache for troubleshooting.
     *
     * Arguments:
     *    struct _FileCache *: file cache pointer
     * Returns:
     *    FileCacheStatus *: status of file cache
     */
    File_cache_status_func file_cache_status;

    /* Get a new file entry in the file cache, if already exists, update file name
     *
     * Arguments:
     *    struct _FileCache *: file cache that stores file segments
     *    void* : packet pointer
     *    uint64_t: file id that is unique
     *    uint8_t *: file name
     *    uint32_t:  file name size
     * Returns:
     *    None
     */
    File_cache_update_entry_func file_cache_update_entry;

    /* Process file segment, when file segment is in order, file data will be
     * processed; otherwise it is stored.
     *
     * Arguments:
     *    struct _FileCache *: file cache that stores file segments
     *    void* : packet pointer
     *    uint64_t: file id that is unique
     *    uint64_t: total file size,
     *    const uint8_t*: file data
     *    int: file data size
     *    uint64_t: file data offset in the file
     *    bool: true for upload, false for download
     * Returns:
     *    1: continue processing/log/block this file
     *    0: ignore this file (no further processing needed)
     */
    File_segment_process_func file_segment_process;

    /* Return a unique file instance number
     *
     * Arguments:
     *   void *ssnptr: session pointer
     * Returns:
     *   (u32) a unique file instance id.
     */
    Get_new_file_instance get_new_file_instance;

    GetFilePosition get_file_position;

    Get_max_file_capture_size get_max_file_capture_size;
    File_config_malware_check  file_config_malware_check;
    /* Return the character encoding of a buffer
     * Arguments:
     *   uint8 *: input buffer
     *   uint32 : input buffer length
     * Returns:
     *     FileCharEncoding
               SNORT_CHAR_ENCODING_ASCII = 0,
               SNORT_CHAR_ENCODING_UTF_16LE,
               SNORT_CHAR_ENCODING_UTF_16BE
     */
    Get_character_encoding get_character_encoding;

    File_cache_mem_adjust_func file_cache_shrink_to_memcap;
    File_cache_mem_set_func    file_cache_set_memcap;
    File_signature_reset       file_signature_reset;   
    /* Return a char string that indicates the file type
     * Arguments:
     *   void * ssnptr: session pointer
     * Returns:
     *   File Type name
     */
    File_get_filetype_func file_get_filetype; 

    /* Logging a file event */
    File_event_log_dump_func file_event_log_dump;
    Set_file_partial_func set_file_partial;

} FileAPI;

/* To be set by Stream */
extern FileAPI *file_api;

static inline void initFilePosition(FilePosition *position,
        uint64_t processed_size)
{
    *position = SNORT_FILE_START;
    if (processed_size)
        *position = SNORT_FILE_MIDDLE;
}
static inline void updateFilePosition(FilePosition *position,
        uint64_t processed_size)
{
    if ((*position == SNORT_FILE_END) || (*position == SNORT_FILE_FULL))
        *position = SNORT_FILE_START;
    else if (processed_size)
        *position = SNORT_FILE_MIDDLE;
}
static inline void finalFilePosition(FilePosition *position)
{
    if (*position == SNORT_FILE_START)
        *position = SNORT_FILE_FULL;
    else if (*position != SNORT_FILE_FULL)
        *position = SNORT_FILE_END;
}

static inline bool isFileStart(FilePosition position)
{
    return ((position == SNORT_FILE_START) || (position == SNORT_FILE_FULL));
}

static inline bool isFileEnd(FilePosition position)
{
    return ((position == SNORT_FILE_END) || (position == SNORT_FILE_FULL));
}
#endif /* FILE_API_H_ */

