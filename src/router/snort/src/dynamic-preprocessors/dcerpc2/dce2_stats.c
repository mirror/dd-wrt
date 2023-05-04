/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "dce2_stats.h"
#include "dce2_utils.h"
#include "dce2_memory.h"
#include "dce2_config.h"
#include <string.h>

/********************************************************************
 * Global variables
 ********************************************************************/
DCE2_Stats dce2_stats;
char **dce2_trans_strs = NULL;

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static inline void DCE2_CreateTransStr(char **, DCE2_TransType, char *);

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_StatsInit(void)
{
    memset(&dce2_stats, 0, sizeof(dce2_stats));

    if (dce2_trans_strs == NULL)
    {
        DCE2_TransType ttype;

        dce2_trans_strs = (char **)DCE2_Alloc((DCE2_TRANS_TYPE__MAX * sizeof(char *)), DCE2_MEM_TYPE__INIT);
        if (dce2_trans_strs == NULL)
        {
            DCE2_Die("%s(%d) Failed to allocate memory for transport string "
                     "array", __FILE__, __LINE__);
        }

        for (ttype = DCE2_TRANS_TYPE__NONE; ttype < DCE2_TRANS_TYPE__MAX; ttype++)
        {
            switch (ttype)
            {
                case DCE2_TRANS_TYPE__NONE:
                    break;

                case DCE2_TRANS_TYPE__SMB:
                    {
                        char *str = "SMB";
                        DCE2_CreateTransStr(dce2_trans_strs, ttype, str);
                    }

                    break;

                case DCE2_TRANS_TYPE__TCP:
                    {
                        char *str = "TCP";
                        DCE2_CreateTransStr(dce2_trans_strs, ttype, str);
                    }

                    break;

                case DCE2_TRANS_TYPE__UDP:
                    {
                        char *str = "UDP";
                        DCE2_CreateTransStr(dce2_trans_strs, ttype, str);
                    }

                    break;

                case DCE2_TRANS_TYPE__HTTP_PROXY:
                    {
                        char *str = "HTTP Proxy";
                        DCE2_CreateTransStr(dce2_trans_strs, ttype, str);
                    }

                    break;

                case DCE2_TRANS_TYPE__HTTP_SERVER:
                    {
                        char *str = "HTTP Server";
                        DCE2_CreateTransStr(dce2_trans_strs, ttype, str);
                    }

                    break;

                default:
                    DCE2_Die("%s(%d) Invalid transport type for allocing "
                             "transport strings: %d", __FILE__, __LINE__, ttype);
                    break;
            }
        }
    }
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static inline void DCE2_CreateTransStr(char **trans_buf, DCE2_TransType ttype, char *trans_str)
{
    if ((trans_buf == NULL) || (trans_str == NULL))
        return;

    trans_buf[ttype] = (char *)DCE2_Alloc(strlen(trans_str) + 1, DCE2_MEM_TYPE__INIT);
    if (trans_buf[ttype] == NULL)
    {
        DCE2_Die("%s(%d) Failed to allocate memory for transport string",
                 __FILE__, __LINE__);
    }

    snprintf(trans_buf[ttype], strlen(trans_str) + 1, "%s", trans_str);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_StatsFree(void)
{
    if (dce2_trans_strs != NULL)
    {
        unsigned int i;

        for (i = DCE2_TRANS_TYPE__NONE; i < DCE2_TRANS_TYPE__MAX; i++)
        {
            if (dce2_trans_strs[i] != NULL)
            {
                DCE2_Free((void *)dce2_trans_strs[i],
                          strlen(dce2_trans_strs[i]) + 1, DCE2_MEM_TYPE__INIT);
            }
        }

        DCE2_Free((void *)dce2_trans_strs, (DCE2_TRANS_TYPE__MAX * sizeof(char *)), DCE2_MEM_TYPE__INIT);

        dce2_trans_strs = NULL;
    }
}

