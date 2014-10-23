/*!
 * @brief Declarations for sstp-options.c
 *
 * @file sstp-options.h
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

#ifndef __SSTP_OPTION_H__
#define __SSTP_OPTION_H__


/*< Don't launch the pppd daemon */
#define SSTP_OPT_NOLAUNCH       0x0001
#define SSTP_OPT_NODAEMON       0x0002
#define SSTP_OPT_DEBUG          0x0004
#define SSTP_OPT_NOPLUGIN       0x0008
#define SSTP_OPT_CERTWARN       0x0010
#define SSTP_OPT_SAVEROUTE      0x0020


/*!
 * @brief Structure to keep all the options enabled
 */
typedef struct
{
    /*< The range of options enabled */
    int enable;

    /*! the CA certificate in PEM format */
    char *ca_cert;

    /*! The CA certificate path */
    char *ca_path;

    /*! The original server string */
    char *server;

    /*! Unique connection parameter */
    char *ipparam;

    /*! Password */
    char *password;

    /*! The proxy URL */
    char *proxy;

    /*! The privilege separation user */
    char *priv_user;
    
    /*! The privilege separation group */
    char *priv_group;

    /*! The privilege separation directory */
    char *priv_dir;

    /*! Username */
    char *user;

    /*! Use a persistent UUID */
    char *uuid;

    /*! The number of arguments to pppd */
    int pppdargc;

    /*! The arguments vector to pppd */
    char **pppdargv;

} sstp_option_st;


/*!
 * @brief Program ran into an initiation failure, will exit w/error code
 * @param prog      [IN] The application name
 * @param code      [IN] The exit/error code
 * @param message   [IN] The message containing formatting attributes
 * @param ...       [IN] The variable list of arguments
 * 
 * @par Note:
 *  Function never returns
 */
void sstp_usage_die(const char *prog, int code, const char *message, ...)
	__attribute__((noreturn));


/*!
 * @brief Terminate program as it ran into an irrecoverable error
 * @param message   [IN] The message containing formatting attributes
 * @param code      [IN] The error/exit code
 * @param ...       [IN] The variable list of arguments
 *
 * @par Note:
 *   Function never returns.
 */
void sstp_die(const char *message, int code, ...)
    __attribute__((noreturn));


/*!
 * @brief Parse the argument input vector and store options in @a opts
 * @param argc      [IN] The number of arguments
 * @param argv      [IN] The vector of arguments
 * 
 * @return 0 on success (always), or die...
 */
int sstp_parse_argv(sstp_option_st *ctx, int argc, char **argv);


/*!
 * @brief Cleanup the option structure
 * @param opts      [IN] The option structure
 * 
 * @par Note:
 *  Mostly to report false-positives with valgrind.
 *
 * @return (none)
 */
void sstp_option_free(sstp_option_st *opts);


#endif /* #ifndef __SSTP_OPTION_H__ */
