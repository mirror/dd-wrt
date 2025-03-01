/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist.test;

import static com.sleepycat.client.persist.model.Relationship.MANY_TO_ONE;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.File;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;
import com.sleepycat.client.STransaction;
import com.sleepycat.client.util.DualTestCase;
import com.sleepycat.client.persist.EntityCursor;
import com.sleepycat.client.persist.EntityStore;
import com.sleepycat.client.persist.PrimaryIndex;
import com.sleepycat.client.persist.SecondaryIndex;
import com.sleepycat.client.persist.StoreConfig;
import com.sleepycat.client.persist.model.AnnotationModel;
import com.sleepycat.client.persist.model.Entity;
import com.sleepycat.client.persist.model.EntityModel;
import com.sleepycat.client.persist.model.Persistent;
import com.sleepycat.client.persist.model.PrimaryKey;
import com.sleepycat.client.persist.model.SecondaryKey;
import com.sleepycat.client.util.test.SharedTestUtils;
import com.sleepycat.client.util.test.TestEnv;

public class SubclassIndexTest extends DualTestCase {

    private File envHome;
    private SEnvironment env;
    private EntityStore store;

    @Before
    public void setUp()
        throws Exception {

        envHome = SharedTestUtils.getTestDir();
        super.setUp();
    }

    @After
    public void tearDown()
        throws Exception {

        super.tearDown();
        envHome = null;
        env = null;
    }

    private void open()
        throws SDatabaseException {

        SEnvironmentConfig envConfig = TestEnv.TXN.getConfig();
        envConfig.setAllowCreate(true);
        env = create(envHome, envConfig);

        EntityModel model = new AnnotationModel();
        model.registerClass(Manager.class);
        model.registerClass(SalariedManager.class);

        StoreConfig storeConfig = new StoreConfig();
        storeConfig.setModel(model);
        storeConfig.setAllowCreate(true);
        store = new EntityStore(env, "foo", storeConfig);
    }

    private void close()
        throws SDatabaseException {

        store.close();
        store = null;
        close(env);
        env = null;
    }

    @Test
    public void testSubclassIndex()
        throws SDatabaseException {

        open();

        PrimaryIndex<String, Employee> employeesById =
            store.getPrimaryIndex(String.class, Employee.class);

        employeesById.put(new Employee("1"));
        employeesById.put(new Manager("2", "a"));
        employeesById.put(new Manager("3", "a"));
        employeesById.put(new Manager("4", "b"));

        Employee e;
        Manager m;

        e = employeesById.get("1");
        assertNotNull(e);
        assertTrue(!(e instanceof Manager));

        /* Ensure DB exists BEFORE calling getSubclassIndex. [#15247] */
        PersistTestUtils.assertDbExists
            (true, env, "foo", Employee.class.getName(), "dept");

        /* Normal use: Subclass index for a key in the subclass. */
        SecondaryIndex<String, String, Manager> managersByDept =
            store.getSubclassIndex
                (employeesById, Manager.class, String.class, "dept");

        m = managersByDept.get("a");
        assertNotNull(m);
        assertEquals("2", m.id);

        m = managersByDept.get("b");
        assertNotNull(m);
        assertEquals("4", m.id);

        STransaction txn = env.beginTransaction(null, null);
        EntityCursor<Manager> managers = managersByDept.entities(txn, null);
        try {
            m = managers.next();
            assertNotNull(m);
            assertEquals("2", m.id);
            m = managers.next();
            assertNotNull(m);
            assertEquals("3", m.id);
            m = managers.next();
            assertNotNull(m);
            assertEquals("4", m.id);
            m = managers.next();
            assertNull(m);
        } finally {
            managers.close();
            txn.commit();
        }

        /* Getting a subclass index for the entity class is also allowed. */
        store.getSubclassIndex
            (employeesById, Employee.class, String.class, "other");

        /* Getting a subclass index for a base class key is not allowed. */
        try {
            store.getSubclassIndex
                (employeesById, Manager.class, String.class, "other");
            fail();
        } catch (IllegalArgumentException expected) {
        }

        close();
    }

    /**
     * Previously this tested that a secondary key database was added only
     * AFTER storing the first instance of the subclass that defines the key.
     * Now that we require registering the subclass up front, the database is
     * created up front also.  So this test is somewhat less useful, but still
     * nice to have around.  [#16399]
     */
    @Test
    public void testAddSecKey()
        throws SDatabaseException {

        open();
        PrimaryIndex<String, Employee> employeesById =
            store.getPrimaryIndex(String.class, Employee.class);
        employeesById.put(new Employee("1"));
        assertTrue(hasEntityKey("dept"));
        close();

        open();
        employeesById = store.getPrimaryIndex(String.class, Employee.class);
        assertTrue(hasEntityKey("dept"));
        employeesById.put(new Manager("2", "a"));
        assertTrue(hasEntityKey("dept"));
        close();

        open();
        assertTrue(hasEntityKey("dept"));
        close();
        
        open();
        employeesById = store.getPrimaryIndex(String.class, Employee.class);
        assertTrue(hasEntityKey("salary"));
        employeesById.put(new SalariedManager("3", "a", "111"));
        assertTrue(hasEntityKey("salary"));
        close();

        open();
        assertTrue(hasEntityKey("dept"));
        assertTrue(hasEntityKey("salary"));
        close();
    }

    private boolean hasEntityKey(String keyName) {
        return store.getModel().
               getRawType(Employee.class.getName()).
               getEntityMetadata().
               getSecondaryKeys().
               keySet().
               contains(keyName);
    }

    @Entity
    private static class Employee {

        @PrimaryKey
        String id;

        @SecondaryKey(relate=MANY_TO_ONE)
        String other;

        Employee(String id) {
            this.id = id;
        }

        private Employee() {}
    }

    @Persistent
    private static class Manager extends Employee {

        @SecondaryKey(relate=MANY_TO_ONE)
        String dept;

        Manager(String id, String dept) {
            super(id);
            this.dept = dept;
        }

        private Manager() {}
    }

    @Persistent
    private static class SalariedManager extends Manager {

        @SecondaryKey(relate=MANY_TO_ONE)
        String salary;

        SalariedManager(String id, String dept, String salary) {
            super(id, dept);
            this.salary = salary;
        }

        private SalariedManager() {}
    }
}
