
/* System includes */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/* Local includes */
#include "UnifiedLog.h"


/* Snort Unified Log Record API ***********************************************/
int UnifiedLog_Destroy(UnifiedLog *unified_log)
{
    if(unified_log)
    {
        if(unified_log->packet)
            free(unified_log->packet);
        free(unified_log);
    }
    return 0;
}
