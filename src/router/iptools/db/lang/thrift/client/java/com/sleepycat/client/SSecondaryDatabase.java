/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TCursor;
import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TDbGetConfig;
import com.sleepycat.thrift.TDbGetMode;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import static com.sleepycat.client.SForeignKeyDeleteAction.NULLIFY;

/**
 * A secondary database handle.
 * <p>
 * Secondary databases are opened with {@link SEnvironment#openSecondaryDatabase}
 * and are always associated with a single primary database. The distinguishing
 * characteristics of a secondary database are:
 * <ul>
 * <li>Records are automatically added to a secondary database when records are
 * added, modified and deleted in the primary database. Direct calls to {@code
 * put()} methods on a secondary database are prohibited.</li>
 * <li>The {@link #delete} method of a secondary database will delete the
 * primary record and as well as all its associated secondary records.</li>
 * <li>Calls to all get() methods will return the data from the associated
 * primary database.</li>
 * <li>Additional {@code get()} method signatures are provided to return the
 * primary key in an additional pKey parameter.</li>
 * <li>Calls to {@link #openCursor} will return a {@link SSecondaryCursor},
 * which itself has {@code get()} methods that return the data of the primary
 * database and additional {@code get()} method signatures for returning the
 * primary key.</li>
 * </ul>
 * Before opening or creating a secondary database you must implement the
 * {@link SSecondaryKeyCreator} or {@link SSecondaryMultiKeyCreator} interface.
 * <p>
 * For example, to create a secondary database that supports duplicates:
 * <pre>
 *  SDatabase primaryDb; // The primary database must already be open.
 *  SSecondaryKeyCreator keyCreator; // Your key creator implementation.
 *  SSecondaryConfig secConfig = new SSecondaryConfig();
 *  secConfig.setAllowCreate(true);
 *  secConfig.setSortedDuplicates(true);
 *  secConfig.setKeyCreator(keyCreator);
 *  SSecondaryDatabase newDb = env.openSecondaryDatabase(transaction,
 *                                                       "myDatabaseName",
 *                                                       primaryDb,
 *                                                       secConfig);
 * </pre>
 * If a primary database is to be associated with one or more secondary
 * databases, it may not be configured for duplicates.
 * <p>
 * Note that the associations between primary and secondary databases are not
 * stored persistently. Whenever a primary database is opened for write access
 * by the application, the appropriate associated secondary databases should
 * also be opened by the application. This is necessary to ensure data
 * integrity when changes are made to the primary database.
 */
public class SSecondaryDatabase extends SDatabase {
    /** The primary database. */
    private final SDatabase primary;

    /** The configuration object. */
    private final SSecondaryConfig config;

    SSecondaryDatabase(TDatabase tDb, String fileName, String databaseName,
            BdbService.Client client, SEnvironment env, SDatabase primary,
            SSecondaryConfig config) {
        super(tDb, fileName, databaseName, client, env);
        this.primary = primary;
        this.config = config;
        primary.associate(this);
        if (config.getForeignKeyDatabase() != null &&
                config.getForeignKeyDeleteAction() == NULLIFY) {
            SForeignMultiKeyNullifier nullifier =
                    config.getForeignMultiKeyNullifier();
            if (nullifier == null) {
                nullifier = config.getForeignKeyNullifier();
            }
            config.getForeignKeyDatabase().associateForeign(this, nullifier);
        }
    }

    @Override
    public void close() throws SDatabaseException {
        primary.disassociate(this);
        if (config.getForeignKeyDatabase() != null &&
                config.getForeignKeyDeleteAction() == NULLIFY) {
            config.getForeignKeyDatabase().disassociateForeign(this);
        }
        super.close();
    }

    /**
     * Returns the primary database associated with this secondary database.
     *
     * @return the primary database associated with this secondary database
     */
    public SDatabase getPrimaryDatabase() {
        return this.primary;
    }

