/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.SecondaryDatabase;
import com.sleepycat.db.Transaction;
import com.sleepycat.server.callbacks.ServerKeyCreator;
import com.sleepycat.server.util.KeyDataPair;

import java.util.List;
import java.util.Map;
import java.util.function.Consumer;

/**
 * A SecondaryDatabaseDescriptor is a HandleDescriptor for a secondary
 * Database.
 */
public class SecondaryDatabaseDescriptor extends DatabaseDescriptor {
    /** The database descriptor of the primary database. */
    private final DatabaseDescriptor primary;

    /**
     * The foreign database for which this secondary has a foreign constraint.
     */
    private final DatabaseDescriptor foreign;

    /** The secondary key creator. */
    private final ServerKeyCreator keyCreator;

    /**
     * Create a secondary database descriptor.
     *
     * @param sdb the secondary database handle
     * @param key the resource key
     * @param env the enclosing environment
     * @param primary the primary database
     * @param keyCreator the secondary key creator
     */
    public SecondaryDatabaseDescriptor(SecondaryDatabase sdb,
            DatabaseKey key, EnvironmentDescriptor env,
            DatabaseDescriptor primary, DatabaseDescriptor foreign,
            ServerKeyCreator keyCreator) {
        super(sdb, key, env, primary);
        this.primary = primary;
        this.foreign = foreign;
        this.keyCreator = keyCreator;
        this.primary.associate(this);
        if (this.foreign != null) {
            this.foreign.associateForeign(this);
        }
    }

    @Override
    protected void closeBdbHandle() throws DatabaseException {
        try {
            this.keyCreator.close();
        } finally {
            this.primary.disassociate(this);
            if (this.foreign != null) {
                this.foreign.disassociateForeign(this);
            }
            super.closeBdbHandle();
        }
    }

    /**
     * Return the primary database associated with this secondary database.
     *
     * @return the primary database
     */
    public DatabaseDescriptor getPrimary() {
        return this.primary;
    }

    /**
     * Add new secondary keys generated from client-side key creators.
     *
     * @param keys new secondary keys grouped by database descriptor
     * ids
     */
    public void setNewSecondaryKeys(
            Map<KeyDataPair, List<DatabaseEntry>> keys) {
        this.keyCreator.setNewKeys(keys);
    }

    /**
     * Set the transaction used for the current operation.
     *
     * @param txn the transaction used for the current operation
     */
    public void setCurrentTxn(Transaction txn) {
        this.keyCreator.setTransaction(txn);
    }

    @Override
    public void forEachSecondary(Consumer<SecondaryDatabaseDescriptor> op) {
        getPrimary().forEachSecondary(op);
    }

    @Override
    public void forEachForeignSecondary(
            Consumer<SecondaryDatabaseDescriptor> op) {
        getPrimary().forEachForeignSecondary(op);
    }
}
