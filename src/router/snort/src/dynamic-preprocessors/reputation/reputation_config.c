/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 * Provides convenience functions for parsing and querying configuration.
 *
 * 6/7/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include "sf_snort_packet.h"
#include "sf_types.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "reputation_config.h"
#include "spp_reputation.h"
#include "reputation_debug.h"
#include "reputation_utils.h"
#ifdef SHARED_REP
#include "./shmem/shmem_mgmt.h"
#include <sys/stat.h>
#endif
enum
{
    IP_INSERT_SUCCESS = 0,
    IP_INVALID,
    IP_INSERT_FAILURE,
    IP_INSERT_DUPLICATE,
    IP_MEM_ALLOC_FAILURE
};


/*
 * Default values for configurable parameters.
 */
#define REPUTATION_DEFAULT_MEMCAP               500 /*Mega bytes*/
#define REPUTATION_DEFAULT_REFRESH_PERIOD       60  /*60 seconds*/
#define REPUTATION_DEFAULT_SHARED_MEM_INSTANCES 50


/*
 * Min/Max values for each configurable parameter.
 */
#define MIN_MEMCAP 1
#define MAX_MEMCAP 4095
#define MIN_SHARED_MEM_REFRESH_PERIOD 1
#define MAX_SHARED_MEM_REFRESH_PERIOD UINT32_MAX
#define MIN_SHARED_MEM_INSTANCES    2
#define MAX_SHARED_MEM_INSTANCES    256

#define MAX_ADDR_LINE_LENGTH    8192

/*
 * Keyword strings for parsing configuration options.
 */
#define REPUTATION_MEMCAP_KEYWORD               "memcap"
#define REPUTATION_SCANLOCAL_KEYWORD            "scan_local"
#define REPUTATION_BLACKLIST_KEYWORD            "blacklist"
#define REPUTATION_WHITELIST_KEYWORD            "whitelist"
#define REPUTATION_MONITORLIST_KEYWORD          "monitorlist"
#define REPUTATION_PRIORITY_KEYWORD             "priority"
#define REPUTATION_NESTEDIP_KEYWORD             "nested_ip"
#define REPUTATION_SHARED_MEM_KEYWORD           "shared_mem"
#define REPUTATION_SHARED_REFRESH_KEYWORD       "shared_refresh"
#define REPUTATION_SHARED_MAX_INSTANCES_KEYWORD "shared_max_instances"
#define REPUTATION_WHITEACTION_KEYWORD          "white"

#define REPUTATION_CONFIG_SECTION_SEPERATORS     ",;"
#define REPUTATION_CONFIG_VALUE_SEPERATORS       " "
#define REPUTATION_SEPARATORS                " \t\r\n"

static char *black_info = REPUTATION_BLACKLIST_KEYWORD;
static char *white_info = REPUTATION_WHITELIST_KEYWORD;
static char *monitor_info = REPUTATION_MONITORLIST_KEYWORD;

char* NestedIPKeyword[] =
{
        "inner",
        "outer",
        "both",
        NULL
};

char* WhiteActionOption[] =
{
        "unblack",
        "trust",
        NULL
};


#define MAX_MSGS_TO_PRINT      20

static unsigned long total_duplicates;
static unsigned long total_invalids;

void **IPtables;

#ifdef SHARED_REP
ReputationConfig *reputation_shmem_config;
table_flat_t *emptyIPtables;
#endif
/*
 * Function prototype(s)
 */
static void IpListInit(uint32_t,ReputationConfig *config);
static void LoadListFile(char *filename, INFO info, ReputationConfig *config);
static void DisplayIPlistStats(ReputationConfig *);
static void DisplayReputationConfig(ReputationConfig *);

/* ********************************************************************
 * Function: estimateSizeFromEntries
 *
 * Estimate the memory segment size based on number of entries and memcap.
 *
 * Arguments:
 *
 * uint32_t num_entries: number of entries.
 * uint32_t the memcap value set in configuration
 *
 * RETURNS: estimated memory size.
 *********************************************************************/
uint32_t estimateSizeFromEntries(uint32_t num_entries, uint32_t memcap)
{
    uint64_t size;
    uint64_t sizeFromEntries;

    /*memcap value is in Megabytes*/
    size = (uint64_t)memcap << 20;

    if (size > UINT32_MAX)
        size = UINT32_MAX;

    /*Worst case,  15k ~ 2^14 per entry, plus one Megabytes for empty table*/
    if (num_entries > ((UINT32_MAX - (1 << 20))>> 15))
        sizeFromEntries = UINT32_MAX;
    else
        sizeFromEntries = (num_entries  << 15) + (1 << 20);

    if (size > sizeFromEntries)
    {
        size = sizeFromEntries;
    }

    return (uint32_t) size;
}
#ifdef SHARED_REP
/****************************************************************************
 *
 * Function: CheckIPlistDir()
 *
 * Purpose: We only check if IP list directory exist and
 *          readable
 * Arguments: None.
 *
 * Returns:
 *   0 : fail
 *   1 : success
 *
 ****************************************************************************/
static int CheckIPlistDir(char *path)
{
    struct stat st;

    if (path == NULL)
        return 0;

    if (stat(path, &st) == -1)
        return 0;

    if (!S_ISDIR(st.st_mode) || (access(path, R_OK) == -1))
    {
        return 0;
    }
    return 1;
}

/* ********************************************************************
 * Function: LoadFileIntoShmem
 *
 * Call back function for shared memory
 * This is called when new files in the list
 * Arguments:
 *
 * void* ptrSegment: start of shared memory segment.
 * ShmemDataFileList** file_list: the list of whitelist/blacklist files
 * int num_files: number of files
 *
 * RETURNS:
 *     0: success
 *     other value fails
 *********************************************************************/

int LoadFileIntoShmem(void* ptrSegment, ShmemDataFileList** file_list, int num_files)
{
    table_flat_t *table;
    int i;
    MEM_OFFSET list_ptr;
    ListInfo *listInfo;
    uint8_t *base;

    if (num_files > MAX_IPLIST_FILES)
    {
        _dpd.logMsg("Reputation preprocessor: Too many IP list files. "
                "The maximum is: %d, current is: %d.\n",
                 MAX_IPLIST_FILES, num_files);
        num_files = MAX_IPLIST_FILES;
    }

    segment_meminit((uint8_t*)ptrSegment, reputation_shmem_config->memsize);

    /*DIR_16x7_4x4 for performance, but memory usage is high
     *Use  DIR_8x16 worst case IPV4 5K, IPV6 15K (bytes)
     *Use  DIR_16x7_4x4 worst case IPV4 500, IPV6 2.5M
     */
    table = sfrt_flat_new(DIR_8x16, IPv6, reputation_shmem_config->numEntries, reputation_shmem_config->memcap);
    if (table == NULL)
    {
        _dpd.errMsg("Reputation preprocessor: Failed to create IP list.\n");
        return -1;
    }

    reputation_shmem_config->iplist = table;
    base = (uint8_t *)ptrSegment;

    /*Copy the list information table to shared memory block*/
    list_ptr = segment_calloc(num_files, sizeof(ListInfo));

    if (list_ptr == 0)
    {
        _dpd.errMsg("Reputation preprocessor:: Failed to create IP list table.\n");
        return -1;
    }

    listInfo = (ListInfo *)&base[list_ptr];
    table->list_info = list_ptr;

    reputation_shmem_config->listInfo = listInfo;

    reputation_shmem_config->memCapReached = false;

    /*Reset the log message count*/
    total_duplicates = 0;
    for (i = 0; i < num_files; i++)
    {
        listInfo[i].listIndex = (uint8_t)i + 1;
        listInfo[i].listType = (uint8_t)file_list[i]->filetype;
        listInfo[i].listId = file_list[i]->listid;
        memcpy(listInfo[i].zones, file_list[i]->zones, MAX_NUM_ZONES);
        LoadListFile(file_list[i]->filename, list_ptr, reputation_shmem_config);
        list_ptr += sizeof(ListInfo);

    }

    _dpd.logMsg("Reputation Preprocessor shared memory summary:\n");
    DisplayIPlistStats(reputation_shmem_config);
    return 0;
}

