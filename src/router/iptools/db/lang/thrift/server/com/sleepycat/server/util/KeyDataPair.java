/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.util;

import com.sleepycat.db.DatabaseEntry;

import java.util.Objects;

/**
 * A KeyDataPair represents a pair of key / data {@link
 * com.sleepycat.db.DatabaseEntry} items.
 */
public class KeyDataPair {
    /** The key part. */
    private final DatabaseEntry key;

    /** The data part. */
    private final DatabaseEntry data;

    /**
     * Create a KeyDataPair.
     *
     * @param key the key part
     * @param data the data part
     */
    public KeyDataPair(DatabaseEntry key, DatabaseEntry data) {
        this.key = Objects.requireNonNull(key);
        this.data = Objects.requireNonNull(data);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        KeyDataPair that = (KeyDataPair) o;
        return Objects.equals(key, that.key) &&
                Objects.equals(data, that.data);
    }

    @Override
    public int hashCode() {
        return Objects.hash(key, data);
    }
}
