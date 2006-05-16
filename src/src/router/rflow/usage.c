/*-
 * Copyright (c) 2001, 2004 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: usage.c,v 1.14 2004/04/12 09:07:18 vlm Exp $
 */


#include "rflow.h"

void
rflow_usage() {

	fprintf(stderr,
	RFLOW_VERSION_STRING
	RFLOW_COPYRIGHT "\n"
	"Usage: ipcad [-hv] [-d] [-c <config>] [-r] [-s]\n"
	"Options are:\n"
	"\t-h:\tthis page\n"
	"\t-v:\tversion information\n"
//	"\t-c:\tspecify configuration file\n"
	"\t-d:\tbecome daemon (otherwise, work in foreground)\n"
	"\t-i:\tinterface(s) to monitor\n"
	"\t-F:\tNetFlow destination (host:port)\n"
	"\t-A:\tActive flows timeout, sec\n"
	"\t-I:\tInactive flows timeout, sec\n"
	);

#ifdef	EX_USAGE
	exit(EX_USAGE);
#else
	exit(1);
#endif
};

void
rsh_usage(FILE *f) {

	if(!f)
		return;

	fprintf(f,
	"Builtin commands:\n"
	"\tshow ip accounting\t # Show actual IP accounting\n"
	"\tclear ip accounting\t # Move IP accounting to checkpoint\n"
	"\tshow ip accounting checkpoint\t # Show saved IP accounting\n"
	"\tclear ip accounting checkpoint\t # Clear checkpoint database\n"
	"\n"
	"\tshow ip cache flow\t # Show NetFlow cache\n"
	"\n"
	"\tshow interface <iface>\t # Show interface counters\n"
	"\n"
	"\tdump [<path>]   \t # Dump current IP accounting\n"
	"\trestore [<path>]\t # Restore IP accounting\n"
	"\timport [<path>] \t # Import (add) IP accounting\n"
	"\n"
	"\tstat            \t # Show current statistics\n"
	"\tshow version    \t # Show version and uptime\n"
	"\n"
	"\tshutdown        \t # Shutdown ipcad\n"
	);

};

