/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.callbacks;

import com.sleepycat.db.Cursor;
import com.sleepycat.db.Database;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DatabaseType;
import com.sleepycat.db.Environment;
import com.sleepycat.db.OperationStatus;
import com.sleepycat.db.SecondaryDatabase;
import com.sleepycat.db.SecondaryMultiKeyCreator;
import com.sleepycat.db.Transaction;
import com.sleepycat.server.handle.DatabaseFileKey;
import com.sleepycat.server.handle.DatabaseKey;
import com.sleepycat.server.handle.FileKey;
import com.sleepycat.server.util.KeyDataPair;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * A generic secondary key creator used for server managed secondary databases.
 * <p>
 * This key creator uses an auxiliary Hash database to store secondary keys
 * created by the key creator on the client side. The keys of the auxiliary
 * Hash database are the key-data pairs of the primary database. The data of
 * the auxiliary database are the secondary keys for corresponding key-data
 * pairs.
 * <p>
 * For put operations, secondary keys for new data values are computed at the
 * client side and passed to this creator with the {@link #setNewKeys} method.
 * These new secondary keys are saved in the auxiliary Hash database. For
 * delete operations (or put operations that update existing values), secondary
 * keys are read from the auxiliary Hash database. These secondary keys are
 * removed from the auxiliary database after the operation.
 * <p>
 * Accesses to the auxiliary database must be protected with the same
 * transaction used to perform the put or delete operation. In case of
 * auto-commit, where the operation is protected with an implicit transaction,
 * an explicit transaction must be created and committed if the operation
 * succeeds, or aborted if the operation fails. The transaction to protect the
 * accesses to the auxiliary database is set with the {@link #setTransaction}
 * method.
 */
public class ServerKeyCreator implements SecondaryMultiKeyCreator {
    /** The suffix used for auxiliary databases. */
    private static final String AUX_DB_SUFFIX = "__aux.db";

    /** The secondary database's environment. */
    private final Environment env;

    /** The database key of the secondary database. */
    private final DatabaseKey secondaryDbKey;

    /** A database which saves secondary keys. */
    private Database auxiliaryDb;

    /** The transaction protecting the current operation. */
    private ThreadLocal<Transaction> operationTxn;

    /** The new secondary keys created for the current operation. */
    private ThreadLocal<Map<KeyDataPair, List<DatabaseEntry>>> newKeys;

    /**
     * Create a key creator for a specified secondary database.
     *
     * @param env the enclosing environment of the secondary database
     * @param sdbKey the resource key of the secondary database
     */
    public ServerKeyCreator(Environment env, DatabaseKey sdbKey)
            throws DatabaseException {
        this.env = env;
        this.secondaryDbKey = sdbKey;
        this.operationTxn = new ThreadLocal<>();
        this.newKeys = new ThreadLocal<>();
    }

    /**
     * Compute the database resource key of the auxiliary database from the
     * database resource key of the specified secondary database.
     *
     * @param sdbKey the resource key of a secondary database
     * @return the resource key of the auxiliary database
     * @throws IOException on file error
     */
    public static DatabaseKey getAuxiliaryDbKey(DatabaseKey sdbKey)
            throws IOException {
        DatabaseFileKey sdbFileKey = sdbKey.getDatabaseFile();
        DatabaseFileKey auxFileKey;
        String auxDbName;
        if (sdbFileKey.isInMemory()) {
            auxFileKey = sdbFileKey;
            auxDbName = getAuxiliaryName(sdbKey.getDatabaseName());
        } else {
            String auxFile = getAuxiliaryName(sdbFileKey.getCanonicalPath());
            String auxRelative = getAuxiliaryName(sdbFileKey.getRelativePath());
            auxFileKey =
                    new DatabaseFileKey(new File(auxFile), auxRelative, false);
            auxDbName = sdbKey.getDatabaseName();
        }
        return new DatabaseKey(auxFileKey, auxDbName);
    }

    /**
     * Return the name used for the auxiliary database. The specified name is
     * either the database file name or the sub-database name.
     *
     * @param name the name used by the secondary database
     * @return the name for the auxiliary database.
     */
    public static String getAuxiliaryName(String name) {
        return name + AUX_DB_SUFFIX;
    }

    /**
     * Open the auxiliary database for this secondary key creator. This must be
     * called after the secondary database using this key creator is opened.
     *
     * @throws DatabaseException on database error
     * @throws IOException on file error
     */
    public void openAuxiliaryDb()
            throws DatabaseException, IOException {
        DatabaseConfig config = createAuxiliaryDbConfig();
        DatabaseKey auxDbKey = getAuxiliaryDbKey(this.secondaryDbKey);
        DatabaseFileKey auxFileKey = auxDbKey.getDatabaseFile();
        String auxFile =
                auxFileKey.isInMemory() ? null : auxFileKey.getRelativePath();

        this.auxiliaryDb = env.openDatabase(this.operationTxn.get(), auxFile,
                auxDbKey.getDatabaseName(), config);
    }

    private DatabaseConfig createAuxiliaryDbConfig() {
        DatabaseConfig config = new DatabaseConfig();
        config.setAllowCreate(true);
        config.setExternalFileThreshold(0);
        config.setSortedDuplicates(true);
        config.setTransactional(true);
        config.setType(DatabaseType.HASH);
        return config;
    }

    /**
     * Set the transaction used to protect the current operation.
     *
     * @param txn the transaction
     */
    public void setTransaction(Transaction txn) {
        this.operationTxn.set(txn);
    }

    /**
     * Set the new secondary keys created for the current operation.
     *
     * @param newKeys the set of new secondary keys
     */
    public void setNewKeys(Map<KeyDataPair, List<DatabaseEntry>> newKeys) {
        this.newKeys.set(newKeys);
    }

    /**
     * Close the auxiliary database. This method must be called when the
     * associated secondary database is closed.
     *
     * @throws DatabaseException on database error.
     */
    public void close() throws DatabaseException {
        this.auxiliaryDb.close();
    }

    @SuppressWarnings("unchecked")
    @Override
    public void createSecondaryKeys(SecondaryDatabase secondary,
            DatabaseEntry key, DatabaseEntry data, Set result)
            throws DatabaseException {
        Cursor auxCursor = null;
        try {
            auxCursor =
                    this.auxiliaryDb.openCursor(this.operationTxn.get(), null);

            DatabaseEntry auxKey = makeAuxKey(key, data);
            KeyDataPair pair = new KeyDataPair(key, data);
            Map<KeyDataPair, List<DatabaseEntry>> keys = this.newKeys.get();

            if (keys != null && keys.containsKey(pair)) {
                List<DatabaseEntry> secondaryKeys = keys.get(pair);
                storeKeys(auxKey, secondaryKeys, auxCursor);
                result.addAll(secondaryKeys);
            } else {
                result.addAll(removeKeys(auxKey, auxCursor));
            }
        } finally {
            if (auxCursor != null) {
                auxCursor.close();
            }
        }
    }

    private DatabaseEntry makeAuxKey(DatabaseEntry key, DatabaseEntry data) {
        int bufSize = key.getSize() + data.getSize() + 2 * Integer.BYTES;
        ByteBuffer buffer = ByteBuffer.allocate(bufSize);

        buffer.putInt(key.getSize())
                .put(key.getData(), key.getOffset(), key.getSize())
                .putInt(data.getSize())
                .put(data.getData(), data.getOffset(), data.getSize());
        return new DatabaseEntry(buffer.array());
    }

    private void storeKeys(DatabaseEntry auxKey,
            List<DatabaseEntry> secondaryKeys, Cursor auxCursor)
            throws DatabaseException {
        for (DatabaseEntry sKey : secondaryKeys) {
            auxCursor.putNoDupData(auxKey, sKey);
        }
    }

    private List<DatabaseEntry> removeKeys(DatabaseEntry auxKey,
            Cursor auxCursor) throws DatabaseException {
        List<DatabaseEntry> keys = new LinkedList<>();

        DatabaseEntry sKey = new DatabaseEntry();
        OperationStatus status = auxCursor.getSearchKey(auxKey, sKey, null);
        while (status == OperationStatus.SUCCESS) {
            keys.add(sKey);
            auxCursor.delete();
            sKey = new DatabaseEntry();
            status = auxCursor.getNextDup(auxKey, sKey, null);
        }

        return keys;
    }
}