/* ********************************************************************
 * Function: GetSegmentSizeFromFileList
 *
 * Call back function for shared memory
 * This is called when new files in the list
 *
 * Arguments:
 *
 * ShmemDataFileList** file_list: the list of whitelist/blacklist files
 * int num_files: number of files
 *
 * RETURNS:
 *    uint32_t: segment size
 *********************************************************************/
uint32_t GetSegmentSizeFromFileList(ShmemDataFileList** file_list, int file_count)
{
    int numlines;
    int totalLines = 0;
    int i;

    if (file_count == 0)
    {
        return ZEROSEG;
    }
    for (i = 0; i < file_count; i++)
    {
        errno = 0;
        numlines = numLinesInFile(file_list[i]->filename);
        if ((0 == numlines) && (0 != errno))
        {
            char errBuf[STD_BUF];
#ifdef WIN32
            snprintf(errBuf, STD_BUF, "%s", strerror(errno));
#else
            strerror_r(errno, errBuf, STD_BUF);
#endif
            DynamicPreprocessorFatalMessage( "Unable to open address file %s, Error: %s\n",
                    file_list[i]->filename, errBuf);
        }

        if (totalLines + numlines < totalLines)
        {
            DynamicPreprocessorFatalMessage("Too many entries.\n");
        }

        totalLines += numlines;
    }

    if (totalLines == 0)
    {
        return ZEROSEG;
    }
    reputation_shmem_config->numEntries = totalLines + 1;

    reputation_shmem_config->memsize =  estimateSizeFromEntries(reputation_shmem_config->numEntries, reputation_shmem_config->memcap);
    return reputation_shmem_config->memsize;
}

/* ********************************************************************
 * Function: InitPerProcessZeroSegment
 *
 * Call back function for shared memory
 * This is called during initialization
 *
 * Arguments:
 *
 * void*** data_ptr: (output) the address of shared memory address
 *
 * RETURNS:
 *    uint32_t: segment size
 *********************************************************************/
int InitPerProcessZeroSegment(void*** data_ptr)
{
    /*The size of empty segment is 1 Megabytes*/
    size_t size = 1;
    long maxEntries = 1;
    static bool initiated = false;

    if (true == initiated)
    {
        *data_ptr = (void **)&emptyIPtables;
        return 0;
    }
    reputation_shmem_config->emptySegment = malloc(size*1024*1024);
    if (reputation_shmem_config->emptySegment == NULL)
    {
        DynamicPreprocessorFatalMessage(
        "Failed to allocate memory for empty segment.\n");
    }

    segment_meminit((uint8_t*) reputation_shmem_config->emptySegment, size*1024*1024);

    initiated = true;

    /*DIR_16x7_4x4 for performance, but memory usage is high
     *Use  DIR_8x16 worst case IPV4 5K, IPV6 15K (bytes)
     *Use  DIR_16x7_4x4 worst case IPV4 500, IPV6 2.5M
     */
    emptyIPtables = sfrt_flat_new(DIR_8x16, IPv6, maxEntries, size);
    if (emptyIPtables == NULL)
    {
        DynamicPreprocessorFatalMessage("Reputation preprocessor: Failed to create IP list.\n");
    }

    *data_ptr = (void **)&emptyIPtables;

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "    Total memory "
            "allocated for empty table: %d bytes\n",
            sfrt_flat_usage(emptyIPtables)););
    return 0;
}

/* ********************************************************************
 * Function: initShareMemory
 *
 * Initialize for shared memory
 * This is called during initialization
 *
 * Arguments:
 *
 * ReputationConfig *config: the configure file
 *
 * RETURNS:
 *    1: success
 *********************************************************************/
void initShareMemory(struct _SnortConfig *sc, void *conf)
{
    uint32_t snortID;
    ReputationConfig *config = (ReputationConfig *)conf;

    switch_state = SWITCHING;
    reputation_shmem_config = config;
    if (InitShmemDataMgmtFunctions(InitPerProcessZeroSegment,
            GetSegmentSizeFromFileList,LoadFileIntoShmem))
    {
        DynamicPreprocessorFatalMessage("Unable to initialize DataManagement functions\n");
    }
    /*use snort instance ID to designate server (writer)*/
    snortID = _dpd.getSnortInstance();
    if (SHMEM_SERVER_ID == snortID)
    {
        if ((available_segment = InitShmemWriter(snortID, IPREP, GROUP_0, NUMA_0,
                config->sharedMem.path, &IPtables, config->sharedMem.updateInterval,
                config->sharedMem.maxInstances)) == NO_ZEROSEG)
        {
            DynamicPreprocessorFatalMessage("Unable to init share memory writer\n");
        }
        switch_state = SWITCHED;
    }
    else
    {
        if ((available_segment = InitShmemReader(snortID, IPREP,GROUP_0, NUMA_0,
                config->sharedMem.path, &IPtables, config->sharedMem.updateInterval,
                config->sharedMem.maxInstances)) == NO_ZEROSEG)
        {
            DynamicPreprocessorFatalMessage("Unable to init share memory reader\n");
        }
        switch_state = SWITCHED;
    }
    SetupReputationUpdate(config->sharedMem.updateInterval);
}
#endif
/* ********************************************************************
 * Function: DisplayIPlistStats
 *
 * Display the statistics for the Reputation iplist table.
 *
 * Arguments:
 *
 * ReputationConfig *config: Reputation preprocessor configuration.
 *
 * RETURNS: Nothing.
 *********************************************************************/
static void DisplayIPlistStats(ReputationConfig *config)
{
    /*Print out the summary*/
    reputation_stats.memoryAllocated = sfrt_flat_usage(config->iplist);
    _dpd.logMsg("    Reputation total memory usage: %u bytes\n",
            reputation_stats.memoryAllocated);
    config->numEntries = sfrt_flat_num_entries(config->iplist);
    _dpd.logMsg("    Reputation total entries loaded: %u, invalid: %u, re-defined: %u\n",
            config->numEntries,total_invalids,total_duplicates);
}
/* ********************************************************************
 * Function: DisplayReputationConfig
 *
 * Display the configuration for the Reputation preprocessor.
 *
 * Arguments:
 *
 * ReputationConfig *config: Reputation preprocessor configuration.
 *
 * RETURNS: Nothing.
 *********************************************************************/
static void DisplayReputationConfig(ReputationConfig *config)
{

    if (config == NULL)
        return;

    _dpd.logMsg("    Memcap: %d %s \n",
            config->memcap,
            config->memcap == REPUTATION_DEFAULT_MEMCAP ? "(Default) M bytes" : "M bytes" );
    _dpd.logMsg("    Scan local network: %s\n",
            config->scanlocal ? "ENABLED":"DISABLED (Default)");
    _dpd.logMsg("    Reputation priority:  %s \n",
            ((config->priority ==  WHITELISTED_TRUST) || (config->priority ==  WHITELISTED_UNBLACK))?
                    REPUTATION_WHITELIST_KEYWORD "(Default)" : REPUTATION_BLACKLIST_KEYWORD );
    _dpd.logMsg("    Nested IP: %s %s \n",
            NestedIPKeyword[config->nestedIP],
            config->nestedIP ==  INNER? "(Default)" : "" );
    _dpd.logMsg("    White action: %s %s \n",
            WhiteActionOption[config->whiteAction],
            config->whiteAction ==  UNBLACK? "(Default)" : "" );
    if (config->sharedMem.path)
    {
        _dpd.logMsg("    Shared memory supported, Update directory: %s\n",
                config->sharedMem.path );
        _dpd.logMsg("    Shared memory refresh period: %d %s \n",
                config->sharedMem.updateInterval,
                config->sharedMem.updateInterval == REPUTATION_DEFAULT_REFRESH_PERIOD ?
                        "(Default) seconds" : "seconds" );
        _dpd.logMsg("    Shared memory max instances: %u\n", config->sharedMem.maxInstances);
    }
    else
    {
        _dpd.logMsg("    Shared memory is Not supported.\n");

    }
    _dpd.logMsg("\n");
}



