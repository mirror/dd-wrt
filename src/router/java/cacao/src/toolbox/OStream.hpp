/* src/toolbox/OStream.hpp - simple output stream

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

#ifndef OSTREAM_HPP_
#define OSTREAM_HPP_ 1

#include <cstdio>

namespace cacao {

class OStream;

OStream& out();
OStream& err();

class SetWidth;
class SetZero;
class SetPrecision;
class SetIndent;
class SetPrefix;

class FillZero;

class Left;
class Right;
class Dec;
class Oct;
class Hex;
class FloatDec;
class Scientific;
class FloatHex;
class Indent;
class Dedent;
class Nl;
class Flush;

class ThreadId;

// ANSI terminal control

enum Color {
	InvalidColor = 0,
	Black,
	Red,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	White,
	BoldBlack,
	BoldRed,
	BoldGreen,
	BoldYellow,
	BoldBlue,
	BoldMagenta,
	BoldCyan,
	BoldWhite
};

class ResetColor  {};
class Bold        {};
class NoBold      {};
class Underline   {};
class NoUnderline {};

/** Simple stream class for formatted output
 *
 * This class is designed for debugging, thus usability trumps performance.
 * It mostly mimics the iostreams library, but requires no global constructors.
 * Interally everything is forwarded to stdio
 *
 * A stream can contain a prefix or intentions.
 * The stream does not detect if your output contains a '\n' (newline) character.
 * You must use the manipulator #nl instead. It works like std::endl but
 * does not flush the stream.
 *
 * Simple examples :
 *   @code
 *      OStream os(stdout);
 *
 *      os << "Hi there, my name is " << bold << "cacao" << nobold             << nl;
 *      os << "I was born in " << 1996                                         << nl;
 *      os << "Test failures are " << underline << red << "BAD" << reset_color << nl;
 *      os                                                                     << nl;
 *      os << "Do you like hex? "       << hex       << 255  << dec            << nl;
 *      os << "Or floating point hex? " << float_hex << 17.3 << float_dec      << nl;
 *   @endcode
 *
 * Unlike a std::iostream you can copy construct an OStream.
 * A copied OStream will write to the same file but has it's own set of
 * format flags that you can set independent of the original stream.
 * But Colors, bold and underline are shared by all streams for a file,
 * because they are stored in the underlying terminal.
 *  You should not write nl to the copied stream since the original will not
 * detect the newline.
 *
 * Example:
 *  @code
 *      struct MyLittlePony {
 *      	const char *name;
 *         Color       color;
 *      };
 *
 *      OStream& operator<<(OStream& os, const MyLittlePony& mlp) {
 *      	OStream os2 = os; // new stream with new flags
 *
 *      	os2 << mlp.color;
 *      	os2 << "My little pony is called " << setw(20) << right << mlp.name;
 *      	os2 << hex;
 *
 *      	// Forgot to unset hex for os2: no problem, hex flag is not shared
 *      	// Forgot to unset color: big problem, colors are shared!
 *
 *      	return os; // always return original stream
 *      }
 *  @endcode
 */
class OStream {
public:
	/// create a new stream with default flags
	OStream(FILE *file);

	/** copy stream
	 *
	 * creates a new stream with the same file
	 * but default
	 */
	OStream(const OStream&);

	OStream& operator<<(char);
	OStream& operator<<(bool);
	OStream& operator<<(long);
	OStream& operator<<(unsigned long);
	OStream& operator<<(long long );
	OStream& operator<<(unsigned long long);
	OStream& operator<<(double);
	OStream& operator<<(const void*);
	OStream& operator<<(const char*);

	OStream& operator<<(unsigned int n) {
		return this->operator<<(static_cast<unsigned long>(n));
	}

	OStream& operator<<(int n) {
		return this->operator<<(static_cast<long>(n));
	}

	// manipulators
	OStream& operator<<(const SetWidth&);
	OStream& operator<<(const SetZero&);
	OStream& operator<<(const SetPrecision&);
	OStream& operator<<(const SetIndent&);
	OStream& operator<<(const SetPrefix&);

	OStream& operator<<(const FillZero&);

	OStream& operator<<(const Left&);
	OStream& operator<<(const Right&);

	OStream& operator<<(const Dec&);
	OStream& operator<<(const Oct&);
	OStream& operator<<(const Hex&);

	OStream& operator<<(const FloatDec&);
	OStream& operator<<(const Scientific&);
	OStream& operator<<(const FloatHex&);

	OStream& operator<<(const Indent&);
	OStream& operator<<(const Dedent&);

	OStream& operator<<(const Nl&);
	OStream& operator<<(const Flush&);

	OStream& operator<<(const ThreadId&);

	// ANSI codes
	OStream& operator<<(Color);

	OStream& operator<<(const ResetColor&);

	OStream& operator<<(const Bold&);
	OStream& operator<<(const NoBold&);
	OStream& operator<<(const Underline&);
	OStream& operator<<(const NoUnderline&);

