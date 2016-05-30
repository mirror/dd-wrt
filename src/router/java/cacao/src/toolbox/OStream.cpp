/* src/toolbox/OStream.cpp - simple output stream

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

#include "toolbox/OStream.hpp"

#include "threads/thread.hpp"

#include <cassert>

namespace cacao {

OStream& err() {
	static OStream stream(stderr);

	return stream;
}

OStream& out() {
	static OStream stream(stdout);

	return stream;
}

FillZero   fillzero;
Left       left;
Right      right;
Dec        dec;
Oct        oct;
Hex        hex;
FloatDec   float_dec;
Scientific scientific;
FloatHex   float_hex;
Indent     indent;
Dedent     dedent;
Nl         nl;
Flush      flush;

ThreadId threadid;

ResetColor  reset_color;
Bold        bold;
NoBold      nobold;
Underline   underline;
NoUnderline nounderline;

OStream::OStream(FILE *file) : file(file), newline(true) {
	assert(file);
	init_flag_defaults();
	// in practice all tty's support ansi escape codes
	use_color = isatty(fileno(file));
}

OStream::OStream(const OStream& os) : file(os.file), newline(os.newline), use_color(os.use_color) {
	init_flag_defaults();
}

void OStream::init_flag_defaults() {
	init_transient_flags();
	init_persistent_flags();
}

void OStream::init_transient_flags() {
	// keep comments in Flags class in sync with this!

	width = 0;
	precision = -1;
	fill_zero = false;
}

void OStream::init_persistent_flags() {
	// keep comments in Flags class in sync with this!

	align           = Align_right;
	int_fmt         = OStream::IntFmt_decimal;
	float_fmt       = OStream::FloatFmt_decimal;
	indent_lvl      = 0;
	prefix          = NULL;
	prefix_color    = InvalidColor;
}

void OStream::on_newline() {
	if (!newline) return;

	newline = false;

	// set color

	if (prefix_color != InvalidColor) (*this) << prefix_color;

	// print prefix

	if (prefix) {
		fputs(prefix, file);
		fputc(' ', file);
	}

	// reset color

	if (prefix_color != InvalidColor) (*this) << reset_color;

	// indent

	for (size_t indent = indent_lvl * 4; indent > 0; indent--) {
		fputc(' ', file);
	}
}

#define PRINT_INT_FLAG(DEC, OCT, HEX, VAL, FLAG)                                        \
	switch (int_fmt) {                                                                  \
		case IntFmt_decimal:                                                            \
			fprintf(file, "%" FLAG "*" #DEC, (int) width, VAL);break;                   \
		case IntFmt_octal:                                                              \
			fprintf(file, "%" FLAG "*" #OCT, (int) width, VAL);break;                   \
		case IntFmt_hexadecimal:                                                        \
			fprintf(file, "%" FLAG "*" #HEX, (int) width, VAL);break;                   \
		default:                 assert(false && "Bad int format");                     \
	}

#define PRINT_INT(DEC, OCT, HEX, VAL)                                                   \
	switch (align) {                                                                    \
	case Align_left:                                                                    \
		/* left align (i.e. - ) and zero padding not valid */                           \
		PRINT_INT_FLAG(DEC,OCT,HEX,VAL,"-");                                            \
		break;                                                                          \
	case Align_right:                                                                   \
		if (fill_zero) {                                                                \
			PRINT_INT_FLAG(DEC,OCT,HEX,VAL,"0");                                        \
		} else {                                                                        \
			PRINT_INT_FLAG(DEC,OCT,HEX,VAL,"");                                         \
		}                                                                               \
		break;                                                                          \
	default:                                                                            \
		assert(false && "Bad alignment");                                               \
	}

