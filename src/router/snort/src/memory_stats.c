/*
 **
 **  memory_stats.c
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Author(s):   Puneeth Kumar C V <puneetku@cisco.com>
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>

#include "snort.h"
#include "preprocids.h"
#include "memory_stats.h"

#define STATS_SEPARATOR \
    "==============================================================================="

extern bool periodic_dump_enable;
static MemoryStatsDisplayFunc MemStatsDisplayCallback[PP_MAX] = {0};
static PreprocMemInfo preproc_mem_info[PP_MAX][PP_MEM_MAX_CATEGORY];
static const char* preproc_names[PP_MAX] = {
    "bo",
    "app_id",
    "dns",
    "frag",
    "ftptelnet",
    "httpinspect",
    "perfmonitor",
    "rpcdecode",
    "shared_rules",
    "sfportscan",
    "smtp",
    "ssh",
    "ssl",
    "stream",
    "telnet",
    "arpspoof",
    "dce",
    "sdf",
    "normalize",
    "isakmp",
    "session",
    "sip",
    "pop",
    "imap",
    "network_discovery",
    "fw_rule_engine",
    "reputation",
    "gtp",
    "modbus",
    "dnp",
    "file",
    "file_inspect",
    "nap_rule_engine",
    "prefilter_rule_engine",
    "httpmod",
    "http",
    "cip",
    "s7commplus",
};

void initMemoryStatsApi()
{
    memset(preproc_mem_info, 0,
              sizeof(PreprocMemInfo) * PP_MAX * PP_MEM_MAX_CATEGORY);
}

static int MemStatsDisplayGeneralStats(uint32_t preproc_id)
{
    LogMessage("\n");
    LogMessage("Heap Statistics of %s:\n", preproc_names[preproc_id]);
    LogMessage("          Total Statistics:\n");
    LogMessage("               Memory in use: %14zu bytes\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_SESSION].used_memory +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_CONFIG].used_memory +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MEMPOOL].used_memory +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MISC].used_memory);
    LogMessage("                No of allocs: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_SESSION].num_of_alloc +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_CONFIG].num_of_alloc +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MEMPOOL].num_of_alloc +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MISC].num_of_alloc);
    LogMessage("                 No of frees: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_SESSION].num_of_free +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_CONFIG].num_of_free +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MEMPOOL].num_of_free +
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MISC].num_of_free);
    if (preproc_mem_info[preproc_id][PP_MEM_CATEGORY_SESSION].num_of_alloc)
    {
        LogMessage("        Session Statistics:\n");
        LogMessage("               Memory in use: %14zu bytes\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_SESSION].used_memory);
        LogMessage("                No of allocs: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_SESSION].num_of_alloc);
        LogMessage("                 No of frees: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_SESSION].num_of_free);
    }
    if (preproc_mem_info[preproc_id][PP_MEM_CATEGORY_CONFIG].num_of_alloc)
    {
        LogMessage("         Config Statistics:\n");
        LogMessage("               Memory in use: %14zu bytes\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_CONFIG].used_memory);
        LogMessage("                No of allocs: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_CONFIG].num_of_alloc);
        LogMessage("                 No of frees: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_CONFIG].num_of_free);
    }
    if (preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MEMPOOL].num_of_alloc)
    {
        LogMessage("        Mempool Statistics:\n");
        LogMessage("               Memory in use: %14zu bytes\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MEMPOOL].used_memory);
        LogMessage("                No of allocs: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MEMPOOL].num_of_alloc);
        LogMessage("                 No of frees: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MEMPOOL].num_of_free);
    }
    if (preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MISC].num_of_alloc)
    {
        LogMessage("  Misc run-time Statistics:\n");
        LogMessage("               Memory in use: %14zu bytes\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MISC].used_memory);
        LogMessage("                No of allocs: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MISC].num_of_alloc);
        LogMessage("                 No of frees: %14u\n",
            preproc_mem_info[preproc_id][PP_MEM_CATEGORY_MISC].num_of_free);
    }
    return 0;
}

static bool CheckSampleInterval(time_t curr_time)
{
    static time_t last_true = 0;
  
    //Initial alignment
    if (last_true == 0)
    {
        last_true = curr_time;
        last_true -= last_true % 60 ;
    }
 
    //Dump on aligned time if possible. If not, get close and say we did.
    if (curr_time - last_true >= MEMSTATS_DUMP_INTERVAL)
    {
        curr_time -= curr_time % 60;
        last_true = curr_time;
        return true;
    }
 
    return false;
}

FILE *dump_stats_fd = NULL;

static void LogFileHeader(FILE *fd)
{
     fprintf(fd,
        "#%s","timestamp");
 /* BO preproc stats */
 /* APPID preproc stats */
    fprintf(fd,
        ",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
        "appid_total_sessions",
        "appid_current_sessions",
        "appid_session_mempool_alloc_count",
        "appid_session_heap_alloc_count",
        "appid_session_mempool_count",
        "appid_session_mempool_size",
        "appid_flowdata_mempool_count",
        "appid_flowdata_mempool_size",
        "appid_tmp_mempool_count",
        "appid_tmp_mempool_size",
        "appid_received_packets",
        "appid_processed_packets",
        "appid_ignored_packets");
    fprintf(fd,
        ",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
        "appid_session_memory",
        "appid_session_allocs",
        "appid_session_frees",
        "appid_config_memory",
        "appid_config_allocs",
        "appid_config_frees",
        "appid_misc_memory",
        "appid_misc_allocs",
        "appid_misc_frees",
        "appid_total_memory",
        "appid_total_allocs",
        "appid_total_frees");
 /* DNS preproc stats */
    fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s",
        "dns_session_memory",
        "dns_session_allocs",
        "dns_session_frees",
        "dns_config_memory",
        "dns_config_allocs",
        "dns_config_frees",
        "dns_total_memory");
 /* FRAG3 preproc stats */
    fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s,%s,%s",
        "frag3_mem_in_use",
        "prealloc_nodes_in_use",
        "frag3_session_memory",
        "frag3_session_allocs",
        "frag3_session_frees",
        "frag3_config_memory",
        "frag3_config_allocs",
        "frag3_config_frees",
        "frag3_total_memory");
 /* FTPTELNET preproc stats */
    fprintf(fd,",%s,%s,%s,%s,%s,%s,%s,%s,%s"
        ",%s,%s,%s,%s",
        "ftp_current_active_sessions",
        "ftp_max_concurrent_sessions",
        "ftp_total_data_sessions",
        "ftp_max_concurrent_data_sessions",
        "telnet_current_active_sessions",
        "telnet_max_concurrent_active_sessions",
        "ftptelnet_session_memory",
        "ftptelnet_session_allocs",
        "ftptelnet_session_frees",
        "ftptelnet_config_memory",
        "ftptelnet_config_allocs",
        "ftptelnet_config_frees",
        "ftptelnet_total_memory");
 /* HTTP Inspect preproc stats */
     fprintf(fd, ",%s,%s,%s,%s,%s,%s"
        ",%s,%s,%s"
        ",%s,%s,%s"
        ",%s,%s,%s"
        ",%s,%s,%s"
        ",%s,%s,%s"
        ",%s,%s,%s"
        ",%s,%s,%s",
        "current_active_session",
        "no_of_POST_methods_encountered",
        "no_of_GET_methods_encountered",
        "no_of_successfully_extract_post_params",
        "no_of_successfully_extract_request_params",
        "no_of_successfully_extract_response_params",
        "http_mempool_free",
        "http_mempool_used",
        "http_mempool_max",
        "mime_decode_mempool_free",
        "mime_decode_mempool_used",
        "mime_decode_mempool_max",
        "hi_gzip_mempool_free",
        "hi_gzip_mempool_used",
        "hi_gzip_mempool_max",
        "mime_log_mempool_free",
        "mime_log_mempool_used",
        "mime_log_mempool_max",
        "httpinspect_session_memory",
        "httpinspect_session_allocs",
        "httpinspect_session_frees",
        "httpinspect_config_memory",
        "httpinspect_config_allocs",
        "httpinspect_config_frees",
        "httpinspect_pool_memory",
        "httpinspect_pool_allocs",
        "httpinspect_pool_frees"
        );
 /* Perfmonitor preproc stats */
 /* RPCDecode preproc stats */
 /* Shared rules preproc stats */
 /* SFPortscan preproc stats */
 /* SMTP preproc stats */
    fprintf(fd,",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
        "smtp_total_sessions",
        "smtp_max_sessions",
        "smtp_cur_active_sessions",
        "smtp_session_memory",
        "smtp_session_allocs",
        "smtp_imapsession_frees",
        "smtp_config_memory",
        "smtp_config_allocs",
        "smtp_config_frees",
        "smtp_total_memory");
 /* SSH preproc stats */
 /* SSL preproc stats */
 /* Stream preproc stats */
     fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s,%s,%s"
         ",%s,%s,%s,%s,%s,%s,%s,%s",
         "total_sessions",
         "total_tcp_sessions",
         "active_tcp_sessions",
         "total_udp_sessions",
         "active_udp_sessions",
         "total_icmp_sessions",
         "active_icmp_sessions",
         "total_ip_sessions",
         "active_ip_sessions",
         "stream_session_memory",
         "stream_session_allocs",
         "stream_session_frees",
         "stream_config_memory",
         "stream_config_allocs",
         "stream_config_frees",
         "stream_max_mempool",
         "stream_total_memory");

 /* Telnet preproc stats */
 /* Arpspoof preproc stats */
 /* DCE preproc stats */
    fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s,%s,%s"
        ",%s,%s,%s,%s,%s,%s,%s,%s"
        ",%s,%s,%s,%s,%s,%s,%s,%s"
        ",%s,%s,%s,%s,%s,%s,%s,%s",
        "dce_total_sessions",
        "dce_active_sessions",
        "dce_total_smb_sessions",
        "dce_smb_current_total",
        "dce_smb_maximum_total",
        "dce_smb_current_session_data",
        "dce_smb_maximum_session_data",
        "dce_total_tcp_sessions",
        "dce_tcp_current_total",
        "dce_tcp_maximum_total",
        "dce_tcp_current_session_data",
        "dce_tcp_maximum_session_data",
        "dce_total_udp_sessions",
        "dce_udp_current_total",
        "dce_udp_maximum_total",
        "dce_udp_current_session_data",
        "dce_udp_maximum_session_data",
        "dce_total_http_server_sessions",
        "dce_total_http_proxy_sessions",
        "dce_http_current_total",
        "dce_http_maximum_total",
        "dce_http_current_session_data",
        "dce_http_maximum_session_data",
        "dce_session_memory",
        "dce_session_allocs",
        "dce_session_frees",
        "dce_config_memory",
        "dce_config_allocs",
        "dce_config_frees",
        "dce_misc_memory",
        "dce_misc_allocs",
        "dce_misc_frees",
        "dce_total_memory");
 /* SDF preproc stats */
 /* Normalize preproc stats */
 /* ISAKMP preproc stats */
 /* Session preproc stats */
 /* SIP preproc stats */
    fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s,%s,%s",
        "sip_total_sessions",
        "sip_cur_active_sessions",
        "sip_session_memory",
        "sip_session_allocs",
        "sip_session_frees",
        "sip_config_memory",
        "sip_config_allocs",
        "sip_config_frees",
        "sip_total_memory");
 /* POP preproc stats */
    fprintf(fd,",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
        "pop_total_sessions",
        "pop_max_sessions",
        "pop_cur_active_sessions",
        "pop_session_memory",
        "pop_session_allocs",
        "pop_imapsession_frees",
        "pop_config_memory",
        "pop_config_allocs",
        "pop_config_frees",
        "pop_total_memory");
 /* IMAP preproc stats */
    fprintf(fd,",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
        "imap_total_sessions",
        "imap_max_sessions",
        "imap_cur_active_sessions",
        "imap_session_memory",
        "imap_session_allocs",
        "imap_session_frees",
        "imap_config_memory",
        "imap_config_allocs",
        "imap_config_frees",
        "imap_total_memory");
 /* Network Discovery preproc stats */
    fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s,%s"
        ",%s,%s",
        "rna_session_memory",
        "rna_session_allocs",
        "rna_session_frees",
        "rna_config_memory",
        "rna_config_allocs",
        "rna_config_frees",
        "rna_misc_memory",
        "rna_misc_allocs",
        "rna_misc_frees",
        "rna_total_memory");
 /* FW Rule Engine preproc stats */
    fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s,%s"
        ",%s,%s,%s,%s,%s,%s,%s",
        "fw_total_sessions",
        "fw_active_sessions",
        "fw_heap_used_count",
        "fw_freelist_total_count",
        "fw_freelist_free_count",
        "fw_session_memory",
        "fw_session_allocs",
        "fw_session_frees",
        "fw_config_memory",
        "fw_config_allocs",
        "fw_config_frees",
        "fw_misc_memory",
        "fw_misc_allocs",
        "fw_misc_frees",
        "fw_total_memory");
 /* Reputation preproc stats */
 /* GTP preproc stats */
 /* Modbus preproc stats */
 /* DNP preproc stats */
 /* File preproc stats */
    fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
        "file_session_memory",
        "file_session_allocs",
        "file_session_frees",
        "file_config_memory",
        "file_config_allocs",
        "file_config_frees",
        "file_mempool_memory",
        "file_mempool_allocs",
        "file_mempool_frees",
        "file_total_memory");
