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
#include <config.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_PTY_H
#include <pty.h>
#else
#include <util.h>
#endif
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sstp-private.h"


/*!
 * @brief The task structure
 */
struct sstp_task
{
    /*< The pid of the process */
    int pid;

    /*< The output file descriptor */
    int out;

    /*< The input file descriptor */
    int in;

    /*< The flags enabled */
    sstp_task_t type;

    /*< The ttydev device to use */
    char ttydev[SSTP_PATH_MAX];
};


static status_t sstp_setup_pty(sstp_task_st *task)
{
    int ret = -1;

    /* Open a pseudo-terminal */
    ret = openpty(&task->in, &task->out, task->ttydev, NULL, NULL);
    if (ret < 0)
    {
        return SSTP_FAIL;
    }

    return SSTP_OKAY;
}


static status_t sstp_setup_pipe(sstp_task_st *task)
{
    int pair[2];
    int ret = 0;
    status_t status = SSTP_FAIL;

    /* Create a pipe for input */
    ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, pair);
    if (ret < 0)
    {
        goto done;
    }

    /*
     * Save the socket pair here, but child will be using the out,
     * and the parent use the in for I/O.
     */
    task->in  = pair[0];
    task->out = pair[1];

    /* Success! */
    status = SSTP_OKAY;

done:
    
    return status;
}


status_t sstp_task_new(sstp_task_st **task, sstp_task_t type)
{
    /* Allocate task structure */
    *task = calloc(1, sizeof(sstp_task_st));
    if (*task == NULL)
    {
        return SSTP_FAIL;
    }

    /* Handle pipe / pty creation */
    switch (type)
    {
    case SSTP_TASK_USEPIPE:
        sstp_setup_pipe(*task);
        break;

    case SSTP_TASK_USEPTY:
        sstp_setup_pty(*task);
        break;

    default:
        break;
    }

    /* Save the flags */
    (*task)->type = type;

    /* Success */
    return SSTP_OKAY;
}


status_t sstp_task_start(sstp_task_st *task, const char *argv[])
{
    status_t status = SSTP_FAIL;
    int ret = -1;

    /* Fork the process */
    ret = fork();
    switch (ret)
    {
    case -1:
        goto done;

    case 0:
        
        /* In case the silent flag was set */
        if (task->type == SSTP_TASK_SILENT)
        {
            task->out = open("/dev/null", O_WRONLY); 
            dup2(task->out, STDOUT_FILENO);
            dup2(task->out, STDERR_FILENO);
        }

        /* Setup the standard I/O descriptors for child */
        if (SSTP_TASK_USEPIPE == task->type ||
            SSTP_TASK_USEPTY  == task->type)
        {
            dup2(task->out, STDOUT_FILENO); // (or tty-fd)
            dup2(task->out,  STDIN_FILENO); // (or tty-fd)
        }

        /* Dispose of any open descriptors */
        sstp_task_close(task);
        
        /* Execute the command given */
        execv(argv[0], (char**) &argv[1]);

        /* If we ever could reach here ... */
        exit(-1);

    default:
        
        /* Save a reference to the pid */
        task->pid = ret;

        /* Setup the I/O for parrent */
        if (SSTP_TASK_USEPIPE == task->type ||
            SSTP_TASK_USEPTY  == task->type)
        {
            close(task->out);       // (or tty-fd)
            task->out = task->in;   // (or pty-fd)
        }

        break;
    }

    /* Success */
    status = SSTP_OKAY;

done:
    
    return status;
}


int sstp_task_stop(sstp_task_st *task)
{
    return (kill(task->pid, SIGTERM) == -1)
        ? SSTP_FAIL
        : SSTP_OKAY;
}


int sstp_task_alive(sstp_task_st *task)
{
    int ret = kill(task->pid, 0);
    if (ret == -1 && errno == ESRCH)
    {   
        return 0;
    }

    return 1;
}


const char *sstp_task_ttydev(sstp_task_st* task)
{
    return (task->ttydev);
}


int sstp_task_stdout(sstp_task_st *task)
{
    return (task->out);
}


int sstp_task_stdin(sstp_task_st *task)
{
    return (task->in);
}


status_t sstp_task_wait(sstp_task_st *task, int *status, int flag)
{
    /* Collect the child if any */
    if (task->pid)
    {
        waitpid(task->pid, status, flag);
        task->pid = 0;
        return SSTP_OKAY;
    }

    return SSTP_FAIL;
}


void sstp_task_close(sstp_task_st *task)
{
    if (task->in)
    {
        close(task->in);
        task->in = 0;
    }

    if (task->out)
    {
        close(task->out);
        task->out = 0;
    }
}


void sstp_task_destroy(sstp_task_st *task)
{
    if (!task)
    {
        return;
    }

    /* Collect child if any */
    sstp_task_wait(task, NULL, WNOHANG);

    /* Close I/O descriptors */
    sstp_task_close(task);
    
    /* Free the memory */
    free(task);
}


#ifdef __SSTP_UNIT_TEST_TASK

#include <stdio.h>

#define TEST_STRING "Hello World"

int main(void)
{
    const char *args[10] = {};
    sstp_task_st *task;
    int i = 0;
    int ret = 0;
    char buf[12] = {};

    args[i++] = "/bin/echo";
    args[i++] = "-n";
    args[i++] = TEST_STRING;
    args[i++] = NULL;

    /* Create the task */
    ret = sstp_task_new(&task, SSTP_TASK_USEPTY);
    if (SSTP_OKAY != ret)
    {
        printf("Could not create task\n");
        return EXIT_FAILURE;
    }

    /* Start the task */
    ret = sstp_task_start(task, args);
    if (SSTP_OKAY != ret)
    {
        printf("Could not start the task\n");
        return EXIT_FAILURE;
    }

    /* Read the string */
    ret = read(sstp_task_stdout(task), buf, sizeof(buf)-1);
    if (ret != sizeof(TEST_STRING)-1)
    {
        printf("Could not read bytes from task %d\n", ret);
        return EXIT_FAILURE;
    }

    /* Make sure it's correct */
    if (strcmp(buf, TEST_STRING))
    {
        printf("The read data was not \"%s\" != \"%s\"\n", TEST_STRING, buf);
        return EXIT_FAILURE;
    }

    /* Wait for the task to terminate */
    ret = sstp_task_wait(task, NULL, 0);
    if (SSTP_OKAY != ret)
    {
        printf("Could not collect child\n");
        return EXIT_FAILURE;
    }

    printf("Successfully executed /bin/echo and validated the output\n");

    sstp_task_destroy(task);
    return EXIT_SUCCESS;
}

#endif /* #ifdef __SSTP_TASK_UNIT_TEST */
