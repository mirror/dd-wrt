/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.util;

import com.sleepycat.db.*;
import com.sleepycat.server.util.TEnumMap.Pair;
import com.sleepycat.thrift.*;
import org.apache.thrift.TBase;
import org.apache.thrift.TException;
import org.apache.thrift.TFieldIdEnum;

import java.beans.BeanInfo;
import java.beans.IntrospectionException;
import java.beans.Introspector;
import java.beans.PropertyDescriptor;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.function.BiConsumer;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * Collection of utilities for converting values between Thrift types and BDB
 * types.
 */
public class Adapters {
    private static TEnumMap<TLockDetectMode, LockDetectMode> lockDetectModeMap;
    private static TEnumMap<TCachePriority, CacheFilePriority> cachePriorityMap;
    private static TEnumMap<TDatabaseType, DatabaseType> databaseTypeMap;
    private static TEnumMap<TFKDeleteAction, ForeignKeyDeleteAction> fkDelMap;
    private static TEnumMap<TOperationStatus, OperationStatus> opStatusMap;

    static {
        lockDetectModeMap = new TEnumMap<>(Arrays.asList(
                new Pair<>(TLockDetectMode.EXPIRE, LockDetectMode.EXPIRE),
                new Pair<>(TLockDetectMode.MAX_LOCKS, LockDetectMode.MAXLOCKS),
                new Pair<>(TLockDetectMode.MAX_WRITE, LockDetectMode.MAXWRITE),
                new Pair<>(TLockDetectMode.MIN_LOCKS, LockDetectMode.MINLOCKS),
                new Pair<>(TLockDetectMode.MIN_WRITE, LockDetectMode.MINWRITE),
                new Pair<>(TLockDetectMode.OLDEST, LockDetectMode.OLDEST),
                new Pair<>(TLockDetectMode.RANDOM, LockDetectMode.RANDOM),
                new Pair<>(TLockDetectMode.YOUNGEST, LockDetectMode.YOUNGEST)));

        cachePriorityMap = new TEnumMap<>(Arrays.asList(
                new Pair<>(TCachePriority.VERY_LOW, CacheFilePriority.VERY_LOW),
                new Pair<>(TCachePriority.LOW, CacheFilePriority.LOW),
                new Pair<>(TCachePriority.DEFAULT, CacheFilePriority.DEFAULT),
                new Pair<>(TCachePriority.HIGH, CacheFilePriority.HIGH),
                new Pair<>(TCachePriority.VERY_HIGH,
                        CacheFilePriority.VERY_HIGH)));

        databaseTypeMap = new TEnumMap<>(Arrays.asList(
                new Pair<>(TDatabaseType.BTREE, DatabaseType.BTREE),
                new Pair<>(TDatabaseType.HASH, DatabaseType.HASH),
                new Pair<>(TDatabaseType.HEAP, DatabaseType.HEAP),
                new Pair<>(TDatabaseType.QUEUE, DatabaseType.QUEUE),
                new Pair<>(TDatabaseType.RECNO, DatabaseType.RECNO)));

        fkDelMap = new TEnumMap<>(Arrays.asList(
                new Pair<>(TFKDeleteAction.ABORT, ForeignKeyDeleteAction.ABORT),
                new Pair<>(TFKDeleteAction.CASCADE,
                        ForeignKeyDeleteAction.CASCADE),
                new Pair<>(TFKDeleteAction.NULLIFY,
                        ForeignKeyDeleteAction.NULLIFY)));

        opStatusMap = new TEnumMap<>(Arrays.asList(
                new Pair<>(TOperationStatus.KEY_EMPTY,
                        OperationStatus.KEYEMPTY),
                new Pair<>(TOperationStatus.KEY_EXIST,
                        OperationStatus.KEYEXIST),
                new Pair<>(TOperationStatus.NOT_FOUND,
                        OperationStatus.NOTFOUND),
                new Pair<>(TOperationStatus.SUCCESS, OperationStatus.SUCCESS)));
    }

    private static LockDetectMode toBdbType(TLockDetectMode mode) {
        return lockDetectModeMap.toBdb(mode);
    }

