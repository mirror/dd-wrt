/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TCachePriority;

/**
 * Priorities that can be assigned to files in the cache.
 */
public enum SCacheFilePriority {
    /** The lowest priority: pages are the most likely to be discarded. */
    VERY_LOW,
    /** The second lowest priority. */
    LOW,
    /** The default priority. */
    DEFAULT,
    /** The second highest priority. */
    HIGH,
    /** The highest priority: pages are the least likely to be discarded. */
    VERY_HIGH;

    static TCachePriority toThrift(SCacheFilePriority priority) {
        return TCachePriority.valueOf(priority.name());
    }

    static SCacheFilePriority toBdb(TCachePriority priority) {
        return SCacheFilePriority.valueOf(priority.name());
    }
}
