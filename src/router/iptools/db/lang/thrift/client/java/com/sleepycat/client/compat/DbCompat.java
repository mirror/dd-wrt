/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client.compat;

import java.io.File;
import java.io.IOException;
import java.nio.ByteOrder; 

import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SDatabaseType;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;
import com.sleepycat.client.SLockDetectMode;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.SSecondaryConfig;
import com.sleepycat.client.SSecondaryCursor;
import com.sleepycat.client.SSecondaryDatabase;
import com.sleepycat.client.STransaction;

/**
 * A minimal set of DB-JE compatibility methods for internal use only.
 * Two versions are maintained in parallel in the DB and JE source trees.
 * Used by the collections package.
 */
public class DbCompat {

    /* Capabilities */

    public static final boolean CDB = false;
    public static final boolean JOIN = true;
    public static final boolean NESTED_TRANSACTIONS = true;
    public static final boolean INSERTION_ORDERED_DUPLICATES = true;
    public static final boolean SEPARATE_DATABASE_FILES = true;
    public static final boolean MEMORY_SUBSYSTEM = true;
    public static final boolean LOCK_SUBSYSTEM = true;
    public static final boolean HASH_METHOD = true;
    public static final boolean RECNO_METHOD = false;
    public static final boolean QUEUE_METHOD = false;
    public static final boolean BTREE_RECNUM_METHOD = true;
    public static final boolean OPTIONAL_READ_UNCOMMITTED = true;
    public static final boolean SECONDARIES = true;
    public static boolean TRANSACTION_RUNNER_PRINT_STACK_TRACES = true;
    public static final boolean DATABASE_COUNT = false;
    public static final boolean NEW_JE_EXCEPTIONS = false;
    public static final boolean POPULATE_ENFORCES_CONSTRAINTS = false;
    private static boolean fSystemCaseSensitive = true;

    /* Check if the underlying file system is case sensitive */ 
    static {
        File file1 = new File("no_such_file");
        File file2 = new File("NO_SUCH_FILE");
        fSystemCaseSensitive = !file1.equals(file2);
    } 

    /* Methods used by the collections package. */

    public static ClassLoader getClassLoader(SEnvironment env) {
        return null;
    }

    public static boolean getInitializeCache(SEnvironmentConfig config) {
        return config.getInitializeCache();
    }

    public static boolean getInitializeLocking(SEnvironmentConfig config) {
        return config.getInitializeLocking();
    }

    public static boolean getInitializeCDB(SEnvironmentConfig config) {
        return config.getInitializeCDB();
    }

    public static boolean isTypeBtree(SDatabaseConfig dbConfig) {
        return dbConfig.getType() == SDatabaseType.BTREE;
    }

    public static boolean isTypeHash(SDatabaseConfig dbConfig) {
        return dbConfig.getType() == SDatabaseType.HASH;
    }

    public static boolean isTypeQueue(SDatabaseConfig dbConfig) {
        return false;
    }

    public static boolean isTypeRecno(SDatabaseConfig dbConfig) {
        return false;
    }

    public static boolean getBtreeRecordNumbers(SDatabaseConfig dbConfig) {
        return dbConfig.getBtreeRecordNumbers();
    }

    public static boolean getReadUncommitted(SDatabaseConfig dbConfig) {
        return dbConfig.getReadUncommitted();
    }

    public static boolean getRenumbering(SDatabaseConfig dbConfig) {
        return dbConfig.getRenumbering();
    }

    public static boolean getSortedDuplicates(SDatabaseConfig dbConfig) {
        return dbConfig.getSortedDuplicates();
    }

    public static boolean getUnsortedDuplicates(SDatabaseConfig dbConfig) {
        return dbConfig.getUnsortedDuplicates();
    }

    public static boolean getDeferredWrite(SDatabaseConfig dbConfig) {
        return false;
    }

    // XXX Remove this when DB and JE support SCursorConfig.cloneConfig
    public static SCursorConfig cloneCursorConfig(SCursorConfig config) {
        SCursorConfig newConfig = new SCursorConfig();
        newConfig.setReadCommitted(config.getReadCommitted());
        newConfig.setReadUncommitted(config.getReadUncommitted());
        return newConfig;
    }

    public static boolean getWriteCursor(SCursorConfig config) {
        return false;
    }

    public static void setWriteCursor(SCursorConfig config, boolean val) {
        throw new IllegalStateException(
                "Concurrent Data Store is not supported.");
    }

    public static void setRecordNumber(SDatabaseEntry entry,
        int recNum, ByteOrder bo) {
        entry.setRecordNumber(recNum, bo);
    }

    public static int getRecordNumber(SDatabaseEntry entry, ByteOrder bo) {
        return entry.getRecordNumber(bo);
    }

    public static String getDatabaseFile(SDatabase db)
        throws SDatabaseException {
        return db.getDatabaseFile();
    }

    public static long getDatabaseCount(SDatabase db)
        throws SDatabaseException {

        throw new UnsupportedOperationException();
    }

