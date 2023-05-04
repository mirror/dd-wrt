/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2013-2013 Sourcefire, Inc.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  4.11.2013 - Initial Source Code. Hcao
 **
 **  File agent uses a separate thread to store files and also sends out
 **  to network. It uses file APIs and provides callbacks.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "sf_types.h"
#include "spp_file.h"
#include "file_agent.h"
#include "mempool.h"
#include "sf_dynamic_preprocessor.h"
#include "circular_buffer.h"
#include "file_sha.h"
#include "sfPolicy.h"

int sockfd = 0;

/*Use circular buffer to synchronize writer/reader threads*/
static CircularBuffer* file_list;

static volatile bool stop_file_capturing = false;
static volatile bool capture_thread_running = false;

static bool file_type_enabled = false;
static bool file_signature_enabled = false;
static bool file_capture_enabled = false;

pthread_t capture_thread_tid;
static pid_t capture_thread_pid;
uint64_t capture_disk_avaiable; /* bytes available */

static pthread_cond_t file_available_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t file_list_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct _FILE_MESSAGE_HEADER
{
    /* All values must be in network byte order */
    uint16_t version;
    uint16_t type;
    uint32_t length;    /* Does not include the header */
    char filename[FILE_NAME_LEN];

} FileMessageHeader;

#define FILE_HEADER_VERSION   0x0001
#define FILE_HEADER_DATA      0x0009

static int file_agent_save_file (FileInfo *, char *);
static int file_agent_send_file (FileInfo *);
static FileInfo* file_agent_get_file(void);
static FileInfo *file_agent_finish_file(void);
static File_Verdict file_agent_type_callback(void*, void*, uint32_t, bool,uint32_t);
static File_Verdict file_agent_signature_callback(void*, void*, uint8_t*,
        uint64_t, FileState *, bool, uint32_t, bool);
static int file_agent_queue_file(void*, void *);
static int file_agent_init_socket(char *hostname, int portno);

/* Initialize sockets for file transfer to other host
 *
 * Args:
 *   char *hostname: host name or IP address of receiver
 *   int portno: port number of receiver
 */
int file_agent_init_socket(char *hostname, int portno)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        FILE_FATAL_ERROR("File inspect: ERROR creating socket!\n");
        return -1;
    }

    /*get the address info by either host name or IP address*/

    server = gethostbyname(hostname);

    if (server == NULL)
    {
        _dpd.errMsg("File inspect: ERROR, no such host\n");
        close(sockfd);
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        _dpd.errMsg("File inspect: ERROR connecting host %s: %d!\n",
                hostname, portno);
        close(sockfd);
        return -1;
    }

    _dpd.logMsg("File inspect: Connection established on host: %s, port: %d\n",
            hostname, portno);

    return 0;
}

/*Send file data to other host*/
static void file_agent_send_data(int socket_fd, const uint8_t *resp,
        uint32_t len)
{
    ssize_t numsent;
    unsigned total_len = len;
    unsigned total = 0;

    do
    {
        numsent = write(socket_fd, (*(uint8_t **)&resp) + total,
                total_len - total);
        if (!numsent)
            return;
        else if (numsent > 0)
            total += numsent;
        else if (errno != EINTR && errno != EAGAIN)
        {
            file_inspect_stats.file_transfer_failures++;
            return;
        }
    } while (total < total_len);
}

