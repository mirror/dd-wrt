/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.util;

import com.sleepycat.db.BtreeStats;
import com.sleepycat.db.CacheFilePriority;
import com.sleepycat.db.CacheFileStats;
import com.sleepycat.db.CacheStats;
import com.sleepycat.db.CompactStats;
import com.sleepycat.db.CursorConfig;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.DatabaseType;
import com.sleepycat.db.EnvironmentConfig;
import com.sleepycat.db.HashStats;
import com.sleepycat.db.HeapStats;
import com.sleepycat.db.LockDetectMode;
import com.sleepycat.db.LockStats;
import com.sleepycat.db.LogSequenceNumber;
import com.sleepycat.db.LogStats;
import com.sleepycat.db.MutexStats;
import com.sleepycat.db.QueueStats;
import com.sleepycat.db.TransactionStats;
import com.sleepycat.thrift.TCachePriority;
import com.sleepycat.thrift.TCompactConfig;
import com.sleepycat.thrift.TCursorConfig;
import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TDatabaseConfig;
import com.sleepycat.thrift.TDatabaseType;
import com.sleepycat.thrift.TDbt;
import com.sleepycat.thrift.TDurabilityPolicy;
import com.sleepycat.thrift.TEnvironmentConfig;
import com.sleepycat.thrift.TFKDeleteAction;
import com.sleepycat.thrift.TIsolationLevel;
import com.sleepycat.thrift.TLockDetectMode;
import com.sleepycat.thrift.TSecondaryDatabaseConfig;
import com.sleepycat.thrift.TSequenceConfig;
import com.sleepycat.thrift.TTransactionConfig;
import org.junit.Test;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;

public class AdaptersTest {

    @Test
    public void testToBdbTypeEnvironmentConfig() throws Exception {
        Adapters.toBdbType((TEnvironmentConfig) null);
        Adapters.toBdbType(new TEnvironmentConfig());
        Adapters.toBdbType(new TEnvironmentConfig()
                .setLockDetectMode(TLockDetectMode.RANDOM).setAllowCreate(true)
                .setCacheCount(0).setCacheSize(0)
                .setDurability(TDurabilityPolicy.NO_SYNC)
                .setEncryptionKey("key").setMultiversion(true)
                .setRunRecovery(true).setTxnNoWait(true).setTxnSnapshot(true));
    }

    @Test
    public void testToBdbTypeDatabaseConfig() throws Exception {
        Adapters.toBdbType((TDatabaseConfig) null);
        Adapters.toBdbType(new TDatabaseConfig());
        Adapters.toBdbType(new TDatabaseConfig().setAllowCreate(true)
                .setPriority(TCachePriority.LOW).setType(TDatabaseType.BTREE)
                .setBlobThreshold(0).setBtreeMinKey(0)
                .setBtreeRecordNumbers(true).setChecksum(true)
                .setExclusiveCreate(true).setHashFillFactor(0)
                .setHashNumElements(0).setHeapRegionSize(0).setHeapsize(0)
                .setNoWaitDbExclusiveLock(true)
                .setMultiversion(true).setPageSize(0).setQueueExtentSize(0)
                .setQueueInOrder(true).setReadOnly(true)
                .setReadUncommitted(true).setRecordLength(0).setRecordPad(0)
                .setRenumbering(true).setReverseSplitOff(true)
                .setSortedDuplicates(true).setTransactionNotDurable(true)
                .setUnsortedDuplicates(true));
    }

    @Test
    public void testToBdbTypeSecondaryConfig() throws Exception {
        Adapters.toBdbType((TSecondaryDatabaseConfig) null);
        Adapters.toBdbType(new TSecondaryDatabaseConfig());
        Adapters.toBdbType(new TSecondaryDatabaseConfig()
                .setDbConfig(new TDatabaseConfig()));
        Adapters.toBdbType(new TSecondaryDatabaseConfig()
                .setDbConfig(new TDatabaseConfig())
                .setForeignDb(new TDatabase(0L))
                .setForeignKeyDeleteAction(TFKDeleteAction.ABORT)
                .setImmutableSecondaryKey(true));
    }

    @Test
    public void testToBdbTypeTransactionConfig() throws Exception {
        Adapters.toBdbType((TTransactionConfig) null);
        Adapters.toBdbType(new TTransactionConfig());
        Adapters.toBdbType(new TTransactionConfig().setBulk(true).setDurability(
                TDurabilityPolicy.SYNC).setIsoLevel(TIsolationLevel.SNAPSHOT)
                .setWait(true));
    }

    @Test
    public void testToBdbTypeCompactConfig() throws Exception {
        Adapters.toBdbType((TCompactConfig) null);
        Adapters.toBdbType(new TCompactConfig());
        Adapters.toBdbType(new TCompactConfig().setFillPercent(0)
                .setFreeListOnly(true).setFreeSpace(true).setMaxPages(0)
                .setTimeout(0));
    }

