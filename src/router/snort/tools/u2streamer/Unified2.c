/* 
 * Copyright(C) 2003 Sourcefire, Inc.   All Rights Reserved
 */

/* System includes */
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#ifdef LINUX
#include <stdint.h>
#endif

/* Sourcefire includes */
#include <sf_error.h>

/* Local includes */
#include "Unified2.h"

int Unified2Record_Destroy(Unified2Record *u2_record)
{
    if(!u2_record)
        return SF_EINVAL;

    free(u2_record->data);

    free(u2_record);

    return SF_SUCCESS;
}


