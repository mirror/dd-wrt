/*!
 * @brief Application's main entry point
 *
 * @file sstp-main.c
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
#ifndef __SSTP_LOG_PRIVATE_H__
#define __SSTP_LOG_PRIVATE_H__


/*! Keep attributes aligned to a 32-bit boundary */
#define LOG_ALIGN32(n)  \
        (((n) + 3) & ~3)

/*< The default application name */
#define SSTP_DFLT_APPNAME    "sstp-client"

/*< The max path size */
#define SSTP_PATH_MAX       255


/*!
 * @brief The type of log attribute 
 */
typedef enum log_type
{
    /*! The time string for when the message was logged */
    LOG_ATTR_TIME       = 1,
    
    /*! The file information for where the message was logged */
    LOG_ATTR_LINEINFO   = 2,
    
    /*! The hostname of the computer */
    LOG_ATTR_HOST       = 3,
    
    /*! The application name */
    LOG_ATTR_APPNAME    = 4,

    /*! The log message is to follow */
    LOG_ATTR_MESSAGE    = 5,
    
    /*
     * Add any additional attributes here
     */
    
    _LOG_ATTR_MAX
    
} log_type_t;

#define LOG_ATTR_MAX (_LOG_ATTR_MAX - 1)


/*! 
 * @brief The log message
 */
typedef struct
{
    /*! The level the message was logged with */
    uint8_t msg_level;
    
    /*! The length of the log-message */
    uint8_t msg_length;
    
    /*! The attribute count */
    uint8_t msg_acount;
    
    /*! The current time-stamp of the message */
    time_t msg_stamp;
    
    /*! The attribute section of this message */
    uint8_t msg_data[0];
    
} log_msg_st;


/*!
 * @brief A type length value structure to keep log-attributes
 */
typedef struct log_attr
{
    /*! The log attribute type */
    uint8_t attr_type;
    
    /*! The log attribute length */
    uint8_t attr_len;
    
    /*! The log attribute payload */
    uint8_t attr_data[0];
    
} log_attr_st;


/* Forward declare this structure */
struct log_ctx;


/*!
 * @brief Write the data to the output module
 * @param msg   The log message 
 * @param table The table holding the attributes of the message
 */
typedef void (*write_fn)(struct log_ctx *ctx, log_msg_st *msg, 
        log_attr_st *table[]);


/*!
 * @brief Close output module
 */
typedef void (*close_fn)(struct log_ctx *ctx);


/*! 
 * @brief A log context structure for setting output channels
 */
typedef struct log_ctx
{
    /*< The associated file descriptor */
    int sock;

    /*< The debug flag if enabled by configuration */
    int debug;
    
    /*< Any associated file */
    char file[SSTP_PATH_MAX];
    
    /* Write callback */
    write_fn write;
    
    /*< Close callback */
    close_fn close;
    
} log_ctx_st;


/*!
 * @brief Initialize the syslog output module
 */
status_t sstp_syslog_init(log_ctx_st *ctx);


/*!
 * @brief Initialize the stdout/err output module
 */
status_t sstp_logstd_init(log_ctx_st *ctx);


/*!
 * @brief Initialize the stdout/err output module
 */
status_t sstp_logfile_init(log_ctx_st *ctx);


/*!
 * @brief Converts a stream of bytes to a table of log-attributes
 */
status_t sstp_logattr_parse(uint8_t stream, log_attr_st *table[], int size);


#endif /* #ifndef __SSTP_LOG_PRIVATE_H__ */
