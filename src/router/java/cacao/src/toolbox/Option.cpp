/* src/toolbox/Option.cpp - Command line option parsing library

   Copyright (C) 1996-2014
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

#include "toolbox/Option.hpp"
#include "toolbox/Debug.hpp"
#include "toolbox/logging.hpp"
#include "toolbox/OStream.hpp"

namespace cacao {

namespace option {

OptionPrefix& root(){
	static OptionPrefix prefix("-");
	return prefix;
}

OptionPrefix& xx_root(){
	static OptionPrefix prefix("-XX:");
	return prefix;
}

} // end namespace option

OptionPrefix::OptionPrefix(const char* name) : name(name), s(std::strlen(name)) {
}
void OptionPrefix::insert(OptionEntry* oe) {
	children.insert(oe);
}

void OptionParser::print_usage(OptionPrefix& root, FILE *fp) {
	static const char* blank29 = "                             "; // 29 spaces
	static const char* blank25 = blank29 + 4;                     // 25 spaces
	OStream OS(fp);

	std::set<OptionEntry*> sorted(root.begin(),root.end());

	for(std::set<OptionEntry*>::iterator i = sorted.begin(), e = sorted.end();
			i != e; ++i) {
		OptionEntry& oe = **i;
		OS << "    " << root.get_name();
		std::size_t name_len = root.size() + oe.print(OS);
		if (name_len < (25-1)) {
			OS << (blank25 + name_len);
		} else {
			OS << nl << blank29;
		}

		const char* c = oe.get_desc();
		for (std::size_t i = 29; *c != 0; c++, i++) {
			/* If we are at the end of the line, break it. */
			if (i == 80) {
				OS << nl << blank29;
				i = 29;
			}
			OS << *c;
		}
		OS << nl;
	}

}

std::size_t option_print(OptionEntry& option, OStream& OS) {
	OS << option.get_name() << "=<value>";
	return option.size() + 8;
}

std::size_t option_print(OptionBase<bool>& option, OStream& OS) {
	OS << '+' << option.get_name();
	return option.size() + 1;
}


namespace {

bool option_matcher(const char* a, std::size_t a_len,
		const char* b, std::size_t b_len) {
	return (a_len == b_len) && std::strncmp(a, b, a_len) == 0;
}

} // end anonymous namespace

bool OptionParser::parse_option(OptionPrefix& root, const char* name, size_t name_len,
		const char* value, size_t value_len) {
	assert(std::strncmp(root.get_name(), name, std::strlen(root.get_name())) == 0);
	name += root.size();
	name_len -= root.size();
	assert(name_len > 0);
	if (name[0] == '-' || name[0] == '+') {
		assert(value_len == 0);
		assert(value == 0);
		value = name;
		value_len = 1;
		++name;
		--name_len;
	}
	for(OptionPrefix::iterator i = root.begin(), e = root.end();
			i != e; ++i) {
		OptionEntry& oe = **i;
		if (option_matcher(oe.get_name(), oe.size(), name, name_len)) {
			if (oe.parse(value,value_len))
				return true;
			return false;
		}
	}
	return false;
}

template<>
bool Option<const char*>::parse(const char* value, std::size_t value_len) {
	set_value(value);
	return true;
}

template<>
bool Option<bool>::parse(const char* value, std::size_t value_len) {
	assert(value_len == 1);
	char first = value[0];
	if (first == '-') {
		set_value(false);
		return true;
	}
	if (first == '+') {
		set_value(true);
		return true;
	}
	ABORT_MSG("ERROR", "boolean option not valid: " << value);
	return false;
}

template<>
bool Option<unsigned int>::parse(const char* value, std::size_t value_len) {
	int verb = os::atoi(value);
	set_value(verb >= 0 ? verb : 0);
	return true;
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
