/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import java.io.File;
import java.io.IOException;

/**
 * A DatabaseFileKey uniquely identifies a database file.
 */
public class DatabaseFileKey extends FileKey {
    /** The relative path name. */
    private final String relativePath;

    /** If the database file is an in-memory database. */
    private final boolean inMemory;

    public DatabaseFileKey(File dbFile, String relativePath,
            boolean inMemory) throws IOException {
        super(dbFile);
        this.relativePath = relativePath;
        this.inMemory = inMemory;
    }

    public String getRelativePath() {
        return relativePath;
    }

    public boolean isInMemory() {
        return this.inMemory;
    }
}
