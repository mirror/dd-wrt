/*!
 * @brief Handle logging for sstp-client
 *
 * @file sstp-log.c
 *
 * @author Copyright (C) 2011 Eivind Naess, 
 *      All Rights Reserved
 *
 * @par License:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <config.h>
#include <fnmatch.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <getopt.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include <sstp-common.h>
#include <sstp-log.h>
#include "sstp-log-private.h"


/*< Type declare the option struct for shorthand */
typedef struct option option_st;


/*!
 * @brief A context structure for the log sub-system
 */
typedef struct
{
    /*! The current log options */
    int opt;
    
    /*! The current log level */
    int level;
    
    /*! Handle to the file output */
    log_ctx_st file;

    /*! Handle to the syslog output */
    log_ctx_st syslog;

    /*! Handle the stdout/err output */
    log_ctx_st out;
    
    /*! The number of tokens to filter on */
    char* token[10];
    
    /*! The host name of the computer */
    char hostname[128];
    
    /*! The application name passed in per command line */
    char appname[32];
    
    /*! The filter string, tokens separated by ',' */
    char filter[256];
    
    /*! The log-message */
    uint8_t buf[2048];

} sstp_log_st;


/*< The global log-context */
static sstp_log_st m_ctx = {};


/*!
 * @brief Parses the log message and returns a table of attribute pointers
 */
static status_t sstp_log_parse(log_msg_st *msg, log_attr_st *table[], int count)
{
    status_t retval = SSTP_FAIL;
    log_attr_st* attr = NULL;
    
    /* Reset the table of pointers */
    memset(table, 0, sizeof(log_attr_st*) * count);
    
    /* Get the first attribute */
    attr = (log_attr_st*) msg->msg_data;
    count = msg->msg_acount;
    
    /* Iterate through the attributes */
    while (count--)
    {
        /* Check the attribute type */
        if (attr->attr_type > LOG_ATTR_MAX)
        {
            goto done;
        }
        
        /* Keep a pointer to the attribute in the table */
        table[attr->attr_type] = attr;
        
        /* Increment to next attribute */
        attr = (log_attr_st*) (attr->attr_data + 
                LOG_ALIGN32(attr->attr_len));
    }
    
    /* Success */
    retval = SSTP_OKAY;
    
done:

    return retval;
}


/*!
 * @brief Log the message to the correct venue
 */
static status_t sstp_log_xmit(log_msg_st *msg)
{
    status_t retval = SSTP_FAIL;
    status_t status = SSTP_FAIL;
    log_attr_st *table[LOG_ATTR_MAX+1];
    int count = LOG_ATTR_MAX + 1;
 
    /* Parse the message into a table */
    status = sstp_log_parse(msg, table, count);
    if (SSTP_OKAY != status)
    {
        goto done;
    }

    /* Log message to stdout/stderr */
    if (SSTP_OPT_STDERR & m_ctx.opt ||
        SSTP_OPT_STDOUT & m_ctx.opt)
    {
        log_ctx_st *ctx = &m_ctx.out;
        ctx->write(ctx, msg, table);
    }

    /* Log message to syslog */
    if (SSTP_OPT_SYSLOG & m_ctx.opt)
    {
        log_ctx_st *ctx = &m_ctx.syslog;
        ctx->write(ctx, msg, table);
    }

    /* Log message to file */
    if (SSTP_OPT_LOGFILE & m_ctx.opt)
    {
        log_ctx_st *ctx = &m_ctx.file;
        ctx->write(ctx, msg, table);
    }

    /* Success! */
    retval = SSTP_OKAY;

done:

    return retval;
}