/********************************************************************
 * Function: IpListInit
 *
 * Initiate an iplist table
 *
 * Arguments:
 *  Reputation_Config *
 *      The configuration to use.
 *
 * Returns: None
 *
 ********************************************************************/

static void IpListInit(uint32_t maxEntries, ReputationConfig *config)
{
    uint8_t *base;
    ListInfo *whiteInfo;
    ListInfo *blackInfo;
    MEM_OFFSET list_ptr;

    if (config->iplist == NULL)
    {
        uint32_t mem_size;
        mem_size = estimateSizeFromEntries(maxEntries, config->memcap);
        config->localSegment = malloc(mem_size);
        if (config->localSegment == NULL)
        {
            DynamicPreprocessorFatalMessage(
            "Failed to allocate memory for local segment\n");
        }

        segment_meminit((uint8_t*)config->localSegment,mem_size);
        base = (uint8_t *)config->localSegment;

        /*DIR_16x7_4x4 for performance, but memory usage is high
         *Use  DIR_8x16 worst case IPV4 5K, IPV6 15K (bytes)
         *Use  DIR_16x7_4x4 worst case IPV4 500, IPV6 2.5M
         */
        config->iplist = sfrt_flat_new(DIR_8x16, IPv6, maxEntries, config->memcap);
        if (config->iplist == NULL ||
            !(list_ptr = segment_calloc((size_t)DECISION_MAX, sizeof(ListInfo))))
        {
            DynamicPreprocessorFatalMessage("%s(%d): Failed to create IP list.\n", *(_dpd.config_file), *(_dpd.config_line));
        }
        config->iplist->list_info = list_ptr;

        config->local_black_ptr = list_ptr + BLACKLISTED * sizeof(ListInfo);
        blackInfo = (ListInfo *)&base[config->local_black_ptr];
        blackInfo->listType = BLACKLISTED;
        blackInfo->listIndex = BLACKLISTED + 1;
#ifdef SHARED_REP
        memset(blackInfo->zones, true, MAX_NUM_ZONES);
#endif
        if (UNBLACK == config->whiteAction)
        {
            config->local_white_ptr = list_ptr + WHITELISTED_UNBLACK * sizeof(ListInfo);
            whiteInfo = (ListInfo *)&base[config->local_white_ptr];
            whiteInfo->listType = WHITELISTED_UNBLACK;
            whiteInfo->listIndex = WHITELISTED_UNBLACK + 1;
#ifdef SHARED_REP
        memset(whiteInfo->zones, true, MAX_NUM_ZONES);
#endif
        }
        else
        {
            config->local_white_ptr = list_ptr + WHITELISTED_TRUST * sizeof(ListInfo);
            whiteInfo = (ListInfo *)&base[config->local_white_ptr];
            whiteInfo->listType = WHITELISTED_TRUST;
            whiteInfo->listIndex = WHITELISTED_TRUST + 1;
#ifdef SHARED_REP
        memset(whiteInfo->zones, true, MAX_NUM_ZONES);
#endif
        }
    }
}


/*********************************************************************
 *
 * Get the last index in the IP repuation information
 *
 * Arguments:
 *
 * IPrepInfo *repInfo: IP reputation information
 * uint8_t *base: the base pointer in shared memory
 *
 * RETURNS:
 *     int *lastIndex: the last index
 *    IPrepInfo *: the last IP info that stores the last index
 *                NULL if not found
 *********************************************************************/

static inline IPrepInfo * getLastIndex(IPrepInfo *repInfo, uint8_t *base, int *lastIndex)
{
    int i;

    assert(repInfo);

    /* Move to the end of current info*/
    while (repInfo->next)
    {
        repInfo =  (IPrepInfo *)&base[repInfo->next];
    }

    for (i = 0; i < NUM_INDEX_PER_ENTRY; i++)
    {
        if (!repInfo->listIndexes[i])
            break;
    }

    if (i > 0)
    {
        *lastIndex = i-1;
        return repInfo;
    }
    else
    {
        return NULL;
    }
}

/*********************************************************************
 *
 * Duplicate IP reputation information
 *
 * Arguments:
 *
 * IPrepInfo *currentInfo: IP reputation information copy from
 * IPrepInfo *destInfo: IP reputation information copy to
 * uint8_t *base: the base pointer in shared memory
 *
 * RETURNS:
 *     number of bytes duplicated
 *     -1 if out of memory
 *
 *********************************************************************/

static inline int duplicateInfo(IPrepInfo *destInfo,IPrepInfo *currentInfo,
         uint8_t *base)
{
    int bytesAllocated = 0;

    while (currentInfo)
    {
        INFO nextInfo;
        *destInfo = *currentInfo;
        if (!currentInfo->next)
            break;
        nextInfo = segment_calloc(1,sizeof(IPrepInfo));
        if (!nextInfo)
        {
            destInfo->next = 0;
            return -1;
        }
        else
        {
            destInfo->next = nextInfo;
        }
        bytesAllocated += sizeof(IPrepInfo);
        currentInfo =  (IPrepInfo *)&base[currentInfo->next];
        destInfo =  (IPrepInfo *)&base[nextInfo];
    }

    return bytesAllocated;
}
/*********************************************************************
 * New information for the same IP will be appended to the current
 *
 * If current information is empty (0), new information will be created.
 *
 * Arguments:
 *
 * INFO *current: (address to the location of) current IP reputation information
 * INFO new: (location of) new IP reputation information
 * uint8_t *base: the base pointer in shared memory
 * SaveDest saveDest: whether to update reputation at current location
 *                    or update at new location
 *
 * Returns: number of bytes allocated, -1 if out of memory
 *
 *********************************************************************/