#if !defined(SFLINUX) && !defined(WRLINUX)
 /* File Inspect preproc stats */
    fprintf(fd, ",%s,%s,%s,%s,%s,%s,%s",
        "file_inspect_session_memory",
        "file_inspect_session_allocs",
        "file_inspect_session_frees",
        "file_inspect_config_memory",
        "file_inspect_config_allocs",
        "file_inspect_config_frees",
        "file_inspect_total_memory");
#endif
 /* NAP Rule Engine preproc stats */
 /* Prefilter Rule Engine preproc stats */
 /* HTTPMOD preproc stats */
 /* HTTP preproc stats */
 /* CIP preproc stats */
 /* S7COMMPLUS preproc stats */
 /* End of preproc stats */
    fprintf(dump_stats_fd, "\n");
}

void dump_preproc_stats(time_t curr_time)
{
    char dump_file_path[PATH_MAX];
  
    if (snort_conf->memdump_file == NULL)
		return;

    if (CheckSampleInterval(curr_time))
    {  
        if( dump_stats_fd == NULL ) 
        {
                sprintf(dump_file_path, "%s", snort_conf->memdump_file);
                dump_stats_fd = fopen(dump_file_path, "w");
                LogFileHeader(dump_stats_fd);
        }
        fprintf(dump_stats_fd, "%lu", (unsigned long)curr_time);
        memory_stats_periodic_handler(dump_stats_fd,true);
        fprintf(dump_stats_fd, "\n");
        fflush(dump_stats_fd);
    } 
}