/* Process all the files in the file queue*/
static inline void file_agent_process_files(CircularBuffer *file_list,
        char *capture_dir, char *hostname)
{
    while (!cbuffer_is_empty(file_list))
    {
        FileInfo *file;
        file = file_agent_get_file();

        if (file && file->sha256)
        {
            /* Save to disk */
            if (capture_dir)
                file_agent_save_file(file, capture_dir);
            /* Send to other host */
            if (hostname)
                file_agent_send_file(file);
            /* Default, memory only */
        }

        file = file_agent_finish_file();

        if (file)
        {
            _dpd.fileAPI->release_file(file->file_mem);
            _dpd.snortFree(file, sizeof(FileInfo), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
        }
    }
}
/* This is the main thread for file capture,
 * either store to disk or send to network based on setting
 */
static void* FileCaptureThread(void *arg)
{
    FileInspectConf* conf = (FileInspectConf*) arg;
    char *capture_dir = NULL;
    char *hostname = NULL;

#if defined(LINUX) && defined(SYS_gettid)
    capture_thread_pid =  syscall(SYS_gettid);
#else
    capture_thread_pid = getpid();
#endif

    capture_thread_running = true;

    capture_disk_avaiable = conf->capture_disk_size<<20;

    if (conf->capture_dir)
        capture_dir = strdup(conf->capture_dir);
    if (conf->hostname)
        hostname = strdup(conf->hostname);

    while(1)
    {
        file_agent_process_files(file_list, capture_dir, hostname);

        if (stop_file_capturing)
            break;

        pthread_mutex_lock(&file_list_mutex);
        if (cbuffer_is_empty(file_list))
            pthread_cond_wait(&file_available_cond, &file_list_mutex);
        pthread_mutex_unlock(&file_list_mutex);
    }

    if (conf->capture_dir)
        free(capture_dir);
    if (conf->hostname)
        free(hostname);
    capture_thread_running = false;
    return NULL;
}

void file_agent_init(struct _SnortConfig *sc, void *config)
{
    FileInspectConf* conf = (FileInspectConf *)config;

    /*Need to check configuration to decide whether to enable them*/

    if (conf->file_type_enabled)
    {
        _dpd.fileAPI->enable_file_type(sc, file_agent_type_callback);
        file_type_enabled = true;
    }
    if (conf->file_signature_enabled)
    {
        _dpd.fileAPI->enable_file_signature(sc, file_agent_signature_callback);
        file_signature_enabled = true;
    }

    if (conf->file_capture_enabled)
    {
        _dpd.fileAPI->enable_file_capture(sc, file_agent_signature_callback);
        file_capture_enabled = true;
    }

    if (!sockfd && conf->hostname)
    {
        file_agent_init_socket(conf->hostname, conf->portno);
    }
}

/* Add another thread for file capture to disk or network
 * When settings are changed, snort must be restarted to get it applied
 */
void file_agent_thread_init(struct _SnortConfig *sc, void *config)
{
    int rval;
    const struct timespec thread_sleep = { 0, 100 };
    sigset_t mask;
    FileInspectConf* conf = (FileInspectConf *)config;

    /* Spin off the file capture handler thread. */
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGPIPE);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGURG);
    sigaddset(&mask, SIGVTALRM);

    pthread_sigmask(SIG_SETMASK, &mask, NULL);

    file_list = cbuffer_init(conf->file_capture_queue_size);

    if(!file_list)
    {
        FILE_FATAL_ERROR("File capture: Unable to create file capture queue!");
    }

    if ((rval = pthread_create(&capture_thread_tid, NULL,
            &FileCaptureThread, conf)) != 0)
    {
        sigemptyset(&mask);
        pthread_sigmask(SIG_SETMASK, &mask, NULL);
        FILE_FATAL_ERROR("File capture: Unable to create a "
                "processing thread: %s", strerror(rval));
    }

    while (!capture_thread_running)
        nanosleep(&thread_sleep, NULL);

    sigemptyset(&mask);
    pthread_sigmask(SIG_SETMASK, &mask, NULL);
    _dpd.logMsg("File capture thread started tid=%p (pid=%u)\n",
            (void *) capture_thread_tid, capture_thread_pid);

}

/*
 * Files are queued in a list
 * Add one file to the list
 */
