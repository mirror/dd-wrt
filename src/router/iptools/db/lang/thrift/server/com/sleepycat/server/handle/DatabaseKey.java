/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import java.util.Objects;

/**
 * A DatabaseKey uniquely identifies a database.
 */
public class DatabaseKey implements ResourceKey {
    /** The key for the database file. */
    private final DatabaseFileKey databaseFile;

    /** The sub-database name, can be null. */
    private final String databaseName;

    public DatabaseKey(DatabaseFileKey file, String name) {
        this.databaseFile = Objects.requireNonNull(file, "file is null.");
        this.databaseName = name;
    }

    public String getDatabaseName() {
        return this.databaseName;
    }

    public DatabaseFileKey getDatabaseFile() {
        return this.databaseFile;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        DatabaseKey that = (DatabaseKey) o;
        return Objects.equals(databaseFile, that.databaseFile) &&
                Objects.equals(databaseName, that.databaseName);
    }

    @Override
    public int hashCode() {
        return Objects.hash(databaseFile, databaseName);
    }

    @Override
    public String toString() {
        return "DatabaseKey{" +
                "databaseFile=" + databaseFile +
                ", databaseName='" + databaseName + '\'' +
                '}';
    }
}
