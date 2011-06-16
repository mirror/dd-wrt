/******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 *****************************************************************************/
 
/**
******************************************************************************
* @file mil_file_interface.c
*
* @defgroup MIL
*
* @ingroup MILFIleInterface
*
* @description
*      This file contains the functions to get the data
from the kernel and write it to log file in
the desired format
*
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dcc/icp_dcc.h"
#include "icp_dcc_cfg.h"
#include "icp_dcc_al.h"
#include "mil_main.h"

#define MODULEID_SIZE 4 /**<Size of moduleid field in bytes*/
#define INFOID_SIZE 4 /**<Size of infoid field in bytes*/
#define INTERNAL_DUMP_SIZE 4 /**<internal dump size field in bytes*/

#define MAX_DATA_SIZE 16
#define MIL_ONE 1
#define MIL_TWO 2

#define WRITE_TO_FILE(fp, format, args...) fprintf(fp, format, ##args)

/**
 *****************************************************************************
 * @ingroup MILFileInterface
 *  Putting Version Dump to File
 *
 * @description
 *  puts version dump into a log file
 *
 * @context
 *      This function runs in the context of the calling function thread.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @return MIL_SUCCESS for success or appropriate mil_error_status_t
 *
 * @pre
 *      None.
 *
 * @post
 *      None.
 *
 *****************************************************************************/
mil_error_status_t
FileVersionDump(void)
{
    uint32_t VersionSize = 0;
    int32_t pAuxPointer = 0;
    int16_t registeredComponentsIndex = 0;
    uint16_t i = 0;
    FILE *fp = NULL;
    icp_dcc_version_buffer_t *pVerInfo = NULL;
    uint8_t filename[MAX_FILENAME_SIZE] = {0};
    mil_error_status_t status = MIL_SUCCESS;
    
    memset(filename, 0,  MAX_FILENAME_SIZE);

    pAuxPointer = (int32_t) &VersionSize;
    status = call_ioctl(IOCTL_CMD_MIL_VERSION_INFO_GET_SIZE,&pAuxPointer);
    if (status != MIL_SUCCESS)
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:FileVersionDump] "\
                                "MIL_VERSION_INFO_GET_SIZE failed \n");
        return status; 
    }
    pVerInfo = (icp_dcc_version_buffer_t *)malloc(VersionSize);
    MIL_EQUAL_TEST_AND_RETURN (pVerInfo, NULL, MIL_MEMORY_ALLOC_FAIL)

    pAuxPointer = (int32_t) pVerInfo;
    status = call_ioctl(IOCTL_CMD_MIL_INTERNAL_VERSION_GET,&pAuxPointer);
    MIL_NOTEQUAL_TEST_AND_RETURN (status, MIL_SUCCESS, status)  

    pAuxPointer = (int32_t) &filename;
    status = call_ioctl(IOCTL_CMD_MIL_FILENAME_GET, &pAuxPointer);
    MIL_NOTEQUAL_TEST_AND_RETURN (status, MIL_SUCCESS, status)  

    fp = fopen((char *)filename,"a");
    MIL_EQUAL_TEST_AND_RETURN (fp, NULL, MIL_FILE_ERROR)
    WRITE_TO_FILE (fp, "DV - Start\n" );
    WRITE_TO_FILE (fp, "PackageName = %s \n" , \
                    pVerInfo->packageVerInfo.packageName );
    WRITE_TO_FILE (fp, "PackageVersion = %s \n" , \
                    pVerInfo->packageVerInfo.packageVersion );

    registeredComponentsIndex = pVerInfo->numModules;

    /*+1 as we want to print natural count of components*/
    MIL_USER_DEBUG_PRINT( "[MIL_us:FileVersionDump] component count = %d\n" ,
                            registeredComponentsIndex);

    if ( registeredComponentsIndex >= 0 )
    {
        /*get the version information*/
        WRITE_TO_FILE (fp, "Component Version - Start\n" );
        for ( i = 0 ; i < registeredComponentsIndex ; i++ )
        {
            uint8_t module_name[MAX_DATA_SIZE];
            WRITE_TO_FILE (fp, "ModuleId = %d \n" ,
                            pVerInfo->moduleVerInfo[i].moduleId );
            /*Get first 16 characters from module name*/
            strncpy( (char *)module_name ,
                     (char *)pVerInfo->moduleVerInfo[i].moduleName ,
                     (MAX_DATA_SIZE-1));
            module_name[MAX_DATA_SIZE-1]='\0';
            WRITE_TO_FILE (fp,"ComponentName = %s \n" ,
                           module_name );
            WRITE_TO_FILE (fp,"ComponentVersion = %s \n" ,
                           pVerInfo->moduleVerInfo[i].moduleVersion );
        }
        WRITE_TO_FILE(fp,"Component Version - End\n" );
    }
    WRITE_TO_FILE(fp,"DV - End\n");
    free (pVerInfo);
    pVerInfo = NULL;
    fclose(fp);
    fp = NULL;
    return MIL_SUCCESS;
}