void sstp_log_msg(int level, const char *file, int line, const char *fmt, ...)
{
    log_msg_st *msg   = NULL;
    log_attr_st *attr = NULL;
    struct tm tm;
    int index = 0;
    va_list list;
    
    /* Check if we are filtering */
    if (SSTP_LOG_TRACE == level &&
        m_ctx.token[0])
    {
        const char *ptr;
        int index;
        
        ptr = strrchr(file, '/');
        ptr = (ptr != NULL)
            ? (ptr + 1)
            : file ;
     
        /* Lookup the appropriate token */
        for (index = 0; index < (sizeof(m_ctx.token)/sizeof(char*)); index++)
        {
            if (!m_ctx.token[index])
            {
                break;
            }
            
            if (0 == fnmatch(m_ctx.token[index], ptr, 0))
            {
                break;
            }
        }
        
        /* Don't log this message */
        if (!m_ctx.token[index])
        {
            return;
        }
    }
    
    /* Get the message structure and fill in the details */
    msg = (log_msg_st*) &m_ctx.buf[index];
    msg->msg_level = level;
    msg->msg_stamp = time(NULL);
    index = sizeof(*msg);
    
    /* Add the TIME STAMP string */
    attr = (log_attr_st*) &m_ctx.buf[index];
    attr->attr_type = LOG_ATTR_TIME;
    attr->attr_len = strftime((char*) attr->attr_data, 255, "%b %e %H:%M:%S",
        localtime_r(&msg->msg_stamp, &tm)) + 1;
    index += (sizeof(*attr) + LOG_ALIGN32(attr->attr_len));
    
    /* Add the LINEINFO attribute */
    attr = (log_attr_st*) &m_ctx.buf[index];
    attr->attr_type = LOG_ATTR_LINEINFO;
    attr->attr_len  = sprintf((char*)attr->attr_data, "(%s:%d)", file, line) + 1;
    index += (sizeof(*attr) + LOG_ALIGN32(attr->attr_len));    
    
    /* Add the HOST attribute */
    attr = (log_attr_st*) &m_ctx.buf[index];
    attr->attr_type = LOG_ATTR_HOST;
    attr->attr_len  = sprintf((char*)attr->attr_data, "%s", m_ctx.hostname) + 1;
    index += (sizeof(*attr) + LOG_ALIGN32(attr->attr_len));
    
    /* Add the APPNAME attribute */
    attr = (log_attr_st*) &m_ctx.buf[index];
    attr->attr_type = LOG_ATTR_APPNAME;
    attr->attr_len  = sprintf((char*)attr->attr_data, "%s", m_ctx.appname) + 1;
    index += (sizeof(*attr) + LOG_ALIGN32(attr->attr_len));
    
    /* Add the MESSAGE attribute */
    va_start(list, fmt);
    attr = (log_attr_st*) &m_ctx.buf[index];
    attr->attr_type = LOG_ATTR_MESSAGE;
    attr->attr_len  = vsprintf((char*)attr->attr_data, fmt, list) + 1;
    va_end(list);
    index += (sizeof(*attr) + LOG_ALIGN32(attr->attr_len));

    /* Update the length */
    msg->msg_length = LOG_ALIGN32(index);
    msg->msg_acount = 5;
    
    /* Transmit this log-message */
    sstp_log_xmit(msg);
}


sstp_level_t sstp_log_level(void)
{
    return m_ctx.level;
}


status_t sstp_init_log(const char *name, int opts, int level)
{
    /* Configure the structure */
    m_ctx.level = level;
    m_ctx.opt   = opts;
    
    /* Initialize syslog if enabled */
    if (SSTP_OPT_SYSLOG & opts)
    {
        log_ctx_st *ctx = &m_ctx.syslog;
        if (!ctx->file[0])
        {
            strncpy(ctx->file, _PATH_LOG, sizeof(ctx->file) - 1);
        }
        
        /* Set the line info flag */
        ctx->debug = !!(SSTP_OPT_LINENO & opts);

        /* Initialize the syslog context */
        sstp_syslog_init(ctx);
    }

    /* Initialize file if enabled */
    if (SSTP_OPT_LOGFILE & opts)
    {   
        /* Any output file */
        log_ctx_st *ctx = &m_ctx.file;
        if (!ctx->file[0])
        {
            snprintf(ctx->file, sizeof(ctx->file), "/tmp/%s.log", name);
        }
        
        /* Set the line info flag */
        ctx->debug = !!(SSTP_OPT_LINENO & opts);

        /* Initialize the file output */
        sstp_logfile_init(ctx);
    }

    /* Initialize stdout/err if enabled */
    if (SSTP_OPT_STDERR & opts ||
        SSTP_OPT_STDOUT & opts)
    {
        log_ctx_st *ctx = &m_ctx.out;
        ctx->sock = (SSTP_OPT_STDOUT & opts)
            ? STDOUT_FILENO
            : STDERR_FILENO;
        ctx->debug = !!(SSTP_OPT_LINENO & opts);

        sstp_logstd_init(ctx);
    }
    
    /* Get the hostname */
    gethostname(m_ctx.hostname, sizeof(m_ctx.hostname));
    
    /* Get the default application name */
    strncpy(m_ctx.appname, (name == NULL) 
            ? SSTP_DFLT_APPNAME 
            : name,
        sizeof(m_ctx.appname));

    return SSTP_OKAY;
}


