/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist.test;

import static org.junit.Assert.assertEquals;

import java.io.File;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;
import com.sleepycat.client.util.DualTestCase;
import com.sleepycat.client.persist.EntityStore;
import com.sleepycat.client.persist.PrimaryIndex;
import com.sleepycat.client.persist.StoreConfig;
import com.sleepycat.client.persist.model.Entity;
import com.sleepycat.client.persist.model.KeyField;
import com.sleepycat.client.persist.model.Persistent;
import com.sleepycat.client.persist.model.PrimaryKey;
import com.sleepycat.client.util.test.SharedTestUtils;
import com.sleepycat.client.util.test.TestEnv;

/**
 * @author Mark Hayes
 */
public class SequenceTest extends DualTestCase {

    private File envHome;
    private SEnvironment env;

    @Before
    public void setUp()
        throws Exception {

        super.setUp();
        envHome = SharedTestUtils.getTestDir();
    }

    @After
    public void tearDown()
        throws Exception {

        super.tearDown();
        envHome = null;
        env = null;
    }

    @Test
    public void testSequenceKeys()
        throws Exception {

        Class[] classes = {
            SequenceEntity_Long.class,
            SequenceEntity_Integer.class,
            SequenceEntity_Short.class,
            SequenceEntity_Byte.class,
            SequenceEntity_tlong.class,
            SequenceEntity_tint.class,
            SequenceEntity_tshort.class,
            SequenceEntity_tbyte.class,
        };

        SEnvironmentConfig envConfig = TestEnv.TXN.getConfig();
        envConfig.setAllowCreate(true);
        env = create(envHome, envConfig);

        StoreConfig storeConfig = new StoreConfig();
        storeConfig.setAllowCreate(true);
        EntityStore store = new EntityStore(env, "foo", storeConfig);

        long seq = 0;

        for (int i = 0; i < classes.length; i += 1) {
            Class entityCls = classes[i];
            SequenceEntity entity = (SequenceEntity) entityCls.newInstance();
            Class keyCls = entity.getKeyClass();

            PrimaryIndex<Object, SequenceEntity> index =
                store.getPrimaryIndex(keyCls, entityCls);
            index.putNoReturn(entity);
            seq += 1;
            assertEquals(seq, entity.getKey());

            index.putNoReturn(entity);
            assertEquals(seq, entity.getKey());

            entity.nullifyKey();
            index.putNoReturn(entity);
            seq += 1;
            assertEquals(seq, entity.getKey());
        }

        store.close();
        close(env);
        env = null;
    }

    interface SequenceEntity {
        Class getKeyClass();
        long getKey();
        void nullifyKey();
    }

    @Entity
    static class SequenceEntity_Long implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Long priKey;

        public Class getKeyClass() {
            return Long.class;
        }

        public long getKey() {
            return priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_Integer implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Integer priKey;

        public Class getKeyClass() {
            return Integer.class;
        }

        public long getKey() {
            return priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_Short implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Short priKey;

        public Class getKeyClass() {
            return Short.class;
        }

        public long getKey() {
            return priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_Byte implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Byte priKey;

        public Class getKeyClass() {
            return Byte.class;
        }

        public long getKey() {
            return priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_tlong implements SequenceEntity {

        @PrimaryKey(sequence="X")
        long priKey;

        public Class getKeyClass() {
            return Long.class;
        }

        public long getKey() {
            return priKey;
        }

        public void nullifyKey() {
            priKey = 0;
        }
    }

    @Entity
    static class SequenceEntity_tint implements SequenceEntity {

        @PrimaryKey(sequence="X")
        int priKey;

        public Class getKeyClass() {
            return Integer.class;
        }

        public long getKey() {
            return priKey;
        }

        public void nullifyKey() {
            priKey = 0;
        }
    }

    @Entity
    static class SequenceEntity_tshort implements SequenceEntity {

        @PrimaryKey(sequence="X")
        short priKey;

        public Class getKeyClass() {
            return Short.class;
        }

        public long getKey() {
            return priKey;
        }

        public void nullifyKey() {
            priKey = 0;
        }
    }

    @Entity
    static class SequenceEntity_tbyte implements SequenceEntity {

        @PrimaryKey(sequence="X")
        byte priKey;

        public Class getKeyClass() {
            return Byte.class;
        }

        public long getKey() {
            return priKey;
        }

        public void nullifyKey() {
            priKey = 0;
        }
    }

    @Entity
    static class SequenceEntity_Long_composite implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Key priKey;

        @Persistent
        static class Key {
            @KeyField(1)
            Long priKey;
        }

        public Class getKeyClass() {
            return Key.class;
        }

        public long getKey() {
            return priKey.priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_Integer_composite implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Key priKey;

        @Persistent
        static class Key {
            @KeyField(1)
            Integer priKey;
        }

        public Class getKeyClass() {
            return Key.class;
        }

        public long getKey() {
            return priKey.priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_Short_composite implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Key priKey;

        @Persistent
        static class Key {
            @KeyField(1)
            Short priKey;
        }

        public Class getKeyClass() {
            return Key.class;
        }

        public long getKey() {
            return priKey.priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_Byte_composite implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Key priKey;

        @Persistent
        static class Key {
            @KeyField(1)
            Byte priKey;
        }

        public Class getKeyClass() {
            return Key.class;
        }

        public long getKey() {
            return priKey.priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_tlong_composite implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Key priKey;

        @Persistent
        static class Key {
            @KeyField(1)
            long priKey;
        }

        public Class getKeyClass() {
            return Key.class;
        }

        public long getKey() {
            return priKey.priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_tint_composite implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Key priKey;

        @Persistent
        static class Key {
            @KeyField(1)
            int priKey;
        }

        public Class getKeyClass() {
            return Key.class;
        }

        public long getKey() {
            return priKey.priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_tshort_composite implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Key priKey;

        @Persistent
        static class Key {
            @KeyField(1)
            short priKey;
        }

        public Class getKeyClass() {
            return Key.class;
        }

        public long getKey() {
            return priKey.priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }

    @Entity
    static class SequenceEntity_tbyte_composite implements SequenceEntity {

        @PrimaryKey(sequence="X")
        Key priKey;

        @Persistent
        static class Key {
            @KeyField(1)
            byte priKey;
        }

        public Class getKeyClass() {
            return Key.class;
        }

        public long getKey() {
            return priKey.priKey;
        }

        public void nullifyKey() {
            priKey = null;
        }
    }
}