    public static CacheFilePriority toBdbType(TCachePriority priority) {
        return cachePriorityMap.toBdb(priority);
    }

    private static DatabaseType toBdbType(TDatabaseType type) {
        return databaseTypeMap.toBdb(type);
    }

    public static EnvironmentConfig toBdbType(TEnvironmentConfig config) {
        return update(new EnvironmentConfig(), config);
    }

    public static EnvironmentConfig update(EnvironmentConfig config,
            TEnvironmentConfig tConfig) {
        Map<TEnvironmentConfig._Fields, BiConsumer<EnvironmentConfig, Object>>
                map = new HashMap<>();
        map.put(TEnvironmentConfig._Fields.DURABILITY, (c, d) -> {
            c.setTxnNoSync(d == TDurabilityPolicy.NO_SYNC);
            c.setTxnWriteNoSync(d == TDurabilityPolicy.WRITE_NO_SYNC);
        });
        map.put(TEnvironmentConfig._Fields.ENCRYPTION_KEY,
                (c, k) -> c.setEncrypted((String) k));
        map.put(TEnvironmentConfig._Fields.LOCK_DETECT_MODE,
                (c, l) -> c.setLockDetectMode(toBdbType((TLockDetectMode) l)));

        return updateBdbObject(config, tConfig,
                TEnvironmentConfig._Fields.values(), map);
    }

    public static DatabaseConfig toBdbType(TDatabaseConfig tConfig) {
        return update(new DatabaseConfig(), tConfig);
    }

    public static DatabaseConfig update(DatabaseConfig config,
            TDatabaseConfig tConfig) {
        Map<TDatabaseConfig._Fields, BiConsumer<DatabaseConfig, Object>> map =
                new HashMap<>();
        map.put(TDatabaseConfig._Fields.HEAP_REGION_SIZE,
                (c, s) -> c.setHeapRegionSize((int) s));
        map.put(TDatabaseConfig._Fields.PRIORITY,
                (c, p) -> c.setPriority(toBdbType((TCachePriority) p)));
        map.put(TDatabaseConfig._Fields.TYPE,
                (c, t) -> c.setType(toBdbType((TDatabaseType) t)));

        return updateBdbObject(config, tConfig,
                TDatabaseConfig._Fields.values(), map);
    }

    public static SecondaryConfig toBdbType(TSecondaryDatabaseConfig tConfig) {
        Map<TSecondaryDatabaseConfig._Fields, BiConsumer<SecondaryConfig, Object>>
                map = new HashMap<>();
        map.put(TSecondaryDatabaseConfig._Fields.DB_CONFIG,
                (c, dc) -> update(c, (TDatabaseConfig) dc));
        map.put(TSecondaryDatabaseConfig._Fields.FOREIGN_DB, (c, d) -> {
        });
        map.put(TSecondaryDatabaseConfig._Fields.FOREIGN_KEY_DELETE_ACTION,
                (c, a) -> c.setForeignKeyDeleteAction(
                        fkDelMap.toBdb((TFKDeleteAction) a)));

        return updateBdbObject(new SecondaryConfig(), tConfig,
                TSecondaryDatabaseConfig._Fields.values(), map);
    }

    public static TransactionConfig toBdbType(TTransactionConfig config) {
        Map<TTransactionConfig._Fields, BiConsumer<TransactionConfig, Object>>
                map = new HashMap<>();
        map.put(TTransactionConfig._Fields.DURABILITY, (c, d) -> {
            c.setNoSync(d == TDurabilityPolicy.NO_SYNC);
            c.setSync(d == TDurabilityPolicy.SYNC);
            c.setWriteNoSync(d == TDurabilityPolicy.WRITE_NO_SYNC);
        });
        map.put(TTransactionConfig._Fields.ISO_LEVEL, (c, level) -> {
            c.setReadCommitted(level == TIsolationLevel.READ_COMMITTED);
            c.setReadUncommitted(level == TIsolationLevel.READ_UNCOMMITTED);
            c.setSnapshot(level == TIsolationLevel.SNAPSHOT);
        });

        return updateBdbObject(new TransactionConfig(), config,
                TTransactionConfig._Fields.values(), map);
    }

