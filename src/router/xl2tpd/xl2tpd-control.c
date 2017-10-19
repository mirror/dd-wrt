/*
 * Layer Two Tunnelling Protocol Daemon Control Utility
 * Copyright (C) 2011 Alexander Dorokhov
 *
 * This software is distributed under the terms
 * of the GPL, which you should have received
 * along with this source.
 *
 * xl2tpd-control client main file
 *
 */
 
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "l2tp.h"

/* Paul: Alex: can we change this to use stdout, and let applications using
 * xl2tpd-control capture the output, instead of creating tmp files?
 */
/* result filename format including absolute path and formatting %i for pid */
#define RESULT_FILENAME_FORMAT "/var/run/xl2tpd/xl2tpd-control-%i.out"

#define ERROR_LEVEL 1
#define DEBUG_LEVEL 2

#define TUNNEL_REQUIRED 1
#define TUNNEL_NOT_REQUIRED 0
#define TIMEOUT 1000000  //timeout is 1s

char result_filename[128];
int result_fd = -1;

int log_level = ERROR_LEVEL;

void print_error (int level, const char *fmt, ...);

int read_result(int result_fd, char* buf, ssize_t size);

/* Definition of a command */
struct command_t
{
    char *name;
    int (*handler) (FILE*, char* tunnel, int optc, char *optv[]);
    int requires_tunnel;
    char *help;
};

int command_add_lac (FILE*, char* tunnel, int optc, char *optv[]);
int command_connect_lac (FILE*, char* tunnel, int optc, char *optv[]);
int command_disconnect_lac (FILE*, char* tunnel, int optc, char *optv[]);
int command_remove_lac (FILE*, char* tunnel, int optc, char *optv[]);
int command_add_lns (FILE*, char* tunnel, int optc, char *optv[]);
int command_status_lac (FILE*, char* tunnel, int optc, char *optv[]);
int command_status_lns (FILE*, char* tunnel, int optc, char *optv[]);
int command_remove_lns (FILE*, char* tunnel, int optc, char *optv[]);
int command_available (FILE*, char* tunnel, int optc, char *optv[]);

struct command_t commands[] = {
    /* Keep this command mapping for backwards compat */
    {"add", &command_add_lac, TUNNEL_REQUIRED,
        "\tadd\tadds new or modify existing lac configuration.\n"
        "\t\tConfiguration must be specified as command options in\n"
        "\t\t<key>=<value> pairs format.\n"
        "\t\tSee available options in xl2tpd.conf(5)\n"
    },
    {"connect", &command_connect_lac, TUNNEL_REQUIRED,
        "\tconnect\ttries to activate the tunnel.\n"
        "\t\tUsername and secret for the tunnel can be passed as\n"
        "\t\tcommand options.\n"
    },
    {"disconnect", &command_disconnect_lac, TUNNEL_REQUIRED,
        "\tdisconnect\tdisconnects the tunnel.\n"
    },
    {"remove", &command_remove_lac, TUNNEL_REQUIRED,
        "\tremove\tremoves lac configuration from xl2tpd.\n"
        "\t\txl2tpd disconnects the tunnel before removing.\n"
    },

    /* LAC commands */
    {"add-lac", &command_add_lac, TUNNEL_REQUIRED},
    {"connect-lac", &command_connect_lac, TUNNEL_REQUIRED},
    {"disconnect-lac", &command_disconnect_lac, TUNNEL_REQUIRED},
    {"remove-lac", &command_remove_lac, TUNNEL_REQUIRED},

    /* LNS commands */
    {"add-lns", &command_add_lns, TUNNEL_REQUIRED,
        "\tadd-lns\tadds new or modify existing lns configuration.\n"
    },
    {"remove-lns", &command_remove_lns, TUNNEL_REQUIRED},

    /* Generic commands */
    {"status", &command_status_lac, TUNNEL_REQUIRED},
    {"status-lns", &command_status_lns, TUNNEL_REQUIRED},
    {"available", &command_available, TUNNEL_NOT_REQUIRED},
    {NULL, NULL}
};

void usage()
{
    int i;

    printf ("\nxl2tpd server version %s\n", SERVER_VERSION);
    printf ("Usage: xl2tpd-control [-c <PATH>] <command> <tunnel name> [<COMMAND OPTIONS>]\n"
            "\n"
            "    -c\tspecifies xl2tpd control file\n"
            "    -d\tspecify xl2tpd-control to run in debug mode\n"
            "--help\tshows extended help\n"
    );

    printf ("Available commands: ");
    for (i = 0; commands[i].name; i++) {
        struct command_t *command = &commands[i];
        int last = command[1].name == NULL;

        printf ("%s%s", command->name, !last ? ", " : "\n");
    }
}

