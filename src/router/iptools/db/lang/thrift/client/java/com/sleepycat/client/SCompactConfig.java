/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TCompactConfig;

/**
 * Configuration for {@link SDatabase#compact} operations.
 * <p>
 * For a user created instance, no attribute is set by default. In addition,
 * calling the getter method of an unset attribute results in an
 * IllegalStateException. To set an attribute, call the setter method of the
 * attribute.
 * <p>
 * When used to perform a compaction, system default values are used for
 * unset attributes.
 */
public class SCompactConfig
        extends ThriftWrapper<TCompactConfig, TCompactConfig._Fields> {
    /**
     * Create an empty SCompactConfig with no attribute set.
     */
    public SCompactConfig() {
        super(new TCompactConfig());
    }

    /**
     * Return the the desired fill percentage.
     *
     * @return the the desired fill percentage
     */
    public int getFillPercent() {
        return (int) getField(TCompactConfig._Fields.FILL_PERCENT);
    }

    /**
     * Set the desired fill percentage. If non-zero, the goal for filling
     * pages, specified as a percentage between 1 and 100. Any page in a Btree
     * or Recno databases not at or above this percentage full will be
     * considered for compaction. The default behavior is to consider every
     * page for compaction, regardless of its page fill percentage.
     *
     * @param fillpercent the desired fill percentage
     * @return this
     */
    public SCompactConfig setFillPercent(int fillpercent) {
        getThriftObj().setFillPercent(fillpercent);
        return this;
    }

    /**
     * Return true if page compaction is skipped.
     *
     * @return true if page compaction is skipped
     */
    public boolean getFreeListOnly() {
        return (boolean) getField(TCompactConfig._Fields.FREE_LIST_ONLY);
    }

    /**
     * Configure whether to skip page compaction, only returning pages to the
     * filesystem that are already free and at the end of the file. This flag
     * must be set if the database is a Hash access method database.
     *
     * @param freeListOnly whether to skip page compaction
     * @return this
     */
    public SCompactConfig setFreeListOnly(boolean freeListOnly) {
        getThriftObj().setFreeListOnly(freeListOnly);
        return this;
    }

    /**
     * Return true if pages are returned to the filesystem.
     *
     * @return true if pages are returned to the filesystem
     */
    public boolean getFreeSpace() {
        return (boolean) getField(TCompactConfig._Fields.FREE_SPACE);
    }

    /**
     * Return pages to the filesystem if possible. If this flag is not
     * specified, pages emptied as a result of compaction will be placed on the
     * free list for re-use, but not returned to the filesystem. Note that only
     * pages at the end of the file may be returned. Given the one pass nature
     * of the algorithm if a page near the end of the file is logically near
     * the begining of the btree it will inhibit returning pages to the file
     * system. A second call to the method with a low fillfactor can be used to
     * return pages in such a situation.
     *
     * @param freeSpace whether to return pages to the filesystem
     * @return this
     */
    public SCompactConfig setFreeSpace(boolean freeSpace) {
        getThriftObj().setFreeSpace(freeSpace);
        return this;
    }

    /**
     * Return the maximum number of pages to free.
     *
     * @return the maximum number of pages to free
     */
    public int getMaxPages() {
        return (int) getField(TCompactConfig._Fields.MAX_PAGES);
    }

    /**
     * Set the maximum number of pages to free.
     *
     * @param maxPages if non-zero, the call will return after that number of
     * pages have been freed
     * @return this
     */
    public SCompactConfig setMaxPages(int maxPages) {
        getThriftObj().setMaxPages(maxPages);
        return this;
    }

    /**
     * Return the lock timeout set for implicit transactions, in microseconds.
     *
     * @return the lock timeout set for implicit transactions, in microseconds
     */
    public int getTimeout() {
        return (int) getField(TCompactConfig._Fields.TIMEOUT);
    }

    /**
     * Set the lock timeout for implicit transactions. If non-zero, and no
     * transaction parameter was specified to {@link SDatabase#compact}, the
     * lock timeout set for implicit transactions, in microseconds.
     *
     * @param timeout the lock timeout set for implicit transactions, in
     * microseconds
     * @return this
     */
    public SCompactConfig setTimeout(int timeout) {
        getThriftObj().setTimeout(timeout);
        return this;
    }
}
