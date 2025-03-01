/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

namespace java com.sleepycat.thrift


/* Protocol version. */
const string PROTOCOL_VERSION = "1.1.0";


/***************************
 *  Data Type Definitions  *
 ***************************/

/*
 * The result type of client protocol version check.
 */
struct TProtocolVersionTestResult {
    // If the protocol version given by the client is supported.
    1: bool supported;
    // The server's protocol version.
    2: string serverProtocolVersion;
    // The server's native byte order
    3: bool serverBigEndian;
    // Error message, if the client protocol version is not supported.
    4: optional string message;
}

enum TLockDetectMode {
    EXPIRE = 1,
    MAX_LOCKS = 2,
    MAX_WRITE = 3,
    MIN_LOCKS = 4,
    MIN_WRITE = 5,
    OLDEST = 6,
    RANDOM = 7,
    YOUNGEST = 8
}

/*
 * The remote environment handle type.
 */
struct TEnvironment {
    // The remote handle value that can be resolved to a local environment
    // handle on server.
    1: i64 handle;
}

struct TEnvironmentConfig {
    1: optional bool allowCreate;
    2: optional i32 cacheCount;
    3: optional i64 cacheSize;
    4: optional string encryptionKey;
    5: optional TLockDetectMode lockDetectMode;
    6: optional bool multiversion;
    7: optional bool runRecovery;
    8: optional bool txnNoWait;
    9: optional bool txnSnapshot;
    10: optional TDurabilityPolicy durability;
}

enum TCachePriority {
    VERY_LOW = 1,
    LOW = 2,
    DEFAULT = 3,
    HIGH = 4,
    VERY_HIGH = 5
}

enum TDatabaseType {
    BTREE = 1,
    HASH = 2,
    HEAP = 3,
    QUEUE = 4,
    RECNO = 5
}

enum TFKDeleteAction {
    ABORT = 1,
    CASCADE = 2,
    NULLIFY = 3
}

/*
 * The remote database handle type.
 */
struct TDatabase {
    1: i64 handle;
}

struct TDatabaseConfig {
    1: optional bool allowCreate;
    2: optional i32 blobThreshold;
    3: optional i32 btreeMinKey;
    4: optional bool btreeRecordNumbers;
    5: optional bool checksum;
    6: optional bool exclusiveCreate;
    7: optional i32 hashFillFactor;
    8: optional i32 hashNumElements;
    9: optional i32 heapRegionSize;
    10: optional i64 heapsize;
    11: optional bool multiversion;
    12: optional bool noWaitDbExclusiveLock;
    13: optional i32 pageSize;
    14: optional TCachePriority priority;
    15: optional i32 queueExtentSize;
    16: optional bool queueInOrder;
    17: optional bool readOnly;
    18: optional bool readUncommitted;
    19: optional i32 recordLength;
    20: optional i32 recordPad;
    21: optional bool renumbering;
    22: optional bool reverseSplitOff;
    23: optional bool sortedDuplicates;
    24: optional bool transactionNotDurable;
    25: optional TDatabaseType type;
    26: optional bool unsortedDuplicates;
}

struct TSecondaryDatabaseConfig {
    1: optional TDatabaseConfig dbConfig;
    2: optional TDatabase foreignDb;
    3: optional TFKDeleteAction foreignKeyDeleteAction;
    4: optional bool immutableSecondaryKey;
}

/*
 * The remote cursor handle type.
 */
struct TCursor {
    1: i64 handle;
}

struct TCursorConfig {
    1: optional bool bulkCursor;
    2: optional TIsolationLevel isoLevel;
}

/*
 * The remote join cursor handle type.
 */
struct TJoinCursor {
    1: i64 handle;
}

enum TIsolationLevel {
    READ_COMMITTED = 1,
    READ_UNCOMMITTED = 2,
    SNAPSHOT = 3
}

enum TDurabilityPolicy {
    NO_SYNC = 1,
    SYNC = 2,
    WRITE_NO_SYNC = 3
}