static int64_t updateEntryInfo (INFO *current, INFO new, SaveDest saveDest, uint8_t *base)
{
    IPrepInfo *currentInfo;
    IPrepInfo *newInfo;
    IPrepInfo *destInfo;
    IPrepInfo *lastInfo;
    int64_t bytesAllocated = 0;
    int i;
    char newIndex;

    if(!(*current))
    {
        /* Copy the data to segment memory*/
        *current = segment_calloc(1,sizeof(IPrepInfo));
        if (!(*current))
        {
            return -1;
        }
        bytesAllocated = sizeof(IPrepInfo);
    }

    if (*current == new)
        return bytesAllocated;

    currentInfo = (IPrepInfo *)&base[*current];
    newInfo = (IPrepInfo *)&base[new];

    /*The latest information is always the last entry
     */
    lastInfo = getLastIndex(newInfo, base, &i);

    if (!lastInfo)
    {
        return bytesAllocated;
    }
    newIndex = lastInfo->listIndexes[i++];

    DEBUG_WRAP( DebugMessage(DEBUG_REPUTATION, "Current IP reputation information: \n"););
    DEBUG_WRAP(ReputationPrintRepInfo(currentInfo, base););
    DEBUG_WRAP( DebugMessage(DEBUG_REPUTATION, "New IP reputation information: \n"););
    DEBUG_WRAP(ReputationPrintRepInfo(newInfo, base););

    if (SAVE_TO_NEW == saveDest)
    {
        int bytesDuplicated;

        /* When updating new entry, current information should be reserved
         * because current information is inherited from parent
         */
        if ((bytesDuplicated = duplicateInfo(newInfo, currentInfo, base)) < 0)
            return -1;
        else
            bytesAllocated += bytesDuplicated;

        destInfo = newInfo;
    }
    else
    {
        destInfo = currentInfo;
    }

    /* Add the new list information to the end
     * This way, the order of list information is preserved.
     * The first one always has the highest priority,
     * because it is checked first during lookup.
     */

    while (destInfo->next)
    {
        destInfo =  (IPrepInfo *)&base[destInfo->next];
    }

    for (i = 0; i < NUM_INDEX_PER_ENTRY; i++)
    {
        if (!destInfo->listIndexes[i])
            break;
        else if (destInfo->listIndexes[i] == newIndex)
        {
            DEBUG_WRAP( DebugMessage(DEBUG_REPUTATION, "Final IP reputation information: \n"););
            DEBUG_WRAP(ReputationPrintRepInfo(destInfo, base););
            return bytesAllocated;
        }

    }

    if (i < NUM_INDEX_PER_ENTRY)
    {
        destInfo->listIndexes[i] = newIndex;
    }
    else
    {
        IPrepInfo *nextInfo;
        MEM_OFFSET ipInfo_ptr = segment_calloc(1,sizeof(IPrepInfo));
        if (!ipInfo_ptr)
            return -1;
        destInfo->next = ipInfo_ptr;
        nextInfo = (IPrepInfo *)&base[destInfo->next];
        nextInfo->listIndexes[0] = newIndex;
        bytesAllocated += sizeof(IPrepInfo);
    }

    DEBUG_WRAP( DebugMessage(DEBUG_REPUTATION, "Final IP reputation information: \n"););
    DEBUG_WRAP(ReputationPrintRepInfo(destInfo, base););

    return bytesAllocated;
}
/********************************************************************
 * Function: AddIPtoList
 *
 * Add ip address to config file
 *
 * Arguments:
 *  sfcidr_t *: ip address
 *  void *: information about the file.
 *  ReputationConfig *:      The configuration to be update.
 *
 * Returns:
 *  IP_INSERT_SUCCESS=0,
 *  IP_INSERT_FAILURE,
 *  IP_INSERT_DUPLICATE
 *
 ********************************************************************/

static int AddIPtoList(sfcidr_t *ipAddr,INFO ipInfo_ptr, ReputationConfig *config)
{
    int iRet;
    int iFinalRet = IP_INSERT_SUCCESS;
    /*This variable is used to check whether a more generic address
     * overrides specific address
     */
    uint32_t usageBeforeAdd;
    uint32_t usageAfterAdd;

#ifdef DEBUG_MSGS
    if (NULL != sfrt_flat_lookup(&ipAddr->addr, config->iplist))
    {
        DebugMessage(DEBUG_REPUTATION,
                "Find address before insert: %s\n", sfip_to_str(&ipAddr->addr) );

    }
    else
    {
        DebugMessage(DEBUG_REPUTATION,
                "Can't find address before insert: %s\n", sfip_to_str(&ipAddr->addr) );
    }
#endif

    usageBeforeAdd =  sfrt_flat_usage(config->iplist);

    /*Check whether the same or more generic address is already in the table*/
    if (NULL != sfrt_flat_lookup(&ipAddr->addr, config->iplist))
    {
        iFinalRet = IP_INSERT_DUPLICATE;
    }

    iRet = sfrt_flat_insert(ipAddr, (unsigned char)ipAddr->bits, ipInfo_ptr, RT_FAVOR_ALL, config->iplist, &updateEntryInfo);
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Unused memory: %d \n",segment_unusedmem()););


    if (RT_SUCCESS == iRet)
    {
#ifdef DEBUG_MSGS
        IPrepInfo * result;
        DebugMessage(DEBUG_REPUTATION, "Number of entries input: %d, in table: %d \n",
                totalNumEntries,sfrt_flat_num_entries(config->iplist) );
        DebugMessage(DEBUG_REPUTATION, "Memory allocated: %d \n",sfrt_flat_usage(config->iplist) );
        result = sfrt_flat_lookup(&ipAddr->addr, config->iplist);
        if (NULL != result)
        {
            DebugMessage(DEBUG_REPUTATION, "Find address after insert: %s \n",sfip_to_str(&ipAddr->addr) );
            DEBUG_WRAP(ReputationPrintRepInfo(result, (uint8_t *)config->iplist););
        }
#endif
        totalNumEntries++;
    }
    else if (MEM_ALLOC_FAILURE == iRet)
    {
        iFinalRet = IP_MEM_ALLOC_FAILURE;
        DEBUG_WRAP( DebugMessage(DEBUG_REPUTATION, "Insert error: %d for address: %s \n",iRet, sfip_to_str(&ipAddr->addr) ););
    }
    else
    {
        iFinalRet = IP_INSERT_FAILURE;
        DEBUG_WRAP( DebugMessage(DEBUG_REPUTATION, "Insert error: %d for address: %s \n",iRet, sfip_to_str(&ipAddr->addr) ););

    }

    usageAfterAdd = sfrt_flat_usage(config->iplist);
    /*Compare in the same scale*/
    if (usageAfterAdd  > (config->memcap << 20))
    {
        iFinalRet = IP_MEM_ALLOC_FAILURE;
    }
    /*Check whether there a more specific address will be overridden*/
    if (usageBeforeAdd > usageAfterAdd )
    {
        iFinalRet = IP_INSERT_DUPLICATE;
    }

    return iFinalRet;

}

static int snort_pton__address( char const *src, sfcidr_t *dest )
{
    unsigned char _temp[sizeof(struct in6_addr)];

    if ( inet_pton(AF_INET, src, _temp) == 1 )
    {
        sfip_set_raw(&dest->addr, _temp, AF_INET);
    }
    else if ( inet_pton(AF_INET6, src, _temp) == 1 )
    {
        sfip_set_raw(&dest->addr, _temp, AF_INET6);
    }
    else
    {
        return 0;
    }
    dest->bits = 128;

    return 1;
}

#define isident(x) (isxdigit((x)) || (x) == ':' || (x) == '.')
static int snort_pton( char const * src, sfcidr_t * dest )
{
    char ipbuf[INET6_ADDRSTRLEN];
    char cidrbuf[sizeof("128")];
    char *out = ipbuf;
    enum {
        BEGIN, IP, CIDR1, CIDR2, END, INVALID
    } state;

    memset(ipbuf, '\0', sizeof(ipbuf));
    memset(cidrbuf, '\0', sizeof(cidrbuf));

    state = BEGIN;

    while ( *src )
    {
        char ch = *src;

        //printf("State:%d; C:%x; P:%p\n", state, ch, src );
        src += 1;

        switch ( state )
        {
        // Scan for beginning of IP address
        case BEGIN:
            if ( isident((int)ch) )
            {
                // Set the first ipbuff byte and change state
                *out++ = ch;
                state = IP;
            }
            else if ( !isspace((int)ch) )
            {
                state = INVALID;
            }
            break;

        // Fill in ipbuf with ip identifier characters
        // Move to CIDR1 if a cidr divider (i.e., '/') is found.
        case IP:
            if ( isident((int)ch) && (out - ipbuf + 1) < (int)sizeof(ipbuf) )
            {
                *out++ = ch;
            }
            else if ( ch == '/' )
            {
                state = CIDR1;
            }
            else if ( isspace((int)ch) )
            {
                state = END;
            }
            else
            {
                state = INVALID;
            }
            break;

        // First cidr digit
        case CIDR1:
            if ( !isdigit((int)ch) )
            {
                state = INVALID;
            }
            else
            {
                // Set output to the cidrbuf buffer
                out = cidrbuf;
                *out++ = ch;
                state = CIDR2;
            }
            break;

        // Consume any addition digits for cidrbuf
        case CIDR2:
            if ( isdigit((int)ch) && (out - cidrbuf + 1) < (int)sizeof(cidrbuf) )
            {
                *out++ = ch;
            }
            else if ( isspace((int)ch) )
            {
                state = END;
            }
            else
            {
                state = INVALID;
            }
            break;

        // Scan for junk at the EOL
        case END:
            if ( !isspace((int)ch) )
            {
                state = INVALID;
            }
            break;

        // Can't get here
        default:
            break;
        }

        if ( state == INVALID )
            return -1;
    }

    if ( snort_pton__address(ipbuf, dest) < 1 )
        return 0;

    if ( *cidrbuf )
    {
        char *end;
        int value = strtol(cidrbuf, &end, 10);

        if ( value > dest->bits || value <= 0 || errno == ERANGE )
            return 0;

        if (sfaddr_family(&dest->addr) == AF_INET && value <= 32)
            dest->bits = value + 96;
        else
            dest->bits = value;
    }

    return 1;
}