void sstp_log_usage(void)
{
    printf("Available logging options:\n");
    printf("  --log-level  <level>     Specify the log-level per command line\n");
    printf("  --log-syslog <sock>      Output to syslog\n");
    printf("  --log-stderr             Output to stderr (negates --log-stdout)\n");
    printf("  --log-stdout             Output to stdout (negates --log-stderr)\n");
    printf("  --log-lineno             Include file/line information in messags\n");
    printf("  --log-filter <tok,tok>   Log messages matching a token\n\n");
}


status_t sstp_log_init_argv(int *argc, char *argv[])
{
    status_t retval = SSTP_FAIL;
    status_t status = SSTP_FAIL;
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    int level = SSTP_LOG_ERR;
    int opt   = SSTP_OPT_SYSLOG;
    int index = 0;
    int iter  = 0;
    char buff[64] = {};
    
    static option_st options [] = 
    {
        { "log-level",  required_argument, NULL, 100 },
        { "log-syslog", optional_argument, NULL, 101 },
        { "log-stderr", no_argument,       NULL, 102 },
        { "log-stdout", no_argument,       NULL, 103 },
        { "log-lineno", no_argument,       NULL, 104 },
        { "log-token",  required_argument, NULL, 105 },
        { "log-file",   optional_argument, NULL, 106 },
        { NULL,         no_argument,       NULL,   0 }
    };
    
    /* Get the application name per command line */
    ptr1 = strrchr(argv[0], '/');
    ptr1 = (ptr1 == NULL) 
        ? argv[0]
        : (ptr1+1);
    
    /* Iterate through the options */
    for (index = 1; index < *argc; index++)
    {
        /* Loop through the options we have, find match? */
        option_st *option = options;
        while (option->name != NULL)
        {
            if (!strcmp(option->name, argv[index]+2))
            {
                break;
            }
            
            option++;
        }
        
        /* Found no such option */
        if (option == NULL)
        {
            continue;
        }
        
        switch (option->val)
        {
        case 100:
            level = strtoul(argv[index+1], NULL, 10);
            break;
            
        case 101:
        {
            opt |= SSTP_OPT_SYSLOG;
            opt &= ~SSTP_OPT_STDOUT;
            opt &= ~SSTP_OPT_STDERR;
            
            /* Check for extra argument */
            if (*argv[index+1] == '-')
            {
                break;
            }
            
            log_ctx_st *ctx = &m_ctx.syslog;
            strncpy(ctx->file, argv[index+1], sizeof(ctx->file));
            break; 
        }
        case 102:
            opt |=  SSTP_OPT_STDERR;
            opt &= ~SSTP_OPT_STDOUT;
            opt &= ~SSTP_OPT_SYSLOG;
            break; 
            
        case 103:
            opt |= SSTP_OPT_STDOUT;
            opt &= ~SSTP_OPT_STDERR;
            opt &= ~SSTP_OPT_SYSLOG;
            break;
            
        case 104:
            opt |= SSTP_OPT_LINENO;
            break;
            
        case 105:
            
            if (*argv[index+1] == '-')
            {
                break;
            }
            
            strncpy(buff, argv[index+1], sizeof(buff));
            break;
            
        case 106:
        {
            opt |= SSTP_OPT_LOGFILE;
            if (*argv[index+1] == '-')
            {
                break;
            }
            log_ctx_st *ctx = &m_ctx.file;
            strncpy(ctx->file, argv[index+1], sizeof(ctx->file));
            break;
        }   
        default:
            continue;
        }
        
        /* Bump each argument one level down */
        for (iter = index; iter < *argc; iter++)
        {
            argv[iter] = argv[iter+1];
        }
        
        *argc = *argc - 1;
        
        switch (option->has_arg)
        {
        /* Skip the current argument */
        case required_argument:
            
            /* Error: Expected argument ... */
            if (*argv[index] == '-')
            {
                index = 0;
                continue;
            }
        
        /* Check if we need to skip argument */
        case optional_argument:
            
            if (*argv[index] != '-')
            {
                break;
            }
        
        /* No more work for this argv[index] element */
        case no_argument:
        default:
            index = 0;
            continue;
        }
        
        /* Skip the argument */
        for (iter = index; iter < *argc; iter++)
        {
            argv[iter] = argv[iter+1];
        }
        
        *argc = *argc - 1;
        index = 0;
    }

    /* Initialize the log-library as we normally would */
    status = sstp_init_log(ptr1, opt, level);
    if (SSTP_OKAY != status)
    {
        goto done;
    }
 
    /* Process the tokens */
    if (buff[0])
    {
        for (ptr1 = buff, index = 0; ptr1 != NULL; ptr1 = ptr2)
        {
            /* Seek to the separator */
            ptr2 = strchr(ptr1, ',');
            if (ptr2)
            {
                *ptr2++ = '\0';
            }
            
            /* Copy the filter token */
            m_ctx.token[index++] = strdup(ptr1);
        }
    }
    
    /* Success */
    retval = SSTP_OKAY;
    
done:
    
    return retval;
}