/**
 *****************************************************************************
 * @ingroup MILFileInterface
 *  Writing anything to a log file
 *
 * @description
 *  Writing anything to a log file
 *
 * @context
 *      This function runs in the context of the calling function thread.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @param bufferToWrite IN buffer of chars that should be written into logfile
 *
 * @return MIL_SUCCESS for success or appropriate mil_error_status_t
 *
 * @pre
 *      None.
 *
 * @post
 *      None.
 *
 *****************************************************************************/
mil_error_status_t
WriteToFile(uint8_t *bufferToWrite)
{
    uint8_t filename[MAX_FILENAME_SIZE] = {0};
    int32_t pAuxPointer = 0;
    FILE *fp = 0;
    mil_error_status_t status = MIL_SUCCESS;

    memset(filename, 0,  MAX_FILENAME_SIZE);

    pAuxPointer = (int32_t) &filename;
    status = call_ioctl(IOCTL_CMD_MIL_FILENAME_GET, &pAuxPointer);
    if(status != MIL_SUCCESS)
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:WriteToFile] MIL_FILENAME_GET failed\n");
        return status;
    }
    MIL_USER_DEBUG_PRINT("[MIL_us:WriteToFile] filename = %s \n", \
                        (char *) filename);
    MIL_USER_DEBUG_PRINT("[MIL_us:WriteToFile] &filename = %p \n", &filename);

    fp = fopen((char *)filename,"a");
    MIL_EQUAL_TEST_AND_RETURN (fp, NULL, MIL_FILE_ERROR)
    MIL_USER_DEBUG_PRINT("[MIL_us:WriteToFile] opened file %s\n", \
                        (char *) filename);
    fprintf(fp,(char *)bufferToWrite);
    MIL_USER_DEBUG_PRINT("[MIL_us:WriteToFile] written \n%s\n\n", \
                        (char *) bufferToWrite);
    fclose(fp);
    MIL_USER_DEBUG_PRINT("[MIL_us:WriteToFile] closed file %s\n", \
                        (char *) filename);
    return MIL_SUCCESS;
}


/**
 *****************************************************************************
 * @ingroup MILFileInterface
 *  writes data related to Livness inquiry to a log file
 *
 * @description
 *  writes data related to Livness inquiry to a log file
 *
 * @context
 *      This function runs in the context of the calling function thread.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @return MIL_SUCCESS for success or appropriate mil_error_status_t
 *
 * @pre
 *      None.
 *
 * @post
 *      None.
 *
 *****************************************************************************/
