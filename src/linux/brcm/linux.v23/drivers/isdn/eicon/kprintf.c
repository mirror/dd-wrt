/*
 * Source file for kernel interface to kernel log facility
 *
 * Copyright (C) Eicon Technology Corporation, 2000.
 *
 * Eicon File Revision :    1.3  
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 */

#include "eicon.h"
#include "sys.h"
#include <stdarg.h>

#include "divas.h"
#include "divalog.h"
#include "uxio.h"

/*
 * Implementation of printf and sprintf for kernel
 */

#define MAX_BUFF    (80)        /* limit size of temporary buffers */

#define WRITE_CHAR(BUFFER, SIZE, C) \
  if (--(SIZE) < 0) { (BUFFER)--; *(BUFFER) = '\0'; return; } *(BUFFER)++ = (C)


/*
 * convert a number to decimal ASCII
 */

static
void    do_decimal( char            *temp,
                    int             temp_len,
                    unsigned int    value,
                    char            *s)

{
    int     i;

    temp[0] = '\0';

    for (i = 1; i < temp_len; i++)
    {
        temp[i] = (char) ((value % 10) + (int) '0');
        value /= 10;
    }

    for (i = (temp_len - 1); temp[i] == '0'; i--)
    {
        ;
    }

    if (i == 0)
    {
        i++;
    }

    while (i >= 0)
    {
        *s++ = temp[i--];
    }

    return;
}

/*
 * convert a number to octal ASCII
 */

static
void    do_octal(   char            *temp,
                    unsigned int    value,
                    char            *s)

{
    int     i;

    temp[0] = '\0';

    for (i = 1; i <= 11; i++)
    {
        temp[i] = (char) ((value & 07) + (int) '0');
        value >>= 3;
    }
    temp[11] &= '3';

    for (i = 11; temp[i] == '0'; i--)
    {
        ;
    }

    if (i == 0)
    {
        i++;
    }

    while (i >= 0)
    {
        *s++ = temp[i--];
    }

    return;
}

/*
 * convert a number to hex ASCII
 */

static
void    do_hex( char            *temp,
                unsigned int    value,
                char            *s)

{
    int     i;
    static
    char    *dec_to_hex = "0123456789abcdef";

    temp[0] = '\0';

    for (i = 1; i <= 8; i++)
    {
        temp[i] = dec_to_hex[value & 0x0f];
        value >>= 4;
    }

    for (i = 8; temp[i] == '0'; i--)
    {
        ;
    }

    if (i == 0)
    {
        i++;
    }

    while (i >= 0)
    {
        *s++ = temp[i--];
    }

    return;
}

/*
 * convert a buffer to ASCII HEX
 */

static
void    do_buffer(  char    *buffer,
                    int     length,
                    char    *s)

{
    static
    char    hex_char [] = "0123456789abcdef";
    char    *b = buffer;
    int     hex_byte;
    int     nybble;

    length = (length >= ((MAX_BUFF / 3) + 1)) ? (MAX_BUFF / 3) : length;

    while (length)
    {
        hex_byte = (int) *b++;
        nybble = (hex_byte >> 4) & 0xf;
        *s++ = hex_char[nybble];
        nybble = hex_byte & 0xf;
        *s++ = hex_char[nybble];
        *s++ = ' ';
        length--;
    }
    *s = '\0';

    return;
}

/*
 * Body of sprintf function: behaves just like standard sprintf, except we
 * have an extra argument (buffer size) which we use to ensure we don't
 * overflow
 */

void    Divas_vsprintf(   char    *buffer,
                    int     size,
                    char    *fmt,
                    va_list argptr)

