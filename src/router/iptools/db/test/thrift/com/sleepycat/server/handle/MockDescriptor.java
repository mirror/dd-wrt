/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.DatabaseException;

public class MockDescriptor extends HandleDescriptor<String> {

    public MockDescriptor(ResourceKey key, HandleDescriptor... parents) {
        super("mock", key, parents);
    }

    public MockDescriptor(String key, HandleDescriptor... parents) {
        this(new StringKey(key), parents);
    }

    @Override
    protected void closeBdbHandle() throws DatabaseException {
    }
}
