/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server;

import com.sleepycat.db.CacheFileStats;
import com.sleepycat.db.CheckpointConfig;
import com.sleepycat.db.CompactConfig;
import com.sleepycat.db.Cursor;
import com.sleepycat.db.CursorConfig;
import com.sleepycat.db.Database;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DatabaseType;
import com.sleepycat.db.Environment;
import com.sleepycat.db.EnvironmentConfig;
import com.sleepycat.db.ForeignKeyDeleteAction;
import com.sleepycat.db.JoinConfig;
import com.sleepycat.db.JoinCursor;
import com.sleepycat.db.KeyRange;
import com.sleepycat.db.LockDetectMode;
import com.sleepycat.db.LockMode;
import com.sleepycat.db.MemoryException;
import com.sleepycat.db.MultipleDataEntry;
import com.sleepycat.db.MultipleEntry;
import com.sleepycat.db.MultipleKeyDataEntry;
import com.sleepycat.db.MultipleRecnoDataEntry;
import com.sleepycat.db.OperationStatus;
import com.sleepycat.db.SecondaryConfig;
import com.sleepycat.db.SecondaryCursor;
import com.sleepycat.db.SecondaryDatabase;
import com.sleepycat.db.Sequence;
import com.sleepycat.db.SequenceConfig;
import com.sleepycat.db.StatsConfig;
import com.sleepycat.db.Transaction;
import com.sleepycat.db.TransactionConfig;
import com.sleepycat.server.callbacks.ServerKeyCreator;
import com.sleepycat.server.config.BdbServiceConfig;
import com.sleepycat.server.config.EnvDirType;
import com.sleepycat.server.function.BiFunctionEx;
import com.sleepycat.server.function.FunctionEx;
import com.sleepycat.server.function.SupplierEx;
import com.sleepycat.server.handle.CursorDescriptor;
import com.sleepycat.server.handle.DatabaseDescriptor;
import com.sleepycat.server.handle.DatabaseFileKey;
import com.sleepycat.server.handle.DatabaseKey;
import com.sleepycat.server.handle.EnvironmentDescriptor;
import com.sleepycat.server.handle.EnvironmentKey;
import com.sleepycat.server.handle.HandleDescriptor;
import com.sleepycat.server.handle.HandleManager;
import com.sleepycat.server.handle.JoinCursorDescriptor;
import com.sleepycat.server.handle.ResourceKey;
import com.sleepycat.server.handle.ResourceMembers;
import com.sleepycat.server.handle.SecondaryDatabaseDescriptor;
import com.sleepycat.server.handle.SequenceDescriptor;
import com.sleepycat.server.handle.SequenceKey;
import com.sleepycat.server.handle.TransactionDescriptor;
import com.sleepycat.server.handle.TransactionDescriptor.Status;
import com.sleepycat.server.util.Adapters;
import com.sleepycat.server.util.DelArgs;
import com.sleepycat.server.util.FileUtils;
import com.sleepycat.server.util.GetArgs;
import com.sleepycat.server.util.KeyDataPair;
import com.sleepycat.server.util.PutArgs;
import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TCachePriority;
import com.sleepycat.thrift.TCompactConfig;
import com.sleepycat.thrift.TCompactResult;
import com.sleepycat.thrift.TCursor;
import com.sleepycat.thrift.TCursorConfig;
import com.sleepycat.thrift.TCursorGetConfig;
import com.sleepycat.thrift.TCursorGetMode;
import com.sleepycat.thrift.TCursorPutConfig;
import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TDatabaseConfig;
import com.sleepycat.thrift.TDatabaseStatResult;
import com.sleepycat.thrift.TDbGetConfig;
import com.sleepycat.thrift.TDbGetMode;
import com.sleepycat.thrift.TDbPutConfig;
import com.sleepycat.thrift.TDbt;
import com.sleepycat.thrift.TDurabilityPolicy;
import com.sleepycat.thrift.TEnvStatConfig;
import com.sleepycat.thrift.TEnvStatOption;
import com.sleepycat.thrift.TEnvStatResult;
import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TEnvironmentConfig;
import com.sleepycat.thrift.TGetResult;
import com.sleepycat.thrift.TGetWithPKeyResult;
import com.sleepycat.thrift.TJoinCursor;
import com.sleepycat.thrift.TJoinCursorGetConfig;
import com.sleepycat.thrift.TKeyData;
import com.sleepycat.thrift.TKeyDataWithPKey;
import com.sleepycat.thrift.TKeyDataWithSecondaryKeys;
import com.sleepycat.thrift.TKeyRangeResult;
import com.sleepycat.thrift.TOperationStatus;
import com.sleepycat.thrift.TProtocolVersionTestResult;
import com.sleepycat.thrift.TPutResult;
import com.sleepycat.thrift.TResourceInUseException;
import com.sleepycat.thrift.TSecondaryDatabaseConfig;
import com.sleepycat.thrift.TSequence;
import com.sleepycat.thrift.TSequenceConfig;
import com.sleepycat.thrift.TTransaction;
import com.sleepycat.thrift.TTransactionConfig;
import com.sleepycat.thrift.dbConstants;
import org.apache.thrift.TException;
import org.apache.thrift.server.TServer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.function.Supplier;
import java.util.stream.Collectors;

/**
 * The implementation of BdbService.
 */
@SuppressWarnings("ThrowableResultOfMethodCallIgnored")
public class BdbServiceHandler implements BdbService.Iface {
    private static final Logger logger =
            LoggerFactory.getLogger(BdbServiceHandler.class);
    /** The initial buffer size for a MultipleEntry. */
    private static final int INIT_BUFFER_SIZE = 4 * 1024 * 1024;
    /** The minimum buffer size for a MultipleEntry. */
    private static final int MIN_BUFFER_SIZE = 64 * 1024;
    /** The handle manager. */
    private final HandleManager handleManager = new HandleManager();
    /** The BDB server. */
    private TServer server = null;
    /** The server configuration. */
    private BdbServiceConfig config;
    /** If the service has been shut down. */
    private volatile boolean shutdown = false;
    /** The executor service for the cleanup worker. */
    private ScheduledExecutorService cleanupWorker = null;

    /**
     * Set the server hosting this service, and the configuration.
     *
     * @param server the hosting server
     * @param config the service configuration
     */
    public void setServerAndConfig(TServer server, BdbServiceConfig config) {
        this.server = Objects.requireNonNull(server, "server cannot be null.");
        this.config = Objects.requireNonNull(config, "config cannot be null.");

        int cleanupInterval = this.config.getCleanupIntervalInSeconds();
        this.cleanupWorker = Executors.newScheduledThreadPool(1);
        this.cleanupWorker.scheduleWithFixedDelay(() -> this.handleManager
                        .closeInactiveHandles(
                                this.config.getHandleTimeoutInSeconds()),
                cleanupInterval, cleanupInterval, TimeUnit.SECONDS);
    }

    @Override
    public void ping() {
    }

    @Override
    public TProtocolVersionTestResult isProtocolVersionSupported(
            String clientVersion) {
        String serverVersion = dbConstants.PROTOCOL_VERSION;
        boolean supported = clientVersion.equals(serverVersion);
        return new TProtocolVersionTestResult(supported, serverVersion,
                ByteOrder.nativeOrder().equals(ByteOrder.BIG_ENDIAN));
    }

    @Override
    public String getBdbVersion() {
        return Environment.getVersionString();
    }

    @Override
    public void shutdown() {
        if (!shutdown) {
            System.out.println("Shutting down the server...");
            this.shutdown = true;
            this.cleanupWorker.shutdown();
            this.server.stop();
            this.handleManager.shutdown();
            System.out.println("The server has been shut down successfully.");
        }
    }

