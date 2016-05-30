/* src/toolbox/logging.cpp - contains logging functions

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/

#include "config.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <inttypes.h>

#include "vm/method.hpp"
#include "vm/types.hpp"

#include "threads/thread.hpp"

#include "toolbox/logging.hpp"
#include "toolbox/util.hpp"

#include "vm/statistics.hpp"

#ifdef ENABLE_LOGGING

using namespace cacao;

static Color current_log_color = BoldWhite;

OStream& cacao::dbg() {
	static OStream stream(stdout);

	return stream;
}

void cacao::set_log_file(FILE *file) {
	dbg().set_file(file);
}

void cacao::set_log_color(Color color) {
	current_log_color = color;
}

Color cacao::log_color() {
	return current_log_color;
}

#endif

/***************************************************************************
                        LOG FILE HANDLING 
***************************************************************************/

static FILE *LOG_FILE = NULL;

void log_init(const char *fname)
{
	if (fname) {
		if (fname[0]) {
			LOG_FILE = fopen(fname, "w");
		}
	}
}

static inline FILE* get_log() 
{
	return LOG_FILE ? LOG_FILE : stdout;
}

// TODO: remove
FILE* log_get_logfile() 
{
	return get_log();
}

/***************************************************************************
                        LOG ENTRY HEADER/FOOTER
***************************************************************************/

/* log_start *******************************************************************

   Writes the preleading LOG: text to the protocol file (if opened) or
   to stdout.

*******************************************************************************/

void log_start(void)
{
	fprintf(get_log(), "LOG: [0x%"PRIxPTR"] ", threads_get_current_tid());
}

/* log_finish ******************************************************************

   Finishes a logtext line with trailing newline and a fflush.

*******************************************************************************/

void log_finish(void)
{
	FILE* log = get_log();

	fputs("\n", log);
	fflush(log);
}

/***************************************************************************
                        PRINT TO CURRENT LOG ENTRY
***************************************************************************/

/* log_vprint ******************************************************************

   Writes logtext to the protocol file (if opened) or to stdout.

*******************************************************************************/

void log_vprint(const char *text, va_list ap)
{
	FILE* log = get_log();

	os::vfprintf(log, text, ap);
}


/* log_print *******************************************************************

   Writes logtext to the protocol file (if opened) or to stdout.

*******************************************************************************/

void log_print(const char *text, ...)
{
	va_list ap;

	va_start(ap, text);
	log_vprint(text, ap);
	va_end(ap);
}

/* log_classname ***************************************************************

   Writes utf string to the protocol replacing '/' by '.'

*******************************************************************************/

void log_classname(Utf8String u)
{
	FILE* log = get_log();

	assert(u);

	Utf8String                str = u;
	Utf8String::byte_iterator it  = str.begin();
	Utf8String::byte_iterator end = str.end();

	for (; it != end; ++it) {
		char c = *it;

		fputc(c=='/' ? '.' : c, log);
	}
}


/***************************************************************************
                        PRINT WHOLE LOG ENTRY
***************************************************************************/

/* log_println *****************************************************************

   Writes logtext to the protocol file (if opened) or to stdout with a
   trailing newline.

*******************************************************************************/

void log_println(const char *text, ...)
{
	va_list ap;

	log_start();

	va_start(ap, text);
	log_vprint(text, ap);
	va_end(ap);

	log_finish();
}

/* log_message_utf *************************************************************

   Outputs log text like this:

   LOG: Creating class: java/lang/Object

*******************************************************************************/

void log_message_utf(const char *msg, Utf8String u)
{
	log_start();

	FILE* log = get_log();

	Utf8String str = u; // TODO: remove

	fputs(msg, log);
	fputs(str.begin(), log);

	log_finish();
}


/* log_message_class ***********************************************************

   Outputs log text like this:

   LOG: Loading class: java/lang/Object

*******************************************************************************/

void log_message_class(const char *msg, classinfo *c)
{
	log_message_utf(msg, c->name);
}


/* log_message_class_message_class *********************************************

   Outputs log text like this:

   LOG: Initialize super class java/lang/Object from java/lang/VMThread

*******************************************************************************/

void log_message_class_message_class(const char *msg1, classinfo *c1,
									 const char *msg2, classinfo *c2)
{
	log_start();

	FILE* log = get_log();

	fputs(msg1, log);
	fputs(Utf8String(c1->name).begin(), log);
	fputs(msg2, log);
	fputs(Utf8String(c2->name).begin(), log);

	log_finish();
}


/* log_message_method **********************************************************

   Outputs log text like this:

   LOG: Compiling: java.lang.Object.clone()Ljava/lang/Object;

*******************************************************************************/

void log_message_method(const char *msg, methodinfo *m)
{
	log_start();

	FILE* log = get_log();

	fputs(msg, log);
	log_classname( m->clazz->name );
	fputc('.', log);
	fputs(Utf8String(m->name).begin(), log);
	fputs(Utf8String(m->descriptor).begin(), log);

	log_finish();
}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