{
    char        c;          /* single character buffer */
    int         i;          /* handy scratch counter */
    int         f;          /* format character (after %) */
    char        *str;       /* pointer into string */
    char        temp[20];   /* temp buffer used in printing numbers */
    char        string[MAX_BUFF]; /* output from number conversion */
    int         length;     /* length of string "str" */
    char        fill;       /* fill character ' ' or '0' */
    boolean_t   leftjust;   /* TRUE if left justified, else right justified */
    int         fmax, fmin; /* field specifiers % MIN . MAX s */
    int         leading;    /* number of leading/trailing fill characters */
    char        sign;       /* set to '-' for negative decimals */
    int         number;     /* numeric argument */

    char        *buff_ptr;  /* pointer to user's buffer of hex data */
    int         buff_len;   /* length of hex data */

    /* make sure we have somthing to write into */

    if ((!buffer) || (size <= 0))
    {
        return;
    }

    while (TRUE)
    {
        /* echo characters until end or '%' encountered */

        while ((c = *fmt++) != '%')
        {
            if (!c)
            {
                *buffer = '\0';
                return;
            }
            WRITE_CHAR(buffer, size, c);
        }

        /* echo %% as % */

        if (*fmt == '%')
        {
            WRITE_CHAR(buffer, size, *fmt);
            continue;
        }

        /* %- turns on left-justify */

        if ((leftjust = (boolean_t) ((*fmt == '-') ? TRUE : FALSE)))
        {
            fmt++;
        }

        /* %0 turns on zero filling */

        if (*fmt == '0')
        {
            fill = '0';
        }
        else
        {
            fill = ' ';
        }

        /* minium field width specifier for %d, u, x, c, s */

        fmin = 0;

        if (*fmt == '*')
        {
            fmin = va_arg(argptr, int);
            fmt++;
        }
        else
        {
            while ('0' <= *fmt && *fmt <= '9')
            {
                fmin = (fmin * 10) + (*fmt++ - '0');
            }
        }

        /* maximum string width specifier for %s */

        fmax = 0;

        if (*fmt == '.')
        {
            if (*(++fmt) == '*')
            {
                fmax = va_arg(argptr, int);
                fmt++;
            }
            else
            {
                while ('0' <= *fmt && *fmt <= '9')
                {
                    fmax = (fmax * 10) + (*fmt++ - '0');
                }
            }
        }

        /* skip over 'l' option (ints are assumed same size as longs) */

        if (*fmt == 'l')
        {
            fmt++;
        }

        /* get the format chacater */

        if (!(f = *fmt++))
        {
            WRITE_CHAR(buffer, size, '%');
            *buffer = '\0';
            return;
        }

        sign = '\0';        /* sign == '-' for negative decimal */

        str = string;

        switch (f)
        {
        case 'c' :
            string[0] = (char) va_arg(argptr, int);
            string[1] = '\0';
            fmax = 0;
            fill = ' ';
            break;

        case 's' :
            str = va_arg(argptr, char *);
            fill = ' ';
            break;

        case 'D' :
        case 'd' :
            number = va_arg(argptr, int);
            if (number < 0)
            {
                sign = '-';
                number = -number;
            }
            do_decimal(temp, DIM(temp), (unsigned int) number, str);
            fmax = 0;
            break;

        case 'U' :
        case 'u' :
            number = va_arg(argptr, int);
            do_decimal(temp, DIM(temp), (unsigned int) number, str);
            fmax = 0;
            break;

        case 'O' :
        case 'o' :
            number = va_arg(argptr, int);
            do_octal(temp, (unsigned int) number, str);
            fmax = 0;
            break;

        case 'X' :
        case 'x' :
            number = va_arg(argptr, int);
            do_hex(temp, (unsigned int) number, str);
            fmax = 0;
            break;

        case 'H' :
        case 'h' :
            buff_ptr = va_arg(argptr, char *);
            buff_len = va_arg(argptr, int);
            do_buffer(buff_ptr, buff_len, str);
            fmax = 0;
            break;

        default :
            WRITE_CHAR(buffer, size, ((char) f));
            break;
        }

        /* get the length of the string */

        length = 0;
        while (str[length])
        {
            length++;
        }

        /* make sure we have fmax and fmin values that are O.K. */

        if (fmin > DIM(string) || fmin < 0)
        {
            fmin = 0;
        }

        if (fmax > DIM(string) || fmax < 0)
        {
            fmax = 0;
        }

        /* figure out how many leading characters thare are */

        leading = 0;

        if (fmax || fmin)
        {
            if (fmax)
            {
                if (length > fmax)
                {
                    length = fmax;
                }
            }

            if (fmin)
            {
                leading = fmin - length;
            }

            if (sign == '-')
            {
                leading--;
            }
        }

        /* output sign now, if fill is numeric */

        if (sign == '-' && fill == '0')
        {
            WRITE_CHAR(buffer, size, '-');
        }

        /* if right justified, output fill characters */

        if (!leftjust)
        {
            for (i = 0; i < leading; i++)
            {
                WRITE_CHAR(buffer, size, fill);
            }
        }

        /* output sign now, if fill is spaces */

        if (sign == '-' && fill == ' ')
        {
            WRITE_CHAR(buffer, size, '-');
        }

        /* now the actual value */

        for (i = 0; i < length; i++)
        {
            WRITE_CHAR(buffer, size, str[i]);
        }

        /* if left justified, fill out with the fill character */

        if (leftjust)
        {
            for (i = 0; i < leading; i++)
            {
                WRITE_CHAR(buffer, size, fill);
            }
        }
    }
}

/*
 * sprintf for kernel
 *
 * call our vsprintf assuming user has a big buffer....
 */

void    DivasSprintf(char *buffer, char *fmt, ...)

{
    va_list     argptr;         /* pointer to additional args */

    va_start(argptr, fmt);

    Divas_vsprintf(buffer, 1024, fmt, argptr);

    va_end(argptr);

    return;
}

void    DivasPrintf(char  *fmt, ...)

{
    klog_t      log;            /* log entry buffer */

    va_list     argptr;         /* pointer to additional args */

    va_start(argptr, fmt);

    /* clear log entry */

    memset((void *) &log, 0, sizeof(klog_t));

    log.card = -1;
    log.type = KLOG_TEXT_MSG;

    /* time stamp the entry */

    log.time_stamp = UxTimeGet();

    /* call vsprintf to format the user's information */

    Divas_vsprintf(log.buffer, DIM(log.buffer), fmt, argptr);

    va_end(argptr);

    /* send to the log streams driver and return */

    DivasLogAdd(&log, sizeof(klog_t));

    return;
}
