/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * libqmi-glib -- GLib/GIO based library to control QMI devices
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2012 Aleksander Morgado <aleksander@lanedo.com>
 * Copyright (C) 2012 Dan Williams <dcbw@redhat.com>
 */

#include <config.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "qmi-utils.h"

/**
 * SECTION:qmi-utils
 * @title: Common utilities
 *
 * This section exposes a set of common utilities that may be used to work
 * with the QMI library.
 **/

/*****************************************************************************/

gchar *
__qmi_utils_str_hex (gconstpointer mem,
                     gsize size,
                     gchar delimiter)
{
    const guint8 *data = mem;
	gsize i;
	gsize j;
	gsize new_str_length;
	gchar *new_str;

	/* Get new string length. If input string has N bytes, we need:
	 * - 1 byte for last NUL char
	 * - 2N bytes for hexadecimal char representation of each byte...
	 * - N-1 bytes for the separator ':'
	 * So... a total of (1+2N+N-1) = 3N bytes are needed... */
	new_str_length =  3 * size;

	/* Allocate memory for new array and initialize contents to NUL */
	new_str = g_malloc0 (new_str_length);

	/* Print hexadecimal representation of each byte... */
	for (i = 0, j = 0; i < size; i++, j += 3) {
		/* Print character in output string... */
		snprintf (&new_str[j], 3, "%02X", data[i]);
		/* And if needed, add separator */
		if (i != (size - 1) )
			new_str[j + 2] = delimiter;
	}

	/* Set output string */
	return new_str;
}

/*****************************************************************************/

#if defined UTILS_ENABLE_TRACE
static void
print_read_bytes_trace (const gchar *type,
                        gconstpointer buffer,
                        gconstpointer out,
                        guint n_bytes)
{
    gchar *str1;
    gchar *str2;

    str1 = __qmi_utils_str_hex (buffer, n_bytes, ':');
    str2 = __qmi_utils_str_hex (out, n_bytes, ':');

    g_debug ("Read %s (%s) --> (%s)", type, str1, str2);
    g_warn_if_fail (g_str_equal (str1, str2));

    g_free (str1);
    g_free (str2);
}
#else
#define print_read_bytes_trace(...)
#endif

/**
 * qmi_utils_read_guint8_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @out: return location for the read variable.
 *
 * Reads an unsigned byte from the buffer.
 *
 * The user needs to make sure that at least 1 byte is available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the 1 byte
 * read.
 */
void
qmi_utils_read_guint8_from_buffer (const guint8 **buffer,
                                   guint16       *buffer_size,
                                   guint8        *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 1);

    *out = (*buffer)[0];

    print_read_bytes_trace ("guint8", &(*buffer)[0], out, 1);

    *buffer = &((*buffer)[1]);
    *buffer_size = (*buffer_size) - 1;
}

/**
 * qmi_utils_read_gint8_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @out: return location for the read variable.
 *
 * Reads a signed byte from the buffer.
 *
 * The user needs to make sure that at least 1 byte is available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the 1 byte
 * read.
 */
void
qmi_utils_read_gint8_from_buffer (const guint8 **buffer,
                                  guint16       *buffer_size,
                                  gint8         *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 1);

    *out = (gint8)(*buffer)[0];

    print_read_bytes_trace ("gint8", &(*buffer)[0], out, 1);

    *buffer = &((*buffer)[1]);
    *buffer_size = (*buffer_size) - 1;
}

/**
 * qmi_utils_read_guint16_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped to host byte order if necessary
 * @out: return location for the read variable.
 *
 * Reads an unsigned 16-bit integer from the buffer. The number in the buffer is
 * expected to be given in the byte order specificed by @endian, and this method
 * takes care of converting the read value to the proper host endianness.
 *
 * The user needs to make sure that at least 2 bytes are available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the 2 bytes
 * read.
 */
void
qmi_utils_read_guint16_from_buffer (const guint8 **buffer,
                                    guint16       *buffer_size,
                                    QmiEndian      endian,
                                    guint16       *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 2);

    memcpy (out, &((*buffer)[0]), 2);
    if (endian == QMI_ENDIAN_BIG)
        *out = GUINT16_FROM_BE (*out);
    else
        *out = GUINT16_FROM_LE (*out);

    print_read_bytes_trace ("guint16", &(*buffer)[0], out, 2);

    *buffer = &((*buffer)[2]);
    *buffer_size = (*buffer_size) - 2;
}