	void set_file(FILE *file) { this->file = file; }
private:
	void on_newline();

	/// initialize all format flags to their default value
	void init_flag_defaults();

	/// initialize all flags that only apply to one write operation
	void init_transient_flags();
	/// initialize all flags that survive a write operation
	void init_persistent_flags();

	/// file stream writes to
	FILE  *file;

	/// true iff we are at the beginning of a new line
	bool newline;

	/// supports ansi escape codes
	bool use_color;

	enum IntegerFormat {
		IntFmt_decimal,
		IntFmt_octal,
		IntFmt_hexadecimal
	};
	enum FloatFormat {
		FloatFmt_decimal,
		FloatFmt_scientific,
		FloatFmt_hexadecimal
	};

	// ********** format flags

	/** padding for next write
	 *
	 * ! width is reset to zero by all standard write operations !
	 *
	 * default value is 0
	 */
	size_t width;

	/** precision
	 *
	 * ! precision is reset to -1 by all standard write operations !
	 *
	 * default value is -1 (i.e. turned off)
	 */
	int precision;

	/** fill_zero
	 *
	 * ! fill_zero is reset to false by all standard write operations !
	 *
	 * default value is false
	 */
	bool fill_zero;

	/** Alignment to use when padding text
	 *
	 * default value is OStream::Align_right
	 */
	enum {
		Align_left,
		Align_right
	} align;

	/** format used to print integer types
	 *
	 * default value is \link OStream::IntegerFormat::IntFmt_decimal decimal \endlink
	 */
	IntegerFormat int_fmt;

	/** format used to print floating point types
	 *
	 * default value is \link OStream::FloatFormat::FloatFmt_decimal decimal \endlink
	 */
	FloatFormat float_fmt;

	/** indentation level
	 *
	 * every new line will start with OStream::Flags::indent_lvl * 4 spaces
	 *
	 * default value is 0
	 */
	size_t indent_lvl;

	/** line prefix
	 *
	 * ignored if NULL
	 * will be printed at start of every new line
	 *
	 * default value is NULL
	 */
	const char *prefix;

	/** color line prefix is printed in
	 *
	 * ignored if negative
	 *
	 * default value is -1
	 */
	Color prefix_color;

	friend class Logging;
};

/// Set width flag for next item to be written.
class SetWidth {
	size_t width;
public:
	SetWidth(size_t width) : width(width) {}

friend class OStream;
};

/// Set width flag and fill zero for next item to be written.
class SetZero {
	size_t width;
public:
	SetZero(size_t width) : width(width) {}

friend class OStream;
};

/// Set precision flag for next item to be written.
class SetPrecision {
	int precision;
public:
	SetPrecision(int precision) : precision(precision) {}

friend class OStream;
};

/// Set indent level in stream
class SetIndent {
	size_t indent;
public:
	SetIndent(size_t indent) : indent(indent) {}

friend class OStream;
};

/// Set stream prefix
class SetPrefix {
	const char *prefix;
	Color       color;
public:
	SetPrefix(const char *prefix, Color color)
	 : prefix(prefix), color(color) {}

friend class OStream;
};

class FillZero   {};

class Left       {};
class Right      {};
class Dec        {};
class Oct        {};
class Hex        {};
class FloatDec   {};
class Scientific {};
class FloatHex   {};
class Indent     {};
class Dedent     {};
class Nl         {};
class Flush      {};

class ThreadId {};

class ResetColor;
class Bold;
class NoBold;
class Underline;
class NoUnderline;

inline static SetWidth setw(size_t w) {
	return SetWidth(w);
}
inline static SetZero setz(size_t w) {
	return SetZero(w);
}
inline static SetPrecision setprecision(int p) {
	return SetPrecision(p);
}
inline static SetIndent setindent(size_t i) {
	return SetIndent(i);
}
inline static SetPrefix setprefix(const char *prefix, Color color) {
	return SetPrefix(prefix, color);
}

extern FillZero   fillzero;
extern Left       left;
extern Right      right;
extern Dec        dec;
extern Oct        oct;
extern Hex        hex;
extern FloatDec   float_dec;
extern Scientific scientific;
extern FloatHex   float_hex;
extern Indent     indent;
extern Dedent     dedent;
extern Nl         nl;
extern Flush      flush;

// pipe this into a stream to print the current thread's id
extern ThreadId threadid;

extern ResetColor  reset_color;
extern Bold        bold;
extern NoBold      nobold;
extern Underline   underline;
extern NoUnderline nounderline;

// helper templates
template <class _ForwardIterator>
inline OStream& print_container(OStream &OS, _ForwardIterator i, const _ForwardIterator &e) {
	if (i == e)
		return OS << "[<empty>]";
	OS << "[" << *i;
	++i;
	for( ; i != e ; ++i) {
		OS << ", " << *i;
	}
	return OS << "]";
}

} // end namespace cacao

#endif // OSTREAM_HPP_


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
