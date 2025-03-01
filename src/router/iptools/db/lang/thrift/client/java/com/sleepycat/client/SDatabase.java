/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TCompactResult;
import com.sleepycat.thrift.TCursor;
import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TDatabaseStatResult;
import com.sleepycat.thrift.TDbGetConfig;
import com.sleepycat.thrift.TDbGetMode;
import com.sleepycat.thrift.TDbPutConfig;
import com.sleepycat.thrift.TIsolationLevel;
import com.sleepycat.thrift.TJoinCursor;
import com.sleepycat.thrift.TKeyData;
import com.sleepycat.thrift.TKeyDataWithSecondaryKeys;
import com.sleepycat.thrift.TSequence;

import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import static com.sleepycat.client.SLockMode.RMW;

/**
 * Creates a database handle for a single Berkeley DB database. A Berkeley DB
 * database provides a mechanism for organizing key-data pairs of information.
 * From the perspective of some database systems, a Berkeley DB database could
 * be thought of as a single table within a larger database.
 * <p>
 * For most database activities, you must open the handle using the open
 * method. When you are done with them, handles must be closed using the close
 * method.
 * <p>
 * Databases are organized within a database environment. Environments provide
 * transactions, recovery, replication and other advanced features. Also, if
 * you are using multiple databases, then environments allow your databases to
 * share a common in-memory cache, which makes for more efficient usage of your
 * hardware's resources. See {@link SEnvironment} for information on using
 * database environments.
 * <p>
 * Database attributes are specified in the {@link SDatabaseConfig} class.
 * <p>
 * To open an existing database with default attributes:
 * <pre>
 *  BdbServerConnection conn = BdbServerConnection.connect(host, port);
 *  SEnvironment env = conn.openEnvironment(home, null);
 *  SDatabase myDatabase = env.openDatabase(null, "mydatabase", null);
 * </pre>
 * To create a database that supports duplicates:
 * <pre>
 *  SDatabaseConfig dbConfig = new SDatabaseConfig();
 *  dbConfig.setAllowCreate(true);
 *  dbConfig.setSortedDuplicates(true);
 *  SDatabase myDatabase = env.openDatabase(null, "mydatabase", dbConfig);
 * </pre>
 */
