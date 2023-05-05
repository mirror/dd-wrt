/* $Id$ */
/*
** file_decomp_PDF.h
**
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef FILE_DECOMP_PDF_H
#define FILE_DECOMP_PDF_H

#include <zlib.h>

#define ELEM_BUF_LEN        (12)
#define FILTER_SPEC_BUF_LEN (40)
#define PARSE_STACK_LEN     (12)

typedef enum pdf_states
{
    PDF_STATE_NEW = 0,
    PDF_STATE_LOCATE_STREAM = 1, /* Found sig bytes, looking for dictionary & stream*/
    PDF_STATE_INIT_STREAM,       /* Init stream */
    PDF_STATE_PROCESS_STREAM     /* Processing stream */
} fd_PDF_States;

typedef struct fd_PDF_Parse_Stack_s
{
    uint8_t State;
    uint8_t Sub_State;
} fd_PDF_Parse_Stack_t, *fd_PDF_Parse_Stack_p_t;

typedef struct fd_PDF_Parse_s
{
    uint8_t Dict_Nesting_Cnt;
    uint8_t Elem_Buf[ELEM_BUF_LEN];
    uint8_t Elem_Index;
    uint8_t Filter_Spec_Buf[FILTER_SPEC_BUF_LEN+1];
    uint8_t Filter_Spec_Index;
    fd_PDF_Parse_Stack_t Parse_Stack[PARSE_STACK_LEN];
    uint8_t Parse_Stack_Index;
    uint32_t Obj_Number;
    uint32_t Gen_Number;
    uint8_t Sub_State;
    uint8_t State;
} fd_PDF_Parse_t, *fd_PDF_Parse_p_t;

typedef struct fd_PDF_Deflate_s
{
    z_stream StreamDeflate;
} fd_PDF_Deflate_t;

typedef struct fd_PDF_s
{
    union
    {
        fd_PDF_Deflate_t Deflate;
    } PDF_Decomp_State;
    fd_PDF_Parse_t Parse;
    uint8_t Decomp_Type;
    uint8_t State;
} fd_PDF_t, *fd_PDF_p_t;

/* API Functions */

fd_status_t File_Decomp_Init_PDF( fd_session_p_t SessionPtr );

fd_status_t File_Decomp_End_PDF( fd_session_p_t SessionPtr );

fd_status_t File_Decomp_PDF();

#endif /* FILE_DECOMP_PDF_H */
