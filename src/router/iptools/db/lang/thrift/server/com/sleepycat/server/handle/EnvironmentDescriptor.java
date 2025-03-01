/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.Environment;

/**
 * An EnvironmentDescriptor is a HandleDescriptor for an Environment.
 */
public class EnvironmentDescriptor extends HandleDescriptor<Environment> {
    /**
     * Create an EnvironmentDescriptor.
     *
     * @param env the BDB environment handle
     * @param key the resource key for the home directory
     */
    public EnvironmentDescriptor(Environment env, EnvironmentKey key) {
        super(env, key);
    }

    @Override
    protected void closeBdbHandle() throws DatabaseException {
        getHandle().close();
    }
}
