//=============================================================================
//
//      test_menu.h - Cyclone Diagnostics
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/*****************************************************************************
* test_menu.h - Menu dispatching header
* 
* modification history
* --------------------
* 30aug00 ejb Ported from MPC8240 Breeze to StrongARM2 Cygmon
*/
/*
DESCRIPTION

Include file for the table-driven menu module menu.c.  This module displays
a menu of numbered items, prompts the user for a choice, then calls an action
routine associated with the choice.  It then redisplays the menu and prompts
for another choice.  A choice of zero (0) exits the menu routine.

To use this menu module, construct a table of type MENU_ITEM, with one entry
for each menu item.  An entry consists of a string to print out with the
menu, an action routine, and an argument for the action routine.  There is
no need to have an "exit" or "quit" item, since the menu routine handles
that by assigning at the zero (0) item number.  For example, a simple menu
of tests would be constructed this way:

	IMPORT VOID doTestA ();
	IMPORT VOID doTestB ();

	#define TESTA_ARG	0
	#define TESTB_ARG	1

	LOCAL MENU_ITEM testMenu[] =
	{
	{"Test A", doTestA, (MENU_ARG)TESTA_ARG},
	{"Test B", doTestB, (MENU_ARG)TESTB_ARG},
	};

	#define	NUM_ITEMS NELEMENTS(testMenu)
	#define MENU_TITLE "Test Menu"

Then, in the test top level routine, call the routine menu(), declared as:

	STATUS menu (menuTable, numMenuItems, title, options)
	MENU_ITEM	menuTable[];
	int		numMenuItems;
	char		*title;
	ULONG		options;

For example:

	status = menu (testMenu, NUM_ITEMS, MENU_TITLE, MENU_OPT_NONE);

*/

/*
 * The following types define an entry in the menu table.  Each entry describes
 * one menu item, including a string to describe that item, an action
 * routine to call when that item is chosen, and an argument to pass when the
 * action routine is called.
 */
typedef volatile void *MENU_ARG;
typedef void (*MENU_RTN) (MENU_ARG);
typedef struct menuItem
{
    char	*itemName;	/* string to print with the menu */
    MENU_RTN	actionRoutine;	/* routine to call when item is chosen */
    MENU_ARG	arg;		/* argument to actionRoutine */
} MENU_ITEM;


/*
 * Menu options
 *
 * These options control the display of the menu.
 *
 * MENU_OPT_NONE - The normal behavior.  The menu is always displayed before
 * prompting the user for a choice.
 *
 * MENU_OPT_SUPPRESS_DISP - The menu is displayed before prompting the user
 * for a choice except after executing a previously valid choice.  This option
 * is useful for large menus consisting of simple commands that produce only
 * a line or two of output.  In this case, after the user executes a valid
 * choice, the redisplay of a large menu may cause output to scroll off the
 * screen.  The menu is redisplayed if the user enters an invalid choice.
 */
#define MENU_OPT_NONE			0x00
#define MENU_OPT_SUPPRESS_DISP	0x01
#define MENU_OPT_VALUE			0x02




MENU_ARG menu (MENU_ITEM		menuTable[],
		       int				numMenuItems,
		       char				*title,
		       unsigned long	options);

