/*!
 * @brief The header glue for logging messges in sstp-client.
 *
 * @file sstp-log.h
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

#ifndef __SSTP_LOG_H__
#define __SSTP_LOG_H__

/*! Log to syslog */ 
#define SSTP_OPT_SYSLOG         0x0001

/*! Log to standard out */
#define SSTP_OPT_STDERR         0x0002

/*! Log to standard out */
#define SSTP_OPT_STDOUT         0x0004

/*! Log to log file */
#define SSTP_OPT_LOGFILE        0x0008

/*! Log using file + line number */
#define SSTP_OPT_LINENO         0x0010


/*!
 * @brief the enumeration fo the different log-levels
 */
typedef enum
{
     SSTP_LOG_ERR   = 0,
     SSTP_LOG_WARN  = 1,
     SSTP_LOG_INFO  = 2,
     SSTP_LOG_DEBUG = 3,
     SSTP_LOG_TRACE = 4,
     SSTP_LOG_DUMP  = 5,

} sstp_level_t;


/*! Expand to appropriate function */
#define logmsg(level,fmt,args...)       \
    sstp_log_msg(level, __FILE__, __LINE__, fmt, ##args)


/*! Write a error log */
#define log_err(fmt, args...)           \
    if (SSTP_LOG_ERR <= sstp_log_level())   \
    {                                   \
        logmsg(SSTP_LOG_ERR, fmt, ##args);  \
    }


/*! Write a warning log */
#define log_warn(fmt, args...)          \
    if (SSTP_LOG_WARN <= sstp_log_level())  \
    {                                   \
        logmsg(SSTP_LOG_WARN, fmt, ##args); \
    }


/*! Write a info log */
#define log_info(fmt, args...)          \
    if (SSTP_LOG_INFO <= sstp_log_level())  \
    {                                   \
        logmsg(SSTP_LOG_INFO, fmt, ##args); \
    }


/*! Write a debug log */
#define log_debug(fmt, args...)         \
    if (SSTP_LOG_DEBUG <= sstp_log_level()) \
    {                                   \
        logmsg(SSTP_LOG_DEBUG, fmt, ##args);\
    }


/*! Write trace logs */
#define log_trace(fmt, args...)         \
    if (SSTP_LOG_TRACE <= sstp_log_level()) \
    {                                   \
        logmsg(SSTP_LOG_TRACE, fmt, ##args);\
    }

/*! Log all levels up to x */
#define sstp_log_upto(x) \
    ((1 << ((x) + 1)) - 1)

/*!
 * @brief Get the current log-level
 */
sstp_level_t sstp_log_level();


/*!
 * @brief Log a message
 */
void sstp_log_msg(int level, const char *file, int line,
        const char *fmt, ...);

/*!
 * @brief Print usage for logging options
 */
void sstp_log_usage(void);


/*!
 *  @brief Initialize the log module
 */
status_t sstp_log_init(const char *name, int mask, int opts);


/*!
 * @brief This initializes the log-library given the command line
 */
status_t sstp_log_init_argv(int *argc, char *argv[]);


/*!
 * @brief Cleanup any resources
 */
status_t sstp_log_fini(void);
    


#endif /* #ifndef __SSTP_LOG_H__ */