void help()
{
    int i;

    usage();
    printf (
        "\n"
        "Commands help:\n"
    );

    for (i = 0; commands[i].name; i++) {
        struct command_t *command = &commands[i];

        if (!command->help)
            continue;
        printf ("%s", command->help);
    }
    /*FIXME Ha! there is currently no manpage for xl2tpd-control */
    printf ("See xl2tpd-control man page for more help\n");
}

void cleanup(void)
{
    /* cleaning up */
    unlink (result_filename);
    if (result_fd >= 0)
        close (result_fd);
}

int main (int argc, char *argv[])
{
    char* control_filename = NULL;
    char* tunnel_name = NULL;
    struct command_t* command = NULL;    
    int i; /* argv iterator */

    if (argv[1] && !strncmp (argv[1], "--help", 6))
    {
        help();
        return 0;
    }
    /* parse global options */
    for (i = 1; i < argc; i++)
    {

        if (!strncmp (argv[i], "-c", 2))
        {
            control_filename = argv[++i];
        } else if (!strncmp (argv[i], "-d", 2)) {
            log_level = DEBUG_LEVEL;
        } else {
            break;
        }
    }
    if (i >= argc)
    {
        print_error (ERROR_LEVEL, "error: command not specified\n");
        usage();
        return -1;
    }
    if (!control_filename)
    {
        control_filename = strdup (CONTROL_PIPE);
    }

    /* parse command name */
    for (command = commands; command->name; command++)
    {
        if (!strcasecmp (argv[i], command->name))
        {
            i++;
            break;
        }
    }
    
    if (!command->name) {
        print_error (ERROR_LEVEL, "error: no such command %s\n", argv[i]);
        return -1;
    }
    
    /* get tunnel name */
    if(command->requires_tunnel){
        if (i >= argc)
        {
            print_error (ERROR_LEVEL, "error: tunnel name not specified\n");
            usage();
            return -1;
        }
        tunnel_name = argv[i++];    
        /* check tunnel name for whitespaces */
        if (strstr (tunnel_name, " "))
        {
            print_error (ERROR_LEVEL,
                "error: tunnel name shouldn't include spaces\n");
            usage();        
            return -1;
        }
    }
    
    char buf[CONTROL_PIPE_MESSAGE_SIZE] = "";
    FILE* mesf = fmemopen (buf, CONTROL_PIPE_MESSAGE_SIZE, "w");

    /* create result pipe for reading */
    snprintf (result_filename, 128, RESULT_FILENAME_FORMAT, getpid());
    unlink (result_filename);
    mkfifo (result_filename, 0600);
    atexit(cleanup);
    result_fd = open (result_filename, O_RDONLY | O_NONBLOCK, 0600);
    if (result_fd < 0)
    {
        print_error (ERROR_LEVEL,
            "error: unable to open %s for reading.\n", result_filename);
        return -2;
    }
   
    /* turn off O_NONBLOCK */
    if (fcntl (result_fd, F_SETFL, O_RDONLY) == -1) {
        print_error (ERROR_LEVEL,
            "Can not turn off nonblocking mode for result_fd: %s\n",
            strerror(errno));
        return -2;
    }
    
    /* pass result filename to command */
    fprintf (mesf, "@%s ", result_filename);
    if (ferror (mesf))
    {
        print_error (ERROR_LEVEL, "internal error: message buffer to short");
        return -2;
    }
    
    /* format command with remaining arguments */
    int command_res = command->handler (
        mesf, tunnel_name, argc - i, argv + i
    );
    if (command_res < 0)
    {
        print_error (ERROR_LEVEL, "error: command parse error\n");
        return -1;
    }
    
    fflush (mesf);
    
    if (ferror (mesf))
    {
        print_error (ERROR_LEVEL,
            "error: message too long (max = %i ch.)\n",
            CONTROL_PIPE_MESSAGE_SIZE - 1);
        return -1;
    }
    
    print_error (DEBUG_LEVEL, "command to be passed:\n%s\n", buf);

    /* try to open control file for writing */
    int control_fd = open (control_filename, O_WRONLY | O_NONBLOCK, 0600);
    if (control_fd < 0)
    {
        int errorno = errno;
        switch (errorno)
        {
        case EACCES:
            print_error (ERROR_LEVEL,
                "Unable to open %s for writing."
                " Is xl2tpd running and you have appropriate permissions?\n",
                control_filename);
            break;
        default:
            print_error (ERROR_LEVEL,
                "Unable to open %s for writing: %s\n",
                control_filename, strerror (errorno));
        }
        return -1;
    }

    /* turn off O_NONBLOCK */
    if (fcntl (control_fd, F_SETFL, O_WRONLY) == -1) {
        print_error (ERROR_LEVEL,
            "Can not turn off nonblocking mode for control_fd: %s\n",
            strerror(errno));
        return -2;
    }
    
    /* pass command to control pipe */
    if (write (control_fd, buf, ftell (mesf)) < 0)
    {
      int errorno = errno;
      print_error (ERROR_LEVEL,
                "Unable to write to %s: %s\n",
                control_filename, strerror (errorno));
      close (control_fd);
      return -1;
    }
    close (control_fd);
    
    /* read result from pipe */
    char rbuf[CONTROL_PIPE_MESSAGE_SIZE] = "";
    int command_result_code = read_result (
        result_fd, rbuf, CONTROL_PIPE_MESSAGE_SIZE
    );
    /* rbuf contains a newline, make it double to form a boundary. */
    print_error (DEBUG_LEVEL, "command response: \n%s\n", rbuf);
    
    return command_result_code;
}