static inline long int preproc_stats_filesize()     
{
    long int res = 0;

    if (dump_stats_fd == NULL)
    {
        return -1;
    }
    fseek(dump_stats_fd, 0L, SEEK_END);

    res = ftell(dump_stats_fd);
    return res;
}

void rotate_preproc_stats()
{
    long int size = 0;
    char dump_file_path[PATH_MAX];
    char rotate_file_path[PATH_MAX];
    time_t ts;
    struct tm *tm;

    if (snort_conf->memdump_file == NULL)
        return;

    size = preproc_stats_filesize();

    if (size >= MIN_FILE_SIZE_TO_ROTATE)
    {
        // Get current time, then subtract one day to get yesterday
        ts = time(NULL);
        ts -= (24*60*60);
        tm = localtime(&ts);
        sprintf(rotate_file_path, "%s-%d-%02d-%02d-%lu", snort_conf->memdump_file,
                                 tm->tm_year + 1900, tm->tm_mon + 1,
                                 tm->tm_mday, ts);
        sprintf(dump_file_path,"%s", snort_conf->memdump_file);
        if (dump_stats_fd)
        {
            fclose(dump_stats_fd);
            dump_stats_fd = NULL;
        }
        rename(dump_file_path, rotate_file_path);
        dump_stats_fd = fopen(dump_file_path, "w");
        LogFileHeader(dump_stats_fd);
    }
}