/**
 * qmi_utils_read_gint16_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped to host byte order if necessary
 * @out: return location for the read variable.
 *
 * Reads a signed 16-bit integer from the buffer. The number in the buffer is
 * expected to be given in the byte order specified by @endian, and this method
 * takes care of converting the read value to the proper host endianness.
 *
 * The user needs to make sure that at least 2 bytes are available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the 2 bytes
 * read.
 */
void
qmi_utils_read_gint16_from_buffer (const guint8 **buffer,
                                   guint16       *buffer_size,
                                   QmiEndian      endian,
                                   gint16        *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 2);

    memcpy (out, &((*buffer)[0]), 2);
    if (endian == QMI_ENDIAN_BIG)
        *out = GINT16_FROM_BE (*out);
    else
        *out = GINT16_FROM_LE (*out);

    print_read_bytes_trace ("gint16", &(*buffer)[0], out, 2);

    *buffer = &((*buffer)[2]);
    *buffer_size = (*buffer_size) - 2;
}

/**
 * qmi_utils_read_guint32_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped to host byte order if necessary
 * @out: return location for the read variable.
 *
 * Reads an unsigned 32-bit integer from the buffer. The number in the buffer is
 * expected to be given in the byte order specified by @endian, and this method
 * takes care of converting the read value to the proper host endianness.
 *
 * The user needs to make sure that at least 4 bytes are available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the 4 bytes
 * read.
 */
void
qmi_utils_read_guint32_from_buffer (const guint8 **buffer,
                                    guint16       *buffer_size,
                                    QmiEndian      endian,
                                    guint32       *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 4);

    memcpy (out, &((*buffer)[0]), 4);
    if (endian == QMI_ENDIAN_BIG)
        *out = GUINT32_FROM_BE (*out);
    else
        *out = GUINT32_FROM_LE (*out);

    print_read_bytes_trace ("guint32", &(*buffer)[0], out, 4);

    *buffer = &((*buffer)[4]);
    *buffer_size = (*buffer_size) - 4;
}

/**
 * qmi_utils_read_gint32_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped to host byte order if necessary
 * @out: return location for the read variable.
 *
 * Reads a signed 32-bit integer from the buffer. The number in the buffer is
 * expected to be given in the byte order specified by @endian, and this method
 * takes care of converting the read value to the proper host endianness.
 *
 * The user needs to make sure that at least 4 bytes are available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the 4 bytes
 * read.
 */
void
qmi_utils_read_gint32_from_buffer (const guint8 **buffer,
                                   guint16       *buffer_size,
                                   QmiEndian      endian,
                                   gint32        *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 4);

    memcpy (out, &((*buffer)[0]), 4);
    if (endian == QMI_ENDIAN_BIG)
        *out = GINT32_FROM_BE (*out);
    else
        *out = GINT32_FROM_LE (*out);

    print_read_bytes_trace ("gint32", &(*buffer)[0], out, 4);

    *buffer = &((*buffer)[4]);
    *buffer_size = (*buffer_size) - 4;
}

/**
 * qmi_utils_read_guint64_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped to host byte order if necessary
 * @out: return location for the read variable.
 *
 * Reads an unsigned 64-bit integer from the buffer. The number in the buffer is
 * expected to be given in the byte order specified by @endian, and this method
 * takes care of converting the read value to the proper host endianness.
 *
 * The user needs to make sure that at least 8 bytes are available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the 8 bytes
 * read.
 */
void
qmi_utils_read_guint64_from_buffer (const guint8 **buffer,
                                    guint16       *buffer_size,
                                    QmiEndian      endian,
                                    guint64       *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 8);

    memcpy (out, &((*buffer)[0]), 8);
    if (endian == QMI_ENDIAN_BIG)
        *out = GUINT64_FROM_BE (*out);
    else
        *out = GUINT64_FROM_LE (*out);

    print_read_bytes_trace ("guint64", &(*buffer)[0], out, 8);

    *buffer = &((*buffer)[8]);
    *buffer_size = (*buffer_size) - 8;
}

