/*
 * ProFTPD - FTP server fuzzing testsuite
 * Copyright (c) 2021-2024 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "json.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  pool *p = NULL;
  char *text = NULL;
  const char *large_text, *malformed_text, *nested_text;
  pr_json_object_t *json = NULL;

  text = (char *) malloc(size + 1);
  if (text == NULL) {
    return 0;
  }

  memcpy(text, data, size);
  text[size] = '\0';

  p = make_sub_pool(NULL);
  if (p == NULL) {
    free(text);
    return 0;
  }

  init_json();

  json = pr_json_object_from_text(p, text);
  pr_json_object_free(json);

  malformed_text = "{\"key\": \"value\",}";
  json = pr_json_object_from_text(p, malformed_text);
  pr_json_object_free(json);

  large_text = "{\"key\": \"value\", \"key2\": \"value2\", \"key3\": \"value3\", \"key4\": \"value4\"}";
  json = pr_json_object_from_text(p, large_text);
  pr_json_object_free(json);

  nested_text = "{\"key\": {\"subkey\": {\"subsubkey\": {\"subsubsubkey\": \"value\"}}}}";
  json = pr_json_object_from_text(p, nested_text);
  pr_json_object_free(json);

  /* Provide deliberately invalid UTF-8 sequences as input now. */
  malformed_text = "{\"key\": \"\x80\x81\x82\"}";
  json = pr_json_object_from_text(p, malformed_text);
  pr_json_object_free(json);

  finish_json();
  destroy_pool(p);

  free(text);
  return 0;
}
