/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TTransaction;

import java.io.IOException;
import java.nio.ByteOrder;
import java.util.Objects;

/**
 * A database environment. Environments include support for all of caching,
 * locking, logging and transactions.
 * <p>
 * To open an existing environment with default attributes the application may
 * use a default environment configuration object or null:
 * <pre>
 *  // Open an environment handle with default attributes.
 *  BdbServerConnection conn = BdbServerConnection.connect(host, port);
 *  SEnvironment env = conn.openEnvironment(home, new SEnvironmentConfig());
 * </pre>
 * or
 * <pre>
 *  SEnvironment env = conn.openEnvironment(home, null);
 * </pre>
 * Note that many SEnvironment objects may access a single environment.
 * <p>
 * To create an environment or customize attributes, the application should
 * customize the configuration class. For example:
 * <pre>
 *  SEnvironmentConfig envConfig = new SEnvironmentConfig();
 *  envConfig.setAllowCreate(true).setCacheSize(1000000);
 *  ...
 *  SEnvironment env = conn.openEnvironment(home, envConfig);
 * </pre>
 * An environment handle is an SEnvironment instance. More than one
 * SEnvironment instance may be created for the same physical directory, which
 * is the same as saying that more than one SEnvironment handle may be open at
 * one time for a given environment.
 * <p>
 * The SEnvironment handle should not be closed while any other handle remains
 * open that is using it as a reference (for example, SDatabase or
 * STransaction). Once SEnvironment.close is called, this object may not be
 * accessed again, regardless of whether or not it throws an exception.
 */