    @Override
    public TEnvironment openEnvironment(String homeDir,
            TEnvironmentConfig envConfig) throws TException {
        ResourceMembers envRes = null;
        boolean runRecovery = false;
        try {
            EnvironmentConfig conf = Adapters.toBdbType(envConfig);

            Map<EnvDirType, File> rootDirs = this.config.getRootDirs();
            Map<EnvDirType, File> envDirs = FileUtils.resolveDirectories(
                    rootDirs, homeDir, conf.getAllowCreate());

            File envHome = envDirs.get(EnvDirType.HOME);
            EnvironmentKey envKey = new EnvironmentKey(envHome);

            if (conf.getRunRecovery()) {
                runRecovery = true;
                envRes = this.handleManager.writeLockResource(envKey);
                this.handleManager.closeHandles(envRes.getMembers());
            } else {
                envRes = readLock(envKey,
                        "The environment is being recovered or deleted.");
            }

            addTransactionalSupport(conf);
            configureDirs(conf, envDirs);

            Environment env = new Environment(envHome, conf);
            HandleDescriptor descriptor = this.handleManager
                    .register(new EnvironmentDescriptor(env, envKey));

            return new TEnvironment(descriptor.getId());
        } catch (Exception e) {
            throw error(buildLogMsg("openEnvironment", homeDir, envConfig), e);
        } finally {
            if (runRecovery) {
                this.handleManager.unlockWrite(envRes);
            } else {
                this.handleManager.unlockRead(envRes);
            }
        }
    }

    private void addTransactionalSupport(EnvironmentConfig envConfig) {
        envConfig.setInitializeCache(true);
        envConfig.setInitializeLocking(true);
        envConfig.setInitializeLogging(true);
        envConfig.setThreaded(true);
        envConfig.setTransactional(true);
        if (envConfig.getLockDetectMode() == LockDetectMode.NONE ||
                envConfig.getLockDetectMode() == LockDetectMode.DEFAULT) {
            envConfig.setLockDetectMode(LockDetectMode.RANDOM);
        }
    }

    private void configureDirs(EnvironmentConfig envConfig,
            Map<EnvDirType, File> envDirs) throws IOException {
        if (envConfig.getAllowCreate()) {
            FileUtils.createDirectories(envDirs.values());
        }
        envDirs.forEach((type, dir) -> {
            switch (type) {
                case DATA:
                    envConfig.addDataDir(dir);
                    envConfig.setCreateDir(dir);
                    break;
                case LOG:
                    envConfig.setLogDirectory(dir);
                    break;
                case BLOB:
                    envConfig.setExternalFileDir(dir);
                    break;
            }
        });
    }

    @Override
    public void closeEnvironmentHandle(TEnvironment env) throws TException {
        closeHandle(env.handle, () -> "closeEnvironment(" + env.handle + ")");
    }

    @Override
    public void closeEnvironmentHandles(String homeDir, long minIdleInMilli)
            throws TException {
        closeAllResourceHandles(minIdleInMilli, () -> {
            File envHome = resolveEnvDir(EnvDirType.HOME, homeDir);
            return new EnvironmentKey(envHome);
        }, () -> buildLogMsg("closeAllEnvironmentHandles", homeDir,
                minIdleInMilli));
    }

    @Override
    public void deleteEnvironmentAndDatabases(String homeDir, boolean force)
            throws TException {
        ResourceMembers envRes = null;
        try {
            File envHome = resolveEnvDir(EnvDirType.HOME, homeDir);
            EnvironmentKey envKey = new EnvironmentKey(envHome);
            envRes = this.handleManager.writeLockResource(envKey);

            closeResource(envRes, force, "environment");

            Map<EnvDirType, File> envDirs = FileUtils.resolveDirectories(
                    this.config.getRootDirs(), homeDir, false);
            for (File dir : envDirs.values()) {
                FileUtils.deleteFileTree(dir);
            }
        } catch (Exception e) {
            throw error(buildLogMsg("deleteEnvironmentAndDatabases", homeDir,
                    force), e);
        } finally {
            this.handleManager.unlockWrite(envRes);
        }
    }

    private File resolveEnvDir(EnvDirType type, String homeDir)
            throws FileNotFoundException {
        File rootDir = this.config.getRootDirs().get(type);
        if (rootDir == null) {
            rootDir = this.config.getRootDirs().get(EnvDirType.HOME);
        }
        File envDir = FileUtils.resolveDirectory(rootDir, homeDir);
        if (envDir == null) {
            throw new FileNotFoundException(homeDir);
        }
        return envDir;
    }

    private void closeResource(ResourceMembers members, boolean force,
            String typeName) throws TResourceInUseException {
        if (!members.isEmpty() && !force) {
            throw new TResourceInUseException(
                    "The " + typeName + " is in use.");
        }

        this.handleManager.closeHandles(members.getMembers());
    }

    @Override
    public TEnvironmentConfig getEnvironmentConfig(TEnvironment env)
            throws TException {
        return handleOp(env.handle,
                e -> Adapters.toThriftType(((Environment) e).getConfig()),
                () -> "getEnvironmentConfig(" + env.handle + ")");
    }

    @Override
    public void setEnvironmentConfig(TEnvironment tEnv,
            TEnvironmentConfig envConfig) throws TException {
        handleOp(tEnv.handle, e -> {
            Environment env = (Environment) e;
            env.setConfig(Adapters.update(env.getConfig(), envConfig));
            return null;
        }, () -> buildLogMsg("setEnvironmentConfig", tEnv.handle, envConfig));
    }

    @Override
    public void removeDatabase(TEnvironment env, TTransaction txn,
            String fileName, String databaseName, boolean force)
            throws TException {
        dbResExclusiveOp(env, txn, fileName, databaseName, force,
                (e, t, auxFileName, auxDbName) -> {
                    e.removeDatabase(t, fileName, databaseName);
                    try {
                        e.removeDatabase(t, auxFileName, auxDbName);
                    } catch (FileNotFoundException ex) {
                        // It's OK if the given database doesn't have an
                        // auxiliary database
                    }
                },
                () -> buildLogMsg("removeDatabase", env.handle, txn, fileName,
                        databaseName, force));
    }

    @Override
    public void renameDatabase(TEnvironment env, TTransaction txn,
            String fileName, String databaseName, String newName, boolean force)
            throws TException {
        dbResExclusiveOp(env, txn, fileName, databaseName, force,
                (e, t, auxFileName, auxDbName) -> {
                    e.renameDatabase(t, fileName, databaseName, newName);
                    String auxNewName = newName;
                    if (auxFileName == null || auxDbName == null) {
                        auxNewName = ServerKeyCreator.getAuxiliaryName(newName);
                    }
                    try {
                        e.renameDatabase(t, auxFileName, auxDbName, auxNewName);
                    } catch (FileNotFoundException ex) {
                        // It's OK if the given database doesn't have an
                        // auxiliary database
                    }
                },
                () -> buildLogMsg("renameDatabase", env.handle, txn, fileName,
                        databaseName, newName, force));
    }

    private void dbResExclusiveOp(TEnvironment tEnv, TTransaction tTxn,
            String fileName, String databaseName, boolean force,
            DatabaseResourceOperation op, Supplier<String> errMsg)
            throws TException {
        if (fileName == null && databaseName == null) {
            throw new IllegalArgumentException(
                    "Rename or delete on temporary database is invalid.");
        }
        transactionalOp(tEnv.handle, tTxn, (e, txn) -> {
            ResourceMembers dbFileRes = null;
            ResourceMembers dbRes = null;
            try {
                Environment env = (Environment) e;
                DatabaseKey dbKey =
                        getDatabaseKey(env, fileName, databaseName, false);
                DatabaseFileKey dbFileKey = dbKey.getDatabaseFile();

                if (databaseName == null) {
                    dbFileRes = this.handleManager.writeLockResource(dbFileKey);
                    closeResource(dbFileRes, force, "database file");
                } else {
                    dbFileRes = readLock(dbFileKey,
                            "The database file is being deleted or renamed.");
                    dbRes = this.handleManager.writeLockResource(dbKey);
                    closeResource(dbRes, force, "database");
                }

                DatabaseKey auxKey = ServerKeyCreator.getAuxiliaryDbKey(dbKey);
                DatabaseFileKey auxFileKey = auxKey.getDatabaseFile();
                String auxFileName = auxFileKey.isInMemory() ? null : auxFileKey
                        .getCanonicalPath();
                return runWithAutoTxn(env, txn, autoTxn -> {
                    op.run(env, autoTxn, auxFileName, auxKey.getDatabaseName());
                    return null;
                });
            } finally {
                this.handleManager.unlockWrite(dbRes);
                if (databaseName == null) {
                    this.handleManager.unlockWrite(dbFileRes);
                } else {
                    this.handleManager.unlockRead(dbFileRes);
                }
            }
        }, errMsg);
    }

