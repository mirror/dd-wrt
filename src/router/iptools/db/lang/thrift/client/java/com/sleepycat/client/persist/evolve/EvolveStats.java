/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist.evolve;

/**
 * Statistics accumulated during eager entity evolution.
 *
 * @see com.sleepycat.client.persist.evolve Class Evolution
 * @author Mark Hayes
 */
public class EvolveStats {

    private int nRead;
    private int nConverted;

    EvolveStats() {
    }

    void add(int nRead, int nConverted) {
        this.nRead += nRead;
        this.nConverted += nConverted;
    }

    /**
     * The total number of entities read during eager evolution.
     *
     * @return the number of entities read.
     */
    public int getNRead() {
        return nRead;
    }

    /**
     * The total number of entities converted during eager evolution.
     *
     * @return the number of entities converted.
     */
    public int getNConverted() {
        return nConverted;
    }
}
