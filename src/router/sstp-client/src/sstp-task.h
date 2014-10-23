/*!
 * @brief API for handling sub-tasks
 *
 * @file sstp-task.c
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
#ifndef __SSTP_TASK_H__
#define __SSTP_TASK_H__


typedef enum
{
    /*! 
     * @brief Redirecting standard error and output to /dev/null.
     */
    SSTP_TASK_SILENT    = 0,

    /*!
     * @brief If this flag is set, the parent's will be able to 
     *   communicate with the child using the task's standard 
     *   input/ouput descriptors.
     */
    SSTP_TASK_USEPIPE   = 1,

    /*!
     * @brief If this flag is set, the parent's task->out is connected 
     *   to the pty, and the child's stdin is connected to the tty
     */
    SSTP_TASK_USEPTY    = 2,

} sstp_task_t;



/*!
 * @brief These are declared in sstp-task.c
 */
struct sstp_task;
typedef struct sstp_task sstp_task_st;


/*!
 * @brief Initialize a task structure
 */
status_t sstp_task_new(sstp_task_st **task, sstp_task_t type);


/*! 
 * @brief Starts a task given the command line
 */
status_t sstp_task_start(sstp_task_st *task, const char *argv[]);


/*!
 * @brief Get standard output
 */
int sstp_task_stdout(sstp_task_st *task);


/*!
 * @brief Get the standard input
 */
int sstp_task_stdin(sstp_task_st *task);


/*!
 * @brief Checks if a task is still running
 */
int sstp_task_alive(sstp_task_st *task);


/*! 
 * @brief Return a pinter to the pty dev
 */
const char *sstp_task_ttydev(sstp_task_st* task);


/*!
 * @brief Close all I/O descriptors
 */
void sstp_task_close(sstp_task_st *task);


/*! 
 * @brief Stops a task sending it a signal (expect SIGCHLD)
 */
status_t sstp_task_stop(sstp_task_st *task);


/*!
 * @brief Wait for the task to finish
 */
status_t sstp_task_wait(sstp_task_st *task, int *status, int flag);


/*!
 * @brief Destroys the task structure, nothing in the structure will
 *   be accessible.
 */
void sstp_task_destroy(sstp_task_st *task);


#endif