/*
 * The remote transaction handle type.
 */
struct TTransaction {
    1: i64 handle;
}

struct TTransactionConfig {
    1: optional bool bulk;
    2: optional TDurabilityPolicy durability;
    3: optional TIsolationLevel isoLevel;
    4: optional bool wait;
}

/*
 * The remote sequnce handle type.
 */
struct TSequence {
    1: i64 handle;
}

struct TSequenceConfig {
    1: optional bool allowCreate;
    2: optional bool autoCommitNoSync;
    3: optional i32 cacheSize;
    4: optional bool decrement;
    5: optional bool exclusiveCreate;
    6: optional i64 initialValue;
    7: optional i64 minimum;
    8: optional i64 maximum;
    9: optional bool wrap;
}

struct TDbt {
    1: optional binary data;
    2: optional i32 partialLength;
    3: optional i32 partialOffset;
    4: optional bool partial;
    5: optional bool blob;
}

/*
 * The type for a single key/data pair.
 */
struct TKeyData {
    1: optional TDbt key;
    2: optional TDbt data;
}

/*
 * The type for a secondary key/data pair with the corresponding primary key.
 */
struct TKeyDataWithPKey {
    // The secondary index
    1: optional TDbt skey;
    // The primary key
    2: optional TDbt pkey;
    // The data from the primary database
    3: optional TDbt pdata;
}

/*
 * The type for a single key/data pair with its secondary keys.
 *
 * Because we cannot use remote callbacks to generate secondary keys for an
 * item in a primary database, we have to generate them locally at the client
 * side and pass them to the server.
 *
 * This type serves this purpose by binding the item and its secondary keys
 * together as a single object, and is used in all 'put' methods.
 */
struct TKeyDataWithSecondaryKeys {
    1: optional TDbt pkey;
    2: optional TDbt pdata;
    3: optional map<TDatabase, list<TDbt>> skeys;
}

/*
 * The error return code for data access methods (get, put, etc.).
 */
enum TOperationStatus {
    KEY_EMPTY = 1,
    KEY_EXIST = 2,
    NOT_FOUND = 3,
    SUCCESS = 4
}

/*
 * The action performed by dbGet and dbGetWithPKey.
 */
enum TDbGetMode {
    CONSUME = 1,
    CONSUME_WAIT = 2,
    DEFAULT = 3,
    GET_BOTH = 4,
    SET_RECNO = 5
}

/*
 * The flags for dbGet and dbGetWithPKey.
 */
struct TDbGetConfig {
    1: TDbGetMode mode;
    2: optional bool multiple;
    3: optional bool rmw;
    4: optional TIsolationLevel isoLevel;
}

enum TCursorGetMode {
    CURRENT = 1,
    FIRST = 2,
    GET_BOTH = 3,
    GET_BOTH_RANGE = 4,
    GET_RECNO = 5,
    LAST = 6,
    NEXT = 7,
    NEXT_DUP = 8,
    NEXT_NO_DUP = 9,
    PREV = 10,
    PREV_DUP = 11,
    PREV_NO_DUP = 12,
    SET = 13,
    SET_RANGE = 14,
    SET_RECNO = 15
}

/*
 * The flags for cursorGet and cursorGetWithPKey.
 */
struct TCursorGetConfig {
    1: TCursorGetMode mode;
    2: optional bool multiple;
    3: optional bool multiKey;
    4: optional i32 batchSize;
    5: optional bool rmw;
    6: optional TIsolationLevel isoLevel;
}

/*
 * The flags for joinCursorGet.
 */
struct TJoinCursorGetConfig {
    1: TKeyData pair;
    2: optional bool readUncommitted;
    3: optional bool rmw;
}

/*
 * The result type for dbGet and cursorGet.
 *
 *
 */
struct TGetResult {
    1: TOperationStatus status;
    2: optional list<TKeyData> pairs;
}

/*
 * The result type for dbGetWithPKey and cursorGetWithPKey.
 */
