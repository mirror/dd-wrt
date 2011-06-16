/**
 **************************************************************************
 * @file icp_asd_ctl.c
 *
 * @ingroup ASD
 *
 * @description
 *      This file contains the code for the ASD CTL user space program.
 *      On boot-up, reads a configuration file, validates each parameter pair
 *      specified and passes the user-specified parameters into the kernel.
 *
 * @par
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
 **************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <syslog.h>
#include "icp_asd_cfg.h"
#include "asd_ctl.h"
#include "asd_cfg_table.h"

/*
 * Length of the 0x or 0X prefix
 */
#define LENGTH_OF_HEX_PREFIX 2

/*
 * Number of arguments expected to be read from the config file
 * i.e. a parameter and a value.
 */
#define NUM_EXPECTED_ARGS 2

#define BASE10 10
#define BASE16 16

#define SUCCESS 0
#define FAIL    1

#define ASD_CTL_DEVICE "/dev/icp_asdctl"

/*
 * The number of times an open of the "/dev/icp_asdctl" will be attempted
 */
#define NUM_OPEN_ATTEMPTS 10

STATIC int asd_read_config_file();
STATIC int parse_parameter_pair(char * parameter_pair_ptr,
                                char * param_name, char * param_value);
STATIC int validate_param_name(char * param_name,
                               icp_asd_cfg_param_t * param_id );
STATIC int validate_param_value(char * str_param_value, 
                                int * numeric_param_value);
STATIC int add_parameter_pair_to_config_list(icp_asd_cfg_param_t param_id,
                                             int param_value);
/*
 * The following global variable configuration parameter list is populated with
 * the configuration parameters and their respective values specified in the 
 * icp_asd.conf file and passed to the kernel to update the Configuration Table
 */
config_parameter_list_t asd_cfg_param_list;

int MAIN(int argc, char* argv[])
{
    int return_value=0;
    int fd=-1;
    int num_retries=0;

    /*
     * Open the icp_asdctl device
     *
     * As the device node is created by uDev when the ASD module is loaded,
     * it may take a few seconds for the device node to appear in the /dev tree
     * The following retry mechanism is in place to cater for this scenario.
     */

    while ((num_retries < NUM_OPEN_ATTEMPTS) && (fd < 0))
    {
        sleep(1);
        fd = open(ASD_CTL_DEVICE, O_RDWR);
        num_retries++;
    }

    if (fd < 0)
    {
        syslog(LOG_ERR, "Error: Could not open ASD CTL device\n");
        return FAIL;
    }

    /*
     * Initialise the number of parameters in the config param list to zero.
     */
    asd_cfg_param_list.number_of_params = 0;

    /*
     * Read the configuration file
     */
    return_value = asd_read_config_file();
    if (SUCCESS != return_value)
    {
        syslog(LOG_ERR,"Error: Error reading config file\n");
        close(fd);
        return FAIL;
    }

    syslog(LOG_DEBUG, "Num params %d\n", asd_cfg_param_list.number_of_params);

    /*
     * IOCTL
     */
    return_value = ioctl(fd, IOCTL_CONFIG_SYS_RESOURCE_PARAMETERS,
                         &asd_cfg_param_list);

    if (SUCCESS != return_value)
    {
        syslog(LOG_WARNING, "Error returned from ioctl: %d \n", return_value);
    }
    else
    {
        syslog(LOG_DEBUG, "success returned from ioctl\n");
    }

    /*
     * Close the icp_asdctl device
     */
    close(fd);

    return SUCCESS;
}

/******************************************************************************
 * remove_newline
 *
 * Function to remove the newline character from the input string
 *
 ******************************************************************************/
void remove_newline(char * str_ptr)
{
    int i=0;

    for (i=0; i<MAX_LINE_SIZE; i++)
    {
        if ('\n' == str_ptr[i])
        {
            str_ptr[i] = '\0';
            return;
        }
    }
}

/******************************************************************************
 * remove_white_spaces
 *
 * Function to remove the white space characeter from the start and end
 * of the input string.
 *
 ******************************************************************************/
char * remove_white_spaces(char * string_ptr)
{
    char * start_ptr=string_ptr;
    char * end_ptr=NULL;

    if (NULL == string_ptr)
    {
        return NULL;
    }

    /*
     * Remove spaces from the start of the string
     */
    while (*start_ptr && isspace((int)*start_ptr))
    {
        start_ptr++;
    }

    /*
     * Remove spaces from the end of the string
     */

    end_ptr   = (char*)( (int)start_ptr + (strlen(start_ptr)-1));

    while (*end_ptr && isspace((int)*end_ptr) )
    {
        *end_ptr='\0';
        end_ptr--;
    }

    return start_ptr;
}


/******************************************************************************
 * asd_read_config_file
 *
 * Function which reads the icp_asd.conf file, parses and validates each entry
 * in the config file and adds each valid parameter, value pair to the asd
 * config list which will be passed to the device driver.
 *
 ******************************************************************************/
