#ifndef PROC_SIG_H
#define PROC_SIG_H
/*
 * Copyright 1998-2003 by Albert Cahalan; all rights resered.
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */

/* return -1 on failure */
extern int signal_name_to_number(const char *name);
extern const char *signal_number_to_name(int signo);
extern int skill_sig_option(int *argc, char **argv);
extern const char *strtosig(const char *s);
extern void unix_print_signals(void);
extern void pretty_print_signals(void);

#endif
