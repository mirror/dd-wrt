
#ifndef __UNIFIED2_FILE_H__
#define __UNIFIED2_FILE_H__

/*! \defgroup Unified2File
 */

/** \addtogroup Unified2File */
/*@{*/
typedef struct _Unified2File
{
    int fd; 

    int read_status;
    int read_errno;
    int read_offset;

    Serial_Unified2_Header s_u2_hdr;
    Serial_Unified2HeaderExtension s_u2_hdr_ext;
    uint32_t checksum;
    Unified2Record *u2_record;

} Unified2File;


int Unified2File_Open(char *filepath, Unified2File **u2_file);

/* 
 * @retval SF_SUCCESS          record read
 * @retval SF_ENOMEM           out of memory
 * @retval SF_EINVAL           invalid argument
 * @retval SF_EREAD            read error
 * @retval SF_EREAD_TRUNCATED  end of file while reading record
 * @retval SF_EREAD_PARTIAL    partial read while reading record
 * @retval SF_END_OF_FILE      end of file on record boundary
 * @retval -1                  should never be reached
 */
int Unified2File_Read(Unified2File *u2_file, Unified2Record **u2_record);

int Unified2File_Close(Unified2File *u2_file);


/*@}*/
#endif /* __UNIFIED2_FILE_H__ */