/********************************************************************
 * Function:
 *
 * Load one IP list file
 *
 * Arguments:
 *  char *: the line to be processed
 *  void *: information about the file.
 *  ReputationConfig *:      The configuration to be update.
 *
 * Returns:
 *  IP_INSERT_SUCCESS,
 *  IP_INSERT_FAILURE,
 *  IP_INSERT_DUPLICATE
 *
 ********************************************************************/
static int ProcessLine(char *line, INFO info, ReputationConfig *config)
{
    sfcidr_t address;

    if ( !line || *line == '\0' )
        return IP_INSERT_SUCCESS;

    if ( snort_pton(line, &address) < 1 )
        return IP_INVALID;

    return AddIPtoList(&address, info, config);
}

/********************************************************************
 * Function: UpdatePathToFile
 *
 * Update the path to a file, if using relative path.
 * The relative path is based on config file directory.
 *
 * Arguments:
 *  full_path_filename: file name string
 *  max_size: ?
 *  char *filename: ?
 *
 * Returns:
 *  1 successful
 *  0 fail
 *
 ********************************************************************/

static int UpdatePathToFile(char *full_path_filename, unsigned int max_size, char *filename)
{

    char *snort_conf_dir = *(_dpd.snort_conf_dir);

    if (!snort_conf_dir || !(*snort_conf_dir) || !full_path_filename || !filename)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => can't create path.\n",
                *(_dpd.config_file), *(_dpd.config_line));
        return 0;
    }
    /*filename is too long*/
    if ( max_size < strlen(filename) )
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => the file name length %u is longer than allowed %u.\n",
                *(_dpd.config_file), *(_dpd.config_line), strlen(filename), max_size);
        return 0;
    }

    /*
     *  If an absolute path is specified, then use that.
     */
#ifndef WIN32
    if(filename[0] == '/')
    {
        snprintf(full_path_filename, max_size, "%s", filename);
    }
    else
    {
        /*
         * Set up the file name directory.
         */
        if (snort_conf_dir[strlen(snort_conf_dir) - 1] == '/')
        {
            snprintf(full_path_filename,max_size,
                    "%s%s", snort_conf_dir, filename);
        }
        else
        {
            snprintf(full_path_filename, max_size,
                    "%s/%s", snort_conf_dir, filename);
        }
    }
#else
    if(strlen(filename)>3 && filename[1]==':' && filename[2]=='\\')
    {
        snprintf(full_path_filename, max_size, "%s", filename);
    }
    else
    {
        /*
         **  Set up the file name directory
         */
        if (snort_conf_dir[strlen(snort_conf_dir) - 1] == '\\' ||
                snort_conf_dir[strlen(snort_conf_dir) - 1] == '/' )
        {
            snprintf(full_path_filename,max_size,
                    "%s%s", snort_conf_dir, filename);
        }
        else
        {
            snprintf(full_path_filename, max_size,
                    "%s\\%s", snort_conf_dir, filename);
        }
    }
#endif
    return 1;
}

/********************************************************************
 * Function: GetListInfo
 *
 * Get information about the file
 *
 * Arguments:
 *
 *  info: information about the file.
 *
 * Returns:
 *  None
 *
 ********************************************************************/

static char* GetListInfo(INFO info)
{
    uint8_t *base;
    ListInfo *info_value;
    base = (uint8_t *)segment_basePtr();
    info_value = (ListInfo *)(&base[info]);

    switch(info_value->listType)
    {
    case DECISION_NULL:
        return NULL;
        break;
    case BLACKLISTED:
        return black_info;
        break;
    case WHITELISTED_UNBLACK:
        return white_info;
        break;
    case MONITORED:
        return monitor_info;
        break;
    case WHITELISTED_TRUST:
        return white_info;
        break;
    default:
        return NULL;
    }
    return NULL;
}
/********************************************************************
 * Function: LoadListFile
 *
 * Load one IP list file
 *
 * Arguments:
 *  filename: file name string
 *  info: information about the file.
 *  ReputationConfig *:  The configuration to be update.
 *
 * Returns:
 *  None
 *
 ********************************************************************/

static void LoadListFile(char *filename, INFO info, ReputationConfig *config)
{

    char linebuf[MAX_ADDR_LINE_LENGTH];
    char full_path_filename[PATH_MAX+1];
    int addrline = 0;
    FILE *fp = NULL;
    char *cmt = NULL;
    char *list_info;
    ListInfo *listInfo;
    IPrepInfo *ipInfo;
    MEM_OFFSET ipInfo_ptr;
    uint8_t *base;

    /*entries processing statistics*/
    unsigned int duplicate_count = 0; /*number of duplicates in this file*/
    unsigned int invalid_count = 0;   /*number of invalid entries in this file*/
    unsigned int fail_count = 0;   /*number of invalid entries in this file*/
    unsigned int num_loaded_before = 0;     /*number of valid entries loaded */

    if ((NULL == filename)||(0 == info)|| (NULL == config)||config->memCapReached)
        return;

    UpdatePathToFile(full_path_filename, PATH_MAX, filename);

    list_info = GetListInfo(info);

    if (!list_info)
        return;

    /*convert list info to ip entry info*/
    ipInfo_ptr = segment_calloc(1,sizeof(IPrepInfo));
    if (!(ipInfo_ptr))
    {
        return;
    }
    base = (uint8_t*)config->iplist;
    ipInfo = ((IPrepInfo *)&base[ipInfo_ptr]);
    listInfo = ((ListInfo *)&base[info]);
    ipInfo->listIndexes[0] = listInfo->listIndex;


    _dpd.logMsg("    Processing %s file %s\n", list_info, full_path_filename);

    if((fp = fopen(full_path_filename, "r")) == NULL)
    {
        char errBuf[STD_BUF];
#ifdef WIN32
        snprintf(errBuf, STD_BUF, "%s", strerror(errno));
#else
        strerror_r(errno, errBuf, STD_BUF);
#endif
        errBuf[STD_BUF-1] = '\0';
        _dpd.errMsg("%s(%d) => Unable to open address file %s, Error: %s\n",
            *(_dpd.config_file), *(_dpd.config_line), full_path_filename, errBuf);
        return;
    }

    num_loaded_before = sfrt_flat_num_entries(config->iplist);
    while( fgets(linebuf, MAX_ADDR_LINE_LENGTH, fp) )
    {
        int iRet;
        addrline++;

        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Reputation configurations: %s\n",linebuf ););

        // Remove comments
        if( (cmt = strchr(linebuf, '#')) )
            *cmt = '\0';

        // Remove newline as well, prevent double newline in logging.
        if( (cmt = strchr(linebuf, '\n')) )
            *cmt = '\0';


        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Reputation configurations: %s\n",linebuf ););

        /* process the line */
        iRet = ProcessLine(linebuf, ipInfo_ptr, config);

        if (IP_INSERT_SUCCESS == iRet)
        {
            continue;
        }
        else if (IP_INSERT_FAILURE == iRet && fail_count++ < MAX_MSGS_TO_PRINT)
        {
            _dpd.errMsg("      (%d) => Failed to insert address: \'%s\'\n", addrline, linebuf);
        }
        else if (IP_INVALID == iRet && invalid_count++ < MAX_MSGS_TO_PRINT)
        {
            _dpd.errMsg("      (%d) => Invalid address: \'%s\'\n", addrline, linebuf);
        }
        else if (IP_INSERT_DUPLICATE == iRet && duplicate_count++ < MAX_MSGS_TO_PRINT)
        {
            _dpd.errMsg("      (%d) => Re-defined address: '%s'\n", addrline, linebuf );
        }
        else if (IP_MEM_ALLOC_FAILURE == iRet)
        {

            _dpd.errMsg("WARNING: %s(%d) => Memcap %u Mbytes reached when inserting IP Address: %s\n",
                full_path_filename, addrline, config->memcap,linebuf);

            if (config->statusBuf)
            {
                snprintf(config->statusBuf, config->statusBuf_len,
                    "WARNING: %s(%d) => Memcap %u Mbytes reached when inserting IP Address: %s\n",
                    full_path_filename, addrline, config->memcap,linebuf);
                config->statusBuf[config->statusBuf_len] = '\0';
            }
            config->memCapReached = true;
            break;
        }
    }

    total_duplicates += duplicate_count;
    total_invalids += invalid_count;
    /*Print out the summary*/
    if (fail_count > MAX_MSGS_TO_PRINT)
        _dpd.errMsg("    Additional addresses failed insertion but were not listed.\n");
    if (invalid_count > MAX_MSGS_TO_PRINT)
        _dpd.errMsg("    Additional invalid addresses were not listed.\n");
    if (duplicate_count > MAX_MSGS_TO_PRINT)
        _dpd.errMsg("    Additional duplicate addresses were not listed.\n");

    _dpd.logMsg("    Reputation entries loaded: %u, invalid: %u, re-defined: %u (from file %s)\n",
            sfrt_flat_num_entries(config->iplist) - num_loaded_before,
            invalid_count, duplicate_count, full_path_filename);

    fclose(fp);
}

