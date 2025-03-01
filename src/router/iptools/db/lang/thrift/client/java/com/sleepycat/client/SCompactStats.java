/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TCompactResult;

/**
 * Statistics returned by a {@link SDatabase#compact} operation.
 */
public class SCompactStats {
    /** The Thrift object. */
    private final TCompactResult result;

    SCompactStats(TCompactResult result) {
        this.result = result;
    }

    /**
     * The number of empty hash buckets that were found during the compaction
     * phase.
     *
     * @return the number of empty hash buckets that were found during the
     * compaction phase
     */
    public int getEmptyBuckets() {
        return this.result.emptyBuckets;
    }

    /**
     * The number of database pages freed during the compaction phase.
     *
     * @return the number of database pages freed during the compaction phase
     */
    public int getPagesFree() {
        return this.result.pagesFree;
    }

    /**
     * The number of database pages reviewed during the compaction phase.
     *
     * @return the number of database pages reviewed during the compaction phase
     */
    public int getPagesExamine() {
        return this.result.pagesExamine;
    }

    /**
     * The number of levels removed from the Btree database during the
     * compaction phase.
     *
     * @return the number of levels removed from the Btree database during the
     * compaction phase
     */
    public int getLevels() {
        return this.result.levels;
    }

    /**
     * If no transaction parameter was specified to {@link SDatabase#compact},
     * the number of deadlocks which occurred.
     *
     * @return the number of deadlocks occurred
     */
    public int getDeadlock() {
        return this.result.deadlock;
    }

    /**
     * The number of database pages returned to the filesystem.
     *
     * @return the number of database pages returned to the filesystem
     */
    public int getPagesTruncated() {
        return this.result.pagesTruncated;
    }

    /**
     * For convenience, the SCompactStats class has a toString method that
     * lists all the data fields.
     *
     * @return a String that lists all fields
     */
    public String toString() {
        return "CompactStats:"
                + "\n  empty_buckets=" + getEmptyBuckets()
                + "\n  pages_free=" + getPagesFree()
                + "\n  pages_examine=" + getPagesExamine()
                + "\n  levels=" + getLevels()
                + "\n  deadlock=" + getDeadlock()
                + "\n  pages_truncated=" + getPagesTruncated()
                ;
    }
}