static int file_agent_queue_file(void* ssnptr, void *file_mem)
{
    FileInfo *finfo;
    char *sha256;

    if (cbuffer_is_full(file_list))
    {
        return -1;
    }

    finfo = _dpd.snortAlloc(1, sizeof(FileInfo), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);

    if (!finfo)
    {
        return -1;
    }

    sha256 = (char *) _dpd.fileAPI->get_sig_sha256(ssnptr);

    if (!sha256)
    {
        _dpd.snortFree(finfo, sizeof(FileInfo), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
        return -1;
    }

    memcpy(finfo->sha256, sha256, SHA256_HASH_SIZE);
    finfo->file_mem = file_mem;
    finfo->file_size = _dpd.fileAPI->get_file_capture_size(file_mem);

    pthread_mutex_lock(&file_list_mutex);

    if (cbuffer_write(file_list, finfo)) 
    {
        pthread_mutex_unlock(&file_list_mutex);
        _dpd.snortFree(finfo, sizeof(FileInfo), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
        return -1;
    }

    pthread_cond_signal(&file_available_cond);
    pthread_mutex_unlock(&file_list_mutex);

    return 0;
}

/*
 * Files are queued in a list
 * Get one file from the list
 */
static FileInfo* file_agent_get_file(void)
{
    ElemType file;

    if(cbuffer_peek(file_list, &file))
    {
        return NULL;
    }

    return (FileInfo*) file;
}

/*
 * Files are queued in a list
 * Remove one file from the list
 * The file in head is removed
 */
static FileInfo* file_agent_finish_file(void)
{
    ElemType file;

    if(cbuffer_read(file_list, &file))
    {
        return NULL;
    }

    return (FileInfo*) file;
}

/*
 * writing file to the disk.
 *
 * In the case of interrupt errors, the write is retried, but only for a
 * finite number of times.
 *
 * Arguments
 *  uint8_t *: The buffer containing the data to write
 *  size_t:  The length of the data to write
 *  FILE *fh:  File handler
 *
 * Returns: None
 *
 */
static void file_agent_write(uint8_t *buf, size_t buf_len, FILE *fh)
{
    int max_retries = 3;
    size_t bytes_written = 0;
    int err;

    /* Nothing to write or nothing to write to */
    if ((buf == NULL) || (fh == NULL))
        return;

    /* writing several times */
    do
    {
        size_t bytes_left = buf_len - bytes_written;

        bytes_written += fwrite(buf + bytes_written, 1, bytes_left, fh);

        err = ferror(fh);
        if (err && (err != EINTR) && (err != EAGAIN))
        {
            break;
        }

        max_retries--;

    } while ((max_retries > 0) && (bytes_written < buf_len));

    if (bytes_written < buf_len)
    {
        _dpd.errMsg("File inspect: disk writing error - %s!\n", strerror(err));
    }
}

/* Store files on local disk
 */
static int file_agent_save_file(FileInfo *file,  char *capture_dir)
{
    FILE *fh;
    struct stat   buffer;
    char filename[FILE_NAME_LEN + 1];
    int filename_len;
    char *findex = filename;
    uint8_t *buff;
    int size;
    void *file_mem;

    filename_len = snprintf(filename, FILE_NAME_LEN, "%s", capture_dir);

    if (filename_len >= FILE_NAME_LEN )
    {
        _dpd.snortFree(file, sizeof(FileInfo), PP_FILE_INSPECT, PP_MEM_CATEGORY_SESSION);
        return -1;
    }

    file_inspect_stats.files_to_disk_total++;

    findex += filename_len;

    filename_len = sha_to_str(file->sha256, findex,
            FILE_NAME_LEN - filename_len);

    /*File exists*/
    if(stat (filename, &buffer) == 0)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FILE, "File exist: %s\n", filename););
        file_inspect_stats.file_duplicates_total++;
        return -1;
    }

    if (!capture_disk_avaiable)
    {
        return -1;
    }
    else if (capture_disk_avaiable < file->file_size)
    {
        capture_disk_avaiable = 0;
        _dpd.errMsg("File inspect: exceeding allocated disk size, "
                "can't store file!\n");
        return -1;
    }
    else
    {
        capture_disk_avaiable -= file->file_size;
    }

    fh = fopen(filename, "w");
    if (!fh )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Can't create file: %s\n",
                filename););
        return -1;
    }

    file_mem = file->file_mem;

    /*Check the file buffer*/
    while (file_mem)
    {
        file_mem = _dpd.fileAPI->read_file(file_mem, &buff, &size);
        /*Get file from file buffer*/
        if (!buff || !size )
        {
            file_inspect_stats.file_read_failures++;
            _dpd.logMsg("File inspect: can't read file!\n");
            return -1;
        }

        file_agent_write(buff, size, fh);
    }

    fclose(fh);

    file_inspect_stats.files_saved++;
    file_inspect_stats.file_data_to_disk += file->file_size;

    return 0;
}

/* Send file data to other host*/
static int file_agent_send_file(FileInfo *file)
{
    /*Save the file*/
    FileMessageHeader fheader;

    void *file_mem;
    uint8_t *buff;
    int size;

    if (!sockfd)
    {
        return 0;
    }

    /*Send the file name*/
    fheader.version = htons(FILE_HEADER_VERSION);
    fheader.type = htons(FILE_HEADER_DATA);
    fheader.length = htonl(file->file_size);

    memset(fheader.filename, 0, sizeof(fheader.filename));

    sha_to_str(file->sha256, fheader.filename, sizeof (fheader.filename));

    file_agent_send_data (sockfd, (uint8_t *)&fheader, sizeof(fheader));

    DEBUG_WRAP(DebugMessage(DEBUG_FILE, "sent file: %s, with size: %d\n",
            fheader.filename, file->file_size););

    file_mem = file->file_mem;

    /*Check the file buffer*/
    while (file_mem)
    {
        file_mem = _dpd.fileAPI->read_file(file_mem, &buff, &size);
        /*Get file from file buffer*/
        if (!buff || !size )
        {
            file_inspect_stats.file_read_failures++;
            _dpd.logMsg("File inspect: can't read file!\n");
            return -1;
        }

        file_agent_send_data(sockfd, buff, size);
    }

    file_inspect_stats.files_to_host_total++;
    file_inspect_stats.file_data_to_host += file->file_size;

    return 0;
}