/********************************************************************
 * Function: Reputation_FreeConfig
 *
 * Frees a reputation configuration
 *
 * Arguments:
 *  Reputation_Config *
 *      The configuration to free.
 *
 * Returns: None
 *
 ********************************************************************/
void Reputation_FreeConfig (ReputationConfig *config)
{

    if (config == NULL)
        return;

    if (config->localSegment != NULL)
    {
        free(config->localSegment);
    }

    if(config->sharedMem.path)
        free(config->sharedMem.path);
    free(config);
}


/*********************************************************************
 * Function: EstimateNumEntries
 *
 * First pass to decide iplist table size.
 *
 * Arguments:
 *
 * ReputationConfig *config: Reputation preprocessor configuration.
 * argp:              Pointer to string containing the config arguments.
 *
 * RETURNS:     int. estimated number of Entries based on number of lines
 *********************************************************************/
static int EstimateNumEntries(ReputationConfig *config, u_char* argp)
{
    char* cur_sectionp = NULL;
    char* next_sectionp = NULL;
    char* argcpyp = NULL;
    int totalLines = 0;


    /*Default values*/

    argcpyp = strdup( (char*) argp );

    if ( !argcpyp )
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory to parse Reputation options.\n");
        return 0;
    }

    cur_sectionp = strtok_r( argcpyp, REPUTATION_CONFIG_SECTION_SEPERATORS, &next_sectionp);
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Arguments token: %s\n",cur_sectionp ););

    while ( cur_sectionp )
    {
        char* next_tokenp = NULL;
        char* cur_tokenp =  strtok_r( cur_sectionp, REPUTATION_CONFIG_VALUE_SEPERATORS, &next_tokenp);

        if (!cur_tokenp)
        {
            cur_sectionp = strtok_r( next_sectionp, REPUTATION_CONFIG_SECTION_SEPERATORS, &next_sectionp);
            continue;
        }

        if ( !strcasecmp( cur_tokenp, REPUTATION_MEMCAP_KEYWORD ))
        {
            int value;
            char *endStr = NULL;

            cur_tokenp = strtok_r(next_tokenp, REPUTATION_CONFIG_VALUE_SEPERATORS, &next_tokenp);

            if ( !cur_tokenp )
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => No option to '%s'.\n",
                        *(_dpd.config_file), *(_dpd.config_line), REPUTATION_MEMCAP_KEYWORD);
            }

            value = _dpd.SnortStrtol( cur_tokenp, &endStr, 10);

            if (( *endStr) || (errno == ERANGE))
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                        "Please specify an integer between %d and %d.\n",
                        *(_dpd.config_file), *(_dpd.config_line),
                        REPUTATION_MEMCAP_KEYWORD, MIN_MEMCAP, MAX_MEMCAP);
            }

            if (value < MIN_MEMCAP || value > MAX_MEMCAP)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Value specified for %s is out of "
                        "bounds.  Please specify an integer between %d and %d.\n",
                        *(_dpd.config_file), *(_dpd.config_line),
                        REPUTATION_MEMCAP_KEYWORD, MIN_MEMCAP, MAX_MEMCAP);
            }
            config->memcap = (uint32_t) value;

        }
        else if ( !strcasecmp( cur_tokenp, REPUTATION_BLACKLIST_KEYWORD )
                ||!strcasecmp( cur_tokenp, REPUTATION_WHITELIST_KEYWORD ))
        {
            int numlines;
            char full_path_filename[PATH_MAX+1];
            cur_tokenp = strtok_r( next_tokenp, REPUTATION_CONFIG_VALUE_SEPERATORS, &next_tokenp);
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Check list size %s\n",cur_tokenp ););
            if(cur_tokenp == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Bad list filename in IP List.\n",
                        *(_dpd.config_file), *(_dpd.config_line));
            }
            errno = 0;
            UpdatePathToFile(full_path_filename,PATH_MAX, cur_tokenp);
            numlines = numLinesInFile(full_path_filename);
            if ((0 == numlines) && (0 != errno))
            {
                char errBuf[STD_BUF];

#ifdef WIN32
                snprintf(errBuf, STD_BUF, "%s", strerror(errno));
#else
                strerror_r(errno, errBuf, STD_BUF);
#endif
                DynamicPreprocessorFatalMessage("%s(%d) => Unable to open address file %s, Error: %s\n",
                        *(_dpd.config_file), *(_dpd.config_line), full_path_filename, errBuf);
            }

            if (totalLines + numlines < totalLines)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Too many entries in one file.\n",
                        *(_dpd.config_file), *(_dpd.config_line));
            }

            totalLines += numlines;

        }
        else if ( !strcasecmp( cur_tokenp, REPUTATION_WHITEACTION_KEYWORD ))
        {
            int i = 0;
            char WhiteActionKeyworBuff[STD_BUF];
            WhiteActionKeyworBuff[0]  = '\0';
            cur_tokenp = strtok_r( next_tokenp, REPUTATION_CONFIG_VALUE_SEPERATORS, &next_tokenp);
            if (!cur_tokenp)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Missing argument for %s\n",
                        *(_dpd.config_file), *(_dpd.config_line), REPUTATION_WHITEACTION_KEYWORD);

            }
            while(NULL != WhiteActionOption[i])
            {
                if( !strcasecmp(WhiteActionOption[i],cur_tokenp))
                {
                    config->whiteAction = (WhiteAction) i;
                    break;
                }
                _dpd.printfappend(WhiteActionKeyworBuff, STD_BUF, "[%s] ", WhiteActionOption[i] );
                i++;
            }
            if (NULL == WhiteActionOption[i])
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Invalid argument: %s for %s, use %s\n",
                        *(_dpd.config_file), *(_dpd.config_line), cur_tokenp,
                        REPUTATION_WHITEACTION_KEYWORD, WhiteActionKeyworBuff);
            }

        }