    private <T> T runWithAutoTxn(Environment env, Transaction userTxn,
            FunctionEx<Transaction, T> op) throws Exception {
        Transaction autoTxn = userTxn;
        if (autoTxn == null) {
            autoTxn = env.beginTransaction(null, null);
        }

        T result;
        try {
            result = op.applyWithException(autoTxn);
        } catch (Exception e) {
            if (userTxn == null) {
                autoTxn.abort();
            }
            throw e;
        }
        if (userTxn == null) {
            autoTxn.commit();
        }
        return result;
    }

    @Override
    public void checkpoint(TEnvironment env, int kBytes, int min, boolean force)
            throws TException {
        handleOp(env.handle, e -> {
            CheckpointConfig conf = new CheckpointConfig();
            conf.setForce(force);
            conf.setKBytes(kBytes);
            conf.setMinutes(min);
            ((Environment) e).checkpoint(conf);
            return null;
        }, () -> buildLogMsg("checkpoint", env.handle, kBytes, min, force));
    }

    @Override
    public TDatabase openDatabase(TEnvironment tEnv, TTransaction tTxn,
            String fileName, String databaseName, TDatabaseConfig dbConfig)
            throws TException {
        return transactionalDescOp(tEnv.handle, tTxn, (ed, td) -> {
            EnvironmentDescriptor envDesc = (EnvironmentDescriptor) ed;
            DatabaseConfig config = Adapters.toBdbType(dbConfig);

            return createDatabaseHandle(envDesc, td, fileName, databaseName,
                    config, (e, txn, dbKey, conf) -> {
                        Environment env = e.getHandle();
                        Database db = env.openDatabase(txn, fileName,
                                databaseName, conf);
                        return new DatabaseDescriptor(db, dbKey, e);
                    });
        }, () -> buildLogMsg("openDatabase", tEnv.handle, tTxn, fileName,
                databaseName, dbConfig));
    }

    private <T extends DatabaseConfig> TDatabase createDatabaseHandle(
            EnvironmentDescriptor envDesc, TransactionDescriptor td,
            String fileName, String databaseName, T config,
            DatabaseDescriptorCreator<T> creator) throws Exception {
        ResourceMembers dbFileRes = null;
        ResourceMembers dbRes = null;
        try {
            Environment e = envDesc.getHandle();
            Transaction txn = td == null ? null : td.getHandle();
            DatabaseKey dbKey = getDatabaseKey(e, fileName, databaseName,
                    config.getAllowCreate());

            dbFileRes = readLock(dbKey.getDatabaseFile(),
                    "The database file is being renamed or deleted.");
            if (databaseName != null) {
                dbRes = readLock(dbKey,
                        "The database is being renamed or deleted.");
            }

            config.setTransactional(true);
            HandleDescriptor descriptor = this.handleManager.register(
                    creator.open(envDesc, txn, dbKey, config));

            return new TDatabase(descriptor.getId());
        } finally {
            this.handleManager.unlockRead(dbRes);
            this.handleManager.unlockRead(dbFileRes);
        }
    }

    private DatabaseKey getDatabaseKey(Environment env, String fileName,
            String databaseName, boolean allowCreate) throws Exception {
        File dataHome = env.getConfig().getCreateDir();
        if (dataHome == null) {
            dataHome = env.getHome();
        }
        DatabaseFileKey dbFileKey =
                resolveDbFileKey(dataHome, fileName, allowCreate);
        return new DatabaseKey(dbFileKey, databaseName);
    }

    private DatabaseFileKey resolveDbFileKey(File parentDir, String fileName,
            boolean allowCreate) throws Exception {
        boolean inMemory = fileName == null;
        File dbFile = inMemory ? parentDir : FileUtils
                .resolveFile(parentDir, fileName, allowCreate);
        if (dbFile == null) {
            throw new IOException("Cannot access database file.");
        }
        if (allowCreate) {
            FileUtils.createDirectories(
                    Collections.singleton(dbFile.getParentFile()));
        }
        return new DatabaseFileKey(dbFile, fileName, inMemory);
    }

    @Override
    public TDatabase openSecondaryDatabase(TEnvironment tEnv, TTransaction tTxn,
            String fileName, String databaseName, TDatabase primaryDb,
            TSecondaryDatabaseConfig sdbConfig) throws TException {
        return transactionalDescOp(tEnv.handle, tTxn, (ed, td) -> {
            DatabaseDescriptor primaryDesc = null;
            DatabaseDescriptor foreignDesc = null;
            try {
                primaryDesc = (DatabaseDescriptor) this.handleManager
                        .readLockHandle(primaryDb.handle);
                if (primaryDesc.getHandle().getConfig().getType() ==
                        DatabaseType.HEAP) {
                    throw new UnsupportedOperationException("Creating " +
                            "secondary databases on a Heap database is not" +
                            "supported.");
                }
                if (sdbConfig.isSetForeignDb()) {
                    foreignDesc = (DatabaseDescriptor) this.handleManager
                            .readLockHandle(sdbConfig.getForeignDb().handle);
                }

                EnvironmentDescriptor envDesc = (EnvironmentDescriptor) ed;
                SecondaryConfig config = Adapters.toBdbType(sdbConfig);

                final DatabaseDescriptor pdb = primaryDesc;
                final DatabaseDescriptor fdb = foreignDesc;
                return createDatabaseHandle(envDesc, td, fileName, databaseName,
                        config, (e, txn, dbKey, conf) -> {
                            Environment env = e.getHandle();
                            ServerKeyCreator keyCreator =
                                    new ServerKeyCreator(env, dbKey);
                            conf.setMultiKeyCreator(keyCreator);
                            if (fdb != null) {
                                conf.setForeignKeyDatabase(fdb.getHandle());
                            }
                            if (conf.getForeignKeyDeleteAction() ==
                                    ForeignKeyDeleteAction.NULLIFY) {
                                conf.setForeignMultiKeyNullifier(
                                        (sdb, pKey, pData, fKey) -> false);
                            }

                            return runWithAutoTxn(env, txn, autoTxn -> {
                                SecondaryDatabase sdb =
                                        env.openSecondaryDatabase(autoTxn,
                                                fileName, databaseName,
                                                pdb.getHandle(), conf);
                                keyCreator.setTransaction(autoTxn);
                                keyCreator.openAuxiliaryDb();
                                return new SecondaryDatabaseDescriptor(sdb,
                                        dbKey, e, pdb, fdb, keyCreator);
                            });
                        });
            } finally {
                this.handleManager.unlockHandle(foreignDesc);
                this.handleManager.unlockHandle(primaryDesc);
            }
        }, () -> buildLogMsg("openSecondaryDatabase", tEnv.handle, tTxn,
                fileName, databaseName, primaryDb.handle, sdbConfig));
    }

    @Override
    public void closeDatabaseHandle(TDatabase db) throws TException {
        closeHandle(db.handle, () -> "closeDatabase(" + db.handle + ")");
    }

    @Override
    public void closeDatabaseHandles(String envHomeDir, String fileName,
            String databaseName, long minIdleInMilli) throws TException {
        closeAllResourceHandles(minIdleInMilli, () -> {
            File envDir = resolveEnvDir(EnvDirType.DATA, envHomeDir);
            DatabaseFileKey dbFileKey =
                    resolveDbFileKey(envDir, fileName, false);

            return databaseName == null ? dbFileKey : new DatabaseKey(dbFileKey,
                    databaseName);
        }, () -> buildLogMsg("closeAllDatabaseHandles", envHomeDir, fileName,
                databaseName, minIdleInMilli));
    }

    @Override
    public TDatabaseConfig getDatabaseConfig(TDatabase db) throws TException {
        return handleOp(db.handle,
                d -> Adapters.toThriftType(((Database) d).getConfig()),
                () -> buildLogMsg("getDatabaseConfig", db.handle));
    }

    @Override
    public void setDatabaseConfig(TDatabase tDb, TDatabaseConfig dbConfig)
            throws TException {
        handleOp(tDb.handle, d -> {
            Database db = (Database) d;
            db.setConfig(Adapters.update(db.getConfig(), dbConfig));
            return null;
        }, () -> buildLogMsg("setDatabaseConfig", tDb.handle, dbConfig));
    }

