/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist.evolve;

/**
 * The event passed to the EvolveListener interface during eager entity
 * evolution.
 *
 * @see com.sleepycat.client.persist.evolve Class Evolution
 * @author Mark Hayes
 */
public class EvolveEvent {

    private EvolveStats stats;
    private String entityClassName;

    EvolveEvent() {
        this.stats = new EvolveStats();
    }

    void update(String entityClassName) {
        this.entityClassName = entityClassName;
    }

    /**
     * The cumulative statistics gathered during eager evolution.
     *
     * @return the cumulative statistics.
     */
    public EvolveStats getStats() {
        return stats;
    }

    /**
     * The class name of the current entity class being converted.
     *
     * @return the class name.
     */
    public String getEntityClassName() {
        return entityClassName;
    }
}
