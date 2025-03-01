/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections;

import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;

/**
 * An interface implemented to assign new primary key values.
 * An implementation of this interface is passed to the {@link StoredMap}
 * or {@link StoredSortedMap} constructor to assign primary keys for that
 * store. Key assignment occurs when <code>StoredMap.append()</code> is called.
 *
 * @author Mark Hayes
 */
public interface PrimaryKeyAssigner {

    /**
     * Assigns a new primary key value into the given buffer.
     *
     * @param keyData the buffer.
     *
     * @throws SDatabaseException to stop the operation and cause this exception
     * to be propagated to the caller of <code>StoredMap.append()</code>.
     */
    void assignKey(SDatabaseEntry keyData)
        throws SDatabaseException;
}
