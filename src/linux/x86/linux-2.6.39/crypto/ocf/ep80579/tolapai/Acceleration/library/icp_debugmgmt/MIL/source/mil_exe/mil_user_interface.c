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
* @file mil_user_interface.c
*
* @defgroup MIL
*
* @ingroup MILUserInterface
*
* @description
*      This file contains the functions to get the data
*       from the user and then send it to the kernel
*	interface the desired format
*
*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

#include "mil_main.h"

#define MIN_ARG_NUMBER 2 /**<minimum number of arguments*/
#define MIN_TIMEOUT_ARG_NUMBER 3 /**<Minimum number of arguments
needed for timeout*/
#define COMMAND_ARG 1 /**<index of command argumnet in argv */
#define TIMEOUT_ARG 2 /**<index of timeout argumnet in argv */
#define MIN_SETFILENAME_ARG_NUMBER 3
#define FILENAME_ARG 2

#define MAX_MESSAGE_SIZE 80
#define DEBUG_ENABLE_STRING "DebugEnable"
#define DEBUG_DISABLE_STRING "DebugDisable"
#define VERSION_DUMP_ALL_STRING "VersionDumpAll"
#define CONFIGURE_LIVENESS_TIMEOUT_SRING "setHC"
#define VERIFY_HEALTH_STRING "SystemHealthCheck"
#define DUMP_ALL_STRING "DataDump"
#define HELP_DISPLAY_STRING "help"
#define SET_FILE_NAME_STRING "SetFileName"

const char *error_strings[] =
        {
            "SUCCESS",
            "FAILURE",
            "Unknown Command",
            "MIL is not enabled",
            "MIL is already enabled",
            "DCC failed while getting the size for version information",
            "DCC failed while getting the data for version information",
            "DCC failed while getting the dump information",
            "DCC failed while getting the data dump",
            "Memory allocation failed",
            "DCC failed while configuring the timeout for liveness",
            "DCC failed while getting the size for liveness information",
            "DCC failed while getting the liveness data",
            "DCC failed while registering the SEN handler",
            "DCC failed while unregister the SEN handler",
            "Error in copy between kernel and user mode",
            "Error in the operation for the device driver",
            "Error in opening the device driver",
            "Error in closing the device driver",
            "Error in getting filename. Size is more than 256",
            "Error in file operations",
            "unknown"
        };
/**
*****************************************************************************
* @ingroup MILUserInterface
*  	Convert the string to the integer
*
* @description
*  This function will get the string and
return the integer value of it.
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
*      Yes
*
* @param s IN the string which needs to be conevrted to
integer
*
* @return integer which is the integer value of string
*
* @pre
*      string must be valid integer
*
* @post
*      None.
*
*****************************************************************************/

uint32_t
convert_string_to_integer (
    char *s )
{
    unsigned int convertedInteger = atoi ( s );
    return convertedInteger;
}

