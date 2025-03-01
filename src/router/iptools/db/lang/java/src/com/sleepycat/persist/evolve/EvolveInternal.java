/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.persist.evolve;

/**
 * Internal access class that should not be used by applications.
 *
 * @author Mark Hayes
 */
public class EvolveInternal {

    /**
     * Internal access method that should not be used by applications.
     *
     * @return the EvolveEvent.
     */
    public static EvolveEvent newEvent() {
        return new EvolveEvent();
    }

    /**
     * Internal access method that should not be used by applications.
     *
     * @param event the EvolveEvent.
     * @param entityClassName the class name.
     * @param nRead the number read.
     * @param nConverted the number converted.
     */
    public static void updateEvent(EvolveEvent event,
                                   String entityClassName,
                                   int nRead,
                                   int nConverted) {
        event.update(entityClassName);
        event.getStats().add(nRead, nConverted);
    }
}
