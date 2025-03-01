/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import java.util.Collection;
import java.util.Collections;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * ResourceMembers is a ReadWriteLockable which maintains a collection of open
 * handles that uses the same resource.
 */
public class ResourceMembers extends ReadWriteLockable {
    /** The collection of open handles. */
    private ConcurrentLinkedQueue<HandleDescriptor> openHandles =
            new ConcurrentLinkedQueue<>();

    public Collection<HandleDescriptor> getMembers() {
        return Collections.unmodifiableCollection(this.openHandles);
    }

    public void add(HandleDescriptor handle) {
        this.openHandles.add(handle);
    }

    public void remove(HandleDescriptor handle) {
        this.openHandles.remove(handle);
    }

    public boolean isEmpty() {
        return this.openHandles.isEmpty();
    }
}