/**
*****************************************************************************
* @ingroup MILUserInterface
*  	Parse the arguments
*
* @description
*  This function takes the argument in command line and
then finds out the appropriate mil_command to be used.
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
*      Yes
*
* @param enteredString IN the string which needs to be parsed
*
* @return mil_commands_t which is in map with the senterd
string
*
* @pre
*      None.
*
* @post
*      None.
*
***************************************************************************/
mil_commands_t
parseArguments (
    char *enteredString )
{
    if ( strncmp ( enteredString ,
                   DEBUG_ENABLE_STRING ,
                   strlen( DEBUG_ENABLE_STRING ) ) == 0 )
    {
        return MIL_DEBUG_ENABLE;
    }
    if ( strncmp ( enteredString ,
                   DEBUG_DISABLE_STRING ,
                   strlen ( DEBUG_DISABLE_STRING ) ) == 0 )
    {
        return MIL_DEBUG_DISABLE;
    }
    if ( strncmp ( enteredString ,
                   VERSION_DUMP_ALL_STRING ,
                   strlen ( VERSION_DUMP_ALL_STRING ) ) == 0 )
    {
        return MIL_DEBUG_VERSION_GET_ALL;
    }
    if ( strncmp ( enteredString ,
                   CONFIGURE_LIVENESS_TIMEOUT_SRING ,
                   strlen ( CONFIGURE_LIVENESS_TIMEOUT_SRING ) ) == 0 )
    {
        return MIL_LIVENESS_CONFIGURE_TIMEOUT;
    }
    if ( strncmp ( enteredString ,
                   VERIFY_HEALTH_STRING ,
                   strlen ( VERIFY_HEALTH_STRING ) ) == 0 )
    {
        return MIL_LIVENESS_VERIFY;
    }
    if ( strncmp ( enteredString ,
                   DUMP_ALL_STRING ,
                   strlen ( DUMP_ALL_STRING ) ) == 0 )
    {
        return MIL_DATA_DUMP_GET_DATA;
    }
    if ( strncmp ( enteredString ,
                   HELP_DISPLAY_STRING ,
                   strlen ( HELP_DISPLAY_STRING ) ) == 0 )
    {
        return MIL_DISPLAY_HELP;
    }

    if ( strncmp ( enteredString ,
                   SET_FILE_NAME_STRING ,
                   strlen ( SET_FILE_NAME_STRING ) ) == 0 )
    {
        return MIL_FILENAME_SET;
    }

    return MIL_INVALID_COMMAND;

}
/**
*****************************************************************************
* @ingroup MILUserInterface
*  	Display the error message
*
* @description
*  This function takes the mil_error_status_t type as an argument
* and display the corresponding error message.
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
*      Yes
*
* @param err IN the mil_error_status_t type
*
* @pre
*      None.
*
* @post
*      None.
*
***************************************************************************/
void
display_error(mil_error_status_t err)
{
    MIL_USER_DEBUG_PRINT ("[MIL_us:display_error] err is %d ", err);
    if (0 > err) err = MIL_UNKNOWN_ERROR;
    printf ( "(%s)\n", (char *) error_strings[err]);
}

/**
*****************************************************************************
* @ingroup MILUserInterface
*       Display the help message
*
* @description
*  This function when called displays the command options.
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
*      Yes
*
* @param None.
*
* @pre
*      None.
*
* @post
*      None.
*
***************************************************************************/
void
showHelp( void )
{
    const char* exe_name = "debugmgr";
    printf("The following are the supported commands \n" );
    printf("%s %s\n" , exe_name, HELP_DISPLAY_STRING);
    printf("%s %s\n" , exe_name, DEBUG_ENABLE_STRING );
    printf("%s %s\n" , exe_name, DEBUG_DISABLE_STRING );
    printf("%s %s\n" , exe_name, VERSION_DUMP_ALL_STRING );
    printf("%s %s <time-out in miliseconds>\t(timeout range is [100-5000])\n" \
            , exe_name, CONFIGURE_LIVENESS_TIMEOUT_SRING );
    printf ("%s %s\n" , exe_name, VERIFY_HEALTH_STRING );
    printf ("%s %s\n" , exe_name, DUMP_ALL_STRING );
    printf ("%s %s\t\t\t\t(maximum length is %d)\n", \
            exe_name, SET_FILE_NAME_STRING, MAX_FILENAME_SIZE);
    return;
}


/**
*****************************************************************************
* @ingroup MILUserInterface
*       makes necessary steps to enable debug
*
* @description
*  makes necessary steps to enable debug
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
*      Yes
*
* @param None.
*
* @return MIL_SUCCESS for success or appropriate mil_error_status_t
*
* @pre
*      None.
*
* @post
*      None.
*
***************************************************************************/
mil_error_status_t
callDebugEnable(void)
{
    mil_error_status_t status = MIL_SUCCESS;
    mil_error_status_t old_status = MIL_SUCCESS;
    uint8_t buffer[MAX_MESSAGE_SIZE] = {0};
    uint8_t filename[MAX_FILENAME_SIZE] = {0};
    int32_t pAuxPointer = 0;
    int32_t pNullPointer = (int32_t) NULL;

    memset(buffer, 0,  MAX_MESSAGE_SIZE);
    memset(filename, 0,  MAX_FILENAME_SIZE);

    /*Print to File Debug Enable*/
    sprintf((char *)buffer, "%s:Debug Enabled\n", MIL_STATIC_VERSION);
    WriteToFile(buffer);

    /*Call Internal Enable*/
    pAuxPointer = (int32_t) &pNullPointer;
    status = call_ioctl(IOCTL_CMD_MIL_INTERNAL_DEBUG_ENABLE, &pAuxPointer);
    if (status != MIL_SUCCESS )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:callDebugEnable] "\
                            "MIL_INTERNAL_DEBUG_ENABLE failed (%d).\n",
                            status);
        return status;
    }

    status = FileVersionDump();
    if (status != MIL_SUCCESS)
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:callDebugEnable] FileVersionDump "\
                            "failed (%d).\n", status);
        old_status = status;
        pAuxPointer = (int32_t) &pNullPointer;
        status = call_ioctl(IOCTL_CMD_MIL_DEBUG_DISABLE, &pAuxPointer);
        if (status != MIL_SUCCESS)
        {
        MIL_USER_DEBUG_PRINT("[MIL_us:callDebugEnable] "\
                            "IOCTL_CMD_MIL_DEBUG_DISABLE "\
                            "failed (%d).\n", status);
        }
       return old_status;
     } 
    pAuxPointer = (int32_t) &filename;
    status = call_ioctl(IOCTL_CMD_MIL_FILENAME_GET, &pAuxPointer);
    if ( status == MIL_SUCCESS )
    {
        printf ("Current log file is file %s \n", (char *) filename);
    }
    return MIL_SUCCESS;
}


