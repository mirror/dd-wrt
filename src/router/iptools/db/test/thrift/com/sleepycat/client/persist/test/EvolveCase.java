/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */
package com.sleepycat.client.persist.test;

import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.persist.EntityStore;
import com.sleepycat.client.persist.StoreConfig;
import com.sleepycat.client.persist.evolve.Mutations;
import com.sleepycat.client.persist.model.ClassMetadata;
import com.sleepycat.client.persist.model.EntityModel;
import com.sleepycat.client.persist.model.Persistent;
import com.sleepycat.client.persist.raw.RawStore;
import com.sleepycat.client.persist.raw.RawType;

@Persistent
abstract class EvolveCase {

    static final String STORE_NAME = "foo";

    transient boolean updated;
    transient boolean newMetadataWritten;

    Mutations getMutations() {
        return null;
    }

    void configure(EntityModel model, StoreConfig config) {
    }

    String getStoreOpenException() {
        return null;
    }

    int getNRecordsExpected() {
        return 1;
    }

    void checkUnevolvedModel(EntityModel model, SEnvironment env) {
    }

    void checkEvolvedModel(EntityModel model,
                           SEnvironment env,
                           boolean oldTypesExist) {
    }

    /**
     * @throws SDatabaseException from subclasses.
     */
    void writeObjects(EntityStore store)
        throws SDatabaseException {
    }

    /**
     * @throws SDatabaseException from subclasses.
     */
    void readObjects(EntityStore store, boolean doUpdate)
        throws SDatabaseException {
    }

    /**
     * @throws SDatabaseException from subclasses.
     */
    void readRawObjects(RawStore store,
                        boolean expectEvolved,
                        boolean expectUpdated)
        throws SDatabaseException {
    }

    /**
     * @throws SDatabaseException from subclasses.
     */
    void copyRawObjects(RawStore rawStore, EntityStore newStore)
        throws SDatabaseException {
    }

    /**
     * Checks for equality and prints the entire values rather than
     * abbreviated values like TestCase.assertEquals does.
     */
    static void checkEquals(Object expected, Object got) {
        if ((expected != null) ? (!expected.equals(got)) : (got != null)) {
            TestCase.fail("Expected:\n" + expected + "\nBut got:\n" + got);
        }
    }

    /**
     * Asserts than an entity database exists or does not exist.
     */
    void assertDbExists(boolean expectExists,
                        SEnvironment env,
                        String entityClassName) {
        assertDbExists(expectExists, env, entityClassName, null);
    }

    /**
     * Checks that an entity class exists or does not exist.
     */
    void checkEntity(boolean exists,
                     EntityModel model,
                     SEnvironment env,
                     String className,
                     int version,
                     String secKeyName) {
        if (exists) {
            TestCase.assertNotNull(model.getEntityMetadata(className));
            ClassMetadata meta = model.getClassMetadata(className);
            TestCase.assertNotNull(meta);
            TestCase.assertEquals(version, meta.getVersion());
            TestCase.assertTrue(meta.isEntityClass());

            RawType raw = model.getRawType(className);
            TestCase.assertNotNull(raw);
            TestCase.assertEquals(version, raw.getVersion());

            RawType rawVersion = model.getRawTypeVersion(className, version);
            TestCase.assertNotNull(rawVersion);
            TestCase.assertTrue(!rawVersion.isDeleted());
        } else {
            TestCase.assertNull(model.getEntityMetadata(className));
            TestCase.assertNull(model.getClassMetadata(className));
            TestCase.assertNull(model.getRawType(className));

            RawType rawVersion = model.getRawTypeVersion(className, version);
            TestCase.assertTrue(rawVersion == null || rawVersion.isDeleted());
        }

        assertDbExists(exists, env, className);
        if (secKeyName != null) {
            assertDbExists(exists, env, className, secKeyName);
        }
    }

    /**
     * Checks that a non-entity class exists or does not exist.
     */
    void checkNonEntity(boolean exists,
                        EntityModel model,
                        SEnvironment env,
                        String className,
                        int version) {
        if (exists) {
            ClassMetadata meta = model.getClassMetadata(className);
            TestCase.assertNotNull(meta);
            TestCase.assertEquals(version, meta.getVersion());
            TestCase.assertTrue(!meta.isEntityClass());

            RawType raw = model.getRawType(className);
            TestCase.assertNotNull(raw);
            TestCase.assertEquals(version, raw.getVersion());

            RawType rawVersion = model.getRawTypeVersion(className, version);
            TestCase.assertNotNull(rawVersion);
            TestCase.assertTrue(!rawVersion.isDeleted());
        } else {
            TestCase.assertNull(model.getClassMetadata(className));
            TestCase.assertNull(model.getRawType(className));

            RawType rawVersion = model.getRawTypeVersion(className, version);
            TestCase.assertTrue(rawVersion == null || rawVersion.isDeleted());
        }

        TestCase.assertNull(model.getEntityMetadata(className));
        assertDbExists(false, env, className);
    }

    /**
     * Asserts than a database expectExists or does not exist. If keyName is
     * null, checks an entity database.  If keyName is non-null, checks a
     * secondary database.
     */
    void assertDbExists(boolean expectExists,
                        SEnvironment env,
                        String entityClassName,
                        String keyName) {

        /*
         * If the evolved metadata has not been written (e.g., we're in
         * read-only mode), then class evolution will not yet have created,
         * removed or renamed databases, and we cannot check their existence.
         */
        if (newMetadataWritten) {
            PersistTestUtils.assertDbExists
                (expectExists, env, STORE_NAME, entityClassName, keyName);
        }
    }

    static void checkVersions(EntityModel model, String name, int version) {
        checkVersions(model, new String[] {name}, new int[] {version});
    }

    static void checkVersions(EntityModel model,
                              String name1,
                              int version1,
                              String name2,
                              int version2) {
        checkVersions
            (model, new String[] {name1, name2},
             new int[] {version1, version2});
    }

    private static void checkVersions(EntityModel model,
                                      String[] names,
                                      int[] versions) {
        List<RawType> all = model.getAllRawTypeVersions(names[0]);
        TestCase.assertNotNull(all);

        assert names.length == versions.length;
        TestCase.assertEquals(all.toString(), names.length, all.size());

        Iterator<RawType> iter = all.iterator();
        for (int i = 0; i < names.length; i += 1) {
            RawType type = iter.next();
            TestCase.assertEquals(versions[i], type.getVersion());
            TestCase.assertEquals(names[i], type.getClassName());
        }
    }
}
