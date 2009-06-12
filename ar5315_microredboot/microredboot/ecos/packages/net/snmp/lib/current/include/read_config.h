//==========================================================================
//
//      ./lib/current/include/read_config.h
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
/*
 *  read_config.h: reads configuration files for extensible sections.
 *
 */
#ifndef READ_CONFIG_H
#define READ_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define STRINGMAX 1024

#define NORMAL_CONFIG 0
#define PREMIB_CONFIG 1
#define EITHER_CONFIG 2



/*
 * Defines a set of file types and the parse and free functions
 * which process the syntax following a given token in a given file.
 */
struct config_files {
   char			*fileHeader;	/* Label for entire file. */
   struct config_line	*start;
   struct config_files	*next;
};

struct config_line {
   char			 *config_token;	/* Label for each line parser
                                           in the given file. */
   void			(*parse_line) (const char *, char *);
   void			(*free_func) (void);
   struct config_line	 *next;
   char			  config_time;	/* {NORMAL,PREMIB,EITHER}_CONFIG */
   char                  *help;
};

void read_config (const char *, struct config_line *, int);
void read_configs (void);
void read_premib_configs (void);
void read_config_files (int);
void free_config (void);
void config_perror (const char *);
void config_pwarn (const char *);
char *skip_white (char *);
char *skip_not_white (char *);
char *skip_token(char *);
char *copy_word (char *, char *);
void read_config_with_type (const char *, const char *);
struct config_line *register_config_handler (const char *, const char *,
                                             void (*parser)(const char *, char *),
                                             void (*releaser) (void),
                                             const char *);
struct config_line *register_app_config_handler (const char *,
                                             void (*parser)(const char *, char *),
                                             void (*releaser) (void),
                                             const char *);
struct config_line *register_premib_handler (const char *, const char *,
                                             void (*parser)(const char *, char *),
                                             void (*releaser) (void),
                                             const char *);
struct config_line *register_app_premib_handler (const char *,
                                             void (*parser)(const char *, char *),
                                             void (*releaser) (void),
                                             const char *);
void unregister_config_handler (const char *, const char *);
void unregister_app_config_handler (const char *);
void read_config_print_usage(const char *lead);
char *read_config_save_octet_string(char *saveto, u_char *str, size_t len);
char *read_config_read_octet_string(char *readfrom, u_char **str, size_t *len);
char *read_config_read_objid(char *readfrom, oid **objid, size_t *len);
char *read_config_save_objid(char *saveto, oid *objid, size_t len);
char *read_config_read_data(int type, char *readfrom, void *dataptr, size_t *len);
char *read_config_store_data(int type, char *storeto, void *dataptr, size_t *len);
void  read_config_store(const char *type, const char *line);
void  read_app_config_store(const char *line);
void  snmp_save_persistent(const char *type);
void  snmp_clean_persistent(const char *type);

#ifdef __cplusplus
}
#endif

#endif /* READ_CONFIG_H */
