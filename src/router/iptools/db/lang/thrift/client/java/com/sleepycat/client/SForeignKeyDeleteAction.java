/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TFKDeleteAction;

/**
 * The action taken when a referenced record in the foreign key database is
 * deleted.
 * <p>
 * The delete action applies to a secondary database that is configured to have
 * a foreign key integrity constraint. The delete action is specified by
 * calling {@link SSecondaryConfig#setForeignKeyDeleteAction}.
 * <p>
 * When a record in the foreign key database is deleted, it is checked to see
 * if it is referenced by any record in the associated secondary database. If
 * the key is referenced, the delete action is applied. By default, the delete
 * action is ABORT.
 */
public enum SForeignKeyDeleteAction {
    /**
     * When a referenced record in the foreign key database is deleted, abort
     * the transaction by throwing a {@link SDatabaseException}.
     */
    ABORT,
    /**
     * When a referenced record in the foreign key database is deleted, delete
     * the primary database record that references it.
     */
    CASCADE,
    /**
     * When a referenced record in the foreign key database is deleted, set the
     * reference to null in the primary database record that references it,
     * thereby deleting the secondary key.
     * @see SForeignKeyNullifier
     * @see SForeignMultiKeyNullifier
     */
    NULLIFY;

    static TFKDeleteAction toThrift(SForeignKeyDeleteAction action) {
        return TFKDeleteAction.valueOf(action.name());
    }

    static SForeignKeyDeleteAction toBdb(TFKDeleteAction action) {
        return SForeignKeyDeleteAction.valueOf(action.name());
    }
}