    public static CompactConfig toBdbType(TCompactConfig config) {
        return updateBdbObject(new CompactConfig(), config,
                TCompactConfig._Fields.values(), null);
    }

    public static CursorConfig toBdbType(TCursorConfig config) {
        Map<TCursorConfig._Fields, BiConsumer<CursorConfig, Object>> map =
                new HashMap<>();
        map.put(TCursorConfig._Fields.ISO_LEVEL, (c, level) -> {
            c.setReadCommitted(level == TIsolationLevel.READ_COMMITTED);
            c.setReadUncommitted(level == TIsolationLevel.READ_UNCOMMITTED);
            c.setSnapshot(level == TIsolationLevel.SNAPSHOT);
        });

        return updateBdbObject(new CursorConfig(), config,
                TCursorConfig._Fields.values(), map);
    }

    public static SequenceConfig toBdbType(TSequenceConfig config) {
        Map<TSequenceConfig._Fields, BiConsumer<SequenceConfig, Object>> map =
                new HashMap<>();
        map.put(TSequenceConfig._Fields.MAXIMUM,
                (c, max) -> c.setRange(c.getRangeMin(), (long) max));
        map.put(TSequenceConfig._Fields.MINIMUM,
                (c, min) -> c.setRange((long) min, c.getRangeMax()));

        return updateBdbObject(new SequenceConfig(), config,
                TSequenceConfig._Fields.values(), map);
    }

    public static DatabaseEntry toBdbType(TDbt dbt) {
        return updateBdbObject(new DatabaseEntry(), dbt, TDbt._Fields.values(),
                null);
    }

    private static TLockDetectMode toThriftType(LockDetectMode mode) {
        return lockDetectModeMap.toThrift(mode);
    }

    public static TCachePriority toThriftType(CacheFilePriority priority) {
        return cachePriorityMap.toThrift(priority);
    }

    private static TDatabaseType toThriftType(DatabaseType type) {
        return databaseTypeMap.toThrift(type);
    }

    public static TEnvironmentConfig toThriftType(EnvironmentConfig config) {
        Map<TEnvironmentConfig._Fields, Function<EnvironmentConfig, ?>> map =
                new HashMap<>();
        map.put(TEnvironmentConfig._Fields.ENCRYPTION_KEY, c -> null);
        map.put(TEnvironmentConfig._Fields.LOCK_DETECT_MODE,
                c -> toThriftType(c.getLockDetectMode()));
        map.put(TEnvironmentConfig._Fields.DURABILITY, c -> {
            if (config.getTxnNoSync())
                return TDurabilityPolicy.NO_SYNC;
            if (c.getTxnWriteNoSync())
                return TDurabilityPolicy.WRITE_NO_SYNC;
            else
                return TDurabilityPolicy.SYNC;
        });
        return updateThriftObject(new TEnvironmentConfig(), config,
                TEnvironmentConfig._Fields.values(), map);
    }

    public static TDatabaseConfig toThriftType(DatabaseConfig config) {
        Map<TDatabaseConfig._Fields, Function<DatabaseConfig, ?>> map =
                new HashMap<>();
        map.put(TDatabaseConfig._Fields.HEAP_REGION_SIZE,
                c -> (int) c.getHeapRegionSize());
        map.put(TDatabaseConfig._Fields.PRIORITY,
                c -> toThriftType(c.getPriority()));
        map.put(TDatabaseConfig._Fields.TYPE, c -> toThriftType(c.getType()));

        return updateThriftObject(new TDatabaseConfig(), config,
                TDatabaseConfig._Fields.values(), map);
    }

    public static TDbt toThriftType(DatabaseEntry item) {
        TDbt tValue = new TDbt();
        if (item.getData() == null) {
            return tValue.setData((byte[]) null);
        } else {
            int from = item.getOffset();
            int to = from + item.getSize();
            return tValue.setData(Arrays.copyOfRange(item.getData(), from, to));
        }
    }

    public static TOperationStatus toThriftType(OperationStatus status) {
        return opStatusMap.toThrift(status);
    }