#ifdef SHARED_REP
        else if ( !strcasecmp( cur_tokenp, REPUTATION_SHARED_MEM_KEYWORD ))
        {

            if (Reputation_IsEmptyStr(next_tokenp))
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Missing argument for %s,"
                        " please specify a path\n",
                        *(_dpd.config_file), *(_dpd.config_line), REPUTATION_SHARED_MEM_KEYWORD);
            }

            if (!CheckIPlistDir(next_tokenp))
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Can't find or access the path: %s\n",
                        *(_dpd.config_file), *(_dpd.config_line), next_tokenp);
            }

            config->sharedMem.path = strdup( (char*) next_tokenp );

            if ( !config->sharedMem.path )
            {
                DynamicPreprocessorFatalMessage("Could not allocate memory to parse Reputation options.\n");

            }

            config->sharedMem.updateInterval = REPUTATION_DEFAULT_REFRESH_PERIOD;

        }
#endif

        cur_sectionp = strtok_r( next_sectionp, REPUTATION_CONFIG_SECTION_SEPERATORS, &next_sectionp);
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Arguments token: %s\n",cur_sectionp ););
    }

    free(argcpyp);
    return totalLines;
}

/*********************************************************************
 * Function: ParseReputationArgs
 *
 *  Parses and processes the configuration arguments
 *  supplied in the Reputation preprocessor rule.
 *
 * Arguments:
 *
 * ReputationConfig *config: Reputation preprocessor configuration.
 * argp:              Pointer to string containing the config arguments.
 *
 * RETURNS:     Nothing.
 *********************************************************************/
void ParseReputationArgs(ReputationConfig *config, u_char* argp)
{
    char* cur_sectionp = NULL;
    char* next_sectionp = NULL;
    char* argcpyp = NULL;
#ifdef SHARED_REP
    long nprocs;
#endif

    if (config == NULL)
        return;

    _dpd.logMsg("Reputation config: \n");

    /*Default values*/
    config->memcap = REPUTATION_DEFAULT_MEMCAP;
    config->priority = WHITELISTED_TRUST;
    config->nestedIP = INNER;
    config->whiteAction = UNBLACK;
    config->emptySegment = NULL;
    config->localSegment = NULL;
    config->memsize = 0;
    config->memCapReached = false;
#ifdef SHARED_REP
    nprocs = sysconf(_SC_NPROCESSORS_CONF);
    if (nprocs < 1)
    {
        _dpd.logMsg("WARNING: Could not determine the number of CPUs on the system, "
                "defaulting to a max of %d concurrent shared memory users.\n",
                REPUTATION_DEFAULT_SHARED_MEM_INSTANCES);
        config->sharedMem.maxInstances = REPUTATION_DEFAULT_SHARED_MEM_INSTANCES;
    }
    else if (nprocs < MIN_SHARED_MEM_INSTANCES)
    {
        _dpd.logMsg("WARNING: Fewer CPUs found than the minimum number of "
                "concurrent shared memory users, defaulting to a max of %d.\n",
                MIN_SHARED_MEM_INSTANCES);
        config->sharedMem.maxInstances = MIN_SHARED_MEM_INSTANCES;
    }
    else
        config->sharedMem.maxInstances = nprocs;
#endif

    /* Sanity check(s) */
    if ( !argp )
    {
        _dpd.logMsg("WARNING: Can't find any %s/%s entries. "
                "Reputation Preprocessor disabled.\n",
                REPUTATION_WHITELIST_KEYWORD, REPUTATION_BLACKLIST_KEYWORD);
        return;
    }

    argcpyp = strdup( (char*) argp );

    if ( !argcpyp )
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory to parse Reputation options.\n");
        return;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Reputation configurations: %s\n",argcpyp ););

    /*We need to parse the memcap, numEntries earlier, then create iplist table*/

    config->numEntries = EstimateNumEntries(config, argp );

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Estimated number of entries: %d\n",config->numEntries ););

    if ((config->numEntries <= 0) && (!config->sharedMem.path))
    {
        _dpd.logMsg("WARNING: Can't find any %s/%s entries. "
                "Reputation Preprocessor disabled.\n",
                REPUTATION_WHITELIST_KEYWORD, REPUTATION_BLACKLIST_KEYWORD);
        free(argcpyp);
        return;
    }
    if (!config->sharedMem.path)
        IpListInit(config->numEntries + 1,config);

    cur_sectionp = strtok_r( argcpyp, REPUTATION_CONFIG_SECTION_SEPERATORS, &next_sectionp);
    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Arguments token: %s\n",cur_sectionp ););
    /*Reset the log message count*/
    total_duplicates = 0;
    while ( cur_sectionp )
    {

        char* cur_config;
        char* cur_tokenp = 	strtok( cur_sectionp, REPUTATION_CONFIG_VALUE_SEPERATORS);

        if (!cur_tokenp)
        {
            cur_sectionp = strtok_r( next_sectionp, REPUTATION_CONFIG_SECTION_SEPERATORS, &next_sectionp);
            continue;
        }
        cur_config = cur_tokenp;

        if ( !strcasecmp( cur_tokenp, REPUTATION_SCANLOCAL_KEYWORD ))
        {
            config->scanlocal = 1;
        }
        else if ( !strcasecmp( cur_tokenp, REPUTATION_MEMCAP_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, REPUTATION_CONFIG_VALUE_SEPERATORS);
            /* processed before */
        }
        else if ( !strcasecmp( cur_tokenp, REPUTATION_BLACKLIST_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, REPUTATION_CONFIG_VALUE_SEPERATORS);
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Loading %s from %s\n", REPUTATION_BLACKLIST_KEYWORD, cur_tokenp ););
            if(cur_tokenp == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Bad list filename in IP List.\n",
                        *(_dpd.config_file), *(_dpd.config_line));
            }
            if (!config->sharedMem.path)
                LoadListFile(cur_tokenp, config->local_black_ptr, config);
            else
            {
                _dpd.logMsg("WARNING: %s(%d) => List file %s is not loaded "
                        "when using shared memory.\n",
                        *(_dpd.config_file), *(_dpd.config_line), cur_tokenp);
            }
        }

        else if ( !strcasecmp( cur_tokenp, REPUTATION_WHITELIST_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, REPUTATION_CONFIG_VALUE_SEPERATORS);
            DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Loading %s from %s\n", REPUTATION_WHITELIST_KEYWORD, cur_tokenp ););
            if(cur_tokenp == NULL)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Bad list filename in IP List.\n",
                        *(_dpd.config_file), *(_dpd.config_line));
            }

            if (!config->sharedMem.path)
                LoadListFile(cur_tokenp, config->local_white_ptr, config);
            else
            {
                _dpd.logMsg("WARNING: %s(%d) => List file %s is not loaded "
                        "when using shared memory.\n",
                        *(_dpd.config_file), *(_dpd.config_line), cur_tokenp);
            }
        }
        else if ( !strcasecmp( cur_tokenp, REPUTATION_PRIORITY_KEYWORD ))
        {

            cur_tokenp = strtok( NULL, REPUTATION_CONFIG_VALUE_SEPERATORS);
            if (!cur_tokenp)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Missing argument for %s\n",
                        *(_dpd.config_file), *(_dpd.config_line), REPUTATION_PRIORITY_KEYWORD);
                return;
            }

            if((strlen(REPUTATION_BLACKLIST_KEYWORD) == strlen (cur_tokenp))
                    && !strcasecmp(REPUTATION_BLACKLIST_KEYWORD,cur_tokenp))
            {
                config->priority = BLACKLISTED;
            }
            else if((strlen(REPUTATION_WHITELIST_KEYWORD) == strlen (cur_tokenp))
                    && !strcasecmp(REPUTATION_WHITELIST_KEYWORD,cur_tokenp))
            {
                config->priority = WHITELISTED_TRUST;
                if (UNBLACK == config->whiteAction)
                {
                    _dpd.logMsg("WARNING: %s(%d) => Keyword %s for %s is not applied "
                            "when white action is unblack.\n", *(_dpd.config_file), *(_dpd.config_line),
                            REPUTATION_PRIORITY_KEYWORD, REPUTATION_WHITELIST_KEYWORD);
                    config->priority = WHITELISTED_UNBLACK;
                }
            }
            else
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Invalid argument: %s for %s,"
                        " Use [%s] or [%s]\n",
                        *(_dpd.config_file), *(_dpd.config_line), cur_tokenp,
                        REPUTATION_PRIORITY_KEYWORD,
                        REPUTATION_BLACKLIST_KEYWORD, REPUTATION_WHITELIST_KEYWORD);
                return;
            }

        }
        else if ( !strcasecmp( cur_tokenp, REPUTATION_NESTEDIP_KEYWORD ))
        {
            int i = 0;
            char NestIPKeyworBuff[STD_BUF];
            NestIPKeyworBuff[0]  = '\0';
            cur_tokenp = strtok( NULL, REPUTATION_CONFIG_VALUE_SEPERATORS);
            if (!cur_tokenp)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Missing argument for %s\n",
                        *(_dpd.config_file), *(_dpd.config_line), REPUTATION_NESTEDIP_KEYWORD);
                return;
            }
            while(NULL != NestedIPKeyword[i])
            {
                if((strlen(NestedIPKeyword[i]) == strlen (cur_tokenp))
                        && !strcasecmp(NestedIPKeyword[i],cur_tokenp))
                {
                    config->nestedIP = (NestedIP) i;
                    break;
                }
                _dpd.printfappend(NestIPKeyworBuff, STD_BUF, "[%s] ", NestedIPKeyword[i] );
                i++;
            }
            if (NULL == NestedIPKeyword[i])
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Invalid argument: %s for %s, use %s\n",
                        *(_dpd.config_file), *(_dpd.config_line), cur_tokenp,
                        REPUTATION_NESTEDIP_KEYWORD, NestIPKeyworBuff);
                return;
            }

        }
        else if ( !strcasecmp( cur_tokenp, REPUTATION_WHITEACTION_KEYWORD ))
        {

            cur_tokenp = strtok( NULL, REPUTATION_CONFIG_VALUE_SEPERATORS);
            /* processed before */

        }
