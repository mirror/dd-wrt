/* tests/gtest/logging_test.cpp - logging tests

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

#include "gtest/gtest.h"
#include "toolbox/logging.hpp"

namespace {
class FileRes {
private:
	FILE *f;
public:
	FileRes(const char* filename, const char* mode) {
		f = fopen(filename,mode);
	}
	operator FILE*() {
		return f;
	}
	~FileRes() {
		fclose(f);
	}
};
}

TEST(WARNING_MSG, test) {
	FileRes f("/dev/null", "w");
	cacao::err().set_file(f);
	EXPECT_NO_THROW(WARNING_MSG("test","test"));
}
TEST(ERROR_MSG, test) {
	FileRes f("/dev/null", "w");
	cacao::err().set_file(f);
	EXPECT_NO_THROW(ERROR_MSG("test","test"));
}
TEST(ABORT_MSG_DeathTest, test) {
	::testing::FLAGS_gtest_death_test_style = "threadsafe";
	cacao::err().set_file(stderr);
	EXPECT_EXIT(ABORT_MSG("test","test"), ::testing::KilledBySignal(SIGABRT),".*test.*");
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