status_t sstp_log_fini(void)
{
    log_ctx_st *ctx = NULL;
    int index = 0;

    ctx = &m_ctx.file;
    if (ctx->close)
    {
        (ctx->close)(ctx);
    }

    ctx = &m_ctx.out;
    if (ctx->close)
    {
        (ctx->close)(ctx);
    }

    ctx = &m_ctx.syslog;
    if (ctx->close)
    {
        (ctx->close)(ctx);
    }
    
    /* Clean up the added memory if any */
    for (index = 0; index < (sizeof(m_ctx.token)/sizeof(char*)); index++)
    {
        if (m_ctx.token[index])
        {
            free(m_ctx.token[index]);
            m_ctx.token[index] = NULL;
        }
    }

    return SSTP_OKAY;
}


#ifdef __TEST_SSTP_LOG


#define TEST_SSTP_HOSTNAME  "sstp-test"
#define TEST_SSTP_MESSAGE   "This is a test message"
#define TEST_SSTP_RESULT    "Jan  1, 00:00:00 [sstp-client]: (sstp-log.c:487) This is a test message\n"

/*< Compare this message */
static char message[255];


/*!
 * @brief Overide the gethostname function
 */
int gethostname(char *host, size_t size)
{
    strncpy(host, TEST_SSTP_HOSTNAME, size);
}


/*!
 * @brief Override the time function
 */
time_t time(time_t *now)
{
    return 0;
}


/*!
 * @brief Force GMT
 */
struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    return gmtime_r(timep, result);
}


/*! 
 * @brief Dummy function
 */ 
status_t sstp_syslog_init(log_ctx_st *ctx)
{
    return SSTP_OKAY;
}


/*! 
 * @brief Dummy function
 */ 
status_t sstp_logstd_init(log_ctx_st *ctx)
{
    return SSTP_OKAY;
}


/*!
 * @brief Check the write callback
 */
void sstp_test_write(log_ctx_st *ctx, log_msg_st *msg, log_attr_st *table[])
{
    log_attr_st *attr;
    int len = 0;

    attr = table[LOG_ATTR_TIME];
    if (attr)
    {
        len += sprintf(message + len, "%s ", attr->attr_data);
    }

    attr = table[LOG_ATTR_APPNAME];
    if (attr)
    {
        len += sprintf(message + len, "[%s]: ", attr->attr_data);
    }

    attr = table[LOG_ATTR_LINEINFO];
    if (attr)
    {
        len += sprintf(message + len, "%s ", attr->attr_data);
    }

    attr = table[LOG_ATTR_MESSAGE];
    if (attr)
    {
        len += sprintf(message + len, "%s\n", attr->attr_data);
    }

    return;
}


/*!
 * @brief Dummy close function
 */
void sstp_test_close(log_ctx_st *ctx)
{
    /* Ignore this */
}


/*!
 * @brief Initialize the test
 */
status_t sstp_logfile_init(log_ctx_st *ctx)
{
    ctx->write = sstp_test_write;
    ctx->close = sstp_test_close;
    return SSTP_OKAY;
}


/*!
 * @brief Override the option in our unit-test
 */
const sstp_option_st *sstp_option_get(void)
{
    static sstp_option_st opts;
    opts.dlevel = 3;
    SSTP_FL_SET(&opts, LOGFILE);
    return &opts;
}


/*!
 * @brief Execute the unit-test
 */
int main(int argc, char *argv[])
{
    int retval = EXIT_FAILURE;
    int status = 0;

    /* Initialize the log-library */
    status = sstp_init_log();
    if (SSTP_OKAY != status)
    {
        printf("Failed to initialize sstp-log\n");
        goto done;
    }

    /* Send a message */
    log_err(TEST_SSTP_MESSAGE);

    /* Compare notes */
    if (strcmp(message, TEST_SSTP_RESULT))
    {
        printf("Message does not match:\n  %s  %s\n", 
                message, TEST_SSTP_RESULT);
        goto done;
    }

    /* Close the log-library */
    status = sstp_fini_log();
    if (SSTP_OKAY != status)
    {
        printf("Failed to de-initialize sstp-log\n");
        goto done;
    }

    printf("Success!\n");
    retval = EXIT_SUCCESS;

done:
    
    return retval;
}

#endif /* #ifdef __TEST_SSTP_LOG */