    public static void syncDeferredWrite(SDatabase db, boolean flushLog)
        throws SDatabaseException {
    }

    public static SOperationStatus getCurrentRecordNumber(SCursor cursor,
                                                         SDatabaseEntry key,
                                                         SLockMode lockMode)
        throws SDatabaseException {
        return cursor.getRecordNumber(key, lockMode);
    }

    public static SOperationStatus getSearchRecordNumber(SCursor cursor,
                                                        SDatabaseEntry key,
                                                        SDatabaseEntry data,
                                                        SLockMode lockMode)
        throws SDatabaseException {
        return cursor.getSearchRecordNumber(key, data, lockMode);
    }

    public static SOperationStatus getSearchRecordNumber(SSecondaryCursor cursor,
                                                        SDatabaseEntry key,
                                                        SDatabaseEntry pKey,
                                                        SDatabaseEntry data,
                                                        SLockMode lockMode)
        throws SDatabaseException {
        return cursor.getSearchRecordNumber(key, pKey, data, lockMode);
    }

    public static SOperationStatus putAfter(SCursor cursor,
                                           SDatabaseEntry key,
                                           SDatabaseEntry data)
        throws SDatabaseException {
        return cursor.putAfter(key, data);
    }

    public static SOperationStatus putBefore(SCursor cursor,
                                            SDatabaseEntry key,
                                            SDatabaseEntry data)
        throws SDatabaseException {
        return cursor.putBefore(key, data);
    }

    public static STransaction getThreadTransaction(SEnvironment env)
	throws SDatabaseException {
        return null;
    }

    /* Methods used by the collections tests. */

    public static void setLockDetectModeOldest(SEnvironmentConfig config) {

        config.setLockDetectMode(SLockDetectMode.OLDEST);
    }

    public static void setTypeBtree(SDatabaseConfig dbConfig) {
        dbConfig.setType(SDatabaseType.BTREE);
    }

    public static void setTypeHash(SDatabaseConfig dbConfig) {
        dbConfig.setType(SDatabaseType.HASH);
    }

    public static void setBtreeRecordNumbers(SDatabaseConfig dbConfig,
                                             boolean val) {
        dbConfig.setBtreeRecordNumbers(val);
    }

    public static void setReadUncommitted(SDatabaseConfig dbConfig,
                                          boolean val) {
        dbConfig.setReadUncommitted(val);
    }

    public static void setRenumbering(SDatabaseConfig dbConfig,
                                      boolean val) {
        dbConfig.setRenumbering(val);
    }

    public static void setSortedDuplicates(SDatabaseConfig dbConfig,
                                           boolean val) {
        dbConfig.setSortedDuplicates(val);
    }

    public static void setUnsortedDuplicates(SDatabaseConfig dbConfig,
                                             boolean val) {
        dbConfig.setUnsortedDuplicates(val);
    }

    public static void setDeferredWrite(SDatabaseConfig dbConfig, boolean val) {
    }

    public static void setRecordLength(SDatabaseConfig dbConfig, int val) {
        dbConfig.setRecordLength(val);
    }

    public static void setRecordPad(SDatabaseConfig dbConfig, int val) {
        dbConfig.setRecordPad(val);
    }

    public static boolean databaseExists(SEnvironment env,
                                         String fileName,
                                         String dbName) {
        /* Currently we only support file names. */
        assert fileName != null;
        assert dbName == null;
        try {
            SDatabase db = env.openDatabase(null, fileName, dbName, null);
            db.close();
            return true;
        } catch (SDatabaseException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            return false;
        }
    }

    public static SDatabase openDatabase(SEnvironment env,
                                        STransaction txn,
                                        String fileName,
                                        String dbName,
                                        SDatabaseConfig config)
        throws SDatabaseException {
        /* Currently we only support file names. */
        assert fileName != null;
        assert dbName == null;
        try {
            return env.openDatabase(txn, fileName, dbName, config);
        } catch (SDatabaseException e) {
            if (isFileExistsError(e)) {
                return null;
            }
            throw e;
        } catch (IOException e) {
            return null;
        }
    }

    public static SSecondaryDatabase openSecondaryDatabase(
                                        SEnvironment env,
                                        STransaction txn,
                                        String fileName,
                                        String dbName,
                                        SDatabase primaryDatabase,
                                        SSecondaryConfig config)
        throws SDatabaseException {
        /* Currently we only support file names. */
        assert fileName != null;
        assert dbName == null;
        try {
            return env.openSecondaryDatabase
                (txn, fileName, dbName, primaryDatabase, config);
        } catch (SDatabaseException e) {
            if (isFileExistsError(e)) {
                return null;
            }
            throw e;
        } catch (IOException e) {
            return null;
        }
    }

    public static boolean truncateDatabase(SEnvironment env,
                                           STransaction txn,
                                           String fileName,
                                           String dbName)
        throws SDatabaseException {
        /* Currently we only support file names. */
        assert fileName != null;
        assert dbName == null;
        SDatabase db;
        try {
            db = env.openDatabase(txn, fileName, dbName, null);
        } catch (IOException e) {
            return false;
        }
        try {
            db.truncate(txn, false /*returnCount*/);
            return true;
        } finally {
            db.close();
        }
    }

