/*
     This file is part of libmicrohttpd
     Copyright (C) 2007-2022 Daniel Pittman, Christian Grothoff, and Evgeny Grin

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file postprocessor.h
 * @brief  Declarations for parsing POST data
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_POSTPROCESSOR_H
#define MHD_POSTPROCESSOR_H 1
#include "internal.h"

/**
 * States in the PP parser's state machine.
 */
enum PP_State
{
  /* general states */
  PP_Error,
  PP_Done,
  PP_Init,
  PP_NextBoundary,

  /* url encoding-states */
  PP_ProcessKey,
  PP_ProcessValue,
  PP_Callback,

  /* post encoding-states  */
  PP_ProcessEntryHeaders,
  PP_PerformCheckMultipart,
  PP_ProcessValueToBoundary,
  PP_PerformCleanup,

  /* nested post-encoding states */
  PP_Nested_Init,
  PP_Nested_PerformMarking,
  PP_Nested_ProcessEntryHeaders,
  PP_Nested_ProcessValueToBoundary,
  PP_Nested_PerformCleanup

};


enum RN_State
{
  /**
   * No RN-preprocessing in this state.
   */
  RN_Inactive = 0,

  /**
   * If the next character is CR, skip it.  Otherwise,
   * just go inactive.
   */
  RN_OptN = 1,

  /**
   * Expect CRLF (and only CRLF).  As always, we also
   * expect only LF or only CR.
   */
  RN_Full = 2,

  /**
   * Expect either CRLF or '--'CRLF.  If '--'CRLF, transition into dash-state
   * for the main state machine
   */
  RN_Dash = 3,

  /**
   * Got a single dash, expect second dash.
   */
  RN_Dash2 = 4
};


/**
 * Bits for the globally known fields that
 * should not be deleted when we exit the
 * nested state.
 */
enum NE_State
{
  NE_none = 0,
  NE_content_name = 1,
  NE_content_type = 2,
  NE_content_filename = 4,
  NE_content_transfer_encoding = 8
};


/**
 * Internal state of the post-processor.  Note that the fields
 * are sorted by type to enable optimal packing by the compiler.
 */
struct MHD_PostProcessor
{

  /**
   * The connection for which we are doing
   * POST processing.
   */
  struct MHD_Connection *connection;

  /**
   * Function to call with POST data.
   */
  MHD_PostDataIterator ikvi;

  /**
   * Extra argument to ikvi.
   */
  void *cls;

  /**
   * Encoding as given by the headers of the connection.
   */
  const char *encoding;

  /**
   * Primary boundary (points into encoding string)
   */
  const char *boundary;

  /**
   * Nested boundary (if we have multipart/mixed encoding).
   */
  char *nested_boundary;

  /**
   * Pointer to the name given in disposition.
   */
  char *content_name;

  /**
   * Pointer to the (current) content type.
   */
  char *content_type;

  /**
   * Pointer to the (current) filename.
   */
  char *content_filename;

  /**
   * Pointer to the (current) encoding.
   */
  char *content_transfer_encoding;

  /**
   * Value data left over from previous iteration.
   */
  char xbuf[2];

  /**
   * Size of our buffer for the key.
   */
  size_t buffer_size;

  /**
   * Current position in the key buffer.
   */
  size_t buffer_pos;

  /**
   * Current position in @e xbuf.
   */
  size_t xbuf_pos;

  /**
   * Current offset in the value being processed.
   */
  uint64_t value_offset;

  /**
   * strlen(boundary) -- if boundary != NULL.
   */
  size_t blen;

  /**
   * strlen(nested_boundary) -- if nested_boundary != NULL.
   */
  size_t nlen;

  /**
   * Do we have to call the 'ikvi' callback when processing the
   * multipart post body even if the size of the payload is zero?
   * Set to #MHD_YES whenever we parse a new multiparty entry header,
   * and to #MHD_NO the first time we call the 'ikvi' callback.
   * Used to ensure that we do always call 'ikvi' even if the
   * payload is empty (but not more than once).
   */
  bool must_ikvi;

  /**
   * Set if we still need to run the unescape logic
   * on the key allocated at the end of this struct.
   */
  bool must_unescape_key;

  /**
   * State of the parser.
   */
  enum PP_State state;

  /**
   * Side-state-machine: skip CRLF (or just LF).
   * Set to 0 if we are not in skip mode.  Set to 2
   * if a CRLF is expected, set to 1 if a CR should
   * be skipped if it is the next character.
   */
  enum RN_State skip_rn;

  /**
   * If we are in skip_rn with "dash" mode and
   * do find 2 dashes, what state do we go into?
   */
  enum PP_State dash_state;

  /**
   * Which headers are global? (used to tell which
   * headers were only valid for the nested multipart).
   */
  enum NE_State have;

};

#endif /* ! MHD_POSTPROCESSOR_H */