    @Test
    public void testToBdbTypeCursorConfig() throws Exception {
        Adapters.toBdbType((TCursorConfig) null);
        Adapters.toBdbType(new TCursorConfig());
        Adapters.toBdbType(new TCursorConfig().setBulkCursor(true).setIsoLevel(
                TIsolationLevel.READ_COMMITTED));
    }

    @Test
    public void testToBdbTypeSequenceConfig() throws Exception {
        Adapters.toBdbType((TSequenceConfig) null);
        Adapters.toBdbType(new TSequenceConfig());
        Adapters.toBdbType(new TSequenceConfig().setAllowCreate(
                true).setAutoCommitNoSync(true).setDecrement(
                true).setCacheSize(0).setExclusiveCreate(true)
                .setInitialValue(0).setMaximum(10).setMinimum(0).setWrap(true));
    }

    @Test
    public void testToBdbTypeDatabaseEntry() throws Exception {
        Adapters.toBdbType((TDbt) null);
        Adapters.toBdbType(new TDbt());
        Adapters.toBdbType(new TDbt().setData("1".getBytes()).setBlob(true)
                .setPartial(true).setPartialLength(1).setPartialOffset(0));
    }

    @Test
    public void testToThriftTypeTEnvironmentConfig() throws Exception {
        EnvironmentConfig config = new EnvironmentConfig();
        config.setLockDetectMode(LockDetectMode.RANDOM);
        Adapters.toThriftType(config);
    }

    @Test
    public void testToThriftTypeTDatabaseConfig() throws Exception {
        DatabaseConfig config = new DatabaseConfig();
        config.setType(DatabaseType.BTREE);
        config.setPriority(CacheFilePriority.DEFAULT);
        Adapters.toThriftType(config);
    }

    @Test
    public void testToThriftTypeTDbt() throws Exception {
        Adapters.toThriftType(new DatabaseEntry(new byte[5]));
        DatabaseEntry entry = new DatabaseEntry(new byte[5]);
        entry.setOffset(2);
        entry.setSize(3);
        Adapters.toThriftType(entry);
    }

    @Test
    public void testToThriftTypeTCursorConfig() throws Exception {
        Adapters.toThriftType(new CursorConfig());
        CursorConfig config = new CursorConfig();
        config.setSnapshot(true);
        Adapters.toThriftType(config);
    }

    @Test
    public void testToThriftTypeTCompactResult() throws Exception {
        Adapters.toThriftType(newInstance(CompactStats.class));
    }

    @Test
    public void testToThriftTypeTCacheStat() throws Exception {
        Adapters.toThriftType(newInstance(CacheStats.class));
    }

    @Test
    public void testToThriftTypeTCacheFileStat() throws Exception {
        Adapters.toThriftType(newInstance(CacheFileStats.class));
    }

    @Test
    public void testToThriftTypeTLockStat() throws Exception {
        Adapters.toThriftType(newInstance(LockStats.class));
    }

    @Test
    public void testToThriftTypeTLogStat() throws Exception {
        Adapters.toThriftType(newInstance(LogStats.class));
    }

    @Test
    public void testToThriftTypeTMutexStat() throws Exception {
        Adapters.toThriftType(newInstance(MutexStats.class));
    }

    @Test
    public void testToThriftTypeTActiveTxnStat() throws Exception {
        Adapters.toThriftType(newActiveTxn());
    }

    @Test
    public void testToThriftTypeTTransactionStat() throws Exception {
        TransactionStats o = newInstance(TransactionStats.class);
        setField(o, "st_last_ckp", new LogSequenceNumber());
        setField(o, "st_txnarray",
                new TransactionStats.Active[]{newActiveTxn(), newActiveTxn()});
        Adapters.toThriftType(o);
    }

    @Test
    public void testToThriftTypeTBtreeStat() throws Exception {
        Adapters.toThriftType(newInstance(BtreeStats.class));
    }

    @Test
    public void testToThriftTypeTHashStat() throws Exception {
        Adapters.toThriftType(newInstance(HashStats.class));
    }

    @Test
    public void testToThriftTypeTHeapStat() throws Exception {
        Adapters.toThriftType(newInstance(HeapStats.class));
    }

    @Test
    public void testToThriftTypeTQueueStat() throws Exception {
        Adapters.toThriftType(newInstance(QueueStats.class));
    }

    private <T> T newInstance(Class<T> clazz) throws Exception {
        Constructor<T> c = clazz.getDeclaredConstructor();
        c.setAccessible(true);
        return c.newInstance();
    }

    private <T> void setField(T obj, String field, Object val) throws Exception {
        Field f = obj.getClass().getDeclaredField(field);
        f.setAccessible(true);
        f.set(obj, val);
    }

    private TransactionStats.Active newActiveTxn() throws Exception {
        TransactionStats.Active o = newInstance(TransactionStats.Active.class);
        setField(o, "lsn", new LogSequenceNumber());
        setField(o, "read_lsn", new LogSequenceNumber());
        return o;
    }
}