    public static TKeyRangeResult toThriftType(KeyRange range) {
        return new TKeyRangeResult()
                .setEqual(range.equal)
                .setGreater(range.greater)
                .setLess(range.less);
    }

    public static TCursorConfig toThriftType(CursorConfig config) {
        Map<TCursorConfig._Fields, Function<CursorConfig, ?>> map =
                new HashMap<>();
        map.put(TCursorConfig._Fields.ISO_LEVEL, c -> {
            if (c.getReadCommitted()) return TIsolationLevel.READ_COMMITTED;
            if (c.getReadUncommitted()) return TIsolationLevel.READ_UNCOMMITTED;
            if (c.getSnapshot()) return TIsolationLevel.SNAPSHOT;
            return null;
        });
        return updateThriftObject(new TCursorConfig(), config,
                TCursorConfig._Fields.values(), map);
    }

    public static TCompactResult toThriftType(CompactStats stats) {
        return updateThriftObject(new TCompactResult(), stats,
                TCompactResult._Fields.values(),
                Collections.singletonMap(TCompactResult._Fields.END_KEY,
                        s -> null));
    }

    public static TCacheStat toThriftType(CacheStats stats) {
        return updateThriftObject(new TCacheStat(), stats,
                TCacheStat._Fields.values(),
                Collections.singletonMap(TCacheStat._Fields.PAGE_MAPPED,
                        CacheStats::getMap));
    }

    public static TCacheFileStat toThriftType(CacheFileStats stats) {
        return updateThriftObject(new TCacheFileStat(), stats,
                TCacheFileStat._Fields.values(),
                Collections.singletonMap(TCacheFileStat._Fields.PAGE_MAPPED,
                        CacheFileStats::getMap));
    }

    public static TLockStat toThriftType(LockStats stats) {
        return updateThriftObject(new TLockStat(), stats,
                TLockStat._Fields.values(), null);
    }

    public static TLogStat toThriftType(LogStats stats) {
        return updateThriftObject(new TLogStat(), stats,
                TLogStat._Fields.values(), null);
    }

    public static TMutexStat toThriftType(MutexStats stats) {
        return updateThriftObject(new TMutexStat(), stats,
                TMutexStat._Fields.values(), null);
    }

    public static TActiveTxnStat toThriftType(TransactionStats.Active stats) {
        Map<TActiveTxnStat._Fields, Function<TransactionStats.Active, ?>> map =
                new HashMap<>();
        map.put(TActiveTxnStat._Fields.LSN_FILE, s -> s.getLsn().getFile());
        map.put(TActiveTxnStat._Fields.LSN_OFFSET, s -> s.getLsn().getOffset());
        map.put(TActiveTxnStat._Fields.READ_LSN_FILE,
                s -> s.getReadLsn().getFile());
        map.put(TActiveTxnStat._Fields.READ_LSN_OFFSET,
                s -> s.getReadLsn().getOffset());

        return updateThriftObject(new TActiveTxnStat(), stats,
                TActiveTxnStat._Fields.values(), map);
    }

    public static TTransactionStat toThriftType(TransactionStats stats) {
        Map<TTransactionStat._Fields, Function<TransactionStats, ?>> map =
                new HashMap<>();
        map.put(TTransactionStat._Fields.LAST_CKP_FILE,
                s -> s.getLastCkp().getFile());
        map.put(TTransactionStat._Fields.LAST_CKP_OFFSET,
                s -> s.getLastCkp().getOffset());
        map.put(TTransactionStat._Fields.ACTIVE_TXNS,
                s -> Arrays.stream(s.getTxnarray()).map(Adapters::toThriftType)
                        .collect(Collectors.toList()));

        return updateThriftObject(new TTransactionStat(), stats,
                TTransactionStat._Fields.values(), map);
    }

    public static TDatabaseStatResult toThriftType(DatabaseStats stats) {
        TDatabaseStatResult tValue = new TDatabaseStatResult();
        if (stats instanceof BtreeStats)
            return tValue.setBtreeStat(toThriftType((BtreeStats) stats));
        if (stats instanceof HashStats)
            return tValue.setHashStat(toThriftType((HashStats) stats));
        if (stats instanceof HeapStats)
            return tValue.setHeapStat(toThriftType((HeapStats) stats));
        if (stats instanceof QueueStats)
            return tValue.setQueueStat(toThriftType((QueueStats) stats));
        return tValue;
    }

