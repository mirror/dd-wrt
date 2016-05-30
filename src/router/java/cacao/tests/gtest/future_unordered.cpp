/* tests/gtest/future_unordered.cpp - test unordered_{map,set/multimap/multiset}

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

#include "future/unordered_set.hpp"
#include "future/unordered_map.hpp"


class TestClass {
public:
	int x;
	explicit TestClass(int x) : x(x) {}
};

inline bool operator==(const TestClass &lhs, const TestClass &rhs) {
	return lhs.x == rhs.x;
}

namespace cacao {

}

namespace cacao {

template<>
struct hash<TestClass> {
	std::size_t operator()(TestClass const& s) const {
		return std::size_t(s.x);
	}
};

TEST(set,Int) {
	unordered_set<int> uset;
	uset.insert(1);
	uset.insert(2);
	uset.insert(3);
	uset.insert(4);
	uset.insert(5);

	EXPECT_TRUE(uset.find(1) != uset.end());
	EXPECT_TRUE(uset.find(2) != uset.end());
	EXPECT_TRUE(uset.find(3) != uset.end());
	EXPECT_TRUE(uset.find(4) != uset.end());
	EXPECT_TRUE(uset.find(5) != uset.end());
	EXPECT_TRUE(uset.find(6) == uset.end());
}


TEST(set,Class) {
	unordered_set<TestClass> uset;
	uset.insert(TestClass(1));
	uset.insert(TestClass(2));
	uset.insert(TestClass(3));
	uset.insert(TestClass(4));
	uset.insert(TestClass(5));

	unordered_set<TestClass> uset2(uset);
	unordered_set<TestClass> uset3 = uset;

	EXPECT_TRUE(uset.find(TestClass(1)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(2)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(3)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(4)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(5)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(6)) == uset.end());

	EXPECT_TRUE(uset2.find(TestClass(1)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(2)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(3)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(4)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(5)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(6)) == uset2.end());

	EXPECT_TRUE(uset3.find(TestClass(1)) != uset3.end());
	EXPECT_TRUE(uset3.find(TestClass(2)) != uset3.end());
	EXPECT_TRUE(uset3.find(TestClass(3)) != uset3.end());
	EXPECT_TRUE(uset3.find(TestClass(4)) != uset3.end());
	EXPECT_TRUE(uset3.find(TestClass(5)) != uset3.end());
	EXPECT_TRUE(uset3.find(TestClass(6)) == uset3.end());
}

TEST(set, Swap) {
	unordered_set<TestClass> uset;
	uset.insert(TestClass(1));
	uset.insert(TestClass(2));
	uset.insert(TestClass(3));

	unordered_set<TestClass> uset2;
	uset2.insert(TestClass(-1));
	uset2.insert(TestClass(-2));
	uset2.insert(TestClass(-3));


	EXPECT_TRUE(uset.find(TestClass(1)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(2)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(3)) != uset.end());

	EXPECT_FALSE(uset.find(TestClass(-1)) != uset.end());
	EXPECT_FALSE(uset.find(TestClass(-2)) != uset.end());
	EXPECT_FALSE(uset.find(TestClass(-3)) != uset.end());

	EXPECT_FALSE(uset2.find(TestClass(1)) != uset2.end());
	EXPECT_FALSE(uset2.find(TestClass(2)) != uset2.end());
	EXPECT_FALSE(uset2.find(TestClass(3)) != uset2.end());

	EXPECT_TRUE(uset2.find(TestClass(-1)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(-2)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(-3)) != uset2.end());

	uset.swap(uset2);

	EXPECT_TRUE(uset2.find(TestClass(1)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(2)) != uset2.end());
	EXPECT_TRUE(uset2.find(TestClass(3)) != uset2.end());

	EXPECT_FALSE(uset2.find(TestClass(-1)) != uset2.end());
	EXPECT_FALSE(uset2.find(TestClass(-2)) != uset2.end());
	EXPECT_FALSE(uset2.find(TestClass(-3)) != uset2.end());

	EXPECT_FALSE(uset.find(TestClass(1)) != uset.end());
	EXPECT_FALSE(uset.find(TestClass(2)) != uset.end());
	EXPECT_FALSE(uset.find(TestClass(3)) != uset.end());

	EXPECT_TRUE(uset.find(TestClass(-1)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(-2)) != uset.end());
	EXPECT_TRUE(uset.find(TestClass(-3)) != uset.end());

}


TEST(map, Int) {
	unordered_map<int,int> umap;
	umap[0] = 0;
	umap[-1] = -1;
	umap[1] = 1;

	EXPECT_EQ(0,umap[0]);
	EXPECT_EQ(-1,umap[-1]);
	EXPECT_EQ(1,umap[1]);
}

TEST(map, Class) {
	unordered_map<TestClass,int> umap;
	umap[TestClass(0)] = 0;
	umap[TestClass(-1)] = -1;
	umap[TestClass(1)] = 1;

	EXPECT_EQ(0,umap[TestClass(0)]);
	EXPECT_EQ(-1,umap[TestClass(-1)]);
	EXPECT_EQ(1,umap[TestClass(1)]);
}


TEST(multiset,Int) {
	unordered_multiset<int> uset;
	uset.insert(1);
	uset.insert(2);
	uset.insert(3);
	uset.insert(4);
	uset.insert(5);
	uset.insert(1);
	uset.insert(2);
	uset.insert(3);
	uset.insert(4);
	uset.insert(5);

	EXPECT_EQ(10,uset.size());

	EXPECT_TRUE(uset.find(1) != uset.end());
	EXPECT_TRUE(uset.find(2) != uset.end());
	EXPECT_TRUE(uset.find(3) != uset.end());
	EXPECT_TRUE(uset.find(4) != uset.end());
	EXPECT_TRUE(uset.find(5) != uset.end());
	EXPECT_TRUE(uset.find(6) == uset.end());
}
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
