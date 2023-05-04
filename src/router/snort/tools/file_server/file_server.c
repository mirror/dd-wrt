/*
**
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**  Copyright (C) 2012-2013 Sourcefire, Inc.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
**  Author(s):  Hui Cao <hcao@sourcefire.com>
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/wait.h>

#define FILE_CAPTURE_SIZE          10485760 /*10M*/
#define VERBOSE_MODE_KEYWORD       "-v"
#define STD_BUF                    1024

typedef struct _THREAD_ELEMENT
{
    struct _THREAD_ELEMENT *next;
    int socket_fd;
} ThreadElement;

typedef enum
{
    PRINT_MODE_FAST,
    PRINT_MODE_DETAIL
} PrintMode;

static PrintMode print_mode = PRINT_MODE_FAST;
static int daemon_mode = 0;

static int exit_signal = 0;

int stop_processing = 0;

#define FILE_NAME_LEN  200

typedef void (*sighandler_t)(int);

typedef struct _FILE_MESSAGE_HEADER
{
    /* All values must be in network byte order */
    uint16_t version;
    uint16_t type;
    uint32_t length;    /* Does not include the header */
    char filename[FILE_NAME_LEN];
} FileMessageHeader;

#define FILE_HEADER_VERSION   0x0001

typedef struct _File_Storage_Stats
{
    int file_count;
    int file_storage_failures;
    int file_duplicates_total;
} File_Storage_Stats;

static File_Storage_Stats file_stats;

static void CheckExit(void);
static void LogMessage(const char *format,...);
static void ErrorMessage(const char *format,...);

static int ReadHeader(int socket_fd, FileMessageHeader *hdr)
{
    ssize_t numread;
    unsigned total = 0;

    do
    {
        numread = read(socket_fd, ((unsigned char *)hdr) + total,
                sizeof(*hdr) - total);
        if (!numread)
            return 0;
        else if (numread > 0)
            total += numread;
        else if (errno != EINTR && errno != EAGAIN)
            return -1;
    } while (total < sizeof(*hdr) );

    if (total < sizeof(*hdr))
        return 0;

    hdr->length = ntohl(hdr->length);
    hdr->type = ntohs(hdr->type);
    hdr->version = ntohs(hdr->version);

    LogMessage("Receiving file %s, length: %d\n", hdr->filename, hdr->length);

    return 1;
}

