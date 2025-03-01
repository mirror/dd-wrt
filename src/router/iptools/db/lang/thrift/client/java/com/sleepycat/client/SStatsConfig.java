/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * Specifies the attributes of a statistics retrieval operation.
 */
public class SStatsConfig {

    private boolean clear;

    private boolean fast;

    /**
     * Create a default SStatsConfig.
     */
    public SStatsConfig() {
        this(false, false);
    }

    /**
     * Create a SStatsConfig with the specified attributes.
     *
     * @param clear if the statistic operation will reset statistics after they
     * are returned
     * @param fast if the statistic operation will return only the values which
     * do not incur some performance penalty
     */
    public SStatsConfig(boolean clear, boolean fast) {
        this.clear = clear;
        this.fast = fast;
    }

    static SStatsConfig nullSafeGet(SStatsConfig config) {
        return config == null ? new SStatsConfig() : config;
    }

    /**
     * Return if the statistics operation is configured to reset statistics
     * after they are returned.
     *
     * @return if the statistics operation is configured to reset statistics
     * after they are returned.
     */
    public boolean getClear() {
        return clear;
    }

    /**
     * Configure the statistics operation to reset statistics after they are
     * returned.
     *
     * @param clear if set to true, configure the statistics operation to reset
     * statistics after they are returned.
     * @return this
     */
    public SStatsConfig setClear(boolean clear) {
        this.clear = clear;
        return this;
    }

    /**
     * Return if the statistics operation is configured to return only the
     * values which do not require expensive actions.
     *
     * @return if the statistics operation is configured to return only the
     * values which do not require expensive actions.
     */
    public boolean getFast() {
        return fast;
    }

    /**
     * Configure the statistics operation to return only the values which do
     * not incur some performance penalty.
     * <p>
     * For example, skip stats that require a traversal of the database or
     * in-memory tree, or which lock down the lock table for a period of time.
     * <p>
     * Among other things, this flag makes it possible for applications to
     * request key and record counts without incurring the performance penalty
     * of traversing the entire database. If the underlying database is of type
     * Recno, or of type Btree and the database was configured to support
     * retrieval by record number, the count of keys will be exact. Otherwise,
     * the count of keys will be the value saved the last time the database was
     * traversed, or 0 if no count of keys has ever been made. If the
     * underlying database is of type Recno, the count of data items will be
     * exact,
     * otherwise, the count of data items will be the value saved the last time
     * the database was traversed, or 0 if no count of data items has ever been
     * done.
     *
     * @param fast if set to true, configure the statistics operation to return
     * only the values which do not incur some performance penalty.
     * @return this
     */
    public SStatsConfig setFast(boolean fast) {
        this.fast = fast;
        return this;
    }
}