/**
 * qmi_utils_read_gint64_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped to host byte order if necessary
 * @out: return location for the read variable.
 *
 * Reads a signed 64-bit integer from the buffer. The number in the buffer is
 * expected to be given in the byte order specified by @endian, and this method
 * takes care of converting the read value to the proper host endianness.
 *
 * The user needs to make sure that at least 8 bytes are available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the 8 bytes
 * read.
 */
void
qmi_utils_read_gint64_from_buffer (const guint8 **buffer,
                                   guint16       *buffer_size,
                                   QmiEndian      endian,
                                   gint64        *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 8);

    memcpy (out, &((*buffer)[0]), 8);
    if (endian == QMI_ENDIAN_BIG)
        *out = GINT64_FROM_BE (*out);
    else
        *out = GINT64_FROM_LE (*out);

    print_read_bytes_trace ("gint64", &(*buffer)[0], out, 8);

    *buffer = &((*buffer)[8]);
    *buffer_size = (*buffer_size) - 8;
}

/**
 * qmi_utils_read_sized_guint_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @n_bytes: number of bytes to read.
 * @endian: endianness of firmware value; swapped to host byte order if necessary
 * @out: return location for the read variable.
 *
 * Reads a @n_bytes-sized unsigned integer from the buffer. The number in the
 * buffer is expected to be given in the byte order specified by @endian, and
 * this method takes care of converting the read value to the proper host
 * endianness.
 *
 * The user needs to make sure that at least @n_bytes bytes are available
 * in the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the @n_bytes
 * bytes read.
 */
void
qmi_utils_read_sized_guint_from_buffer (const guint8 **buffer,
                                        guint16       *buffer_size,
                                        guint          n_bytes,
                                        QmiEndian      endian,
                                        guint64       *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= n_bytes);

    *out = 0;
    memcpy (out, *buffer, n_bytes);
    if (endian == QMI_ENDIAN_BIG)
        *out = GUINT64_FROM_BE (*out);
    else
        *out = GUINT64_FROM_LE (*out);

    *buffer = &((*buffer)[n_bytes]);
    *buffer_size = (*buffer_size) - n_bytes;
}

/**
 * qmi_utils_write_guint8_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @in: location of the variable to be written.
 *
 * Writes an unsigned byte into the buffer.
 *
 * The user needs to make sure that the buffer is at least 1 byte long.
 *
 * Also note that both @buffer and @buffer_size get updated after the 1 byte
 * write.
 */
void
qmi_utils_write_guint8_to_buffer (guint8  **buffer,
                                  guint16  *buffer_size,
                                  guint8   *in)
{
    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 1);

    memcpy (&(*buffer)[0], in, sizeof (*in));

    *buffer = &((*buffer)[1]);
    *buffer_size = (*buffer_size) - 1;
}

/**
 * qmi_utils_write_gint8_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @in: location of the variable to be written.
 *
 * Writes a signed byte into the buffer.
 *
 * The user needs to make sure that the buffer is at least 1 byte long.
 *
 * Also note that both @buffer and @buffer_size get updated after the 1 byte
 * write.
 */
void
qmi_utils_write_gint8_to_buffer (guint8  **buffer,
                                 guint16  *buffer_size,
                                 gint8    *in)
{
    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 1);

    memcpy (&(*buffer)[0], in, sizeof (*in));

    *buffer = &((*buffer)[1]);
    *buffer_size = (*buffer_size) - 1;
}

/**
 * qmi_utils_write_guint16_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped from host byte order if necessary
 * @in: location of the variable to be written.
 *
 * Writes an unsigned 16-bit integer into the buffer. The number to be written
 * is expected to be given in host endianness, and this method takes care of
 * converting the value written to the byte order specified by @endian.
 *
 * The user needs to make sure that the buffer is at least 2 bytes long.
 *
 * Also note that both @buffer and @buffer_size get updated after the 2 bytes
 * write.
 */
void
qmi_utils_write_guint16_to_buffer (guint8  **buffer,
                                   guint16  *buffer_size,
                                   QmiEndian endian,
                                   guint16  *in)
{
    guint16 tmp;

    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 2);

    if (endian == QMI_ENDIAN_BIG)
        tmp = GUINT16_TO_BE (*in);
    else
        tmp = GUINT16_TO_LE (*in);
    memcpy (&(*buffer)[0], &tmp, sizeof (tmp));

    *buffer = &((*buffer)[2]);
    *buffer_size = (*buffer_size) - 2;
}

