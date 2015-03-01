
#ifndef _SPOOL_FILE_ITERATOR_H_
#define _SPOOL_FILE_ITERATOR_H_

#include <sys/types.h>
#ifdef LINUX
#include <stdint.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* Sourcefire includes */
#include <sflsq.h>

/* Local includes */
#include <Unified2.h>
#include <Unified2File.h>


#define SPOOL_FILE_TYPE_UNIFIED 1
#define SPOOL_FILE_TYPE_UNIFIED2 2

/* Snort Unfied Log Iterator API **********************************************/
//typedef void SpoolFileIterator;
typedef struct _SpoolFileIterator
{
    /* Configuration data */
    char *directory;
    char *bookmark_file;

    /* Runtime data */
    uint8_t new_file_found;
    uint8_t initialized;
    uint8_t search_mode;
    uint8_t go_to_end;
    uint8_t tail_mode;
    int status;
    int err_status;
    uint32_t timestamp;

    /* Position seek data */
    uint8_t seek_set;
    uint32_t seek_extension;
    uint32_t seek_position;

    /* Current file data */
    uint8_t file_type;
    char *file_prefix;
    uint32_t file_extension; /* Timestamp extension of the current file */
    uint32_t record_number;  /* Record number from the current file */
    Unified2File   *u2f;
    char *filepath;

    /* Bookmarking data */
    int bookmark_fd;

    /* Caching */
    uint16_t event_type;     
    //Unified2IPSEvent *ids_event;
    uint32_t ids_event_record_number;
                    
    //Unified2Packet   *packet;

    uint32_t packet_record_number;

    Unified2ExtraData *extra_data;
    uint32_t extra_data_record_number;

    uint32_t flow_event_record_number;

    Unified2Record *unified2_record;
} SpoolFileIterator;


int SpoolFileIterator_New(const char *directory, char *file_prefix,
        const char *bookmark_file, SpoolFileIterator **iterator);

int SpoolFileIterator_Destroy(SpoolFileIterator *iterator);

int SpoolFileIterator_GetNext(SpoolFileIterator *iterator, 
        Unified2Record **p_record, uint32_t *p_file, uint32_t *p_position);

int SpoolFileIterator_Ack(SpoolFileIterator *iterator);
int SpoolFileIterator_SetPosition(SpoolFileIterator *iterator, 
        uint32_t extension,
        uint32_t record_number);

#endif /* _SPOOL_FILE_ITERATOR_H_ */