void memory_stats_periodic_handler(FILE *fd, bool file_dump)
{
    uint32_t pp_id;

    for (pp_id = PP_BO; pp_id < PP_MAX; pp_id++)
    {
        if(MemStatsDisplayCallback[pp_id] != 0)
        {
            if (!file_dump)
                LogMessage("%s\n", STATS_SEPARATOR);
            MemStatsDisplayCallback[pp_id](fd, NULL, preproc_mem_info[pp_id]);
            if (!file_dump)
                MemStatsDisplayGeneralStats(pp_id);
        }
    }
}

static inline void PreprocDisplaystats(char *buffer, uint32_t preproc_id)
{
    int len = 0;
    if (!(preproc_id != PP_ALL && preproc_id >= PP_MAX))
    {
         if ( (preproc_id != PP_ALL) && (MemStatsDisplayCallback[preproc_id] != 0 && preproc_mem_info[preproc_id] != NULL) )
         {
             if (preproc_id == PP_SMTP || preproc_id == PP_POP || preproc_id == PP_IMAP)
                 len += MemStatsDisplayCallback[preproc_id](NULL, buffer, preproc_mem_info[preproc_id]);
         }
         else {
             for (preproc_id = PP_BO; preproc_id < PP_MAX; preproc_id++)
                if(MemStatsDisplayCallback[preproc_id] != 0 && preproc_mem_info[preproc_id] != NULL)
                {
                     if (preproc_id == PP_SMTP || preproc_id == PP_POP || preproc_id == PP_IMAP)
                         len += MemStatsDisplayCallback[preproc_id](NULL, buffer + len, preproc_mem_info[preproc_id]);
                }
         }
    }
    else
    {
        snprintf(buffer, CS_STATS_BUF_SIZE, "\nInvalid preprocessor.\n");   
    }
}