/**
 * qmi_utils_write_gint16_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped from host byte order if necessary
 * @in: location of the variable to be written.
 *
 * Writes a signed 16-bit integer into the buffer. The number to be written
 * is expected to be given in host endianness, and this method takes care of
 * converting the value written to the byte order specified by @endian.
 *
 * The user needs to make sure that the buffer is at least 2 bytes long.
 *
 * Also note that both @buffer and @buffer_size get updated after the 2 bytes
 * write.
 */
void
qmi_utils_write_gint16_to_buffer (guint8  **buffer,
                                  guint16  *buffer_size,
                                  QmiEndian endian,
                                  gint16   *in)
{
    gint16 tmp;

    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 2);

    if (endian == QMI_ENDIAN_BIG)
        tmp = GINT16_TO_BE (*in);
    else
        tmp = GINT16_TO_LE (*in);
    memcpy (&(*buffer)[0], &tmp, sizeof (tmp));

    *buffer = &((*buffer)[2]);
    *buffer_size = (*buffer_size) - 2;
}

/**
 * qmi_utils_write_guint32_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped from host byte order if necessary
 * @in: location of the variable to be written.
 *
 * Writes an unsigned 32-bit integer into the buffer. The number to be written
 * is expected to be given in host endianness, and this method takes care of
 * converting the value written to the byte order specified by @endian.
 *
 * The user needs to make sure that the buffer is at least 4 bytes long.
 *
 * Also note that both @buffer and @buffer_size get updated after the 4 bytes
 * write.
 */
void
qmi_utils_write_guint32_to_buffer (guint8  **buffer,
                                   guint16  *buffer_size,
                                   QmiEndian endian,
                                   guint32  *in)
{
    guint32 tmp;

    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 4);

    if (endian == QMI_ENDIAN_BIG)
        tmp = GUINT32_TO_BE (*in);
    else
        tmp = GUINT32_TO_LE (*in);
    memcpy (&(*buffer)[0], &tmp, sizeof (tmp));

    *buffer = &((*buffer)[4]);
    *buffer_size = (*buffer_size) - 4;
}

/**
 * qmi_utils_write_gint32_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped from host byte order if necessary
 * @in: location of the variable to be written.
 *
 * Writes a signed 32-bit integer into the buffer. The number to be written
 * is expected to be given in host endianness, and this method takes care of
 * converting the value written to the byte order specified by @endian.
 *
 * The user needs to make sure that the buffer is at least 4 bytes long.
 *
 * Also note that both @buffer and @buffer_size get updated after the 4 bytes
 * write.
 */
void
qmi_utils_write_gint32_to_buffer (guint8  **buffer,
                                  guint16  *buffer_size,
                                  QmiEndian endian,
                                  gint32   *in)
{
    gint32 tmp;

    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 4);

    if (endian == QMI_ENDIAN_BIG)
        tmp = GINT32_TO_BE (*in);
    else
        tmp = GINT32_TO_LE (*in);
    memcpy (&(*buffer)[0], &tmp, sizeof (tmp));

    *buffer = &((*buffer)[4]);
    *buffer_size = (*buffer_size) - 4;
}

/**
 * qmi_utils_write_guint64_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped from host byte order if necessary
 * @in: location of the variable to be written.
 *
 * Writes an unsigned 64-bit integer into the buffer. The number to be written
 * is expected to be given in host endianness, and this method takes care of
 * converting the value written to the byte order specified by @endian.
 *
 * The user needs to make sure that the buffer is at least 8 bytes long.
 *
 * Also note that both @buffer and @buffer_size get updated after the 8 bytes
 * write.
 */
void
qmi_utils_write_guint64_to_buffer (guint8  **buffer,
                                   guint16  *buffer_size,
                                   QmiEndian endian,
                                   guint64  *in)
{
    guint64 tmp;

    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 8);

    if (endian == QMI_ENDIAN_BIG)
        tmp = GUINT64_TO_BE (*in);
    else
        tmp = GUINT64_TO_LE (*in);
    memcpy (&(*buffer)[0], &tmp, sizeof (tmp));

    *buffer = &((*buffer)[8]);
    *buffer_size = (*buffer_size) - 8;
}