/**
*****************************************************************************
* @ingroup MILUserInterface
*       Depending on command parsed from command line takes actions
*
* @description
*       Depending on command parsed from command line takes actions
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
*      Yes
*
* @param None.
*
* @return MIL_SUCCESS for success or appropriate mil_error_status_t
*
* @pre
*      None.
*
* @post
*      None.
*
***************************************************************************/
/*Depending on command take cations*/
mil_error_status_t
doOperation(mil_commands_t enteredCommand)
{
    mil_error_status_t err = MIL_SUCCESS;
    uint8_t buffer[MAX_MESSAGE_SIZE] = {0};
    uint8_t filename[MAX_FILENAME_SIZE] = {0};
    int32_t pAuxPointer = 0;
    int32_t pNullPointer = (int32_t) NULL;

    memset(buffer, 0,  MAX_MESSAGE_SIZE);
    memset(filename, 0,  MAX_FILENAME_SIZE);

    switch (enteredCommand)
    {
    case MIL_DISPLAY_HELP:
        showHelp();
        err = MIL_SUCCESS ;
        break;

    case MIL_INVALID_COMMAND:
        showHelp();
        err = MIL_FAILURE;
        break;

    case MIL_DEBUG_ENABLE:
        err = callDebugEnable();
        break;

    case MIL_DEBUG_DISABLE:
        /*sen event flag etc.*/
        pAuxPointer = (int32_t) &pNullPointer;
        err = call_ioctl(IOCTL_CMD_MIL_DEBUG_DISABLE, &pAuxPointer);
        if(err != MIL_SUCCESS)
        {
            MIL_USER_DEBUG_PRINT("[MIL_us:doOperation] "\
                                "IOCTL_CMD_MIL_DEBUG_DISABLE failed\n");
            return err;
        }
        /*put msg to file*/
        sprintf((char *)buffer, "%s:%s", MIL_STATIC_VERSION,
                "Debug Disabled\n");
        WriteToFile(buffer);
      
        pAuxPointer = (int32_t) &filename;
        err = call_ioctl(IOCTL_CMD_MIL_FILENAME_GET, &pAuxPointer);
        if ( err == MIL_SUCCESS )
        {
            printf ("Current log file is file %s \n", (char *) filename);
        }
        break;

    case MIL_DEBUG_VERSION_GET_ALL:
        sprintf((char *)buffer, "Version Dump All Triggered\n");
        WriteToFile(buffer);
        err = FileVersionDump();
        break;

    case MIL_LIVENESS_VERIFY:
        err = FileLivenessVerify();
        break;

    case MIL_DATA_DUMP_GET_DATA:
        sprintf((char *)buffer, "Data Dump All Triggered\n");
        WriteToFile(buffer);
        err = FileDataDump();
        break;

    default:
        MIL_USER_DEBUG_PRINT("[MIL_us:doOperation] command not recocnized.\n");
        err = MIL_FAILURE;
        break;
    }
    return err;
}
/**
*****************************************************************************
* @ingroup MILUserInterface
*  	Main function
*
* @description
*  This is main from where the program will start.
* it reads the comamnd and the timeout
* as argument. It does the necessary things to
* complete the task as per the entered command.
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
*      Yes
*
* @param argc IN the number of arguments
* @param argv IN the array of arguments entered at command prompt
*
* @return 0 if everyting goes fine.
-1 if error occur
*
* @pre
*      None.
*
* @post
*      None.
*
***************************************************************************/
int
main(
    int argc ,
    char *argv [] )
{
    mil_commands_t enteredCommand = 0;
    uint32_t timeout_value = 0;
    mil_error_status_t err = MIL_SUCCESS;
    uint8_t filename[MAX_FILENAME_SIZE] = {0};
    int32_t pAuxPointer = 0;

    memset(filename, 0,  MAX_FILENAME_SIZE);

    printf("MIL debugmgr (" MIL_STATIC_VERSION ") - " \
            __DATE__ " - " __TIME__ "\n");

    /*check is arguments are present*/
    if ( argc < MIN_ARG_NUMBER )
    {
        showHelp ();
        exit ( MIL_FAILURE );
    }
    /* Read the argv and parse accordinagly
       return the command to the main*/
    enteredCommand = parseArguments( argv[ COMMAND_ARG ] );

    MIL_USER_DEBUG_PRINT("[MIL_us:main] "\
                        "enteredCommand = %d\n", enteredCommand);
    switch (enteredCommand)
    {
    /*The commands which are processed here takes arguments 
      and hence they are processed here rather than in other function
      to avoid overhead of passing argument*/
    case MIL_LIVENESS_CONFIGURE_TIMEOUT :
        if ( argc < ( MIN_TIMEOUT_ARG_NUMBER) )
        {
            printf ("You must enter timeout value (range is 100 - 5000 ms)\n");
            err = MIL_FAILURE;
        }
        else
        {
            timeout_value = convert_string_to_integer ( argv[ TIMEOUT_ARG ] );
            /* Check timeout value for the range*/
            if ( ( timeout_value < MIN_TIMEOUT_VALUE ) ||
                    ( timeout_value > MAX_TIMEOUT_VALUE ) )
            {
                printf ("Invalid timeout value (range is 100 - 5000 ms)\n");
                err = MIL_FAILURE ;
            }
            else
            {
                pAuxPointer = (int32_t) &timeout_value;
                err = call_ioctl(IOCTL_CMD_MIL_LIVENESS_CONFIGURE_TIMEOUT, \
                        &pAuxPointer);
            }
        }
      break;

    case MIL_FILENAME_SET :
        if ( argc < ( MIN_SETFILENAME_ARG_NUMBER) )
        {
            printf ( "%s - Failed \nYou must enter filename \n" ,
                     argv[ COMMAND_ARG ] );
            exit ( MIL_FAILURE );
        }
        MIL_USER_DEBUG_PRINT ("[MIL_us:main] filename size = %d\n", 
                               strlen(argv[FILENAME_ARG]));
        if (strlen(argv[FILENAME_ARG]) > MAX_FILENAME_SIZE)
        {
            MIL_USER_DEBUG_PRINT("[MIL_us:main] filename is too long "\
                                "(limit = %d)\n", MAX_FILENAME_SIZE);
            err = MIL_FILENAME_SIZE_IS_BIG;
        }
        else
        {
            MIL_USER_DEBUG_PRINT("[MIL_us:main] filename size is OK.\n");
            
            bzero(&filename, MAX_FILENAME_SIZE);
            memcpy(filename, argv[FILENAME_ARG], MAX_FILENAME_SIZE);
            MIL_USER_DEBUG_PRINT("[MIL_us:main] argv[FILENAME_ARG] = %s\n", \
            argv[FILENAME_ARG]);
            MIL_USER_DEBUG_PRINT("[MIL_us:main] filename = %s\n", filename);

            pAuxPointer = (int32_t) &filename;
            err = call_ioctl(IOCTL_CMD_MIL_FILENAME_SET, &pAuxPointer);
            if(err == MIL_SUCCESS)
            {
                printf ("Log filename is set to %s\n", filename);
            }
        }
        break;
    default:
        err = doOperation(enteredCommand);
        break;
    }
    if ( err == MIL_SUCCESS )
    {
        printf ( "%s succeded \n" , argv[ COMMAND_ARG ] );
        exit(0);
    }
    printf ( "%s failed\n", argv[ COMMAND_ARG ] );
    display_error( err);
    exit( err);
}