int asd_read_config_file(void)
{
    FILE * fp=NULL;
    int is_valid_name = FALSE;
    int is_valid_value = FALSE;
    icp_asd_cfg_param_t param_id = ICP_ASD_CFG_PARAM_INVALID;
    int numeric_param_value = 0;
    int line_number=0;
    int return_value=0;
    char * ptr=NULL;
    char parameter_pair_string[MAX_LINE_SIZE];
    char param_value[MAX_PARAM_VALUE_STR_LEN];
    char param_name[MAX_PARAM_NAME_STR_LEN];
    /*
     * Open the config file
     */
    fp = fopen(CONFIG_FILE, "r");

    if (NULL == fp)
    {
        syslog(LOG_ERR, "Error: Could not open configuration file: %s \n",
                         CONFIG_FILE);
        return FAIL;
    }

    /*
     * Read each line in the config file until end of file is reached
     */
    while ( NULL != (fgets(parameter_pair_string, MAX_LINE_SIZE, fp)))
    {
        /*
         * Increment the line number, this is used for error reporting
         */
        line_number++;

        /*
         * Remove white spaces from the start and end of the input string
         */
        ptr = remove_white_spaces(parameter_pair_string);

        if (ptr == NULL){
            continue;
        }

        /*
         * Ignore comment lines and blank lines
         */
        if ( ('#' == *ptr) || (0 == *ptr) )
        {
            continue;
        }
        else
        {
            /*
             * Parse the input string to extract the parameter name and
             * parameter value in there "string" format
             */
            return_value = parse_parameter_pair(ptr, param_name, param_value);
            if (SUCCESS != return_value)
            {
                syslog(LOG_WARNING,"%s:%d: Parse Error - skipping line\n",
                       CONFIG_FILE, line_number);
                continue;
            }

            /*
             * Validate the parameter name, converting it to its
             * icp_asd_cfg_param_t type if it is a valid parameter
             */
            is_valid_name = validate_param_name(param_name, &param_id);
            if (FALSE == is_valid_name)
            {
                syslog(LOG_WARNING,"%s:%d: Validation Error - Unknown "
                                   "parameter: '%s' - skipping line \n",
                                    CONFIG_FILE, line_number, param_name);
                continue;
            }
            else
            {
                /* 
                 * Validate the parameter value, converting it to its
                 * icp_asd_cfg_value_t type if it is a valid parameter value
                 */
                is_valid_value = validate_param_value(param_value,
                                                      &numeric_param_value);

                if (FALSE == is_valid_value)
                {
                    syslog(LOG_WARNING,"%s:%d: Validation Error- Invalid value"
                          " '%s' specified for parameter %s - skipping line \n",
                           CONFIG_FILE, line_number, param_value,param_name);
                    continue;
                }
            }
            /*
             * Add the parameter pair to the config list
             */
            return_value = add_parameter_pair_to_config_list(param_id,
                                                          numeric_param_value);

            if (SUCCESS != return_value)
            {
                syslog(LOG_WARNING, "Failed to add the parameter pair to the "
                       "config list - number of parameters specified exceeds "
                        "limit \n");
            }
        }
    }

    /*
     * Close the configuration file
     */
    fclose(fp);

    return SUCCESS;
}

/******************************************************************************
 * parse_parameter_pair
 *
 * Extract the parameter name and parameter value from the input string
 *
 ******************************************************************************/
STATIC int parse_parameter_pair(char * parameter_pair_ptr,
                              char * param_name, char * param_value)
{

    if (NULL == parameter_pair_ptr)
    {
        return FAIL;
    }
    if ((sscanf(parameter_pair_ptr,"%[^=] = %[^\n]", param_name, param_value) !=
                                                            NUM_EXPECTED_ARGS)||
        (sscanf (parameter_pair_ptr,"%[^=] = %[^#]", param_name, param_value) != 
                                                            NUM_EXPECTED_ARGS) )
    {
        return FAIL;
    }

    return SUCCESS;
}

/******************************************************************************
 * validate_param_name
 *
 * Function which validates the parameter specified in the input string and
 * converts it to its icp_asd_cfg_param_t type.
 *
 ******************************************************************************/
STATIC int validate_param_name(char * param_name, 
                               icp_asd_cfg_param_t * param_id)
{
    int index=0;
    char * str_ptr=NULL;

    if (NULL == param_name)
    {
        return FALSE;
    }

    /*
     * Remove newline and white space characters
     */
    str_ptr = remove_white_spaces(param_name);

    /*
     * Search for the specified parameter in the config parameter string
     * mapping list
     */
    for (index=0; index<ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS; index++)
    {
        if (0 == strcmp(str_ptr,
                    config_parameter_string_mapping_list[index].param_string))
        {
            /*
             * Valid parameter, retrieve the icp_asd_cfg_param_t type from the
             * list for the specified parameter
             */
            *param_id = config_parameter_string_mapping_list[index].cfg_param;
            return TRUE;
        }
    }

    /*
     * Return FALSE, not a valid parameter
     */
    return FALSE;
}