mil_error_status_t
FileLivenessVerify(void)
{
    int16_t numberofLivenessThreads = 0;
    mil_error_status_t status = MIL_SUCCESS;
    uint32_t LivenessResponseSize = 0;
    icp_dcc_liveness_status_t *pLivenessStatusBuffer = NULL;
    FILE *fp = NULL;
    int16_t i = 0;
    uint8_t filename[MAX_FILENAME_SIZE] = {0};
    int32_t pAuxPointer = 0;

    memset(filename, 0,  MAX_FILENAME_SIZE);

    pAuxPointer = (int32_t) &LivenessResponseSize;
    status = call_ioctl(IOCTL_CMD_MIL_LIVENESS_GET_SIZE, &pAuxPointer);
    if (status != MIL_SUCCESS)
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:FileLivenessVerify] "
                                "MIL_LIVENESS_GET_SIZE failed.\n");
        return status;
    }
    MIL_USER_DEBUG_PRINT("[MIL_us:FileLivenessVerify] "\
                        "MIL liveness size is %d \n",
                         LivenessResponseSize);
    if (LivenessResponseSize < sizeof(icp_dcc_liveness_status_t))
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:FileLivenessVerify] "\
                                "MIL liveness size is 0\n");
        return MIL_SUCCESS;
    }
    pLivenessStatusBuffer = (icp_dcc_liveness_status_t *)
                             malloc(LivenessResponseSize);

    MIL_USER_DEBUG_PRINT("[MIL_us:FileLivenessVerify] %p %p \n",
                            pLivenessStatusBuffer,  &LivenessResponseSize);
    memcpy (pLivenessStatusBuffer,  &LivenessResponseSize, sizeof(uint32_t));
    MIL_USER_DEBUG_PRINT("[MIL_us:FileLivenessVerify] %p %p \n",
                            pLivenessStatusBuffer,  &LivenessResponseSize);

    pAuxPointer = (int32_t) pLivenessStatusBuffer;
    status = call_ioctl(IOCTL_CMD_MIL_INTERNAL_LIVENESS_VERIFY, \
                &pAuxPointer);
    MIL_NOTEQUAL_TEST_AND_RETURN(status, MIL_SUCCESS, status)
    MIL_USER_DEBUG_PRINT("[MIL_us:FileLivenessVerify] "\
                        "MIL liveness size is %d \n",
                         LivenessResponseSize);

    /*calculate the number of threads*/
    numberofLivenessThreads =
        LivenessResponseSize / sizeof(icp_dcc_liveness_status_t);

    MIL_USER_DEBUG_PRINT("[MIL_us:FileLivenessVerify] number of threads "\
                        "for liveness is %d (%d / %d)\n", 
                        numberofLivenessThreads, LivenessResponseSize , 
                        sizeof(icp_dcc_liveness_status_t));

    /* Since we start count from 0 */
    numberofLivenessThreads--;
    
    pAuxPointer = (int32_t) &filename;
    status = call_ioctl(IOCTL_CMD_MIL_FILENAME_GET, &pAuxPointer);
    MIL_NOTEQUAL_TEST_AND_RETURN(status, MIL_SUCCESS, status)
    
    fp = fopen((char *)filename,"a");
    MIL_EQUAL_TEST_AND_RETURN (fp, NULL, MIL_FILE_ERROR)
    WRITE_TO_FILE (fp, "\nDL - Start\n" );
    /* Display System Liveness */
    for ( i = 0 ; i <= numberofLivenessThreads ; i++ )
    {
        if ( ( pLivenessStatusBuffer + i ) != NULL )
        {
            WRITE_TO_FILE (fp, "%s: " ,
                ( pLivenessStatusBuffer + i )->threadId.threadIdString );
            switch ( ( pLivenessStatusBuffer + i )->threadStatus )
            {
            case ICP_DCC_THREAD_ID_LIVE :
                WRITE_TO_FILE (fp, "LIVE\n" );
                break;
            case  ICP_DCC_THREAD_ID_DEAD :
                WRITE_TO_FILE (fp, "DEAD\n" );
                break;
            default:
                WRITE_TO_FILE (fp, "DEAD\n" );
                break;

            }
        }
    }
    WRITE_TO_FILE (fp, "DL - End\n" );
    fclose(fp);
    fp = NULL;
    free(pLivenessStatusBuffer);
    pLivenessStatusBuffer = NULL;
    return MIL_SUCCESS;
}


/**
 *****************************************************************************
 * @ingroup MILFileInterface
 *  prepares and inquries dump data for writing to a log file
 *
 * @description
 *  prepares and inquries dump data for writing to a log file
 *
 * @context
 *      This function runs in the context of the calling function thread.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @return MIL_SUCCESS for success or appropriate mil_error_status_t
 *
 * @pre
 *      None.
 *
 * @post
 *      None.
 *
 *****************************************************************************/