    public static TBtreeStat toThriftType(BtreeStats stats) {
        return updateThriftObject(new TBtreeStat(), stats,
                TBtreeStat._Fields.values(), null);
    }

    public static THashStat toThriftType(HashStats stats) {
        return updateThriftObject(new THashStat(), stats,
                THashStat._Fields.values(), null);
    }

    public static THeapStat toThriftType(HeapStats stats) {
        return updateThriftObject(new THeapStat(), stats,
                THeapStat._Fields.values(), null);
    }

    public static TQueueStat toThriftType(QueueStats stats) {
        return updateThriftObject(new TQueueStat(), stats,
                TQueueStat._Fields.values(), null);
    }

    public static TException toThriftType(Exception e) {
        if (e instanceof RuntimeException) {
            return new TRuntimeException(e.getClass().getName(),
                    e.getMessage());
        }

        if (e instanceof DatabaseException) {
            TDatabaseExceptionType type = TDatabaseExceptionType.DATABASE;
            if (e instanceof LockNotGrantedException)
                type = TDatabaseExceptionType.LOCK_NOT_GRANTED;
            else if (e instanceof DeadlockException)
                type = TDatabaseExceptionType.DEADLOCK;
            else if (e instanceof HeapFullException)
                type = TDatabaseExceptionType.HEAP_FULL;
            else if (e instanceof MetaCheckSumFailException)
                type = TDatabaseExceptionType.META_CHECKSUM_FAIL;
            else if (e instanceof RunRecoveryException)
                type = TDatabaseExceptionType.RUN_RECOVERY;
            else if (e instanceof VersionMismatchException)
                type = TDatabaseExceptionType.VERSION_MISMATCH;

            TDatabaseException dbe =
                    new TDatabaseException(type, e.getMessage());
            dbe.setErrorNumber(((DatabaseException) e).getErrno());
            return dbe;
        }

        if (e instanceof FileNotFoundException)
            return new TFileNotFoundException(e.getMessage());
        if (e instanceof IOException)
            return new TIOException(e.getMessage());
        if (e instanceof TException)
            return (TException) e;

        throw new IllegalArgumentException("Unknown exception type.", e);
    }

    private static <F extends TFieldIdEnum, B> B updateBdbObject(B bdbObj,
            TBase<?, F> tObj, F[] fields,
            Map<F, BiConsumer<B, Object>> setters) {
        if (tObj == null)
            return bdbObj;
        try {
            Map<String, PropertyDescriptor> properties = getProperties(bdbObj);
            for (F f : fields) {
                if (tObj.isSet(f)) {
                    Object value = tObj.getFieldValue(f);
                    if (setters != null && setters.containsKey(f)) {
                        setters.get(f).accept(bdbObj, value);
                    } else {
                        PropertyDescriptor p = properties.get(f.getFieldName());
                        p.getWriteMethod().invoke(bdbObj, value);
                    }
                }
            }
            return bdbObj;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private static <T extends TBase<?, F>, F extends TFieldIdEnum, B>
    T updateThriftObject(
            T tObj, B bdbObj, F[] fields, Map<F, Function<B, ?>> valueMappers) {
        try {
            Map<String, PropertyDescriptor> properties = getProperties(bdbObj);
            for (F f : fields) {
                Object value;
                if (valueMappers != null && valueMappers.containsKey(f)) {
                    value = valueMappers.get(f).apply(bdbObj);
                } else {
                    PropertyDescriptor p = properties.get(f.getFieldName());
                    value = p.getReadMethod().invoke(bdbObj);
                }
                tObj.setFieldValue(f, value);
            }
            return tObj;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private static Map<String, PropertyDescriptor> getProperties(Object obj)
            throws IntrospectionException {
        BeanInfo beanInfo = Introspector.getBeanInfo(obj.getClass());
        return Arrays.stream(beanInfo.getPropertyDescriptors())
                .collect(Collectors.toMap(PropertyDescriptor::getName,
                        Function.identity()));
    }
}
