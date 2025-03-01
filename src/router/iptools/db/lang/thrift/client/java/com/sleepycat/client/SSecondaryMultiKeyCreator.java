/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import java.util.Set;

/**
 * The interface implemented for extracting multi-valued secondary keys from
 * primary records.
 * <p>
 * The key creator object is specified by calling {@link
 * SSecondaryConfig#setMultiKeyCreator}. The secondary database configuration
 * is specified when calling {@link SEnvironment#openSecondaryDatabase}.
 * <p>
 * For example:
 * <pre>
 *  class MyMultiKeyCreator implements SSecondaryMultiKeyCreator {
 *      public void createSecondaryKeys(SSecondaryDatabase secondary,
 *              SDatabaseEntry key,
 *              SDatabaseEntry data,
 *              Set&lt;SDatabaseEntry&gt; results) throws SDatabaseException {
 *          //
 *          // DO HERE: Extract the secondary keys from the primary key and
 *          // data.  For each key extracted, create a SDatabaseEntry and add
 *          // it to the results set.
 *          //
 *      }
 *  }
 *  ...
 *  SSecondaryConfig secConfig = new SSecondaryConfig();
 *  secConfig.setMultiKeyCreator(new MyMultiKeyCreator());
 *  // Now pass secConfig to Environment.openSecondaryDatabase
 * </pre>
 * Use this interface when any number of secondary keys may be present in a
 * single primary record, in other words, for many-to-many and one-to-many
 * relationships. When only zero or one secondary key is present (for
 * many-to-one and one-to-one relationships) you may use the
 * SSecondaryKeyCreator interface instead. The table below summarizes how to
 * create all four variations of relationships.
 * <table border="yes">
 * <caption>Key creator for different relationships</caption>
 * <tr><th>Relationship</th>
 * <th>Interface</th>
 * <th>Duplicates</th>
 * <th>Example</th>
 * </tr>
 * <tr><td>One-to-one</td>
 * <td>{@link SSecondaryKeyCreator}</td>
 * <td>No</td>
 * <td>A person record with a unique social security number key.</td>
 * </tr>
 * <tr><td>Many-to-one</td>
 * <td>{@link SSecondaryKeyCreator}</td>
 * <td>Yes</td>
 * <td>A person record with a non-unique employer key.</td>
 * </tr>
 * <tr><td>One-to-many</td>
 * <td>{@link SSecondaryMultiKeyCreator}</td>
 * <td>No</td>
 * <td>A person record with multiple unique email address keys.</td>
 * </tr>
 * <tr><td>Many-to-many</td>
 * <td>{@link SSecondaryMultiKeyCreator}</td>
 * <td>Yes</td>
 * <td>A person record with multiple non-unique organization keys.</td>
 * </tr>
 * </table>
 * To configure a database for duplicates, pass true to {@link
 * SDatabaseConfig#setSortedDuplicates}.
 * <p>
 * Note that SSecondaryMultiKeyCreator may also be used for single key
 * secondaries (many-to-one and one-to-one); in this case, at most a single key
 * is added to the results set. SSecondaryMultiKeyCreator is only slightly less
 * efficient than SecondaryKeyCreator in that two or three temporary sets must
 * be created to hold the results.
 */
@FunctionalInterface
public interface SSecondaryMultiKeyCreator {
    /**
     * Create multiple secondary key entries, given a primary key and data
     * entry.
     * <p>
     * A secondary key may be derived from the primary key, primary data, or a
     * combination of the primary key and data.  Zero or more secondary keys
     * may be derived from the primary record and returned in the results
     * parameter. To ensure the integrity of a secondary database the key
     * creator method must always return the same results for a given set of
     * input parameters.
     *
     * @param secondary the database to which the secondary key will be added.
     * This parameter is passed for informational purposes but is not commonly
     * used.
     * @param key the primary key entry.  This parameter must not be modified
     * by this method.
     * @param data the primary data entry.  This parameter must not be modified
     * by this method.
     * @param results the set to contain the the secondary key SDatabaseEntry
     * objects created by this method.
     * @throws SDatabaseException if an error occurs attempting to create the
     * secondary key.
     */
    void createSecondaryKeys(SSecondaryDatabase secondary, SDatabaseEntry key,
            SDatabaseEntry data, Set<SDatabaseEntry> results)
            throws SDatabaseException;
}