/**
 * qmi_utils_write_gint64_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @endian: endianness of firmware value; swapped from host byte order if necessary
 * @in: location of the variable to be written.
 *
 * Writes a signed 64-bit integer into the buffer. The number to be written
 * is expected to be given in host endianness, and this method takes care of
 * converting the value written to the byte order specified by @endian.
 *
 * The user needs to make sure that the buffer is at least 8 bytes long.
 *
 * Also note that both @buffer and @buffer_size get updated after the 8 bytes
 * write.
 */
void
qmi_utils_write_gint64_to_buffer (guint8  **buffer,
                                  guint16  *buffer_size,
                                  QmiEndian endian,
                                  gint64   *in)
{
    gint64 tmp;

    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= 8);

    if (endian == QMI_ENDIAN_BIG)
        tmp = GINT64_TO_BE (*in);
    else
        tmp = GINT64_TO_LE (*in);
    memcpy (&(*buffer)[0], &tmp, sizeof (tmp));

    *buffer = &((*buffer)[8]);
    *buffer_size = (*buffer_size) - 8;
}

/**
 * qmi_utils_write_sized_guint_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @n_bytes: number of bytes to write.
 * @endian: endianness of firmware value; swapped from host byte order if necessary
 * @in: location of the variable to be written.
 *
 * Writes a @n_bytes-sized unsigned integer into the buffer. The number to be
 * written is expected to be given in host endianness, and this method takes
 * care of converting the value written to the byte order specified by @endian.
 *
 * The user needs to make sure that the buffer is at least @n_bytes bytes long.
 *
 * Also note that both @buffer and @buffer_size get updated after the @n_bytes
 * bytes write.
 */
void
qmi_utils_write_sized_guint_to_buffer (guint8  **buffer,
                                       guint16  *buffer_size,
                                       guint     n_bytes,
                                       QmiEndian endian,
                                       guint64  *in)
{
    guint64 tmp;

    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (*buffer_size >= n_bytes);

    if (endian == QMI_ENDIAN_BIG)
        tmp = GUINT64_TO_BE (*in);
    else
        tmp = GUINT64_TO_LE (*in);
    memcpy (*buffer, &tmp, n_bytes);

    *buffer = &((*buffer)[n_bytes]);
    *buffer_size = (*buffer_size) - n_bytes;
}

/**
 * qmi_utils_read_string_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @length_prefix_size: size of the length prefix integer in bits.
 * @max_size: maximum number of bytes to read, or 0 to read all available bytes.
 * @out: return location for the read string. The returned value should be freed with g_free().
 *
 * Reads a string from the buffer.
 *
 * If @length_prefix_size is greater than 0, only the amount of bytes given
 * there will be read. Otherwise, up to @buffer_size bytes will be read.
 *
 * Also note that both @buffer and @buffer_size get updated after the write.
 */
void
qmi_utils_read_string_from_buffer (const guint8 **buffer,
                                   guint16       *buffer_size,
                                   guint8         length_prefix_size,
                                   guint16        max_size,
                                   gchar        **out)
{
    guint16 string_length;
    guint16 valid_string_length;
    guint8 string_length_8;
    guint16 string_length_16;

    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (length_prefix_size == 0 ||
              length_prefix_size == 8 ||
              length_prefix_size == 16);

    switch (length_prefix_size) {
    case 0:
        /* If no length prefix given, read the whole buffer into a string */
        string_length = *buffer_size;
        break;
    case 8:
        qmi_utils_read_guint8_from_buffer (buffer,
                                           buffer_size,
                                           &string_length_8);
        string_length = string_length_8;
        break;
    case 16:
        qmi_utils_read_guint16_from_buffer (buffer,
                                            buffer_size,
                                            QMI_ENDIAN_LITTLE,
                                            &string_length_16);
        string_length = string_length_16;
        break;
    default:
        g_assert_not_reached ();
    }

    if (max_size > 0 && string_length > max_size)
        valid_string_length = max_size;
    else
        valid_string_length = string_length;

    /* Read 'valid_string_length' bytes */
    *out = g_malloc (valid_string_length + 1);
    memcpy (*out, *buffer, valid_string_length);
    (*out)[valid_string_length] = '\0';

    /* And walk 'string_length' bytes */
    *buffer = &((*buffer)[string_length]);
    *buffer_size = (*buffer_size) - string_length;
}