struct TGetWithPKeyResult {
    1: TOperationStatus status;
    2: optional TKeyDataWithPKey tuple;
}

/*
 * The flags for dbPut.
 */
enum TDbPutConfig {
    APPEND = 1,
    DEFAULT = 2,
    NO_DUP_DATA = 3,
    NO_OVERWRITE = 4,
    OVERWRITE_DUP = 5
}

/*
 * The result type for dbPut.
 */
struct TPutResult {
    1: TOperationStatus status;
    2: optional binary newRecordNumber;
}

/*
 * The flags for cursorPut.
 */
enum TCursorPutConfig {
    AFTER = 1,
    BEFORE = 2,
    CURRENT = 3,
    DEFAULT = 4,
    KEY_FIRST = 5,
    KEY_LAST = 6,
    NO_DUP_DATA = 7,
    NO_OVERWRITE = 8
}

/*
 * The result type for dbKeyRange.
 */
struct TKeyRangeResult {
    1: optional double equal;
    2: optional double greater;
    3: optional double less;
}

/*
 * The configuration parameters for database compaction.
 */
struct TCompactConfig {
    1: optional i32 fillPercent;
    2: optional bool freeListOnly;
    3: optional bool freeSpace;
    4: optional i32 maxPages;
    5: optional i32 timeout;
}

/*
 * The result type for dbCompact.
 */
struct TCompactResult {
    1: optional i32 deadlock;
    2: optional i32 emptyBuckets;
    3: optional i32 levels;
    4: optional i32 pagesExamine;
    5: optional i32 pagesFree;
    6: optional i32 pagesTruncated;
    7: optional TDbt endKey;
}

struct TCacheFileStat {
    1: i64 backupSpins;
    2: i64 cacheHit;
    3: i64 cacheMiss;
    4: string fileName;
    5: i32 pageSize;
    6: i64 pageCreate;
    7: i32 pageMapped;
    8: i64 pageIn;
    9: i64 pageOut;
}

struct TCacheStat {
    1: i64 alloc;
    2: i64 allocBuckets;
    3: i64 allocMaxBuckets;
    4: i64 allocMaxPages;
    5: i64 allocPages;
    6: i32 bytes;
    7: i64 cacheHit;
    8: i64 cacheMiss;
    9: i32 gbytes;
    10: i32 hashBuckets;
    11: i64 hashExamined;
    12: i32 hashLongest;
    13: i64 hashMaxNowait;
    14: i64 hashMaxWait;
    15: i32 hashMutexes;
    16: i64 hashNowait;
    17: i32 hashSearches;
    18: i64 hashWait;
    19: i64 ioWait;
    20: i32 maxNumCache;
    21: i32 maxOpenfd;
    22: i32 maxWrite;
    23: i32 maxWriteSleep;
    24: i64 mmapSize;
    25: i64 multiversionFreed;
    26: i64 multiversionFrozen;
    27: i64 multiversionReused;
    28: i64 multiversionThawed;
    29: i32 numCache;
    30: i32 pages;
    31: i32 pageSize;
    32: i32 pageClean;
    33: i64 pageCreate;
    34: i32 pageDirty;
    35: i32 pageMapped;
    36: i64 pageIn;
    37: i64 pageTrickle;
    38: i64 pageOut;
    39: i64 regionNowait;
    40: i64 regionWait;
    41: i64 regmax;
    42: i64 regSize;
    43: i64 roEvict;
    44: i64 rwEvict;
    45: i64 syncInterrupted;
}

