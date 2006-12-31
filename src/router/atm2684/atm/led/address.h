/*
 * Marko Kiiskila carnil@cs.tut.fi 
 * 
 * Tampere University of Technology - Telecommunications Laboratory
 *
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * TUT ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 * 
 */

/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#ifndef ADDRESS_H
#define ADDRESS_H

/* Get ESI for the physical ATM interface */
int addr_getesi(unsigned char *mac_addr, int phys_addr);

/* Get the ATM address for the physical ATM interface */
int get_listenaddr(unsigned char *atm_addr, int phys_addr);

#endif /* ADDRESS_H */
