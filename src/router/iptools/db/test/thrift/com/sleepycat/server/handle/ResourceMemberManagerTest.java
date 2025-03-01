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

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.CoreMatchers.notNullValue;
import static org.hamcrest.MatcherAssert.assertThat;

public class ResourceMemberManagerTest {

    private ResourceMemberManager manager;

    @Before
    public void setUp() throws Exception {
        manager = new ResourceMemberManager();
    }

    @Test
    public void testRegister() throws Exception {
        ResourceKey key = new StringKey("mock");
        HandleDescriptor handle = new MockDescriptor(key);
        manager.register(handle);

        assertThat(
                manager.getResourceMembers(key).getMembers().contains(handle),
                is(true));
    }

    @Test
    public void testRemove() throws Exception {
        ResourceKey key = new StringKey("mock");
        HandleDescriptor handle = new MockDescriptor(key);
        manager.register(handle);
        manager.remove(handle);

        assertThat(
                manager.getResourceMembers(key).getMembers().contains(handle),
                is(false));
    }

    @Test
    public void testRemoveHandleNotRegistered() throws Exception {
        ResourceKey key = new StringKey("mock");
        HandleDescriptor handle = new MockDescriptor(key);
        manager.remove(handle);
    }

    @Test
    public void testGetResourceMembers() throws Exception {
        ResourceKey key = new StringKey("mock");
        assertThat(manager.getResourceMembers(key), notNullValue());

        HandleDescriptor handle = new MockDescriptor(key);
        manager.register(handle);

        assertThat(manager.getResourceMembers(key).getMembers().size(), is(1));
    }
}