/**
 * qmi_utils_read_fixed_size_string_from_buffer:
 * @buffer: a buffer with raw binary data.
 * @buffer_size: size of @buffer.
 * @fixed_size: number of bytes to read.
 * @out: return location for the read string. The returned value should be freed with g_free().
 *
 * Reads a @fixed_size-sized string from the buffer.
 *
 * Also note that both @buffer and @buffer_size get updated after the
 * @fixed_size bytes read.
 */
void
qmi_utils_read_fixed_size_string_from_buffer (const guint8 **buffer,
                                              guint16       *buffer_size,
                                              guint16        fixed_size,
                                              gchar         *out)
{
    g_assert (out != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (fixed_size > 0);

    memcpy (out, *buffer, fixed_size);

    *buffer = &((*buffer)[fixed_size]);
    *buffer_size = (*buffer_size) - fixed_size;
}

/**
 * qmi_utils_write_string_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @length_prefix_size: size of the length prefix integer in bits.
 * @in: string to write.
 *
 * Writes a string to the buffer.
 *
 * If @length_prefix_size is greater than 0, a length prefix integer will be
 * included in the write operation.
 *
 * The user needs to make sure that the buffer has enough space for both the
 * whole string and the length prefix.
 *
 * Also note that both @buffer and @buffer_size get updated after the write.
 */
void
qmi_utils_write_string_to_buffer (guint8      **buffer,
                                  guint16      *buffer_size,
                                  guint8        length_prefix_size,
                                  const gchar  *in)
{
    guint16 len;
    guint8 len_8;
    guint16 len_16;

    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (length_prefix_size == 0 ||
              length_prefix_size == 8 ||
              length_prefix_size == 16);

    len = (guint16) strlen (in);

    switch (length_prefix_size) {
    case 0:
        break;
    case 8:
        g_warn_if_fail (len <= G_MAXUINT8);
        len_8 = (guint8)len;
        qmi_utils_write_guint8_to_buffer (buffer,
                                          buffer_size,
                                          &len_8);
        break;
    case 16:
        g_warn_if_fail (len <= G_MAXUINT16);
        len_16 = (guint16)len;
        qmi_utils_write_guint16_to_buffer (buffer,
                                           buffer_size,
                                           QMI_ENDIAN_LITTLE,
                                           &len_16);
        break;
    default:
        g_assert_not_reached ();
    }

    memcpy (*buffer, in, len);
    *buffer = &((*buffer)[len]);
    *buffer_size = (*buffer_size) - len;
}

/**
 * qmi_utils_write_fixed_size_string_to_buffer:
 * @buffer: a buffer.
 * @buffer_size: size of @buffer.
 * @fixed_size: number of bytes to write.
 * @in: string to write.
 *
 * Writes a @fixed_size-sized string to the buffer, without any length prefix.
 *
 * The user needs to make sure that the buffer is at least @fixed_size bytes
 * long.
 *
 * Also note that both @buffer and @buffer_size get updated after the
 * @fixed_size bytes write.
 */
void
qmi_utils_write_fixed_size_string_to_buffer (guint8      **buffer,
                                             guint16      *buffer_size,
                                             guint16       fixed_size,
                                             const gchar  *in)
{
    g_assert (in != NULL);
    g_assert (buffer != NULL);
    g_assert (buffer_size != NULL);
    g_assert (fixed_size > 0);

    memcpy (*buffer, in, fixed_size);
    *buffer = &((*buffer)[fixed_size]);
    *buffer_size = (*buffer_size) - fixed_size;
}

/*****************************************************************************/

static volatile gint __traces_enabled = FALSE;

/**
 * qmi_utils_get_traces_enabled:
 *
 * Checks whether QMI message traces are currently enabled.
 *
 * Returns: %TRUE if traces are enabled, %FALSE otherwise.
 */
gboolean
qmi_utils_get_traces_enabled (void)
{
    return (gboolean) g_atomic_int_get (&__traces_enabled);
}

/**
 * qmi_utils_set_traces_enabled:
 * @enabled: %TRUE to enable traces, %FALSE to disable them.
 *
 * Sets whether QMI message traces are enabled or disabled.
 */
void
qmi_utils_set_traces_enabled (gboolean enabled)
{
    g_atomic_int_set (&__traces_enabled, enabled);
}