    @Override
    public TGetResult dbGet(TDatabase tDb, TTransaction txn, TKeyData keyData,
            TDbGetConfig config) throws TException {
        return transactionalOp(tDb.handle, txn, (d, t) -> {
            Database db = (Database) d;
            GetArgs getArgs = new GetArgs(keyData, config);

            Class<? extends DatabaseEntry> dataClass = DatabaseEntry.class;
            if (config.isSetMultiple() && config.multiple) {
                dataClass = MultipleDataEntry.class;
            }

            OperationStatus opResult;
            opResult = getData(getArgs, dataClass, INIT_BUFFER_SIZE,
                    args -> selectDbGetFunction(db, config.mode)
                            .get(t, args.key, args.data, args.lockMode));

            TGetResult result = new TGetResult(Adapters.toThriftType(opResult));
            if (opResult == OperationStatus.SUCCESS) {
                result.setPairs(createPairs(
                        filterDbGetOutput(getArgs, config.mode), db));
            }
            return result;
        }, () -> buildLogMsg("dbGet", tDb.handle, txn, keyData, config));
    }

    private DbGetFunction selectDbGetFunction(Database db, TDbGetMode mode) {
        switch (mode) {
            case CONSUME:
                return (t, key, data, l) -> db.consume(t, key, data, false);
            case CONSUME_WAIT:
                return (t, key, data, l) -> db.consume(t, key, data, true);
            case DEFAULT:
                return db::get;
            case GET_BOTH:
                return db::getSearchBoth;
            case SET_RECNO:
                return db::getSearchRecordNumber;
            default:
                throw new IllegalArgumentException("Invalid get mode.");
        }
    }

    private GetArgs filterDbGetOutput(GetArgs getArgs, TDbGetMode mode) {
        switch (mode) {
            case GET_BOTH:
                getArgs.data = null;
                /* Fall through. */
            case DEFAULT:
                getArgs.key = null;
        }
        return getArgs;
    }

    private List<TKeyData> createPairs(GetArgs getArgs, Database db)
            throws DatabaseException {
        List<TKeyData> list = new LinkedList<>();
        if (getArgs.data instanceof MultipleKeyDataEntry) {
            DatabaseEntry key = new DatabaseEntry();
            DatabaseEntry item = new DatabaseEntry();
            while (((MultipleKeyDataEntry) getArgs.data).next(key, item)) {
                TKeyData pair = new TKeyData();
                pair.setKey(Adapters.toThriftType(key));
                pair.setData(Adapters.toThriftType(item));
                pair.getData().setPartial(false);
                list.add(pair);
            }
        } else if (getArgs.data instanceof MultipleRecnoDataEntry) {
            DatabaseEntry rec = new DatabaseEntry();
            DatabaseEntry item = new DatabaseEntry();
            while (((MultipleRecnoDataEntry) getArgs.data).next(rec, item)) {
                TKeyData pair = new TKeyData();
                pair.setKey(Adapters.toThriftType(rec));
                pair.setData(Adapters.toThriftType(item));
                pair.getData().setPartial(false);
                list.add(pair);
            }
        } else if (getArgs.data instanceof MultipleDataEntry) {
            TKeyData pair = new TKeyData();
            if (getArgs.key != null) {
                pair.setKey(Adapters.toThriftType(getArgs.key));
            }
            DatabaseEntry item = new DatabaseEntry();
            while (((MultipleDataEntry) getArgs.data).next(item)) {
                pair.setData(Adapters.toThriftType(item));
                pair.getData().setPartial(false);
                list.add(pair);
                pair = new TKeyData();
            }
        } else {
            TKeyData pair = new TKeyData();
            if (getArgs.key != null) {
                pair.setKey(Adapters.toThriftType(getArgs.key));
            }
            if (getArgs.data != null) {
                pair.setData(Adapters.toThriftType(getArgs.data));
            }
            list.add(pair);
        }
        return list;
    }

    private <T extends DatabaseEntry> OperationStatus getData(GetArgs getArgs,
            Class<T> dataClass, int initBufSize,
            FunctionEx<GetArgs, OperationStatus> func)
            throws Exception {
        if (MultipleEntry.class.isAssignableFrom(dataClass)) {
            int bufSize = initBufSize;
            if (bufSize < MIN_BUFFER_SIZE) {
                bufSize = MIN_BUFFER_SIZE;
            }
            OperationStatus result = null;
            while (result == null) {
                try {
                    getArgs.data = dataClass.newInstance();
                    getArgs.data.setData(new byte[bufSize]);
                    getArgs.data.setUserBuffer(bufSize, true);
                    result = func.applyWithException(getArgs);
                } catch (MemoryException e) {
                    bufSize *= 2;
                }
            }
            return result;
        } else {
            return func.applyWithException(getArgs);
        }
    }

    @Override
    public TGetWithPKeyResult dbGetWithPKey(TDatabase tSdb, TTransaction txn,
            TKeyDataWithPKey keyPKey, TDbGetConfig config) throws TException {
        return transactionalOp(tSdb.handle, txn, (d, t) -> {
            SecondaryDatabase sdb = (SecondaryDatabase) d;
            GetArgs getArgs = new GetArgs(keyPKey, config);

            OperationStatus opResult = selectDbPGetFunction(sdb, config.mode)
                    .get(t, getArgs.key, getArgs.pKey, getArgs.data,
                            getArgs.lockMode);

            TGetWithPKeyResult result =
                    new TGetWithPKeyResult(Adapters.toThriftType(opResult));
            if (opResult == OperationStatus.SUCCESS) {
                result.setTuple(createTuple(
                        filterDbPGetOutput(getArgs, config.mode)));
            }
            return result;
        }, () -> buildLogMsg("dbGetWithPKey", tSdb.handle, txn, keyPKey,
                config));
    }

    private DbPGetFunction selectDbPGetFunction(SecondaryDatabase sdb,
            TDbGetMode mode) {
        switch (mode) {
            case SET_RECNO:
                return sdb::getSearchRecordNumber;
            case GET_BOTH:
                return sdb::getSearchBoth;
            case DEFAULT:
                return sdb::get;
            default:
                throw new IllegalArgumentException("Invalid get mode.");
        }
    }

    private GetArgs filterDbPGetOutput(GetArgs getArgs, TDbGetMode mode) {
        switch (mode) {
            case GET_BOTH:
                getArgs.pKey = null;
                /* Fall through. */
            case DEFAULT:
                getArgs.key = null;
        }
        return getArgs;
    }

    private TKeyDataWithPKey createTuple(GetArgs getArgs)
            throws DatabaseException {
        TKeyDataWithPKey tuple = new TKeyDataWithPKey();
        if (getArgs.key != null) {
            tuple.setSkey(Adapters.toThriftType(getArgs.key));
        }
        if (getArgs.pKey != null) {
            tuple.setPkey(Adapters.toThriftType(getArgs.pKey));
        }
        if (getArgs.data != null) {
            tuple.setPdata(Adapters.toThriftType(getArgs.data));
        }
        return tuple;
    }

    private boolean isRecordKey(Database db) throws DatabaseException {
        DatabaseType type = db.getConfig().getType();
        return type == DatabaseType.QUEUE || type == DatabaseType.RECNO;
    }

    @Override
    public TPutResult dbPut(TDatabase db, TTransaction txn,
            List<TKeyDataWithSecondaryKeys> pairs, TDbPutConfig config)
            throws TException {
        return transactionalDescOp(db.handle, txn, (d, td) -> {
            DatabaseDescriptor primaryDesc = (DatabaseDescriptor) d;
            if (primaryDesc.hasSecondaryDb() && config == TDbPutConfig.APPEND) {
                throw new UnsupportedOperationException("Append is not " +
                        "supported for databases having secondary databases.");
            }

            Database pDb = primaryDesc.getHandle();
            Environment env = pDb.getEnvironment();
            Map<Long, Map<KeyDataPair, List<DatabaseEntry>>> secondaryKeys =
                    groupSecondaryKeysByDatabase(pairs);
            PutArgs putArgs = new PutArgs(pairs, isRecordKey(pDb), config);

            Transaction t = td == null ? null : td.getHandle();
            OperationStatus opResult;
            opResult = runWithAutoTxn(env, t, autoTxn -> {
                primaryDesc.forEachSecondary(sd -> {
                    sd.setCurrentTxn(autoTxn);
                    sd.setNewSecondaryKeys(secondaryKeys.get(sd.getId()));
                });
                Database primary = primaryDesc.getHandle();
                if (putArgs.key instanceof MultipleEntry) {
                    return primary.putMultipleKey(autoTxn,
                            (MultipleEntry) putArgs.key,
                            config == TDbPutConfig.OVERWRITE_DUP);
                } else {
                    return selectDbPutFunction(primary, config)
                            .put(autoTxn, putArgs.key, putArgs.data);
                }
            });

            TPutResult result = new TPutResult();
            result.setStatus(Adapters.toThriftType(opResult));
            if (config == TDbPutConfig.APPEND) {
                DatabaseEntry recordNum = new DatabaseEntry();
                recordNum.setRecordNumber(putArgs.key.getRecordNumber());
                result.setNewRecordNumber(recordNum.getData());
            }
            return result;
        }, () -> buildLogMsg("dbPut", db.handle, txn, "pairs", config));
    }

