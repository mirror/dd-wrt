/* tests/gtest/future_algorithm.cpp - test future algorithm library features

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

#include "future/algorithm.hpp"
#include <vector>

namespace {

template<class T>
struct isTrue {
	bool operator()(const T& x) {
		return bool(x);
	}
};

} // end anonymous namespace

// test fixture
class future : public ::testing::Test {
public:
	std::vector<int> all;
	std::vector<int> none;
	std::vector<int> any;
	// ctor
	future() : all(10,1) , none(10,0), any(10,1) {
		any[3] = 0;
	}
};

TEST_F(future, all_of) {
	EXPECT_TRUE(cacao::all_of(all.begin(),all.end(),isTrue<int>()));
	EXPECT_FALSE(cacao::none_of(all.begin(),all.end(),isTrue<int>()));
	EXPECT_TRUE(cacao::any_of(all.begin(),all.end(),isTrue<int>()));
}

TEST_F(future, none_of) {
	EXPECT_FALSE(cacao::all_of(none.begin(),none.end(),isTrue<int>()));
	EXPECT_TRUE(cacao::none_of(none.begin(),none.end(),isTrue<int>()));
	EXPECT_FALSE(cacao::any_of(none.begin(),none.end(),isTrue<int>()));
}

TEST_F(future, any_of) {
	EXPECT_FALSE(cacao::all_of(any.begin(),any.end(),isTrue<int>()));
	EXPECT_FALSE(cacao::none_of(any.begin(),any.end(),isTrue<int>()));
	EXPECT_TRUE(cacao::any_of(any.begin(),any.end(),isTrue<int>()));
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
