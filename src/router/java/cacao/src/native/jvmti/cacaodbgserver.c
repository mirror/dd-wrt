
/* src/native/jvmti/cacaodbgserver.c - contains the cacaodbgserver process. This
                                       process controls the cacao vm through gdb

   Copyright (C) 1996-2005, 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

   Contact: cacao@complang.tuwien.ac.at

   Authors: Martin Platter

   Changes: Edwin Steiner
            Samuel Vinson

*/

#include "native/jvmti/cacaodbgserver.h"
#include "native/jvmti/cacaodbg.h"
#include "native/jvmti/dbg.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

FILE *gdbin, *gdbout; /* file descriptor for gdb pipes */

struct _pending_brkpt {
	int brknumber;
	unsigned long threadid;
	char* regs;
};

struct _pending_brkpt *pending_brkpts;
int pending_brkpts_size;


static void closepipeend (int fd) {
	if (close(fd) == -1) {
		perror("unable to close pipe - ");
		exit(1);
	}
}

/* startgdb *******************************************************************

   starts a gdb session and creates two pipes connection gdb stdout/stdin to
   gdbin/gdbout

*******************************************************************************/

static void startgdb() {
	char gdbargs[20];
	int cacao2gdbpipe[2],gdb2cacaopipe[2];

	pipe(cacao2gdbpipe);
	pipe(gdb2cacaopipe);

	snprintf(gdbargs,20,"--pid=%d",getppid());		

	switch(fork()) {
	case -1:
		fprintf(stderr,"cacaodbgserver: fork error\n");
		exit(1);
	case 0:		
		/* child */
		closepipeend(gdb2cacaopipe[0]); /* read end */
		closepipeend(cacao2gdbpipe[1]); /* write end */
		
		/* connect stdin of gdb to cacao2gdbpipe */
		dup2(cacao2gdbpipe[0],0);
		/* connect stdout of gdb to gdb2cacaopipe */
		dup2(gdb2cacaopipe[1],1);

		if (execlp("gdb","gdb","--interpreter=mi" ,gdbargs,(char *) NULL)==-1){
			fprintf(stderr,"cacaodbgserver: unable to start gdb\n");
			exit(1);
		}
	default:
		/* parent */
		closepipeend(gdb2cacaopipe[1]); /* write end */
		closepipeend(cacao2gdbpipe[0]); /* read end */

		gdbin = fdopen(gdb2cacaopipe[0],"r");
		gdbout = fdopen(cacao2gdbpipe[1],"w");
	}

}


#define SENDCMD(CMD) \
	fprintf(gdbout,"%s",CMD); \
	fflush(gdbout); \


static void getgdboutput(char *inbuf,int buflen) {
	int i=0;
	inbuf[0]='\0';
	do {
		i += strlen(&inbuf[i]);
		if (fgets(&inbuf[i],buflen-i,gdbin)==NULL) {
			perror("cacaodbgserver: ");
			exit(1);
		}
	} while(!(strncmp(OUTPUTEND,&inbuf[i],OUTPUTENDSIZE)==0));
}


/* dataevaluate ***************************************************************

   evaluates expr returning long in gdb and returns the result 

*******************************************************************************/

static unsigned long dataevaluate(char *expr) {
	char *match, inbuf[160];

	fprintf(gdbout,"-data-evaluate-expression %s\n",expr); 
	fflush(gdbout); 

	getgdboutput(inbuf,160);
	if ((match=strstr(inbuf,DATAEVALUATE)) == NULL) {
		fprintf(stderr,"dataevaluate: no matching value\n");
		return -1;
	}
	return strtoll(&match[strlen(DATAEVALUATE)], NULL, 16);
}


/* commonbreakpointhandler *****************************************************

   called by gdb and hard coded breakpoint handler

*******************************************************************************/

static bool commonbreakpointhandler(char* sigbuf, int sigtrap) {
	int numberofbreakpoints, i;
	char tmp[INBUFLEN], *match;
	unsigned long addr;

	if ((match=strstr(sigbuf,SIGADDR))==NULL) {
		fprintf(stderr,"commonbreakpointhandler: no matching address(%s)\n",
				sigbuf);
		return true;
	}

	addr = strtoll(&match[strlen(SIGADDR)],NULL,16);
	if (sigtrap) addr--;


	numberofbreakpoints = (int)dataevaluate("dbgcom->jvmtibrkpt.num");

	i = -1;
	do {
		i++; 
		snprintf(tmp,INBUFLEN,"dbgcom->jvmtibrkpt.brk[%d].addr",i);
	} while ((i<numberofbreakpoints) && (dataevaluate(tmp) != addr));

	assert(i<numberofbreakpoints);

	/* handle system breakpoints */
	switch(i) {
	case SETSYSBRKPT:
		/* add a breakpoint */
		fprintf(gdbout,"break *0x%lx\n",dataevaluate("dbgcom->brkaddr"));
		fflush(gdbout);
		getgdboutput(tmp,INBUFLEN);
		break;
	case CACAODBGSERVERQUIT:
		SENDCMD("-gdb-exit\n");
		return false;
	default:
		/* other breakpoints -> call jvmti_cacao_generic_breakpointhandler 
		   in the cacao vm */
		fprintf(gdbout,"call jvmti_cacao_generic_breakpointhandler(%d)\n",i);
		fflush(gdbout);
		getgdboutput(tmp,INBUFLEN);
	}
	SENDCMD(CONTINUE);
	getgdboutput(tmp,INBUFLEN);
	return true;
}

/* controlloop ****************************************************************

   this function controls the gdb behaviour

*******************************************************************************/

static void controlloop() {
	char inbuf[INBUFLEN], *match;
	bool running = true;

	pending_brkpts_size = 5;
	pending_brkpts = malloc(sizeof(struct _pending_brkpt)*pending_brkpts_size);

	getgdboutput(inbuf,INBUFLEN); 	/* read gdb welcome message */

	SENDCMD("handle SIGSEGV SIGPWR SIGXCPU SIGBUS noprint nostop\n");
	getgdboutput(inbuf,INBUFLEN);

	SENDCMD("print dbgcom\n");
	getgdboutput(inbuf,INBUFLEN);

	SENDCMD(CONTINUE);
	getgdboutput(inbuf,INBUFLEN);

	while(running) {
		getgdboutput(inbuf,INBUFLEN);

		if ((match=strstr(inbuf,HCSIGTRAP))!=NULL) {
			running = commonbreakpointhandler(match,1);
			continue;
			}

		if ((match=strstr(inbuf,GDBBREAKPOINT))!=NULL) {
			running = commonbreakpointhandler(match,0);
			continue;
		}

		if (strstr(inbuf,EXITEDNORMALLY) != NULL) {
			/* quit gdb */
			SENDCMD ("-gdb-exit");
			running = false;
			continue;
		}
			
		if ((inbuf[0]!=LOGSTREAMOUTPUT) && (inbuf[0]!=CONSOLESTREAMOUTPUT)) {
			fprintf(stderr,"gdbin not handled %s\n",inbuf);
			fprintf(gdbout,"bt\n");
			fflush(gdbout);
			fprintf(stderr,"not handled 1\n");
			fflush(stderr);
			getgdboutput(inbuf,INBUFLEN);
			fprintf(stderr,"gdbin: %s\n",inbuf);
			SENDCMD("-gdb-exit\n");
			return;
		}
	}

	free(pending_brkpts);
}

/* main (cacaodbgserver) ******************************************************

   main function for cacaodbgserver process.

*******************************************************************************/

int main(int argc, char **argv) {
	startgdb();

	controlloop();

	return 0;
}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