/* Close file agent
 * 1) stop capture thread: waiting all files queued to be captured
 * 2) free file queue
 * 3) close socket
 */
void file_agent_close(void)
{
    int rval;

    stop_file_capturing = true;

    pthread_mutex_lock(&file_list_mutex);
    pthread_cond_signal(&file_available_cond);
    pthread_mutex_unlock(&file_list_mutex);

    if ((rval = pthread_join(capture_thread_tid, NULL)) != 0)
    {
        FILE_FATAL_ERROR("Thread termination returned an error: %s\n",
                strerror(rval));
    }

    while(capture_thread_running)
        sleep(1);

    cbuffer_free(file_list);

    if (sockfd)
    {
        close(sockfd);
        sockfd = 0;
    }
}

/*
 * File type callback when file type is identified
 *
 * For file capture or file signature, FILE_VERDICT_PENDING must be returned
 */
static File_Verdict file_agent_type_callback(void* p, void* ssnptr,
        uint32_t file_type_id, bool upload, uint32_t file_id)
{
    file_inspect_stats.file_types_total++;
    if (file_signature_enabled || file_capture_enabled)
        return FILE_VERDICT_UNKNOWN;
    else
        return FILE_VERDICT_LOG;
}

static inline int file_agent_capture_error(FileCaptureState capture_state)
{
    if (capture_state != FILE_CAPTURE_SUCCESS)
    {
        file_inspect_stats.file_reserve_failures++;

        _dpd.logMsg("File inspect: can't reserve file!\n");
        switch(capture_state)
        {
        case FILE_CAPTURE_MIN:
            file_inspect_stats.file_capture_min++;
            break;
        case FILE_CAPTURE_MAX:
            file_inspect_stats.file_capture_max++;
            break;
        case FILE_CAPTURE_MEMCAP:
            file_inspect_stats.file_capture_memcap++;
            break;
        default:
            break;
        }
        return 1;
    }
    return 0;
}

/*
 * File signature callback when file transfer is completed
 * or capture/singature is aborted
 */
static File_Verdict file_agent_signature_callback (void* p, void* ssnptr,
        uint8_t* file_sig, uint64_t file_size, FileState *state, bool upload, uint32_t file_id, bool is_partial)
{
    FileCaptureInfo *file_mem = NULL;
    FileCaptureState capture_state;
    File_Verdict verdict = FILE_VERDICT_UNKNOWN;
    FileInspectConf *conf = sfPolicyUserDataGetDefault(file_config);
    uint64_t capture_file_size;

    SFSnortPacket *pkt = (SFSnortPacket*)p;

    file_inspect_stats.file_signatures_total++;

    if (conf && file_sig)
    {
        FileSigInfo *file_verdict;
        file_verdict = (FileSigInfo *)sha_table_find(conf->sig_table, file_sig);
        if (file_verdict)
        {
#if defined(DEBUG_MSGS) || defined (REG_TEST)
            static int verdict_delay = 0;
            if ((verdict_delay++) < conf->verdict_delay)
            {
                verdict = FILE_VERDICT_PENDING;
            }
            else
#endif
                verdict = file_verdict->verdict;
        }
    }

    if (!file_capture_enabled)
        return verdict;

    /* Check whether there is any error during processing file*/
    if (state->capture_state != FILE_CAPTURE_SUCCESS)
    {
        if (state->sig_state != FILE_SIG_PROCESSING)
            file_agent_capture_error(state->capture_state);
        return verdict;
    }

    /* Reserve buffer for file capture */
    capture_state = _dpd.fileAPI->reserve_file(ssnptr, &file_mem);

    /*Check whether there is any error for the last piece of file*/
    if (file_agent_capture_error(capture_state))
    {
        return verdict;
    }

    /* Check file size */
    capture_file_size = _dpd.fileAPI->get_file_capture_size(file_mem);
    if (file_size != capture_file_size)
    {
        _dpd.logMsg("File inspect: file size error %d != %d\n",
                file_size, capture_file_size);
    }

    /*Save the file to our file queue*/
    if ((!is_partial) && (file_agent_queue_file(pkt->stream_session, file_mem) < 0))
    {
        file_inspect_stats.file_agent_memcap_failures++;
        _dpd.logMsg("File inspect: can't queue file!\n");
        return verdict;
    }

    return verdict;
}