/******************************************************************************
 * is_decimal
 *
 * Function which determines if the specified input string is a decimal value
 *
 ******************************************************************************/
int is_decimal(char *str_ptr)
{
    int index=0;

    /*
     * Check if each character is the string is a digit
     */
    for(index=0; index < ((strlen(str_ptr))); index++)
    {
        if (FALSE == (isdigit(str_ptr[index]))) 
        {
            /*
             * Return FALSE, not a decimal value
             */
            return FALSE;
        }
    }

    /*
     * Return TRUE, a decimal value
     */

    return TRUE;
}

/******************************************************************************
 * is_hexadecimal
 *
 * Function which determines if the specified input string is a hexadecimal
 * value
 *
 ******************************************************************************/
int is_hexadecimal(char * str_value_ptr)
{
    int index=0;
    char * str_ptr=NULL;

    /*
     * If the string starts with a "0x" or a "0X" to signify that it's a
     * hexadecimal number, increment the str_ptr beyond this.
     */
    if ( (str_value_ptr[0] == '0') &&
         ((str_value_ptr[1] == 'x') || (str_value_ptr[1] == 'X') ) ) 
    {
        str_ptr = str_value_ptr+ LENGTH_OF_HEX_PREFIX;
    }
    else
    {
        str_ptr = str_value_ptr;
    }

    /*
     * Check that each character in the string is a valid hex character i.e.
     * A to F or a to f or 0 to 9
     */
    for(index=0; index < ((strlen(str_ptr))); index++)
    {
        if (FALSE == ( ((str_ptr[index] >= 'a') && (str_ptr[index] <= 'f')) ||
                       ((str_ptr[index] >= 'A') && (str_ptr[index] <= 'F')) ||
                       ((str_ptr[index] >= '0') && (str_ptr[index] <= '9')) ) )
        {

        /*
         * Return FALSE - not a hexadecimal value
         */
            return FALSE;
        }
    }

    /*
     * Return TRUE - a hexadecimal value
     */

    return TRUE;
}

/******************************************************************************
 * validate_param_value
 *
 * Function which validates the parameter value specified in the input string
 * and converts it to its icp_asd_cfg_value_t type.
 *
 ******************************************************************************/
STATIC int validate_param_value(char * str_param_value, 
                                int * numeric_param_value)
{
    char * str_ptr=NULL;

    if (NULL == str_param_value)
    {
        return FALSE;
    }

    /*
     * Remove newline and white space characters
     */
    remove_newline(str_param_value);
    str_ptr = remove_white_spaces(str_param_value);

    /*
     * If the input string contains a decimal value
     *   - Convert the value to a decimal value
     * Else If the input string contains a hexadecimal value
     *   - Convert the value to a hexadecimal value
     * Else
     *   - Not a valid value
     */

    if (TRUE == is_decimal(str_ptr))
    {
        /*
         * Convert the string to a decimal value
         */
        *numeric_param_value = strtol(str_ptr, NULL , BASE10);
    }
    else if (TRUE == is_hexadecimal(str_ptr))
    {
        /*
         * Convert the string to a hexadecimal value
         */
        *numeric_param_value = strtol(str_ptr, NULL, BASE16);
    }
    else
    {
        /*
         * Return FALSE, not a valid value
         */
        return FALSE;
    }

    /*
     * Return TRUE, a valid value
     */
    return TRUE;
}

/******************************************************************************
 * add_parameter_pair_to_config_list
 *
 * Function to add the specified parameter pair to the global variable parameter
 * config list and increment the number of parameters in the list.
 *
 ******************************************************************************/
STATIC int add_parameter_pair_to_config_list(icp_asd_cfg_param_t param_id,
                                             int param_value)
{

    /*
     * Ensure the number of parameters specified doesn't exceed the total number
     * of parameters.
     */

    if ( asd_cfg_param_list.number_of_params  >=
                                 ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS )
    {
        return FAIL;
    }

    asd_cfg_param_list.config_param_list[asd_cfg_param_list.
                                       number_of_params].param = param_id;
    asd_cfg_param_list.config_param_list[asd_cfg_param_list.
                                       number_of_params].value = param_value;

    syslog(LOG_DEBUG, "Adding %d, %d pair to config list; ",
          (int)asd_cfg_param_list.config_param_list[asd_cfg_param_list.
                                                   number_of_params].param,
          (int)asd_cfg_param_list.config_param_list[asd_cfg_param_list.
                                                   number_of_params].value);

    asd_cfg_param_list.number_of_params++;

    return SUCCESS;
}