struct TLockStat {
    1: i32 curMaxId;
    2: i32 hashLen;
    3: i32 id;
    4: i32 initlockers;
    5: i32 initlocks;
    6: i32 initobjects;
    7: i32 lockers;
    8: i64 lockersNowait;
    9: i64 lockersWait;
    10: i64 lockNowait;
    11: i32 locks;
    12: i64 locksteals;
    13: i32 lockTimeout;
    14: i64 lockWait;
    15: i32 maxHlocks;
    16: i32 maxHobjects;
    17: i32 maxLockers;
    18: i32 maxLocks;
    19: i64 maxLsteals;
    20: i32 maxNlockers;
    21: i32 maxNlocks;
    22: i32 maxNobjects;
    23: i32 maxObjects;
    24: i64 maxOsteals;
    25: i64 numDeadlocks;
    26: i64 numDowngrade;
    27: i32 numLockers;
    28: i64 numLockersHit;
    29: i64 numLockersReused;
    30: i32 numLocks;
    31: i64 numLockTimeouts;
    32: i32 numModes;
    33: i32 nobjects;
    34: i64 numReleases;
    35: i64 numRequests;
    36: i64 numTxnTimeouts;
    37: i64 numUpgrade;
    38: i32 objects;
    39: i64 objectsteals;
    40: i64 objsNowait;
    41: i64 objsWait;
    42: i32 partitions;
    43: i64 partMaxNowait;
    44: i64 partMaxWait;
    45: i64 partNowait;
    46: i64 partWait;
    47: i64 regionNowait;
    48: i64 regionWait;
    49: i64 regSize;
    50: i32 tableSize;
    51: i32 txnTimeout;
}

struct TLogStat {
    1: i32 curFile;
    2: i32 curOffset;
    3: i32 diskFile;
    4: i32 diskOffset;
    5: i32 fileidInit;
    6: i32 lgBSize;
    7: i32 lgSize;
    8: i32 magic;
    9: i32 maxCommitperflush;
    10: i32 maxNfileId;
    11: i32 minCommitperflush;
    12: i32 mode;
    13: i32 numFileId;
    14: i64 RCount;
    15: i64 record;
    16: i64 regionNowait;
    17: i64 regionWait;
    18: i64 regSize;
    19: i64 SCount;
    20: i32 version;
    21: i32 WBytes;
    22: i32 wcBytes;
    23: i32 wcMbytes;
    24: i32 WMbytes;
    25: i64 WCount;
    26: i64 WCountFill;
}

struct TMutexStat {
    1: i32 mutexAlign;
    2: i32 mutexCount;
    3: i32 mutexFree;
    4: i32 mutexInit;
    5: i32 mutexInuse;
    6: i32 mutexInuseMax;
    7: i32 mutexMax;
    8: i32 mutexTasSpins;
    9: i64 regionNowait;
    10: i64 regionWait;
    11: i64 regmax;
    12: i64 regSize;
}

struct TActiveTxnStat {
    1: binary GId;
    2: i32 lsnFile;
    3: i32 lsnOffset;
    4: i32 multiversionRef;
    5: string name;
    6: i32 parentId;
    7: i32 pid;
    8: i32 priority;
    9: i32 readLsnFile;
    10: i32 readLsnOffset;
    11: i32 txnId;
}

struct TTransactionStat {
    1: i32 inittxns;
    2: i32 lastCkpFile;
    3: i32 lastCkpOffset;
    4: i32 lastTxnId;
    5: i32 maxNactive;
    6: i32 maxNsnapshot;
    7: i32 maxTxns;
    8: i64 naborts;
    9: i32 nactive;
    10: i64 numBegins;
    11: i64 numCommits;
    12: i32 numRestores;
    13: i32 numSnapshot;
    14: i64 regionNowait;
    15: i64 regionWait;
    16: i64 regSize;
    17: i64 timeCkp;
    18: list<TActiveTxnStat> activeTxns;
}


/*
 * The flags for each type of statistics.
 */
enum TEnvStatOption {
    // Return the statistics and reset it.
    CLEAR = 1,
    // Return the statistics.
    INCLUDE = 2,
    // Do not return the statistics.
    EXCLUDE = 3
}

struct TEnvStatConfig {
    // The options for each subsystem.
    1: optional TEnvStatOption cache;
    2: optional TEnvStatOption cacheFile;
    3: optional TEnvStatOption lock;
    4: optional TEnvStatOption log;
    5: optional TEnvStatOption mutex;
    6: optional TEnvStatOption transaction;
}

