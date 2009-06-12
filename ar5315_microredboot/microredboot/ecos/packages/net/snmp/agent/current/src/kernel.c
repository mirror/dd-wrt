//==========================================================================
//
//      ./agent/current/src/kernel.c
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
 *  13 Jun 91  wsak (wk0x@andrew) added mips support
 */

#include <config.h>

#ifdef CAN_USE_NLIST

#include <sys/types.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <errno.h>
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_KVM_H
#include <kvm.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#include "snmp_impl.h"
#include "snmp_logging.h"
#include "default_store.h"

#include "kernel.h"
#include "ds_agent.h"

#ifndef NULL
#define NULL 0
#endif


#if HAVE_KVM_H
kvm_t *kd;

void
init_kmem(const char *file)
{
#if HAVE_KVM_OPENFILES
    char err[4096];
    kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, err);
    if (kd == NULL && !ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_NO_ROOT_ACCESS)) {
	snmp_log(LOG_CRIT, "init_kmem: kvm_openfiles failed: %s\n", err);
	exit(1);
    }
#else
    kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
    if (!kd && !ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_NO_ROOT_ACCESS)) {
	snmp_log(LOG_CRIT, "init_kmem: kvm_open failed: %s\n", strerror(errno));
	exit(1);
    }
#endif	/* HAVE_KVM_OPENFILES */
}


/*
 *  klookup:
 *
 *  It seeks to the location  off  in kmem
 *  It does a read into  target  of  siz  bytes.
 *
 *  Return 0 on failure and 1 on sucess.
 *
 */


int
klookup(unsigned long off,
	char   *target,
	int     siz)
{
    int result;
    if (kd == NULL) return 0;
    result = kvm_read(kd, off, target, siz);
    if (result != siz) {
#if HAVE_KVM_OPENFILES
 snmp_log(LOG_ERR,"kvm_read(*, %lx, %p, %d) = %d: %s\n", off, target, siz,
		result, kvm_geterr(kd));
#else
 snmp_log(LOG_ERR,"kvm_read(*, %lx, %p, %d) = %d: ", off, target, siz,
		result);
	snmp_log_perror("klookup");
#endif
	return 0;
    }
    return 1;
}

#else /* HAVE_KVM_H */

static off_t klseek (off_t);
static int klread (char *, int);
int swap, mem, kmem;

void
init_kmem(const char *file)
{
  kmem = open(file, O_RDONLY);
  if (kmem < 0 && !ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_NO_ROOT_ACCESS)){
    snmp_log_perror(file);
    exit(1);
  }
  fcntl(kmem,F_SETFD,1);
  mem = open("/dev/mem",O_RDONLY);    
  if (mem < 0 && !ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_NO_ROOT_ACCESS)){
    snmp_log_perror("/dev/mem");
    exit(1);
  }
  fcntl(mem,F_SETFD,1);
#ifdef DMEM_LOC
  swap = open(DMEM_LOC,O_RDONLY);
  if (swap < 0 && !ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_NO_ROOT_ACCESS)){
    snmp_log_perror(DMEM_LOC);
    exit(1);
  }
  fcntl(swap,F_SETFD,1);
#endif
}


/*
 *  Seek into the kernel for a value.
 */
static off_t
klseek(off_t base)
{
  return (lseek(kmem, (off_t)base, SEEK_SET));
}


/*
 *  Read from the kernel 
 */
static int
klread(char *buf,
       int buflen)
{
  return (read(kmem, buf, buflen));
}


/*
 *  klookup:
 *
 *  It seeks to the location  off  in kmem
 *  It does a read into  target  of  siz  bytes.
 *
 *  Return 0 on failure and 1 on sucess.
 *
 */


int
klookup(unsigned long off,
	char   *target,
	int     siz)
{
  long retsiz;

  if (kmem < 0) return 0;

  if ((retsiz = klseek((off_t) off)) != off) {
    snmp_log(LOG_ERR, "klookup(%lx, %p, %d): ", off, target, siz);
    snmp_log_perror("klseek");
#ifdef EXIT_ON_BAD_KLREAD
    exit(1);
#endif
    return (0);
  }
  if ((retsiz = klread(target, siz)) != siz ) {
    if (snmp_get_do_debugging()) {
    /* these happen too often on too many architectures to print them
       unless we're in debugging mode. People get very full log files. */
      snmp_log(LOG_ERR, "klookup(%lx, %p, %d): ", off, target, siz);
      snmp_log_perror("klread");
    }
#ifdef EXIT_ON_BAD_KLREAD
    exit(1);
#endif
    return(0);
  }
  return (1);
}

#endif /* HAVE_KVM_H */

#endif /* CAN_USE_NLIST */
