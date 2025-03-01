/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import java.util.Arrays;
import java.util.Objects;

/**
 * A SequenceKey uniquely identifies a sequence.
 */
public class SequenceKey implements ResourceKey {
    /** The key for the enclosing database. */
    private final DatabaseKey database;

    /** The BDB key of the sequence. */
    private final byte[] key;

    public SequenceKey(DatabaseKey dbKey, byte[] key) {
        this.database = Objects.requireNonNull(dbKey, "dbKey is null.");
        this.key = Arrays.copyOf(Objects.requireNonNull(key, "key is null."),
                key.length);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        SequenceKey that = (SequenceKey) o;
        return Objects.equals(database, that.database) &&
                Objects.equals(key, that.key);
    }

    @Override
    public int hashCode() {
        return Objects.hash(database, key);
    }

    @Override
    public String toString() {
        return "SequenceKey{" +
                "database=" + database +
                ", key=" + Arrays.toString(key) +
                '}';
    }
}
