/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.Database;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.SecondaryDatabase;

import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.function.Consumer;

/**
 * A DatabaseDescriptor is a HandleDescriptor for a Database.
 */
public class DatabaseDescriptor extends HandleDescriptor<Database> {
    /** A list of associated secondary databases. */
    private ConcurrentLinkedDeque<SecondaryDatabaseDescriptor> secondaries;

    /**
     * A list of secondary databases that have foreign key constraints for
     * this database.
     */
    private ConcurrentLinkedDeque<SecondaryDatabaseDescriptor> foreignKeys;

    /**
     * Create a DatabaseDescriptor for a primary database.
     *
     * @param db the BDB database handle
     * @param key the resource key
     * @param env the parent environment
     */
    public DatabaseDescriptor(Database db, DatabaseKey key,
            EnvironmentDescriptor env) {
        super(db, key, env);
        this.secondaries = new ConcurrentLinkedDeque<>();
        this.foreignKeys = new ConcurrentLinkedDeque<>();
    }

    /**
     * Create a DatabaseDescriptor for a secondary database.
     *
     * @param sdb the BDB secondary database handle
     * @param key the resource key
     * @param env the parent environment
     * @param primary the primary database
     */
    protected DatabaseDescriptor(SecondaryDatabase sdb, DatabaseKey key,
            EnvironmentDescriptor env, DatabaseDescriptor primary) {
        super(sdb, key, env, primary);
    }

    protected void associate(SecondaryDatabaseDescriptor secondary) {
        this.secondaries.add(secondary);
    }

    protected void disassociate(SecondaryDatabaseDescriptor secondary) {
        this.secondaries.remove(secondary);
    }

    protected void associateForeign(SecondaryDatabaseDescriptor secondary) {
        this.foreignKeys.add(secondary);
    }

    protected void disassociateForeign(SecondaryDatabaseDescriptor secondary) {
        this.foreignKeys.remove(secondary);
    }

    @Override
    public DatabaseKey getResourceKey() {
        return (DatabaseKey) super.getResourceKey();
    }

    @Override
    public ResourceKey[] resourceOwners() {
        DatabaseKey key = getResourceKey();
        if (key.getDatabaseName() == null) {
            return new ResourceKey[]{key.getDatabaseFile()};
        } else {
            return new ResourceKey[]{key.getDatabaseFile(), key};
        }
    }

    @Override
    protected void closeBdbHandle() throws DatabaseException {
        getHandle().close();
    }

    /**
     * Return if this database has associated secondary databases.
     *
     * @return if this database has associated secondary databases
     */
    public boolean hasSecondaryDb() {
        return !this.secondaries.isEmpty();
    }

    /**
     * Perform an operation on each secondary database associated with this
     * primary database.
     *
     * @param op the operation
     */
    public void forEachSecondary(Consumer<SecondaryDatabaseDescriptor> op) {
        this.secondaries.forEach(op);
    }

    /**
     * Perform an operation on each foreign secondary database that have foreign
     * constraints for this database.
     *
     * @param op the operation
     */
    public void forEachForeignSecondary(
            Consumer<SecondaryDatabaseDescriptor> op) {
        this.foreignKeys.forEach(op);
    }
}