#define PRINT_FLOAT_FLAG(DEC, SCI, HEX, VAL, FLAG)                                      \
	switch (float_fmt) {                                                                \
		case FloatFmt_decimal:                                                          \
			fprintf(file, "%" FLAG "*.*" #DEC, (int) width, precision, VAL); break;     \
		case FloatFmt_scientific:                                                       \
			fprintf(file, "%" FLAG "*.*" #SCI, (int) width, precision, VAL); break;     \
		case FloatFmt_hexadecimal:                                                      \
			fprintf(file, "%" FLAG "*.*" #HEX, (int) width, precision, VAL); break;     \
		default:                                                                        \
			assert(false && "Bad float format");                                        \
	}

#define PRINT_FLOAT(DEC, SCI, HEX, VAL)                                                 \
	switch (align) {                                                                    \
	case Align_left:                                                                    \
		/* left align (i.e. - ) and zero padding not valid */                           \
		PRINT_FLOAT_FLAG(DEC,SCI,HEX,VAL,"-");                                          \
		break;                                                                          \
	case Align_right:                                                                   \
		if (fill_zero) {                                                                \
			PRINT_FLOAT_FLAG(DEC,SCI,HEX,VAL,"0");                                      \
		} else {                                                                        \
			PRINT_FLOAT_FLAG(DEC,SCI,HEX,VAL,"");                                       \
		}                                                                               \
		break;                                                                          \
	default:                                                                            \
		assert(false && "Bad alignment");                                               \
	}

OStream& OStream::operator<<(char c) {
	on_newline();

	if (!width) {
		fputc(c, file);
	} else {
		switch (align) {
		case Align_left:
			fprintf(file, "%-*c", (int) width, c);
			break;
		case Align_right:
			fprintf(file, "%*c", (int) width, c);
			break;
		}
	}

	init_transient_flags();
	return (*this);
}
OStream& OStream::operator<<(bool b) {
	return (*this) << (b ? "true" : "false");
}
OStream& OStream::operator<<(long n) {
	on_newline();

	PRINT_INT(ld, lo, lx, n);

	init_transient_flags();
	return (*this);
}
OStream& OStream::operator<<(long long n) {
	on_newline();

	PRINT_INT(lld, llo, llx, n);

	init_transient_flags();
	return (*this);
}
OStream& OStream::operator<<(unsigned long n) {
	on_newline();

	PRINT_INT(lu, lo, lx, n);

	init_transient_flags();
	return (*this);
}
OStream& OStream::operator<<(unsigned long long n) {
	on_newline();

	PRINT_INT(llu, llo, llx, n);

	init_transient_flags();
	return (*this);
}
/**
 * @Cpp11 Flag %a introduced in C99, disable hex float for now
 */
OStream& OStream::operator<<(double n) {
	on_newline();

	//PRINT_FLOAT(f, e, a, n);
	PRINT_FLOAT(f, e, e, n);

	init_transient_flags();
	return (*this);
}
OStream& OStream::operator<<(const void *p) {
	on_newline();

	if (!p) return (*this) << "NULL";

	OStream OS = (*this); // new OStream for new flags
	OS << "0x" << hex << (const long int)p;
	return (*this);
}
OStream& OStream::operator<<(const char *cs) {
	on_newline();

	if (!cs) cs = "(null)";

	if (!width) {
		fputs(cs, file);
	} else {
		switch (align) {
		case Align_left:
			fprintf(file, "%-*s", (int) width, cs);
			break;
		case Align_right:
			fprintf(file, "%*s", (int) width, cs);
			break;
		}
	}

	init_transient_flags();
	return (*this);
}

OStream& OStream::operator<<(const SetWidth& s) {
	width = s.width;

	return (*this);
}
OStream& OStream::operator<<(const SetZero& s) {
	width = s.width;
	fill_zero = true;
	return (*this);
}
OStream& OStream::operator<<(const SetPrecision& s) {
	precision = s.precision;

	return (*this);
}
OStream& OStream::operator<<(const SetIndent& s) {
	indent_lvl = s.indent;

	return (*this);
}
OStream& OStream::operator<<(const SetPrefix& s) {
	prefix       = s.prefix;
	prefix_color = s.color;

	return (*this);
}

OStream& OStream::operator<<(const FillZero&) {
	fill_zero = true;

	return (*this);
}

OStream& OStream::operator<<(const Left&) {
	align = OStream::Align_left;

	return (*this);
}
OStream& OStream::operator<<(const Right&) {
	align = OStream::Align_right;

	return (*this);
}
OStream& OStream::operator<<(const Dec&) {
	int_fmt = OStream::IntFmt_decimal;

	return (*this);
}
OStream& OStream::operator<<(const Oct&) {
	int_fmt = OStream::IntFmt_octal;

	return (*this);
}
OStream& OStream::operator<<(const Hex&) {
	int_fmt = OStream::IntFmt_hexadecimal;

	return (*this);
}
OStream& OStream::operator<<(const FloatDec&) {
	float_fmt = OStream::FloatFmt_decimal;

	return (*this);
}
OStream& OStream::operator<<(const Scientific&) {
	float_fmt = OStream::FloatFmt_scientific;

	return (*this);
}
OStream& OStream::operator<<(const FloatHex&) {
	float_fmt = OStream::FloatFmt_hexadecimal;

	return (*this);
}
OStream& OStream::operator<<(const Indent&) {
	indent_lvl++;

	return (*this);
}
OStream& OStream::operator<<(const Dedent&) {
	indent_lvl--;

	return (*this);
}
OStream& OStream::operator<<(const Nl&) {
	(*this) << '\n';
	newline = true;

	return (*this);
}
OStream& OStream::operator<<(const Flush&) {
	fflush(file);

	return (*this);
}

OStream& OStream::operator<<(const ThreadId&) {
	return (*this) << "[" << hex << setw(16) << fillzero << threads_get_current_tid() << dec << "]";
}

OStream& OStream::operator<<(Color c) {
	if (!use_color) return (*this);

	switch (c) {
	case Black:       return (*this) << "\033[30m";
	case Red:         return (*this) << "\033[31m";
	case Green:       return (*this) << "\033[32m";
	case Yellow:      return (*this) << "\033[33m";
	case Blue:        return (*this) << "\033[34m";
	case Magenta:     return (*this) << "\033[35m";
	case Cyan:        return (*this) << "\033[36m";
	case White:       return (*this) << "\033[37m";
	case BoldBlack:   return (*this) << "\033[30m\033[1m";
	case BoldRed:     return (*this) << "\033[31m\033[1m";
	case BoldGreen:   return (*this) << "\033[32m\033[1m";
	case BoldYellow:  return (*this) << "\033[33m\033[1m";
	case BoldBlue:    return (*this) << "\033[34m\033[1m";
	case BoldMagenta: return (*this) << "\033[35m\033[1m";
	case BoldCyan:    return (*this) << "\033[36m\033[1m";
	case BoldWhite:   return (*this) << "\033[37m\033[1m";
	default:
		assert(false && "Unknown color code");
		break;
	}

	return (*this);
}
OStream& OStream::operator<<(const ResetColor&) {
	if (!use_color) return (*this);
	return (*this) << "\033[0m";
}
OStream& OStream::operator<<(const Bold&) {
	if (!use_color) return (*this);
	return (*this) << "\033[1m";
}
OStream& OStream::operator<<(const NoBold&) {
	if (!use_color) return (*this);
	return (*this) << "\033[21m";
}
OStream& OStream::operator<<(const Underline&) {
	if (!use_color) return (*this);
	return (*this) << "\033[4m";
}
OStream& OStream::operator<<(const NoUnderline&) {
	if (!use_color) return (*this);
	return (*this) << "\033[24m";
}

} // end namespace cacao

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