    private Map<Long, Map<KeyDataPair, List<DatabaseEntry>>>
    groupSecondaryKeysByDatabase(List<TKeyDataWithSecondaryKeys> pairs) {
        Map<Long, Map<KeyDataPair, List<DatabaseEntry>>> map = new HashMap<>();
        pairs.stream().filter(pair -> pair.isSetSkeys() && pair.skeys != null)
                .forEach(pair -> {
                    DatabaseEntry pKey = Adapters.toBdbType(pair.pkey);
                    DatabaseEntry pData = Adapters.toBdbType(pair.pdata);
                    KeyDataPair pPair = new KeyDataPair(pKey, pData);
                    for (TDatabase tDb : pair.skeys.keySet()) {
                        Map<KeyDataPair, List<DatabaseEntry>> secondaryKeys =
                                map.getOrDefault(tDb.handle, new HashMap<>());
                        secondaryKeys.put(pPair, pair.skeys.get(tDb).stream()
                                .map(Adapters::toBdbType)
                                .collect(Collectors.toList()));
                        map.put(tDb.handle, secondaryKeys);
                    }
                });
        return map;
    }

    private DbPutFunction selectDbPutFunction(Database db,
            TDbPutConfig config) {
        switch (config) {
            case APPEND:
                return db::append;
            case DEFAULT:
                return db::put;
            case NO_DUP_DATA:
                return db::putNoDupData;
            case NO_OVERWRITE:
                return db::putNoOverwrite;
            default:
                throw new IllegalArgumentException("Invalid put mode.");
        }
    }

    @Override
    public TOperationStatus dbDelete(TDatabase tDb, TTransaction tTxn,
            List<TKeyData> keyOrPairs) throws TException {
        return transactionalDescOp(tDb.handle, tTxn, (dd, td) -> {
            DatabaseDescriptor dbDesc = (DatabaseDescriptor) dd;
            Database db = dbDesc.getHandle();
            Environment env = db.getEnvironment();
            Transaction t = td == null ? null : td.getHandle();

            DelArgs delArgs = new DelArgs(keyOrPairs, isRecordKey(db));
            OperationStatus opResult;
            opResult = runWithAutoTxn(env, t, autoTxn -> {
                dbDesc.forEachSecondary(sd -> sd.setCurrentTxn(autoTxn));
                dbDesc.forEachForeignSecondary(sd -> sd.setCurrentTxn(autoTxn));
                if (keyOrPairs.get(0).isSetData()) {
                    return db.deleteMultipleKey(autoTxn, delArgs.key);
                } else {
                    return db.deleteMultiple(autoTxn, delArgs.key);
                }
            });
            return Adapters.toThriftType(opResult);
        }, () -> buildLogMsg("dbDelete", tDb.handle, tTxn, "keyOrPairs"));
    }

    @Override
    public TOperationStatus dbKeyExists(TDatabase db, TTransaction txn,
            TDbt key) throws TException {
        return transactionalOp(db.handle, txn, (d, t) -> {
            DatabaseEntry keyEntry = new DatabaseEntry(key.data);
            return Adapters.toThriftType(((Database) d).exists(t, keyEntry));
        }, () -> buildLogMsg("dbKeyExists", db.handle, txn, key));
    }

    @Override
    public TKeyRangeResult dbKeyRange(TDatabase db, TTransaction txn, TDbt key)
            throws TException {
        return transactionalOp(db.handle, txn, (d, t) -> {
            DatabaseEntry keyEntry = new DatabaseEntry(key.data);
            KeyRange range = ((Database) d).getKeyRange(t, keyEntry);
            return Adapters.toThriftType(range);
        }, () -> buildLogMsg("dbKeyRange", db.handle, txn, key));
    }

    @Override
    public TCompactResult dbCompact(TDatabase db, TTransaction txn, TDbt start,
            TDbt stop, TCompactConfig config) throws TException {
        return transactionalOp(db.handle, txn, (d, t) -> {
            DatabaseEntry startEntry = new DatabaseEntry(start.data);
            DatabaseEntry stopEntry = new DatabaseEntry(stop.data);
            DatabaseEntry endEntry = new DatabaseEntry();
            CompactConfig conf = Adapters.toBdbType(config);

            TCompactResult result = Adapters.toThriftType(((Database) d)
                    .compact(t, startEntry, stopEntry, endEntry, conf));
            result.setEndKey(new TDbt().setData(endEntry.getData()));

            return result;
        }, () -> buildLogMsg("dbCompact", db.handle, txn, start, stop, config));
    }

    @Override
    public int dbTruncate(TDatabase db, TTransaction txn, boolean countRecords)
            throws TException {
        return transactionalDescOp(db.handle, txn, (dd, td) -> {
            DatabaseDescriptor primaryDesc = (DatabaseDescriptor) dd;
            Database primary = primaryDesc.getHandle();
            Transaction transaction = td == null ? null : td.getHandle();

            return runWithAutoTxn(primary.getEnvironment(), transaction,
                    autoTxn -> {
                        int ret = primary.truncate(autoTxn, countRecords);
                        primaryDesc.forEachSecondary(sd -> {
                            try {
                                sd.getHandle().truncate(autoTxn, false);
                            } catch (DatabaseException e) {
                                error("Failed to truncate " +
                                        "an auxiliary database", e);
                            }
                        });
                        return ret;
                    });
        }, () -> buildLogMsg("dbTruncate", db.handle, txn, countRecords));
    }

    @Override
    public TCursor openCursor(TDatabase db, TTransaction txn,
            TCursorConfig cursorConfig) throws TException {
        return transactionalDescOp(db.handle, txn, (dd, td) -> {
            DatabaseDescriptor dbDesc = (DatabaseDescriptor) dd;
            Transaction transaction = td == null ? null : td.getHandle();

            CursorConfig conf = Adapters.toBdbType(cursorConfig);
            Cursor cursor = dbDesc.getHandle().openCursor(transaction, conf);

            HandleDescriptor descriptor = this.handleManager
                    .register(new CursorDescriptor(cursor, dbDesc, td));
            return new TCursor(descriptor.getId());
        }, () -> buildLogMsg("openCursor", db.handle, txn, cursorConfig));
    }

    @Override
    public void closeCursorHandle(TCursor cursor) throws TException {
        closeHandle(cursor.handle, () -> "closeCursor(" + cursor.handle + ")");
    }

    @Override
    public TCursorConfig getCursorConfig(TCursor cursor) throws TException {
        return handleOp(cursor.handle,
                c -> Adapters.toThriftType(((Cursor) c).getConfig()),
                () -> buildLogMsg("getCursorConfig", cursor.handle));
    }

    @Override
    public TCachePriority getCursorCachePriority(TCursor cursor)
            throws TException {
        return handleOp(cursor.handle,
                c -> Adapters.toThriftType(((Cursor) c).getPriority()),
                () -> "getCursorCachePriority(" + cursor.handle + ")");
    }

    @Override
    public void setCursorCachePriority(TCursor cursor, TCachePriority priority)
            throws TException {
        handleOp(cursor.handle, c -> {
            ((Cursor) c).setPriority(Adapters.toBdbType(priority));
            return null;
        }, () -> buildLogMsg("setCursorCachePriority", cursor.handle,
                priority));
    }