static int ReadData(int socket_fd, uint8_t *buffer, uint32_t length)
{
    ssize_t numread;
    unsigned total = 0;

    do
    {
        numread = read(socket_fd, buffer + total, length - total);
        if (!numread)
            return 0;
        else if (numread > 0)
            total += numread;
        else if (errno != EINTR && errno != EAGAIN)
            return -1;
    } while (total < length);

    if (total < length)
        return 0;

    return 1;
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
static void WriteFile(const uint8_t *buf, size_t buf_len, const char *file_name)
{
    int max_retries = 3;
    size_t bytes_written = 0;
    int err;
    char filename[1024];
    FILE *fh;
    struct stat   buffer;

    /*save the file*/
    sprintf(filename, "%s", file_name);

    filename[sizeof (filename) - 1] = '\0';

    /*File exists*/
    if(stat (filename, &buffer) == 0)
    {

        LogMessage("File exist: %s\n", filename);
        file_stats.file_duplicates_total++;
        return;
    }

    /*Opening file for writing in binary print_mode*/
    fh = fopen(filename,"wb");

    /* Nothing to write or nothing to write to */
    if ((buf == NULL) || (fh == NULL))
        return;

    /* Writing data to file */
    /* writing several times */
    do
    {
        size_t bytes_left = buf_len - bytes_written;

        bytes_written += fwrite(buf + bytes_written, 1, bytes_left, fh);

        err = ferror(fh);
        if (err && (err != EINTR) && (err != EAGAIN))
            break;

        max_retries--;

    } while ((max_retries > 0) && (bytes_written < buf_len));

    if (bytes_written < buf_len)
    {
        file_stats.file_storage_failures++;
        ErrorMessage("File server: disk writing error - %s!\n", strerror(err));
    }

    /*Closing File*/
    fclose(fh);
    file_stats.file_count++;
}

static void *FileSocketProcessThread(void *arg)
{
    ThreadElement *t = (ThreadElement *)arg;

    if (t == NULL)
    {
        ErrorMessage("File Socket: Invalid process thread parameter\n");
        return NULL;
    }
    if (t->socket_fd == -1)
    {
        ErrorMessage("File Socket: Invalid process thread socket\n");
        return NULL;
    }

    while (!stop_processing)
    {
        FileMessageHeader hdr;
        int rval;

        if ((rval = ReadHeader(t->socket_fd, &hdr)) == 0)
            break;
        else if (rval < 0)
        {
            ErrorMessage("Failed to read!\n");
            break;
        }

        if (hdr.version != FILE_HEADER_VERSION)
        {
            ErrorMessage("Bad message header version\n");
            continue;
        }

        if (hdr.length > FILE_CAPTURE_SIZE)
        {
            ErrorMessage("Bad message data\n");
            break;
        }

        if (hdr.length)
        {
            uint8_t *data;

            if ((data = malloc(hdr.length)) == NULL)
            {
                break;
            }

            LogMessage( "File Socket: Reading %u bytes\n",  hdr.length);

            if ((rval = ReadData(t->socket_fd, data, hdr.length)) == 0)
            {
                ErrorMessage("File Socket: Socket closed before data read\n");
                free(data);
                break;
            }
            else if (rval < 0)
            {
                ErrorMessage("File Socket: Failed to read %d\n", rval);
                free(data);
                continue;
            }

            WriteFile(data, hdr.length, hdr.filename);
            free(data);
        }

        CheckExit();
    }

    LogMessage("File Socket: Close a processing thread for %d\n", t->socket_fd);

    free(t);
    return NULL;
}

/* Add a signal handler
 * Return:
 *     0: error
 *     1: success
 */
int AddSignal(int sig, sighandler_t signal_handler, int check_needed)
{
    sighandler_t pre_handler;

#ifdef HAVE_SIGACTION
    struct sigaction action;
    struct sigaction old_action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = signal_handler;
    sigaction(sig, &action, &old_action);
    pre_handler = old_action.sa_handler;
#else
    pre_handler = signal(sig, signal_handler);
#endif
    if (SIG_ERR == pre_handler)
    {
        ErrorMessage("Could not add handler for signal %d \n", sig);
        return 0;
    }
    else if (check_needed && (SIG_IGN != pre_handler) && (SIG_DFL!= pre_handler))
    {
        ErrorMessage("WARNING: Handler is already installed for signal %d.\n", sig);
    }
    return 1;
}

/* Signal Handlers ************************************************************/
static void SigExitHandler(int signal)
{
    exit_signal = signal;
}

static void CheckExit()
{
    if ((SIGTERM == exit_signal) || (SIGINT == exit_signal))
    {
        stop_processing = 1;
    }
}

static void PrintFileStats(File_Storage_Stats *stats)
{
    LogMessage("Total files stored:        %d\n", stats->file_count);
    LogMessage("Total file storage errors: %d\n", stats->file_storage_failures);
    LogMessage("Total duplicated files:    %d\n", stats->file_duplicates_total);
}

static int ProcessClientRequest(int sockfd)
{
    struct timeval to;
    socklen_t clilen;
    fd_set rfds;
    struct sockaddr_in cli_addr;
    int rval;
    pthread_t tid;
    ThreadElement *t;
    int newsockfd;

    to.tv_sec = 2;
    to.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    //accept incoming connections
    clilen = sizeof(cli_addr);

    rval = select(sockfd + 1, &rfds, NULL, NULL, &to);

    if (rval > 0)
    {
        memset(&cli_addr, 0, sizeof(cli_addr));
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
        {
            if (errno != EINTR)
            {
                ErrorMessage("File Socket: Accept failed: %s\n", strerror(errno));
                return -1;
            }
        }
        else
        {

            LogMessage("File Socket: Creating a processing thread for %d\n", newsockfd);

            if ((t = calloc(1, sizeof(*t))) == NULL)
            {
                close(newsockfd);
                ErrorMessage("File Socket: Failed to allocate a thread struct");
                return -1;
            }

            t->socket_fd = newsockfd;
            if ((rval = pthread_create(&tid, NULL, &FileSocketProcessThread, (void *)t)) != 0)
            {
                close(newsockfd);
                ErrorMessage("File Socket: Unable to create a processing thread: %s", strerror(rval));
                return -1;
            }

            pthread_join(tid, NULL);
        }
    }
    else if (rval < 0)
    {
        if (errno != EINTR)
        {
            ErrorMessage("File Socket: Select failed: %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}

/*
 * Print a message to stderr or with logfacility.
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
void LogMessage(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    if (print_mode == PRINT_MODE_FAST)
        return;

    va_start(ap, format);

    vsnprintf(buf, STD_BUF, format, ap);
    buf[STD_BUF] = '\0';
    syslog(LOG_DAEMON | LOG_NOTICE, "%s", buf);
    printf("%s", buf);
    va_end(ap);
}

/*
 * Print a message to stderr or with logfacility.
 *
 * Arguments: format => the formatted error string to print out
 *            ... => format commands/fillers
 *
 * Returns: void function
 */
void ErrorMessage(const char *format,...)
{
    char buf[STD_BUF+1];
    va_list ap;

    va_start(ap, format);

    vsnprintf(buf, STD_BUF, format, ap);
    buf[STD_BUF] = '\0';
    syslog(LOG_CONS | LOG_DAEMON | LOG_ERR, "%s", buf);
    printf("%s", buf);
    va_end(ap);
}

/* Puts the program into daemon print_mode, nice and quiet like....*/
void GoDaemon(void)
{

    int exit_val = 0;
    pid_t cpid;
    int i;

    LogMessage("Initializing daemon mode\n");

    /* Don't daemonize if we've already daemonized */
    if(getppid() != 1)
    {
        /* now fork the child */
        printf("Spawning daemon child...\n");
        cpid = fork();

        if(cpid > 0)
        {
            /* Parent */
            printf("Daemon child %d lives...\n", cpid);

            printf("Daemon parent exiting (%d)\n", exit_val);

            exit(exit_val);                /* parent */
        }

        if(cpid < 0)
        {
            /* Daemonizing failed... */
            perror("fork");
            exit(1);
        }
    }
    /* Child */
    setsid();

    close(0);
    close(1);
    close(2);

    /* redirect stdin/stdout/stderr to /dev/null */
    i = open("/dev/null", O_RDWR);  /* stdin, fd 0 */
    dup(i);
    dup(i);

}

static void PrintHelp()
{
    printf("Usage: file_server  <portno>  <-dvh> -\n");
    printf("d: daemon mode -\n");
    printf("v: verbos mode -\n");
    printf("h: help -\n");
}

static void ParseArgs(char *arg)
{
    int len;
    int i;

    if (!arg)
        return;

    len = strlen(arg);

    if (len < 2)
    {
        printf("Option length two short!\n");
        return;
    }

    if (arg[0] != '-')
    {
        printf("Please provide option start with -\n");
    }

    for (i = 1; i < len; i++)
    {
        switch(arg[i])
        {
        case 'd':
            daemon_mode = 1;
            break;
        case 'v':
            print_mode = PRINT_MODE_DETAIL;
            LogMessage("Verbose print_mode specified!\n");
            break;
        case 'h':
            PrintHelp();
            break;
        default:
            printf("Please provide correct option!\n");
            PrintHelp();
            exit(1);

        }
    }
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;

    int one = 1;

    setlogmask (LOG_UPTO (LOG_NOTICE));
    openlog("file_server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    if (argc < 2)
    {
        fprintf(stderr,"please specify a port number\n");
        exit(1);
    }

    if(argc > 2)
    {
        int i;
        for (i = 2; i < argc; i++)
            ParseArgs(argv[i]);
    }

    if (daemon_mode)
    {
        GoDaemon();
    }

    AddSignal(SIGTERM, SigExitHandler, 1);
    AddSignal(SIGINT, SigExitHandler, 1);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        ErrorMessage("ERROR create socket.\n");
        exit(1);
    }

    //allow reuse of port
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);

    //bind to a local address
    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        ErrorMessage("ERROR on bind.\n");
        exit(1);
    }

    //listen marks the socket as passive socket listening to incoming connections,
    //it allows max 5 backlog connections: backlog connections are pending in queue
    //if pending connections are more than 5, later request may be ignored

    if (listen(sockfd,5))
    {
        ErrorMessage("ERROR on listen.\n");
        exit(1);
    }

    while (!stop_processing)
    {
        if (ProcessClientRequest(sockfd) < 0)
            break;

        CheckExit();
    }

    close(sockfd);

    LogMessage("----------Exiting.........!\n");

    PrintFileStats(&file_stats);

    closelog();

    return 0;
}
