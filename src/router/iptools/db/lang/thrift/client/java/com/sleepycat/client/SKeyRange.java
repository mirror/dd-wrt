/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TKeyRangeResult;

/**
 * An object that returns status from the {@link SDatabase#getKeyRange} method.
 */
public class SKeyRange {
    /** Zero if there is no matching key, and non-zero otherwise. */
    public final double equal;

    /**
     * A value between 0 and 1, the proportion of keys greater than the
     * specified key.
     * <p>
     * For example, if the value is 0.05, 5% of the keys in the database are
     * greater than the specified key.
     */
    public final double greater;

    /**
     * A value between 0 and 1, the proportion of keys less than the specified
     * key.
     * <p>
     * For example, if the value is 0.05, 5% of the keys in the database are
     * less than the specified key.
     */
    public final double less;

    SKeyRange(TKeyRangeResult keyRange) {
        this.equal = keyRange.equal;
        this.greater = keyRange.greater;
        this.less = keyRange.less;
    }
}