mil_error_status_t
FileDataDump(void)
{
    uint32_t max_size = 0;
    uint32_t noOfModules = 0;
    uint32_t index = 0;
    uint32_t moduleDumpSize = 0;
    int32_t pAuxPointer = 0;

    DataDumpSizeBuffer_t dump_buffer = {0};
    uint8_t filename[MAX_FILENAME_SIZE] = {0};
    uint8_t * pDataDump = NULL;
    FILE *fp = NULL;
    mil_error_status_t status = MIL_SUCCESS;

    memset(filename, 0,  MAX_FILENAME_SIZE);

    pAuxPointer = (int32_t) &dump_buffer;
    status = call_ioctl(IOCTL_CMD_MIL_DATA_DUMP_GET_SIZE, &pAuxPointer);
    if(status != MIL_SUCCESS)
   {
       MIL_USER_DEBUG_PRINT("[MIL_us:FileDataDump] "\
                            "MIL_DATA_DUMP_GET_SIZE failed\n");
       return status;
   }
    noOfModules = dump_buffer.noOfModule;
    max_size = dump_buffer.maxDumpSize;
    MIL_USER_DEBUG_PRINT("[MIL_us:FileDataDump] max size is %d, " \
                           "no. of modules are %d\n", \
                           max_size, noOfModules);
    if ( (noOfModules && max_size) == 0) 
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:FileDataDump] no data to dump\n");
        return MIL_SUCCESS;
    }
    pAuxPointer = (int32_t) &filename;
    status = call_ioctl(IOCTL_CMD_MIL_FILENAME_GET, &pAuxPointer);
    fp = fopen((char *)filename,"a");
    MIL_EQUAL_TEST_AND_RETURN (fp, NULL, MIL_FILE_ERROR)

    /*pDataDump is the buffer which will be passed between user and kernel
    containing data dump. While going from user mode to kernel mode it will 
    contain the index as first four bytes. This is needed for dcc in kernel 
    mode. Rest of the data will be ignored in kernel mode. While coming back
    from kernel mode to user mode this buffer will contain the datadump 
    and also the size of that module's datadump. So first four bytes will be 
    size and rest is data. Hence we have to allocate four more bytes than
    the maximum dump size to get the module dump size value*/

    pDataDump = (uint8_t *)malloc(max_size + sizeof(uint32_t));
    MIL_DEBUG_POSITIVE_TEST_AND_RETURN (pDataDump, 
                                        NULL, 
                                        MIL_MEMORY_ALLOC_FAIL)

    for ( index = MIL_ONE ; index <= noOfModules ; index++ )
    {
        memset ( pDataDump , 0 , max_size + sizeof(uint32_t));
        memcpy(pDataDump, &index, sizeof(uint32_t));
        moduleDumpSize = 0 ;


        pAuxPointer = (int32_t) pDataDump;
        status = call_ioctl(IOCTL_CMD_MIL_INTERNAL_DATA_DUMP_GET, 
                            &pAuxPointer);
        if (status != MIL_SUCCESS)
        {
            printf("[MIL_us:FileDataDump] icp_DccDataDumpGet failed "\
                    "for module: %d\n", index);
            continue;
        }
        /*So first we will extract the dump size from first four bytes*/
        memcpy(&moduleDumpSize, pDataDump, sizeof(uint32_t));
        MIL_USER_DEBUG_PRINT("[MIL_us:FileDataDump] module dump size is %u\n",
                                moduleDumpSize);
        if ( ( (pDataDump + sizeof(uint32_t) )== NULL ) 
              || ( moduleDumpSize == 0 ) 
              || ( moduleDumpSize > max_size) )
        {
            MIL_USER_DEBUG_PRINT("[MIL_us:FileDataDump] module dump size = 0 "\
                                   "or module dump size > max_size (%d, %d) " \
                                   "or data dump is NULL\n ",
                                    moduleDumpSize, max_size);
            continue;
        }
        else
        {
        /*As the dump begins after four bytes we will pass that pointer 
        location for parsing data from that point*/
            WriteDataDumpToLog ( fp, pDataDump + sizeof(uint32_t)  , 
                                  moduleDumpSize);
        }
    }
    free ( pDataDump );
    pDataDump = NULL;
    fclose(fp);
    fp = NULL;
    return MIL_SUCCESS;

}


/**
 *****************************************************************************
 * @ingroup MILFileInterface
 *  writes dump data to a log file
 *
 * @description
 *  writes dump data to a log file
 *
 * @context
 *      This function runs in the context of the calling function thread.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @return MIL_SUCCESS for success or appropriate mil_error_status_t
 *
 * @pre
 *      None.
 *
 * @post
 *      None.
 *
 *****************************************************************************/