    @Override
    public TGetResult cursorGet(TCursor tCursor, TKeyData keyData,
            TCursorGetConfig config) throws TException {
        return handleOp(tCursor.handle, c -> {
            Cursor cursor = (Cursor) c;
            GetArgs getArgs = new GetArgs(keyData, config);

            Class<? extends DatabaseEntry> dataClass = DatabaseEntry.class;
            if (config.isSetMultiple() && config.multiple) {
                dataClass = MultipleDataEntry.class;
            } else if (config.isSetMultiKey() && config.multiKey) {
                dataClass = isRecordKey(cursor.getDatabase()) ?
                        MultipleRecnoDataEntry.class :
                        MultipleKeyDataEntry.class;
            }

            OperationStatus opResult;
            opResult = getData(getArgs, dataClass, config.batchSize,
                    args -> selectCursorGetFunction(cursor, config.mode)
                            .get(args.key, args.data, args.lockMode));

            TGetResult result = new TGetResult(Adapters.toThriftType(opResult));
            if (opResult == OperationStatus.SUCCESS) {
                result.setPairs(createPairs(
                        filterCursorGetOutput(getArgs, config.mode),
                        cursor.getDatabase()));
            }
            return result;
        }, () -> buildLogMsg("cursorGet", tCursor.handle, keyData, config));
    }

    private CursorGetFunction selectCursorGetFunction(Cursor cursor,
            TCursorGetMode mode) {
        switch (mode) {
            case CURRENT:
                return cursor::getCurrent;
            case FIRST:
                return cursor::getFirst;
            case GET_BOTH:
                return cursor::getSearchBoth;
            case GET_BOTH_RANGE:
                return cursor::getSearchBothRange;
            case GET_RECNO:
                return (k, data, lock) -> cursor.getRecordNumber(data, lock);
            case LAST:
                return cursor::getLast;
            case NEXT:
                return cursor::getNext;
            case NEXT_DUP:
                return cursor::getNextDup;
            case NEXT_NO_DUP:
                return cursor::getNextNoDup;
            case PREV:
                return cursor::getPrev;
            case PREV_DUP:
                return cursor::getPrevDup;
            case PREV_NO_DUP:
                return cursor::getPrevNoDup;
            case SET:
                return cursor::getSearchKey;
            case SET_RANGE:
                return cursor::getSearchKeyRange;
            case SET_RECNO:
                return cursor::getSearchRecordNumber;
            default:
                throw new IllegalArgumentException("Invalid get mode.");
        }
    }

    private GetArgs filterCursorGetOutput(GetArgs getArgs,
            TCursorGetMode mode) {
        switch (mode) {
            case GET_BOTH:
                getArgs.data = null;
                /* Fall through. */
            case GET_BOTH_RANGE:
            case GET_RECNO:
            case SET:
                getArgs.key = null;
        }
        return getArgs;
    }

    @Override
    public TGetWithPKeyResult cursorGetWithPKey(TCursor tCursor,
            TKeyDataWithPKey keyPKey, TCursorGetConfig config)
            throws TException {
        return handleOp(tCursor.handle, c -> {
            SecondaryCursor cursor = (SecondaryCursor) c;
            GetArgs getArgs = new GetArgs(keyPKey, config);

            OperationStatus opResult =
                    selectCursorPGetFunction(cursor, config.mode)
                            .get(getArgs.key, getArgs.pKey, getArgs.data,
                                    getArgs.lockMode);

            TGetWithPKeyResult result =
                    new TGetWithPKeyResult(Adapters.toThriftType(opResult));
            if (opResult == OperationStatus.SUCCESS) {
                result.setTuple(createTuple(
                        filterCursorPGetOutput(getArgs, config.mode)));
            }
            return result;
        }, () -> buildLogMsg("cursorGetWithPKey", tCursor.handle, keyPKey,
                config));
    }

    private CursorPGetFunction selectCursorPGetFunction(SecondaryCursor cursor,
            TCursorGetMode mode) {
        switch (mode) {
            case CURRENT:
                return cursor::getCurrent;
            case FIRST:
                return cursor::getFirst;
            case GET_BOTH:
                return cursor::getSearchBoth;
            case GET_BOTH_RANGE:
                return cursor::getSearchBothRange;
            case GET_RECNO:
                return (sk, pk, data, lock) -> cursor
                        .getRecordNumber(pk, data, lock);
            case LAST:
                return cursor::getLast;
            case NEXT:
                return cursor::getNext;
            case NEXT_DUP:
                return cursor::getNextDup;
            case NEXT_NO_DUP:
                return cursor::getNextNoDup;
            case PREV:
                return cursor::getPrev;
            case PREV_DUP:
                return cursor::getPrevDup;
            case PREV_NO_DUP:
                return cursor::getPrevNoDup;
            case SET:
                return cursor::getSearchKey;
            case SET_RANGE:
                return cursor::getSearchKeyRange;
            case SET_RECNO:
                return cursor::getSearchRecordNumber;
            default:
                throw new IllegalArgumentException("Invalid get mode.");
        }
    }

    private GetArgs filterCursorPGetOutput(GetArgs getArgs,
            TCursorGetMode mode) {
        switch (mode) {
            case GET_BOTH:
                getArgs.pKey = null;
                /* Fall through. */
            case GET_BOTH_RANGE:
            case GET_RECNO:
            case SET:
                getArgs.key = null;
        }
        return getArgs;
    }

    @Override
    public TPutResult cursorPut(TCursor tCursor, TKeyDataWithSecondaryKeys pair,
            TCursorPutConfig config) throws TException {
        return cursorOp(tCursor, (cursorDesc, txn) -> {
            Map<Long, Map<KeyDataPair, List<DatabaseEntry>>> secondaryKeys =
                    groupSecondaryKeysByDatabase(
                            Collections.singletonList(pair));
            PutArgs putArgs = new PutArgs(pair);

            DatabaseDescriptor dbDesc = cursorDesc.getDb();
            dbDesc.forEachSecondary(sd -> {
                sd.setCurrentTxn(txn);
                sd.setNewSecondaryKeys(secondaryKeys.get(sd.getId()));
            });

            OperationStatus opResult =
                    selectCursorPutFunction(cursorDesc.getHandle(), config)
                            .put(putArgs.key, putArgs.data);

            TPutResult result = new TPutResult();
            result.setStatus(Adapters.toThriftType(opResult));
            DatabaseType dbType = dbDesc.getHandle().getConfig().getType();
            if (dbType == DatabaseType.RECNO &&
                    (config == TCursorPutConfig.AFTER ||
                            config == TCursorPutConfig.BEFORE)) {
                DatabaseEntry recordNum = new DatabaseEntry();
                recordNum.setRecordNumber(putArgs.key.getRecordNumber());
                result.setNewRecordNumber(recordNum.getData());
            }
            return result;
        }, () -> buildLogMsg("cursorPut", tCursor.handle, pair, config));
    }

    private CursorPutFunction selectCursorPutFunction(Cursor cursor,
            TCursorPutConfig config) {
        switch (config) {
            case AFTER:
                return cursor::putAfter;
            case BEFORE:
                return cursor::putBefore;
            case CURRENT:
                return (k, d) -> cursor.putCurrent(d);
            case DEFAULT:
                return cursor::put;
            case KEY_FIRST:
                return cursor::putKeyFirst;
            case KEY_LAST:
                return cursor::putKeyLast;
            case NO_DUP_DATA:
                return cursor::putNoDupData;
            case NO_OVERWRITE:
                return cursor::putNoOverwrite;
            default:
                throw new IllegalArgumentException("Invalid put mode.");
        }
    }

    @Override
    public TOperationStatus cursorDelete(TCursor cursor) throws TException {
        return cursorOp(cursor, (cursorDesc, txn) -> {
            DatabaseDescriptor dbDesc = cursorDesc.getDb();
            dbDesc.forEachSecondary(sd -> sd.setCurrentTxn(txn));
            dbDesc.forEachForeignSecondary(sd -> sd.setCurrentTxn(txn));
            return Adapters.toThriftType(cursorDesc.getHandle().delete());
        }, () -> buildLogMsg("cursorDelete", cursor.handle));
    }

