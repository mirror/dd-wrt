/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.Cursor;
import com.sleepycat.db.DatabaseException;

/**
 * A CursorDescriptor is a HandleDescriptor for a Cursor.
 */
public class CursorDescriptor extends HandleDescriptor<Cursor> {
    /** The enclosing transaction. */
    private final TransactionDescriptor transaction;

    /** The enclosing database. */
    private final DatabaseDescriptor db;

    /**
     * Create a CursorDescriptor.
     *
     * @param cursor the BDB cursor handle
     * @param db the parent database
     * @param txn the parent transaction
     */
    public CursorDescriptor(Cursor cursor, DatabaseDescriptor db,
            TransactionDescriptor txn) {
        super(cursor, null, db, txn);
        this.db = db;
        this.transaction = txn;
    }

    /**
     * Create a CursorDescriptor that is a duplicate of another cursor
     *
     * @param cursor the BDB cursor handle
     * @param dup the handle this cursor is duplicated from
     */
    public CursorDescriptor(Cursor cursor, CursorDescriptor dup) {
        super(cursor, null, dup.getParents());
        this.db = dup.db;
        this.transaction = dup.transaction;
    }

    @Override
    public ResourceKey[] resourceOwners() {
        return new ResourceKey[0];
    }

    @Override
    protected void closeBdbHandle() throws DatabaseException {
        getHandle().close();
    }

    /**
     * Return the enclosing database of this cursor.
     *
     * @return the enclosing database
     */
    public DatabaseDescriptor getDb() {
        return this.db;
    }

    /**
     * Return the enclosing transaction of this cursor.
     *
     * @return the enclosing transaction
     */
    public TransactionDescriptor getTransaction() {
        return this.transaction;
    }
}
