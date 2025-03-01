/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * The configuration properties of a {@link SJoinCursor}. The join cursor
 * configuration is specified when calling {@link SDatabase#join}.
 * <p>
 * To create a configuration object with default attributes:
 * <pre>
 *  SJoinConfig config = new SJoinConfig();
 * </pre>
 * To set custom attributes:
 * <pre>
 *  SJoinConfig config = new SJoinConfig();
 *  config.setNoSort(true);
 * </pre>
 */
public class SJoinConfig {
    /** The no-sort flag. */
    private boolean noSort;

    /**
     * Creates an instance with the default settings.
     */
    public SJoinConfig() {
        this.noSort = false;
    }

    /**
     * Returns whether automatic sorting of the input cursors is disabled.
     *
     * @return whether automatic sorting of the input cursors is disabled
     */
    public boolean getNoSort() {
        return this.noSort;
    }

    /**
     * Specifies whether automatic sorting of the input cursors is disabled.
     * <p>
     * Joined values are retrieved by doing a sequential iteration over the
     * first cursor in the cursor array, and a nested iteration over each
     * following cursor in the order they are specified in the array. This
     * requires database traversals to search for the current datum in all the
     * cursors after the first. For this reason, the best join performance
     * normally results from sorting the cursors from the one that refers to
     * the least number of data items to the one that refers to the most.
     * Unless this method is called with true, {@link SDatabase#join} does this
     * sort on behalf of its caller.
     * <p>
     * If the data are structured so that cursors with many data items also
     * share many common elements, higher performance will result from listing
     * those cursors before cursors with fewer data items; that is, a sort
     * order other than the default. Calling this method permits applications
     * to perform join optimization prior to calling {@link SDatabase#join}.
     *
     * @param noSort whether automatic sorting of the input cursors is disabled
     */
    public void setNoSort(boolean noSort) {
        this.noSort = noSort;
    }
}
