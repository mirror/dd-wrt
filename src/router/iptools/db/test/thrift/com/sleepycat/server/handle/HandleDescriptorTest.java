/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import org.junit.Test;

import java.util.Arrays;
import java.util.Collection;

import static org.hamcrest.CoreMatchers.is;
import static org.junit.Assert.assertThat;

public class HandleDescriptorTest {

    @Test
    public void testIsExpired() throws Exception {
        MockDescriptor mock = new MockDescriptor("p1");
        mock.touch(1000L);

        assertThat(mock.isExpired(1000L, 1000L), is(false));
        assertThat(mock.isExpired(1999L, 1000L), is(false));
        assertThat(mock.isExpired(2000L, 1000L), is(false));
        assertThat(mock.isExpired(2001L, 1000L), is(true));
    }

    @Test(expected = IllegalArgumentException.class)
    public void testIsExpiredWithNowInPast() throws Exception {
        MockDescriptor mock = new MockDescriptor("p1");
        mock.touch(1000L);

        mock.isExpired(999L, 1000L);
    }

    @Test
    public void testTouch() throws Exception {
        MockDescriptor p1 = new MockDescriptor("p1");
        MockDescriptor p2 = new MockDescriptor("p2");
        MockDescriptor child = new MockDescriptor("child", p1, p2);

        child.touch(1000L);

        assertThat(child.getLastAccessTime(), is(1000L));
        assertThat(p1.getLastAccessTime(), is(1000L));
        assertThat(p2.getLastAccessTime(), is(1000L));
    }

    @Test
    public void testClose() throws Exception {
        MockDescriptor parent = new MockDescriptor("parent");
        MockDescriptor c1 = new MockDescriptor("c1", parent);
        MockDescriptor c2 = new MockDescriptor("c2", parent);
        MockDescriptor c3 = new MockDescriptor("c3", parent);

        HandleManager manager = new HandleManager();
        manager.register(parent);
        manager.register(c1);
        manager.register(c2);
        manager.register(c3);

        Exception e = c1.close();
        if (e != null) throw e;

        Collection<HandleDescriptor> children = parent.getChildren();
        assertThat(children.size(), is(2));
        assertThat(children.containsAll(Arrays.asList(c2, c3)), is(true));

        e = parent.close();
        if (e != null) throw e;

        assertThat(c2.isClosed(), is(true));
        assertThat(c3.isClosed(), is(true));
    }
}