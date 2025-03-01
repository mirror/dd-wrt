/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import org.junit.Before;
import org.junit.Test;

import java.util.Arrays;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.CoreMatchers.not;
import static org.hamcrest.MatcherAssert.assertThat;

public class HandleManagerTest {

    private HandleManager manager;

    @Before
    public void setUp() throws Exception {
        manager = new HandleManager();
    }

    @Test
    public void testRegister() throws Exception {
        HandleDescriptor m1 = manager.register(new MockDescriptor("m1"));
        HandleDescriptor m2 = manager.register(new MockDescriptor("m2"));

        assertThat(m1.getId(), not(is(m2.getId())));

        HandleDescriptor h1 = manager.readLockHandle(m1.getId());
        assertThat(h1, is(m1));

        manager.unlockHandle(h1);
    }

    @Test
    public void testCloseHandle() throws Exception {
        HandleDescriptor m1 = manager.register(new MockDescriptor("m1"));
        manager.closeHandle(m1.getId());
        assertThat(m1.isClosed(), is(true));
    }

    @Test(expected = IllegalArgumentException.class)
    public void testCloseHandleWithInvalidId() throws Exception {
        manager.closeHandle(0);
    }

    @Test(expected = IllegalArgumentException.class)
    public void testCloseHandleWithDoubleClose() throws Exception {
        HandleDescriptor m1 = manager.register(new MockDescriptor("m1"));
        manager.closeHandle(m1.getId());
        manager.closeHandle(m1.getId());
    }

    @Test
    public void testCloseHandles() throws Exception {
        HandleDescriptor m1 = manager.register(new MockDescriptor("m1"));
        HandleDescriptor m2 = manager.register(new MockDescriptor("m2"));
        HandleDescriptor m3 = manager.register(new MockDescriptor("m3"));

        Exception e = manager.closeHandles(Arrays.asList(m1, m2));
        if (e != null) throw e;

        assertThat(m1.isClosed(), is(true));
        assertThat(m2.isClosed(), is(true));
        assertThat(m3.isClosed(), is(false));

        e = manager.closeHandles(Arrays.asList(m2, m3));
        if (e != null) throw e;

        assertThat(m2.isClosed(), is(true));
        assertThat(m3.isClosed(), is(true));
    }

    @Test
    public void testCloseHandlesWithDoubleClose() throws Exception {
        HandleDescriptor m1 = manager.register(new MockDescriptor("m1"));
        HandleDescriptor m2 = manager.register(new MockDescriptor("m2"));

        manager.closeHandle(m1.getId());

        Exception e = manager.closeHandles(Arrays.asList(m1, m2));
        if (e != null) throw e;

        assertThat(m1.isClosed(), is(true));
        assertThat(m2.isClosed(), is(true));
    }

    @Test
    public void testReadLockHandle() throws Exception {
        HandleDescriptor m1 = manager.register(new MockDescriptor("m1"));
        HandleDescriptor h1 = manager.readLockHandle(m1.getId());

        assertThat(h1, is(m1));
        assertThat(h1.getRwLock().getReadHoldCount(), is(1));

        manager.unlockHandle(h1);
    }

    @Test(expected = IllegalArgumentException.class)
    public void testReadLockHandleWithInvalidId() throws Exception {
        manager.readLockHandle(0);
    }

    @Test(expected = IllegalArgumentException.class)
    public void testReadLockHandleWithClosedId() throws Exception {
        HandleDescriptor m1 = manager.register(new MockDescriptor("m1"));
        HandleDescriptor h1 = manager.readLockHandle(m1.getId());
        manager.unlockHandle(h1);

        manager.closeHandle(m1.getId());
        manager.readLockHandle(m1.getId());
    }

    @Test
    public void testShutdown() throws Exception {
        HandleDescriptor m1 = manager.register(new MockDescriptor("m1"));
        HandleDescriptor m2 = manager.register(new MockDescriptor("m2"));

        manager.shutdown();

        assertThat(m1.isClosed(), is(true));
        assertThat(m2.isClosed(), is(true));
    }
}