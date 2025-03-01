/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * The interface implemented for setting single-valued foreign keys to null.
 * <p>
 * A key nullifier is used with a secondary database that is configured to
 * have a foreign key integrity constraint and a delete action of {@link
 * SForeignKeyDeleteAction#NULLIFY}.  The key nullifier is specified by calling
 * {@link SSecondaryConfig#setForeignKeyNullifier}.</p>
 * <p>
 * When a referenced record in the foreign key database is deleted and the
 * foreign key delete action is <code>NULLIFY</code>, the {@link
 * SForeignKeyNullifier#nullifyForeignKey} method is called.  This method sets
 * the foreign key reference to null in the datum of the primary database.  The
 * primary database is then updated to contain the modified datum.  The result
 * is that the secondary key is deleted.</p>
 * <p>
 * This interface may be used along with {@link SSecondaryKeyCreator} for
 * many-to-one and one-to-one relationships.  It may <em>not</em> be used with
 * {@link SSecondaryMultiKeyCreator} because the secondary key is not passed as
 * a parameter to the nullifyForeignKey method and this method would not know
 * which key to nullify.  When using {@link SSecondaryMultiKeyCreator}, use
 * {@link SForeignMultiKeyNullifier} instead.
 */
public interface SForeignKeyNullifier extends SForeignMultiKeyNullifier {
    /**
     * Sets the foreign key reference to null in the datum of the primary
     * database.
     *
     * @param secondary the database in which the foreign key integrity
     * constraint is defined. This parameter is passed for informational
     * purposes but is not commonly used.
     * @param data the existing primary datum in which the foreign key
     * reference should be set to null.  This parameter should be updated by
     * this method if it returns true.
     * @return true if the datum was modified, or false to indicate that the
     * key is not present.
     * @throws SDatabaseException if an error occurs attempting to clear the
     * key reference.
     */
    boolean nullifyForeignKey(SSecondaryDatabase secondary, SDatabaseEntry data)
            throws SDatabaseException;

    @Override
    default boolean nullifyForeignKey(
            SSecondaryDatabase secondary, SDatabaseEntry key,
            SDatabaseEntry data, SDatabaseEntry secKey)
            throws SDatabaseException {
        return nullifyForeignKey(secondary, data);
    }
}