    private <T> T cursorOp(TCursor tCursor,
            BiFunctionEx<CursorDescriptor, Transaction, T> op,
            Supplier<String> errMsg) throws TException {
        CursorDescriptor cursorDesc = null;
        try {
            cursorDesc = (CursorDescriptor) this.handleManager
                    .readLockHandle(tCursor.handle);
            TransactionDescriptor txnDesc = cursorDesc.getTransaction();
            DatabaseDescriptor dbDesc = cursorDesc.getDb();

            Transaction txn = txnDesc == null ? null : txnDesc.getHandle();
            Environment env = dbDesc.getHandle().getEnvironment();
            final CursorDescriptor c = cursorDesc;
            return runWithAutoTxn(env, txn,
                    autoTxn -> op.applyWithException(c, autoTxn));
        } catch (Exception e) {
            throw error(errMsg.get(), e);
        } finally {
            this.handleManager.unlockHandle(cursorDesc);
        }
    }

    @Override
    public TCursor cursorDup(TCursor cursor, boolean samePos)
            throws TException {
        CursorDescriptor desc = null;
        try {
            desc = (CursorDescriptor) this.handleManager
                    .readLockHandle(cursor.handle);
            Cursor newCursor = desc.getHandle().dup(samePos);

            HandleDescriptor descriptor = this.handleManager
                    .register(new CursorDescriptor(newCursor, desc));
            return new TCursor(descriptor.getId());
        } catch (Exception e) {
            throw error(buildLogMsg("cursorDup", cursor.handle, samePos), e);
        } finally {
            this.handleManager.unlockHandle(desc);
        }
    }

    @Override
    public short cursorCompare(TCursor cursor1, TCursor cursor2)
            throws TException {
        return handleOp(cursor1.handle, c1 -> {
            CursorDescriptor c2 = null;
            try {
                c2 = (CursorDescriptor) this.handleManager
                        .readLockHandle(cursor2.handle);
                return (short) ((Cursor) c1).compare(c2.getHandle());
            } finally {
                this.handleManager.unlockHandle(c2);
            }
        }, () -> buildLogMsg("cursorCompare", cursor1.handle, cursor2.handle));
    }

    @Override
    public int cursorCount(TCursor cursor) throws TException {
        return handleOp(cursor.handle, c -> ((Cursor) c).count(),
                () -> buildLogMsg("cursorCount", cursor.handle));
    }

    @Override
    public TJoinCursor openJoinCursor(TDatabase pdb, List<TCursor> secCursors,
            boolean sortCursors) throws TException {
        DatabaseDescriptor dbDesc = null;
        List<CursorDescriptor> cursorDescList = new LinkedList<>();
        try {
            dbDesc = (DatabaseDescriptor) this.handleManager.readLockHandle(
                    pdb.handle);
            secCursors.forEach(sc -> cursorDescList
                    .add((CursorDescriptor) this.handleManager.readLockHandle(
                            sc.handle)));

            Cursor[] cursors =
                    cursorDescList.stream().map(HandleDescriptor::getHandle)
                            .toArray(Cursor[]::new);

            JoinConfig config = new JoinConfig();
            config.setNoSort(!sortCursors);

            JoinCursor joinCursor = dbDesc.getHandle().join(cursors, config);
            HandleDescriptor descriptor = this.handleManager
                    .register(new JoinCursorDescriptor(joinCursor, dbDesc,
                            cursorDescList));

            return new TJoinCursor(descriptor.getId());
        } catch (Exception e) {
            throw error(buildLogMsg("openJoinCursor", pdb.handle, secCursors,
                    sortCursors), e);
        } finally {
            cursorDescList.forEach(this.handleManager::unlockHandle);
            this.handleManager.unlockHandle(dbDesc);
        }
    }

    @Override
    public void closeJoinCursorHandle(TJoinCursor cursor) throws TException {
        closeHandle(cursor.handle,
                () -> buildLogMsg("closeJoinCursor", cursor.handle));
    }

    @Override
    public TGetResult joinCursorGet(TJoinCursor cursor,
            TJoinCursorGetConfig config) throws TException {
        return handleOp(cursor.handle, c -> {
            JoinCursor joinCursor = (JoinCursor) c;
            GetArgs getArgs = new GetArgs(config);

            OperationStatus opResult = joinCursor
                    .getNext(getArgs.key, getArgs.data, getArgs.lockMode);

            TGetResult result = new TGetResult(Adapters.toThriftType(opResult));
            if (opResult == OperationStatus.SUCCESS) {
                result.setPairs(createPairs(getArgs, joinCursor.getDatabase()));
            }
            return result;
        }, () -> buildLogMsg("joinCursorGet", cursor.handle, config));
    }

    @Override
    public TTransaction beginTransaction(TEnvironment env, TTransaction parent,
            TTransactionConfig config) throws TException {
        return transactionalDescOp(env.getHandle(), parent, (ed, td) -> {
            EnvironmentDescriptor e = (EnvironmentDescriptor) ed;
            Transaction parentTxn = td == null ? null : td.getHandle();

            TransactionConfig conf = Adapters.toBdbType(config);
            Transaction txn = e.getHandle().beginTransaction(parentTxn, conf);

            HandleDescriptor descriptor = this.handleManager.register(
                    new TransactionDescriptor(txn, e, td));
            return new TTransaction(descriptor.getId());
        }, () -> "beginTransaction(" + env.handle + ", " + parent + ", " +
                config + ")");
    }

    @Override
    public void txnAbort(TTransaction txn) throws TException {
        resolveTxn(txn, Status.ABORTED, () -> "txnAbort(" + txn.handle + ")");
    }

    @Override
    public void txnCommit(TTransaction txn, TDurabilityPolicy durability)
            throws TException {
        resolveTxn(txn, Status.toStatus(durability),
                () -> "txnCommit(" + txn.handle + ", " + durability + ")");
    }

    private void resolveTxn(TTransaction txn, Status resolvedStatus,
            Supplier<String> errMsg) throws TException {
        try {
            this.handleManager.closeHandle(txn.handle,
                    t -> ((TransactionDescriptor) t).resolve(resolvedStatus));
        } catch (Exception e) {
            throw error(errMsg.get(), e);
        }
    }

    @Override
    public int txnGetPriority(TTransaction txn) throws TException {
        return handleOp(txn.handle, t -> ((Transaction) t).getPriority(),
                () -> "txnGetPriority(" + txn.handle + ")");
    }

    @Override
    public void txnSetPriority(TTransaction txn, int priority)
            throws TException {
        handleOp(txn.handle, t -> {
            ((Transaction) t).setPriority(priority);
            return null;
        }, () -> "txnSetPriority(" + txn.handle + ", " + priority + ")");
    }

    @Override
    public TSequence openSequence(TDatabase tDb, TTransaction tTxn, TDbt key,
            TSequenceConfig config) throws TException {
        return transactionalDescOp(tDb.handle, tTxn, (dd, td) -> {
            ResourceMembers seqRes = null;
            try {
                DatabaseDescriptor dbDesc = (DatabaseDescriptor) dd;
                Database db = dbDesc.getHandle();
                Transaction txn = td == null ? null : td.getHandle();

                SequenceConfig conf = Adapters.toBdbType(config);

                DatabaseKey dbKey = dbDesc.getResourceKey();
                DatabaseEntry keyEntry = new DatabaseEntry(key.getData());
                SequenceKey seqKey = new SequenceKey(dbKey, key.getData());

                seqRes = readLock(seqKey, "The sequence is being deleted.");

                Sequence seq = db.openSequence(txn, keyEntry, conf);
                HandleDescriptor descriptor = this.handleManager
                        .register(new SequenceDescriptor(seq, seqKey, dbDesc));

                return new TSequence(descriptor.getId());
            } finally {
                this.handleManager.unlockRead(seqRes);
            }
        }, () -> buildLogMsg("openSequence", tDb.handle, tTxn, key, config));
    }

    @Override
    public void closeSequenceHandle(TSequence seq) throws TException {
        closeHandle(seq.handle, () -> buildLogMsg("closeSequence", seq.handle));
    }