    public static boolean removeDatabase(SEnvironment env,
                                         STransaction txn,
                                         String fileName,
                                         String dbName)
        throws SDatabaseException {
        /* Currently we only support file names. */
        assert fileName != null;
        assert dbName == null;
        try {
            env.removeDatabase(txn, fileName, dbName, false);
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    public static boolean renameDatabase(SEnvironment env,
                                         STransaction txn,
                                         String oldFileName,
                                         String oldDbName,
                                         String newFileName,
                                         String newDbName)
        throws SDatabaseException {
        /* Currently we only support file names. */
        assert oldFileName != null;
        assert newFileName != null;
        assert oldDbName == null;
        assert newDbName == null;
        try {
            File oldFile = new File(oldFileName);
            File newFile = new File(newFileName);
            if(!oldFile.equals(newFile)) {
                env.renameDatabase(txn, oldFileName, null, newFileName, false);
            }
            if (oldDbName != null && !oldDbName.equals(newDbName)) {
                env.renameDatabase(txn, newFileName, oldDbName, newDbName,
                        false);
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    public static SDatabase testOpenDatabase(SEnvironment env,
                                            STransaction txn,
                                            String file,
                                            String name,
                                            SDatabaseConfig config)
        throws SDatabaseException {
        /* Currently we only support file names. */
        assert file != null;
        assert name == null;
        try {
            return env.openDatabase(txn, file, name, config);
        } catch (SDatabaseException e) {
            if (isFileExistsError(e)) {
                assert false;
                return null;
            }
            throw e;
        } catch (IOException e) {
            assert false;
            return null;
        }
    }

    public static SSecondaryDatabase
                  testOpenSecondaryDatabase(SEnvironment env,
                                            STransaction txn,
                                            String file,
                                            String name,
                                            SDatabase primary,
                                            SSecondaryConfig config)
        throws SDatabaseException {
        /* Currently we only support file names. */
        assert file != null;
        assert name == null;
        try {
            return env.openSecondaryDatabase(txn, file, name, primary, config);
        } catch (SDatabaseException e) {
            if (isFileExistsError(e)) {
                assert false;
                return null;
            }
            throw e;
        } catch (IOException e) {
            assert false;
            return null;
        }
    }

    /**
     * Would like to check for errno 17 (EEXIST) but the value may not be
     * standard on all platforms.
     */
    private static boolean isFileExistsError(SDatabaseException e) {
        return (e.getMessage().contains("File exists") ||
            e.getMessage().contains("Do not specify an existing file"));
    }
	
    public static boolean isDalvik(){
	return false;
    }

    public static boolean setImportunate(final STransaction txn,
                                         final boolean importunate) {
        return false;
    }

    public static RuntimeException unexpectedException(Exception cause) {
        if (!(cause instanceof SDatabaseException)) {
            cause = new SDatabaseException(cause);
        }
        throw new RuntimeException(new SDatabaseException(cause));
    }

    public static RuntimeException unexpectedException(String msg,
                                                       Exception cause) {
        if (!(cause instanceof SDatabaseException)) {
            cause = new SDatabaseException(cause);
        }
        throw new RuntimeException(new SDatabaseException(cause));
    }

    public static RuntimeException unexpectedState(String msg) {
        throw new RuntimeException(new SDatabaseException(msg));
    }

    public static RuntimeException unexpectedState() {
        throw new RuntimeException();
    }

    public static boolean hasCaseInsensitiveOnDiskDbFile(){
        return !fSystemCaseSensitive;
    }

    public static void enableDeadlockDetection(SEnvironmentConfig envConfig,
                                               boolean isCDB) {
        envConfig.setLockDetectMode(SLockDetectMode.MAX_WRITE);
    }

    public static class OpResult {

        public static final OpResult SUCCESS =
            new OpResult(SOperationStatus.SUCCESS);

        public static final OpResult FAILURE =
            new OpResult(SOperationStatus.NOTFOUND);

        private SOperationStatus status;

        private OpResult(SOperationStatus result) {
            status = result;
        }

        public boolean isSuccess() {
            return (this == SUCCESS);
        }

        public SOperationStatus status() {
            return status;
        }

        public static OpResult make(SOperationStatus status) {
            return (status == SOperationStatus.SUCCESS) ?
                SUCCESS : FAILURE;
        }
    }

    public static class OpReadOptions {

        public static final OpReadOptions EMPTY =
            new OpReadOptions(null);

        private SLockMode lockMode;

        private OpReadOptions(SLockMode options) {
            lockMode = options;
        }

        public SLockMode getLockMode() {
            return lockMode;
        }

        public static OpReadOptions make(SLockMode lockMode) {
            return (lockMode != null) ?
                new OpReadOptions(lockMode) : EMPTY;
        }
    }

    public static class OpWriteOptions {

        public static final OpWriteOptions EMPTY =
            new OpWriteOptions();

        private OpWriteOptions() {
        }
    }

}
