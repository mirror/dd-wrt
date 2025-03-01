/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * The interface implemented for extracting single-valued secondary keys from
 * primary records.
 * <p>
 * The key creator object is specified by calling {@link
 * SSecondaryConfig#setKeyCreator}. The secondary database configuration is
 * specified when calling {@link SEnvironment#openSecondaryDatabase}.
 * <p>
 * For example:
 * <pre>
 *  class MyKeyCreator implements SSecondaryKeyCreator {
 *      public boolean createSecondaryKey(SSecondaryDatabase secondary,
 *              SDatabaseEntry key,
 *              SDatabaseEntry data,
 *              SDatabaseEntry result) throws SDatabaseException {
 *          //
 *          // DO HERE: Extract the secondary key from the primary key and
 *          // data, and set the secondary key into the result parameter.
 *          //
 *          return true;
 *      }
 *  }
 *  ...
 *  SSecondaryConfig secConfig = new SSecondaryConfig();
 *  secConfig.setKeyCreator(new MyKeyCreator());
 *  // Now pass secConfig to SEnvironment.openSecondaryDatabase
 * </pre>
 */
@FunctionalInterface
public interface SSecondaryKeyCreator {
    /**
     * Creates a secondary key entry, given a primary key and data entry.
     * <p>
     * A secondary key may be derived from the primary key, primary data, or a
     * combination of the primary key and data.  For secondary keys that are
     * optional, the key creator method may return false and the key/data pair
     * will not be indexed.  To ensure the integrity of a secondary database
     * the key creator method must always return the same result for a given
     * set of input parameters.
     *
     * @param secondary the database to which the secondary key will be added.
     * This parameter is passed for informational purposes but is not commonly
     * used.
     * @param key the primary key entry.  This parameter must not be modified
     * by this method.
     * @param data the primary data entry.  This parameter must not be modified
     * by this method.
     * @param result the secondary key created by this method.
     * @return true if a key was created, or false to indicate that the key is
     * not present.
     * @throws SDatabaseException if an error occurs attempting to create the
     * secondary key.
     */
    boolean createSecondaryKey(SSecondaryDatabase secondary, SDatabaseEntry key,
            SDatabaseEntry data, SDatabaseEntry result)
            throws SDatabaseException;
}