struct TEnvStatResult {
    1: optional TCacheStat cacheStat;
    2: optional list<TCacheFileStat> cacheFileStat;
    3: optional TLockStat lockStat;
    4: optional TLogStat logStat;
    5: optional TMutexStat mutexStat;
    6: optional TTransactionStat txnStat;
}

struct TBtreeStat {
    1: i32 dupPages;
    2: i64 dupPagesFree;
    3: i32 emptyPages;
    4: i32 free;
    5: i32 intPages;
    6: i64 intPagesFree;
    7: i32 leafPages;
    8: i64 leafPagesFree;
    9: i32 levels;
    10: i32 magic;
    11: i32 metaFlags;
    12: i32 minKey;
    13: i32 numBlobs;
    14: i32 numData;
    15: i32 numKeys;
    16: i32 overPages;
    17: i64 overPagesFree;
    18: i32 pageCount;
    19: i32 pageSize;
    20: i32 reLen;
    21: i32 rePad;
    22: i32 version;
}

struct THashStat {
    1: i64 BFree;
    2: i64 bigBFree;
    3: i32 bigPages;
    4: i32 buckets;
    5: i32 dup;
    6: i64 dupFree;
    7: i32 ffactor;
    8: i32 free;
    9: i32 magic;
    10: i32 metaFlags;
    11: i32 numBlobs;
    12: i32 numData;
    13: i32 numKeys;
    14: i32 overflows;
    15: i64 ovflFree;
    16: i32 pageCount;
    17: i32 pageSize;
    18: i32 version;
}

struct THeapStat {
    1: i32 heapMagic;
    2: i32 heapMetaFlags;
    3: i32 heapNumBlobs;
    4: i32 heapNumRecs;
    5: i32 heapNumRegions;
    6: i32 heapPageCount;
    7: i32 heapPageSize;
    8: i32 heapRegionSize;
    9: i32 heapVersion;
}

struct TQueueStat {
    1: i32 curRecno;
    2: i32 extentSize;
    3: i32 firstRecno;
    4: i32 magic;
    5: i32 metaFlags;
    6: i32 numData;
    7: i32 numKeys;
    8: i32 pages;
    9: i32 pagesFree;
    10: i32 pageSize;
    11: i32 reLen;
    12: i32 rePad;
    13: i32 version;
}

struct TDatabaseStatResult {
    1: optional TBtreeStat btreeStat;
    2: optional THashStat hashStat;
    3: optional THeapStat heapStat;
    4: optional TQueueStat queueStat;
}


/***************************
 *  Exception Definitions  *
 ***************************/

/*
 * The types of subclasses of DatabaseException. Leave some gaps between values
 * so we can insert new types in future.
 */
enum TDatabaseExceptionType {
    DATABASE = 0,
    DEADLOCK = 10,
    HEAP_FULL = 20,
    LOCK_NOT_GRANTED = 30,
    META_CHECKSUM_FAIL = 40,
    RUN_RECOVERY = 50,
    VERSION_MISMATCH = 60
}

/*
 * Exception type for all DatabaseExceptions.
 */
exception TDatabaseException {
    // The actual subclass of the exception.
    1: TDatabaseExceptionType type;
    // The error message.
    2: string message;
    // The BDB error number.
    3: optional i32 errorNumber;
}

/*
 * Singals that an attempt to remove a resource (environment or database) has
 * failed because the resource is being used by some clients.
 */
exception TResourceInUseException {
    1: string message;
}

/* Exceptions for common Java checked exceptions. */
exception TFileNotFoundException {
    1: string message;
}

exception TIOException {
    1: string message;
}

/* Exception for all Java RuntimeExceptions. */
exception TRuntimeException {
    1: string fullClassName;
    2: string message;
}


/*************************
 *  Service Definitions  *
 *************************/

/*
 * The remote service interface for BDB Server.
 */