public class SDatabase
        implements GetHelper, PutHelper, TxnHelper, AutoCloseable {
    /** The remote database handle. */
    protected final TDatabase tDb;

    /** The remote service client. */
    protected final BdbService.Client client;

    /** The enclosing environment. */
    private final SEnvironment env;

    /** The database file name. */
    private final String fileName;

    /** The database name. */
    private final String databaseName;

    /** The set of secondary databases associated with this primary database. */
    private final Set<SSecondaryDatabase> secondaries;

    /**
     * The set of secondary databases which use this database as a foreign
     * key database and which use nullifiers to update data when foreign keys
     * are deleted.
     */
    private final Map<SSecondaryDatabase, SForeignMultiKeyNullifier>
            fkNullifiers;

    /**
     * Constructor for subclasses.
     *
     * @param tDb the Thrift object
     * @param fileName the file name
     * @param databaseName the database name
     * @param client the Thrift client object
     * @param env the enclosing environment
     */
    protected SDatabase(TDatabase tDb, String fileName, String databaseName,
            BdbService.Client client, SEnvironment env) {
        this.tDb = tDb;
        this.fileName = fileName;
        this.databaseName = databaseName;
        this.client = client;
        this.env = env;
        this.secondaries = new HashSet<>();
        this.fkNullifiers = new HashMap<>();
    }

    TDatabase getThriftObj() {
        return this.tDb;
    }

    void associate(SSecondaryDatabase secondary) {
        this.secondaries.add(secondary);
    }

    void disassociate(SSecondaryDatabase secondary) {
        this.secondaries.remove(secondary);
    }

    void associateForeign(SSecondaryDatabase secondary,
            SForeignMultiKeyNullifier nullifier) {
        this.fkNullifiers.put(secondary, nullifier);
    }

    void disassociateForeign(SSecondaryDatabase secondary) {
        this.fkNullifiers.remove(secondary);
    }

    /**
     * Flush any cached database information to disk, close any open cursors,
     * free allocated resources, close underlying files, and discard the
     * database handle.
     * <p>
     * Closing a database handle will close any open cursors that refer to it.
     * However, you should make sure to close all your transaction handles
     * before closing your database handle.
     * <p>
     * Because key/data pairs are cached in memory, failing to sync the file
     * with the this methods may result in inconsistent or lost information.
     * So, to ensure that any data cached in main memory are reflected in the
     * underlying file system, applications should make a point to always close
     * database handles.
     * <p>
     * The database handle may not be accessed again after this method is
     * called, regardless of the method's success or failure.
     * <p>
     * When called on a database that is the primary database for a secondary
     * index, the primary database should be closed only after all secondary
     * indices which reference it have been closed.
     *
     * @throws SDatabaseException if any error occurs
     */
    @Override
    public void close() throws SDatabaseException {
        remoteCall(() -> {
            this.client.closeDatabaseHandle(this.tDb);
            return null;
        });
    }

    /**
     * Return this SDatabase object's configuration.
     * <p>
     * This may differ from the configuration used to open this object if
     * the database existed previously.
     *
     * @return this SDatabase object's configuration
     * @throws SDatabaseException if any error occurs
     */
    public SDatabaseConfig getConfig() throws SDatabaseException {
        return remoteCall(() -> new SDatabaseConfig(
                this.client.getDatabaseConfig(this.tDb)));
    }

    /**
     * Change the settings in an existing database handle. Only set
     * attributes are changed. Therefore, there is no need to call {@link
     * #getConfig()} to get the current settings because unset attributes are
     * not changed.
     *
     * @param config the database attributes; if null, no attribute is changed
     * @throws SDatabaseException if any error occurs
     */
    public void setConfig(SDatabaseConfig config) throws SDatabaseException {
        remoteCall(() -> {
            this.client.setDatabaseConfig(this.tDb,
                    ThriftWrapper.nullSafeGet(config));
            return null;
        });
    }

    /**
     * Return the database's underlying file name.
     *
     * @return the database's underlying file name
     */
    public String getDatabaseFile() {
        return this.fileName;
    }

    /**
     * Return the database name.
     *
     * @return the database name
     */
    public String getDatabaseName() {
        return this.databaseName;
    }

    /**
     * Return the {@link SEnvironment} handle for the database environment
     * underlying this database.
     *
     * @return the {@link SEnvironment} handle for the database environment
     * underlying this database
     */
    @Override
    public SEnvironment getEnvironment() {
        return this.env;
    }

    @Override
    public Set<SSecondaryDatabase> getSecondaryDatabases() {
        return Collections.unmodifiableSet(this.secondaries);
    }

    /**
     * Return the native byte order of the connected server.
     *
     * @return the native byte order of the connected server
     */
    public ByteOrder getServerByteOrder() {
        return getEnvironment().getServerByteOrder();
    }

    /**
     * Retrieves the key/data pair with the given key from the database. If the
     * matching key has duplicate values, the first data item in the set of
     * duplicates is returned.
     * <p>
     * Duplicates are sorted by:
     * <ul>
     * <li>their sort order, if a duplicate sort function was specified.</li>
     * <li>any explicit cursor designated insertion.</li>
     * <li>by insert order. This is the default behavior.</li>
     * </ul>
     * Retrieval of duplicates requires the use of {@link SCursor} operations.
     * When called on a database that has been made into a secondary index,
     * this method returns the key from the secondary index and the data item
     * from the primary database.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key used as input. It must be initialized by the caller.
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return all duplicates of the key.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus get(STransaction txn, SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        if (key.getPartial()) {
            throw new IllegalArgumentException("Partial key is not supported.");
        }
        return dbGet(txn, key, data, lockMode, TDbGetMode.DEFAULT);
    }

    /**
     * Retrieves the key/data pair with the given key and data value, that is,
     * both the key and data items must match.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key used as input. It must be initialized by the caller.
     * @param data the data used as input; only {@link SDatabaseEntry} is
     * supported
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchBoth(STransaction txn, SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        if (key.getPartial()) {
            throw new IllegalArgumentException("Partial key is not supported.");
        }
        if (data != null && !(data instanceof SDatabaseEntry)) {
            throw new IllegalArgumentException(
                    "data must be a SDatabaseEntry.");
        }
        return dbGet(txn, key, data, lockMode, TDbGetMode.GET_BOTH);
    }

    /**
     * Retrieves the key/data pair associated with the specific numbered record
     * of the database.
     * <p>
     * The specified key must be a record number as described in
     * {@link SDatabaseEntry}. This determines the record to be retrieved.
     * <p>
     * For this method to be called, the underlying database must be of type
     * Btree, and it must have been configured to support record numbers.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key returned as output
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return all duplicates of the key.
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
            SDatabaseEntry key, SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        return dbGet(txn, key, data, lockMode, TDbGetMode.SET_RECNO);
    }

    /* Currently disabled.
     *
     * Return the record number and data from the available record closest to
     * the head of the queue, and delete the record. The record number will be
     * returned in the key parameter, and the data will be returned in the data
     * parameter. A record is available if it is not deleted and is not
     * currently locked. The underlying database must be of type Queue for this
     * method to be called.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key returned as output
     * @param data the data returned as output
     * @param wait if there is no record available, this parameter determines
     * whether the method waits for one to become available, or returns
     * immediately with status {@link SOperationStatus#NOTFOUND}
     * @return {@link SOperationStatus#NOTFOUND} if no record is available;
     * {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     *
    public SOperationStatus consume(STransaction txn, SDatabaseEntry key,
            SDatabaseEntry data, boolean wait) throws SDatabaseException {
        return dbGet(txn, key, data, null,
                wait ? TDbGetMode.CONSUME_WAIT : TDbGetMode.CONSUME);
    }
    */

    private SOperationStatus dbGet(STransaction txn, SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode, TDbGetMode mode) {
        if (!isValidType(data)) {
            throw new IllegalArgumentException(
                    "data must be a SDatabaseEntry or SMultipleDataEntry.");
        }
        return remoteGet(key, data, searchTerm -> {
            TDbGetConfig config = createConfig(data, lockMode).setMode(mode);
            return this.client.dbGet(this.tDb, STransaction.nullSafeGet(txn),
                    searchTerm, config);
        });
    }

    private boolean isValidType(SDatabaseEntryBase data) {
        return data == null || data instanceof SDatabaseEntry ||
                data instanceof SMultipleDataEntry;
    }

    protected TDbGetConfig createConfig(SDatabaseEntryBase data,
            SLockMode lockMode) {
        TDbGetConfig config = new TDbGetConfig();
        if (lockMode != null) {
            switch (lockMode) {
                case READ_COMMITTED:
                    config.setIsoLevel(TIsolationLevel.READ_COMMITTED);
                case READ_UNCOMMITTED:
                    config.setIsoLevel(TIsolationLevel.READ_UNCOMMITTED);
                case RMW:
                    config.setRmw(true);
            }
        }
        if (data instanceof SMultipleDataEntry) {
            config.setMultiple(true);
        }
        return config;
    }

    /**
     * Checks if the specified key appears in the database.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key used as input
     * @return {@link SOperationStatus#NOTFOUND} if no record is available;
     * {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus exists(STransaction txn, SDatabaseEntry key)
            throws SDatabaseException {
        return remoteCall(() -> SOperationStatus.toBdb(
                this.client.dbKeyExists(this.tDb, STransaction.nullSafeGet(txn),
                        key.getThriftObj())));
    }

    boolean isEmpty(STransaction txn) throws SDatabaseException {
        try (SCursor cursor = openCursor(txn, null)) {
            return (cursor.getFirst(null, null, null) ==
                    SOperationStatus.NOTFOUND);
        }
    }

    /**
     * Return an estimate of the proportion of keys in the database less than,
     * equal to, and greater than the specified key.
     * <p>
     * The underlying database must be of type Btree.
     * <p>
     * This method does not retain the locks it acquires for the life of the
     * transaction, so estimates are not repeatable.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key being compared
     * @return an estimate of the proportion of keys in the database less than,
     * equal to, and greater than the specified key.
     * @throws SDatabaseException if any error occurs
     */
    public SKeyRange getKeyRange(STransaction txn, SDatabaseEntry key)
            throws SDatabaseException {
        return remoteCall(() -> new SKeyRange(
                this.client.dbKeyRange(this.tDb, STransaction.nullSafeGet(txn),
                        key.getThriftObj())));
    }

    /**
     * Store the key/data pair into the database.
     * <p>
     * If the key already appears in the database and duplicates are not
     * configured, the existing key/data pair will be replaced. If the key
     * already appears in the database and sorted duplicates are configured,
     * the new data value is inserted at the correct sorted location. If the
     * key already appears in the database and unsorted duplicates are
     * configured, the new data value is appended at the end of the duplicate
     * set.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key SDatabaseEntry operated on
     * @param data the data SDatabaseEntry stored
     * @return the operation status
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus put(STransaction txn, SDatabaseEntry key,
            SDatabaseEntry data) throws SDatabaseException {
        return putSingle(txn, key, data, TDbPutConfig.DEFAULT);
    }

    /**
     * Store the key/data pair into the database if it does not already appear
     * in the database.
     * <p>
     * This method may only be called if the underlying database has been
     * configured to support sorted duplicates. (This method may not be
     * specified to the Queue or Recno access methods.)
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key SDatabaseEntry operated on
     * @param data the data SDatabaseEntry stored
     * @return the operation status
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putNoDupData(STransaction txn, SDatabaseEntry key,
            SDatabaseEntry data) throws SDatabaseException {
        return putSingle(txn, key, data, TDbPutConfig.NO_DUP_DATA);
    }

    /**
     * Store the key/data pair into the database if the key does not already
     * appear in the database.
     * <p>
     * This method will fail if the key already exists in the database, even if
     * the database supports duplicates.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key SDatabaseEntry operated on
     * @param data the data SDatabaseEntry stored
     * @return the operation status
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putNoOverwrite(STransaction txn, SDatabaseEntry key,
            SDatabaseEntry data) throws SDatabaseException {
        return putSingle(txn, key, data, TDbPutConfig.NO_OVERWRITE);
    }

    /* Currently disabled.
     *
     * Append the key/data pair to the end of the database.
     * <p>
     * The underlying database must be a Queue or Recno database. The record
     * number allocated to the record is returned in the key parameter.
     * <p>
     * There is a minor behavioral difference between the Recno and Queue
     * access methods this method. If a transaction enclosing this method
     * aborts, the record number may be decremented (and later reallocated by a
     * subsequent operation) in the Recno access method, but will not be
     * decremented or reallocated in the Queue access method.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key SDatabaseEntry operated on
     * @param data the data SDatabaseEntry stored
     * @return the operation status
     * @throws SDatabaseException if any error occurs
     *
    public SOperationStatus append(STransaction txn, SDatabaseEntry key,
            SDatabaseEntry data) throws SDatabaseException {
        if (!this.secondaries.isEmpty()) {
            throw new UnsupportedOperationException("Append is not supported" +
                    "for databases having secondary databases.");
        }
        return putSingle(txn, key, data, TDbPutConfig.APPEND);
    }
    */

    /**
     * Store a set of key/data pairs into the database.
     * <p>
     * This method may not be called on databases configured with unsorted
     * duplicates.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param pairs the key and data pairs operated on
     * @param overwrite if this flag is true and any of the keys already exist
     * in the database, they will be replaced. Otherwise a {@link
     * SOperationStatus#KEYEXIST} error will be returned
     * @return if any of the key/data pairs already appear in the database, this
     * method will return {@link SOperationStatus#KEYEXIST}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putMultipleKey(STransaction txn,
            SMultiplePairs pairs, boolean overwrite) throws SDatabaseException {
        return dbPut(txn, null, pairs.map(this::calculateSKey),
                overwrite ? TDbPutConfig.OVERWRITE_DUP : TDbPutConfig.DEFAULT);
    }

    private SOperationStatus putSingle(STransaction txn, SDatabaseEntry key,
            SDatabaseEntry data, TDbPutConfig option)
            throws SDatabaseException {
        return dbPut(txn, key,
                Collections.singletonList(calculateSKey(key, data)), option);
    }

    private SOperationStatus dbPut(STransaction txn, SDatabaseEntry key,
            List<TKeyDataWithSecondaryKeys> pairs, TDbPutConfig config)
            throws SDatabaseException {
        if (pairs.isEmpty()) {
            return SOperationStatus.SUCCESS;
        }
        return remotePut(pairs, key,
                pairList -> this.client.dbPut(this.tDb,
                        STransaction.nullSafeGet(txn), pairList, config));
    }

    /**
     * Remove key/data pairs from the database.
     * <p>
     * The key/data pair associated with the specified key is discarded from
     * the database. In the presence of duplicate key values, all records
     * associated with the designated key will be discarded.
     * <p>
     * The key/data pair is also deleted from any associated secondary
     * databases. When called on a database that has been made into a secondary
     * index, this method deletes the key/data pair from the primary database
     * and all secondary indices.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key the key operated on
     * @return the method will return {@link SOperationStatus#NOTFOUND} if the
     * specified key is not found in the database; the method will return
     * {@link SOperationStatus#KEYEMPTY} if the database is a Queue or Recno
     * database and the specified key exists, but was never explicitly created
     * by the application or was later deleted; otherwise the method will
     * return {@link SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus delete(STransaction txn, SDatabaseEntry key)
            throws SDatabaseException {
        return runInSingleTxn(txn, autoTxn -> {
            updatePrimaryData(autoTxn, Collections.singletonList(key));

            return remoteDelete(autoTxn, Collections
                    .singletonList(new TKeyData().setKey(key.getThriftObj())));
        });
    }

    /**
     * Remove key/data pairs from the database.
     * <p>
     * The key/data pairs associated with the specified keys are discarded from
     * the database. In the presence of duplicate key values, all records
     * associated with the designated keys will be discarded.
     * <p>
     * The key/data pairs are also deleted from any associated secondary
     * databases. When called on a database that has been made into a secondary
     * index, this method deletes the key/data pairs from the primary database
     * and all secondary indices.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param keys the set of keys operated on
     * @return the method will return {@link SOperationStatus#NOTFOUND} if the
     * specified key is not found in the database; the method will return
     * {@link SOperationStatus#KEYEMPTY} if the database is a Queue or Recno
     * database and the specified key exists, but was never explicitly created
     * by the application or was later deleted; otherwise the method will
     * return {@link SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus deleteMultiple(STransaction txn,
            SMultipleDataEntry keys) throws SDatabaseException {
        return runInSingleTxn(txn, autoTxn -> {
            updatePrimaryData(autoTxn, keys.map(key -> key));

            return remoteDelete(autoTxn,
                    keys.map(k -> new TKeyData().setKey(k.getThriftObj())));
        });
    }

    /**
     * Remove key/data pairs from the database.
     * <p>
     * The specified key/data pairs are discarded from the database.
     * <p>
     * The key/data pairs are also deleted from any associated secondary
     * databases. When called on a database that has been made into a secondary
     * index, this method deletes the key/data pairs from the primary database
     * and all secondary indices.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param pairs the set of key/data pairs operated on
     * @return the method will return {@link SOperationStatus#NOTFOUND} if the
     * specified key is not found in the database; the method will return
     * {@link SOperationStatus#KEYEMPTY} if the database is a Queue or Recno
     * database and the specified key exists, but was never explicitly created
     * by the application or was later deleted; otherwise the method will
     * return {@link SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus deleteMultipleKey(STransaction txn,
            SMultiplePairs pairs) throws SDatabaseException {
        return runInSingleTxn(txn, autoTxn -> {
            updatePrimaryData(autoTxn, pairs.map((key, data) -> key));

            return remoteDelete(autoTxn, pairs.map(
                    (k, d) -> new TKeyData().setKey(k.getThriftObj())
                            .setData(d.getThriftObj())));
        });
    }

    void updatePrimaryData(STransaction txn, List<SDatabaseEntry> fKeys)
            throws SDatabaseException {
        SCursorConfig cursorConfig = new SCursorConfig().setBulkCursor(true);
        this.fkNullifiers.keySet().forEach(sdb -> {
            Map<SDatabaseEntry, SDatabaseEntry> nullifiedData;
            try (SSecondaryCursor cursor = sdb.openCursor(txn, cursorConfig)) {
                nullifiedData = calculateNullifiedData(cursor, fKeys);
            }
            try (SCursor cursor = sdb.getPrimaryDatabase()
                    .openCursor(txn, new SCursorConfig())) {
                nullifiedData.forEach(cursor::putKeyFirst);
            }
        });
    }

    private Map<SDatabaseEntry, SDatabaseEntry> calculateNullifiedData(
            SSecondaryCursor sCursor, List<SDatabaseEntry> fKeys)
            throws SDatabaseException {
        SSecondaryDatabase sdb = sCursor.getDatabase();
        Map<SDatabaseEntry, SDatabaseEntry> nullifiedData = new HashMap<>();

        fKeys.forEach(fKey -> {
            SDatabaseEntry pKey = new SDatabaseEntry();
            SDatabaseEntry pData = new SDatabaseEntry();

            SOperationStatus status =
                    sCursor.getSearchKey(fKey, pKey, pData, RMW);
            while (status == SOperationStatus.SUCCESS) {
                pData = nullifiedData.getOrDefault(pKey, pData);
                if (this.fkNullifiers.get(sdb)
                        .nullifyForeignKey(sdb, pKey, pData, fKey)) {
                    nullifiedData.put(pKey.deepCopy(), pData.deepCopy());
                }
                status = sCursor.getNextDup(fKey, pKey, pData, RMW);
            }
        });

        return nullifiedData;
    }

    private SOperationStatus remoteDelete(STransaction txn,
            List<TKeyData> pairs) throws SDatabaseException {
        return remoteCall(() -> SOperationStatus.toBdb(this.client.dbDelete(
                this.tDb, STransaction.nullSafeGet(txn), pairs)));
    }

    /**
     * Return a cursor into the database.
     *
     * @param txn To use a cursor for writing to a transactional database, an
     * explicit transaction must be specified. For read-only access to a
     * transactional database, the transaction may be null.
     * To transaction-protect cursor operations, cursors must be opened and
     * closed within the context of a transaction, and the txn parameter
     * specifies the transaction context in which the cursor will be used.
     * @param config the cursor attributes; if null, default attributes are
     * used
     * @return a database cursor
     * @throws SDatabaseException if any error occurs
     */
    public SCursor openCursor(STransaction txn, SCursorConfig config)
            throws SDatabaseException {
        return remoteCall(() -> {
            TCursor cursor = this.client
                    .openCursor(this.tDb, STransaction.nullSafeGet(txn),
                            SCursorConfig.nullSafeGet(config));
            return new SCursor(cursor, this, txn, this.client);
        });
    }

    /**
     * Creates a specialized join cursor for use in performing equality or
     * natural joins on secondary indices.
     * <p>
     * Each cursor in the cursors array must have been initialized to refer to
     * the key on which the underlying database should be joined. Typically,
     * this initialization is done by calling {@link SCursor#getSearchKey}.
     * <p>
     * Once the cursors have been passed to this method, they should not be
     * accessed or modified until the newly created join cursor has been
     * closed, or else inconsistent results may be returned. However, the
     * position of the cursors will not be changed by this method or by the
     * methods of the join cursor.
     *
     * @param cursors an array of cursors associated with this primary database
     * @param config the join attributes; if null, default attributes are used
     * @return a specialized cursor that returns the results of the equality
     * join operation
     * @throws SDatabaseException if any error occurs
     */
    public SJoinCursor join(SCursor[] cursors, SJoinConfig config)
            throws SDatabaseException {
        return remoteCall(() -> {
            List<TCursor> tCursors = Arrays.stream(cursors)
                    .map(c -> c.tCursor)
                    .collect(Collectors.toList());
            TJoinCursor joinCursor = this.client.openJoinCursor(this.tDb,
                    tCursors, (config == null || !config.getNoSort()));
            return new SJoinCursor(joinCursor, config, this, this.client);
        });
    }

    /**
     * Open a sequence represented by the key in the database.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key The key {@link SDatabaseEntry} of the sequence. It specifies
     * which record in the database stores the persistent sequence data.
     * @param config The sequence attributes; If null, default attributes are
     * used
     * @return a sequence handle
     * @throws SDatabaseException if any error occurs
     */
    public SSequence openSequence(STransaction txn, SDatabaseEntry key,
            SSequenceConfig config) throws SDatabaseException {
        return remoteCall(() -> {
            TSequence sequence = this.client
                    .openSequence(this.tDb, STransaction.nullSafeGet(txn),
                            key.getThriftObj(), config.getThriftObj());
            return new SSequence(sequence, this.client, this, key);
        });
    }

    /**
     * Remove the sequence from the database. This method should not be called
     * if there are open handles on this sequence.
     * <p>
     * If {@code force} is set to true, all open handles for the sequence are
     * closed so that the remove can proceed. If {@code force} is
     * false, and there is at least one open handle for the sequence, {@link
     * SResourceInUseException} is thrown.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param key The key {@link SDatabaseEntry} of the sequence. It specifies
     * which record in the database stores the persistent sequence data.
     * @param autoCommitNoSync if true, configure auto-commit operations on the
     * sequence to not flush the transaction log
     * @param force if true all open handles on the sequence are closed and the
     * sequence is removed; if false the sequence is removed only if there is
     * no open handle on it
     * @throws SDatabaseException if any error occurs
     */
    public void removeSequence(STransaction txn, SDatabaseEntry key,
            boolean autoCommitNoSync, boolean force) throws SDatabaseException {
        remoteCall(() -> {
            this.client.removeSequence(this.tDb, STransaction.nullSafeGet(txn),
                    key.getThriftObj(), autoCommitNoSync, force);
            return null;
        });
    }

    /**
     * Compact a Btree or Recno database or returns unused Btree, Hash or Recno
     * database pages to the underlying filesystem.
     *
     * @param txn if the operation is part of an application-specified
     * transaction, the txn parameter is a transaction handle returned from
     * {@link SEnvironment#beginTransaction}, otherwise {@code NULL}.
     * If a transaction handle is supplied to this method, then the operation
     * is performed using that transaction. In this event, large sections of
     * the tree may be locked during the course of the transaction.
     * If no transaction handle is specified, the operation will be implicitly
     * transaction protected using multiple transactions. These transactions
     * will be periodically committed to avoid locking large sections of the
     * tree. Any deadlocks encountered cause the compaction operation to be
     * retried from the point of the last transaction commit.
     * @param start if not null, the start parameter is the starting point for
     * compaction in a Btree or Recno database. Compaction will start at the
     * smallest key greater than or equal to the specified key. If null,
     * compaction will start at the beginning of the database.
     * @param stop if not null, the stop parameter is the stopping point for
     * compaction in a Btree or Recno database. Compaction will stop at the
     * page with the smallest key greater than the specified key. If null,
     * compaction will stop at the end of the database.
     * @param end if not null, the end parameter will be filled in with the key
     * marking the end of the compaction operation in a Btree or Recno
     * database. It is generally the first key of the page where processing
     * stopped.
     * @param config the compaction operation attributes; if null, default
     * attributes are used
     * @return compaction operation statistics
     * @throws SDatabaseException if any error occurs
     */
    public SCompactStats compact(STransaction txn, SDatabaseEntry start,
            SDatabaseEntry stop, SDatabaseEntry end, SCompactConfig config)
            throws SDatabaseException {
        return remoteCall(() -> {
            TCompactResult result = this.client.dbCompact(this.tDb,
                    STransaction.nullSafeGet(txn), start.getThriftObj(),
                    stop.getThriftObj(), SCompactConfig.nullSafeGet(config));
            if (end != null) {
                end.setDataFromTDbt(result.endKey);
            }
            return new SCompactStats(result);
        });
    }

    /**
     * Empty the database, discarding all records it contains.
     * <p>
     * When called on a database configured with secondary indices, this method
     * truncates the primary database and all secondary indices. If configured
     * to return a count of the records discarded, the returned count is the
     * count of records discarded from the primary database.
     * <p>
     * It is an error to call this method on a database with open cursors.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param countRecords if true, count and return the number of records
     * discarded
     * @return the number of records discarded, or -1 if {@code countRecords} is
     * false
     * @throws SDatabaseException if any error occurs
     */
    public int truncate(STransaction txn, boolean countRecords)
            throws SDatabaseException {
        return remoteCall(() -> this.client.dbTruncate(this.tDb,
                STransaction.nullSafeGet(txn), countRecords));
    }

    /**
     * Return database statistics.
     * <p>
     * If this method has not been configured to avoid expensive operations
     * (using the {@link SStatsConfig#setFast} method), it will access some of
     * or all the pages in the database, incurring a severe performance penalty
     * as well as possibly flushing the underlying buffer pool.
     * <p>
     * In the presence of multiple threads or processes accessing an active
     * database, the information returned by this method may be out-of-date.
     * <p>
     * If the database was not opened read-only and this method was not
     * configured using the {@link SStatsConfig#setFast} method, cached key and
     * record numbers will be updated after the statistical information has
     * been gathered.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param config the statistics returned; if null, default statistics are
     * returned
     * @return database statistics
     * @throws SDatabaseException if any error occurs
     */
    public SDatabaseStats getStats(STransaction txn, SStatsConfig config)
            throws SDatabaseException {
        return remoteCall(() -> {
            TDatabaseStatResult result = this.client.getDatabaseStatistics(
                    this.tDb, STransaction.nullSafeGet(txn),
                    SStatsConfig.nullSafeGet(config).getFast());
            if (result.isSetBtreeStat())
                return new SBtreeStats(result.btreeStat);
            if (result.isSetHashStat())
                return new SHashStats(result.hashStat);
            if (result.isSetHeapStat())
                return new SHeapStats(result.heapStat);
            if (result.isSetQueueStat())
                return new SQueueStats(result.queueStat);
            else
                throw new RuntimeException("no stats available.");
        });
    }
}
