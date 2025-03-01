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
import java.util.Objects;

/**
 * A FileKey uniquely identifies either an environment's home directory or a
 * database file.
 */
public abstract class FileKey implements ResourceKey {
    /** The canonical path of the file. */
    private final String canonicalPath;

    /**
     * Create a FileKey representing either a database file or an
     * environment's home directory.
     *
     * @param file the database file or the environment's home directory
     * @throws IOException if an error occurs retrieving the canonical path
     */
    protected FileKey(File file) throws IOException {
        this.canonicalPath = file.getCanonicalPath();
    }

    public String getCanonicalPath() {
        return this.canonicalPath;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        FileKey fileKey = (FileKey) o;
        return Objects.equals(canonicalPath, fileKey.canonicalPath);
    }

    @Override
    public int hashCode() {
        return Objects.hash(canonicalPath);
    }

    @Override
    public String toString() {
        return getClass().getSimpleName() + "{" +
                "canonicalPath='" + canonicalPath + '\'' +
                '}';
    }
}