service BdbService {

  ////////  Server info. & control methods.  ////////

    /*
     * For checking the server is reachable and responsive.
     */
    void ping();

    /*
     * Check if the protocol version used by the client is supported by the server.
     */
    TProtocolVersionTestResult isProtocolVersionSupported(1: string clientVersion);

    /*
     * Return the release version of BDB used by the server.
     */
    string getBdbVersion();

    /*
     * Instruct the server to shutdown.
     */
    oneway void shutdown();

  ////////  Environment methods.  ////////

    /*
     * Open an environment at the specified home directory using the specified
     * configuration and return a remote handle to the environment.
     *
     * @throws TIOException
     *     if the environment home directory is not accessible
     * @throws TResourceInUseException
     *     if the environment is being recovered or deleted
     */
    TEnvironment openEnvironment(1: string homeDir, 2: TEnvironmentConfig envConfig)
        throws (1: TIOException ioe,
                2: TResourceInUseException iue,
                3: TDatabaseException dbe,
                4: TRuntimeException re);

    /*
     * Close the remote environment handle and all handles opened within it.
     */
    void closeEnvironmentHandle(1: TEnvironment env)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Close environment handles which are opened at the specified home
     * directory and which have been idle longer than minIdelInMilli
     * milleseconds. All handles which depend on the closed environment handles
     * are also closed. If a close operation fails, this function continues to
     * close the rest of handles and throws the first exception occured.
     *
     * This function can be used to clean up inactive handles left over by
     * disconnected clients. In particular, all pending transactions on any
     * database opened by any idle environment are aborted to release locks.
     */
    void closeEnvironmentHandles(1: string homeDir, 2: i64 minIdleInMilli)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Delete the environment at the specified home directory and all of its
     * database and log files. This is in contrast with DB_ENV->remove() which
     * does NOT delete any log or database files.
     *
     * If the environment or any of its databases still has open handles,
     * TResourceInUseException is thrown unless force is true. In that case,
     * all open handles are forcibly closed and the environment is deleted.
     *
     * @throws TFileNotFoundException
     *     if the specified path is not a valid environment home directory
     * @throws TIOException
     *     if the environment or its databases cannot be deleted
     * @throws TResourceInUseException
     *     if the environment or any of its databases still has open handles
     */
    void deleteEnvironmentAndDatabases(1: string homeDir, 2: bool force)
        throws (1: TFileNotFoundException fe,
                2: TIOException ioe,
                3: TResourceInUseException iue,
                4: TDatabaseException dbe,
                5: TRuntimeException re);

    TEnvironmentConfig getEnvironmentConfig(1: TEnvironment env)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    void setEnvironmentConfig(1: TEnvironment env, 2: TEnvironmentConfig envConfig)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Remove the database specified by the file name and database name parameters.
     * If the database still has open handles, TResourceInUseException is thrown
     * unless force is true. In that case, all open handles are closed and the
     * database is removed.
     *
     * @throws TFileNotFoundException
     *     if no database file is found with the specified file name
     * @throws TResourceInUseException
     *     if the database still has open handles
     */
    void removeDatabase(1: TEnvironment env, 2: TTransaction txn,
        3: string fileName, 4: string databaseName, 5: bool force)
        throws (1: TIOException ioe,
                2: TResourceInUseException iue,
                3: TDatabaseException dbe,
                4: TRuntimeException re);

    /*
     * Rename the database specified by the file name and database name to the
     * new name. If the database still has open handles, TResourceInUseException
     * is thrown unless force is true. In that case, all open handles are closed
     * and the database is renamed.
     */
    void renameDatabase(1: TEnvironment env, 2: TTransaction txn,
        3: string fileName, 4: string databaseName, 5: string newName,
        6: bool force)
        throws (1: TIOException ioe,
                2: TResourceInUseException iue,
                3: TDatabaseException dbe,
                4: TRuntimeException re);

    /*
     * Perform a checkpoint if more than kbytes kilobytes of log data has been
     * written or min minutes has passed since last checkpoint. If force is ture,
     * always perform a checkpoint.
     */
    void checkpoint(1: TEnvironment env, 2: i32 kbytes, 3: i32 min,
        4: bool force)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

  ////////  Database methods.  ////////
    /*
     * Open a database represented by the file name and database name parameter,
     * and return a remote database handle.
     *
     * @throws TResourceInUseException
     *     if the database is being deleted or renamed
     */
    TDatabase openDatabase(1: TEnvironment env, 2: TTransaction txn,
        3: string fileName, 4: string databaseName, 5: TDatabaseConfig dbConfig)
        throws (1: TIOException ioe,
                2: TResourceInUseException iue,
                3: TDatabaseException dbe,
                4: TRuntimeException re);

    TDatabase openSecondaryDatabase(1: TEnvironment env, 2: TTransaction txn,
        3: string fileName, 4: string databaseName, 5: TDatabase primaryDb,
        6: TSecondaryDatabaseConfig sdbConfig)
        throws (1: TIOException ioe,
                2: TResourceInUseException iue,
                3: TDatabaseException dbe,
                4: TRuntimeException re)

    /*
     * Close the remote database handle and all handles opened within it.
     */
    void closeDatabaseHandle(1: TDatabase db)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Close database handles which are opened for the specified database
     * under the specified environment home directory and which have been idle
     * longer than minIdleInMilli milliseconds. All handles which depend on the
     * closed database handles are also closed. If a close operation fails,
     * this function continues to close the rest of handles and throws the
     * first exception occured.
     *
     * This function can be used to clean up inactive handles left over by
     * disconnected clients.
     */
    void closeDatabaseHandles(1: string envHomeDir, 2: string fileName,
        3: string databaseName, 4: i64 minIdleInMilli)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    TDatabaseConfig getDatabaseConfig(1: TDatabase db)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    void setDatabaseConfig(1: TDatabase db, 2: TDatabaseConfig dbConfig)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    TGetResult dbGet(1: TDatabase db, 2: TTransaction txn, 3: TKeyData keyData,
        4: TDbGetConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Similar to the pget function in C API, retrive secondary key/data pairs
     * with corresponding primary keys. The specified database must be a handle
     * to a secondary database.
     */
    TGetWithPKeyResult dbGetWithPKey(1: TDatabase sdb, 2: TTransaction txn,
        3: TKeyDataWithPKey keyPKey, 4: TDbGetConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Store key/data pair(s) into the database. Secondary keys (for secondary
     * databases only) are generated at the client side and passed to the
     * server using TKeyDataWithSecondaryKeys. For primary databases, secondary
     * keys are ignored and can be omitted.
     */
    TPutResult dbPut(1: TDatabase db, 2: TTransaction txn,
        3: list<TKeyDataWithSecondaryKeys> pairs, 4: TDbPutConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    TOperationStatus dbDelete(1: TDatabase db, 2: TTransaction txn,
        3: list<TKeyData> keyOrPairs)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    TOperationStatus dbKeyExists(1: TDatabase db, 2: TTransaction txn, 3: TDbt key)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    TKeyRangeResult dbKeyRange(1: TDatabase db, 2: TTransaction txn, 3: TDbt key)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Compact the specified database and optionally return unused pages.
     */
    TCompactResult dbCompact(1: TDatabase db, 2: TTransaction txn,
        3: TDbt start, 4: TDbt stop, 5: TCompactConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Empty the database, discarding all records it contains. Return the
     * number of records discarded if countRecords is true.
     */
    i32 dbTruncate(1: TDatabase db, 2: TTransaction txn, 3: bool countRecords)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

  ////////  Cursor methods.  ////////
    TCursor openCursor(1: TDatabase db, 2: TTransaction txn,
        3: TCursorConfig cursorConfig)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    void closeCursorHandle(1: TCursor cursor)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    TCursorConfig getCursorConfig(1: TCursor cursor)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Get the cache priority set for the cursur.
     */
    TCachePriority getCursorCachePriority(1: TCursor cursor)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Set the cache priority for pages referenced by the cursor handle.
     */
    void setCursorCachePriority(1: TCursor cursor, 2: TCachePriority priority)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    TGetResult cursorGet(1: TCursor cursor, 2: TKeyData keyData,
        3: TCursorGetConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * See dbGetWithPKey.
     */
    TGetWithPKeyResult cursorGetWithPKey(1: TCursor cursor,
        2: TKeyDataWithPKey keyPKey, 3: TCursorGetConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * See dbPut for secondary index handling.
     */
    TPutResult cursorPut(1: TCursor cursor,
        2: TKeyDataWithSecondaryKeys pair, 3: TCursorPutConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Delete the key/data pair currently referenced by the cursor.
     */
    TOperationStatus cursorDelete(1: TCursor cursor)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Create a new cursor that uses the same transaction as the specified
     * cursor.
     */
    TCursor cursorDup(1: TCursor cursor, 2: bool samePos)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Compare the position of two cursors.
     * Return: 0, two cursors are positioned on the same item
     *         1, cursor1 is positioned on the greater item
     *        -1, cursor2 is positioned on the greater item
     */
    i16 cursorCompare(1: TCursor cursor1, 2: TCursor cursor2)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Return the number of data items for the key to which the cursor refers.
     */
    i32 cursorCount(1: TCursor cursor)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Create a join cursor to perform equality joins on a list of secondary
     * index cursors.
     */
    TJoinCursor openJoinCursor(1: TDatabase pdb, 2: list<TCursor> scursors,
        3: bool sortCursors)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Close the join cursor.
     */
    void closeJoinCursorHandle(1: TJoinCursor cursor)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Get the next primary key/data pair resulting from the join operation.
     *
     * @throws TIllegalArgumentException
     *     if some configuration parameter is invalid
     */
    TGetResult joinCursorGet(1: TJoinCursor cursor,
        2: TJoinCursorGetConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

  ////////  Transaction methods.  ////////
    /*
     * Start a new transaction, optionally as a child transaction of the
     * specified parent transaction.
     */
    TTransaction beginTransaction(1: TEnvironment env, 2: TTransaction parent,
        3: TTransactionConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    void txnAbort(1: TTransaction txn)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    void txnCommit(1: TTransaction txn, 2: TDurabilityPolicy durability)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Get the transaction's priority. The deadlock detector will reject lock
     * requests from lower priority transactions.
     */
    i32 txnGetPriority(1: TTransaction txn)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    void txnSetPriority(1: TTransaction txn, 2: i32 priority)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

  ////////  Sequence methods.  ////////
    /*
     * Open a sequence represented by the key in the specified database.
     *
     * @throws TResourceInUseException
     *     if the sequence is being deleted
     */
    TSequence openSequence(1: TDatabase db, 2: TTransaction txn, 3: TDbt key,
        4: TSequenceConfig config)
        throws (1: TResourceInUseException iue,
                2: TDatabaseException dbe,
                3: TRuntimeException re);

    void closeSequenceHandle(1: TSequence seq)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    /*
     * Remove the sequence specified by the database and sequence key. If the
     * sequence still has open handles, TResourceInUseException is thrown
     * unless force is true. In that case, all open handles are closed and the
     * sequnece is removed.
     */
    void removeSequence(1: TDatabase db, 2: TTransaction txn, 3: TDbt key,
        4: bool isAutoCommitNoSync, 5: bool force)
        throws (1: TResourceInUseException iue,
                2: TDatabaseException dbe,
                3: TRuntimeException re);

    i64 sequenceGet(1: TSequence seq, 2: TTransaction txn, 3: i32 delta)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

  ////////  Statistic methods.  ////////
    TEnvStatResult getEnvStatistics(1: TEnvironment env, 2: TEnvStatConfig config)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);

    TDatabaseStatResult getDatabaseStatistics(1: TDatabase db,
        2: TTransaction txn, 3: bool fast)
        throws (1: TDatabaseException dbe,
                2: TRuntimeException re);
}