void print_error (int level, const char *fmt, ...)
{
    if (level > log_level) return;
    va_list args;
    va_start (args, fmt);
    fprintf (stderr, "xl2tpd-control: ");
    vfprintf (stderr, fmt, args);
    va_end (args);
}


int read_result(int result_fd, char* buf, ssize_t size)
{
    /* read result from result_fd */
    /*FIXME: there is a chance to hang up reading.
             Should I create watching thread with timeout?
     */
    ssize_t readed = 0;
    ssize_t len;
    int write_pipe = 0;
    struct timeval tvs;
    struct timeval tve;
    unsigned long diff;
    gettimeofday(&tvs, NULL);

    do
    {
        len = read (result_fd, buf + readed, size - readed);
        if (len < 0)
        {
            if (errno == EINTR)
                continue;
            print_error (ERROR_LEVEL,
                "error: can't read command result: %s\n", strerror (errno));
            break;
        } else if (len == 0) {
            if(!write_pipe) {
                 gettimeofday(&tve, NULL);
                 diff = (tve.tv_sec - tvs.tv_sec) * 1000000 + (tve.tv_usec - tvs.tv_usec);
                 if (diff >= TIMEOUT) {
                     print_error (DEBUG_LEVEL, "error: read timout\n");
                     break;
                 } else {
                     usleep(10);
                     continue;
                 }
            }
            break;
        } else {
            write_pipe = 1;
            readed += len;
            if ((size - readed) <= 0)
                break;
       }
    } while (1);
    buf[readed] = '\0';

    /* scan result code */
    int command_result_code = -3;
    sscanf (buf, "%i", &command_result_code);

    return command_result_code;
}

int command_add
(FILE* mesf, char* tunnel, int optc, char *optv[], int reqopt)
{
    if (optc <= 0)
    {
        print_error (ERROR_LEVEL, "error: tunnel configuration expected\n");
        return -1;
    }
    fprintf (mesf, "%c %s ", reqopt, tunnel);
    int i;
    int wait_key = 1;
    for (i = 0; i < optc; i++)
    {
        fprintf (mesf, "%s", optv[i]);
        if (wait_key)
        {
            /* try to find '=' */
            char* eqv = strstr (optv[i], "=");
            if (eqv)
            {
                /* check is it not last symbol */
                if (eqv != (optv[i] + strlen(optv[i]) - 1))
                {
                    fprintf (mesf, ";"); /* end up option */
                } else {
                    wait_key = 0; /* now we waiting for value */
                }
            } else { /* two-word key */
                fprintf (mesf, " "); /* restore space */
            }
        } else {
            fprintf (mesf, ";"); /* end up option */        
            wait_key = 1; /* now we again waiting for key */
        }
    }
    return 0;
}

int command_add_lac
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    return command_add(mesf, tunnel, optc, optv, CONTROL_PIPE_REQ_LAC_ADD_MODIFY);
}

int command_add_lns
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    return command_add(mesf, tunnel, optc, optv, CONTROL_PIPE_REQ_LNS_ADD_MODIFY);
}


int command_connect_lac
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "%c %s", CONTROL_PIPE_REQ_LAC_CONNECT, tunnel);
    /* try to read authname and password from opts */
    if (optc > 0) {
        if (optc == 1)
            fprintf (mesf, " %s", optv[0]);
        else // optc >= 2
            fprintf (mesf, " %s %s", optv[0], optv[1]);
    }
    return 0;
}

int command_disconnect_lac
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "%c %s", CONTROL_PIPE_REQ_LAC_DISCONNECT, tunnel);
    return 0;
}

int command_remove_lac
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "%c %s", CONTROL_PIPE_REQ_LAC_REMOVE, tunnel);
    return 0;
}

int command_status_lns
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "%c %s", CONTROL_PIPE_REQ_LNS_STATUS, tunnel);
    return 0;
}

int command_status_lac
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "%c %s", CONTROL_PIPE_REQ_LAC_STATUS, tunnel);
    return 0;
}

int command_available
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "%c %s", CONTROL_PIPE_REQ_AVAILABLE, tunnel);
    return 0;
}

int command_remove_lns
(FILE* mesf, char* tunnel, int optc, char *optv[])
{
    fprintf (mesf, "%c %s", CONTROL_PIPE_REQ_LNS_REMOVE, tunnel);
    return 0;
}

