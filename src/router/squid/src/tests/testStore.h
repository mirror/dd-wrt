/*
 * Copyright (C) 1996-2024 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef SQUID_SRC_TESTS_TESTSTORE_H
#define SQUID_SRC_TESTS_TESTSTORE_H

#include "Store.h"
#include "store/Controlled.h"

#include "compat/cppunit.h"

/*
 * test the store framework
 */

class testStore : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( testStore );
    CPPUNIT_TEST( testSetRoot );
    CPPUNIT_TEST( testUnsetRoot );
    CPPUNIT_TEST( testStats );
    CPPUNIT_TEST( testMaxSize );
    CPPUNIT_TEST( testSwapMetaTypeClassification );
    CPPUNIT_TEST_SUITE_END();

public:

protected:
    void testSetRoot();
    void testUnsetRoot();
    void testStats();
    void testMaxSize();
    void testSwapMetaTypeClassification();
};

/// allows testing of methods without having all the other components live
class TestStore : public Store::Controller
{

public:
    TestStore() : statsCalled (false) {}

    bool statsCalled;

    int callback() override;

    virtual StoreEntry* get(const cache_key*);

    virtual void get(String, void (*)(StoreEntry*, void*), void*);

    void init() override;

    void maintain() override {};

    uint64_t maxSize() const override;

    uint64_t minSize() const override;

    uint64_t currentSize() const override;

    uint64_t currentCount() const override;

    int64_t maxObjectSize() const override;

    void getStats(StoreInfoStats &) const override;

    void stat(StoreEntry &) const override; /* output stats to the provided store entry */

    virtual void reference(StoreEntry &) {} /* Reference this object */

    virtual bool dereference(StoreEntry &) { return true; }

    virtual StoreSearch *search();
};

typedef RefCount<TestStore> TestStorePointer;

#endif /* SQUID_SRC_TESTS_TESTSTORE_H */