    /**
     * Returns a copy of the secondary configuration of this database.
     *
     * @return a copy of the secondary configuration of this database.
     * @throws SDatabaseException if a failure occurs.
     */
    public SSecondaryConfig getSecondaryConfig()
            throws SDatabaseException {
        return remoteCall(() -> new SSecondaryConfig(this.config,
                this.client.getDatabaseConfig(this.tDb)));
    }

    /**
     * Retrieves the secondary key / primary key / primary data tuple with the
     * given secondary key.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param sKey the secondary key used as input
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus get(STransaction txn, SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        if (sKey.getPartial()) {
            throw new IllegalArgumentException(
                    "Partial secondary key is not supported.");
        }
        return dbPGet(txn, sKey, pKey, pData, lockMode, TDbGetMode.DEFAULT);
    }

    /**
     * Retrieves the key/data pair with the specified secondary and primary
     * key, that is, both the primary and secondary key items must match.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param sKey the secondary key used as input
     * @param pKey the primary key used as input
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchBoth(STransaction txn, SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        if (sKey.getPartial()) {
            throw new IllegalArgumentException(
                    "Partial secondary key is not supported.");
        }
        return dbPGet(txn, sKey, pKey, pData, lockMode, TDbGetMode.GET_BOTH);
    }

    /**
     * Retrieves the key/data pair associated with the specific numbered record
     * of the database.
     * <p>
     * The specified secondary key must be a record number as described in
     * {@link SDatabaseEntry}. This determines the record to be retrieved.
     * <p>
     * For this method to be called, the underlying database must be of type
     * Btree, and it must have been configured to support record numbers.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param sKey the record number as input; the secondary key returned as
     * output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchRecordNumber(STransaction txn,
            SDatabaseEntry sKey, SDatabaseEntry pKey, SDatabaseEntry pData,
            SLockMode lockMode) throws SDatabaseException {
        return dbPGet(txn, sKey, pKey, pData, lockMode, TDbGetMode.SET_RECNO);
    }

    private SOperationStatus dbPGet(STransaction txn, SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode,
            TDbGetMode mode) {
        return remotePGet(sKey, pKey, pData, searchTerm -> {
            TDbGetConfig config = createConfig(pData, lockMode).setMode(mode);
            return this.client.dbGetWithPKey(this.tDb,
                    STransaction.nullSafeGet(txn), searchTerm, config);
        });
    }

    /**
     * Return a {@link SSecondaryCursor} into this secondary database.
     *
     * @param txn To use a cursor for writing to a transactional database, an
     * explicit transaction must be specified. For read-only access to a
     * transactional database, the transaction may be null.
     * To transaction-protect cursor operations, cursors must be opened and
     * closed within the context of a transaction, and the txn parameter
     * specifies the transaction context in which the cursor will be used.
     * @param config the cursor attributes; if null, default attributes are
     * used
     * @return a secondary database cursor
     * @throws SDatabaseException if any error occurs
     */
    @Override
    public SSecondaryCursor openCursor(STransaction txn, SCursorConfig config)
            throws SDatabaseException {
        return remoteCall(() -> {
            TCursor cursor = this.client.openCursor(this.tDb,
                    STransaction.nullSafeGet(txn),
                    SCursorConfig.nullSafeGet(config));
            return new SSecondaryCursor(cursor, this, txn, this.client);
        });
    }

    Set<SDatabaseEntry> calculateSKeys(SDatabaseEntry key,
            SDatabaseEntry data) {
        if (this.config.getKeyCreator() != null) {
            SDatabaseEntry result = new SDatabaseEntry();
            if (this.config.getKeyCreator().createSecondaryKey(this, key, data,
                    result)) {
                return Collections.singleton(result);
            }
        } else if (this.config.getMultiKeyCreator() != null) {
            Set<SDatabaseEntry> results = new HashSet<>();
            this.config.getMultiKeyCreator().createSecondaryKeys(this, key,
                    data, results);
            return results;
        }
        return Collections.emptySet();
    }
}
