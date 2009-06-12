//==========================================================================
//
//      ledit.h
//
//      Header for the utterly simple line editor
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      Header for the udderly simple line editor
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#ifndef LEDIT_H
#define LEDIT_H

#define LF '\n'
#define CR '\r'
#define BOLCMD '\001'
#define EOLCMD '\005'
#define FORWCMD '\006'
#define BACKCMD '\002'
#define DELBACK '\010'
#define DELEOL '\013'
#define YANKCH '\031'
#define DELCURRCH '\004'
#define PREVCMD '\020'
#define ERASELINE '\025'
#define NEXTCMD '\016'

/* Prompt for one line of input using PROMPT. The input from the user
   (up to MAXINPLEN characters) will be stored in BUFFER. The number
   of characters read will be returned. */
extern int lineedit(char *prompt, char *buffer, int maxInpLen);

/* Add CMD to the user's command history. */
extern void addHistoryCmd (char *cmd);

/* Configure the editor to use the specified terminal. */
extern void set_term_name (char *name);

/* Beep the terminal. */
extern void beep (void);

/* Print a history list. */
extern void printHistoryList(void);

#endif