void
WriteDataDumpToLog (
    FILE *fp,
    uint8_t *pDataDump ,
    uint32_t dumpSize )
{
    uint32_t module_id = 0;
    uint32_t info_id = 0;
    uint32_t internal_dump_size = 0; /**<Size of the data dump in bytes
                                         excluding module id and info id */
    uint32_t pointer = 0; /**<used to track the position in buffer*/
    uint32_t linesToWrite = 0;
    uint32_t line_index = 0;
    uint8_t *pCurrentLocation = NULL;
    uint8_t byte_index = 0, lastLineBytes = 0;

    /*Check for the dump size less than 4*/
    if ( dumpSize < MODULEID_SIZE )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:WriteDataDumpToLog] dump size "\
                            "is not enough (dumpSize < MODULEID_SIZE).\n");
        return;
    }

    /*Get the module id from first four bytes*/
    memcpy ( &module_id , pDataDump , MODULEID_SIZE );
    WRITE_TO_FILE (fp, "DD - ModuleId = %u\n" ,
                    module_id );
    pointer += MODULEID_SIZE;

    /*check for the dump size less than 8*/
    if ( dumpSize < (MODULEID_SIZE + INFOID_SIZE))
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:WriteDataDumpToLog] dump size "\
            "is not enough (dumpSize < (MODULEID_SIZE + INFOID_SIZE)).\n");
        return;
    }
    /*get the info id from next 4 bytes*/
    memcpy ( &info_id , pDataDump + pointer , INFOID_SIZE );
    WRITE_TO_FILE(fp, "DD - InfoId = %u\n" , info_id );

    pointer += INFOID_SIZE;

    /*Check for the dumpsize less than 12*/
    if ( dumpSize <  ( MODULEID_SIZE + INFOID_SIZE + INTERNAL_DUMP_SIZE ) )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:WriteDataDumpToLog] dump size "\
                            "is not enough (dumpSize < (MODULEID_SIZE + "\
                            "INFOID_SIZE + INTERNAL_DUMP_SIZE)).\n");
        return;
    }

    /*get the dump size from next 4 bytes*/
    memcpy ( &internal_dump_size ,
             pDataDump+pointer ,
             INTERNAL_DUMP_SIZE );
    pointer += INTERNAL_DUMP_SIZE;
    /*Convert to actual size*/
    internal_dump_size *= INTERNAL_DUMP_SIZE;
    WRITE_TO_FILE (fp, "DD - DataDumpSize = %u\n" ,
                    internal_dump_size );

    if ( internal_dump_size == 0 )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:WriteDataDumpToLog] dump data "\
                            "not available.\n");
        return;
    }

    if ( ( internal_dump_size + MODULEID_SIZE +
            INFOID_SIZE + INTERNAL_DUMP_SIZE )
            > dumpSize  )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:WriteDataDumpToLog] internal dump "\
                            "is bigger than expected.\n" );
        return;
    }

    WRITE_TO_FILE (fp, "DD - Start\n" );
    pCurrentLocation = pDataDump + pointer;
    /*get the data dump and put it in syslog*/

    linesToWrite = internal_dump_size / MAX_DATA_SIZE;
    for ( line_index = 0 ; line_index < linesToWrite ; line_index++ )
    {
        WRITE_TO_FILE (fp, "DD - %04u - " ,
                        line_index * MAX_DATA_SIZE );
        for ( byte_index = 0 ; byte_index < MAX_DATA_SIZE ; byte_index++ )
        {
            WRITE_TO_FILE (fp, "%02X " , *pCurrentLocation  );
            pCurrentLocation++;
        }
        WRITE_TO_FILE (fp, "\n" );
    }

    /*print the last line*/
    if  ( ( internal_dump_size % MAX_DATA_SIZE ) != 0)
    {
        WRITE_TO_FILE (fp, "DD - %04u - ",
                        linesToWrite * MAX_DATA_SIZE );
        lastLineBytes = internal_dump_size % MAX_DATA_SIZE ;
        for ( byte_index = 0 ; byte_index < lastLineBytes ; byte_index++ )
        {
            WRITE_TO_FILE (fp, "%02X " , *pCurrentLocation  );
            pCurrentLocation++ ;
        }
        WRITE_TO_FILE (fp, "\n" );
    }
    WRITE_TO_FILE (fp, "DD - End\n" );
}
