// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * usbhid-dump - entry point *
 *
 * Copyright (C) 2010-2011 Nikolai Kondrashov <spbnick@gmail.com>
 */

#include "iface_list.h"
#include "misc.h"
#include <libusb.h>

#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>

/* Define LIBUSB_CALL for libusb <= 1.0.8 */
#ifndef LIBUSB_CALL
#define LIBUSB_CALL
#endif

#define GENERIC_ERROR(_fmt, _args...) \
    fprintf(stderr, _fmt "\n", ##_args)

#define IFACE_ERROR(_iface, _fmt, _args...) \
    GENERIC_ERROR("%s:" _fmt, _iface->addr_str, ##_args)

#define GENERIC_FAILURE(_fmt, _args...) \
    GENERIC_ERROR("Failed to " _fmt, ##_args)

#define IFACE_FAILURE(_iface, _fmt, _args...) \
    IFACE_ERROR(_iface, "Failed to " _fmt, ##_args)

#define LIBUSB_FAILURE(_fmt, _args...) \
    GENERIC_FAILURE(_fmt ": %s", ##_args, libusb_strerror(err))

#define LIBUSB_IFACE_FAILURE(_iface, _fmt, _args...) \
    IFACE_FAILURE(_iface, _fmt ": %s", ##_args, libusb_strerror(err))

#define ERROR_CLEANUP(_fmt, _args...) \
    do {                                \
        GENERIC_ERROR(_fmt, ##_args);   \
        goto cleanup;                   \
    } while (0)

#define FAILURE_CLEANUP(_fmt, _args...) \
    do {                                \
        GENERIC_FAILURE(_fmt, ##_args); \
        goto cleanup;                   \
    } while (0)

#define LIBUSB_FAILURE_CLEANUP(_fmt, _args...) \
    do {                                        \
        LIBUSB_FAILURE(_fmt, ##_args);          \
        goto cleanup;                           \
    } while (0)

#define LIBUSB_IFACE_FAILURE_CLEANUP(_iface, _fmt, _args...) \
    do {                                                        \
        LIBUSB_IFACE_FAILURE(_iface, _fmt, ##_args);            \
        goto cleanup;                                           \
    } while (0)

#define LIBUSB_GUARD(_expr, _fmt, _args...) \
    do {                                            \
        err = _expr;                                \
        if (err != LIBUSB_SUCCESS)                  \
            LIBUSB_FAILURE_CLEANUP(_fmt, ##_args);  \
    } while (0)

#define LIBUSB_IFACE_GUARD(_expr, _iface, _fmt, _args...) \
    do {                                                            \
        err = _expr;                                                \
        if (err != LIBUSB_SUCCESS)                                  \
            LIBUSB_IFACE_FAILURE_CLEANUP(_iface, _fmt, ##_args);    \
    } while (0)

/**< Number of the signal causing the exit */
static volatile sig_atomic_t exit_signum  = 0;

static void
exit_sighandler(int signum)
{
    if (exit_signum == 0)
        exit_signum = signum;
}

/**< "Stream paused" flag - non-zero if paused */
static volatile sig_atomic_t stream_paused = 0;

static void
stream_pause_sighandler(int signum)
{
    (void)signum;
    stream_paused = 1;
}

static void
stream_resume_sighandler(int signum)
{
    (void)signum;
    stream_paused = 0;
}

/**< "Stream feedback" flag - non-zero if feedback is enabled */
static volatile sig_atomic_t stream_feedback = 0;

static void
dump(const uhd_iface   *iface,
     const char        *entity,
     const uint8_t     *ptr,
     size_t             len)
{
    static const char   xd[]    = "0123456789ABCDEF";
    static char         buf[]   = " XX\n";
    size_t              pos;
    uint8_t             b;
    struct timeval      tv;

    gettimeofday(&tv, NULL);

    fprintf(stdout, "%s:%-16s %12llu.%.6u\n",
            iface->addr_str, entity,
            (unsigned long long int)tv.tv_sec,
            (unsigned int)tv.tv_usec);

    for (pos = 1; len > 0; len--, ptr++, pos++)
    {
        b = *ptr;
        buf[1] = xd[b >> 4];
        buf[2] = xd[b & 0xF];

        (void)fwrite(buf, ((pos % 16 == 0) ? 4 : 3), 1, stdout);
    }

    if (pos % 16 != 1)
        fputc('\n', stdout);
    fputc('\n', stdout);

    fflush(stdout);
}


static bool
dump_iface_list_descriptor(const uhd_iface *list)
{
    const uhd_iface    *iface;
    uint8_t             buf[UHD_MAX_DESCRIPTOR_SIZE];
    int                 rc;
    enum libusb_error   err;

    UHD_IFACE_LIST_FOR_EACH(iface, list)
    {
        if (iface->rd_len > sizeof(buf))
        {
            err = LIBUSB_ERROR_NO_MEM;
            LIBUSB_IFACE_FAILURE(iface, "report descriptor too long: %hu",
                                 iface->rd_len);
            return false;
        }

        rc = libusb_control_transfer(iface->dev->handle,
                                     /* See HID spec, 7.1.1 */
                                     0x81,
                                     LIBUSB_REQUEST_GET_DESCRIPTOR,
                                     (LIBUSB_DT_REPORT << 8), iface->number,
                                     buf, iface->rd_len, UHD_IO_TIMEOUT);
        if (rc < 0)
        {
            err = rc;
            LIBUSB_IFACE_FAILURE(iface, "retrieve report descriptor");
            return false;
        }
        dump(iface, "DESCRIPTOR", buf, rc);
    }

    return true;
}


static void LIBUSB_CALL
dump_iface_list_stream_cb(struct libusb_transfer *transfer)
{
    enum libusb_error   err;
    uhd_iface          *iface;

    assert(transfer != NULL);

    iface = (uhd_iface *)transfer->user_data;
    assert(uhd_iface_valid(iface));

    /* Clear interface "has transfer submitted" flag */
    iface->submitted = false;

    switch (transfer->status)
    {
        case LIBUSB_TRANSFER_COMPLETED:
            /* Dump the result */
            if (!stream_paused)
            {
                dump(iface, "STREAM",
                     transfer->buffer, transfer->actual_length);
                if (stream_feedback)
                    fputc('.', stderr);
            }
            /* Resubmit the transfer */
            err = libusb_submit_transfer(transfer);
            if (err != LIBUSB_SUCCESS)
                LIBUSB_IFACE_FAILURE(iface, "resubmit a transfer");
            else
            {
                /* Set interface "has transfer submitted" flag */
                iface->submitted = true;
            }
            break;

#define MAP(_name, _desc) \
    case LIBUSB_TRANSFER_##_name: \
        IFACE_ERROR(iface, _desc);  \
        break

        MAP(ERROR,      "Interrupt transfer failed");
        MAP(TIMED_OUT,  "Interrupt transfer timed out");
        MAP(STALL,      "Interrupt transfer halted (endpoint stalled)");
        MAP(NO_DEVICE,  "Device was disconnected");
        MAP(OVERFLOW,   "Interrupt transfer overflowed "
                        "(device sent more data than requested)");
#undef MAP

        case LIBUSB_TRANSFER_CANCELLED:
            break;
    }
}


static const char *
format_time_interval(unsigned int i)
{
    static char     buf[128];
    char           *p           = buf;
    unsigned int    h           = i / (60 * 60 * 1000);
    unsigned int    m           = (i % (60 * 60 * 1000)) / (60 * 1000);
    unsigned int    s           = (i % (60 * 1000)) / 1000;
    unsigned int    ms          = i % 1000;

#define FRACTION(_prev_sum, _name, _val) \
    do {                                                \
        if ((_val) > 0)                                 \
            p += snprintf(p, sizeof(buf) - (p - buf),   \
                          "%s%u " _name "%s",           \
                          ((_prev_sum) > 0 ? " " : ""), \
                          _val,                         \
                          (((_val) == 1) ? "" : "s"));  \
        if (p >= (buf + sizeof(buf)))                   \
            return buf;                                 \
    } while (0)

    FRACTION(0, "hour", h);
    FRACTION(h, "minute", m);
    FRACTION(h + m, "second", s);
    FRACTION(h + m + s, "millisecond", ms);

#undef FRACTION

    return buf;
}


static const char *
format_timeout(unsigned int i)
{
    return (i == 0) ? "infinite" : format_time_interval(i);
}


static bool
dump_iface_list_stream(libusb_context  *ctx,
                       uhd_iface       *list,
                       unsigned int     timeout)
{
    bool                        result              = false;
    enum libusb_error           err;
    size_t                      transfer_num        = 0;
    struct libusb_transfer    **transfer_list       = NULL;
    struct libusb_transfer    **ptransfer;
    uhd_iface                  *iface;
    bool                        submitted           = false;

    fprintf(stderr,
            "Starting dumping interrupt transfer stream\n"
            "with %s timeout.\n\n",
            format_timeout(timeout));

    UHD_IFACE_LIST_FOR_EACH(iface, list)
    {
        /* Set report protocol */
        LIBUSB_IFACE_GUARD(uhd_iface_set_protocol(iface, true,
                                                  UHD_IO_TIMEOUT),
                           iface, "set report protocol");
        /* Set infinite idle duration */
        LIBUSB_IFACE_GUARD(uhd_iface_set_idle(iface, 0, UHD_IO_TIMEOUT),
                           iface, "set infinite idle duration");
    }

    /* Calculate number of interfaces and thus transfers */
    transfer_num = uhd_iface_list_len(list);

    /* Allocate transfer list */
    transfer_list = malloc(sizeof(*transfer_list) * transfer_num);
    if (transfer_list == NULL)
        FAILURE_CLEANUP("allocate transfer list");

    /* Zero transfer list */
    for (ptransfer = transfer_list;
         (size_t)(ptransfer - transfer_list) < transfer_num;
         ptransfer++)
        *ptransfer = NULL;

    /* Allocate transfers */
    for (ptransfer = transfer_list;
         (size_t)(ptransfer - transfer_list) < transfer_num;
         ptransfer++)
    {
        *ptransfer = libusb_alloc_transfer(0);
        if (*ptransfer == NULL)
            FAILURE_CLEANUP("allocate a transfer");
        /*
         * Set user_data to NULL explicitly, since libusb_alloc_transfer
         * does memset to zero only and zero is not NULL, strictly speaking.
         */
        (*ptransfer)->user_data = NULL;
    }

    /* Initialize the transfers as interrupt transfers */
    for (ptransfer = transfer_list, iface = list;
         (size_t)(ptransfer - transfer_list) < transfer_num;
         ptransfer++, iface = iface->next)
    {
        void           *buf;
        const size_t    len = iface->int_in_ep_maxp;

        /* Allocate the transfer buffer */
        buf = malloc(len);
        if (len > 0 && buf == NULL)
            FAILURE_CLEANUP("allocate a transfer buffer");

        /* Initialize the transfer */
        libusb_fill_interrupt_transfer(*ptransfer,
                                       iface->dev->handle, iface->int_in_ep_addr,
                                       buf, len,
                                       dump_iface_list_stream_cb,
                                       (void *)iface,
                                       timeout);

        /* Ask to free the buffer when the transfer is freed */
        (*ptransfer)->flags |= LIBUSB_TRANSFER_FREE_BUFFER;
    }

    /* Submit first transfer requests */
    for (ptransfer = transfer_list;
         (size_t)(ptransfer - transfer_list) < transfer_num;
         ptransfer++)
    {
        LIBUSB_GUARD(libusb_submit_transfer(*ptransfer),
                     "submit a transfer");
        /* Set interface "has transfer submitted" flag */
        ((uhd_iface *)(*ptransfer)->user_data)->submitted = true;
        /* Set "have any submitted transfers" flag */
        submitted = true;
    }

    /* Run the event machine */
    while (submitted && exit_signum == 0)
    {
        /* Handle the transfer events */
        err = libusb_handle_events(ctx);
        if (err != LIBUSB_SUCCESS && err != LIBUSB_ERROR_INTERRUPTED)
            LIBUSB_FAILURE_CLEANUP("handle transfer events");

        /* Check if there are any submitted transfers left */
        submitted = false;
        for (ptransfer = transfer_list;
             (size_t)(ptransfer - transfer_list) < transfer_num;
             ptransfer++)
        {
            iface = (uhd_iface *)(*ptransfer)->user_data;

            if (iface != NULL && iface->submitted)
                submitted = true;
        }
    }

    /* If all the transfers were terminated unexpectedly */
    if (transfer_num > 0 && !submitted)
        ERROR_CLEANUP("No more interfaces to dump");

    result = true;

cleanup:

    /* Cancel the transfers */
    if (submitted)
    {
        submitted = false;
        for (ptransfer = transfer_list;
             (size_t)(ptransfer - transfer_list) < transfer_num;
             ptransfer++)
        {
            iface = (uhd_iface *)(*ptransfer)->user_data;

            if (iface != NULL && iface->submitted)
            {
                err = libusb_cancel_transfer(*ptransfer);
                if (err == LIBUSB_SUCCESS)
                    submitted = true;
                else
                {
                    LIBUSB_FAILURE("cancel a transfer, ignoring");
                    /*
                     * XXX are we really sure
                     * the transfer won't be finished?
                     */
                    iface->submitted = false;
                }
            }
        }
    }

    /* Wait for transfer cancellation */
    while (submitted)
    {
        /* Handle cancellation events */
        err = libusb_handle_events(ctx);
        if (err != LIBUSB_SUCCESS && err != LIBUSB_ERROR_INTERRUPTED)
        {
            LIBUSB_FAILURE("handle transfer cancellation events, "
                           "aborting transfer cancellation");
            break;
        }

        /* Check if there are any submitted transfers left */
        submitted = false;
        for (ptransfer = transfer_list;
             (size_t)(ptransfer - transfer_list) < transfer_num;
             ptransfer++)
        {
            iface = (uhd_iface *)(*ptransfer)->user_data;

            if (iface != NULL && iface->submitted)
                submitted = true;
        }
    }

    /*
     * Free transfer list along with non-submitted transfers and their
     * buffers.
     */
    if (transfer_list != NULL)
    {
        for (ptransfer = transfer_list;
             (size_t)(ptransfer - transfer_list) < transfer_num;
             ptransfer++)
        {
            iface = (uhd_iface *)(*ptransfer)->user_data;

            /*
             * Only free a transfer if it is not submitted. Better leak some
             * memory than have some important memory overwritten.
             */
            if (iface == NULL || !iface->submitted)
                libusb_free_transfer(*ptransfer);
        }

        free(transfer_list);
    }

    return result;
}


static int
run(bool            dump_descriptor,
    bool            dump_stream,
    unsigned int    stream_timeout,
    uint8_t         bus_num,
    uint8_t         dev_addr,
    uint16_t        vid,
    uint16_t        pid,
    int             iface_num)
{
    int                 result      = 1;
    enum libusb_error   err;
    libusb_context     *ctx         = NULL;
    uhd_dev            *dev_list    = NULL;
    uhd_iface          *iface_list  = NULL;
    uhd_iface          *iface;

    /* Create libusb context */
    LIBUSB_GUARD(libusb_init(&ctx), "create libusb context");

    /* Set libusb debug level to informational only */
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);

    /* Open device list */
    LIBUSB_GUARD(uhd_dev_list_open(ctx, bus_num, dev_addr,
                                   vid, pid, &dev_list),
                 "find and open the devices");

    /* Retrieve the list of HID interfaces from the device list */
    LIBUSB_GUARD(uhd_iface_list_new(dev_list, &iface_list),
                 "find HID interfaces");

    /* Filter the interface list by specified interface number */
    if (iface_num != UHD_IFACE_NUM_ANY)
        iface_list = uhd_iface_list_fltr_by_num(iface_list, iface_num);

    /* Check if there are any interfaces left */
    if (uhd_iface_list_empty(iface_list))
        ERROR_CLEANUP("No matching HID interfaces");

    /* Detach and claim the interfaces */
    UHD_IFACE_LIST_FOR_EACH(iface, iface_list)
    {
        LIBUSB_IFACE_GUARD(uhd_iface_detach(iface),
                           iface, "detach from the kernel driver");
        LIBUSB_IFACE_GUARD(uhd_iface_claim(iface),
                           iface, "claim");
    }

    /* Run with the prepared interface list */
    result = (!dump_descriptor || dump_iface_list_descriptor(iface_list)) &&
             (!dump_stream || dump_iface_list_stream(ctx, iface_list,
                                                     stream_timeout))
               ? 0
               : 1;

cleanup:

    /* Release and attach the interfaces back */
    UHD_IFACE_LIST_FOR_EACH(iface, iface_list)
    {
        err = uhd_iface_release(iface);
        if (err != LIBUSB_SUCCESS)
            LIBUSB_IFACE_FAILURE(iface, "release");

        err = uhd_iface_attach(iface);
        if (err != LIBUSB_SUCCESS)
            LIBUSB_IFACE_FAILURE(iface, "attach to the kernel driver");
    }

    /* Free the interface list */
    uhd_iface_list_free(iface_list);

    /* Close the device list */
    uhd_dev_list_close(dev_list);

    /* Destroy the libusb context */
    if (ctx != NULL)
        libusb_exit(ctx);

    return result;
}


static bool
parse_number_pair(const char   *str,
                  int           base,
                  long         *pn1,
                  long         *pn2)
{
    const char *p;
    char       *end;
    long        n1;
    long        n2;

    assert(str != NULL);

    p = str;

    /* Skip space (prevent strtol doing so) */
    while (isspace((int)*p))
        p++;

    /* Extract the first number */
    errno = 0;
    n1 = strtol(p, &end, base);
    if (errno != 0)
        return false;

    /* If nothing was read */
    if (end == p)
        return false;

    /* Move on */
    p = end;

    /* Skip space */
    while (isspace((int)*p))
        p++;

    /* If it is the end of string */
    if (*p == '\0')
        n2 = 0;
    else
    {
        /* If it is not the number separator */
        if (*p != ':')
            return false;

        /* Skip the number separator */
        p++;

        /* Skip space (prevent strtol doing so) */
        while (isspace((int)*p))
            p++;

        /* Extract the second number */
        errno = 0;
        n2 = strtol(p, &end, base);
        if (errno != 0)
            return false;
        /* If nothing was read */
        if (end == p)
            return false;

        /* Move on */
        p = end;

        /* Skip space */
        while (isspace((int)*p))
            p++;

        /* If it is not the end of string */
        if (*p != '\0')
            return false;
    }

    /* Output the numbers */
    if (pn1 != NULL)
        *pn1 = n1;
    if (pn2 != NULL)
        *pn2 = n2;

    return true;
}


static bool
parse_address(const char   *str,
              uint8_t      *pbus_num,
              uint8_t      *pdev_addr)
{
    long    bus_num;
    long    dev_addr;

    assert(str != NULL);

    if (!parse_number_pair(str, 10, &bus_num, &dev_addr))
        return false;

    if (bus_num < 0 || bus_num > UINT8_MAX ||
        dev_addr < 0 || dev_addr > UINT8_MAX)
        return false;

    if (pbus_num != NULL)
        *pbus_num = bus_num;
    if (pdev_addr != NULL)
        *pdev_addr = dev_addr;

    return true;
}


static bool
parse_model(const char   *str,
            uint16_t     *pvid,
            uint16_t     *ppid)
{
    long    vid;
    long    pid;

    assert(str != NULL);

    if (!parse_number_pair(str, 16, &vid, &pid))
        return false;

    if (vid < 0 || vid > UINT16_MAX ||
        pid < 0 || pid > UINT16_MAX)
        return false;

    if (pvid != NULL)
        *pvid = vid;
    if (ppid != NULL)
        *ppid = pid;

    return true;
}


static bool
parse_iface_num(const char *str,
                uint8_t    *piface_num)
{
    long        iface_num;
    const char *p;
    char       *end;

    assert(str != NULL);

    p = str;

    /* Skip space (prevent strtol doing so) */
    while (isspace((int)*p))
        p++;

    /* Extract interface number */
    errno = 0;
    iface_num = strtol(p, &end, 10);
    if (errno != 0 || end == p || iface_num < 0 || iface_num > UINT8_MAX)
        return false;

    /* Output interface number */
    if (piface_num != NULL)
        *piface_num = iface_num;

    return true;
}


static bool
parse_timeout(const char   *str,
              unsigned int *ptimeout)
{
    long long   timeout;
    const char *p;
    char       *end;

    assert(str != NULL);

    p = str;

    /* Skip space (prevent strtoll doing so) */
    while (isspace((int)*p))
        p++;

    /* Extract timeout */
    errno = 0;
    timeout = strtoll(p, &end, 10);
    if (errno != 0 || end == p || timeout < 0 || timeout > UINT_MAX)
        return false;

    /* Output timeout */
    if (ptimeout != NULL)
        *ptimeout = timeout;

    return true;
}


static bool
version(FILE *stream)
{
    return
        fprintf(
            stream,
"usbhid-dump (" PACKAGE_NAME ") " VERSION "\n"
"Copyright (C) 2010 Nikolai Kondrashov\n"
"License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.\n"
"\n"
"This is free software: you are free to change and redistribute it.\n"
"There is NO WARRANTY, to the extent permitted by law.\n") >= 0;
}


static bool
usage(FILE *stream, const char *name)
{
    return
        fprintf(
            stream,
"Usage: %s [OPTION]...\n"
"Dump USB device HID report descriptor(s) and/or stream(s).\n"
"\n"
"Options:\n"
"  -h, --help                       output this help message and exit\n"
"  -v, --version                    output version information and exit\n"
"\n"
"  -s, -a, --address=bus[:dev]      limit interfaces by bus number\n"
"                                   (1-255) and device address (1-255),\n"
"                                   decimal; zeroes match any\n"
"  -d, -m, --model=vid[:pid]        limit interfaces by vendor and\n"
"                                   product IDs (0001-ffff), hexadecimal;\n"
"                                   zeroes match any\n"
"  -i, --interface=NUMBER           limit interfaces by number (0-254),\n"
"                                   decimal; 255 matches any\n"
"\n"
"  -e, --entity=STRING              what to dump: either \"descriptor\",\n"
"                                   \"stream\" or \"all\"; value can be\n"
"                                   abbreviated\n"
"\n"
"  -t, --stream-timeout=NUMBER      stream interrupt transfer timeout, ms;\n"
"                                   zero means infinity\n"
"  -p, --stream-paused              start with the stream dump output\n"
"                                   paused\n"
"  -f, --stream-feedback            enable stream dumping feedback: for\n"
"                                   every transfer dumped a dot is\n"
"                                   printed to stderr\n"
"\n"
"Default options: --stream-timeout=60000 --entity=descriptor\n"
"\n"
"Signals:\n"
"  USR1/USR2                        pause/resume the stream dump output\n"
"\n",
            name) >= 0;
}


typedef enum opt_val {
    OPT_VAL_HELP            = 'h',
    OPT_VAL_VERSION         = 'v',
    OPT_VAL_ADDRESS         = 'a',
    OPT_VAL_ADDRESS_COMP    = 's',
    OPT_VAL_MODEL           = 'm',
    OPT_VAL_MODEL_COMP      = 'd',
    OPT_VAL_INTERFACE       = 'i',
    OPT_VAL_ENTITY          = 'e',
    OPT_VAL_STREAM_TIMEOUT  = 't',
    OPT_VAL_STREAM_PAUSED   = 'p',
    OPT_VAL_STREAM_FEEDBACK = 'f',
} opt_val;


static const struct option long_opt_list[] = {
    {.val       = OPT_VAL_HELP,
     .name      = "help",
     .has_arg   = no_argument,
     .flag      = NULL},
    {.val       = OPT_VAL_VERSION,
     .name      = "version",
     .has_arg   = no_argument,
     .flag      = NULL},
    {.val       = OPT_VAL_ADDRESS,
     .name      = "address",
     .has_arg   = required_argument,
     .flag      = NULL},
    {.val       = OPT_VAL_MODEL,
     .name      = "model",
     .has_arg   = required_argument,
     .flag      = NULL},
    {.val       = OPT_VAL_INTERFACE,
     .name      = "interface",
     .has_arg   = required_argument,
     .flag      = NULL},
    {.val       = OPT_VAL_ENTITY,
     .name      = "entity",
     .has_arg   = required_argument,
     .flag      = NULL},
    {.val       = OPT_VAL_STREAM_TIMEOUT,
     .name      = "stream-timeout",
     .has_arg   = required_argument,
     .flag      = NULL},
    {.val       = OPT_VAL_STREAM_PAUSED,
     .name      = "stream-paused",
     .has_arg   = no_argument,
     .flag      = NULL},
    {.val       = OPT_VAL_STREAM_FEEDBACK,
     .name      = "stream-feedback",
     .has_arg   = no_argument,
     .flag      = NULL},
    {.val       = 0,
     .name      = NULL,
     .has_arg   = 0,
     .flag      = NULL}
};


static const char  *short_opt_list = "hvs:a:d:m:i:e:t:pf";


int
main(int argc, char **argv)
{
    int                 result;

    const char         *name;

    int                 c;

    uint8_t             bus_num         = UHD_BUS_NUM_ANY;
    uint8_t             dev_addr        = UHD_DEV_ADDR_ANY;

    uint16_t            vid             = UHD_VID_ANY;
    uint16_t            pid             = UHD_PID_ANY;

    uint8_t             iface_num       = UHD_IFACE_NUM_ANY;

    bool                dump_descriptor = true;
    bool                dump_stream     = false;
    unsigned int        stream_timeout  = 60000;

    struct sigaction    sa;

    /*
     * Extract program invocation name
     */
    name = strrchr(argv[0], '/');
    if (name == NULL)
        name = argv[0];
    else
        name++;

#define USAGE_ERROR(_fmt, _args...) \
    do {                                        \
        fprintf(stderr, _fmt "\n", ##_args);    \
        usage(stderr, name);                    \
        return 1;                               \
    } while (0)

    /*
     * Parse command line arguments
     */
    while ((c = getopt_long(argc, argv,
                            short_opt_list, long_opt_list, NULL)) >= 0)
    {
        switch (c)
        {
            case OPT_VAL_HELP:
                usage(stdout, name);
                return 0;
                break;
            case OPT_VAL_VERSION:
                version(stdout);
                return 0;
                break;
            case OPT_VAL_ADDRESS:
            case OPT_VAL_ADDRESS_COMP:
                if (!parse_address(optarg, &bus_num, &dev_addr))
                    USAGE_ERROR("Invalid device address \"%s\"", optarg);
                break;
            case OPT_VAL_MODEL:
            case OPT_VAL_MODEL_COMP:
                if (!parse_model(optarg, &vid, &pid))
                    USAGE_ERROR("Invalid model \"%s\"", optarg);
                break;
            case OPT_VAL_INTERFACE:
                if (!parse_iface_num(optarg, &iface_num))
                    USAGE_ERROR("Invalid interface number \"%s\"", optarg);
                break;
            case OPT_VAL_ENTITY:
                if (strncmp(optarg, "descriptor", strlen(optarg)) == 0)
                {
                    dump_descriptor = true;
                    dump_stream = false;
                }
                else if (strncmp(optarg, "stream", strlen(optarg)) == 0)
                {
                    dump_descriptor = false;
                    dump_stream = true;
                }
                else if (strncmp(optarg, "all", strlen(optarg)) == 0)
                {
                    dump_descriptor = true;
                    dump_stream = true;
                }
                else
                    USAGE_ERROR("Unknown entity \"%s\"", optarg);

                break;
            case OPT_VAL_STREAM_TIMEOUT:
                if (!parse_timeout(optarg, &stream_timeout))
                    USAGE_ERROR("Invalid stream timeout \"%s\"", optarg);
                break;
            case OPT_VAL_STREAM_PAUSED:
                stream_paused = 1;
                break;
            case OPT_VAL_STREAM_FEEDBACK:
                stream_feedback = 1;
                break;
            case '?':
                usage(stderr, name);
                return 1;
                break;
        }
    }

    /*
     * Verify positional arguments
     */
    if (optind < argc)
        USAGE_ERROR("Positional arguments are not accepted");

    /*
     * Setup signal handlers
     */
    /* Setup SIGINT to terminate gracefully */
    sigaction(SIGINT, NULL, &sa);
    if (sa.sa_handler != SIG_IGN)
    {
        sa.sa_handler = exit_sighandler;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGTERM);
        sa.sa_flags = 0;    /* NOTE: no SA_RESTART on purpose */
        sigaction(SIGINT, &sa, NULL);
    }

    /* Setup SIGTERM to terminate gracefully */
    sigaction(SIGTERM, NULL, &sa);
    if (sa.sa_handler != SIG_IGN)
    {
        sa.sa_handler = exit_sighandler;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGINT);
        sa.sa_flags = 0;    /* NOTE: no SA_RESTART on purpose */
        sigaction(SIGTERM, &sa, NULL);
    }

    /* Setup SIGUSR1/SIGUSR2 to pause/resume the stream output */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = stream_pause_sighandler;
    sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = stream_resume_sighandler;
    sigaction(SIGUSR2, &sa, NULL);

    /* Make stdout buffered - we will flush it explicitly */
    setbuf(stdout, NULL);

    /* Run! */
    result = run(dump_descriptor, dump_stream, stream_timeout,
                 bus_num, dev_addr, vid, pid, iface_num);

    /*
     * Restore signal handlers
     */
    sigaction(SIGINT, NULL, &sa);
    if (sa.sa_handler != SIG_IGN)
        signal(SIGINT, SIG_DFL);

    sigaction(SIGTERM, NULL, &sa);
    if (sa.sa_handler != SIG_IGN)
        signal(SIGTERM, SIG_DFL);

    /*
     * Reproduce the signal used to stop the program to get proper exit
     * status.
     */
    if (exit_signum != 0)
        raise(exit_signum);

    return result;
}


