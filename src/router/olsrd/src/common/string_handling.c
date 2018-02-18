/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include <assert.h>
#include <string.h>

#include "common/string_handling.h"

/**
 * A somewhat safe version of strncpy.
 * 1) This function never writes more than dest_size bytes.
 * 2) This function always terminates the content of the dest buffer with
 *    a zero byte.
 * 3) This function does not write more than strlen(src)+1 bytes
 *
 * @param dest pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param dest_size length of destination buffer in bytes
 *
 * @return pointer to destination buffer
 */
static char *_internal_strscpy(char *dest, const char *src, size_t dest_size) {
  /* number of bytes to be copied without null byte */
  size_t length = 0;

  assert(dest);
  assert(src);
  assert(dest_size);

  /* reserve space for null byte in dest */
  dest_size--;

  /* determine src length (without null byte) */
  while ((length < dest_size) && src[length]) {
    length++;
  }

  /* force null byte */
  dest[length] = 0;

  /* copy src content without null byte */
  return strncpy(dest, src, length);
}

/**
 * A somewhat safe version of strncpy.
 * 1) This function never writes more than dest_size bytes.
 * 2) This function always terminates the content of the dest buffer with
 *    a zero byte.
 * 3) This function does not write more than strlen(src)+1 bytes
 *
 * @param dest pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param dest_size length of destination buffer in bytes
 *
 * @return pointer to destination buffer
 */
char * strscpy(char *dest, const char *src, size_t dest_size) {
  assert(dest);
  assert(src);
  assert(dest_size);

  /* paranoid checks */
  if (!dest || !src || !dest_size) {
    return dest;
  }

  return _internal_strscpy(dest, src, dest_size);
}

/**
 * A somewhat safe version of strncat, it appends the string
 * content of src to dest.
 * 1) This function never writes more than dest_size bytes.
 * 2) This function always terminates the content of the dest buffer with
 *    a zero byte.
 * 3) This function does not write more than strlen(src)+1 bytes
 *
 * The function returns an error if dst or src or dst_size is zero.
 *
 * @param dest pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param dest_size length of destination buffer in bytes
 *
 * @return pointer to destination buffer, NULL if an error happened
 */
char * strscat(char *dest, const char *src, size_t dest_size) {
  size_t dst_content_len;

  assert(dest);
  assert(src);
  assert(dest_size);

  /* paranoid checks */
  if (!dest || !src || !dest_size) {
    return dest;
  }

  dst_content_len = strlen(dest);
  if ((dest_size - dst_content_len) <= 0) {
    return dest;
  }
  return _internal_strscpy(dest + dst_content_len, src, dest_size - dst_content_len);
}
