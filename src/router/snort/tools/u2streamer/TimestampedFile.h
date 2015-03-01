
#ifndef __TIMESTAMPED_FILE_H__
#define __TIMESTAMPED_FILE_H__

#include <sys/types.h>
#include "sf_types.h"


/**
 * Find the next file using the timestamp extension.
 *
 * Search the specified directory for files matching file_prefix.  The filename
 * information after the prefix is interpreted as a unix timestamp.  Select
 * the oldest file that is after or within the specified timestamp (depending
 * on the mode) and returns the timestamp value to the user.  If mode is set
 * to 1, we will return a timestamp equal or greater than the search timestamp.
 * If mode is set to 0, we will return a timestamp less than or equal to the
 * specified timestamp (this will indicate the file that is most likely to 
 * contain data for the specified timestamp).
 *
 * @param directory         The directory to scan
 * @param file_prefix       The file prefix
 * @param timestamp         search timestamp
 * @param mode              operational mode (1 or 0)
 * @param next_timestamp    return reference for the selected timestamp
 *
 * @retval SF_SUCCESS       search succeeded
 * @retval SF_EINVAL        invalid argument
 * @retval SF_ENOENT        no files found
 * @retval SF_EOPEN         unable to open directory
 * @retval SF_EREAD         unbale to read from directory
 */
int FindNextTimestampedFile(char *directory, char *file_prefix,
        uint32_t timestamp, int after, uint32_t *next_timestamp);

#endif /* __TIMESTAMPED_FILE_H__ */
