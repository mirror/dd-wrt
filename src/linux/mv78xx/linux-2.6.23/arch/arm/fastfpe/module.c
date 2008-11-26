/*
    Fast Floating Point Emulator
    (c) Peter Teichmann <mail@peter-teichmann.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/init.h>

#ifndef MODULE
#define kern_fp_enter	fp_enter

extern char fpe_type[];
#endif

static void (*orig_fp_enter)(void);	/* old kern_fp_enter value */
extern void (*kern_fp_enter)(void);	/* current FP handler */
extern void fastfpe_enter(void);	/* forward declarations */
extern int fastfpe_test(void);		/* long multiply available ? */

static int __init fpe_init(void)
{
  if (fpe_type[0] && strcmp(fpe_type, "fastfpe"))
    return 0;

  printk("Fast Floating Point Emulator V0.94");
  if (fastfpe_test() == 1) printk("M");
  printk(" by Peter Teichmann.\n");

  /* Save pointer to the old FP handler and then patch ourselves in */
  orig_fp_enter = kern_fp_enter;
  kern_fp_enter = fastfpe_enter;

  return 0;
}

static void __exit fpe_exit(void)
{
  /* Restore the values we saved earlier. */
  kern_fp_enter = orig_fp_enter;
}

module_init(fpe_init);
module_exit(fpe_exit);

MODULE_AUTHOR("Peter Teichmann <mail@peter-teichmann.de>");
MODULE_DESCRIPTION("Fast floating point emulator with full precision");