    @Override
    public void removeSequence(TDatabase tDb, TTransaction tTxn, TDbt key,
            boolean isAutoCommitNoSync, boolean force) throws TException {
        transactionalDescOp(tDb.handle, tTxn, (dd, td) -> {
            ResourceMembers seqRes = null;
            try {
                DatabaseDescriptor dbDesc = (DatabaseDescriptor) dd;
                Database db = dbDesc.getHandle();
                Transaction txn = td == null ? null : td.getHandle();

                DatabaseKey dbKey = dbDesc.getResourceKey();
                SequenceKey seqKey = new SequenceKey(dbKey, key.getData());
                seqRes = this.handleManager.writeLockResource(seqKey);

                if (!seqRes.isEmpty() && !force) {
                    throw new TResourceInUseException(
                            "The sequence is in use.");
                }
                this.handleManager.closeHandles(seqRes.getMembers());

                SequenceConfig conf = new SequenceConfig();
                conf.setAutoCommitNoSync(isAutoCommitNoSync);
                db.removeSequence(txn, new DatabaseEntry(key.getData()), conf);
                return null;
            } finally {
                this.handleManager.unlockWrite(seqRes);
            }
        }, () -> buildLogMsg("removeSequence", tDb.handle, tTxn, key, force));
    }

    @Override
    public long sequenceGet(TSequence seq, TTransaction txn, int delta)
            throws TException {
        return transactionalOp(seq.handle, txn,
                (s, t) -> ((Sequence) s).get(t, delta),
                () -> buildLogMsg("sequenceGet", seq.handle, txn, delta));
    }

    @Override
    public TEnvStatResult getEnvStatistics(TEnvironment tEnv,
            TEnvStatConfig config) throws TException {
        return handleOp(tEnv.handle, e -> {
            Environment env = (Environment) e;
            TEnvStatOption EXCLUDE = TEnvStatOption.EXCLUDE;

            TEnvStatResult result = new TEnvStatResult();
            if (config.isSetCache() && config.cache != EXCLUDE) {
                result.setCacheStat(Adapters.toThriftType(
                        getStat(config.cache, env::getCacheStats)));
            }
            if (config.isSetCacheFile() && config.cacheFile != EXCLUDE) {
                CacheFileStats[] stats =
                        getStat(config.cacheFile, env::getCacheFileStats);
                if (stats == null) {
                    stats = new CacheFileStats[0];
                }
                result.setCacheFileStat(Arrays.stream(stats)
                        .map(Adapters::toThriftType)
                        .collect(Collectors.toList()));
            }
            if (config.isSetLock() && config.lock != EXCLUDE) {
                result.setLockStat(Adapters.toThriftType(
                        getStat(config.lock, env::getLockStats)));
            }
            if (config.isSetLog() && config.log != EXCLUDE) {
                result.setLogStat(Adapters.toThriftType(
                        getStat(config.log, env::getLogStats)));
            }
            if (config.isSetMutex() && config.mutex != EXCLUDE) {
                result.setMutexStat(Adapters.toThriftType(
                        getStat(config.mutex, env::getMutexStats)));
            }
            if (config.isSetTransaction() && config.transaction != EXCLUDE) {
                result.setTxnStat(Adapters.toThriftType(
                        getStat(config.transaction, env::getTransactionStats)));
            }
            return result;
        }, () -> buildLogMsg("getEnvStatistics", tEnv.handle, config));
    }

    private <R> R getStat(TEnvStatOption option, FunctionEx<StatsConfig, R> op)
            throws Exception {
        StatsConfig clearConfig = new StatsConfig();
        clearConfig.setClear(true);

        return op.applyWithException(
                option == TEnvStatOption.CLEAR ? clearConfig : null);
    }

    @Override
    public TDatabaseStatResult getDatabaseStatistics(TDatabase tDb,
            TTransaction txn, boolean fast) throws TException {
        return transactionalOp(tDb.handle, txn, (d, t) -> {
            Database db = (Database) d;
            StatsConfig conf = new StatsConfig();
            conf.setFast(fast);

            return Adapters.toThriftType(db.getStats(t, conf));
        }, () -> buildLogMsg("getDatabaseStatistics", tDb.handle, txn, fast));
    }

    private <R> R handleOp(long handleId, FunctionEx<Object, R> op,
            Supplier<String> errMsg) throws TException {
        HandleDescriptor desc = null;
        try {
            desc = this.handleManager.readLockHandle(handleId);
            return op.applyWithException(desc.getHandle());
        } catch (Exception e) {
            throw error(errMsg.get(), e);
        } finally {
            this.handleManager.unlockHandle(desc);
        }
    }

    private <R> R transactionalDescOp(long handleId, TTransaction txn,
            BiFunctionEx<HandleDescriptor, TransactionDescriptor, R> op,
            Supplier<String> errMsg) throws TException {
        HandleDescriptor desc = null;
        TransactionDescriptor txnDesc = null;
        try {
            desc = this.handleManager.readLockHandle(handleId);
            if (txn != null) {
                txnDesc = (TransactionDescriptor) this.handleManager
                        .readLockHandle(txn.handle);
            }
            return op.applyWithException(desc, txnDesc);
        } catch (Exception e) {
            throw error(errMsg.get(), e);
        } finally {
            this.handleManager.unlockHandle(txnDesc);
            this.handleManager.unlockHandle(desc);
        }
    }

    private <R> R transactionalOp(long handleId, TTransaction txn,
            BiFunctionEx<Object, Transaction, R> op, Supplier<String> errMsg)
            throws TException {
        return transactionalDescOp(handleId, txn,
                (h, t) -> op.applyWithException(
                        h.getHandle(), t == null ? null : t.getHandle()),
                errMsg);
    }

    private ResourceMembers readLock(ResourceKey key, String errMsg)
            throws TResourceInUseException {
        ResourceMembers resource = this.handleManager.readLockResource(key);
        if (resource == null) {
            throw new TResourceInUseException(errMsg);
        }
        return resource;
    }

    private void closeHandle(long id, Supplier<String> errMsg)
            throws TException {
        try {
            this.handleManager.closeHandle(id);
        } catch (Exception e) {
            throw error(errMsg.get(), e);
        }
    }

    private void closeAllResourceHandles(long minIdleInMilli,
            SupplierEx<ResourceKey> resKey, Supplier<String> errMsg)
            throws TException {
        try {
            ResourceKey key = resKey.getWithException();
            long now = System.currentTimeMillis();

            List<HandleDescriptor> closeList =
                    this.handleManager.getResourceMembers(key).getMembers()
                            .stream()
                            .filter(h -> h.isExpired(now, minIdleInMilli))
                            .collect(Collectors.toList());

            DatabaseException e = this.handleManager.closeHandles(closeList);
            if (e != null) {
                throw e;
            }
        } catch (Exception e) {
            throw error(errMsg.get(), e);
        }
    }

    private String buildLogMsg(String funcName, Object... objects) {
        return Arrays.stream(objects).map(Objects::toString).collect(
                Collectors.joining(", ", funcName + "(", ")"));
    }

    private TException error(String msg, Exception e) {
        logger.error(msg, e);
        return Adapters.toThriftType(e);
    }

    @FunctionalInterface
    private interface DatabaseResourceOperation {
        void run(Environment env, Transaction txn, String auxFileName,
                String auxDbName) throws Exception;
    }

    @FunctionalInterface
    private interface DatabaseDescriptorCreator<T extends DatabaseConfig> {
        DatabaseDescriptor open(EnvironmentDescriptor envDesc, Transaction txn,
                DatabaseKey dbKey, T config) throws Exception;
    }

    @FunctionalInterface
    private interface DbGetFunction {
        OperationStatus get(Transaction txn, DatabaseEntry key,
                DatabaseEntry data, LockMode lockMode) throws DatabaseException;
    }

    @FunctionalInterface
    private interface DbPGetFunction {
        OperationStatus get(Transaction txn, DatabaseEntry sKey,
                DatabaseEntry pKey, DatabaseEntry pData, LockMode lockMode)
                throws DatabaseException;
    }

    @FunctionalInterface
    private interface CursorGetFunction {
        OperationStatus get(DatabaseEntry key, DatabaseEntry data,
                LockMode lockMode) throws DatabaseException;
    }

    @FunctionalInterface
    private interface CursorPGetFunction {
        OperationStatus get(DatabaseEntry sKey, DatabaseEntry pKey,
                DatabaseEntry pData, LockMode lockMode)
                throws DatabaseException;
    }

    @FunctionalInterface
    private interface DbPutFunction {
        OperationStatus put(Transaction txn, DatabaseEntry key,
                DatabaseEntry data) throws DatabaseException;
    }

    @FunctionalInterface
    private interface CursorPutFunction {
        OperationStatus put(DatabaseEntry key, DatabaseEntry data)
                throws DatabaseException;
    }
}