public class SEnvironment
        implements RemoteCallHelper, TxnHelper, AutoCloseable {
    /** The remote environment handle. */
    private final TEnvironment tEnv;

    /** The home directory. */
    private final String home;

    /** The server connection. */
    private BdbServerConnection connection;

    /** The remote service client. */
    private final BdbService.Client client;

    SEnvironment(TEnvironment environment, String home,
            BdbServerConnection connection, BdbService.Client client) {
        this.tEnv = Objects.requireNonNull(environment);
        this.home = Objects.requireNonNull(home);
        this.connection = Objects.requireNonNull(connection);
        this.client = Objects.requireNonNull(client);
    }

    /**
     * Create a shallow copy of the given environment. The original environment
     * handle must not be used anymore after this method returns. This
     * constructor is reserved for tests.
     *
     * @param env the environment
     */
    protected SEnvironment(SEnvironment env) {
        this.tEnv = env.tEnv;
        this.home = env.home;
        this.client = env.client;
    }

    /**
     * Close the database environment, freeing any allocated resources and
     * closing any underlying subsystems.
     * <p>
     * When you call this method, all open database and cursor handles that
     * depend on this are closed automatically, and should not be reused.
     * <p>
     * The SEnvironment handle should not be closed while any other handle that
     * refers to it is not yet closed.
     * <p>
     * Closing the environment aborts any unresolved transactions. Applications
     * should not depend on this behavior for transactions involving databases;
     * all such transactions should be explicitly resolved. The problem with
     * depending on this semantic is that aborting an unresolved transaction
     * involving database operations requires a database handle. Because the
     * database handles should have been closed before closing the environment,
     * it will not be possible to abort the transaction, and recovery will have
     * to be run on the database environment before further operations are
     * done.
     * <p>
     * After this method has been called, regardless of its return, the
     * SEnvironment handle may not be accessed again.
     *
     * @throws SDatabaseException if any error occurs
     */
    @Override
    public void close() throws SDatabaseException {
        remoteCall(() -> {
            this.client.closeEnvironmentHandle(this.tEnv);
            return null;
        });
    }

    /**
     * Return this environment.
     *
     * @return this environment
     */
    @Override
    public SEnvironment getEnvironment() {
        return this;
    }

    /**
     * Return the native byte order of the connected server.
     *
     * @return the native byte order of the connected server
     */
    public ByteOrder getServerByteOrder() {
        return this.connection.getServerByteOrder();
    }

    /**
     * Return this object's configuration.
     *
     * @return this object's configuration
     * @throws SDatabaseException if any error occurs
     */
    public SEnvironmentConfig getConfig() throws SDatabaseException {
        return remoteCall(() -> new SEnvironmentConfig(
                this.client.getEnvironmentConfig(this.tEnv)));
    }

    /**
     * Change the settings in an existing environment handle. Only set
     * attributes are changed. Therefore, there is no need to call {@link
     * #getConfig()} to get the current setting because unset attributes are
     * not changed.
     *
     * @param config the database environment attributes; if null, no attribute
     * is changed
     * @throws SDatabaseException if any error occurs
     */
    public void setConfig(final SEnvironmentConfig config)
            throws SDatabaseException {
        remoteCall(() -> {
            this.client.setEnvironmentConfig(this.tEnv,
                    ThriftWrapper.nullSafeGet(config));
            return null;
        });
    }

    /**
     * Return the database environment home directory. This directory is
     * normally identified in {@link BdbServerConnection#openEnvironment(String,
     * SEnvironmentConfig)}.
     *
     * @return the database environment home directory
     */
    public String getHome() {
        return this.home;
    }

    /**
     * Open a database.
     * <p>
     * The database is represented by the file and database parameters.
     * <p>
     * The currently supported database file formats (or access methods) are
     * Btree, Hash, Queue, and Recno. The Btree format is a representation of a
     * sorted, balanced tree structure. The Hash format is an extensible,
     * dynamic hashing scheme. The Queue format supports fast access to
     * fixed-length records accessed sequentially or by logical record number.
     * The Recno format supports fixed- or variable-length records, accessed
     * sequentially or by logical record number.
     * <p>
     * Storage and retrieval are based on key/data pairs; see {@link
     * SDatabaseEntry} for more information.
     * <p>
     * Opening a database is a relatively expensive operation, and maintaining
     * a set of open databases will normally be preferable to repeatedly
     * opening and closing the database for each new query.
     *
     * @param txn for a transactional database, an explicit transaction may be
     * specified, or null may be specified to use auto-commit
     * @param fileName the name of an underlying file that will be used to back
     * the database
     * @param databaseName an optional parameter that allows applications to
     * have multiple databases in a single file. Although no databaseName
     * parameter needs to be specified, it is an error to attempt to open a
     * second database in a physical file that was not initially created using
     * a databaseName parameter; further, the databaseName parameter is not
     * supported by the Queue format
     * @param config the database open attributes; if null, default attributes
     * are used.
     * @return a new database handle
     * @throws IOException if the database file cannot be accessed
     * @throws SResourceInUseException if the database is being renamed or
     * removed
     * @throws SDatabaseException if any error occurs
     */
    public SDatabase openDatabase(STransaction txn, String fileName,
            String databaseName, SDatabaseConfig config) throws
            IOException, SDatabaseException {
        return remoteCallWithIOException(() -> {
            TDatabase db = this.client.openDatabase(this.tEnv,
                    STransaction.nullSafeGet(txn),
                    fileName, databaseName,
                    ThriftWrapper.nullSafeGet(config));
            return new SDatabase(db, fileName, databaseName, this.client, this);
        });
    }

    /**
     * Open a secondary database.
     * <p>
     * The database is represented by the file and database parameters.
     * <p>
     * The currently supported database file formats (or access methods) are
     * Btree, Hash, Queue, and Recno. The Btree format is a representation of a
     * sorted, balanced tree structure. The Hash format is an extensible,
     * dynamic hashing scheme. The Queue format supports fast access to
     * fixed-length records accessed sequentially or by logical record number.
     * The Recno format supports fixed- or variable-length records, accessed
     * sequentially or by logical record number.
     * <p>
     * Storage and retrieval are based on key/data pairs; see {@link
     * SDatabaseEntry} for more information.
     * <p>
     * Opening a database is a relatively expensive operation, and maintaining
     * a set of open databases will normally be preferable to repeatedly
     * opening and closing the database for each new query.
     *
     * @param txn for a transactional database, an explicit transaction may be
     * specified, or null may be specified to use auto-commit
     * @param fileName the name of an underlying file that will be used to back
     * the database
     * @param databaseName an optional parameter that allows applications to
     * have multiple databases in a single file. Although no databaseName
     * parameter needs to be specified, it is an error to attempt to open a
     * second database in a physical file that was not initially created using
     * a databaseName parameter; further, the databaseName parameter is not
     * supported by the Queue format
     * @param primaryDatabase a database handle for the primary database that
     * is to be indexed
     * @param config the secondary database open attributes; if null, default
     * attributes are used
     * @return a new secondary database handle
     * @throws IOException if the database file cannot be accessed
     * @throws SResourceInUseException if the database is being renamed or
     * removed
     * @throws SDatabaseException if any error occurs
     */
    public SSecondaryDatabase openSecondaryDatabase(STransaction txn,
            String fileName, String databaseName, SDatabase primaryDatabase,
            SSecondaryConfig config) throws IOException, SDatabaseException {
        return runInSingleTxnWithIOException(txn, autoTxn -> {
            SSecondaryDatabase sdb = remoteOpenSecondary(autoTxn,
                    fileName, databaseName, primaryDatabase, config);

            if (config.getAllowPopulate() && sdb.isEmpty(autoTxn)) {
                populateSecondaryDatabase(autoTxn, primaryDatabase);
            }

            return sdb;
        });
    }

    private SSecondaryDatabase remoteOpenSecondary(STransaction txn,
            String fileName, String dbName, SDatabase pDb,
            SSecondaryConfig config) throws IOException, SDatabaseException {
        return remoteCallWithIOException(() -> {
            TDatabase sdb = this.client.openSecondaryDatabase(this.tEnv,
                    STransaction.nullSafeGet(txn),
                    fileName, dbName, pDb.getThriftObj(),
                    SSecondaryConfig.nullSafeGet(config));
            return new SSecondaryDatabase(sdb, fileName, dbName,
                    this.client, this, pDb, new SSecondaryConfig(config));
        });
    }

    private void populateSecondaryDatabase(STransaction txn,
            SDatabase primaryDatabase) throws SDatabaseException {
        final int batchSize = 1024;
        try (SCursor cursor = primaryDatabase.openCursor(txn, null)) {
            SMultipleKeyDataEntry multiEntry = new SMultipleKeyDataEntry();
            multiEntry.setBatchSize(batchSize);
            while (cursor.getNext(null, multiEntry, null) !=
                    SOperationStatus.NOTFOUND) {
                primaryDatabase.putMultipleKey(txn, multiEntry, true);
            }
        }
    }

    /**
     * Remove the database specified by the fileName and databaseName
     * parameters.
     * <p>
     * If no database is specified, the underlying file specified is removed,
     * incidentally removing all of the databases it contained.
     * <p>
     * Applications should never remove databases with open {@link SDatabase}
     * handles, or in the case of removing a file, when any database in the
     * file has an open handle.
     * <p>
     * If {@code force} is set to true, all open handles for the database, or
     * in the case of removing a file, all open handles for any database in the
     * file are closed so that the remove can proceed. If {@code force} is
     * false, and there is at least one open handle for the database, {@link
     * SResourceInUseException} is thrown.
     *
     * @param txn if the operation is part of an application-specified
     * transaction, the txn parameter is a {@link STransaction} object returned
     * from the {@link SEnvironment#beginTransaction} method; otherwise null.
     * For a transactional database, an explicit transaction may be specified,
     * or null may be specified to use auto-commit.
     * @param fileName the physical file which contains the database to be
     * removed
     * @param databaseName the database to be removed
     * @param force if true, open handles are closed as necessary to allow the
     * remove operation to proceed; otherwise, throw {@link
     * SResourceInUseException} if the database has at least one open handle
     * @throws IOException the database file cannot be accessed
     * @throws SResourceInUseException if {@code force} is false and there is
     * at
     * least one open handle for the database
     * @throws SDatabaseException if any error occurs
     */
    public void removeDatabase(STransaction txn, String fileName,
            String databaseName, boolean force)
            throws IOException, SDatabaseException {
        remoteCallWithIOException(() -> {
            this.client.removeDatabase(this.tEnv, STransaction.nullSafeGet(txn),
                    fileName, databaseName, force);
            return null;
        });
    }

    /**
     * Rename a database.
     * <p>
     * If no database name is specified, the underlying file specified is
     * renamed, incidentally renaming all of the databases it contains.
     * <p>
     * Applications should never rename databases that are currently in use. If
     * an underlying file is being renamed and logging is currently enabled in
     * the database environment, no database in the file may be open when this
     * method is called.
     * <p>
     * If {@code force} is set to true, all open handles for the database, or
     * in the case of removing a file, all open handles for any database in the
     * file are closed so that the rename can proceed. If {@code force} is
     * false, and there is at least one open handle for the database, {@link
     * SResourceInUseException} is thrown.
     *
     * @param txn if the operation is part of an application-specified
     * transaction, the txn parameter is a {@link STransaction} object returned
     * from the {@link SEnvironment#beginTransaction} method; otherwise null.
     * For a transactional database, an explicit transaction may be specified,
     * or null may be specified to use auto-commit.
     * @param fileName the physical file which contains the database to be
     * renamed
     * @param databaseName the database to be renamed
     * @param newName the new name of the database or file
     * @param force if true, open handles are closed as necessary to allow the
     * remove operation to proceed; otherwise, throw {@link
     * SResourceInUseException} if the database has at least one open handle
     * @throws IOException the database file cannot be accessed
     * @throws SResourceInUseException if {@code force} is false and there is
     * at
     * least one open handle for the database
     * @throws SDatabaseException if any error occurs
     */
    public void renameDatabase(STransaction txn, String fileName,
            String databaseName, String newName, boolean force)
            throws IOException, SDatabaseException {
        remoteCallWithIOException(() -> {
            this.client.renameDatabase(this.tEnv, STransaction.nullSafeGet(txn),
                    fileName, databaseName, newName, force);
            return null;
        });
    }

    /**
     * Create a new transaction in the database environment.
     * <p>
     * A parent transaction may not issue any Berkeley DB driver operations --
     * except for {@link #beginTransaction}, {@link STransaction#abort} and
     * {@link STransaction#commit} -- while it has active child transactions
     * (child transactions that have not yet been committed or aborted).
     *
     * @param parent if the parent parameter is non-null, the new transaction
     * will be a nested transaction, with the transaction indicated by parent
     * as its parent; transactions may be nested to any level
     * @param config the transaction attributes; if null, default attributes
     * are
     * used.
     * @return the newly created transaction's handle
     * @throws SDatabaseException if any error occurs
     */
    public STransaction beginTransaction(STransaction parent,
            STransactionConfig config) throws SDatabaseException {
        return remoteCall(() -> {
            TTransaction txn = this.client.beginTransaction(this.tEnv,
                    STransaction.nullSafeGet(parent),
                    ThriftWrapper.nullSafeGet(config));
            return new STransaction(txn, this.client);
        });
    }

    /**
     * Synchronously checkpoint the database environment.
     *
     * @param config the checkpoint attributes; if null, default attributes are
     * used.
     * @throws SDatabaseException if any error occurs
     */
    public void checkpoint(SCheckpointConfig config) throws SDatabaseException {
        SCheckpointConfig c = config == null ? new SCheckpointConfig() : config;
        remoteCall(() -> {
            this.client.checkpoint(this.tEnv, c.getKBytes(), c.getMinutes(),
                    c.getForce());
            return null;
        });
    }

    /**
     * Return statistics for individual files in the cache.
     *
     * @param config the statistics attributes; if null, default attributes are
     * used.
     * @return statistics for individual files in the cache
     * @throws SDatabaseException if any error occurs
     */
    public SCacheFileStats[] getCacheFileStats(SStatsConfig config)
            throws SDatabaseException {
        return getMultipleStats(
                new SMultiStatsConfig().setCacheFileConfig(true, config))
                .getCacheFileStats();
    }

    /**
     * Returns the memory pool (that is, the buffer cache) subsystem
     * statistics.
     *
     * @param config the statistics attributes
     * @return the memory pool (that is, the buffer cache) subsystem statistics
     * @throws SDatabaseException if any error occurs
     */
    public SCacheStats getCacheStats(SStatsConfig config)
            throws SDatabaseException {
        return getMultipleStats(
                new SMultiStatsConfig().setCacheConfig(true, config))
                .getCacheStats();
    }

    /**
     * Return the database environment's locking statistics.
     *
     * @param config the statistics attributes
     * @return the database environment's locking statistics
     * @throws SDatabaseException if any error occurs
     */
    public SLockStats getLockStats(SStatsConfig config)
            throws SDatabaseException {
        return getMultipleStats(
                new SMultiStatsConfig().setLockConfig(true, config))
                .getLockStats();
    }

    /**
     * Return the database environment's logging statistics.
     *
     * @param config the statistics attributes
     * @return the database environment's logging statistics
     * @throws SDatabaseException if any error occurs
     */
    public SLogStats getLogStats(SStatsConfig config)
            throws SDatabaseException {
        return getMultipleStats(
                new SMultiStatsConfig().setLogConfig(true, config))
                .getLogStats();
    }

    /**
     * Return the database environment's mutex statistics.
     *
     * @param config the statistics attributes
     * @return the database environment's mutex statistics
     * @throws SDatabaseException if any error occurs
     */
    public SMutexStats getMutexStats(SStatsConfig config)
            throws SDatabaseException {
        return getMultipleStats(
                new SMultiStatsConfig().setMutexConfig(true, config))
                .getMutexStats();
    }

    /**
     * Return the database environment's transactional statistics.
     *
     * @param config the statistics attributes
     * @return the database environment's transactional statistics
     * @throws SDatabaseException if any error occurs
     */
    public STransactionStats getTransactionStats(SStatsConfig config)
            throws SDatabaseException {
        return getMultipleStats(
                new SMultiStatsConfig().setTransactionConfig(true, config))
                .getTransactionStats();
    }

    /**
     * Return multiple database environment's statistics.
     * <p>
     * The {@code config} parameter specifies which statistics are retrieved
     * and what attributes apply for each statistics retrieval operation.
     * <p>
     * Compared to other statistics retrieval operations that retrieve only a
     * single statistics, this method can save a few network round trips when
     * multiple statistics are retrieved.
     *
     * @param config the multiple statistics attributes
     * @return the selected database environment's statistics
     * @throws SDatabaseException if any error occurs
     */
    public SMultiStats getMultipleStats(SMultiStatsConfig config)
            throws SDatabaseException {
        return remoteCall(() -> new SMultiStats(this.client
                .getEnvStatistics(this.tEnv, config.getThriftObj())));
    }
}
