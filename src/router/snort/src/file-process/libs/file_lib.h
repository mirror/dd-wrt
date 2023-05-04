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
 **  5.25.12 - Initial Source Code. Hcao
 */

#ifndef __FILE_LIB_H__
#define __FILE_LIB_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include "sf_types.h" /* for bool */

#include "file_api.h"
#include "sfdaq.h"

#define SNORT_FILE_TYPE_UNKNOWN          1024  /**/
#define SNORT_FILE_TYPE_CONTINUE         0 /**/

typedef struct _MagicData
{
    uint8_t *content;       /* magic content to match*/
    int content_len;        /* length of magic content */
    uint32_t offset;             /* pattern search start offset */

    /* Used in ds_list - do not try to iterate after parsing a rule
     * since the detection option tree will eliminate duplicates and
     * the list may have missing pmds */
    struct _MagicData *next; /* ptr to next match struct */

} MagicData;

typedef struct _RuleInfo
{
    char       *message;
    char       *type;
    char       *category;
    char       *version;
    MagicData  *magics;
    void       *groups;
    uint32_t   id;
    uint32_t   rev;
} RuleInfo;

typedef struct _FileContext
{
    bool file_type_enabled;
    bool file_signature_enabled;
    bool       file_name_saved;
    bool       upload;
    uint32_t   file_name_size;
    uint8_t    *file_name;
    uint64_t   file_size;
    uint64_t   processed_bytes;
    uint8_t    *sha256;
    void *     file_type_context;
    void *     file_signature_context;
    void *     file_config;
    time_t     expires;
    uint16_t   app_id;
    bool file_capture_enabled;
    bool partial_file;
    uint32_t   file_type_id;
    FileCaptureInfo *file_capture;
    uint8_t *current_data;  /*current file data*/
    uint32_t current_data_len;
    File_Verdict verdict;
    bool suspend_block_verdict;
    /* for some SMB upload cases, file size is not known during SMB negotiation. We are setting 
     * SIG_FLUSH during end of stream, but END of file is not set in case of SMB unknown 
     * file size upload. This causing file capture to fail. This flag is used to set End of file, 
     * only for SMB upload cases.
     */
    bool smb_unknown_file_size;
    void* attached_file_entry;
    FileState file_state;
    uint32_t file_id;
    uint32_t file_config_version;
} FileContext;

/*Main File Processing functions */
void file_type_id( FileContext* context, uint8_t* file_data, int data_size, FilePosition position);
void file_signature_sha256( FileContext* context, uint8_t* file_data, int data_size, FilePosition position);

/*File context management*/
FileContext *file_context_create(void);
void file_context_reset(FileContext *context);
void file_context_free(void *context);
/*File properties*/
void file_name_set (FileContext *context, uint8_t *file_name, uint32_t name_size, bool save_in_context);
int file_name_get (FileContext *context, uint8_t **file_name, uint32_t *name_size);
void file_size_set (FileContext *context, uint64_t file_size);
uint64_t file_size_get (FileContext *context);
void file_direction_set (FileContext *context, bool upload);
bool file_direction_get (FileContext *context);
void file_sig_sha256_set (FileContext *context, uint8_t *signature);
uint8_t* file_sig_sha256_get (FileContext *context);

char* file_type_name(void *conf, uint32_t);

bool file_IDs_from_type(const void *conf, const char *type,
     uint32_t **ids, uint32_t *count);

bool file_IDs_from_type_version(const void *conf, const char *type,
    const char *version, uint32_t **ids, uint32_t *count);

bool file_IDs_from_group(const void *conf, const char *group,
     uint32_t **ids, uint32_t *count);

extern int64_t file_type_depth;
extern int64_t file_signature_depth;

#if defined(DEBUG_MSGS) || defined (REG_TEST)
void file_sha256_print(unsigned char *hash);
#endif

#if defined (DAQ_VERSION) && DAQ_VERSION > 9
const DAQ_PktHdr_t* daq_pktHdr;
#define SAVE_DAQ_PKT_HDR(p) daq_pktHdr = ((Packet*)(p))->pkth
#define FILE_PKT_DEBUG(logLevel, msg, args...)\
    SNORT_DEBUG_PKT_LOG(daq_pktHdr,DAQ_DEBUG_PKT_MODULE_SNORTFILEPP,logLevel, msg, ##args)

#define FILE_EMERGENCY(msg,args...) FILE_PKT_DEBUG(DAQ_DEBUG_PKT_LEVEL_EMERGENCY,msg,##args) 
#define FILE_ALERT(msg,args...) FILE_PKT_DEBUG(DAQ_DEBUG_PKT_LEVEL_ALERT,msg,##args)
#define FILE_CRITICAL(msg,args...) FILE_PKT_DEBUG(DAQ_DEBUG_PKT_LEVEL_CRITICAL,msg,##args)
#define FILE_ERROR(msg,args...) FILE_PKT_DEBUG(DAQ_DEBUG_PKT_LEVEL_ERROR,msg,##args)
#define FILE_WARNING(msg,args...) FILE_PKT_DEBUG(DAQ_DEBUG_PKT_LEVEL_WARNING,msg,##args)
#define FILE_NOTICE(msg,args...) FILE_PKT_DEBUG(DAQ_DEBUG_PKT_LEVEL_NOTICE,msg,##args)
#define FILE_INFO(msg,args...) FILE_PKT_DEBUG(DAQ_DEBUG_PKT_LEVEL_INFO,msg,##args)
#define FILE_DEBUG(msg,args...) FILE_PKT_DEBUG(DAQ_DEBUG_PKT_LEVEL_DEBUG,msg,##args)

#else

#define FILE_LOG_MSGS(msg, args...) DEBUG_WRAP(DebugMessage(DEBUG_FILE, msg"\n", ##args);)
#define FILE_EMERGENCY(msg,args...) FILE_LOG_MSGS(msg,##args)
#define FILE_ALERT(msg,args...) FILE_LOG_MSGS(msg,##args)
#define FILE_CRITICAL(msg,args...) FILE_LOG_MSGS(msg,##args)
#define FILE_ERROR(msg,args...) FILE_LOG_MSGS(msg,##args)
#define FILE_WARNING(msg,args...) FILE_LOG_MSGS(msg,##args)
#define FILE_NOTICE(msg,args...) FILE_LOG_MSGS(msg,##args)
#define FILE_INFO(msg,args...) FILE_LOG_MSGS(msg,##args)
#define FILE_DEBUG(msg,args...) FILE_LOG_MSGS(msg,##args)
#define SAVE_DAQ_PKT_HDR(p)
#endif

#endif