#ifdef SHARED_REP
        else if ( !strcasecmp( cur_tokenp, REPUTATION_SHARED_MEM_KEYWORD ))
        {
            cur_sectionp = strtok_r( next_sectionp, REPUTATION_CONFIG_SECTION_SEPERATORS, &next_sectionp);
            continue;
            /* processed before */
        }
        else if ( !strcasecmp( cur_tokenp, REPUTATION_SHARED_REFRESH_KEYWORD ))
        {
            unsigned long value;
            char *endStr = NULL;

            if (!config->sharedMem.path)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Specify option '%s' when using option '%s'.\n",
                        *(_dpd.config_file), *(_dpd.config_line),
                        REPUTATION_SHARED_MEM_KEYWORD, REPUTATION_SHARED_REFRESH_KEYWORD);
            }
            cur_tokenp = strtok(NULL, REPUTATION_CONFIG_VALUE_SEPERATORS);

            if ( !cur_tokenp )
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => No option to '%s'.\n",
                        *(_dpd.config_file), *(_dpd.config_line), REPUTATION_SHARED_REFRESH_KEYWORD);
            }

            value = _dpd.SnortStrtoul( cur_tokenp, &endStr, 10);

            if ( *endStr)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                        "Please specify an integer between %u and %u.\n",
                        *(_dpd.config_file), *(_dpd.config_line),
                        REPUTATION_SHARED_REFRESH_KEYWORD,
                        MIN_SHARED_MEM_REFRESH_PERIOD, MAX_SHARED_MEM_REFRESH_PERIOD);
            }

            if (value < MIN_SHARED_MEM_REFRESH_PERIOD || value > MAX_SHARED_MEM_REFRESH_PERIOD
                    || (errno == ERANGE))
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Value specified for %s is out of "
                        "bounds.  Please specify an integer between %u and %u.\n",
                        *(_dpd.config_file), *(_dpd.config_line),
                        REPUTATION_SHARED_REFRESH_KEYWORD, MIN_SHARED_MEM_REFRESH_PERIOD,
                        MAX_SHARED_MEM_REFRESH_PERIOD);
            }
            config->sharedMem.updateInterval = (uint32_t) value;

        }
        else if (!strcasecmp(cur_tokenp, REPUTATION_SHARED_MAX_INSTANCES_KEYWORD))
        {
            unsigned long value;
            char *endStr = NULL;

            cur_tokenp = strtok(NULL, REPUTATION_CONFIG_VALUE_SEPERATORS);
            if (!cur_tokenp)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => No option to '%s'.\n",
                        *(_dpd.config_file), *(_dpd.config_line), REPUTATION_SHARED_MAX_INSTANCES_KEYWORD);
            }

            value = _dpd.SnortStrtoul( cur_tokenp, &endStr, 10);

            if (*endStr || (errno == ERANGE) || value < MIN_SHARED_MEM_INSTANCES || value > MAX_SHARED_MEM_INSTANCES)
            {
                DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                        "Please specify an integer between %u and %u.\n",
                        *(_dpd.config_file), *(_dpd.config_line),
                        REPUTATION_SHARED_MAX_INSTANCES_KEYWORD,
                        MIN_SHARED_MEM_INSTANCES, MAX_SHARED_MEM_INSTANCES);
            }
            config->sharedMem.maxInstances = value;
        }
#endif
        else
        {
            DynamicPreprocessorFatalMessage(" %s(%d) => Invalid argument: %s\n",
                    *(_dpd.config_file), *(_dpd.config_line), cur_tokenp);
            return;
        }
        /*Check whether too many parameters*/
        if (NULL != strtok( NULL, REPUTATION_CONFIG_VALUE_SEPERATORS))
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Too many arguments: %s\n",
                    *(_dpd.config_file), *(_dpd.config_line), cur_config);

        }
        cur_sectionp = strtok_r( next_sectionp, REPUTATION_CONFIG_SECTION_SEPERATORS, &next_sectionp);
        DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Arguments token: %s\n",cur_sectionp ););
    }
    DisplayIPlistStats(config);
    DisplayReputationConfig(config);
    free(argcpyp);
}

void ReputationRepInfo(IPrepInfo * repInfo, uint8_t *base, char *repInfoBuff,
    int bufLen)
{

    char *index = repInfoBuff;
    int  len = bufLen -1 ;
    int writed;

    writed = snprintf(index, len, "Reputation Info: ");
    if (writed >= len || writed < 0)
        return;

    index += writed;
    len -= writed;

    while(repInfo)
    {
        int i;
        for(i = 0; i < NUM_INDEX_PER_ENTRY; i++)
        {
            writed = snprintf(index, len, "%d,",repInfo->listIndexes[i]);
            if (writed >= len || writed < 0)
                return;
            else
            {
                index += writed;
                len -=writed;
            }

        }
        writed = snprintf(index, len, "->");
        if (writed >= len || writed < 0)
            return;
        else
        {
            index += writed;
            len -=writed;
        }

        if (!repInfo->next) break;

        repInfo = (IPrepInfo *)(&base[repInfo->next]);
    }
}
#ifdef DEBUG_MSGS
void ReputationPrintRepInfo(IPrepInfo * repInfo, uint8_t *base)
{

    char repInfoBuff[STD_BUF];
    int  len = STD_BUF -1 ;

    repInfoBuff[STD_BUF -1] = '\0';

    ReputationRepInfo(repInfo, base, repInfoBuff, len);

    DEBUG_WRAP(DebugMessage(DEBUG_REPUTATION, "Reputation Info: %s \n",
            repInfoBuff););
}
#endif