void MemoryPostFunction(uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    char *buffer = (char*) calloc(MEM_STATS_BUF_SIZE + 1, 1);
    uint32_t preproc_id;

    if(old_context)
    {
        preproc_id = *((uint32_t *) old_context);
        PreprocDisplaystats(buffer, preproc_id); 

        if (-1 == f(te, (const uint8_t *)buffer, strlen(buffer)))
            LogMessage("Unable to send data to the frontend\n");
    }
    free(buffer);  
}

int MemoryControlFunction(uint16_t type, void *new_context, void **old_context)
{
    if(new_context)
        *old_context = new_context;
    else    
        LogMessage("\nnew_context is NULL\n");
    return 0;
}

int MemoryPreFunction(uint16_t type, const uint8_t *data, uint32_t length,
                        void **new_context, char *statusBuf, int statusBuf_len)
{
    if(data)
        *new_context = (void*) data;
    else
         LogMessage("\ndata is NULL\n"); 
    return 0; 
}

int PPMemoryStatsDumpCfg(uint16_t type, const uint8_t *data, uint32_t length,
                      void **new_context, char* statusBuf, int statusBuf_len)
{
    if (!data)
        return 0; 

    char *args = (char*) data;

    if (0 == strncmp(args, "enable", strlen("enable")))
    {
        periodic_dump_enable = true;
    }
    else if (0 == strncmp(args, "disable", strlen("disable")))
    {
        periodic_dump_enable = false;
        if (dump_stats_fd)
        {
            fclose(dump_stats_fd);
            dump_stats_fd = NULL;
        }
    }
    return 0;
}

void PPMemoryStatsDumpShow(uint16_t type, void *old_context,
                            struct _THREAD_ELEMENT *te, ControlDataSendFunc f)
{
    char buffer[CS_STATS_BUF_SIZE + 1];
    int len = 0;

    if (periodic_dump_enable)
        len = snprintf(buffer, CS_STATS_BUF_SIZE,
                        "\nPeriodic memory stats dump is enabled.\n\n");
    else
        len = snprintf(buffer, CS_STATS_BUF_SIZE,
                        "\nPeriodic memory stats dump is disabled.\n\n");

    if (-1 == f(te, (const uint8_t *)buffer, len))
        LogMessage("Unable to send data to the frontend\n");
}

int RegisterMemoryStatsFunction(uint preproc, MemoryStatsDisplayFunc cb)
{
    if (preproc >= PP_MAX)
    {
        return -1;
    }

    MemStatsDisplayCallback[preproc] = cb;

    return 0;
}

void* SnortPreprocAlloc (int num, unsigned long size, uint32_t preproc, uint32_t sub)
{
    void *pv;

    if (sub >= PP_MEM_MAX_CATEGORY) {
        FatalError("Unable to allocate memory!  (requested %lu)\n", size);
    }

    pv = calloc(num, size);

    if ( pv )
    {
        preproc_mem_info[preproc][sub].used_memory += (num*size);
        preproc_mem_info[preproc][sub].num_of_alloc++;
    }
   
    else
    { 
        FatalError("Unable to allocate memory!  (%lu requested)\n", size);
    }

    return pv;
}

void SnortPreprocFree (void *ptr, uint32_t size, uint32_t preproc, uint32_t sub)
{
    if (sub >= PP_MEM_MAX_CATEGORY) {
        FatalError("Unable to free memory!\n");
    }

    if( ptr )
    {
        free(ptr);
        ptr = NULL;
    }

    preproc_mem_info[preproc][sub].used_memory -= size;
    preproc_mem_info[preproc][sub].num_of_free++;

}
