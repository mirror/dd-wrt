/*
 * Copyright (C) 1996-2024 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef SQUID_SRC_TESTS_TESTURL_H
#define SQUID_SRC_TESTS_TESTURL_H

#include "compat/cppunit.h"

/*
 * test the URL class.
 */

class testURL : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( testURL );
    CPPUNIT_TEST( testConstructScheme );
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

protected:

    void testConstructScheme();
    void testDefaultConstructor();
};

#endif /* SQUID_SRC_TESTS_TESTURL_H */

